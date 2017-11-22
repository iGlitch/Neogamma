/* Copyright 2009 WiiGator. */
/* Read optimisation based on code from Waninkoko. */
#include "types.h"
#include "dvd.h"
#include "cache.h"
//#include "memcpy.h"
#include "exception.h"

#ifdef RELOAD_SUPPORT
#include "dvdreadpatch.h"
#endif

#include "dvd_orig.h"

#include "plugin.h"

#include "debugprintf.h"

#ifdef GEKKO_DEBUG
#include "usb.h"
#endif


/** DVD sector size. */
#define SECTOR_SIZE				0x800
/** Memory alignment needed for DMA and cache. */
//#define MEM_ALIGNMENT			0x20
/** Maximum size that can be read in one step. */
#define MAX_SECTOR_SIZE			0x7f8000
/** DVD return code for success. */
#define DVD_TRANSFER_COMPLETE 1
/** DVD return code for read error. */
#define DVD_DEVICE_ERROR 2
/** DVD return code for cover. */
//#define DVD_COVER_INTERRUPT 4
/** DVD return code for break. */
//#define DVD_BREAK_COMPLETE 8
/** DVD return code for timeout. */
//#define DVD_TIMEOUT 16
/** Memory must be aligned to 32 (DMA and cache line size). */
#define MEMORY_ALIGNMENT		0x20
/** Physical start address of DMA memory (mem1). */
#define DMA1_START_ADDRESS		0x00000000
/** Physical end address of DMA memory (mem1). */
#define DMA1_END_ADDRESS		0x01800000
/** Maximum read retried for each access. */
#define MAX_READ_RETRY 8

#ifdef GEKKO_DEBUG
#ifdef LOW_PLUGIN
#define debug_printf(...) /* not enough memory */
#endif
#endif

/** Temporary DVD buffer. */
static unsigned char dvd_buffer[SECTOR_SIZE] __attribute__((aligned(32)));
/** DVD callback function, should be called when transfer is complete. */
static interrupt_callback_t *orig_dvd_callback_function = NULL;
/** Source address for final memcpy (DVD temporary buffer.). */
static void *cpy_src;
/** Destination address for final memcpy (buffer for DVD read specified by caller. */
static void *cpy_dst = NULL;
/** Size for final memcpy (specified by caller). */
static unsigned long cpy_size;
/** Outbuf for last read cmd. */
static void *current_read_outbuf;
/** len  for last read cmd. */
static unsigned long current_read_len;
/** lba for last read cmd. */
static unsigned long current_read_lba;
/** Read error counter for one read. */
static unsigned long current_read_errors;

/* Global DI_loop variables. */
/** Number of Bytes read. */
static unsigned long global_read_cnt;
/** Next lba to read. */
static unsigned long global_read_lba;
/** Total length to read. */
static unsigned long global_read_len;
/** Next offset to read. */
static unsigned long global_read_offset;
/** Start address of read buffer specified by caller. */
static unsigned char *global_read_buffer;
/** Last sector read, which was succesfully read. Use this for DVD command simulation.
 * The access to this sector will be faster, because the data are still in the buffer.
 */
//static unsigned long last_sector = 0;

//static unsigned long audio_offset = 0;

int DI_loop(int result);


#ifdef GEKKO_DEBUG
#ifdef SHOW_FILENAMES
typedef struct {
    u8 filetype;
    char name_offset[3];
    u32 fileoffset;
    u32 filelen;
} __attribute__((packed)) FST_ENTRY;

static char systemfile[20] = "some system file";

char *getfilenamebyoffset(u32 offset, u32 *pos)
{
	FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
	
	u32 i;
	for (i=1;i<fst[0].filelen;i++)
	{
		if (fst[i].filetype == 0 && offset >= fst[i].fileoffset && offset < fst[i].fileoffset+fst[i].filelen)
		{
			*pos = offset - fst[i].fileoffset;
			return (char *)(*(u32 *)0x80000038 + fst[0].filelen*12 + (*(u32 *)&(fst[i]) & 0x00ffffff));
		}
	}
	*pos = 0;
	return &systemfile[0];
}
#endif
#endif


#ifdef HIGH_PLUGIN
void check_memory_setup_for_high_plugin()
{
	if (*(u32 *)0x80000028 == 0x01800000 || *(u32 *)0x800000EC == 0x81800000 || *(u32 *)0x800000F0 == 0x01800000)
	{
		/*
		debug_printf("Memory setup:\n");
		u32 i;
		for (i=0;i < 16;i++)
		{
			debug_printf("0x%08x, 0x%08x, 0x%08x, 0x%08x,\n", *(u32 *)(0x80000000+i*16), *(u32 *)(0x80000000+i*16+4), *(u32 *)(0x80000000+i*16+8), *(u32 *)(0x80000000+i*16+12));
		}
		*/
		*(u32 *)0x80000028 = 0x01800000 - HIGH_PLUGIN_SIZE;
		
		//TODO: Check if this is required, and if yes, how to get the fst size
		//*(u32 *)0x80000034 = 0x81800000 - HIGH_PLUGIN_SIZE - 0x16380;
		//*(u32 *)0x80000038 = 0x81800000 - HIGH_PLUGIN_SIZE - 0x1740;
		
		*(u32 *)0x800000EC = 0x81800000 - HIGH_PLUGIN_SIZE;
		*(u32 *)0x800000F0 = 0x01800000 - HIGH_PLUGIN_SIZE;	
		DCFlushRange_and_ICInvalidateRange((void *)0x80000000, 0x100);
		//ICInvalidateRange((void *)0x80000000, 0x100);
		
		debug_printf("\nReverted memory setup\n");
	}
}
#endif

/*
Currently not used by anything
#ifdef GEKKO_DEBUG
void hexDump(unsigned long *buffer, unsigned long size)
{
	int i;

	for (i = 0; i < size; i++) {
		if ((i > 0) && ((i & 0x3) == 0)) {
			debug_printf("\n");
		}
		if ((i & 0x3) == 0) {
			debug_printf("%x: ", i);
		}
		unsigned long l;

		l = buffer[i];
		debug_printf("%x", l);
		if ((i + 1) < size) {
			debug_printf(" ");
		}
	}
	debug_printf("\n");
}
#endif
*/

#ifdef HIGH_PLUGIN // Action replay support only in high plugin.
#ifdef ACTION_REPLAY
unsigned int dvd_read_sector(void *outbuf, int len, unsigned int lba)
{
	volatile uint32_t *dvdio = (volatile uint32_t *)0xCC006000;

	/* dst must be 32 Byte aligned. */
	/* Size should be 0x800. */
	if ((((unsigned int)outbuf) & 0xC0000000) == 0x80000000)
	{ /* cached? */
		dvdio[0] = 0x2E;

//#ifdef HIGH_PLUGIN // Only high version has enough memory.
		DCInvalidateRange(outbuf, len);
//#else

		// This may be slower.
		//DCFlushRange(outbuf, len);
		//ICInvalidateRange(outbuf, len);
//#endif
	}
	dvdio[1] = 0;	
	dvdio[2] = 0xD0000000;
	dvdio[3] = lba;
	dvdio[4] = len >> 11;
	dvdio[5] = (unsigned long)outbuf;
	dvdio[6] = len;
	dvdio[7] = 3; /* Start transfer. */
	while (dvdio[7] & 1);

	if (dvdio[0] & 0x4) {
		debug_printf("Read error\n");
		return 1;
	}
	return 0;
}

void ar_dvd_read_replacement1(unsigned long off, void *outbuf, unsigned long len)
{
	check_memory_setup_for_high_plugin();
	unsigned long cnt, lba;
	unsigned long offset;

	unsigned int ret = 0;
	debug_printf("\nar_dvd_read1 buf: 0x%08x off: 0x%08x size: 0x%08x\n", outbuf, off, len);

	jmpNr = 0;
	reloader_status=2;

	offset = base_offset + (off >> 2);

	/* Set variables */
	lba = offset >> 9;
	cnt = 0;

	while (cnt < len) {
		unsigned long size, skip;

		/* Skip bytes */
		skip = (offset > (lba << 9)) ? (offset - (lba << 9)) << 2 : 0;

		/* Data length */
		size = len - cnt;
		if (size + skip > SECTOR_SIZE)
			size = SECTOR_SIZE - skip;

		/* Do read */
		ret = dvd_read_sector(dvd_buffer, SECTOR_SIZE, lba);
		if (ret) {
			break;
		}

		//memcpy(outbuf + cnt, dvd_buffer + skip, size);
		/* Copy data */
		u8 *p = (u8 *)(dvd_buffer + skip);
		u8 *q = (u8 *)(outbuf + cnt);
		u32 cpysize = size;

		/** Code can be optimized for PPC. */
		while (cpysize--) {
			*q++ = *p++;
		}
#ifndef RELOAD_SUPPORT
		DCFlushRange_and_ICInvalidateRange(outbuf + cnt, size);
		//ICInvalidateRange(outbuf + cnt, size);
		DCFlushRange_and_ICInvalidateRange(dvd_buffer, SECTOR_SIZE);
		//ICInvalidateRange(dvd_buffer, SECTOR_SIZE);
#endif

		/* Update variables */
		cnt += size;
		lba++;
	}
#ifdef RELOAD_SUPPORT
	if (!ret) {
		dvd_patchread(outbuf, len, &jumptable[1], off);
		DCFlushRange_and_ICInvalidateRange(outbuf, len);
		//ICInvalidateRange(outbuf, len);
	}
#endif
}

void ar_dvd_read_disk_id_replacement1(void *buffer)
{
	check_memory_setup_for_high_plugin();
	debug_printf("\nar_dvd_read_disk_id1\n");
	u32 saved_offset;

	/* Switch disc: */
	saved_offset = base_offset;
	base_offset = base_offset_2nd_disc;
	base_offset_2nd_disc = saved_offset;

	ar_dvd_read_replacement1(0, buffer, 32);
}
#endif
#endif

/** Replacement for real DVD callback function. This is called if transfer is complete.
 * @param result DVD_TRANSFER_COMPLETE on success, DVD_DEVICE_ERROR on error.
 */
void DI_callback(int result)
{
	interrupt_callback_t *callback_function;
#ifdef SHOW_DVD_DETAILS
	debug_printf("DI_callback by 0x%x result 0x%x\n", __builtin_return_address(0), result);
#endif
#ifdef GEKKO_DEBUG
	if (backuplaunching == 0)
	{
		if (result & DVD_TRANSFER_COMPLETE)
		{
			dvd_patchread(global_read_buffer, global_read_len, &jumptable[!jmpNr], global_read_offset);
/*			if (jmpNr == 0) {
				dvd_patchread(global_read_buffer, global_read_len, &jumptable[1]);
			} else {
				dvd_patchread(global_read_buffer, global_read_len, &jumptable[0]);
			}
*/			DCFlushRange_and_ICInvalidateRange(global_read_buffer, global_read_len);
			//ICInvalidateRange(global_read_buffer, global_read_len);
		}

		callback_function = orig_dvd_callback_function;
		orig_dvd_callback_function = NULL;
		callback_function(result);
		return;
	}

#endif

	if (orig_dvd_callback_function != NULL)
	{
		if (result & DVD_TRANSFER_COMPLETE)
		{
			if (cpy_dst != NULL)
			{
				//last_sector = current_read_lba + ((current_read_len - 1) % SECTOR_SIZE);

				/* Copy the data from the dvd buffer to the real user buffer. */
				if (cpy_dst != cpy_src)
				{
#ifdef SHOW_DVD_DETAILS
					debug_printf("Copying dvd buffer 0x%x, 0x%x, 0x%x\n", cpy_dst, cpy_src, cpy_size);
#endif
					//memcpy(cpy_dst, cpy_src, cpy_size);
					u8 *p = (u8 *)cpy_src;
					u8 *q = (u8 *)cpy_dst;
					u32 size = cpy_size;

					/** Code can be optimized for PPC. */
					while (size--) {
						*q++ = *p++;
					}
				}
				/* The game is expecting a DMA transfer, so we need to flush to physical memory.
				 * XXX: Removing this will increase speed.
				 */
#ifndef RELOAD_SUPPORT
				DCFlushRange_and_ICInvalidateRange(cpy_dst, cpy_size);
				//ICInvalidateRange(cpy_dst, cpy_size);
#endif

				cpy_dst = NULL;
			} else
			{
#ifdef SHOW_DVD_DETAILS
				debug_printf("DMA transfer\n");
#endif
			}
			
			/* Process next data. */
			DI_loop(result);
		} else
		{
			if (current_read_errors >= MAX_READ_RETRY)
			{
				callback_function = orig_dvd_callback_function;
				orig_dvd_callback_function = NULL;
				callback_function(result);
				return;
			}

			debug_printf("DVD read error %d #%u\n", result, current_read_errors);
		
			/* Retry on error. */
			//debug_printf("outbuf 0x%x lba 0x%x len 0x%x\n", current_read_outbuf, current_read_lba, current_read_len);
			current_read_errors++;
#ifdef RELOAD_SUPPORT
			if (jmpNr == 0) {
#endif
				dvd_read_orig1(current_read_outbuf, current_read_len, current_read_lba, DI_callback);
#ifdef RELOAD_SUPPORT
			} else {
				dvd_read_orig2(current_read_outbuf, current_read_len, current_read_lba, DI_callback);
			}
#endif
		}
	} else
	{
		debug_printf("\n\nCallback from a finished read!\n\n");
	}
}

/** Execute read transfer.
 * @param outbuf Destination buffer for read data (must be 32 Byte aligned).
 * @param len Size of transfer (multiple of 0x800).
 * @param callback_fn Callback function if transfer is complete or on error.
 * @returns Always 0 (success)
 */
int DI_ReadDvd(void *outbuf, unsigned long len, unsigned long lba)
{
#ifdef GEKKO_DEBUG
#ifdef SHOW_DVD_DETAILS
	debug_printf("DI_ReadDvd() mem 0x%x, lba 0x%x, len 0x%x\n", outbuf, lba, len);
#endif
#if 0
	int i;

	/** DI hardware register (DVD). */
	volatile uint32_t *dvdio = (volatile uint32_t *)0xCC006000;
#endif
#endif

	/* Invalidate cache
	 * XXX: To increase speed move to DI_ReadSector, because the DVD driver should already have been done this.
	 */

//#ifdef HIGH_PLUGIN // Only high version has enough memory.
	DCInvalidateRange(outbuf, len);
//#else

	/* This may be slower. */
	//DCFlushRange(outbuf, len);
	//ICInvalidateRange(outbuf, len);
//#endif

	/* Set callback function. */
	//orig_dvd_callback_function = callback_fn;

	/* Store values, so read can be repeated on error. */
	current_read_outbuf = outbuf;
	current_read_len = len;
	current_read_lba = lba;
	current_read_errors = 0;

	//debug_printf("outbuf 0x%x lba 0x%x len 0x%x\n", outbuf, lba, len);

	/* Call original DVD read function with changed parameters. */
	/* The original DVD read function is patched to use 0xD0 instead of 0xA8. */
#ifdef RELOAD_SUPPORT
	if (jmpNr == 0) {
#endif
		dvd_read_orig1(outbuf, len, lba, DI_callback);
#ifdef RELOAD_SUPPORT
	} else {
		dvd_read_orig2(outbuf, len, lba, DI_callback);
	}
#endif

#ifdef GEKKO_DEBUG
#if 0
	/* Print DI register for debug purpose. */
	for (i = 0; i < 9; i++) {
		debug_printf("dvdio[0x%x] 0x%x\n", i, dvdio[i]);
	}
#endif
#endif
	return 0;
}

/** Read one sectors to dvd buffer and copy later to memory address where it is expected using offset pos within sector,
 * because of the aligment restrictions.
 * @param outbuf Destination buffer for read data (need not to be aligned).
 * @param len Size of transfer (need not to be a multiple of something).
 * @param pos Offset within sector.
 * @param lba lba * SECTOR_SIZE is start offset of sector.
 * @param callback_fn Callback function if transfer is complete or on error.
 * @returns Always 0 (success)
 */
int DI_ReadSector(void *outbuf, unsigned long len, unsigned long pos, unsigned long lba)
{
#ifdef SHOW_DVD_DETAILS
	debug_printf("DI_ReadSector b 0x%x, pos 0x%x, len 0x%x, lba 0x%x\n", outbuf, pos, len, lba);
#endif

	/* Check length */
	//This shouldn't be possible
	//if ((len + pos) > SECTOR_SIZE)
	//	return 1;
	
	/* Store what to copy in callback function. */
	cpy_src = dvd_buffer + pos;
	cpy_dst = outbuf;
	cpy_size = len;

	/* Use temporary dvd buffer for reading. */
	return DI_ReadDvd(dvd_buffer, SECTOR_SIZE, lba);
}

/** Check if data can be transfered using DMA for the expected target address.
 * @returns 0 if DMA is not possible, otherwise multiple of SECTOR_SIZE.
 */
unsigned long DI_DMACheck(unsigned long mem, unsigned long size)
{
	unsigned long ret = 0;

	mem = mem & 0x0FFFFFFF;

	/* Check for memory alignment */
	if (!(mem % MEMORY_ALIGNMENT))
	{
		unsigned long dmalen = 0;

		/* DMA1 range check */
		if ((mem >= DMA1_START_ADDRESS) && (mem < DMA1_END_ADDRESS))
			dmalen = (DMA1_END_ADDRESS - mem);

		if (dmalen > SECTOR_SIZE) {
			ret = (dmalen < size) ? dmalen : size;
			/* Align to SECTOR_SIZE. */
			ret -= ret % SECTOR_SIZE;
		}
	}

	return ret;
}

/** Each time called one block of data is transfered. Call callback function if finished.
 * @param callback_fn Callback function if transfer is complete or an error happened.
 * @param result Result of last transfer.
 * @return 0 on success
 * @return 1 on error
 */
int DI_loop(int result)
{
	if (result != DVD_TRANSFER_COMPLETE)
	{
		debug_printf("ERROR? DI_loop(), result 0x%x, read done: 0x%x/0x%x\n", result, global_read_cnt, global_read_len);
	} else
	{
#ifdef SHOW_DVD_DETAILS
		debug_printf("DI_loop(), result 0x%x, read done: 0x%x/0x%x\n", result, global_read_cnt, global_read_len);
#endif	
	}
	int ret;

	if ((global_read_cnt < global_read_len) && (result == DVD_TRANSFER_COMPLETE))
	{
		unsigned long dmalen, blksize, mem, off_lba, pos;

		/* Output buffer address */
		mem  = (unsigned long)(global_read_buffer + global_read_cnt);

		/* Set values */
		blksize = global_read_len - global_read_cnt;
		off_lba = global_read_lba << 9;
		pos     = (global_read_offset > off_lba) ? (global_read_offset - off_lba) << 2 : 0;

		/* Check size limit */
		if (blksize > MAX_SECTOR_SIZE)
			blksize = MAX_SECTOR_SIZE;

		/* Check DMA range. */
		dmalen = DI_DMACheck(mem, blksize);

		/* Use proper read method */
		if ((dmalen == 0) || pos) {
			/* Check size limit */
			if ((blksize + pos) > SECTOR_SIZE)
				blksize = SECTOR_SIZE - pos;

			/* Read sector */
			ret = DI_ReadSector((void *) mem, blksize, pos, global_read_lba);

			/* Update values */
			global_read_cnt += blksize;
			global_read_lba += (blksize + pos) >> 11;
		} else
		{
			/* Read data */
			ret = DI_ReadDvd((void *) mem, dmalen, global_read_lba);

			/* Update values */
			global_read_cnt += dmalen;
			global_read_lba += dmalen >> 11;
		}

		if (ret)
			return ret;
	} else
	{
		/* All data transfered. */
#ifdef RELOAD_SUPPORT
		if (result & DVD_TRANSFER_COMPLETE) {
			/* Check if the data, which the game load, need to be patched. */
			/* Patch jumptable which is currently not used. */
			dvd_patchread(global_read_buffer, global_read_len, &jumptable[!jmpNr], global_read_offset);
/*			if (jmpNr == 0) {
				dvd_patchread(global_read_buffer, global_read_len, &jumptable[1]);
			} else {
				dvd_patchread(global_read_buffer, global_read_len, &jumptable[0]);
			}
*/			DCFlushRange_and_ICInvalidateRange(global_read_buffer, global_read_len);
			//ICInvalidateRange(global_read_buffer, global_read_len);
		}
#endif
		interrupt_callback_t *callback_function = orig_dvd_callback_function;
		orig_dvd_callback_function = NULL;
		callback_function(result);
	}

	return 0;
}

/** Setup global variables for next request and transfer all data.
 * @param out Output buffer.
 * @param length Length of data to be read.
 * @param off DVD offset (ahiftes >> 2).
 * @param callback_fn Callback function if transfer is finished or has an error.
 */
int DI_ReadUnencrypted(unsigned char *out, unsigned long length, unsigned long off, interrupt_callback_t *callback_fn)
{
#ifdef SHOW_DVD_DETAILS
	debug_printf("DI_ReadUnencrypted() mem 0x%x, offset 0x%x, len 0x%x\n", out, off, length);
#endif	

	if (orig_dvd_callback_function != NULL)
	{
		debug_printf("Last transfer not finished, waiting...\n");
		while (orig_dvd_callback_function != NULL) ;
	}
	orig_dvd_callback_function = callback_fn;

	global_read_cnt = 0;
	global_read_len = length;
	global_read_offset = off;
	global_read_buffer = out;

	/* Initial LBA */
	global_read_lba = off >> 9;

	return DI_loop(DVD_TRANSFER_COMPLETE);
}

void dvd_read_replacementfunction(
	void *buffer,
	unsigned long size,
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
#ifdef GEKKO_DEBUG
	exception_init();

#ifdef SHOW_FILENAMES
	u32 pos;
	char *s;
	if (mios_mode != 0)
	{
		s = getfilenamebyoffset(offset << 2, &pos);
	} else
	{
		s = getfilenamebyoffset(offset, &pos);
	}
	debug_printf("\ndvd_read%u buf: 0x%08x off: 0x%08x size: 0x%08x file: %s", jmpNr+1, buffer, offset, size, s);
	if (pos > 0)
	{
		debug_printf("(@0x%08x)", pos);
	}
	debug_printf("\n");
#else
	debug_printf("\ndvd_read%u buf: 0x%08x off: 0x%08x size: 0x%08x\n", jmpNr+1, buffer, offset, size);
#endif

	if (backuplaunching == 1)
	{
		int ret;
		if (mios_mode != 0)
		{
			check_memory_setup_for_high_plugin();
			// MIOS use unshifted offset, later reloaded games use shifted offset.
			ret = DI_ReadUnencrypted(buffer, size, base_offset + offset, callback_fn);
		} else
		{
			ret = DI_ReadUnencrypted(buffer, size, base_offset + (offset >> 2), callback_fn);
		}
		if (ret)
		{
			debug_printf("Error: DI_ReadUnencrypted() failed ret = %d\n", ret);
		}
	} else
	{
		global_read_buffer = buffer;
		global_read_len = size;
		orig_dvd_callback_function = callback_fn;		

		if (jmpNr == 0)
		{
			if (mios_mode != 0)
			{
				check_memory_setup_for_high_plugin();
			}
			dvd_read_orig1(buffer, size, offset, DI_callback);
		} else
		{
			dvd_read_orig2(buffer, size, offset, DI_callback);
		}
	}
#else
	if (mios_mode != 0)
	{
#ifdef HIGH_PLUGIN
		check_memory_setup_for_high_plugin();
#endif	
		// MIOS use unshifted offset, later reloaded games use shifted offset.
		DI_ReadUnencrypted(buffer, size, base_offset + offset, callback_fn);
	} else
	{
		DI_ReadUnencrypted(buffer, size, base_offset + (offset >> 2), callback_fn);
	}
#endif
}

/** This function replaces the original DVD read function.
 * @param buffer Destination address for DVD read (this is always 32 Byte aligned).
 * @param size Length of data to be read.
 * @param offset Offset to DVD data (already shifted by >> 2 by MIOS), Byte offset to DVD data in games.
 * param callback_fn Function is called if DVD transfer is complete or on error.
 */
void dvd_read_replacement1(
	void *buffer,
	unsigned long size,
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif
	dvd_read_replacementfunction(buffer, size, offset, callback_fn);
}

#ifdef RELOAD_SUPPORT
/** This function replaces the original DVD read function.
 * @param buffer Destination address for DVD read (this is always 32 Byte aligned).
 * @param size Length of data to be read.
 * @param offset Offset to DVD data, Byte offset to DVD data in games.
 * param callback_fn Function is called if DVD transfer is complete or on error.
 */
void dvd_read_replacement2(
	void *buffer,
	unsigned long size,
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
	jmpNr = 1;
	mios_mode = 0;	// (jmpNr == 1) ==> MIOS is done
	dvd_read_replacementfunction(buffer, size, offset, callback_fn);
}
#endif

/** This function replaces the original DVD read disk id function.
 * @param buffer Destination address for read.
 * @param callback_fn Callback function if transfer is complete or on error.
 */
void dvd_read_id_replacement1(
	void *buffer,
	interrupt_callback_t *callback_fn)
{
	exception_init();
#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif

	/* This function is called after dvd_reset_replacement(). */
	debug_printf("\ndvd_read_id1\n");

#if 0
	dvd_read_id_orig1(buffer, callback_fn);
#else

	u32 saved_offset;

	if (mios_mode == 0)	// Don't switch the disc while booting MIOS
	{
		/* Switch disc: */
		saved_offset = base_offset;
		base_offset = base_offset_2nd_disc;
		base_offset_2nd_disc = saved_offset;
	}
	
	/* Simulate DVD Read Disc ID by reading first 32 Bytes of disc. */
	DI_ReadUnencrypted(buffer, 32, base_offset, callback_fn);
	
#endif
}

#ifdef RELOAD_SUPPORT
/** This function replaces the original DVD read disk id function.
 * @param buffer Destination address for read.
 * @param callback_fn Callback function if transfer is complete or on error.
 */
void dvd_read_id_replacement2(
	void *buffer,
	interrupt_callback_t *callback_fn)
{
	exception_init();
	jmpNr = 1;
	mios_mode = 0;

	/* This function is called after dvd_reset_replacement(). */
	debug_printf("\ndvd_read_id2\n");

	u32 saved_offset;

	/* Switch disc. */
	saved_offset = base_offset;
	base_offset = base_offset_2nd_disc;
	base_offset_2nd_disc = saved_offset;
#if 0
	dvd_read_id_orig2(buffer, callback_fn);
#else
	/* Simulate DVD Read Disc ID by reading first 32 Bytes of disc. */
	DI_ReadUnencrypted(buffer, 32, base_offset, callback_fn);
#endif
}
#endif

void dvd_seek_replacement1(
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	//jmpNr = 0;
#endif

	debug_printf("\ndvd_seek1 off: 0x%08x\n", offset);

	callback_fn(1);
	
	/* Data not important, just read from the specific lba. */
	//offset = (offset + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);

	/* Read last sector to simulate seek. */
	//DI_ReadUnencrypted(dvd_buffer, SECTOR_SIZE, last_sector << 9, callback_fn);
}

#ifdef RELOAD_SUPPORT
void dvd_seek_replacement2(
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
	exception_init();

	//jmpNr = 1;

	debug_printf("\ndvd_seek2 off: 0x%08x\n", offset);

	callback_fn(1);
	
	/* Data not important, just read from the specific lba. */
	//offset = (offset + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1);

	/* Read last sector to simulate seek. */
	//DI_ReadUnencrypted(dvd_buffer, SECTOR_SIZE, last_sector << 9, callback_fn);
}
#endif



void dvd_audio_config_replacement1(
	unsigned long activate,
	unsigned long size,
	interrupt_callback_t *callback_fn)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	//jmpNr = 0;
#endif

	debug_printf("\ndvd_read_config1 activate 0x%08x size 0x%08x\n", activate, size);

	callback_fn(1);
}

#ifdef RELOAD_SUPPORT
void dvd_audio_config_replacement2(
	unsigned long activate,
	unsigned long size,
	interrupt_callback_t *callback_fn)
{
	exception_init();

	//jmpNr = 1;

	debug_printf("\ndvd_read_config2 activate 0x%08x size 0x%08x\n", activate, size);

	callback_fn(1);
	
	/* Read one sector to simulate audio config. */
	/* XXX: Use a faster call for simulating audio buffer config. */
	//DI_ReadUnencrypted(dvd_buffer, SECTOR_SIZE, last_sector << 9, callback_fn);
}
#endif

void dvd_audio_status_replacement1(
	u32 subcmd,
	interrupt_callback_t *function)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	//jmpNr = 0;	Not required here
#endif

#ifdef GEKKO_DEBUG
	debug_printf("\ndvd_audio_status1 cmd: 0x%08x\n", subcmd);
	
/*
	u32 result;
	__asm__ __volatile__ (
		"mftb	%0\n"
		: "=r" (result)
	);


	debug_printf("Time: 0x%08x\n", result);
*/	
	if (backuplaunching == 1)
	{
		// Fake enabled audio streaming
		if (audio_stream_fix == 1 && subcmd == 0)
		{
			*(u32 *)0xCC006020 = 1;
			debug_printf("returning: 1\n");
		} else
		{
			*(u32 *)0xCC006020 = 0;
			debug_printf("returning: 0\n");
		}
		
		function(1);
	} else
	{
		dvd_audio_status_orig1(subcmd, function);
		debug_printf("returned: 0x%x\n", *(u32 *)0xCC006020);
	}
#else
	// Fake enabled audio streaming
	if (audio_stream_fix == 1 && subcmd == 0)
	{
		*(u32 *)0xCC006020 = 1;
	} else
	{
		*(u32 *)0xCC006020 = 0;
	}
	
	function(1);

	/* Simulate by calling report error. */
	//dvd_report_error_orig1(function);
#endif
}

#ifdef RELOAD_SUPPORT
void dvd_audio_status_replacement2(
	u32 subcmd,
	interrupt_callback_t *function)
{
	exception_init();

	//jmpNr = 1;	Not required here

#ifdef GEKKO_DEBUG
	debug_printf("\ndvd_audio_status2 cmd: 0x%08x\n", subcmd);

/*	u32 result;
	__asm__ __volatile__ (
		"mftb	%0\n"
		: "=r" (result)
	);

	debug_printf("Time: 0x%08x\n", result);
*/
	if (backuplaunching == 1)
	{
		// Fake enabled audio streaming
		if (audio_stream_fix == 1 && subcmd == 0)
		{
			*(u32 *)0xCC006020 = 1;
			debug_printf("returning: 1\n");
		} else
		{
			*(u32 *)0xCC006020 = 0;
			debug_printf("returning: 0\n");
		}
		
		function(1);
	} else
	{
		dvd_audio_status_orig2(subcmd, function);
		debug_printf("returned: 0x%x\n", *(u32 *)0xCC006020);
	}
#else
	// Fake enabled audio streaming
	if (audio_stream_fix == 1 && subcmd == 0)
	{
		*(u32 *)0xCC006020 = 1;
	} else
	{
		*(u32 *)0xCC006020 = 0;
	}
	
	function(1);

	/* Simulate by calling report error. */
	//dvd_report_error_orig1(function);
#endif
}
#endif

void dvd_read_audio_replacement1(
	unsigned long subcmd,
	unsigned long length,
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	//jmpNr = 0;
#endif

/*	if (length != 0)
	{
		audio_offset = (offset>>2) + length;
	}*/

#ifdef GEKKO_DEBUG

#ifdef SHOW_FILENAMES
	u32 pos;
	char *s;
	if (offset == 0)
	{
		debug_printf("\ndvd_read_audio1 cmd: 0x%08x off: 0x%08x size: 0x%08x\n", subcmd, offset, length);
	} else
	{
		s = getfilenamebyoffset(offset, &pos);
		debug_printf("\ndvd_read_audio1 cmd: 0x%08x off: 0x%08x size: 0x%08x file: %s\n", subcmd, offset, length, s);
		if (pos > 0)
		{
			debug_printf("(@0x%08x)", pos);
		}
		debug_printf("\n");
	}
#else
	debug_printf("\ndvd_read_audio1 cmd: 0x%08x off: 0x%08x size: 0x%08x\n", subcmd, offset, length);
#endif

	if (backuplaunching == 1)
	{
		callback_fn(1);
	} else
	{
		dvd_read_audio_orig1(subcmd, length, offset, callback_fn);
	}
#else
	callback_fn(1);
#endif
}

#ifdef RELOAD_SUPPORT
void dvd_read_audio_replacement2(
	unsigned long subcmd,
	unsigned long length,
	unsigned long offset,
	interrupt_callback_t *callback_fn)
{
	exception_init();

	//jmpNr = 1;

/*	if (length != 0)
	{
		audio_offset = (offset>>2) + length;
	}*/

#ifdef GEKKO_DEBUG

#ifdef SHOW_FILENAMES
	u32 pos;
	char *s;
	if (offset == 0)
	{
		debug_printf("\ndvd_read_audio2 cmd: 0x%08x off: 0x%08x size: 0x%08x\n", subcmd, offset, length);
	} else
	{
		s = getfilenamebyoffset(offset, &pos);
		debug_printf("\ndvd_read_audio2 cmd: 0x%08x off: 0x%08x size: 0x%08x file: %s\n", subcmd, offset, length, s);
		if (pos > 0)
		{
			debug_printf("(@0x%08x)", pos);
		}
		debug_printf("\n");
	}
#else
	debug_printf("\ndvd_read_audio1 cmd: 0x%08x off: 0x%08x size: 0x%08x\n", subcmd, offset, length);
#endif

	if (backuplaunching == 1)
	{
		callback_fn(1);
	} else
	{
		dvd_read_audio_orig1(subcmd, length, offset, callback_fn);
	}
#else
	callback_fn(1);
#endif
}
#endif



#ifdef GEKKO_DEBUG
void dvd_report_error_replacement1(
	interrupt_callback_t *function)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	//jmpNr = 0;	Not required here
#endif

	debug_printf("\ndvd_report_error1\n");
	dvd_report_error_orig1(function);
}

#ifdef RELOAD_SUPPORT
void dvd_report_error_replacement2(
	interrupt_callback_t *function)
{
	exception_init();

	//jmpNr = 1;	Not required here

	debug_printf("\ndvd_report_error2\n");
	dvd_report_error_orig2(function);
}
#endif

/** This function replaces the original DVD reset function. */
void dvd_reset_replacement1(void)
{
	exception_init();
#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif

	/* This function will be called if DVD was ejected and reinserted.*/
	debug_printf("\ndvd_reset1\n");

	dvd_reset_orig1();
}

#ifdef RELOAD_SUPPORT
/** This function replaces the original DVD reset function. */
void dvd_reset_replacement2(void)
{
	exception_init();
	jmpNr = 1;

	/* This function will be called if DVD was ejected and reinserted.*/
	debug_printf("\ndvd_reset2\n");

	dvd_reset_orig2();
}
#endif

#ifdef REPLACE_DVD_CALLBACK
/** Replacement for callback in DVD driver code (only for debugging).
 * @param result Result of last transfer.
 * @param callback_fn Callback function that would normally be called.
 */
void dvd_callback_replacement1(int result, interrupt_callback_t *callback_fn)
{
#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif

	debug_printf("\ndvd_callback1 result 0x%x, fn 0x%x\n", result, callback_fn);
	callback_fn(result);
}

#ifdef RELOAD_SUPPORT
/** Replacement for callback in DVD driver code (only for debugging).
 * @param result Result of last transfer.
 * @param callback_fn Callback function that would normally be called.
 */
void dvd_callback_replacement2(int result, interrupt_callback_t *callback_fn)
{
	exception_init();
	jmpNr = 1;

	debug_printf("\ndvd_callback2 result 0x%x, fn 0x%x\n", result, callback_fn);
	callback_fn(result);
}
#endif
#endif



/*
Currently not used by anything
void printDVDRegs(void)
{
	int i;

	// DI hardware register (DVD).
	volatile uint32_t *dvdio = (volatile uint32_t *)0xCC006000;

	// Print DI register for debug purpose.
	for (i = 0; i < 9; i++) {
		debug_printf("dvdio[0x%x] 0x%x\n", i, dvdio[i]);
	}
}
*/

void dvd_stop_motor_replacement1(
	interrupt_callback_t *function)
{
	exception_init();

#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif

	debug_printf("\ndvd_stop_motor1\n");
	dvd_stop_motor_orig1(function);
}

#ifdef RELOAD_SUPPORT
void dvd_stop_motor_replacement2(
	interrupt_callback_t *function)
{
	exception_init();

	jmpNr = 1;

	debug_printf("\ndvd_stop_motor2\n");
	dvd_stop_motor_orig2(function);
}
#endif

#endif





