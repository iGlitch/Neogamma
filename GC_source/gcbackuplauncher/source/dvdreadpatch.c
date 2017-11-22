/* Copyright 2009 WiiGator. */
#include <stdint.h>
#include <stdio.h>
#include <gctypes.h>
#include <string.h>
#include <ogc/cache.h>

#include "debugprintf.h"
#include "dvdreadpatch.h"
#include "writebranch.h"
#include "codecmp.h"
#include "dvd.h"

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

static const u32 ar_dvd_high_read[] = {
	/* Freeloader, Action Relpay */
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

static const u32 _Read_originalMIOSv5[] = {
	0x9421fff0,
	0x7c0802a6,
	0x38e00000,
	0x90010014,
	0x93e1000c,
	0x93c10008,
	0x800d8274,
	0x90cd8230,
	0x2c000000,
	0x90ed8208,
	0x40820008,
	0x3ca50009,
	0x3ce0cc00,
	0x3c00a800,
	0x90076008
};

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

static const u32 dvd_seek_MIOS_original[] = {
	0x9421fff0,
	0x7c0802a6,
	0x38e00000,
	0x3ca0cc00,
	0x90010014,
	0x3cc0ab00,
	0x38000001,
	0x93e1000c,
	0x93c10008,
	0x908d8230,
	0x3c808000,
	0x90ed8208,
	0x90c56008
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

static const u32 _dvdlowreadid_orgMIOSv5[] = {
	0x9421fff0,
	0x7c0802a6,
	0x3ca0a800,
	0x39000000,
	0x90010014,
	0x38e50040,
	0x3cc0cc00,
	0x38a00020,
	0x93e1000c,
	0x38000003,
	0x93c10008
};

static const u32 dvd_audio_config_MIOS_original[] = {
	0x9421fff0,
	0x7c0802a6,
	0x38e00000,
	0x3cc0cc00,
	0x90010014,
	0x7c0300d0,
	0x7c001b78,
	0x93e1000c,
	0x7c00fe70,
	0x540303de,
	0x93c10008,
	0x6463e400,
	0x7c841b78,
	0x38000001,
	0x90ad8230,
	0x3c608000,
	0x90ed8208,
	0x90866008
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


unsigned long second_disc_offset;

u32 patched_read = 0;
bool patched_readMIOS = false;
bool patched_seek = false;
bool patched_readID = false;
bool patched_readIDMIOS = false;
bool patched_readaudio = false;
bool patched_audioconfig = false;
bool patched_audiostatus = false;

// Only for debugging:
bool patched_reporterror = false;
bool patched_reset = false;
bool patched_stop_motor = false;
bool patched_stream_write = false;

/** Patch DVD read function. The patch should be applied only one time. */
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_debug_t *jmp, bool miosplugin, bool dopatch, bool highplugin, bool debug_plugin)
{
	void *addr_start = addr;
	void *addr_end = addr+len;

	while(addr_start<addr_end)
	{
		if (codecmp(addr_start,_Read_original,sizeof(_Read_original))==0)
		{
			debug_printf("Patching DVD read at 0x%x\n", (u32) addr_start);
			//printf("Patching DVD read at 0x%x\n", (u32) addr_start);

			if (dopatch)
			{
				install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

				if (backuploading)
				{
					// Change DVD command from 0xA8 to 0xD0:
					*(unsigned long*)(addr_start +0x5C) = 0x3C60D000;
					// Shift (>>11) the size and put in r3:
					*(unsigned long*)(addr_start +0x64) = 0x57A3AAFE;
					// Don't shift (>>2) the offset, because this is now LBA:
					*(unsigned long*)(addr_start +0x6C) = 0x93C4000C;
					// Store shifted size value:
					*(unsigned long*)(addr_start +0x74) = 0x90640010;
				}
			}
			
			patched_read = 1;
		}

		if (codecmp(addr_start,_Read_original2,sizeof(_Read_original2))==0)
		{
			debug_printf("Patching DVD read 2 at 0x%x\n", (u32) addr_start);
			//printf("Patching DVD read 2 at 0x%x\n", (u32) addr_start);

			if (dopatch)
			{
				install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

				if (backuploading)
				{
					// Change DVD command from 0xA8 to 0xD0:
					*(unsigned long*)(addr_start +0x44) = 0x3C00D000;
					// Don't shift (>>2) the offset, because this is now LBA:
					// Shift (>>11) the size and put in r4:
					*(unsigned long*)(addr_start +0x54) = 0x57E4AAFE;
					// Store shifted size value:
					*(unsigned long*)(addr_start +0x64) = 0x90836010;
				}
			}
			
			patched_read = 2;
		}

		if (codecmp(addr_start,_Read_original3,sizeof(_Read_original3))==0)
		{
			debug_printf("Patching DVD read 3 at 0x%x\n", (u32) addr_start);
			//printf("Patching DVD read 3 at 0x%x\n", (u32) addr_start);

			if (dopatch)
			{
				install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

				if (backuploading)
				{
					// Change DVD command from 0xA8 to 0xD0:
					*(unsigned long*)(addr_start +0x54) = 0x3C60D000;
					// Shift (>>11) the size and put in r3:
					*(unsigned long*)(addr_start +0x5c) = 0x57A3AAFE;
					// Don't shift (>>2) the offset, because this is now LBA:
					*(unsigned long*)(addr_start +0x64) = 0x93C4000C;
					// Store shifted size value:
					*(unsigned long*)(addr_start +0x6c) = 0x90640010;
				}
			}
			patched_read = 3;
		}

		if (codecmp(addr_start,_Read_originalMIOSv5,sizeof(_Read_originalMIOSv5))==0)
		{
			debug_printf("Patching DVD read MIOSv5 at 0x%x\n", (u32) addr_start);
			//printf("Patching DVD read MIOSv5 at 0x%x\n", (u32) addr_start);

			if (dopatch)
			{
				install_replacement((void*)addr_start, jmp->dvd_read_replacement, jmp->dvd_read_orig);

				if (backuploading)
				{
					// Shift (>>11) the size and put in r30:
					*(unsigned long*)(addr_start +0x28) = 0x549EAAFE;
					// Remove offset + 9
					*(unsigned long*)(addr_start +0x2c) = 0x60000000;
					// Change DVD command from 0xA8 to 0xD0:
					*(unsigned long*)(addr_start +0x34) = 0x3C00D000;
					// Store shifted size value:
					*(unsigned long*)(addr_start +0x48) = 0x93C76010;
				}
			}
			
			patched_readMIOS = true;
		}

		if (highplugin)
		{
			if (jmp->ar_dvd_read_replacement != 0)
			{
				if (codecmp(addr_start, ar_dvd_high_read, sizeof(ar_dvd_high_read))==0)
				{
					debug_printf("Patching DVD ar read at 0x%x\n", (u32) addr_start);
					//printf("Patching DVD ar read at 0x%x\n", (u32) addr_start);

					if (dopatch)
					{
						writeabsolutebranch((void*)addr_start, jmp->ar_dvd_read_replacement);

						/* Get address for dvd wait cmd. */
						jmp->ar_dvd_wait_for_last_cmd = (void *) get_bl_target((uint32_t *)(addr_start + 0x2c));
					}
				}
			}

			if (jmp->ar_dvd_read_disk_id_replacement != 0)
			{
				if (codecmp(addr_start, ar_dvd_high_read_disk_id, sizeof(ar_dvd_high_read_disk_id))==0)
				{
					debug_printf("Patching DVD ar read disk id at 0x%x\n", (u32) addr_start);
					//printf("Patching DVD ar read disk id at 0x%x\n", (u32) addr_start);

					if (dopatch)
					{
						writeabsolutebranch((void*)addr_start, jmp->ar_dvd_read_disk_id_replacement);

						/* Get address for dvd wait cmd. */
						jmp->ar_dvd_wait_for_last_cmd = (void *) get_bl_target((uint32_t *)(addr_start + 0x1c));
					}
				}
			}
		}

		//if (jmp->dvd_seek_replacement != 0)
		{
			if ((codecmp(addr_start,dvd_seek_original,sizeof(dvd_seek_original))==0)
				|| (codecmp(addr_start,dvd_seek_MIOS_original,sizeof(dvd_seek_MIOS_original))==0))
			{
				debug_printf("Patching DVD seek at 0x%x\n", (u32) addr_start);
	
				if (backuploading && dopatch)
				{
					writebranch((void*) addr_start, (void *) jmp->dvd_seek_replacement);
				}

				patched_seek = true;
			}
		}

		if ((codecmp(addr_start,_dvdlowreadid_org,sizeof(_dvdlowreadid_org))==0)
			|| (codecmp(addr_start,_dvdlowreadid_org2,sizeof(_dvdlowreadid_org2))==0)
			|| (codecmp(addr_start,_dvdlowreadid_org3,sizeof(_dvdlowreadid_org3))==0)
			|| (codecmp(addr_start,_dvdlowreadid_orgMIOSv5,sizeof(_dvdlowreadid_orgMIOSv5))==0))
		{
			debug_printf("Patching DVD Read ID at 0x%x\n", (u32) addr_start);

			if (backuploading && dopatch)
			{
				writebranch((void*) addr_start, (void *) jmp->dvd_read_id_replacement);
				//install_replacement((void*)addr_start, jmp->dvd_read_id_replacement, jmp->dvd_read_id_orig);
			}
			
			if (codecmp(addr_start,_dvdlowreadid_orgMIOSv5,sizeof(_dvdlowreadid_orgMIOSv5))==0)
			{
				patched_readIDMIOS = true;
			} else
			{
				patched_readID = true;
			}
		}

		//if (jmp->dvd_read_audio_replacement != 0)
		{
			if (codecmp(addr_start,dvd_read_audio_original,sizeof(dvd_read_audio_original))==0)
			{
				debug_printf("Patching DVD read audio at 0x%x\n", (u32) addr_start);
	
				if (dopatch)
				{
					if (debug_plugin)
					{
						// Debugging retail discs requires the original function
						install_replacement((void*)addr_start, (void *) jmp->dvd_read_audio_replacement, (void *) jmp->dvd_read_audio_orig);
					} else
					{
						writebranch((void*) addr_start, (void *) jmp->dvd_read_audio_replacement);
					}
				}

				patched_readaudio = true;
			}
		}

		//if (jmp->dvd_audio_config_replacement != 0)
		{
			if ((codecmp(addr_start,dvd_audio_config_MIOS_original,sizeof(dvd_audio_config_MIOS_original))==0)
				|| (codecmp(addr_start,dvd_audio_config_original,sizeof(dvd_audio_config_original))==0))
			{
				debug_printf("Patching DVD audio config at 0x%x\n", (u32) addr_start);
	
				if (dopatch)
				{
					writebranch((void*) addr_start, (void *) jmp->dvd_audio_config_replacement);
				}
				
				patched_audioconfig = true;
			}
		}

		//if (jmp->dvd_audio_status_replacement != 0)
		{
			if (codecmp(addr_start,dvd_audio_status_original,sizeof(dvd_audio_status_original))==0)
			{
				debug_printf("Patching DVD audio status at 0x%x\n", (u32) addr_start);
	
				if (dopatch)
				{
					if (debug_plugin)
					{
						// Debugging retail discs requires the original function
						install_replacement((void*)addr_start, (void *) jmp->dvd_audio_status_replacement, (void *) jmp->dvd_audio_status_orig);
					} else
					{
						writebranch((void*) addr_start, (void *) jmp->dvd_audio_status_replacement);
					}
				}
				
				patched_audiostatus = true;
			}
		}

		if (jmp->memset_replacement != 0)
		{
			if (codecmp(addr_start, memset_original, sizeof(memset_original))==0)
			{
				debug_printf("Patching memset at 0x%x\n", (u32) addr_start);
	
				if (dopatch)
				{
					install_replacement((void*)addr_start, (void *) jmp->memset_replacement, jmp->memset_orig);
				}
			}
		}

		if (codecmp(addr_start,dvd_report_error_original,sizeof(dvd_report_error_original))==0)
		{
			if (debug_plugin && dopatch && jmp->dvd_report_error_replacement != 0)
			{
				debug_printf("Patching DVD report error at 0x%x\n", (u32) addr_start);
				install_replacement((void*)addr_start, (void *)jmp->dvd_report_error_replacement, (void *)jmp->dvd_report_error_orig);
			}
			patched_reporterror = true;
		}

		if (codecmp(addr_start,_dvdlowreset_org,sizeof(_dvdlowreset_org))==0)
		{
			if (debug_plugin && dopatch && jmp->dvd_reset_replacement != 0)
			{
				debug_printf("Patching DVD Reset at 0x%x\n", (u32) addr_start);
				install_replacement((void*)addr_start, (void *)jmp->dvd_reset_replacement, (void *)jmp->dvd_reset_orig);
			}
			patched_reset = true;				
		}

		if (codecmp(addr_start,dvd_stop_motor_original,sizeof(dvd_stop_motor_original))==0)
		{
			if (debug_plugin && dopatch && jmp->dvd_stop_motor_replacement != 0)
			{
				debug_printf("Patching DVD motor stop at 0x%x\n", (u32) addr_start);
				install_replacement((void*)addr_start, (void *)jmp->dvd_stop_motor_replacement, (void *)jmp->dvd_stop_motor_orig);
			}
			patched_stop_motor = true;			
		}

		/* Patch debug printf. */
		if ((	codecmp(addr_start, __fwriteGC_orignal, sizeof(__fwriteGC_orignal))==0)
			|| (codecmp(addr_start, __fwriteGC_orignal2, sizeof(__fwriteGC_orignal2))==0))
		{
			if (debug_plugin && dopatch && jmp->stream_write_replacement != 0)
			{
				debug_printf("Patching debug write at 0x%x\n", (u32) addr_start);
				install_replacement((void*)addr_start, (void *)jmp->stream_write_replacement, (void *)jmp->stream_write_orig);
			}
			patched_stream_write = true;
		}

#ifdef GEKKO_DEBUG
		/* The following patches are only working with special games. To get the value for other
		 * games, take the caller address of dvd_read_replacement() from serial debug output and subtract 4.
		 */

/*		if (((gcplugin_jumptable_debug_t *)jmp)->dvd_callback_entry != 0)
		{
			// Super Mario Sunshine SMS USA.
			if ((gameid != NULL) && (strncmp(gameid, "GMSE", 4) == 0)) {
				if ((addr_start == (void *) 0x8034AA18) && ((*(unsigned long*) addr_start) == 0x4E800021)) {
					debug_printf("Patching DVD callback at 0x%x\n", (u32) addr_start);
					// Jump from game code to new code.
					writebranch_with_link((void*)addr_start, ((gcplugin_jumptable_debug_t *)jmp)->dvd_callback_entry);
				}
			}
	
			// Starfox USA.
			if ((gameid != NULL) && strncmp(gameid, "GF7E", 4) == 0) {
				if ((addr_start == (void *) 0x8013b750) && ((*(unsigned long*) addr_start) == 0x4E800021)) {
					debug_printf("Patching DVD callback at 0x%x\n", (u32) addr_start);
	
					// Jump from game code to new code.
					writebranch_with_link((void*)addr_start, ((gcplugin_jumptable_debug_t *)jmp)->dvd_callback_entry);
				}
			}

			// Mario Kart Double Dash.
			if ((gameid != NULL) && strncmp(gameid, "GM4E01", 6) == 0) {
				if ((addr_start == (void *) 0x800aa9a8) && ((*(unsigned long*) addr_start) == 0x4E800021)) {
					debug_printf("Patching DVD callback at 0x%x\n", (u32) addr_start);
	
					// Jump from game code to new code.
					writebranch_with_link((void*)addr_start, ((gcplugin_jumptable_debug_t *)jmp)->dvd_callback_entry);
				}
			}
			// Baten Kaitos Origins NTSC-U
			if ((gameid != NULL) && strncmp(gameid, "GK4E01", 6) == 0)
			{
				//if ((addr_start == (void *) ) && ((*(unsigned long*) addr_start) == 0x4E800021)) {
					debug_printf("Patching DVD callback at 0x%x\n", (u32) addr_start);
	
					// Jump from game code to new code.
					writebranch_with_link((void*)addr_start, ((gcplugin_jumptable_debug_t *)jmp)->dvd_callback_entry);
				}
			}
		}
*/
#endif

		if (miosplugin)
		{
			if (debug_plugin && dopatch && jmp->printf_replacement != 0)
			{
				if ( ((addr_start == (void *) 0x81301cec) || (addr_start == (void *) 0x81301da0)) && (*((uint32_t *) addr_start) == 0x9421FF90) )
				{
					debug_printf("Patching debug printf at 0x%x (MIOS v5 only)\n", (u32) addr_start);

					writebranch((void*)addr_start, (void *)jmp->printf_replacement);
				}
			}

			/* Disable detection of Action Replay in MIOS. */
			if (memcmp(addr_start, "GNHE", 4) == 0)  {
				debug_printf("Patching AR support at 0x%x.\n", addr_start);
				((volatile char *) addr_start)[0] = 0;
				DCFlushRange(addr_start, 32);
				ICInvalidateRange(addr_start, 32);
			}
		} else
		{
#if 0 /* XXX: only for AR debug. */
			/*
			if (addr_start == (void *) 0x8002788c) { // vsprintf
				if (*((uint32_t *) addr_start) == 0x9421ffe0) {
					if (jmp->ar_stream_write_replacement != 0) {
						debug_printf("patching debug write at 0x%x (ar only)\n", (u32) addr_start);
	
						install_replacement((void*)addr_start, jmp->ar_stream_write_replacement, jmp->ar_stream_write_orig);
					}
				}
			}
			*/
#endif
		}
		addr_start += 4;
	}
}
