/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdint.h>
#include <gctypes.h>
#include <ogc/cache.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "dvd.h"
#include "debugprintf.h"

#define SECTOR_SIZE 0x800

static volatile unsigned long* dvd = (volatile unsigned long*) 0xCC006000;
static volatile unsigned long *piio = (volatile unsigned long *) 0xCC003000;
unsigned long base_offset = 0;

bool backuploading = false;

/** After calling this function, the DVD drive can be reseted (needed for disc change). */
void wii_dvd_reset_unlock(void)
{
	volatile unsigned long *pi_cmd = (unsigned long *) 0xCC003024;
	volatile unsigned long *ios_magic = (unsigned long *) 0x800030F8;
	volatile unsigned long *ios_sync_base = (unsigned long *) 0x800030E0;
	volatile unsigned long *phys_ios_magic = (unsigned long *) 0xC00030F8;

	debug_printf("wii_dvd_reset_unlock()\n");

	/* I assume this will synchronize with Starlet/MIOS. */
	*pi_cmd |= 7;

	*ios_magic = 0xDEAEBEEF;
	DCFlushRange((void *) ios_sync_base, 32);

	while(*phys_ios_magic != 0) {
		/* I assume this waits for Starlet. */
	}
}


unsigned long ack_cover_interrupt(void)
{
	unsigned int val;

	val = dvd[0];
	//debug_printf("DISR 0x%x\n", val);
	dvd[0] = val;

	return val;
}

/** Get cover status and clear interrupt.
 * @returns Cover status
 * @retval 0 if cover is closed.
 * @retval 1 if cover is open.
 */
unsigned long get_cover_status(void)
{
	unsigned int val;

	val = dvd[1];
	//debug_printf("DICVR 0x%x\n", val);
	dvd[1] = val;

	return val & 0x05;
}

void dvd_reset(void)
{
	unsigned long v;

	debug_printf("dvd_reset()\n");

	/* Clear cover interrupt. */
	dvd[1] = 2;

	v = piio[9];

	piio[9] = (v & ~4) | 1;
	usleep(1000);
	piio[9] = v | 5;
}

void dvd_motor_off(void)
{
	dvd[0] = 0x2E;
	dvd[1] = 0;

	dvd[2] = 0xe3000000;
	dvd[3] = 0;
	dvd[4] = 0;
	dvd[5] = 0;
	dvd[6] = 0;
	dvd[7] = 1; // enable reading!
	while (dvd[7] & 1);

	ack_cover_interrupt();
}


unsigned int dvd_read_sector(void *outbuf, int len, unsigned int lba)
{
	unsigned long disr;

	/* dst must be 32 Byte aligned. */
	/* Size should be 0x800. */
	//debug_printf("dvd_read_sector(outbuf 0x%08x, len 0x%x, lba 0x%x)\n", outbuf, len, lba);
	if ((((unsigned int)outbuf) & 0xC0000000) == 0x80000000) { // cached?
		dvd[0] = 0x2E;
		DCInvalidateRange(outbuf, len);
	}
	dvd[1] = 0;	
	dvd[2] = 0xD0000000;
	dvd[3] = lba;
	dvd[4] = len >> 11;
	dvd[5] = (unsigned long)outbuf;
	dvd[6] = len;
	dvd[7] = 3; // enable reading!	
	while (dvd[7] & 1);

	disr = ack_cover_interrupt();
	if (disr & 0x4) {
		debug_printf("Read error\n");
		
		return 1;
	}
	return 0;
}

unsigned int dvd_read_offset(void *outbuf, int len, unsigned int offset)
{
	unsigned long disr;

	/* dst must be 32 Byte aligned. */
	/* Size should be 0x800. */
	//debug_printf("dvd_read_sector(outbuf 0x%08x, len 0x%x, lba 0x%x)\n", outbuf, len, lba);
	if ((((unsigned int)outbuf) & 0xC0000000) == 0x80000000) { // cached?
		dvd[0] = 0x2E;
		DCInvalidateRange(outbuf, len);
	}
	dvd[1] = 0;	
	dvd[2] = 0xa8000000;
	dvd[3] = offset;
	dvd[4] = len;
	dvd[5] = (unsigned long)outbuf;
	dvd[6] = len;
	dvd[7] = 3; // enable reading!	
	while (dvd[7] & 1);

	disr = ack_cover_interrupt();
	if (disr & 0x4) {
		debug_printf("Read error\n");
		
		return 1;
	}
	return 0;
}

unsigned int dvd_read(void *outbuf, int len, unsigned long long off)
{
	static unsigned char dvdbuf[SECTOR_SIZE] __attribute__((aligned(32)));
	unsigned long cnt, lba;
	unsigned long offset;

	unsigned int ret = 0;
	debug_printf("dvd_read(outbuf 0x%08x, len 0x%x, offset 0x%x)\n", outbuf, len, off);
	//printf("dvd_read(outbuf 0x%08x, len 0x%x, offset 0x%x)\n", (unsigned long)outbuf, len, off);

	offset = base_offset + (off >> 2);

	/* Set variables */
	lba = offset >> 9;
	cnt = 0;

	while (cnt < len)
	{
		unsigned long size, skip;

		/* Data length */
		size = len - cnt;

		if (!backuploading)
		{
			if (size > SECTOR_SIZE)
				size = SECTOR_SIZE;
				
			/* Do read */
			ret = dvd_read_offset(dvdbuf, size, offset);
			if (ret)
			{
				// Do a retry first
				ret = dvd_read_offset(dvdbuf, size, offset);
				if (ret)
				{
					// read error
					break;
					//backuploading = true;
				}
			}

			if (!backuploading)
			{
				memcpy(outbuf + cnt, dvdbuf, size);
				DCFlushRange(outbuf + cnt, size);
				ICInvalidateRange(outbuf + cnt, size);
				DCFlushRange(dvdbuf, SECTOR_SIZE);
				ICInvalidateRange(dvdbuf, SECTOR_SIZE);
				/* Update variables */
				cnt += size;
				offset += size >> 2;
			}
		}	

		if (backuploading)
		{
			/* Skip bytes */
			skip = (offset > (lba << 9)) ? (offset - (lba << 9)) << 2 : 0;

			if (size + skip > SECTOR_SIZE)
				size = SECTOR_SIZE - skip;

			/* Do read */
			ret = dvd_read_sector(dvdbuf, SECTOR_SIZE, lba);
			if (ret)
			{
				// Do a retry first
				ret = dvd_read_sector(dvdbuf, SECTOR_SIZE, lba);
				if (ret)
				{
					// read error
					break;
					//backuploading = false;
				}
			}
			if (backuploading)
			{
				/* Copy data */
				memcpy(outbuf + cnt, dvdbuf + skip, size);
				DCFlushRange(outbuf + cnt, size);
				ICInvalidateRange(outbuf + cnt, size);
				DCFlushRange(dvdbuf, SECTOR_SIZE);
				ICInvalidateRange(dvdbuf, SECTOR_SIZE);
				/* Update variables */
				cnt += size;
				lba++;
			}
		}

	}

	debug_printf("dvd_read() ret = %d\n", ret);
	return ret;
}

unsigned int dvd_read_id(void)
{
	dvd[0] = 0x2E;
	dvd[1] = 0;
	dvd[2] = 0xA8000040;
	dvd[3] = 0;
	dvd[4] = 0x20;
	dvd[5] = 0x80000000;
	dvd[6] = 0x20;
	dvd[7] = 3; // enable reading!
	
	while (dvd[7] & 1);
	if (dvd[0] & 0x4)
	{
		backuploading = true;
		int rv = dvd_read((void *) 0x80000000, 0x20, 0);
		DVD_CHECK();
	} else
	{
		backuploading = false;
	}
	return 0;
}

unsigned int dvd_get_error(void)
{
	dvd[2] = 0xE0000000;
	dvd[8] = 0;
	dvd[7] = 1;
	while (dvd[7] & 1);

	ack_cover_interrupt();

	return dvd[8];
}

const char *dvd_get_error_message(unsigned int err)
{
	switch (err >> 24)
	{
		/* as documented in NinDoc */
	case 0x00:
		return "Ready.";
	case 0x01:
		return "Cover is opened.";
	case 0x02:
		return "Disk change.";
	case 0x03:
		return "No Disk.";
	case 0x04:
		return "Motor stop.";
	case 0x05:
		return "Disk ID not read.";
	default:
		return "Modified";
	}
}

s32 dvd_set_streaming(void)
{
	if ( (*(u32*)0x80000008)>>24 )
	{
		dvd[0] = 0x2E;
		if( ((*(u32*)0x80000008)>>16) & 0xFF )
		{
			dvd[2] = 0xE4000000 | (1<<16) | 0xA;
			dvd[7] = 1;
			while (dvd[7] & 1);
			return 2;
		} else 
		{
			dvd[2] = 0xE4000000 | (1<<16) | 0;
			dvd[7] = 1;
			while (dvd[7] & 1);
			return 1;
		}
	}
	else
	{		
		dvd[0] = 0x2E;
		dvd[2] = 0xE4000000 | (0<<16) | 0;
		dvd[7] = 1;
		while (dvd[7] & 1);
		return 0;
	}
}

void dvd_disable_streaming(void)
{
	dvd[0] = 0x2E;
	dvd[2] = 0xE4000000 | (0<<16) | 0; 
	dvd[7] = 1;
	while (dvd[7] & 1);
}

s32 dvd_RequestAudioStatus(u32 subcmd)
{
// Possible values for subcmd 0, 0x00010000, 0x00020000 and 0x00030000
	dvd[2] = 0xE2000000|subcmd;	 
	dvd[7] = 1;
	while (dvd[7] & 1);	

	return 0;
}


