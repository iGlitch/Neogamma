#include <stdint.h>
#include <string.h>
#include <ogc/cache.h>

#include "systemconfig.h"
#include "video.h"
#include "gcplugin.h"
#include "Toy.h"
#include "Toy_video.h"
#include "debugprintf.h"
#include "loadapp.h"

#define GC_INIT_BASE			   	 (0x80000020)

/*
static uint32_t GC_HighConfig[64] = {
	0x0D15EA5E, 0x00000001, MEM_END_PHYS, 0x00000002,						//  0.. 3 80000020
	0x00000000, MEM_END_VIRT - 0x16380, MEM_END_VIRT - 0x1740, 0x00000024,	//  4.. 7 80000030
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							//  8..11 80000040
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							// 12..15 80000050
	0x38A00040, 0x7C6802A6, 0x9065000C, 0x80650008,							// 16..19 80000060
	0x64638000, 0x7C6803A6, 0x38600030, 0x7C600124,							// 20..23 80000070
	0x4E800020, 0x00000000, 0x00000000, 0x00000000,							// 24..27 80000080
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							// 28..31 80000090
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							// 32..35 800000A0
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							// 36..39 800000B0
	0x015D47F8, 0xF8248360, 0x00000000, 0x00000001,							// 40..43 800000C0 // unused
	0x00000000, 0x00000000, 0x00000000, 0x00000000,							// 44..47 800000D0 // unused
	0x814B7F50, 0x815D47F8, 0x00000000, MEM_END_VIRT,						// 48..51 800000E0
	MEM_END_PHYS, 0x00000000, 0x09A7EC80, 0x1CF7C580						// 52..55 800000F0
};
*/

static uint32_t GC_DefaultConfig[64] = {
	0x0D15EA5E, 0x00000001, 0x01800000, 0x00000002,	//  0.. 3 80000020
	0x00000000, 0x817e9c80, 0x817FE8C0, 0x00000024,	//  4.. 7 80000030
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	//  8..11 80000040
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	// 12..15 80000050
	0x38A00040, 0x7C6802A6, 0x9065000C, 0x80650008,	// 16..19 80000060
	0x64638000, 0x7C6803A6, 0x38600030, 0x7C600124,	// 20..23 80000070
	0x4E800020, 0x00000000, 0x00000000, 0x00000000,	// 24..27 80000080
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	// 28..31 80000090
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	// 32..35 800000A0
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	// 36..39 800000B0
	0x015D47F8, 0xF8248360, 0x00000000, 0x00000001,	// 40..43 800000C0 // unused
	0x00000000, 0x00000000, 0x00000000, 0x00000000,	// 44..47 800000D0 // unused
	0x814B7F50, 0x815D47F8, 0x00000000, 0x81800000,	// 48..51 800000E0
	0x01800000, 0x817FC8C0, 0x09A7EC80, 0x1CF7C580	// 52..55 800000F0
};

/* Values from Swiss:
/// The default boot up MEM1 lowmem values (from ipl when booting a game)
static const u32 GC_DefaultConfig[56] =
{
	0x0D15EA5E, 0x00000001, 0x01800000, 0x00000003, //  0.. 3 80000020
	0x00000000, 0x816FFFF0, 0x817FE8C0, 0x00000024, //  4.. 7 80000030
	0x00000000, 0x00000000, 0x00000000, 0x00000000, //  8..11 80000040
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 12..15 80000050
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 16..19 80000060
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 20..23 80000070
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 24..27 80000080
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 28..31 80000090
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 32..35 800000A0
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 36..39 800000B0
	0x015D47F8, 0xF8248360, 0x00000000, 0x00000001, // 40..43 800000C0
	0x00000000, 0x00000000, 0x00000000, 0x00000000, // 44..47 800000D0
	0x814B7F50, 0x815D47F8, 0x00000000, 0x81800000, // 48..51 800000E0
	0x01800000, 0x817FC8C0, 0x09A7EC80, 0x1CF7C580  // 52..55 800000F0
};
*/

u32 videomode;

void setvideomode()
{
	switch (videomode) 
	{
		case VIDEO_MODE_NTSC:
		case 'E':	// NTSC-U
		case 'J':	// NTSC-J
		case 'N':	// Japanese Import to USA
			/* NTSC */
			set_game_videomode(VIDEO_MODE_NTSC);
			//GC_DefaultConfig[43] = 0;
			//GC_HighConfig[43] = 0;
		break;

		case VIDEO_MODE_PAL:
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
			set_game_videomode(VIDEO_MODE_PAL);
			//GC_DefaultConfig[43] = 1;
			//GC_HighConfig[43] = 1;
		break;
		
		case VIDEO_MODE_PAL60:
			/* PAL60 */
			set_game_videomode(VIDEO_MODE_PAL60);
			//GC_DefaultConfig[43] = 5;
			//GC_HighConfig[43] = 5;
		break;
		
		case VIDEO_MODE_NTSC480P:
			/* 480P */
			set_game_videomode(VIDEO_MODE_NTSC480P);
			//GC_DefaultConfig[43] = 0;
			//GC_HighConfig[43] = 0;
		break;

		case VIDEO_MODE_PAL480P:
			/* 480P */
			set_game_videomode(VIDEO_MODE_PAL480P);
			//GC_DefaultConfig[43] = 5;
			//GC_HighConfig[43] = 5;
		break;
		
		default:
			/* Unknown mode. Don't change. */
		break;
	}
}


/** Initialization routine with the defaults */
void setup_video(unsigned long mode)
{
	videomode = mode;
	switch (mode) 
	{
		case VIDEO_MODE_NTSC:
		case 'E':	// NTSC-U
		case 'J':	// NTSC-J
		case 'N':	// Japanese Import to USA
			/* NTSC */
			set_rmode(&TVNtsc480IntDf);
		break;

		case VIDEO_MODE_PAL:
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
			set_rmode(&TVPal528IntDf);
		break;
		
		case VIDEO_MODE_PAL60:
			/* PAL60 */
			set_rmode(&TVEurgb60Hz480IntDf);
		break;
		
		case VIDEO_MODE_NTSC480P:
			/* 480P */
			set_rmode(&TVNtsc480Prog);
		break;

		case VIDEO_MODE_PAL480P:
			/* 480P */
			set_rmode(&TVEurgb60Hz480Prog);
		break;
		
		default:
			/* Unknown mode. Don't change. */
		break;
	}
}

void setup_memory_for_high_plugin()
{
	debug_printf("Setup memory for high plugin + move fst and bi2\n");

	u32 MEM_END_VIRT = ((u32) HIGH_PLUGIN_BASE);
	u32 MEM_END_PHYS = (MEM_END_VIRT & 0x01FFFFFF);

	u32 old_fst_offset = *(u32 *)0x80000038;

	u32 arena_high_gap = 0x81800000 - *(u32 *)0x80000034;
	u32 fst_size = 0x81800000 - old_fst_offset;

	u32 new_fst_offset = MEM_END_VIRT - fst_size;
	
	u32 old_bi2_offset = *(u32 *)0x800000F4;
	u32 bi2_size = 0x2000;
	u32 new_bi2_offset = MEM_END_VIRT - (0x81800000 - old_bi2_offset);
	
	memcpy((void *)0x81700000, (void *)old_bi2_offset, bi2_size);
	memcpy((void *)0x81702000, (void *)old_fst_offset, fst_size);

	memset((void *)old_bi2_offset, 0, bi2_size);
	memset((void *)old_fst_offset, 0, fst_size);

	memcpy((void *)new_bi2_offset, (void *)0x81700000, bi2_size);
	memcpy((void *)new_fst_offset, (void *)0x81702000, fst_size);

	memset((void *)0x81700000, 0, bi2_size);
	memset((void *)0x81702000, 0, fst_size);
	
	*(u32 *)0x80000028 = MEM_END_PHYS;
	*(u32 *)0x80000034 = MEM_END_VIRT - arena_high_gap;
	*(u32 *)0x80000038 = new_fst_offset;
	*(u32 *)0x800000EC = MEM_END_VIRT;
	*(u32 *)0x800000F0 = MEM_END_PHYS;
	*(u32 *)0x800000F4 = new_bi2_offset;

	DCFlushRange((void *)new_bi2_offset, bi2_size);
	ICInvalidateRange((void *)new_bi2_offset, bi2_size);

	DCFlushRange((void *)new_fst_offset, fst_size);
	ICInvalidateRange((void *)new_fst_offset, fst_size);

	DCFlushRange((void *)0x80000000, 0x100);
	ICInvalidateRange((void *)0x80000000, 0x100);
}

void setup_sys_config()
{
	/* Copy configuration, but don't overwrite OS context pointers. */
	debug_printf("Setup memory for low/no plugin\n");
	/* Game plugin loaded to low memory (reserved exception handler area). */
	//memcpy((void *) GC_INIT_BASE, GC_DefaultConfig, 0xc0);
	//memcpy((void *) (GC_INIT_BASE + 0xe8), (void *) (((uint32_t) GC_DefaultConfig) + 0xe8), 0x18);
	memcpy((void *) GC_INIT_BASE, GC_DefaultConfig, 0xa0);
	memcpy((void *) (GC_INIT_BASE + 0xc8), (void *) (((uint32_t) GC_DefaultConfig) + 0xc8), 0x18);

	DCFlushRange((void *) 0x80000000, 0x3400);
	ICInvalidateRange((void *) 0x80000000, 0x3400);
}

