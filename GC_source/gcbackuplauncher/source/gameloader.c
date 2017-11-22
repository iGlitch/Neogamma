/* (C) Copyright 2009 WiiGator */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <gctypes.h>
#include <ogc/cache.h>
#include <ogcsys.h>
#include <gccore.h>
#include <string.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/cache.h>

#include "dvd.h"
#include "gameloader.h"
#include "debugprintf.h"
#include "loadapp.h"
#include "screen.h"
#include "dvdreadpatch.h"
#include "systemconfig.h"
#include "video.h"
#include "Toy.h"
#include "patchcode.h"

#include "GCcodehandleronly_bin.h"
#include "GCcodehandlerDebug_bin.h"

#include "sram.h"

void Con_ClearLine(void)
{
	s32 cols, rows;
	u32 cnt;

	printf("\r");
	fflush(stdout);

	CON_GetMetrics(&cols, &rows);

	for (cnt = 1; cnt < cols; cnt++) {
		printf(" ");
		fflush(stdout);
	}

	printf("\r");
	fflush(stdout);
}

u32 streaming = 0;
u32 hookselect = 0;
u32 hook2select = 0;
u32 ocarinaselect = 0;
u32 debuggerselect = 0;

u8 reloader = 0;
u32 videopatchmode = 0;

/*
static inline u32 read32(u32 addr)
{
	u32 x;
	asm volatile("lwz %0,0(%1) ; sync" : "=r"(x) : "b"(0xc0000000 | addr));
	return x;
}

static inline void write32(u32 addr, u32 x)
{
	asm("stw %0,0(%1) ; eieio" : : "r"(x), "b"(0xc0000000 | addr));
}

static bool have_ahbprot() {
    return read32(0xcd800064) == 0xffffffff;
}
*/

//---------------------------------------------------------------------------------
void load_handler()
//---------------------------------------------------------------------------------
{
	if (debuggerselect != 0)
	{
		memcpy((void*)0x80001800,GCcodehandlerDebug_bin,GCcodehandlerDebug_bin_size);
		
		// Paused start
		if (debuggerselect == 2) *(u32*)0x80002774 = 1;
		
		memcpy((void*)0x80001CDE, &codelist, 2);
		memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
		memcpy((void*)0x80001F5A, &codelist, 2);
		memcpy((void*)0x80001F5E, ((u8*) &codelist) + 2, 2);	
	} else
	{
		memcpy((void*)0x80001800,GCcodehandleronly_bin,GCcodehandleronly_bin_size);
		memcpy((void*)0x80001906, &codelist, 2);
		memcpy((void*)0x8000190A, ((u8*) &codelist) + 2, 2);
	}
	
	memcpy((void *)0x80001800, (void*)0x80000000, 6);
	
	// The codes are copied later
	
	DCFlushRange((void*)0x80001800, 0x1800);
	ICInvalidateRange((void*)0x80001800, 0x1800);
}

char languages[6][22] = 
{{"English"},
{"German"},
{"French"},
{"Spanish"},
{"Italian"},
{"Dutch"}
};

void start_from_NeoGamma(void)
{
	memset((void*)0x80001800, 0, 0x1800);
	DCFlushRange((void*)0x80001800, 0x1800);
	ICInvalidateRange((void*)0x80001800, 0x1800);

	use_high_plugin = false;
	use_debug_plugin = false;
	use_patched_MIOS = false;
	dopatch__GXSetVAT = false;
	audio_status_request_fix = false;
	audio_32K_fix = false;

	base_offset = 0;
	int rv;
	u32 errornumber;
	u32 pluginselect;
	u32 audiofixselect;
	u32 patchedMIOSselect;
	
	base_offset = *(u32 *)0x807FFF20;
	second_disc_offset = *(u32 *)0x807FFF24;
	videopatchmode = *(u8 *)0x807FFF30;
	hookselect = *(u8 *)0x807FFF31;
	hook2select = *(u8 *)0x807FFF32;
	ocarinaselect = *(u8 *)0x807FFF33;
	debuggerselect = *(u8 *)0x807FFF34;
	reloader = *(u8 *)0x807FFF35;

	pluginselect = *(u8 *)0x807FFF36;
	audiofixselect = *(u8 *)0x807FFF37;
	patchedMIOSselect = *(u8 *)0x807FFF38;

	codelist = (u8 *)(*(u32 *)0x807FFF40);
	codelistend = (u8 *)(*(u32 *)0x807FFF44);
	
	if (pluginselect > 1)
	{
		use_high_plugin = true;
	}
	if (pluginselect == 3)
	{
		use_debug_plugin = true;
	}
	
	if (patchedMIOSselect == 2)
	{
		use_patched_MIOS = true;
	}
	
	if (audiofixselect == 2 || audiofixselect == 4)
	{
		audio_status_request_fix = true;
	}
	if (audiofixselect == 3 || audiofixselect == 4)
	{
		audio_32K_fix = true;
	}
	
/*
	u32 value;
	bool have;

	printf("Before have_ahbprot...\n");
	debug_printf("Before have_ahbprot...\n");

	sleep(5);

	have = have_ahbprot();
	
	printf("Reading at have_ahbprot returned: %u\n", have);
	debug_printf("Reading at have_ahbprot returned: %u\n", have);
	
	sleep(5);

	write32(0xd8b420a, read32(0xd8b420a) & 0x0000FFFF);
	
	printf("Disabled memory protection...\n");
	debug_printf("Disabled memory protection...\n");
	
	sleep(5);	

	printf("Before mem2 access...\n");
	debug_printf("Before mem2 access...\n");

	sleep(5);

	value = read32(0x91000000);
	
	printf("Reading at 0x91000000 returned: 0x%08x\n", value);
	debug_printf("Reading at 0x91000000 returned: 0x%08x\n", value);
	
	sleep(5);
	
	write32(0x91000000, 0x12345678);
	
	printf("Wrote to mem2...\n");
	debug_printf("Wrote to mem2...\n");

	sleep(5);

	value = read32(0x91000000);
	
	printf("Reading at 0x91000000 returned: 0x%08x\n", value);
	debug_printf("Reading at 0x91000000 returned: 0x%08x\n", value);
	
	sleep(20);	
*/		
	
	get_cover_status();		// It seems the drive notices if this was checked

	// check if the disc is a backup or retail
	dvd_read_id();

	if (backuploading)
	{
#ifdef DONTTELL
		if ( (hookselect != 0 || hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0) )
		{
			use_high_plugin = true;
#else
		if ( (hookselect != 0) && (ocarinaselect != 0 || debuggerselect != 0) )
		{
			printf("Ocarina/Wiird is not possible on backup discs\n");
			hookselect = 0;
			hook2select = 0;
			ocarinaselect = 0;
			debuggerselect = 0;			
#endif
		}
		
		// Pokemon Box: Ruby and Sapphire
		if (memcmp((void *) GAMECODE_ADDRESS, "GPX", 3) == 0 && memcmp((void *)((u32)GAMECODE_ADDRESS+4),"01" , 2) == 0)
		{
			if (use_high_plugin == false)
			{
				if (pluginselect == 0)
				{
					use_high_plugin = true;
				} else
				{
					printf("This game requires to use the high plugin...\n");
				}
			}
		}	
		
		// Action Replay
		if (memcmp((void *) GAMECODE_ADDRESS, "GNHE", 4) == 0)
		{
			// Action Replay puts its own code at 0x80001800
			if (pluginselect == 0)
			{
				use_high_plugin = true;
			}
			
			if (reloader == 0)
			{
				printf("Action Replay won't work without reloader, overriding setting...\n");
				reloader = 1;
			}
		}
		
/*		The changed patch__GXSetVAT function might work for the japanese version of Wind Waker as well
		// Zelda Wind Waker NTSC-J
		if (memcmp((void *)GAMECODE_ADDRESS, "GZL", 3) == 0 && memcmp((void *)(GAMECODE_ADDRESS+3), "J", 1) == 0)
		{
			if (pluginselect == 1 || patchedMIOSselect == 1)
			{
				printf("This game requires to use the high plugin + patched MIOS...\n");
			}
			if (patchedMIOSselect == 0)
			{
				use_patched_MIOS = true;
				if (pluginselect == 0)
				{
					use_high_plugin = true;
				}
			}			
		}
*/		
		// Ikaruga needs a different Audio Status request handling
		if (memcmp((void *) GAMECODE_ADDRESS, "GIK", 3) == 0 && memcmp((void *)((u32)GAMECODE_ADDRESS+4),"70" , 2) == 0)
		{
			if (audio_status_request_fix == false)
			{
				if (audiofixselect == 0)
				{
					audio_status_request_fix = true;
					audio_32K_fix = true;
				} else
				{
					printf("This game requires to use the Audio Status Request fix...\n");
				}
			}
		}		
	} else
	{
		if (use_patched_MIOS && !use_debug_plugin)
		{
			printf("Using patched MIOS is not possible on retail discs without debugging\n");
			use_patched_MIOS = false;
		}
		if (!use_debug_plugin)
		{
			use_high_plugin = false;
		}

		audio_32K_fix = false;
	}
	
	if (reloader == 1)
	{
		// 007: Agent Under Fire
		if (memcmp((void *) GAMECODE_ADDRESS, "GW7", 3) == 0 && memcmp((void *)((u32)GAMECODE_ADDRESS+4),"69" , 2) == 0)
		{
			reloader = 4;
		} else
		{
			reloader = 2; // Fast reloader as default
		}
	}
	
		
	if (use_patched_MIOS)
	{
#ifdef DONTTELL
		if ( (hookselect != 0 || hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0) )
#else
		if ( (hookselect != 0) && (ocarinaselect != 0 || debuggerselect != 0) )
#endif
		{
			printf("Ocarina/Wiird is not possible when using patched MIOS\n");
			hookselect = 0;
			hook2select = 0;
			ocarinaselect = 0;
			debuggerselect = 0;			
		}

		if (videopatchmode != 0)
		{
			printf("Video mode patches not possible when using patched MIOS\n");
		}
	}
		
	if (!use_patched_MIOS)
	{
		wii_dvd_reset_unlock();
	}

	if (backuploading)
	{
		printf("Loading backup disc...\n");
	} else
	{
		printf("Loading retail disc...\n");
	}

	if (!use_patched_MIOS)
	{
		dvd_get_error();
		streaming = dvd_set_streaming();
		errornumber = dvd_get_error();
			
		if (errornumber != 0 && streaming != 0)
		{
			dvd_RequestAudioStatus(0);	// If this works, it means the audio streaming has been set in wii mode already
			errornumber = dvd_get_error();
		
			// Do a reset for retail discs, this should allow to set the audio streaming if the wii code didn't/couldn't do it
			if (errornumber != 0 && !use_patched_MIOS && !backuploading)
			{
				printf("Resetting the drive...");
				
				dvd_motor_off();
				//ack_cover_interrupt();
				dvd_reset();

				// Wait until reset is finished.
				unsigned int cvstatus;
				do {
					cvstatus = get_cover_status();
				} while ((cvstatus & 0x01) != 0);
				
				dvd_read_id();

				Con_ClearLine();
				
				streaming = dvd_set_streaming();
				errornumber = dvd_get_error();				
			}
		}

		if (errornumber != 0 && streaming != 0)	// Ignore error when using a cIOS that doesn't allow audio config + retail disc without audio streaming
		{
			//The audio streaming always causes an error on backups
			if (!backuploading)
			{
				printf("DVD Error: 0x%x\n", errornumber);
				sleep(5);
			}
		}
	}
	
	// Read the name of the game
	static dvdinfo_t gcm_disk __attribute((aligned(32)));
	rv = dvd_read(&gcm_disk, sizeof(gcm_disk), 0);
	DVD_CHECK();

	char discid[7];
	memset(discid, 0, 7);
	memcpy(discid, (void *) GAMECODE_ADDRESS, 6);
	
	printf("DiscID: %s, Name: %s\n", discid, gcm_disk.GameName);
	debug_printf("DiscID: %s, Name: %s\n", discid, gcm_disk.GameName);

	// Patch for Zelda Wind Waker PAL+NTSC-U freeze on minimap, and try to patch the same function for some other games as well
	if(memcmp((void *) GAMECODE_ADDRESS, "GZL", 3) == 0	// Wind Waker
	|| memcmp((void *) GAMECODE_ADDRESS, "GSR", 3) == 0	// Smuggler's Run: Warzones
	|| memcmp((void *) GAMECODE_ADDRESS, "PZL", 3) == 0	// Zelda: Collector's Edition
	|| memcmp((void *) GAMECODE_ADDRESS, "GT3P", 4) == 0	// Tony Hawk's Pro Skater 3
	|| memcmp((void *) GAMECODE_ADDRESS, "GT3F", 4) == 0	// Tony Hawk's Pro Skater 3
	|| memcmp((void *) GAMECODE_ADDRESS, "GT3D", 4) == 0	// Tony Hawk's Pro Skater 3
	|| memcmp((void *) GAMECODE_ADDRESS, "GC6J", 4) == 0	// Pokémon Colosseum
	|| memcmp((void *) GAMECODE_ADDRESS, "GC6E", 4) == 0	// Pokémon Colosseum
	|| memcmp((void *) GAMECODE_ADDRESS, "GC6P", 4) == 0	// Pokémon Colosseum
	//|| memcmp((void *) GAMECODE_ADDRESS, "GPOJ", 4) == 0	// Phantasy Star Online Episode I & II
	|| memcmp((void *) GAMECODE_ADDRESS, "GPO", 3) == 0)	// Phantasy Star Online Episode I & II
	{
		if (!use_patched_MIOS || !use_high_plugin)
		{
			// Show a warning for thoses games except Wind Waker
			if (memcmp((void *) GAMECODE_ADDRESS, "GZL", 3) != 0)
			{
				printf("MIOS patches this game somehow, trying to patch __GXSetVAT\n");
				printf("If it does not work, try again with patched MIOS + high plugin\n");
			}

			// Zelda Wind Waker NTSC-J
			if (memcmp((void *)GAMECODE_ADDRESS, "GZL", 3) == 0 && memcmp((void *)(GAMECODE_ADDRESS+3), "J", 1) == 0)
			{
				printf("Sorry this game might freeze when opening the mini map\n");
				printf("If it happens, then use patched MIOS + high plugin\n");
			}
		}
	
		dopatch__GXSetVAT = true;
	} else
	{
		dopatch__GXSetVAT = false;
	}

	u8 SRAMlanguage = getSRAMlanguage();
	u8 SRAMflags = getSRAMflags();
/*	
	SRAMflags =  SRAMflags |  (1 << 6);
//	SRAMflags =  SRAMflags & ~(1 << 0);
//	SRAMflags =  SRAMflags & ~(1 << 1);
//	SRAMflags =  SRAMflags & ~(1 << 6);
	
	setSRAMflags(SRAMflags);
*/	
	// PAL60 and prog could be switched
	
	if (SRAMlanguage > 5)
	{
		printf("System language: unknown(%u), video region: %s, Prog: %s, PAL60: %s\n", SRAMlanguage, ((SRAMflags & (1 << 6)) == 0)?"NTSC":"PAL", ((SRAMflags & (1 << 0)) == 0)?"No":"Yes", ((SRAMflags & (1 << 1)) == 0)?"No":"Yes" );
	} else
	{
		printf("System language: %s, video region: %s, Prog: %s, PAL60: %s\n", languages[SRAMlanguage], ((SRAMflags & (1 << 6)) == 0)?"NTSC":"PAL", ((SRAMflags & (1 << 0)) == 0)?"No":"Yes", ((SRAMflags & (1 << 1)) == 0)?"No":"Yes");
	}

#ifdef DONTTELL
	if ( (hookselect != 0 || hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0))
#else
	if (!backuploading && hookselect != 0 && (ocarinaselect != 0 || debuggerselect != 0))
#endif
	{
		load_handler();
		if (ocarinaselect != 0)
		{
			// Copy codes now before the memory is cleared
			memcpy(codelist, (void *)(0x807FFF00-((u32)codelistend - (u32)codelist)), (u32)codelistend - (u32)codelist);
		} else
		{
			// Clear codes area for debugger without codes
			memset(codelist, 0, (u32)codelistend - (u32)codelist);
		}

		DCFlushRange(codelist, (u32)codelistend - (u32)codelist);
		ICInvalidateRange(codelist, (u32)codelistend - (u32)codelist);
	}

	u32 mode;
	
	switch (*(u32 *)0x807FFF28)	// video mode
	{
		case 0:	// Video Mode Wii
			if (*(u32 *)0x807FFF2C == 0)	// region: NTSC
			{
				mode = VIDEO_MODE_NTSC;
			} else							// region: PAL
			{
				if (gcm_disk.CountryCode == 'E' || gcm_disk.CountryCode == 'J')
				{
					mode = VIDEO_MODE_PAL60;
				} else
				{
					mode = VIDEO_MODE_PAL;		// PAL games can't be booted with PAL60...
				}
			}
		break;
		
		case 2:	// NTSC480i
			mode = VIDEO_MODE_NTSC;
		break;
		
		case 3:	// NTSC480p
			mode = VIDEO_MODE_NTSC480P;
		break;
		
		case 4:	// PAL(auto)
			if (gcm_disk.CountryCode == 'E' || gcm_disk.CountryCode == 'J')
			{
				mode = VIDEO_MODE_PAL60;
			} else
			{
				mode = VIDEO_MODE_PAL;		// PAL games can't be booted with PAL60...
			}
		break;	
		
		case 5:	// PAL576i
			mode = VIDEO_MODE_PAL;
		break;
		
		case 6:	// PAL480i
			mode = VIDEO_MODE_PAL60;
		break;
		
		case 7:	// PAL480p
			mode = VIDEO_MODE_PAL480P;
		break;

		case 1:	// video Mode Disc
		default:
			mode = gcm_disk.CountryCode;
		break;
	}
	
	setup_video(mode);	// Also clears memory
	load_app();
}
