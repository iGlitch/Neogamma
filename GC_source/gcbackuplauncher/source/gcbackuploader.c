#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <unistd.h>

#include "dvd.h"
#include "loadapp.h"
#include "gameloader.h"
#include "screen.h"
#include "debugprintf.h"
#include "dvd.h"
#include "background.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void clearScreen(void)
{
	//VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	printf("\x1b[J");
}

int main(int argc, char **argv)
{
	VIDEO_Init();

	rmode = VIDEO_GetPreferredMode(0);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	//VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

 	CON_InitEx(rmode, 24, 212, rmode->fbWidth -32, rmode->xfbHeight -212 -32);

	DrawBackground(rmode, (u32)xfb);

	if (strcmp((char *)0x807FFF00, "GAMECUBELOADER01") == 0)
	{
		start_from_NeoGamma();
	} else
	{
	
	}
	printf("Error: Loader not started from NeoGamma, rebooting...");
	sleep(7);

	SYS_ResetSystem(SYS_RESTART, 0, 0);
	return 0;
}
