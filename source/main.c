/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *  Copyright (c) 2009 WiiPower
 *
 *  this file is part of GeckoOS for USB Gecko
 *  http://www.usbgecko.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <sys/unistd.h>
#include "geckomenu.h"
#include "font.h"
#include "dvd_broadway.h"
#include "apploader.h"
#include "patchcode.h"
#include "tools.h"
#include "libcios.h"
#include "config.h"
#include "tools.h"

#include "easywbfs.h"


#define logogfx_size 563200


//extern u32 *xfb;
u8 gecko_command = 0;

extern s32 __SYS_LoadMenu();

static void power_cb () 
{
	Power_Flag = true;
}

static void reset_cb () 
{
	Reset_Flag = true;
}

int main(int argc, char **argv) 
{
	bwDVD_LowInit();
	bwDVD_LowStopMotor(0, 0);
	
	// Detect if Sneek is running
	Sneek_GameCount = 0;
	int ret = Sneek_DVDGetGameCount( &Sneek_GameCount );
	
	if (ret >= 0 && Sneek_GameCount > 0)
	{
		Sneek_Mode = true;
	} else
	{
		Sneek_Mode = false;
		bwDVD_LowClose();
	}	

	if (Sneek_Mode == false)
	{
		// Get the ID for the return to channel function
		if (ES_GetTitleID(&old_title_id) < 0)
		{
			old_title_id = (0x00010001ULL << 32) | *(u32 *)0x80000000;
		}
		
		if (TITLE_UPPER(old_title_id) <= 1 || TITLE_LOWER(old_title_id) <= 2)
		{
			// HBC reload stub -> assuming boot from HBC
			if (strncmp("STUBHAXX", (char *)0x80001804, 8) == 0)
			{
				old_title_id = (0x00010001ULL << 32) | 0xAF1BF516;
			}	
		}
	}

	drivedate = 0;
	drive_state = 0;

	Load_Config();

	if (Sneek_Mode == false)
	{
		if (Load_IOS(CIOS_VERSION) < 0)
		{
			CIOS_VERSION = 249;
			Load_IOS(CIOS_VERSION);
		}
	} else
	{
		PAD_Init();
		WPAD_Init();
		WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	}

	//*(vu32*)0xCD00643C = 0x00000000;	// 32Mhz on Bus

	init_video();
	
	Power_Flag = false;
	Reset_Flag = false;
	SYS_SetPowerCallback (power_cb);
    SYS_SetResetCallback (reset_cb);	
	
/*	// Autoboot code part 1
	time_t t;
	t = time(NULL) + 1; // Wait 1 to finish wpad init process 
	while (time(NULL) < t) ;
*/

	show_banner();
	
/*	// Autoboot code part 2
	t = time(NULL) + 3; // Wait 3 seconds for button press to cancel autoboot
	bool autoboot = true;

	while (time(NULL) < t)
	{
		WPAD_ScanPads();
		PAD_ScanPads();

		if (WPAD_ButtonsDown(0) || PAD_ButtonsDown(0)) 
		{
			autoboot = false;
			break;
		}
	}
*/
	clearmenu();
	
	// Load the Sneek games list
	if (Sneek_Mode == true)
	{
		Sneek_DICfg = (Sneek_DIConfig *)allocate_memory(Sneek_GameCount * 0x80 + 0x10 );
		if (Sneek_DICfg == NULL)
		{
			return 0;
		}
		Sneek_DVDReadGameInfo( 0, Sneek_GameCount * 0x80 + 0x10, Sneek_DICfg );
		bwDVD_LowClose();
		
		//ISFS_Initialize();
		//write_file_to_nand("/dicfg.bin", (void *)Sneek_DICfg, Sneek_GameCount * 0x80 + 0x10);

		Sneek_build_sorted_game_list();
	}
	
	drawmenu(0);
	drawselected(0);
	
/*	// Autoboot code part 3
	if (autoboot)
	{
		clearmenu();
		write_font(185, 125 + 1*24,true, "Launching Game");
		write_font(80, screenheight - 60,true, "NeoGamma R8 RC1, IOS%u (Rev %u)", IOS_GetVersion(), IOS_GetRevision());
		apploader();
		sleep(6);
		clearmenu();
	} else
	{
		write_font(80, screenheight - 60,true, "Autoboot canceled");
		VIDEO_WaitVSync();
		t = time(NULL) + 3; // Wait 3 seconds for button pressing to stop 
		while (time(NULL) < t) ;
		clearscreen(xfb, 20, screenheight - 60, 278, 24, 0x00800080);
	}
*/	

	while(1)
	{
		processwpad();
	}

	return 0;
}
