#include <stdint.h>
#include <string.h>
#include <gccore.h>

#include "video.h"

/** Base address for video registers. */
#define MEM_VIDEO_BASE (0xCC002000)

/* Region defaults by tmbinc (Copied from GCOS). */
static uint32_t vi_Mode640X480NtscYUV16[32] = {
	0x0F060001, 0x476901AD, 0x02EA5140, 0x00030018, 0x00020019,
	0x410C410C, 0x40ED40ED, 0x00435A4E, 0x00000000, 0x00435A4E,
	0x00000000, 0x00000000, 0x110701AE, 0x10010001, 0x00010001,
	0x00010001, 0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
	0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF, 0x13130F08,
	0x00080C0F, 0x00FF0000, 0x00000000, 0x02800000, 0x000000FF,
	0x00FF00FF, 0x00FF00FF
};

static uint32_t vi_Mode640x480NtscProgressiveYUV16[32] = {
	0x1e0c0005, 0x476901ad, 0x02ea5140, 0x00060030, 0x00060030,
	0x81d881d8, 0x81d881d8, 0x10000000, 0x00000000, 0x00000000,
	0x00000000, 0x037702b6, 0x90010001, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x28280100, 0x1ae771f0,
	0x0db4a574, 0x00c1188e, 0xc4c0cbe2, 0xfcecdecf, 0x13130f08,
	0x00080c0f, 0x00ff0000, 0x00010001, 0x02800000, 0x000000ff,
	0x00ff00ff, 0x00ff00ff
};

static uint32_t vi_Mode640X576Pal50YUV16[32] = {
	0x11F50101, 0x4B6A01B0, 0x02F85640, 0x00010023, 0x00000024,
	0x4D2B4D6D, 0x4D8A4D4C, 0x0066D480, 0x00000000, 0x0066D980,
	0x00000000, 0x00C901F3, 0x913901B1, 0x90010001, 0x00010001,
	0x00010001, 0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
	0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF, 0x13130F08,
	0x00080C0F, 0x00FF0000, 0x00000000, 0x02800000, 0x000000FF,
	0x00FF00FF, 0x00FF00FF
};

static uint32_t vi_Mode640X480Pal60YUV16[32] = {
	0x0F060001, 0x476901AD, 0x02EA5140, 0x00030018, 0x00020019,
	0x410C410C, 0x40ED40ED, 0x0066D480, 0x00000000, 0x0066D980,
	0x00000000, 0x00C9010F, 0x910701AE, 0x90010001, 0x00010001,
	0x00010001, 0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
	0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF, 0x13130F08,
	0x00080C0F, 0x00FF0000, 0x00000000, 0x02800000, 0x000000FF,
	0x00FF00FF, 0x00FF00FF
};
/*
uint32_t GXPal528IntDf[15] = {
	0x00000004, 0x02800210, 0x02100028, 0x00170280,
	0x02100000, 0x00000001, 0x00000606, 0x06060606,
	0x06060606, 0x06060606, 0x06060606, 0x06060606,
	0x06060808, 0x0A0C0A08, 0x08000000
};
	
uint32_t GXMpal480IntDf[15] = {
	0x00000008, 0x028001E0, 0x01E00028, 0x00000280,
	0x01E00000, 0x00000001, 0x00000606, 0x06060606,
	0x06060606, 0x06060606, 0x06060606, 0x06060606,
	0x06060808, 0x0A0C0A08, 0x08000000
};
	
uint32_t GXNtsc480Int[15] = {
	0x00000000, 0x028001E0, 0x01E00028, 0x00000280,
	0x01E00000, 0x00000001, 0x00000606, 0x06060606,
	0x06060606, 0x06060606, 0x06060606, 0x06060606,
	0x06060000, 0x15161500, 0x00000000
};
	
uint32_t GXNtsc480IntDf[15] = {
	0x00000000, 0x028001E0, 0x01E00028, 0x00000280,
	0x01E00000, 0x00000001, 0x00000606, 0x06060606,
	0x06060606, 0x06060606, 0x06060606, 0x06060606,
	0x06060808, 0x0A0C0A08, 0x08000000
};
	
uint32_t GXEurgb60Hz480IntDf[15] = {
	0x00000014, 0x028001E0, 0x01E00028, 0x00000280,
	0x01E00000, 0x00000001, 0x00000606, 0x06060606,
	0x06060606, 0x06060606, 0x06060606, 0x06060606,
	0x06060808, 0x0A0C0A08, 0x08000000
};
*/


void set_game_videomode(int mode)
{
	uint32_t *mem_video_base = (void *) MEM_VIDEO_BASE;

	VIDEO_WaitVSync();
	VIDEO_WaitVSync();

	switch (mode) 
	{
		case VIDEO_MODE_NTSC:
			*(u32 *)0x800000CC = 0;		
			/* Keep current screen addresses to avoid green screen. */
			memcpy(&vi_Mode640X480NtscYUV16[7], &mem_video_base[7], 16);
			/* Set video configuration. */
			memcpy(mem_video_base, vi_Mode640X480NtscYUV16, 0x80);
		break;

		case VIDEO_MODE_PAL:
			*(u32 *)0x800000CC = 1;		
			/* Keep current screen addresses to avoid green screen. */
			memcpy(&vi_Mode640X576Pal50YUV16[7], &mem_video_base[7], 16);
			/* Set video configuration. */
			memcpy(mem_video_base, vi_Mode640X576Pal50YUV16, 0x80);
		break;

		case VIDEO_MODE_PAL60:
			*(u32 *)0x800000CC = 5;	// or is 2 correct?	
			/* Keep current screen addresses to avoid green screen. */
			memcpy(&vi_Mode640X480Pal60YUV16[7], &mem_video_base[7], 16);
			/* Set video configuration. */
			memcpy(mem_video_base, vi_Mode640X480Pal60YUV16, 0x80);
		break;

		case VIDEO_MODE_NTSC480P:
			*(u32 *)0x800000CC = 0;		
			/* Keep current screen addresses to avoid green screen. */
			memcpy(&vi_Mode640x480NtscProgressiveYUV16[7], &mem_video_base[7], 16);
			/* Set video configuration. */
			memcpy(mem_video_base, vi_Mode640x480NtscProgressiveYUV16, 0x80);
		break;

		case VIDEO_MODE_PAL480P:
			*(u32 *)0x800000CC = 5;		
			/* Keep current screen addresses to avoid green screen. */
			memcpy(&vi_Mode640x480NtscProgressiveYUV16[7], &mem_video_base[7], 16);
			/* Set video configuration. */
			memcpy(mem_video_base, vi_Mode640x480NtscProgressiveYUV16, 0x80);
		break;

		default:
			/* Unknown video mode. Don't change anything. */
		break;
	}

}
