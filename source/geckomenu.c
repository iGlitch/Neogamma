/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *  Copyright (C) 2009 WiiPower
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

// This menu code is pretty shit, i wrote it very quickly, and just for speed.
// Would really like for someone to help out to write a decent menu
// Progress bar isn't threaded etc


#include <malloc.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogcsys.h>
#include <sys/unistd.h>
#include "geckomenu.h"
#include "font.h"
#include "apploader.h"
#include "dvd_broadway.h"
#include "patchcode.h"
#include "geckogreen.h"
#include "geckogrey.h"
#include "tools.h"
#include "identify.h"
#include "libcios.h"
#include "easywbfs.h"
#include "config.h"
#include "sdhc.h"


#define fontheight 24
#define innermenu_start 20
#define innermenu_end 306
#define screenwidth 320
#define progtotal 132

u32 barstate;
u32 skipvalue = 0;
u32 startypos = 125;

char languages[12][22] = 
{{"Console Default"},
{"Japanese"},
{"English"},
{"German"},
{"French"},
{"Spanish"},
{"Italian"},
{"Dutch"},
{"S. Chinese"},
{"T. Chinese"},
{"Korean"},
{"Auto Force"}};

char wii_hook_options[8][15] = 
{{"None"},
{"VBI"},
{"KPAD"},
{"Joypad"},
{"GXDraw"},
{"GXFlush"},
{"OSSleepThread"},
{"AXNextFrame"}
};

char wii_hook2_options[9][15] = 
{{"None"},
{"VBI"},
{"KPAD"},
{"Joypad"},
{"GXDraw"},
{"GXFlush"},
{"OSSleepThread"},
{"AXNextFrame"},
{"= 1st hook"}
};

char gc_hook_options[8][15] = 
{{"None"},
{"VI"},
{"OSSleepthread"},
{"gxdrawdone"},
{"Unknown0"},
{"Unknown1"},
{"Unknown2"},
{"Unknown3"}
};

char gc_hook2_options[9][15] = 
{{"None"},
{"VI"},
{"OSSleepthread"},
{"gxdrawdone"},
{"Unknown0"},
{"Unknown1"},
{"Unknown2"},
{"Unknown3"},
{"= 1st hook"}
};

char video_options[9][15] =
{{"Force Wii"},
{"Disc(default)"},
{"NTSC480i"},
{"NTSC480p"},
{"PAL480i"},
{"PAL480p"},
{"PAL576i"},
{"MPAL480i"},
{"MPAL480p"}};

char gc_video_options[8][15] =
{{"Force Wii"},
{"Disc(default)"},
{"NTSC480i"},
{"NTSC480p"},
{"PAL(auto)"},
{"PAL576i"},
{"PAL480i"},
{"PAL480p"}};

char videopatch_options[8][16] =
{{"No"},
{"Normal"},
{"More"},
{"All"},
{"Sneek"},
{"Sneek+Normal"},
{"Sneek+More"},
{"Sneek+All"}};

char wbfs_options[4][27] =
{{"Load Games from: <DVD>"},
{"Load Games from: <USB>"},
{"Load Games from: <SD/SDHC>"},
{"Load GC Games via DML"}};

char patchedMIOS_options[3][6] = 
{{"Auto"},
{"No"},
{"Yes"}};

char highplugin_options[4][6] = 
{{"Auto"},
{"Low"},
{"High"},
{"Debug"}};

char audiofix_options[5][16] = 
{{"Auto"},
{"No"},
{"Yes"},
{"No + 32K"},
{"Yes + 32K"}};


char alternativedol_options[5][11] = 
{{"No"},
{"Storage"},
{"Disc"},
{"Disc+"},
{"Disc+ only"}};

char storage_options[6][16] = 
{{"SD using cIOS"},
{"USB using cIOS"},
{"SD using IOS36"},
{"USB using IOS36"},
{"SD using IOS61"},
{"USB using IOS61"}};

char correctios_options[3][16] = 
{{"Default"},
{"correct IOS"},
{"Autodetect"},
};

char gamecubemode_options[3][9] = 
{{"MIOS"},
{"internal"},
{"external"}};

char debugger_options[3][9] = 
{{"No"},
{"Yes"},
{"p. start"}};

char reloader_options[5][9] = 
{{"No"},
{"Auto"},
{"fast"},
{"forced"},
{".elf fix"}};

char bool_options[2][6] = 
{{"No"},
{"Yes"}};


extern u32 *xfb;
extern const unsigned char kenobiwii[];
extern const int kenobiwii_size;
extern s32 __SYS_LoadMenu();


u32 currentmenu = ROOTMENU;	// 0 ROOT
s32 rootmenu_item = 0;
u32 menufreeze = 0;

u32 mainmenu_item = 0;
u32 optionmenu_item = 0;


static void _plotpixel(u32 *framebuffer, u16 x, u16 y, u32 color)
{
	framebuffer[320*y+x] = color;
}

void drawicon(u32 *framebuffer, u16 xscreen, u16 yscreen, u16 width, u16 height, u32 gicon)
{
	
	u16 x = 0;
	u16 y = 0;
	u32 i = 0;
	u32 colorgreen = 0;
	u32 colorgrey = 0;

	for(y = yscreen; y < yscreen+height; y++)
	{
		for(x = xscreen; x < xscreen+width; x++)
		{
			if(gicon){	// high green, grey low
				colorgreen = be32(geckogreen+i);
				_plotpixel(framebuffer, x, y, colorgreen);
			}
			else {
				colorgrey = be32(geckogrey+i);
				_plotpixel(framebuffer, x, y, colorgrey);
			}
			
			i += 4;
		}
	}
}


void clearscreen(u32 *framebuffer, u16 xscreen, u16 yscreen, u16 width, u16 height, u32 color)
{	
	u16 x;
	u16 y;

	for(y = yscreen; y < yscreen+height; y++)
	{
		for(x = xscreen; x < xscreen+width; x++)
		{
			_plotpixel(framebuffer, x, y, color);
		}
	}
}

void clearmenu()
{
	clearscreen(xfb, 0, 110, 319, 290, 0x00800080);
}

/* All wii remote joypad handling and menu processing */
void processwpad()
{
	u32 pressed;
	u32 pressedGC;
	s32 ret;
	
	VIDEO_WaitVSync();
	clearmenu();
	drawselected(rootmenu_item);
	drawmenu(currentmenu);

	waitforbuttonpress(&pressed, &pressedGC, NULL);
 
	/* Root menu actions */
	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)))
	{
		// Launch game
		if (rootmenu_item == 0 && (Sneek_Mode == true || wbfsdevice == 0 || getGameCount() > 0) )
		{
			clearmenu();
			
			Save_Config();
			
			write_font(185, startypos + 1*24,true, "Launching Game");
			
			ret = apploader();
			
			bwDVD_LowStopMotor(0, 0);
			
			drive_state = 0;

			show_banner();
			
			if (ret != -101)
			{
				// Being here means there was an error
				waitforbuttonpress(NULL, NULL, NULL);
			}

			clearmenu();
			pressed = 0;
			pressedGC = 0;
		}
		
		// Mount wbfs
		if ( (Sneek_Mode == false) && ((rootmenu_item == 0 || rootmenu_item == 1) && getGameCount() == 0 && wbfsdevice != 0))
		{
			u32 trynr = 1;
			s32 ret = -1;
			if (wbfsdevice == 3) 	// DML
			{
				clearscreen(xfb, 0, startypos + 1*24, 319, 26, 0x00800080);
				write_font(115, startypos + 1*24,true, "Searching gamecube games..");
			
				ret = get_DML_game_list();
				if (ret < 0)
				{
					clearscreen(xfb, 0, startypos + 1*24, 319, 26, 0x00800080);
					switch (ret)
					{
						case -1:
							write_font(115, startypos + 1*24,true, "No SD card found");
						break;
						case -2:
							write_font(115, startypos + 1*24,true, "No FAT partition found");
						break;
						case -3:
							write_font(115, startypos + 1*24,true, "No games found");
						break;
						case -4:
							write_font(115, startypos + 1*24,true, "Out of memory");
						break;	
					}
					wait(5);
				}
			} else					// WBFS
			{
				while (ret < 0  && trynr <= 30)
				{
					if (trynr == 7)
					{
						WPAD_Shutdown();
					}
					if (trynr >= 7)
					{
						block_ios_reload(false);
						IOS_ReloadIOS(CIOS_VERSION);
						block_ios_reload(false);
						set_cIOS_stealth_mode(false);
					}
					ret = Try_WBFS_Init();
					
					if (ret < 0)
					{
						Verify_Flags(); // Standby and Reset
					
						WPAD_ScanPads();
						u32 pressed = WPAD_ButtonsDown(0);

						PAD_ScanPads();
						u32 pressedGC = PAD_ButtonsDown(0);

						clearscreen(xfb, 0, startypos + 1*24, 319, 26, 0x00800080);
						if (wbfsdevice == 1)
						{
							write_font(115, startypos + 1*24,true, "Mounting USB Tries left: %u", 30-trynr);
						} else
						{
							write_font(115, startypos + 1*24,true, "Mounting SD/SDHC Tries left: %u", 30-trynr);
						}
						if (pressed || pressedGC)
						{
							break;		
						}
						VIDEO_WaitVSync();
					}
					trynr++;
				}			
				if (trynr >= 7)
				{
					PAD_Init();
					WPAD_Init();
					WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
				}
				clearscreen(xfb, 0, startypos + 1*24, 319, 26, 0x00800080);
				if (ret >= 0)
				{
					ret = initGameList();
					if (ret < 0)
					{
						switch (ret)
						{
							case -1:
								write_font(115, startypos + 1*24,true, "Out of memory(1)");
							break;
							case -2:
								write_font(115, startypos + 1*24,true, "storage read1 failed");
							break;
							case -3:
								write_font(115, startypos + 1*24,true, "storage read2 failed");
							break;
							case -4:
								write_font(115, startypos + 1*24,true, "No WBFS partition found");
							break;
							case -5:
								write_font(115, startypos + 1*24,true, "WBFS is NULL...");
							break;						
							case -6:
								write_font(115, startypos + 1*24,true, "WBFS_GetCount() failed");
							break;
							case -7:
								write_font(115, startypos + 1*24,true, "No games found");
							break;
							case -8:
								write_font(115, startypos + 1*24,true, "Out of memory(2)");
							break;
							case -9:
								write_font(115, startypos + 1*24,true, "WBFS_GetHeaders() failed");
							break;
							default:
								write_font(115, startypos + 1*24,true, "???");
							break;					
						}
						wait(5);
					}				
					
					SDHC_Close();
				}
			}
		}	

		if (rootmenu_item == 3)
		{
			mainmenu_item = rootmenu_item;
			currentmenu = OPTIONSMENU;
			rootmenu_item = 0;
			//clearmenu();
		}

		if (rootmenu_item == 4 && showrebooterselect == 1)
		{
			clearmenu();
			Save_Config();	
			write_font(185, startypos + 1*24,true, "Rebooting With Hooks");
			boot_menu();
			drive_state = 0;
			
			show_banner();
			
			// Being here means there was an error
			waitforbuttonpress(NULL, NULL, NULL);

			clearmenu();
			pressed = 0;
			pressedGC = 0;
		}

		if (rootmenu_item == 4+showrebooterselect)
		{
			mainmenu_item = rootmenu_item;
			currentmenu = ABOUTMENU;
			rootmenu_item = about_itemcount -1;
			menufreeze = 1;
			//clearmenu();
		}

		if (rootmenu_item == 6+showrebooterselect)
		{
			Save_Config();
			if (strncmp("STUBHAXX", (char *)0x80001804, 8) == 0) exit(0);
			SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
		}
		
		// Do this because don't want button A be recognised by one of the sub menus, if it's not pressed there
		pressed = 0;
		pressedGC = 0;
	}
	
	// Root Left
	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{	
		if(rootmenu_item == 0 && getGameCount() != 0)
		{
			if(gameselected == 0)
			{
				gameselected = getGameCount()-1;
			}
			else {
				gameselected -= 1;
			}	
		}

		if(rootmenu_item == 1 && Sneek_Mode == false)
		{
			gameselected = 0;
			if(wbfsdevice == 0)
			{
				wbfsdevice = 3;
			}
			else {
				wbfsdevice -= 1;
			}
		}
	}
	
	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_MINUS) || (pressed & WPAD_CLASSIC_BUTTON_FULL_L) || (pressedGC & PAD_TRIGGER_L)))
	{
		if(rootmenu_item == 0)
		{
			if(gameselected >= 7)
			{
				gameselected -= 7;
			}
		}
	}
	
	// Root Right
	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{	
		if (rootmenu_item == 0 && getGameCount() > 1)
		{
			if ( (gameselected == getGameCount()-1) )
			{
				gameselected = 0;
			}
			else {
				gameselected += 1;
			}	
		}

		if(rootmenu_item == 1 && Sneek_Mode == false)
		{
			gameselected = 0;
			if(wbfsdevice == 3)
			{
				wbfsdevice = 0;
			}
			else 
			{
				wbfsdevice += 1;
			}
		}
	}

	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_PLUS) || (pressed & WPAD_CLASSIC_BUTTON_FULL_R) || (pressedGC & PAD_TRIGGER_R)))
	{
		if(rootmenu_item == 0)
		{
			if(gameselected+7 <= getGameCount()-1)
			{
				gameselected += 7;
			}
		}
	}

	if (currentmenu == OPTIONSMENU && ((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)))
	{
		optionmenu_item = rootmenu_item;
		if (rootmenu_item == 0)
		{
			currentmenu = GENERALMENU;
			rootmenu_item = 0;
			//clearmenu();
		}

		if (rootmenu_item == 1)
		{
			currentmenu = WIIMENU;
			rootmenu_item = 0;
			//clearmenu();
		}
	
		if (rootmenu_item == 2)
		{
			currentmenu = GCMENU;
			rootmenu_item = 0;
			//clearmenu();
		}

		if (rootmenu_item == 3)
		{
			currentmenu = OCARINAMENU;
			rootmenu_item = 0;
			//clearmenu();
		}

		if (rootmenu_item == 4)
		{
			currentmenu = REBOOTMENU;
			rootmenu_item = 0;
			//clearmenu();
		}

		if (rootmenu_item == options_itemcount - 1)
		{
			rootmenu_item = mainmenu_item;
			currentmenu = ROOTMENU;
			//clearmenu();
		}
	}
	
	if (currentmenu == ROOTMENU && ((pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = 0;
		//clearmenu();
	}

	if (currentmenu == OPTIONSMENU && ((pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = mainmenu_item;
		currentmenu = ROOTMENU;
		//clearmenu();
	}

	if (currentmenu == GENERALMENU && ( (((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)) && rootmenu_item == general_itemcount - 1) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = optionmenu_item;
		currentmenu = OPTIONSMENU;
		//clearmenu();
	}

	if (currentmenu == WIIMENU && ( (((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)) && rootmenu_item == wii_itemcount - 1) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = optionmenu_item;
		currentmenu = OPTIONSMENU;
		//clearmenu();
	}

	if (currentmenu == GCMENU && ( (((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)) && rootmenu_item == gc_itemcount - 1) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = optionmenu_item;
		currentmenu = OPTIONSMENU;
		//clearmenu();
	}
	
	if (currentmenu == OCARINAMENU && ( (((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)) && rootmenu_item == ocarina_itemcount - 1) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = optionmenu_item;
		currentmenu = OPTIONSMENU;
		//clearmenu();
	}

	if (currentmenu == REBOOTMENU && ( (((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A)) && rootmenu_item == rebooter_itemcount - 1) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		rootmenu_item = optionmenu_item;
		currentmenu = OPTIONSMENU;
		//clearmenu();
	}

	/* About menu actions */
	if (currentmenu == ABOUTMENU && ((pressed & WPAD_BUTTON_A) || (pressed & WPAD_CLASSIC_BUTTON_A) || (pressedGC & PAD_BUTTON_A) || (pressed & WPAD_BUTTON_B) || (pressed & WPAD_CLASSIC_BUTTON_B) || (pressedGC & PAD_BUTTON_B)))
	{
		menufreeze = 0;
		rootmenu_item = mainmenu_item;
		currentmenu = ROOTMENU;			
		//clearmenu();
	}

	if (currentmenu == GENERALMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{		
		if(rootmenu_item == 0 && Sneek_Mode == false){
			if(usecorrectiosoption == 0){
				usecorrectiosoption = 2;
			}
			else {
				usecorrectiosoption -= 1;
			}	
		}		

		if(rootmenu_item == 1 && Sneek_Mode == false){
			if(storageselect == 0){
				storageselect = 5;
			}
			else {
				storageselect -= 1;
			}	
		}		

		if(rootmenu_item == 2){
			if(showrebooterselect == 0){
				showrebooterselect = 1;
			}
			else {
				showrebooterselect -= 1;
			}	
		}

		if(rootmenu_item == 3){
			if(configselect == 0){
				configselect = 1;
			}
			else {
				configselect -= 1;
			}	
		}		
	}

	if (currentmenu == GENERALMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{
		if(rootmenu_item == 0 && Sneek_Mode == false){
			if(usecorrectiosoption == 2){
				usecorrectiosoption = 0;
			}
			else {
				usecorrectiosoption += 1;
			}	
		}	

		if(rootmenu_item == 1 && Sneek_Mode == false){
			if(storageselect == 5){
				storageselect = 0;
			}
			else {
				storageselect += 1;
			}	
		}		

		if(rootmenu_item == 2){
			if(showrebooterselect == 1){
				showrebooterselect = 0;
			}
			else {
				showrebooterselect += 1;
			}	
		}

		if(rootmenu_item == 3){
			if(configselect == 1){
				configselect = 0;
			}
			else {
				configselect += 1;
			}	
		}		
	}
	
	if (currentmenu == WIIMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{	
		if(rootmenu_item == 0){
			if(langselect == 0){
				langselect = 11;
			}
			else {
				langselect -= 1;
			}	
		}

		if(rootmenu_item == 1){
			if(videoselect == 0){
				videoselect = 8;
			}
			else {
				videoselect -= 1;
			}	
		}

		/* force ntsc patch */
		if(rootmenu_item == 2){
			if(videopatchselect == 0){
				videopatchselect = 7;
			}
			else {
				videopatchselect -= 1;
			}	
		}

		if(rootmenu_item == 3){
			if(viselect == 0){
				viselect = 1;
			}
			else {
				viselect -= 1;
			}	
		}

		if(rootmenu_item == 4){
			if(countrystringselect == 0){
				countrystringselect = 1;
			}
			else {
				countrystringselect -= 1;
			}	
		}
		
		if(rootmenu_item == 5){
			if(alternativedolselect == 0){
				alternativedolselect = 4;
			}
			else {
				alternativedolselect -= 1;
			}	
		}		
		
		if(rootmenu_item == 6){
			if(patchselect == 0){
				patchselect = 1;
			}
			else {
				patchselect -= 1;
			}	
		}		

		if(rootmenu_item == 7){
			if(blockiosreloadselect == 0){
				blockiosreloadselect = 1;
			}
			else {
				blockiosreloadselect -= 1;
			}	
		}		
	}

	if (currentmenu == WIIMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{	
		if (rootmenu_item == 0){
			if (langselect == 11){
				langselect = 0;
			}
			else {
				langselect += 1;
			}	
		}

		if (rootmenu_item == 1){
			if (videoselect == 8){
				videoselect = 0;
			}
			else {
				videoselect += 1;
			}	
		}

		if (rootmenu_item == 2){
			if (videopatchselect == 7){
				videopatchselect = 0;
			}
			else {
				videopatchselect += 1;
			}	
		}

		if (rootmenu_item == 3){
			if (viselect == 1){
				viselect = 0;
			}
			else {
				viselect += 1;
			}	
		}

		if (rootmenu_item == 4){
			if (countrystringselect == 1){
				countrystringselect = 0;
			}
			else {
				countrystringselect += 1;
			}	
		}

		if (rootmenu_item == 5){
			if (alternativedolselect == 4){
				alternativedolselect = 0;
			}
			else {
				alternativedolselect += 1;
			}	
		}		

		if (rootmenu_item == 6){
			if (patchselect == 1){
				patchselect = 0;
			}
			else {
				patchselect += 1;
			}	
		}		

		if (rootmenu_item == 7){
			if( blockiosreloadselect == 1){
				blockiosreloadselect = 0;
			}
			else {
				blockiosreloadselect += 1;
			}	
		}		
	}

	if (currentmenu == GCMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{	
		if(rootmenu_item == 0){
			if(gamecubemodeselect == 0){
				gamecubemodeselect = 2;
			}
			else {
				gamecubemodeselect -= 1;
			}	
		}		
		
		if(rootmenu_item == 1){
			if(gc_videoselect == 0){
				gc_videoselect = 7;
			}
			else {
				gc_videoselect -= 1;
			}	
		}
		
		if(rootmenu_item == 2){
			if(gc_videopatchselect == 0){
				gc_videopatchselect = 3;
			}
			else {
				gc_videopatchselect -= 1;
			}	
		}	

		if(rootmenu_item == 3){
			if(gcreloaderselect == 0){
				gcreloaderselect = 4;
			}
			else {
				gcreloaderselect -= 1;
			}	
		}	

		if(rootmenu_item == 4){
			if(gchighplugin == 0){
				gchighplugin = 3;
			}
			else {
				gchighplugin -= 1;
			}	
		}
		
		if(rootmenu_item == 5){
			if(audiostatusrequestfixselect == 0){
				audiostatusrequestfixselect = 4;
			}
			else {
				audiostatusrequestfixselect -= 1;
			}	
		}

		if(rootmenu_item == 6){
			if(patchedMIOSselect == 0){
				patchedMIOSselect = 2;
			}
			else {
				patchedMIOSselect -= 1;
			}	
		}	

		/*
		if(rootmenu_item == 6){
			if(gcplugindebug == 0){
				gcplugindebug = 1;
			}
			else {
				gcplugindebug -= 1;
			}	
		}
		*/
	}

	if (currentmenu == GCMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{	
		if(rootmenu_item == 0){
			if(gamecubemodeselect == 2){
				gamecubemodeselect = 0;
			}
			else {
				gamecubemodeselect += 1;
			}	
		}		

		if(rootmenu_item == 1){
			if(gc_videoselect == 7){
				gc_videoselect = 0;
			}
			else {
				gc_videoselect += 1;
			}	
		}

		if(rootmenu_item == 2){
			if(gc_videopatchselect == 3){
				gc_videopatchselect = 0;
			}
			else {
				gc_videopatchselect += 1;
			}	
		}

		if(rootmenu_item == 3){
			if(gcreloaderselect == 4){
				gcreloaderselect = 0;
			}
			else {
				gcreloaderselect += 1;
			}	
		}

		if(rootmenu_item == 4){
			if(gchighplugin == 3){
				gchighplugin = 0;
			}
			else {
				gchighplugin += 1;
			}	
		}		
				
		if(rootmenu_item == 5){
			if(audiostatusrequestfixselect == 4){
				audiostatusrequestfixselect = 0;
			}
			else {
				audiostatusrequestfixselect += 1;
			}	
		}		

		if(rootmenu_item == 6){
			if(patchedMIOSselect == 2){
				patchedMIOSselect = 0;
			}
			else {
				patchedMIOSselect += 1;
			}	
		}	
		/*
		if(rootmenu_item == 6){
			if(gcplugindebug == 1){
				gcplugindebug = 0;
			}
			else {
				gcplugindebug += 1;
			}	
		}	
		*/
	}

	if (currentmenu == OCARINAMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{
		if (rootmenu_item == 0)
		{
			if(wiihookselect == 0){
				wiihookselect = 7;
			}
			else {
				wiihookselect -= 1;
			}	
		}

		if (rootmenu_item == 1)
		{
			if(wii2ndhookselect == 0){
				wii2ndhookselect = 8;
			}
			else {
				wii2ndhookselect -= 1;
			}	
		}

		if(rootmenu_item == 2){
			if(wiiocarinaselect == 0){
				wiiocarinaselect = 1;
			}
			else {
				wiiocarinaselect -= 1;
			}	
		}
		
		if(rootmenu_item == 3){
			if(wiidebuggerselect == 0){
				wiidebuggerselect = 2;
			}
			else {
				wiidebuggerselect -= 1;
			}	
		}

		if (rootmenu_item == 4){
			if(gchookselect == 0){
				gchookselect = 7;
			}
			else {
				gchookselect -= 1;
			}	
		}

#ifdef DONTTELL
		if (rootmenu_item == 5){
			if(gc2ndhookselect == 0){
				gc2ndhookselect = 8;
			}
			else {
				gc2ndhookselect -= 1;
			}	
		}
#endif			

		if(rootmenu_item == 6){
			if(gcocarinaselect == 0){
				gcocarinaselect = 1;
			}
			else {
				gcocarinaselect -= 1;
			}	
		}
		
		if(rootmenu_item == 7){
			if(gcdebuggerselect == 0){
				gcdebuggerselect = 2;
			}
			else {
				gcdebuggerselect -= 1;
			}	
		}
			
	}

	if (currentmenu == OCARINAMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{
		if(rootmenu_item == 0)
		{
			if(wiihookselect == 7){
				wiihookselect = 0;
			}
			else {
				wiihookselect += 1;
			}	
		}

		if(rootmenu_item == 1)
		{
			if(wii2ndhookselect == 8){
				wii2ndhookselect = 0;
			}
			else {
				wii2ndhookselect += 1;
			}	
		}

		if(rootmenu_item == 2){
			if(wiiocarinaselect == 1){
				wiiocarinaselect = 0;
			}
			else {
				wiiocarinaselect += 1;
			}	
		}

		if(rootmenu_item == 3)
		{
			if(wiidebuggerselect == 2){
				wiidebuggerselect = 0;
			}
			else {
				wiidebuggerselect += 1;
			}	
		}
		
		if(rootmenu_item == 4){
			if(gchookselect == 7){
				gchookselect = 0;
			}
			else {
				gchookselect += 1;
			}	
		}
#ifdef DONTTELL
		if (rootmenu_item == 5){
			if(gc2ndhookselect == 8){
				gc2ndhookselect = 0;
			}
			else {
				gc2ndhookselect += 1;
			}	
		}
#endif		

		if(rootmenu_item == 6){
			if(gcocarinaselect == 1){
				gcocarinaselect = 0;
			}
			else {
				gcocarinaselect += 1;
			}	
		}

		if(rootmenu_item == 7)
		{
			if(gcdebuggerselect == 2){
				gcdebuggerselect = 0;
			}
			else {
				gcdebuggerselect += 1;
			}	
		}
	}

	/* REBOOT menu left */
	if (currentmenu == REBOOTMENU && ((pressed & WPAD_BUTTON_LEFT) || (pressed & WPAD_CLASSIC_BUTTON_LEFT) || (pressedGC & PAD_BUTTON_LEFT)))
	{
		if(rootmenu_item == 0){
			if(recoveryselect == 0){
				recoveryselect = 1;
			}
			else {
				recoveryselect -= 1;
			}	
		}

		if(rootmenu_item == 1){
			if(regionfreeselect == 0){
				regionfreeselect = 1;
			}
			else {
				regionfreeselect -= 1;
			}	
		}

		if(rootmenu_item == 2){
			if(nocopyselect == 0){
				nocopyselect = 1;
			}
			else {
				nocopyselect -= 1;
			}	
		}

		if(rootmenu_item == 3){
			if(buttonskipselect == 0){
				buttonskipselect = 1;
			}
			else {
				buttonskipselect -= 1;
			}	
		}

		if(rootmenu_item == 4){
			if(skipupdateselect == 0){
				skipupdateselect = 1;
			}
			else {
				skipupdateselect -= 1;
			}	
		}

		if(rootmenu_item == 5){
			if(bendiosreloadselect == 0){
				bendiosreloadselect = 1;
			}
			else {
				bendiosreloadselect -= 1;
			}	
		}

		if(rootmenu_item == 6){
			if(preloaderselect == 0){
				preloaderselect = 1;
			}
			else {
				preloaderselect -= 1;
			}	
		}		
	}

	/* REBOOT menu right */
	if (currentmenu == REBOOTMENU && ((pressed & WPAD_BUTTON_RIGHT) || (pressed & WPAD_CLASSIC_BUTTON_RIGHT) || (pressedGC & PAD_BUTTON_RIGHT)))
	{
		if(rootmenu_item == 0){
			if(recoveryselect == 1){
				recoveryselect = 0;
			}
			else {
				recoveryselect += 1;
			}	
		}

		if(rootmenu_item == 1){
			if(regionfreeselect == 1){
				regionfreeselect = 0;
			}
			else {
				regionfreeselect += 1;
			}	
		}

		if(rootmenu_item == 2){
			if(nocopyselect == 1){
				nocopyselect = 0;
			}
			else {
				nocopyselect += 1;
			}	
		}

		if(rootmenu_item == 3){
			if(buttonskipselect == 1){
				buttonskipselect = 0;
			}
			else {
				buttonskipselect += 1;
			}	
		}
		
		if(rootmenu_item == 4){
			if(skipupdateselect == 1){
				skipupdateselect = 0;
			}
			else {
				skipupdateselect += 1;
			}	
		}		

		if(rootmenu_item == 5){
			if(bendiosreloadselect == 1){
				bendiosreloadselect = 0;
			}
			else {
				bendiosreloadselect += 1;
			}	
		}		

		if(rootmenu_item == 6){
			if(preloaderselect == 1){
				preloaderselect = 0;
			}
			else {
				preloaderselect += 1;
			}	
		}			
	}

	/* Wiimote Pad UP */

	if (menufreeze == 0 && ((pressed & WPAD_BUTTON_UP) || (pressed & WPAD_CLASSIC_BUTTON_UP) || (pressedGC & PAD_BUTTON_UP)))
	{
		rootmenu_item -= 1;
		if (rootmenu_item < 0)
		{
			switch (currentmenu)
			{
				case ROOTMENU:
					rootmenu_item = root_itemcount-1-1+showrebooterselect;
					break;
				case OPTIONSMENU:
					rootmenu_item = options_itemcount-1;
					break;
				case GENERALMENU:
					rootmenu_item = general_itemcount-1;
					break;
				case WIIMENU:
					rootmenu_item = wii_itemcount-1;
					break;			
				case GCMENU:
					rootmenu_item = gc_itemcount-1;
					break;			
				case OCARINAMENU:
					rootmenu_item = ocarina_itemcount-1;
					break;			
				case REBOOTMENU:
					rootmenu_item = rebooter_itemcount-1;
					break;			
			}		
		}
		
		if (CIOS_VERSION != IOS_GetVersion() || (IOS_GetVersion() >= 100 /*&& IOS_GetRevision() >= 9*/))
		{
			if (currentmenu == ROOTMENU && (rootmenu_item == 2 || rootmenu_item == root_itemcount - 2-1+showrebooterselect))
			{
				rootmenu_item -= 1;
			}
		} else
		{
			if (currentmenu == ROOTMENU && rootmenu_item == 2)
			{
				rootmenu_item -= 2;
			}
			if (currentmenu == ROOTMENU && rootmenu_item == root_itemcount - 2-1+showrebooterselect)
			{
				rootmenu_item -= 1;
			}
		}

		if (currentmenu == OPTIONSMENU && (rootmenu_item == options_itemcount - 2))
		{
			rootmenu_item -= 1;
		}

		if (currentmenu == GENERALMENU && (rootmenu_item == general_itemcount - 2))
		{
			rootmenu_item -= 1;
		}

		if (currentmenu == WIIMENU && (rootmenu_item == wii_itemcount - 2))
		{
			rootmenu_item -= 1;
		}

		if (currentmenu == GCMENU && (rootmenu_item == gc_itemcount - 2))
		{
			rootmenu_item -= 1;
		}

		if (currentmenu == OCARINAMENU && (rootmenu_item == ocarina_itemcount - 2))
		{
			rootmenu_item -= 1;
		}

		if (currentmenu == REBOOTMENU && (rootmenu_item == rebooter_itemcount - 2))
		{
			rootmenu_item -= 1;
		}
	}

	/* Wiimote Pad Down */
	if (menufreeze == 0 && ((pressed & WPAD_BUTTON_DOWN) || (pressed & WPAD_CLASSIC_BUTTON_DOWN) || (pressedGC & PAD_BUTTON_DOWN)))
	{
		rootmenu_item += 1;
		if (	(currentmenu == ROOTMENU && rootmenu_item >= root_itemcount-1+showrebooterselect)
		||	 	(currentmenu == OPTIONSMENU && rootmenu_item >= options_itemcount)
		||	 	(currentmenu == GENERALMENU && rootmenu_item >= general_itemcount)
		||	 	(currentmenu == WIIMENU && rootmenu_item >= wii_itemcount)
		||	 	(currentmenu == GCMENU && rootmenu_item >= gc_itemcount)
		||	 	(currentmenu == OCARINAMENU && rootmenu_item >= ocarina_itemcount)
		||	 	(currentmenu == REBOOTMENU && rootmenu_item >= rebooter_itemcount) )
		{
			rootmenu_item = 0;
		}
		
		if (CIOS_VERSION != IOS_GetVersion() || (IOS_GetVersion() >= 100 /*&& IOS_GetRevision() >= 9*/))
		{
			if (currentmenu == ROOTMENU && (rootmenu_item == 2 || rootmenu_item == root_itemcount - 2-1+showrebooterselect))
			{
				rootmenu_item += 1;
			}
		} else
		{
			if (currentmenu == ROOTMENU && rootmenu_item == 1)
			{
				rootmenu_item += 2;
			}
			if (currentmenu == ROOTMENU && rootmenu_item == root_itemcount - 2-1+showrebooterselect)
			{
				rootmenu_item += 1;
			}
		}

		if (currentmenu == OPTIONSMENU && (rootmenu_item == options_itemcount - 2))
		{
			rootmenu_item += 1;
		}

		if (currentmenu == GENERALMENU && (rootmenu_item == general_itemcount - 2))
		{
			rootmenu_item += 1;
		}

		if (currentmenu == WIIMENU && (rootmenu_item == wii_itemcount - 2))
		{
			rootmenu_item += 1;
		}

		if (currentmenu == GCMENU && (rootmenu_item == gc_itemcount - 2))
		{
			rootmenu_item += 1;
		}

		if (currentmenu == OCARINAMENU && (rootmenu_item == ocarina_itemcount - 2))
		{
			rootmenu_item += 1;
		}

		if (currentmenu == REBOOTMENU && (rootmenu_item == rebooter_itemcount - 2))
		{
			rootmenu_item += 1;
		}
	}
}

u32 CvtRGB (u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2)
{
  int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
 
  y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
  cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
  cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;
 
  y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
  cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
  cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;
 
  cb = (cb1 + cb2) >> 1;
  cr = (cr1 + cr2) >> 1;
 
  return (y1 << 24) | (cb << 16) | (y2 << 8) | cr;
}

/* highlight selected item */
void drawselected(u32 menuidpos)
{
	u32 x;
	u32 y;
	u32 startpos = startypos+1;
	if (currentmenu != ROOTMENU)
	{
		startpos = startpos + 24;
	} 
	u32 yscreen = startpos + (menuidpos * fontheight);
	u32 color = CvtRGB(44, 120, 8, 44, 120, 8);
	
	for (y = yscreen; y < yscreen + fontheight; y++)
	{
		for(x = innermenu_start; x < innermenu_end; x++)
		{
			if(xfb[x + screenwidth * y] == 0x00800080)
			{
				xfb[x + screenwidth * y] = color;
			}
		}
	}
}

static void __blitmenu (char items[][27], u32 maxitems, u32 shift, bool centered)
{
	int i;
	int y = startypos + shift * 24;
	
	for(i=0;i<maxitems;i++)
	{
		if (centered)
		{
			write_font_centered(42, 614, y, (char *) items[i]);
		} else
		{
			write_font(160, y,true, (char *) items[i]);
		}
		y += 24;
	}
}

void drawmenu(u32 menuid)
{
	char rootitems1[root_itemcount][27] = {{""}, {""}, {""}, {"     Options"}, {"     Use Rebooter"},{"     Credits"},{""}, {"     Exit NeoGamma"}};
	char rootitems2[root_itemcount-1][27] = {{""}, {""}, {""}, {"Options  "}, {"Credits  "},{""}, {"Exit NeoGamma"}};

	char optionitems[options_itemcount][27] = {{"  General Options"}, {"  Wii Options"}, {"  GC options"}, {"  Ocarina/Wiird"}, {"  Rebooter Options"}, {""}, {"  Return to Menu"}};

	char generalitems[general_itemcount][27] = {{"IOS for games"}, {"Storage device:"}, {"Show Rebooter:"}, {"Save Config:"}, {""}, {"Return to Menu"}};
	char wiiitems[wii_itemcount][27] = {{"Boot Lang:"}, {"Video Mode:"}, {"Patch Video:"}, {"VIDTV Patch:"}, {"Patch Country Str.:"}, {"Altern. .dol:"}, {"Search patches:"}, {"Block IOS Reload:"}, {""}, {"Return to Menu"}};
	
	char gcitems[gc_itemcount][27] = {{"Gamecube Mode:"}, {"Video Mode:"}, {"Patch Video:"}, {"Reloader:"}, {"Backup plugin:"}, {"Audio fix:"}, {"Patched MIOS:"}, {""}, {"Return to Menu"}};

#ifdef DONTTELL
	char ocarinaitems[ocarina_itemcount][27] = {{"Wii Hook Type:"}, {"Wii 2nd Hook:"}, {"Wii Ocarina:"},{"Wii Debugger:"}, {"GC Hook Type:"},{"GC 2nd Hook: "}, {"GC Ocarina:"},{"GC Debugger:"}, {""}, {"Return to Menu"}};
#else
	char ocarinaitems[ocarina_itemcount][27] = {{"Wii Hook Type:"}, {"Wii 2nd Hook:"},{"Wii Ocarina:"},{"Wii Debugger:"}, {"GC Hook Type:"},{"GC Ocarina:"},{"GC Debugger:"}, {""}, {"Return to Menu"}};
#endif

	char rebooteritems[rebooter_itemcount][27] = {{"Recovery Mode:"},{"Region Free:"},{"Remove Copy Flags:"},{"Button Skip:"},{"Skip Updates:"}, {"Force cIOS:"},{"Preloader Support:"}, {""},{"Return to Menu"}};

	char aboutitems[about_itemcount][27] = {{"Gamma Loader - WiiGator"}, {"Modifications - WiiPower"},{"Graphics - Empyr69er"}, {""},{"Return to Menu"}};

	switch (menuid)
	{
		case ROOTMENU:
			if (showrebooterselect == 1)
			{
				__blitmenu(&rootitems1[0], root_itemcount, 0, false);
			} else
			{
				__blitmenu(&rootitems2[0], root_itemcount-1, 0, true);
			}
			
			//if (Gecko_OS_MOD == false)
			//{
				if (wbfsdevice > 0 || Sneek_Mode == true)
				{
					write_font_centered(42, 614, startypos + 0*24, "%s", getGameTitle(gameselected));
				} else
				{
					write_font_centered(42, 614, startypos + 0*24, "Launch Game on DVD");
				}
				if (Sneek_Mode == false)
				{
					write_font_centered(42, 614, startypos + 1*24, "%s", wbfs_options[wbfsdevice]);
				} else
				{
					write_font_centered(42, 614, startypos + 1*24, "Load Games via Sneek+DI");
				}
			//} else
			//{
			//	write_font(160, startypos + 0*24,true, "  Launch Game on DVD");
			//}
		break;

		case ABOUTMENU:
			__blitmenu(&aboutitems[0], about_itemcount, 1, false);
		break;

		case OPTIONSMENU:
			__blitmenu(&optionitems[0], options_itemcount, 1, false);
		break;

		case GENERALMENU:
			__blitmenu(&generalitems[0], general_itemcount, 1, false);
			if (Sneek_Mode == false)
			{			
				write_font(395, startypos + 1*24,true, "%s", correctios_options[usecorrectiosoption]);
				write_font(395, startypos + 2*24,true, "%s", storage_options[storageselect]);
			} else
			{
				write_font(395, startypos + 1*24,true, "correct IOS");
				write_font(395, startypos + 2*24,true, "NAND");
			}
			write_font(395, startypos + 3*24,true, "%s", bool_options[showrebooterselect]);
			write_font(395, startypos + 4*24,true, "%s", bool_options[configselect]);
		break;

		case WIIMENU:
			__blitmenu(&wiiitems[0], wii_itemcount, 1, false);
			write_font(410, startypos + 1*24,true, "%s", languages[langselect]);
			write_font(410, startypos + 2*24,true, "%s", video_options[videoselect]);
			write_font(410, startypos + 3*24,true, "%s", videopatch_options[videopatchselect]);
			write_font(410, startypos + 4*24,true, "%s", bool_options[viselect]);
			write_font(410, startypos + 5*24,true, "%s", bool_options[countrystringselect]);
			write_font(410, startypos + 6*24,true, "%s", alternativedol_options[alternativedolselect]);
			write_font(410, startypos + 7*24,true, "%s", bool_options[patchselect]);
			
			if (Sneek_Mode == false)
			{			
				write_font(410, startypos + 8*24,true, "%s", bool_options[blockiosreloadselect]);
			} else
			{
				write_font(410, startypos + 8*24,true, "Yes");
			}

		break;

		case GCMENU:
			__blitmenu(&gcitems[0], gc_itemcount, 1, false);
			write_font(410, startypos + 1*24,true, "%s", gamecubemode_options[gamecubemodeselect]);
			write_font(410, startypos + 2*24,true, "%s", gc_video_options[gc_videoselect]);
			if (gamecubemodeselect == 1)
			{
				write_font(410, startypos + 3*24,true, "%s", videopatch_options[gc_videopatchselect]);
				write_font(410, startypos + 4*24,true, "%s", reloader_options[gcreloaderselect]);
				write_font(410, startypos + 5*24,true, "%s", highplugin_options[gchighplugin]);				
				write_font(410, startypos + 6*24,true, "%s", audiofix_options[audiostatusrequestfixselect]);
				write_font(410, startypos + 7*24,true, "%s", patchedMIOS_options[patchedMIOSselect]);
			} else
			{
				write_font(410, startypos + 3*24,false, "%s", videopatch_options[gc_videopatchselect]);
				write_font(410, startypos + 4*24,false, "%s", reloader_options[gcreloaderselect]);
				write_font(410, startypos + 5*24,false, "%s", highplugin_options[gchighplugin]);
				write_font(410, startypos + 6*24,false, "%s", audiofix_options[audiostatusrequestfixselect]);
				write_font(410, startypos + 7*24,false, "%s", patchedMIOS_options[patchedMIOSselect]);
			}
		break;
		
		case OCARINAMENU:
			__blitmenu(&ocarinaitems[0], ocarina_itemcount, 1, false);
			write_font(375, startypos + 1*24,true, "%s", wii_hook_options[wiihookselect]);
			write_font(375, startypos + 2*24,true, "%s", wii_hook2_options[wii2ndhookselect]);
			
			if (wiihookselect != 0 || (wii2ndhookselect != 0 && wii2ndhookselect != 8))
			{
				write_font(375, startypos + 3*24,true, "%s", bool_options[wiiocarinaselect]);
				write_font(375, startypos + 4*24,true, "%s", debugger_options[wiidebuggerselect]);
			} else
			{
				write_font(375, startypos + 3*24,false, "%s", bool_options[wiiocarinaselect]);
				write_font(375, startypos + 4*24,false, "%s", debugger_options[wiidebuggerselect]);
			}
			if (gamecubemodeselect == 1)
			{
				write_font(375, startypos + 5*24,true, "%s", gc_hook_options[gchookselect]);
#ifdef DONTTELL
				write_font(375, startypos + 6*24,true, "%s", gc_hook2_options[gc2ndhookselect]);
				if (gchookselect != 0 || (gc2ndhookselect != 0 && gc2ndhookselect != 8))
#else				
				if (gchookselect != 0)
#endif
				{
					write_font(375, startypos + 7*24,true, "%s", bool_options[gcocarinaselect]);
					write_font(375, startypos + 8*24,true, "%s", debugger_options[gcdebuggerselect]);
				} else
				{
					write_font(375, startypos + 7*24,false, "%s", bool_options[gcocarinaselect]);
					write_font(375, startypos + 8*24,false, "%s", debugger_options[gcdebuggerselect]);
				}
			} else
			{
				write_font(375, startypos + 5*24,false, "%s", gc_hook_options[gchookselect]);
#ifdef DONTTELL
				write_font(375, startypos + 6*24,false, "%s", gc_hook2_options[gc2ndhookselect]);
#endif
				write_font(375, startypos + 7*24,false, "%s", bool_options[gcocarinaselect]);
				write_font(375, startypos + 8*24,false, "%s", debugger_options[gcdebuggerselect]);
			}
		break;

		case REBOOTMENU:
			__blitmenu(&rebooteritems[0], rebooter_itemcount, 1, false);
			write_font(455, startypos + 1*24,true, "%s", bool_options[recoveryselect]);
			write_font(455, startypos + 2*24,true, "%s", bool_options[regionfreeselect]);
			write_font(455, startypos + 3*24,true, "%s", bool_options[nocopyselect]);
			write_font(455, startypos + 4*24,true, "%s", bool_options[buttonskipselect]);
			write_font(455, startypos + 5*24,true, "%s", bool_options[skipupdateselect]);
			write_font(455, startypos + 6*24,true, "%s", bool_options[bendiosreloadselect]);
			write_font(455, startypos + 7*24,true, "%s", bool_options[preloaderselect]);		
		break;

	}	
}

