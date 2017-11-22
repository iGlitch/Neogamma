/*
 *
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
#include <stdio.h>
#include <unistd.h>
#include <ogcsys.h>
#include <gccore.h>
#include <ogc/cache.h>
#include <string.h>
#include <stdint.h>
#include <ogc/lwp_watchdog.h>
#include <time.h>

#include "dvd.h"
#include "dvdreadpatch.h"
#include "highplugin_bin.h"
#include "lowplugin_bin.h"
#include "debugplugin_bin.h"
#include "gcplugin.h"
#include "debugprintf.h"
#include "loadapp.h"
#include "systemconfig.h"
#include "patchcode.h"
#include "asm.h"
#include "processor.h"
#include "gameloader.h"
#include "Toy.h"
#include "sram.h"

static u8 plugin_buffer[HIGH_PLUGIN_SIZE] __attribute__((aligned(32)));

extern void __exception_closeall();

void nothing()
{
}

bool hookpatched;
u32 GXSetVAT_count;

u32 plugin_size = 0;
const u8 *plugin = NULL;
void *plugin_base = 0;

u8 waittime = 0;

typedef struct {
    u8 filetype;
    char name_offset[3];
    u32 fileoffset;
    u32 filelen;
} __attribute__((packed)) FST_ENTRY;


char *fstfilename(u32 index)
{
	FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
	u32 count = fst[0].filelen;
	u32 stringoffset;
	if (index < count)
	{
		stringoffset = *(u32 *)&(fst[index]) & 0x00ffffff;
		return (char *)(*(u32 *)0x80000038 + count*12 + stringoffset);
	} else
	{
		return NULL;
	}
}

void getaudiostreamnumbers(u32 *streams, u32 *badstreams)
{
	FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
	
	*streams = 0;
	*badstreams = 0;
	
	u32 i;
	for (i=1;i< fst[0].filelen;i++)
	{
		if (fst[i].filetype == 0 && strstr(fstfilename(i), ".adp") != NULL)
		{
			(*streams)++;
			if (fst[i].fileoffset % 0x8000 != 0)		// not 32KB aligned
			{
				(*badstreams)++;
				if (audio_32K_fix)
				{
					fst[i].fileoffset = fst[i].fileoffset & 0xffff8000;
				}
			}			
		}		
	}
}


/** Start PowerPC MIOS part and patch some code before. */
void call_mios()
{
	unsigned long *entry = MIOS_PATCH_ADDR;

	if (use_high_plugin)
	{
		memset((void *) 0x81340000, 0, 0x4c0000-HIGH_PLUGIN_SIZE);
		DCFlushRange((void *) 0x81340000, 0x4c0000-HIGH_PLUGIN_SIZE);
		ICInvalidateRange((void *) 0x81340000, 0x4c0000-HIGH_PLUGIN_SIZE);
	} else
	{
		memset((void *) 0x81340000, 0, 0x4c0000);
		DCFlushRange((void *) 0x81340000, 0x4c0000);
		ICInvalidateRange((void *) 0x81340000, 0x4c0000);
	}

	if (*entry != GC_HOMEBREW_LAUNCHER_ENTRY)
	{
		debug_printf("MIOS entrypoint 0x%08x is not 0x%08x as it should be\n", (u32)(*entry), (u32)GC_HOMEBREW_LAUNCHER_ENTRY);
		printf("MIOS entrypoint 0x%08x is not 0x%08x as it should be\n", (u32)(*entry), (u32)GC_HOMEBREW_LAUNCHER_ENTRY);
		printf("Please reboot\n");
		while (true);
	}

	/* Restore patched entry point. */
	*entry = MIOS_ENTRY;
	DCFlushRange((void *) 0x80003700, 0x100);
	ICInvalidateRange((void *) 0x80003700, 0x100);

	/* Simulate boot. */
	__asm__(
		"bl DCDisable\n"
		"bl ICDisable\n"
		"li %r3, 0\n"
		"mtsrr1 %r3\n"
		"li %r4, 0x3400\n"
		"mtsrr0 %r4\n"
		"rfi\n"
	);
}

void setup_MIOS_video_mode(int mode)
{
	int miosmode;

	/* Select MIOS video mode by disc mode. */
	switch (mode) {
	case 'E':	// NTSC-U
	case 'J':	// NTSC-J
	case 'N':	// Japanese Import to USA
		/* NTSC */
		miosmode = 0;
		break;

	case 'P':	// PAL
	case 'D':	// German
	case 'F':	// French
	case 'S':	// Spanish
	case 'I':	// Italian
	case 'L':	// Japanese Import to PAL
	case 'M':	// American Import to PAL
	case 'X':	// PAL other languages?
	case 'Y':	// PAL other languages?
		/* PAL */
		miosmode = 1;
		break;
	default: {
			/* Unknown mode. Don't change. */
			int tv;

			tv = VIDEO_GetCurrentTvMode();
			switch(tv) {
				case VI_NTSC:
					/* NTSC */
					miosmode = 0;
					break;

				case VI_MPAL:
				case VI_PAL:
				case VI_EURGB60:
					/* PAL */
					miosmode = 1;
					break;
				default:
					/* NTSC */
					miosmode = 0;
					break;
			}
			break;
		}
	}

	//debug_printf("Setting MIOS video mode to %s...", (miosmode == 1) ? "PAL" : "NTSC");

	// This changes the prog setting and PAL60 setting
	//setSRAMvideomode(miosmode);

	//debug_printf("done\n");
}


// Patch for Wind Waker freeze on minimap
int patch__GXSetVAT(u8 *buf, u32 size) 
{
	u32 i;
	u32 match_count = 0;
	
	const u32 old_table[31] = {
	/*0x8142ce00,*/ 0x39800000, 0x39600000, 0x3ce0cc01,
	0x48000070, 0x5589063e, 0x886a04f2, 0x38000001,
	0x7c004830, 0x7c600039, 0x41820050, 0x39000008,
	0x99078000, 0x61230070, 0x380b001c, 0x98678000,
	0x61250080, 0x388b003c, 0x7cca002e, 0x61230090,
	0x380b005c, 0x90c78000, 0x99078000, 0x98a78000,
	0x7c8a202e, 0x90878000, 0x99078000, 0x98678000,
	0x7c0a002e, 0x90078000, 0x396b0004, 0x398c0001
	};
	
	const u32 new_table[31] = {
	/*0x8122ce00,*/ 0x39400000, 0x896904f2, 0x7d284b78,
	0x556007ff, 0x41820050, 0x38e00008, 0x3cc0cc01,
	0x98e68000, 0x61400070, 0x61440080, 0x61430090,
	0x98068000, 0x38000000, 0x80a8001c, 0x90a68000,
	0x98e68000, 0x98868000, 0x8088003c, 0x90868000,
	0x98e68000, 0x98668000, 0x8068005c, 0x90668000,
	0x98068000, 0x556bf87f, 0x394a0001, 0x39080004,
	0x4082ffa0, 0x38000000, 0x980904f2, 0x4e800020
	};
	
	if (size > sizeof(old_table))
	{
		for (i=4; i<size-sizeof(old_table); i+=4) 
		{
			if (!memcmp(buf + i, old_table, sizeof(old_table))) 
			{
				if ( buf[i - 4] == 0x81 && buf[i - 3] == 0x42 )
				{			
					buf[i - 3] = 0x22;
					memcpy(buf + i, new_table, sizeof(new_table));
					i += sizeof(new_table)+4;
					match_count++;
				}
			}
		}
	}
	return match_count;
}


void plugin_select()
{
	// Select which plugin to use + memory setup
	if (use_high_plugin)	
	{
		if (use_debug_plugin)
		{
			plugin = debugplugin_bin;
			plugin_size = debugplugin_bin_size;
			printf("Using debug plugin...\n");
			debug_printf("Using debug plugin...\n");
		} else
		{
			plugin = highplugin_bin;
			plugin_size = highplugin_bin_size;
			printf("Using high plugin...\n");
			debug_printf("Using high plugin...\n");
		}
		plugin_base = HIGH_PLUGIN_BASE;
	} else
	{
		plugin_size = lowplugin_bin_size;
		plugin = lowplugin_bin;
		plugin_base = LOW_PLUGIN_BASE;
		//printf("Using low plugin...\n");
		debug_printf("Using low plugin...\n");
	}
}

void plugin_init()
{
	if (backuploading || use_debug_plugin)
	{
		/* Install DVD replacement functions. */
		memcpy(plugin_buffer, plugin, plugin_size);
		//debug_printf("Plugin copied...\n");

		u32 var_offset;
		if (use_high_plugin)
		{
			if (use_debug_plugin)
			{
				var_offset = ((u32) plugin_buffer) + 2*sizeof(gcplugin_jumptable_debug_t);
			} else
			{
				var_offset = ((u32) plugin_buffer) + 2*sizeof(gcplugin_jumptable_high_t);
			}
#ifdef DONTTELL
			var_offset += 64;		// The hook is located after the jumptable before the other vars
#endif
		} else
		{
			var_offset = ((u32) plugin_buffer) + 2*sizeof(gcplugin_jumptable_low_t);
		}
		*(u32 *) (var_offset) = base_offset;
		*(u32 *) (var_offset + sizeof(u32)) = second_disc_offset;
		if (reloader > 0)
		{
			if 	(reloader == 4)				
			{
				*(u8 *) (var_offset + 2*sizeof(u32)) = 3;	// Plugin variable reloader_enabled
				if (backuploading)
				{
					printf("Reloader set to fix moved .elfs...\n");
				}
			} else
			if 	(reloader == 3)				
			{
				*(u8 *) (var_offset + 2*sizeof(u32)) = 2;	// Plugin variable reloader_enabled
				if (backuploading)
				{
					printf("Reloader set to force...\n");
				}
			} else
			{
				*(u8 *) (var_offset + 2*sizeof(u32)) = 1;	// Plugin variable reloader_enabled
				if (backuploading)
				{
					printf("Reloader activated...\n");
				}
			}
		} else
		{
			*(u8 *) (var_offset + 2*sizeof(u32)) = 0;	// Plugin variable reloader_enabled
		}

		if (audio_status_request_fix) 
		{
			*(u8 *) (var_offset + 2*sizeof(u32) + 1) = 1;	// Plugin variable audio_stream_fix
			//printf("Audio Status Request fix activated...\n");
		} else
		{
			*(u8 *) (var_offset + 2*sizeof(u32) + 1) = 0;	// Plugin variable audio_stream_fix
		}
		
		if (use_patched_MIOS)
		{
			*(u8 *) (var_offset + 2*sizeof(u32) + 2) = 1;	// Plugin variable mios_mode
		} else
		{
			*(u8 *) (var_offset + 2*sizeof(u32) + 2) = 0;	// Plugin variable mios_mode
		}		

		if (use_debug_plugin)
		{
			if (backuploading)
			{
				*(u8 *) (var_offset + 2*sizeof(u32) + 3) = 1;	// Plugin variable backuplaunching
			} else
			{
				*(u8 *) (var_offset + 2*sizeof(u32) + 3) = 0;	// Plugin variable backuplaunching
			}
		}

#ifdef DONTTELL
		if (use_high_plugin)
		{
			if (use_debug_plugin)
			{
				if ((hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0))
				{
					*(u8 *) (var_offset + 2*sizeof(u32) + 4) = 1;	// Plugin variable hooks
				} else
				{
					*(u8 *) (var_offset + 2*sizeof(u32) + 4) = 0;	// Plugin variable hooks
				}
			} else
			{
				if ((hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0))
				{
					*(u8 *) (var_offset + 2*sizeof(u32) + 3) = 1;	// Plugin variable hooks
				} else
				{
					*(u8 *) (var_offset + 2*sizeof(u32) + 3) = 0;	// Plugin variable hooks
				}
			}
			copy_2nd_hook((void *)(var_offset - 64));
		}
#endif
		debug_printf("Plugin variables set...\n");

		DCFlushRange(plugin_buffer, plugin_size);
		ICInvalidateRange(plugin_buffer, plugin_size);
	}
}

void copy_plugin()
{
	if (backuploading || use_debug_plugin)
	{
		memcpy(plugin_base, plugin_buffer, plugin_size);
		debug_printf("Plugin copied...\n");
		DCFlushRange(plugin_base, plugin_size);
		ICInvalidateRange(plugin_base, plugin_size);
	}
}


void show_patch_warnings()
{
	char text[32];
	if (backuploading)
	{
		snprintf(text, 32, "WARNING: Could");
	} else
	{
		snprintf(text, 32, "WARNING: Would");
	}

	if (patched_read > 1)
	{
		printf("The game uses dvd read function %u\n", patched_read);
	}

	// No warnings for Action Replay
	if (memcmp((void *) GAMECODE_ADDRESS, "GNHE", 4) != 0 && (patched_read == 0 || !patched_seek || !patched_readID 
	|| !patched_readaudio || !patched_audioconfig || !patched_audiostatus
	|| (use_debug_plugin &&
	(!patched_reporterror || !patched_reset || !patched_stop_motor || !patched_stream_write))
	))
	{
		waittime+=15;
		if (!patched_read)
		{
			if (patched_readMIOS)
			{
				printf("%s patch read, but the reloader wouldn't\n", text);
			} else
			{
				printf("%s not patch read\n", text);
			}
		}
		
		if (!patched_readID)
		{
			if (patched_readIDMIOS)
			{
				printf("%s patch read id, but the reloader wouldn't\n", text);
			} else
			{
				printf("%s not patch read id\n", text);
			}
		}
		
		if (!patched_seek)
		{
			printf("%s not patch seek\n", text);
		}

		if (streaming != 0)
		{
			if (!patched_readaudio)
			{
				printf("%s not patch read audio\n", text);
			}

			if (!patched_audioconfig)
			{
				printf("%s not patch audio config\n", text);
			}

			if (!patched_audiostatus)
			{
				printf("%s not patch audio status\n", text);
			}
		}

		if (use_debug_plugin)
		{
			if (!patched_reporterror)
			{
				printf("%s not patch report error\n", text);
			}

			if (!patched_reset)
			{
				printf("%s not patch reset\n", text);
			}

			if (!patched_stop_motor)
			{
				printf("%s not patch stop motor\n", text);
			}

			if (!patched_stream_write)
			{
				printf("%s not patch stream write\n", text);
			}
		}
	}
}

void clearmemory()
{
	memset((void *) 0x80003100, 0, 0x00d00000 - 0x3100);
	DCFlushRange((void *) 0x80003100, 0x00d00000 - 0x3100);
	ICInvalidateRange((void *) 0x80003100, 0x00d00000 - 0x3100);	
}

void maindolpatches(void *dst, u32 len)
{
#ifdef DONTTELL
	if ((hookselect != 0) && (ocarinaselect != 0 || debuggerselect != 0))
#else
	if (!backuploading && hookselect != 0 && (ocarinaselect != 0 || debuggerselect != 0))
#endif
	{
		if (dogamehooks(dst,len))
		{
			hookpatched = true;
		}
	}

	dvd_patchread(dst, len, (gcplugin_jumptable_debug_t *)plugin_base, false, backuploading || use_debug_plugin, use_high_plugin, use_debug_plugin);

	toy_register_apploader_file_loading(dst, len);
	
	if (dopatch__GXSetVAT)
	{
		GXSetVAT_count+=patch__GXSetVAT(dst, len);
	}
	DCFlushRange(dst, len);
	ICInvalidateRange(dst, len);	
}

void load_app()
{
	int rv;

	void (*app_init)(void (*report)(const char* fmt, ...));
	int (*app_main)(void** dst, int* size, int* offset);
	void* (*app_final)();
	void (*app_entry)(void(**init)(void (*report)(const char* fmt, ...)), 	int (**main)(), void *(**final)());

	static char apploader_header[32] __attribute__((aligned(32)));

	settime(secs_to_ticks(time(NULL) - 946684800));
	
	plugin_select();
	setup_sys_config();

	if (!use_patched_MIOS)
	{
		clearmemory();
	
		// Read apploader header
		rv = dvd_read(apploader_header, sizeof(apploader_header), APPLOADER_HEADER_OFFSET);
		DVD_CHECK();

		// Read apploader
		debug_printf("Reading apploader address 0x%x, len 0%x, offset 0x%x\n", (void*)APPLOADER_ADDRESS, ((*(unsigned long*)(apploader_header + 0x14)) + 31) & ~31, APPLOADER_OFFSET);
		rv = dvd_read((void*)APPLOADER_ADDRESS, ((*(unsigned long*)(apploader_header + 0x14)) + 31) & ~31, APPLOADER_OFFSET);
		DVD_CHECK();

		app_entry = (void (*)(void(**)(void (*)(const char*, ...)), int (**)(), void *(**)()))(*(unsigned long*)(apploader_header + 0x10));
		debug_printf("Calling app_entry...\n");
		app_entry(&app_init, &app_main, &app_final);
		
		debug_printf("Calling app_init...\n");
		app_init((void (*)(const char*, ...))nothing);
	}

	plugin_init();

	void* dst_array[64];
	int len_array[64];
	int last_index = -1;
	int fststart = 0;

	GXSetVAT_count = 0;
	hookpatched = false;

	if (!use_patched_MIOS)
	{
		toy_reset();

		while (1)
		{
			void* dst = 0;
			int len = 0,offset = 0;
			debug_printf("Requesting next section(app_main)...\n");
			int res = app_main(&dst, &len, &offset);

			if (!res)
				break;
			debug_printf("Reading section buffer 0x%x, len 0%x, offset 0x%x\n", dst, len, offset);
			rv = dvd_read(dst, len, offset);
			DVD_CHECK();

			last_index++;
			dst_array[last_index] = dst;
			len_array[last_index] = len;
			
			debug_printf("Reading section complete...\n");

			DCFlushRange(dst, len);
			ICInvalidateRange(dst, len);
		}
		debug_printf("All sections loaded...\n");

		dst_array[last_index+1] = (void *)0x81800000;
		int j = 0;
		while ( j <= last_index && (u32)dst_array[last_index-j] + len_array[last_index-j] == (u32)dst_array[last_index-j+1]) 
		{
			fststart = last_index - j;
			j++;
		}		
		if (fststart == 0)
		{
			for (j = 4; j <= last_index; j++)
			{
				if ((u32)dst_array[j] == *(u32 *)0x80000038)
				{
					fststart = j;
				}
			}
			if (fststart == 0)
			{
				fststart = last_index;
			}
		}		
	}

	if (use_high_plugin)
	{
		setup_memory_for_high_plugin();		// Also moves the fst	
	}

	copy_plugin();
	
	if (!use_patched_MIOS)
	{
		toy_patch_video_modes();
		u32 i;
		u32 patchstart = 3;
		
		// Action Replay
		if (memcmp((void *) GAMECODE_ADDRESS, "GNHE", 4) == 0)
		{
			patchstart = 0;
		}

		for (i=patchstart;i<fststart;i++)
		{
			maindolpatches(dst_array[i], len_array[i]);
		}

		// Check if the Wind Waker patch was applied
		if (dopatch__GXSetVAT)
		{
			if (GXSetVAT_count > 0)
			{
				printf("__GXSetVAT patched\n");
				debug_printf("__GXSetVAT patched\n");
			} else
			{
				printf("Warning: __GXSetVAT not patched\n");
				debug_printf("Warning: __GXSetVAT not patched\n");
				waittime+=10;
			}
		}
	} else
	{		
		dvd_patchread(MIOS_BASE, MIOS_SIZE, (gcplugin_jumptable_debug_t *)plugin_base, true, true, use_high_plugin, use_debug_plugin);
		DCFlushRange(MIOS_BASE, MIOS_SIZE);
		ICInvalidateRange(MIOS_BASE, MIOS_SIZE);	
	}	
	
	if (backuploading || use_debug_plugin)
	{
		DCFlushRange(plugin_base, plugin_size);
		ICInvalidateRange(plugin_base, plugin_size);
	}
	
	if (!use_patched_MIOS && memcmp((void *) GAMECODE_ADDRESS, "GCO", 3) != 0)	// GCOS doesn't work anyways
	{
		show_patch_warnings();
	}
	
	if (!use_patched_MIOS)
	{
#ifdef DONTTELL
		if ((hookselect != 0 || hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0))
#else
		if (!backuploading && hookselect != 0 && (ocarinaselect != 0 || debuggerselect != 0))
#endif
		{
			if (hookselect != 0)
			{
				if (!hookpatched)
				{
					printf("Hook not patched\n");
					waittime+=10;
				} else
				{
					//This is already printed when patching the hook
					//printf("Hook patched\n");
					//sleep(2);
					waittime+=5;
				}
			}
			//load_handler(); loaded before copying codes now
		}
	}
	
	if (!use_patched_MIOS && memcmp((void *) GAMECODE_ADDRESS, "GCO", 3) != 0)	// GCOS doesn't work anyways
	{
		u32 audiostreams;
		u32 badstreams;
		
		getaudiostreamnumbers(&audiostreams, &badstreams);
		
		if (streaming == 0 && audiostreams == 0)
		{
			printf("The game has no audio streaming\n");
		} else
		{
			if (audiostreams == 0)
			{
				printf("Audio stream bit set, but no audio streams identified\n");
				printf("This doesn't mean the game has no audio streaming, but it's possible\n");
			} else
			{		
				if (streaming != 0)
				{
					printf("Audio streaming bit set and\n");
				} else
				{
					printf("Audio streaming bit not set, but\n");
					waittime+=10;
				}
			
				if (badstreams == 0)
				{
					printf("found %u audio streams\n", audiostreams);
				} else
				if (audiostreams == badstreams)
				{
					if (audio_32K_fix)
					{
						printf("found %u audio streams, needed to 'fix' all of them(dirty fix!)\n", audiostreams);
						waittime+=5;
					} else
					{
						printf("found %u audio streams, all without 32K alignment\n", audiostreams);
						waittime+=10;
					}
				} else
				{
					if (audio_32K_fix)
					{
						printf("found %u audio streams, 'fixed' %u of them(dirty fix!)\n", audiostreams, badstreams);
						waittime+=5;
					} else
					{
						printf("found %u audio streams, %u without 32K alignment\n", audiostreams, badstreams);
						waittime+=10;
					}
				}
				
				if (backuploading)
				{
					printf("Expect missing audio!\n");
				}				
			}
			if (streaming == 2 || streaming == 4)
			{
				printf("Please report: This game uses 10 bytes streaming size\n");
			}	
		}
	} else
	{
		//TODO read fst into memory when booting MIOS to get the audio stream numbers
	}

	if (waittime > 0)
	{
		printf("Waiting...\n");
		sleep(waittime);
	}

	if (!use_patched_MIOS)
	{
		debug_printf("Launching game...\n");
		printf("Launching game...\n");
	} else
	{
		setup_MIOS_video_mode(*(char *)(0x80000003));
		debug_printf("Booting MIOS...\n");
		printf("Booting MIOS...\n");
	}

	sleep(5);
	
	if (!use_patched_MIOS)
	{
		setvideomode();
		
		appentrypoint = (u32)app_final();
		debug_printf("Entrypoint: 0x%0x8\n", appentrypoint);
		
		/* Close Exceptions: */
		unsigned long level = 0;
		_CPU_ISR_Disable(level);
		__exception_closeall();

#ifdef DONTTELL
		if ((hookselect != 0 || hook2select != 0) && (ocarinaselect != 0 || debuggerselect != 0))
#else
		if (!backuploading && hookselect != 0 && (ocarinaselect != 0 || debuggerselect != 0))
#endif
		{		
			__asm__(
				"lis %r3, appentrypoint@h\n"
				"ori %r3, %r3, appentrypoint@l\n"
				"lwz %r3, 0(%r3)\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"mtctr %r3\n"
				"bctr\n"
			);		
		} else
		{	
			void (*entrypoint)() = (void *)appentrypoint;
			entrypoint();
		}	
	} else
	{
		call_mios();
	}
}

