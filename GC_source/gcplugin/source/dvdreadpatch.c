/* Copyright 2009 WiiGator. */
#include "types.h"

#include "debugprintf.h"
#include "dvdreadpatch.h"
#include "writebranch.h"
#include "codecmp.h"
#include "cache.h"


/** Original DVD read function begins with the following bytes: */
static const u32 _Read_original[] = {
	0x7C0802A6,
	0x90010004,
	0x38000000,
	0x9421FFD8,
	0x93E10024,
	0x93C10020,
	0x3BC50000,
	0x93a1001c,
	0x3ba40000,
	0x93810018,
	0x3b830000,
	0x900d0c60,
	0x38000001,
	0x90cd0c68,
	0x0,
	0x0,
	0x900d0ca0,
	0x48017085,
	0x908d0c9c,
	0x3c80cc00,
	0x3c0000a0,
	0x906d0c98,
	0x38846000,
	0x3c60a800,
};

static const u32 _Read_original2[] = {
	0x7c0802a6,
	0x90010004,
	0x9421ffe0,
	0x93e1001c,
	0x90610008,
	0x7c9f2378,
	0x90a10010,
	0x90c10014,
	0x80010014,
	0x900da720,
	0x38000000,
	0x900da718,
	0x38000001,
	0x900da758,
};

static const u32 _Read_original3[] = {
	0x7c0802a6,
	0x90010004,
	0x38000001,
	0x9421ffd8,
	0x93e10024,
	0x93c10020,
	0x3bc50000,
	0x93a1001c,
	0x3ba40000,
	0x93810018,
	0x3b830000,
	0x90cd16bc,
	0x3cc0804d,
	0x3be6bdc0,
	0x900d16f8
};

#ifdef ACTION_REPLAY
/* This shouldn't be required inside the plugin
static const u32 ar_dvd_high_read[] = {
	// Freeloader, Action Relpay 
	0x9421ffe0,
	0x7c0802a6,
	0x9361000c,
	0x93810010,
	0x93a10014,
	0x93c10018,
	0x93e1001c,
	0x90010024,
	0x7c9c2378,
	0x7cbd2b78,
	0x7c7b1b78,
	0x4bfffebd,
	0x3d208130,
	0x3c00a800,
};

static const u32 ar_dvd_high_read_disk_id[] = {
	0x9421ffe8,
	0x7c0802a6,
	0x93a1000c,
	0x93c10010,
	0x93e10014,
	0x9001001c,
	0x7c7d1b78,
	0x4bffffb9,
	0x3c00a800,
	0x0,
	0x60000040,
};
*/
#endif

#ifdef GEKKO_DEBUG
// New patterns from Crediar
static const u32  __fwriteGC_orignal[] =
{
    0x9421FFD0,
	0x7C0802A6,
	0x90010034,
	0xBF210014, 
    0x7C992378,
	0x7CDA3378,
	0x7C7B1B78,
	0x7CBC2B78, 
    0x38800000,
	0x7F43D378
};

static const u32  __fwriteGC_orignal2[] =
{
    0x7C0802A6,
	0x90010004,
	0x9421FFB8,
	0xBF21002C, 
    0x3B440000,
	0x3B660000,
	0x3B830000,
	0x3B250000, 
    0x387B0000,
	0x38800000
};

/*	Old patterns from WiiGator
static const u32 stream_write_orignal[] = {
	0x7c0802a6,
	0x90010004,
	0x9421ffe0,
	0x93e1001c,
	0x3be50000,
	0x93c10018,
	0x3bc30000,
	0x38640000,
	0x38de0000,
	0x38800001
};

static const u32 stream_write_orignal2[] = {
	0x9421fff0,
	0x7c0802a6,
	0x90010014,
	0x93e1000c,
	0x7cbf2b78,
	0x93c10008,
	0x7c7e1b78,
	0x7c832378,
	0x38800001,
	0x7fc6f378
};

static const u32 stream_write_orignal3[] = {
	0x9421ffe0,
	0x7c0802a6,
	0x90010024,
	0x93e1001c,
	0x93c10018,
	0x7cbe2b78,
	0x90810008,
	0x38800001,
	0x93a10014,
	0x7c7d1b78,
	0x80610008,
	0x7fa6eb78
};
*/
#endif

static const u32 dvd_seek_original[] = {
	0x7c0802a6,
	0x90010004,
	0x38000000,
	0x9421ffe8,
	0x93e10014,
	0x93c10010,
	0x908d9478,
	0x3c80cc00,
	0x38846000,
	0x900d9470,
	0x3c00ab00,
	0x90040008
};

static const u32 dvd_read_audio_original[] = {
	0x7c0802a6,
	0x90010004,
	0x38000000,
	0x9421ffe0,
	0x93e1001c,
	0x93c10018,
	0x90cdad68,
	0x3cc0cc00,
	0x38c66000,
	0x900dad60,
	0x6460e100
};

static const u32 dvd_audio_config_original[] = {
	0x7c0802a6,
	0x2c030000,
	0x90010004,
	0x38000000,
	0x9421ffe0,
	0x93e1001c,
	0x93c10018,
	0x90adad68,
	0x900dad60,
	0x41820008,
	0x3c000001,
	0x6400e400,
	0x3c60cc00,
	0x7c800378,
	0x38636000,
	0x90030008
};

static const u32 _dvdlowreadid_org[] = {
	0x7C0802A6,
	0x39000000,
	0x90010004,
	0x3CA0A800,
	0x38050040,
	0x9421FFE8,
	0x38C00020,
	0x3CA08000,
	0x93E10014,
	0x93C10010	
};

static const u32 _dvdlowreadid_org2[] = {
	0x7c0802a6,
	0x3ca0a800,
	0x90010004,
	0x38050040,
	0x38c00020,
	0x9421ffe8,
	0x3ca08000,
	0x93e10014,
	0x93c10010,
	0x908d16bc,
	0x3c80cc00,
	0x38e46000,
	0x90046008
};
/*
static const u32 _dvdlowreadid_org3[] = {
	0x7c0802a6,
	0x90010004,
	0x9421ffe8,
	0x93e10014,
	0x7c7f1b78,
	0x9081000c,
	0x57e006ff,
	0x41820020,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x4cc63182,
	0x4be71191,
	0x8001000c,
	0x900da720,
	0x38000000,
	0x900da718,
	0x3c80a800,
	0x38840040,
	0x3c60cc00,
	0x90836008
};
*/
#ifdef GEKKO_DEBUG
static const u32 _dvdlowreset_org[] = {
	0x7C0802A6,
	0x3C80CC00,
	0x90010004,
	0x38000002,
	0x9421FFE0,
	0xBF410008,
	0x3BE43000,
	0x90046004,
	0x83C43024,
	0x57C007B8,
	0x60000001,
	0x941F0024
};

static const u32 dvd_stop_motor_original[] = {
	0x7c0802a6,
	0x90010004,
	0x38000000,
	0x9421ffe8,
	0x93e10014,
	0x93c10010,
	0x906d8988,
	0x3c60cc00,
	0x38836000,
	0x900d8980,
	0x3c00e300,
	0x90036008
};

static const u32 dvd_report_error_original[] = {
	0x7c0802a6,
	0x90010004,
	0x38000000,
	0x9421ffe8,
	0x93e10014,
	0x93c10010,
	0x906d8988,
	0x3c60cc00,
	0x38836000,
	0x900d8980,
	0x3c00e000,
	0x90036008
};
#endif

static const u32 dvd_audio_status_original[] = {
	0x7c0802a6,
	0x90010004,
	0x38000000,
	0x9421ffe8,
	0x93e10014,
	0x93c10010,
	0x908d8988,
	0x3c80cc00,
	0x38846000,
	0x900d8980,
	0x6460e200,
};

static const u32 memset_original[] = {
	0x28050020,
	0x5484063e,
	0x38c3ffff,
	0x7c872378,
	0x41800090,
	0x7cc030f8,
	0x540307bf,
	0x41820014,
	0x7ca32850,
	0x3463ffff,
	0x9ce60001,
	0x4082fff8
};

#if defined(RELOAD_SUPPORT) && defined(HIGH_PLUGIN) && defined(DONTTELL)
extern void patchhook(u32 address, u32 len);
#endif

/** Patch DVD read function. The patch should be applied only one time. */

#ifdef GEKKO_DEBUG
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_debug_t *jmp, u32 dvd_offset)
#else
#ifdef LOW_PLUGIN
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_low_t *jmp, u32 dvd_offset)
#endif
#ifdef HIGH_PLUGIN
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_high_t *jmp, u32 dvd_offset)
#endif
#endif
{
	void *addr_start = addr;
	void *addr_end = addr+len;

	if (reloader_status%2 != jmpNr)		// jump number changed
	{
		if (reloader_status > 3)
		{
			reloader_status=2+jmpNr;	// Just loaded apploader, now patch everything that is read
		} else
		{
			reloader_status = jmpNr;	// Back to looking for signs
		}
		elf_base_offset = 0;
	}	
	
	if ((u32)addr_start == 0x81300000)							// Apploader loading detected
	{
		debug_printf("Loading apploader?\n\n");
		reloader_status=4+jmpNr;
	}
	if (*(u32 *)addr == 0x7f454c46)	// elf header
	{
		debug_printf("Loading .elf?\n\n");
		elf_base_offset = *(u32 *)((u32)addr_start+0x3C) - ((dvd_offset - base_offset) << 2) - *(u32 *)((u32)addr_start+0x38);

		reloader_status=2+jmpNr;
	}

	if (mios_mode != 0)
	{
		debug_printf("Patch everything, still booting MIOS\n");
		reloader_status=2+jmpNr;
	}

	debug_printf("dvd_patchread reloader_status=%u\n", reloader_status);

	if ((reloader_status < 2 && reloader_enabled != 2) || reloader_enabled == 0)
	{
		return;
	}
	
	if (reloader_enabled == 3 && elf_base_offset != 0)
	{
		elf_offset = ((dvd_offset - base_offset) << 2) + elf_base_offset - (u32)addr_start;
	} else
	{
		elf_offset = 0;
	}

	while(addr_start<addr_end) {
#if defined(RELOAD_SUPPORT) && defined(HIGH_PLUGIN) && defined(DONTTELL)
	// Only patch hooks in game .dols
		if (reloader_status < 4 && hooks != 0
		&& codecmp(addr_start, (u32 *)hook, 64) == 0)
		{
			patchhook((unsigned long)addr_start, len);
		}
#endif		
		if (codecmp(addr_start,_Read_original,sizeof(_Read_original))==0)  {
			if (*(unsigned long*)(addr_start +0x5C) == 0x3c60a800) {
				debug_printf("Patching DVD read 1 at 0x%x\n", (u32) addr_start);

				install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

#ifdef GEKKO_DEBUG
				if (backuplaunching == 1)
				{
#endif
				// Change DVD command from 0xA8 to 0xD0:
				*(unsigned long*)(addr_start +0x5C) = 0x3C60D000;
				// Shift (>>11) the size and put in r3:
				*(unsigned long*)(addr_start +0x64) = 0x57A3AAFE;
				// Don't shift (>>2) the offset, because this is now LBA:
				*(unsigned long*)(addr_start +0x6C) = 0x93C4000C;
				// Store shifted size value:
				*(unsigned long*)(addr_start +0x74) = 0x90640010;
#ifdef GEKKO_DEBUG
				}
#endif
			}
		}

		if (codecmp(addr_start,_Read_original2,sizeof(_Read_original2))==0)  {
			debug_printf("Patching DVD read 2 at 0x%x\n", (u32) addr_start);

			install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

#ifdef GEKKO_DEBUG
				if (backuplaunching == 1)
				{
#endif
			// Change DVD command from 0xA8 to 0xD0:
			*(unsigned long*)(addr_start +0x44) = 0x3C00D000;
			// Don't shift (>>2) the offset, because this is now LBA:
			// Shift (>>11) the size and put in r4:
			*(unsigned long*)(addr_start +0x54) = 0x57E4AAFE;
			// Store shifted size value:
			*(unsigned long*)(addr_start +0x64) = 0x90836010;
#ifdef GEKKO_DEBUG
				}
#endif
		}

		if (codecmp(addr_start,_Read_original3,sizeof(_Read_original3))==0)  {
			debug_printf("Patching DVD read 3 at 0x%x\n", (u32) addr_start);

			install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

#ifdef GEKKO_DEBUG
				if (backuplaunching == 1)
				{
#endif
			// Change DVD command from 0xA8 to 0xD0:
			*(unsigned long*)(addr_start +0x54) = 0x3C60D000;
			// Shift (>>11) the size and put in r3:
			*(unsigned long*)(addr_start +0x5c) = 0x57A3AAFE;
			// Don't shift (>>2) the offset, because this is now LBA:
			*(unsigned long*)(addr_start +0x64) = 0x93C4000C;
			// Store shifted size value:
			*(unsigned long*)(addr_start +0x6c) = 0x90640010;
#ifdef GEKKO_DEBUG
				}
#endif
		}
		
		if ((codecmp(addr_start,_dvdlowreadid_org,sizeof(_dvdlowreadid_org))==0)
			|| (codecmp(addr_start,_dvdlowreadid_org2,sizeof(_dvdlowreadid_org2))==0)
			//|| (codecmp(addr_start,_dvdlowreadid_org3,sizeof(_dvdlowreadid_org3))==0)
			) {
			debug_printf("Patching DVD Read ID at 0x%x\n", (u32) addr_start);

#ifdef GEKKO_DEBUG
				if (backuplaunching == 1)
				{
#endif
			writebranch((void*) addr_start, (void *) jmp->dvd_read_id_replacement);
			//install_replacement((void*)addr_start, jmp->dvd_read_id_replacement, jmp->dvd_read_id_orig);
#ifdef GEKKO_DEBUG
				}
#endif

		}

		// All plugins do this
		//if (jmp->dvd_audio_status_replacement != 0)
		{
			if (codecmp(addr_start,dvd_audio_status_original,sizeof(dvd_audio_status_original))==0)
			{
				debug_printf("Patching DVD audio status at 0x%x\n", (u32) addr_start);
	
#ifdef GEKKO_DEBUG
				install_replacement((void*)addr_start, (void *) jmp->dvd_audio_status_replacement, (void *) jmp->dvd_audio_status_orig);
#else
				writebranch((void*) addr_start, (void *) jmp->dvd_audio_status_replacement);
#endif
			}
		}

		// All plugins do this
		//if (jmp->dvd_read_audio_replacement != 0)
		{
			if (codecmp(addr_start,dvd_read_audio_original,sizeof(dvd_read_audio_original))==0)
			{
				debug_printf("Patching DVD read audio at 0x%x\n", (u32) addr_start);
	
#ifdef GEKKO_DEBUG
				install_replacement((void*)addr_start, (void *) jmp->dvd_read_audio_replacement, (void *) jmp->dvd_read_audio_orig);
#else
				writebranch((void*) addr_start, (void *) jmp->dvd_read_audio_replacement);
#endif
			}
		}

		// All plugins do this
		//if (jmp->dvd_audio_config_replacement != 0)
		{
			if (codecmp(addr_start,dvd_audio_config_original,sizeof(dvd_audio_config_original))==0) {
				debug_printf("Patching DVD audio config at 0x%x\n", (u32) addr_start);
	
				writebranch((void*) addr_start, (void *) jmp->dvd_audio_config_replacement);
			}
		}
		
		// All plugins do this
		// if (jmp->memset_replacement != 0)
		{
			if (codecmp(addr_start, memset_original, sizeof(memset_original))==0)  {
				debug_printf("Patching memset at 0x%x\n", (u32) addr_start);
	
				install_replacement((void*)addr_start, (void *) jmp->memset_replacement, jmp->memset_orig);
			}
		}

		// All plugins do this
		//if (jmp->dvd_seek_replacement != 0)
		{
			if (codecmp(addr_start,dvd_seek_original,sizeof(dvd_seek_original))==0) {
				debug_printf("Patching DVD seek at 0x%x\n", (u32) addr_start);
	
#ifdef GEKKO_DEBUG
				if (backuplaunching == 1)
				{
#endif
				writebranch((void*) addr_start, (void *) jmp->dvd_seek_replacement);
#ifdef GEKKO_DEBUG
				}
#endif
			}
		}

#ifdef ACTION_REPLAY
/*
		// TODO check if this is necessary at all, does the reloader load action replay?
		//if (jmp->ar_dvd_read_replacement != 0)
		{
			if (codecmp(addr_start, ar_dvd_high_read, sizeof(ar_dvd_high_read))==0)  {
				debug_printf("Patching DVD ar read at 0x%x\n", (u32) addr_start);

				writeabsolutebranch((void*)addr_start, jmp->ar_dvd_read_replacement);

				// Get address for dvd wait cmd.
				jmp->ar_dvd_wait_for_last_cmd = (void *) get_bl_target((uint32_t *)(addr_start + 0x2c));
			}
		}

		// TODO check if this is necessary at all, does the reloader load action replay?
		//if (jmp->ar_dvd_read_disk_id_replacement != 0)
		{
			if (codecmp(addr_start, ar_dvd_high_read_disk_id, sizeof(ar_dvd_high_read_disk_id))==0)  {
				debug_printf("Patching DVD ar read disk id at 0x%x\n", (u32) addr_start);

				writeabsolutebranch((void*)addr_start, jmp->ar_dvd_read_disk_id_replacement);

				// Get address for dvd wait cmd.
				jmp->ar_dvd_wait_for_last_cmd = (void *) get_bl_target((uint32_t *)(addr_start + 0x1c));
			}
		}
*/		
#endif

#ifdef GEKKO_DEBUG
		// All plugins do this
		//if (jmp->dvd_report_error_replacement != 0)
		{
			if (codecmp(addr_start,dvd_report_error_original,sizeof(dvd_report_error_original))==0)  {
				debug_printf("Patching DVD report error at 0x%x\n", (u32) addr_start);
	
				install_replacement((void*)addr_start, (void *) jmp->dvd_report_error_replacement, (void *) jmp->dvd_report_error_orig);
			}
		}

		/* Patch debug printf. */
		//if (jmp->stream_write_replacement != 0)
		{
			if ((codecmp(addr_start, __fwriteGC_orignal, sizeof(__fwriteGC_orignal))==0)
				|| (codecmp(addr_start, __fwriteGC_orignal2, sizeof(__fwriteGC_orignal2))==0)) {
				debug_printf("Patching debug write at 0x%x\n", (u32) addr_start);

				writebranch((void*)addr_start, (void *) jmp->stream_write_replacement);
			}
		}
		if (codecmp(addr_start,_dvdlowreset_org,sizeof(_dvdlowreset_org))==0)
		{
			debug_printf("Patching DVD Reset at 0x%x\n", (u32) addr_start);

			install_replacement((void*)addr_start, (void *)jmp->dvd_reset_replacement,(void *) jmp->dvd_reset_orig);
		}

		//if (jmp->dvd_stop_motor_replacement != 0)
		{
			if (codecmp(addr_start,dvd_stop_motor_original,sizeof(dvd_stop_motor_original))==0)  {
				debug_printf("Patching DVD motor stop at 0x%x\n", (u32) addr_start);
	
				install_replacement((void*)addr_start, (void *) jmp->dvd_stop_motor_replacement, (void *) jmp->dvd_stop_motor_orig);
			}
		}
#endif
		addr_start += 4;
	}
}
