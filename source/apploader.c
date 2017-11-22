/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *  Copyright (C) 2008 WiiGator
 *  Copyright (C) 2009 WiiPower
 *
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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <gccore.h>
#include <ogcsys.h>
#include <sys/unistd.h>
#include <wiiuse/wpad.h>
#include <ogc/lwp_watchdog.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>
#include <time.h>
#include <ogc/system.h>

#include "tools.h"
#include "apploader.h"
#include "patchcode.h"
#include "font.h"
#include "dvd_broadway.h"
#include "patchcode.h"
#include "geckomenu.h"

#include "libcios.h"
#include "easywbfs.h"
#include "wbfs.h"
#include "config.h"
#include "Toy.h"
#include "Toy_video.h"
#include "dol.h"
#include "tools.h"
#include "codes.h"
#include "wiip.h"
#include "storage.h"
#include "dolmenu.h"

#include "gcbackuplauncher_dol.h"

#define appbuffer 0x80100000
#define BC 0x0000000100000100ULL
#define MIOS 0x0000000100000101ULL
#define SIG_RSA_2048 0x00010001
#define SIG_RSA_4096 0x00010000
#define SECTOR_SIZE 0x800

syssram* __SYS_LockSram();
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);

void printadditionalerrorinfo(u32 error)
{
	wait(2);
	
	if (error == 0x03023A00)
	{
		print_status("DVD+R/bad burn/DL problem?");
		// This error happens on almost everything, DVD+R without bitsetting with and without cIOS, bad burns and
		// on drives that can't read DL. Only no cIOS + DVD-R/DVD+R with bitsetting results in another error(0x053000).
	} else
	{
		if (error>>24)
		{
			switch(error>>24)
			{
				case 1:
					print_status("No disc inserted");
				break;
				case 2:
					print_status("No disc/Disc changed");
				break;
				case 3:
					print_status("No disc");
				break;
				case 4:
					print_status("Motor off");
				break;
				case 5:
					print_status("Disc not initialized");
			}
		} else
		{
			switch(error&0xFFFFFF)
			{
				case 0:
				break;
				case 0x020400:
					print_status("Motor Stopped");
				break;
				case 0x020401:
					print_status("Disc ID not read");
				break;
				case 0x023A00:
					print_status("Medium not present / Cover opened");
				break;
				case 0x030200:
					print_status("No Seek complete");
				break;
				case 0x031100:
					print_status("Unrecovered read error");
				break;
				case 0x040800:
					print_status("Transfer protocol error");
				break;
				case 0x052000:
					print_status("Invalid command operation code");
				break;
				case 0x052001:
					print_status("Audio Buffer not set");
				break;
				case 0x052100:
					print_status("Logical block address out of range");
				break;
				case 0x052400:
					print_status("Invalid Field in command packet");
				break;
				case 0x052401:
					print_status("Invalid audio command");
				break;
				case 0x052402:
					print_status("Configuration out of permitted period");
				break;
				case 0x053000:
					print_status("DVD-R + no cIOS!");
				break;
				case 0x053100:
					print_status("Wrong Read Type"); //?
				break;
				case 0x056300:
					print_status("End of user area encountered on this track");
				break;
				case 0x062800:
					print_status("Medium may have changed");
				break;
				case 0x0B5A01:
					print_status("Operator medium removal request");
			}
		}
	}

	if (drivedate == 1)
	{
		wait(3);
		print_status("No drive connected?");
	}
	
	if (wbfsdevice == 0 && Sneek_Mode == false)		// Loading a disc
	{
		// Latest known safe drive 20070213
		if (drivedate > 0x20070213 && drivedate < 0x20080714)
		{
			wait(3);
			print_status("Drive date: %x", drivedate);
			wait(5);
			print_status("Unknown drive please report");
			wait(5);
			print_status("Drive date: %x", drivedate);
		} else
		// Everything newer than 20080714 should be backup resistant
		if (drivedate >= 0x20080714)
		{
			wait(3);
			print_status("Drive date: %x", drivedate);
			wait(5);
			print_status("Drive too new for DVD-Rs :-(");
		}
	}	
}


#define DVD_CHECK() \
	do { \
		char err_buffer[64]; \
		if (ret < 0) { \
			snprintf(err_buffer, sizeof(err_buffer), "DVD Ioctl Err(%d), %s", __LINE__, dvderror); \
			print_status(err_buffer); \
			wait(5); \
			return -1; \
		} \
		t = time(NULL) + 30; \
		while (dvddone == 0) \
		{ \
			if (time(NULL) >= t) \
			{ \
				print_status("DVD Timeout, %s", dvderror); \
				wait(5); \
				return -1; \
			} \
		} \
		if (dvddone != 1) { \
			u32 errorcode = bwDVD_LowRequestError(); \
			snprintf(err_buffer, sizeof(err_buffer), "DVD Err(%08x), %s", errorcode, dvderror); \
			print_status(err_buffer); \
			printadditionalerrorinfo(errorcode); \
			wait(5); \
			return -1; \
		} \
	} while(0)

char dvderror[64];

char dolname[32];
u32 dolparameter;
static char gameidbuffer[8];
bool Country_Strings_Patched;
bool wip_patch_applied;

extern GXRModeObj TVEurgb60Hz480Int;
static GXRModeObj *rmode = NULL;

static vu32 dvddone = 0;
static u32 Video_Mode;
static time_t t;
bool hookpatched;
u32 dolcount;
u32 dolindex[10];

extern u32 *xfb;

static u32 memsetloaderbuffer[] =
{	
	0x3FC00000, 0x63DE0000,
	0x3FA00000, 0x63BD0000,
	0x3B800000, 0x3B600000,
	0x7F3EE850, 0x7F9ED92E,
	0x7C1ED86C, 0x7C0004AC,
	0x7C1EDFAC, 0x7C19D800,
	0x3B7B0004, 0x40A2FFE8,
	0x3FA00000, 0x63BD0000,
	0x7FA903A6, 0x4E800420
};

struct _toc
{
	u32 bootInfoCnt;
	u32 partInfoOff;
};

struct _pinfo
{
	u32 offset;
	u32 len;
};

//extern int findcodes(char* codes);
extern s32 __IOS_InitializeSubsystems();

static dvddiskid *g_diskID = (dvddiskid*)0x80000000;
static char _gameTocBuffer[0x20] ATTRIBUTE_ALIGN(32);
static char _gamePartInfo[0x20] ATTRIBUTE_ALIGN(32);
static char _gameTmdBuffer[18944] ATTRIBUTE_ALIGN(32);
static u32 *__dvd_Tmd = NULL;
static struct _toc *__dvd_gameToc = NULL;
static struct _pinfo *__dvd_partInfo = NULL;
static struct _pinfo *__dvd_bootGameInfo = NULL;
static tikview view ATTRIBUTE_ALIGN(32);


char video_options2[9][9] =
{{"Wii"},
{"Disc"},
{"NTSC480i"},
{"NTSC480p"},
{"PAL480i"},
{"PAL480p"},
{"PAL576i"},
{"MPAL480i"},
{"MPAL480p"}};

char bool_options2[2][6] = 
{{"No"},
{"Yes"}};

char videopatch_options2[4][8] =
{{"No"},
{"Normal"},
{"More"},
{"All"}};

static void __dvd_readidcb(s32 result)
{
	dvddone = result;
}

void set_videomode()
{	
	/* Set video mode to PAL or NTSC */
	*(u32*)0x800000CC = Video_Mode;

	// Overwrite all progressive video modes as they are broken in libogc
	if (videomode_interlaced(rmode) == 0)
	{
		rmode = &TVNtsc480Prog;
	}
	VIDEO_Configure(rmode);
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(xfb);

	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}


static void __setappvideo(char *discid)
{
	// Get rmode and Video_Mode for Wii Video Mode first
	u32 tvmode = CONF_GetVideo();

	// Attention: This returns &TVNtsc480Prog for all progressive video modes
    rmode = VIDEO_GetPreferredMode(0);

	switch (tvmode) 
	{
		case CONF_VIDEO_PAL:
			if (CONF_GetEuRGB60() > 0) 
			{
				Video_Mode = 5;
			}
			else 
			{
				Video_Mode = 1;
			}
		break;

		case CONF_VIDEO_MPAL:
			Video_Mode = 4;
		break;

		case CONF_VIDEO_NTSC:
		default:
			Video_Mode = 0;
		break;
	}

	// Overwrite rmode and Video_Mode when disc region video mode is selected and Wii region doesn't match disc region
	if (videoselect == 1)
	{
		switch (discid[3]) 
		{
			case 'P':	// PAL
			case 'D':	// German
			case 'F':	// French
			case 'S':	// Spanish
			case 'I':	// Italian
			case 'L':	// Japanese Import to PAL
			case 'M':	// American Import to PAL
			case 'X':	// PAL other languages?
			case 'Y':	// PAL other languages?
				if (CONF_GetVideo() != CONF_VIDEO_PAL)
				{
					Video_Mode = 5;

					if (CONF_GetProgressiveScan() > 0 && VIDEO_HaveComponentCable())
					{
						rmode = &TVNtsc480Prog; // This seems to be correct!
					}
					else
					{
						rmode = &TVEurgb60Hz480IntDf;
					}				
				}
			break;

			case 'E':	// NTSC-U
			case 'J':	// NTSC-J
			case 'N':	// Japanese Import to USA
			default:
				if (CONF_GetVideo() != CONF_VIDEO_NTSC)
				{
					Video_Mode = 0;

					if (CONF_GetProgressiveScan() > 0 && VIDEO_HaveComponentCable())
					{
						rmode = &TVNtsc480Prog;
					}
					else
					{
						rmode = &TVNtsc480IntDf;
					}				
				}
			break;	
		}
	}
	
	if (videoselect >= 2)
	{
		if (videoselect == 2)
		{
			rmode = &TVNtsc480IntDf;
		}
		if (videoselect == 3)
		{
			rmode = &TVNtsc480Prog;
		}
		if (videoselect == 4)
		{
			rmode = &TVEurgb60Hz480IntDf;
		}
		if (videoselect == 5)
		{
			rmode = &TVEurgb60Hz480Prog;
		}
		if (videoselect == 6)
		{
			rmode = &TVPal528IntDf;
		}
		if (videoselect == 7)
		{
			rmode = &TVMpal480IntDf;
		}
		if (videoselect == 8)
		{
			rmode = &TVMpal480Prog;
		}
		Video_Mode = (rmode->viTVMode) >> 2;
	}
}

static void __setappvideo_GC(char *discid)
{
	if (gc_videoselect == 0)
	{
		if  (CONF_GetVideo() == CONF_VIDEO_NTSC)
		{
			rmode = &TVNtsc480IntDf;
			Video_Mode = 0;
		} else
		{
			if (discid[3] == 'E' || discid[3] == 'J' || discid[3] == 'N')
			{
				rmode = &TVEurgb60Hz480IntDf;
				Video_Mode = 5;
			} else
			{
				rmode = &TVPal528IntDf;
				Video_Mode = 1;
			}
		}
	}

	if (gc_videoselect == 1)
	{
		switch (discid[3]) 
		{
			case 'P':	// PAL
			case 'D':	// German
			case 'F':	// French
			case 'S':	// Spanish
			case 'I':	// Italian
			case 'L':	// Japanese Import to PAL
			case 'M':	// American Import to PAL
			case 'X':	// PAL other languages?
			case 'Y':	// PAL other languages?
				rmode = &TVPal528IntDf;
				Video_Mode = 1;
			break;

			case 'E':	// NTSC-U
			case 'J':	// NTSC-J
			case 'N':	// Japanese Import to USA
			default:
				rmode = &TVNtsc480IntDf;
				Video_Mode = 0;
			break;
		}
	}
	
	if (gc_videoselect >= 2)
	{
		if (gc_videoselect == 2)
		{
			rmode = &TVNtsc480IntDf;
		}
		if (gc_videoselect == 3)
		{
			rmode = &TVNtsc480Prog;
		}
		if (gc_videoselect == 4)
		{
			if (discid[3] == 'E' || discid[3] == 'J' || discid[3] == 'N')
			{
				rmode = &TVEurgb60Hz480IntDf;
			} else
			{
				rmode = &TVPal528IntDf;
			}
		}
		if (gc_videoselect == 5)
		{
			rmode = &TVPal528IntDf;
		}
		if (gc_videoselect == 6)
		{
			rmode = &TVEurgb60Hz480IntDf;
		}
		if (gc_videoselect == 7)
		{
			rmode = &TVEurgb60Hz480Prog;
		}
		Video_Mode = (rmode->viTVMode) >> 2;
	}
	
	syssram *sram;
	sram = __SYS_LockSram();

	if (Video_Mode == 0)
	{
		sram->flags = sram->flags & ~(1 << 0);	// Clear bit 0 to set the video mode to NTSC
	} else
	{
		sram->flags = sram->flags |  (1 << 0);	// Set bit 0 to set the video mode to PAL
	}
	__SYS_UnlockSram(1); // 1 -> write changes
	while(!__SYS_SyncSram());
}


void nothing()
{
}

s32 ocarinarebooter()
{
	drive_state = 0;
	s32 ret = 0;

	if ((wiiocarinaselect != 0 || wiidebuggerselect != 0) && wiihookselect != 0)
	{
		if (wiiocarinaselect != 0)
		{
			if (storageselect == 0)
			{
				print_status("Searching SD Codes");
			} else
			{
				print_status("Searching USB Codes");
			}

			ret = init_drive();
			if (ret < 0) return ret;
			
			drive_state = 1;

			memset(gameidbuffer, 0, 8);
			memcpy(gameidbuffer, (char*)0x80000000, 6);		

			if(gameidbuffer[1] == 0 && gameidbuffer[2] == 0)
			{
				print_status("Error: No DVD (o)");
				return -1;
			}

			write_font(185, 180,true, "Game ID:");
			write_font(325, 180,true, gameidbuffer);
			wait(2);
		}

		if (prepare_storage_access() < 0)
		{
			print_status("Storage access failed...");
			wait(3);
			
			do_codes(false);
		} else
		{
			do_codes(true);
			resume_disc_loading();
		}		
	}

	wait(1);
	return 1;
}

/*******************************************************************************
 * Patch_Country_Strings: Patches the Country Strings
 * -----------------------------------------------------------------------------
 * Return Values:
 *	returns true if patched
 *
 ******************************************************************************/

bool Patch_Country_Strings(void *Address, int Size)
{
	u8 SearchPattern[4];
	u8 PatchData[2];
	u8 *Addr			= (u8*)Address;

	int wiiregion = CONF_GetRegion();

	switch (wiiregion)
	{
		case CONF_REGION_JP:
			SearchPattern[0] = 0x00;
			SearchPattern[1] = 0x4A; // J
			SearchPattern[2] = 0x50; // P
			SearchPattern[3] = 0x00;
			break;
		case CONF_REGION_EU:
			SearchPattern[0] = 0x02;
			SearchPattern[1] = 0x45; // E
			SearchPattern[2] = 0x55; // U
			SearchPattern[3] = 0x00;
			break;
		case CONF_REGION_KR:
			SearchPattern[0] = 0x04;
			SearchPattern[1] = 0x4B; // K
			SearchPattern[2] = 0x52; // R
			SearchPattern[3] = 0x00;
			break;
		case CONF_REGION_CN:
			SearchPattern[0] = 0x05;
			SearchPattern[1] = 0x43; // C
			SearchPattern[2] = 0x4E; // N
			SearchPattern[3] = 0x00;
			break;
		case CONF_REGION_US:
		default:
			SearchPattern[0] = 0x01;
			SearchPattern[1] = 0x55; // U
			SearchPattern[2] = 0x53; // S
			SearchPattern[3] = 0x00;
	}

	switch (*(char*)0x80000003) 
	{
		case 'J':
			PatchData[0] = 0x4A; // J
			PatchData[1] = 0x50; // P
			break;

		case 'P':	// PAL
		case 'D':	// German
		case 'F':	// French
		case 'S':	// Spanish
		case 'I':	// Italian
		case 'X':	// PAL other languages?
		case 'Y':	// PAL other languages?
			PatchData[0] = 0x45; // E
			PatchData[1] = 0x55; // U
			break;

		case 'E':	// NTSC-U
		case 'N':	// Japanese Import to USA
		default:
			PatchData[0] = 0x55; // U
			PatchData[1] = 0x53; // S
	}

	while (Size >= 4)
	{
		if (Addr[0] == SearchPattern[0] && Addr[1] == SearchPattern[1] && Addr[2] == SearchPattern[2] && Addr[3] == SearchPattern[3])
		{
			Addr += 1;
			*Addr = PatchData[0];
			Addr += 1;
			*Addr = PatchData[1];
			return true;
		} else
		{
			Addr += 4;
			Size -= 4;
		}
	}
	return false;
}

bool Remove_001_Protection(void *Address, int Size)
{
	// Only works for the oldest 001 error version
	u8 SearchPattern[16] = 	{ 0x40, 0x82, 0x00, 0x0C, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };
	u8 PatchData[16] = 		{ 0x40, 0x82, 0x00, 0x04, 0x38, 0x60, 0x00, 0x01, 0x48, 0x00, 0x02, 0x44, 0x38, 0x61, 0x00, 0x18 };

	void *Addr = Address;
	void *Addr_end = Address+Size;

	while(Addr <= Addr_end-sizeof(SearchPattern))
	{
		if(memcmp(Addr, SearchPattern, sizeof(SearchPattern))==0) 
		{
			memcpy(Addr,PatchData,sizeof(PatchData));
			return true;
		}
		Addr += 4;
	}
	return false;
}

void Anti_002_fix(void *Address, int Size)
{
	// Reverts the main.dol 002 patch, some games crash when it's used, and the main.dol 002 patch doesn't work to remove the 002 error on those games anyways
	u8 SearchPattern[12] = 	{ 0x2C, 0x00, 0x00, 0x00, 0x48, 0x00, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };
	u8 PatchData[12] = 		{ 0x2C, 0x00, 0x00, 0x00, 0x40, 0x82, 0x02, 0x14, 0x3C, 0x60, 0x80, 0x00 };

	void *Addr = Address;
	void *Addr_end = Address+Size;

	while(Addr <= Addr_end-sizeof(SearchPattern))
	{
		if(memcmp(Addr, SearchPattern, sizeof(SearchPattern))==0) 
		{
			memcpy(Addr,PatchData,sizeof(PatchData));
		}
		Addr += 4;
	}
}


bool mount_disc()
{
	if (Sneek_Mode == true)
	{
		Sneek_DVDSelectGame(Sneek_game_list[gameselected].index);
		return true;
	}

	// Reset to disc first
	SetWBFSMode(0, NULL);

	if (wbfsdevice == 3)
	{
		return (DML_select_game() >= 0);		
	}
	
	if (gameselected != getGameCount() && wbfsdevice > 0)
	{
		if (!WBFS_isalive(wbfsdevice))
		{
			return false;
		}
		if (SetWBFSMode(wbfsdevice, getGameHeader(gameselected)->id) != 0)
		{
			return false;
		}
	}
	
	return true;
}


u32 load_gamecube_dol(u8 *offset)
{
	print_status("Loading GC dol...");
	if (prepare_storage_access() < 0)
	{
		wait(3);
		print_status("Storage access failed...");
		wait(3);
		return 0;
	}

	static char buf[128];
	FILE *fp = NULL;
	u32 filesize = 0;

	snprintf(buf, 128, "fat:/NeoGamma/GC.dol");
	fp = fopen(buf, "rb");

	if (!fp)
	{
		print_status("No GC dol found");
		wait(3);		
		return 0;
	}	

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	fread(offset, filesize, 1, fp);

	fclose(fp);

	print_status("GC dol loaded");
	wait(1);
	
	resume_disc_loading();
	
	return filesize;
}

u8 *wip_buffer = NULL;
u32 wip_filesize = 0;

void load_wip_patches()
{
	if (patchselect == 1)
	{
		print_status("Searching patch...");
		wait(1);

		static char buf[128];
		char tempbuffer[8];

		if (Sneek_Mode == false)
		{
			FILE *fp = NULL;

			memset(tempbuffer, 0, 8);
			memcpy(tempbuffer, (char*)0x80000000, 6);		

			snprintf(buf, 128, "fat:/NeoGamma/%s_%s.wip", tempbuffer, dolname);
			fp = fopen(buf, "rb");
			if (!fp)
			{
				if (strncmp(dolname, "main", 4) == 0 && strlen(dolname) == 4)
				{
					snprintf(buf, 128, "fat:/NeoGamma/%s.wip", tempbuffer);
					fp = fopen(buf, "rb");
				}
			}
			if (!fp)
			{
				memset(tempbuffer, 0, 8);
				memcpy(tempbuffer, (char*)0x80000000, 4);		
				snprintf(buf, 128, "fat:/NeoGamma/%s_%s.wip", tempbuffer, dolname);
				fp = fopen(buf, "rb");
			}
			if (!fp)
			{
				if (strncmp(dolname, "main", 4) == 0 && strlen(dolname) == 4)
				{
					snprintf(buf, 128, "fat:/NeoGamma/%s.wip", tempbuffer);
					fp = fopen(buf, "rb");
				}
			}

			if (!fp)
			{
				print_status("No patch found");
				wait(1);		
				return;
			}	

			fseek(fp, 0, SEEK_END);
			wip_filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			if (wip_buffer != NULL)
			{
				free(wip_buffer);
			}
			wip_buffer = malloc(wip_filesize+1);
			if (wip_buffer == NULL)
			{
				print_status("Out of memory");
				wait(3);		
				fclose(fp);
				return;
			}
			memset(wip_buffer, 0, wip_filesize);

			fread(wip_buffer, wip_filesize, 1, fp);
			fclose(fp);
			
			wip_buffer[wip_filesize] = 0;
		} else
		{
			u8 *temp_wip_buffer = NULL;
			int ret;
			
			memset(tempbuffer, 0, 8);
			memcpy(tempbuffer, (char*)0x80000000, 6);		

			snprintf(buf, 128, "/NeoGamma/%s_%s.wip", tempbuffer, dolname);

			ISFS_Initialize();
			
			ret = read_file_from_nand(buf, &temp_wip_buffer, &wip_filesize);
			if (ret < 0)
			{
				if (strncmp(dolname, "main", 4) == 0 && strlen(dolname) == 4)
				{
					snprintf(buf, 128, "/NeoGamma/%s.wip", tempbuffer);
					ret = read_file_from_nand(buf, &temp_wip_buffer, &wip_filesize);
				}
			}
			if (ret < 0)
			{
				memset(tempbuffer, 0, 8);
				memcpy(tempbuffer, (char*)0x80000000, 4);		
				snprintf(buf, 128, "/NeoGamma/%s_%s.wip", tempbuffer, dolname);
				ret = read_file_from_nand(buf, &temp_wip_buffer, &wip_filesize);
			}
			if (ret < 0)
			{
				if (strncmp(dolname, "main", 4) == 0 && strlen(dolname) == 4)
				{
					snprintf(buf, 128, "/NeoGamma/%s.wip", tempbuffer);
					ret = read_file_from_nand(buf, &temp_wip_buffer, &wip_filesize);
				}
			}

			if (ret < 0)
			{
				print_status("No patch found");
				wait(1);		
				return;
			}	

			if (wip_buffer != NULL)
			{
				free(wip_buffer);
			}
			wip_buffer = malloc(wip_filesize+1);
			if (wip_buffer == NULL)
			{
				print_status("Out of memory");
				wait(3);		
				free(temp_wip_buffer);			
				return;
			}
			memcpy(wip_buffer, temp_wip_buffer, wip_filesize);
			wip_buffer[wip_filesize] = 0;
			free(temp_wip_buffer);			
		}		
	}
}

void apply_wip_patches()
{
	if (patchselect == 1 && wip_buffer != NULL)
	{
		if (wipparsebuffer(wip_buffer, wip_filesize))
		{
			print_status("Patch applied");
		} else
		{
			print_status("Patch forced!");
		}
		wip_patch_applied = true;
		wait(2);
		free(wip_buffer);
	}	
}

void sneek_video_patch(void *addr, u32 len)
{
	u8 *addr_start = addr;
	u8 *addr_end = addr+len;
	
	while(addr_start < addr_end)
	{
		if( *(vu32*)(addr_start) == 0x3C608000 )
		{
			if( ((*(vu32*)(addr_start+4) & 0xFC1FFFFF ) == 0x800300CC) && ((*(vu32*)(addr_start+8) >> 24) == 0x54 ) )
			{
				//dbgprintf("DIP:[patcher] Found VI pattern:%08X\n", (u32)(addr_start) | 0x80000000 );
				*(vu32*)(addr_start+4) = 0x5400F0BE | ((*(vu32*)(addr_start+4) & 0x3E00000) >> 5	);
			}
		}
		addr_start += 4;
	}
}

void maindolpatches(void *dst, u32 len)
{
	wipregisteroffset((u32)dst, len);
	
	DCFlushRange(dst, len);
	
	if (viselect)	//VIDTV patch
	{
		vidolpatcher(dst,len);
	}
	
	//if(debugmode)
	//{
	//	patchdebug(dst,len);
	//}
	langpatcher(dst,len);
	if (!Country_Strings_Patched) Patch_Country_Strings(dst, len);
	//if (anti002fixselect) Anti_002_fix(dst, len);

	if (dogamehooks(dst,len))
	{
		hookpatched = true;
	}
	
	if (videopatchselect > 3)
	{
		sneek_video_patch(dst, len);
	}
	
	DCFlushRange(dst, len);
	ICInvalidateRange(dst, len);
	
	toy_register_apploader_file_loading(dst, len);
}


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
		stringoffset = *(u32 *)&(fst[index]) % (256*256*256);
		return (char *)(*(u32 *)0x80000038 + count*12 + stringoffset);
	} else
	{
		return NULL;
	}
}

u32 fstfileoffset(u32 index)
{
	FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
	u32 count = fst[0].filelen;
	if (index < count)
	{
		return fst[index].fileoffset;
	} else
	{
		return 0;
	}
}

u32 Load_Dol_from_disc(u32 doloffset)
{
	int ret;
	void *dol_header;	
	u32 entrypoint;

	dol_header = allocate_memory( sizeof(dolheader));
	if (dol_header == NULL)
	{
		print_status("Out of memory");
		wait(3);
		return 0;
	}

	dvddone = 0;
	ret = bwDVD_LowRead(dol_header, sizeof(dolheader), doloffset, __dvd_readidcb);
	strcpy(dvderror, ".dol header");
	DVD_CHECK();

	entrypoint = load_dol_start(dol_header);

	if (entrypoint == 0)
	{
		print_status("Invalid .dol");
		wait(3);
		free(dol_header);
		return 0;
	}
	
	void *offset;
	u32 pos;
	u32 len;

	while (load_dol_image(&offset, &pos, &len))
	{
		if (len != 0)
		{
			dvddone = 0;
			ret = bwDVD_LowRead(offset, len, (doloffset+pos/4), __dvd_readidcb);
			strcpy(dvderror, ".dol loop");
			DVD_CHECK();

			maindolpatches(offset, len);
			//Remove_001_Protection(offset, len);
		}
	}
	
	free(dol_header);

	return entrypoint;
}	

void getdolnames()
{
	FST_ENTRY *fst = (FST_ENTRY *)*(u32 *)0x80000038;
	u32 count = fst[0].filelen;
	int i;

	dolcount = 0;

	for (i=1;i<count;i++)
	{		
		if (strstr(fstfilename(i), ".dol") != NULL)
		{
			if (dolcount < 10)
			{
				dolindex[dolcount] = i;
				dolcount++;
			}
		}		
	}
}

u32 stringcompare(char *s1, char *s2)
{
	u32 index = 0;
	while (true)
	{
		if (s1[index] == 0 || s2[index] == 0)
		{
			return 0;
		}
		if (s1[index] != '?' && s2[index] != '?' && toupper((u8)s1[index]) != toupper((u8)s2[index]))
		{
			return 1;
		}
		index++;	
	}
}

u32 getdoloffsetbyname(char *name)
{
	if (stringcompare(dolname, "main") == 0 && strlen(dolname) == 4)
	{
		return 0;
	}

	u32 i;
	u32 index = 0;
	
	for (i = 0; i < dolcount;i++)
	{
		if (stringcompare(fstfilename(dolindex[i]), dolname) == 0)
		{
			if (index != 0)
			{
				return 0;
			}
			index = dolindex[i];
		}
	}
	memset(dolname, 0, 32);
	strncpy(dolname, fstfilename(index), strlen(fstfilename(index)));
	return fstfileoffset(index);
}


u32 showdolmenu()
{
	u32 pressed;
	u32 pressedGC;	

	int index = 0;
	
	while (true)
	{
		if (dolmenubuffer[index].count == 0)
		{
			memset(dolname, 0, 32);
			strncpy(dolname, dolmenubuffer[index].dolname, strlen(dolmenubuffer[index].dolname));
			dolparameter = dolmenubuffer[index].parameter;
			free(dolmenubuffer);
			return 1;
		}
		if (dolmenubuffer[index].count == 1)
		{
			memset(dolname, 0, 32);
			strncpy(dolname, dolmenubuffer[index+1].dolname, strlen(dolmenubuffer[index+1].dolname));
			dolparameter = dolmenubuffer[index+1].parameter;
			free(dolmenubuffer);
			return 1;
		}
		
		int parent = index;
		index++;
		
		while (true)
		{		
			print_status(dolmenubuffer[index].name);
			
			pressed = 0;
			pressedGC = 0;
			
			waitforbuttonpress(&pressed, &pressedGC, NULL);
			
			if (pressed == WPAD_BUTTON_LEFT || pressedGC == PAD_BUTTON_LEFT)
			{
				if (index == parent + 1)
				{
					index = parent + dolmenubuffer[parent].count + 2;
				}

				int left = parent + 1;
				while (left + dolmenubuffer[left].count + 1 < index && left + dolmenubuffer[left].count + 1 <= parent + dolmenubuffer[parent].count)
				{
					left = left + dolmenubuffer[left].count + 1;
				}
				
				index = left;
			}

			if (pressed == WPAD_BUTTON_RIGHT || pressedGC == PAD_BUTTON_RIGHT)
			{
				if (index + dolmenubuffer[index].count + 1 <= parent + dolmenubuffer[parent].count)
				{
					index = index + dolmenubuffer[index].count + 1;
				} else
				{
					index = parent + 1;
				}
			}

			if (pressed == WPAD_BUTTON_A || pressedGC == PAD_BUTTON_A)
			{
				break;
			}
			
			if (pressed == WPAD_BUTTON_B || pressedGC == PAD_BUTTON_B)
			{
				if (dolmenubuffer[parent].parent == -1)
				{
					free(dolmenubuffer);
					return 0;
				} else
				{
					index = dolmenubuffer[parent].parent;
					break;
				}
			}			
		}
	}		
}


u32 Load_Dol_from_storage()
{
	if (Sneek_Mode == true)
	{
		print_status("Alt .dol from storage disabled");
		wait(3);
		return 0;	
	}
	
	print_status("Loading .dol from storage...");
	
	int ret;
	FILE* file;
	void *dol_header;	
	u32 entrypoint;
	
	ret = prepare_storage_access();
	if (ret < 0)
	{
		wait(3);
		print_status("Storage access failed...");
		wait(3);
		return ret;
	}

	char buf[128];
	char tempbuffer[7];
	
	memset(tempbuffer, 0, 7);
	memcpy(tempbuffer, (char*)0x80000000, 6);		
	snprintf(buf, 128, "fat:/NeoGamma/%s.dol", tempbuffer);

	file = fopen( buf, "rb");
	
	if (file == NULL) 
	{
		memset(tempbuffer, 0, 7);
		memcpy(tempbuffer, (char*)0x80000000, 4);		
		snprintf(buf, 128, "fat:/NeoGamma/%s.dol", tempbuffer);

		file = fopen( buf, "rb");

		if (file == NULL) 
		{
			
			memset(tempbuffer, 0, 7);
			memcpy(tempbuffer, (char*)0x80000000, 6);		
			print_status("%s.dol not found", tempbuffer);

			wait(5);                
			resume_disc_loading();
			return 0;
		}
	}
	
	u32 filesize;
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	dol_header = malloc(sizeof(dolheader));
	if (dol_header == NULL)
	{
		print_status("Out of memory");
		wait(3);
		fclose(file);
		resume_disc_loading();
		return 0;
	}

	ret = fread( dol_header, 1, sizeof(dolheader), file);
	if(ret != sizeof(dolheader))
	{
		print_status("Error reading dol header");
		wait(3);
		free(dol_header);
		fclose(file);
		resume_disc_loading();
		return 0;
	}
	
	entrypoint = load_dol_start(dol_header);
	
	if (entrypoint == 0)
	{
		print_status("Invalid .dol");
		wait(3);
		free(dol_header);
		fclose(file);
		resume_disc_loading();
		return 0;
	}
	
	void *offset;
	u32 pos;
	u32 len;
	
	while (load_dol_image(&offset, &pos, &len))
	{
		if(pos+len > filesize)
		{
			print_status(".dol too small");
			wait(3);
			free(dol_header);
			fclose(file);
			resume_disc_loading();
			return 0;
		}		
		
		if (len != 0)
		{
			fseek(file, pos, 0);
			ret = fread( offset, 1, len, file);
			if(ret != len)
			{
				print_status("Error reading .dol");
				wait(3);
				free(dol_header);
				fclose(file);
				resume_disc_loading();
				return 0;
			}
			maindolpatches(offset, len);
		}	
	}
	
	free(dol_header);
	fclose(file);

	resume_disc_loading();

	return entrypoint;
}	


/* Identify as DVD title, created by WiiGator. */
static int DVD_Identify(u8 *ios)
{
	static u8 buffer[SECTOR_SIZE] __attribute__((aligned(32)));
	signed_blob *p_certs = NULL, *p_tik = NULL, *p_tmd = NULL;
	u32 certs_len, tik_len, tmd_len;
	sig_rsa2048 *signature;
	int ret;

	ret = __CIOS_GetCerts(&p_certs, &certs_len);
	if (ret < 0) 
	{
		return -1;
	}
	
	dvddone = 0;
	ret = bwDVD_SetOffset(0, __dvd_readidcb);
	strcpy(dvderror, "SetOffset 2");
	DVD_CHECK();

	dvddone = 0;
	ret = bwDVD_LowRead(buffer, SECTOR_SIZE, 0,__dvd_readidcb);

//  LowUnencryptedRead on new cIOS is not working the same way it did with old cIOS
//	ret = bwDVD_LowUnencryptedRead(buffer, SECTOR_SIZE, 0,__dvd_readidcb);

	strcpy(dvderror, "DVD Identify");
	DVD_CHECK();

	dvddone = 0;
	ret = bwDVD_SetOffset(SECTOR_SIZE >> 2, __dvd_readidcb);
	strcpy(dvderror, "SetOffset 3");
	DVD_CHECK();

	p_tik = (signed_blob *) buffer;
	signature       = (sig_rsa2048 *)p_tik;
	signature->type = ES_SIG_RSA2048;

	tik_len = SIGNED_TIK_SIZE(p_tik);

	int mod_size = ((tik_len + 64 - 1) & ~(64 - 1));

	p_tmd = (signed_blob *)(((u32) p_tik) + mod_size);
	signature       = (sig_rsa2048 *)p_tmd;
	signature->type = ES_SIG_RSA2048;

	tmd_len = SIGNED_TMD_SIZE(p_tmd);
	
	ret = ES_Identify(p_certs, certs_len, p_tmd, tmd_len, p_tik, tik_len, NULL);

	*ios = ((u8 *)p_tmd)[0x18b];

	return ret;
}


s32 init_drive()
{
	s32 ret = 1;

	print_status("Init drive...");

	ret = bwDVD_LowInit();

	if (!mount_disc())
	{
		print_status("Connection lost");
		return -1;
	}
	
	print_status("Reset drive...");

	dvddone = 0;
	ret = bwDVD_LowReset(__dvd_readidcb);
	strcpy(dvderror, "Reset");
	DVD_CHECK();
	
	// Only do this once
	if (drivedate == 0)
	{
		dvddrvinfo *info;
		info = allocate_memory(sizeof(dvddrvinfo));
		if (info == NULL)
		{
			print_status("Out of memory(Inquiry)");
			return -1;			
		}
		
		dvddone = 0;
		ret = bwDVD_LowInquiry(info, __dvd_readidcb);
		strcpy(dvderror, "Inquiry");
		drivedate = 1;
		DVD_CHECK();

		drivedate = info->rel_date;
		free(info);
	}

	// If a previous loading was canceled a partition could still be opened, which would result in an error
	dvddone = 0;
	ret = bwDVD_LowClosePartition(__dvd_readidcb);
	strcpy(dvderror, "Close Partition");
	DVD_CHECK();

	/* clear existing GameID */
	memset((char*)0x80000000, 0, 0x20);

	//print_status("Read Game ID...");

	dvddone = 0;
	ret = bwDVD_LowReadID(g_diskID,__dvd_readidcb);
	strcpy(dvderror, "Disc ID");
	DVD_CHECK();
	
	DCFlushRange((char *)0x80000000, 32);
	ICInvalidateRange((char *)0x80000000, 32);

	return 1;
}


s32 open_partition()
{		
	s32 ret;
	dvddone = 0;
	__dvd_Tmd = (u32*)_gameTmdBuffer;
	ret = bwDVD_LowOpenPartition(__dvd_bootGameInfo->offset,NULL,0,NULL,(void*)_gameTmdBuffer,__dvd_readidcb);
	strcpy(dvderror, "Open Partition");
	DVD_CHECK();
	return 1;
}

typedef struct {
	u8 ConsoleID;				//G = Gamecube, R = Wii.
	u8 GamecodeA;				//2 Ascii letters to indicate the GameID.
	u8 GamecodeB;				//2 Ascii letters to indicate the GameID.
	u8 CountryCode;				//J=JAP . P=PAL . E=USA . D=OoT MasterQuest
	u8 MakerCodeA;				//Eg 08 = Sega etc.
	u8 MakerCodeB;
	u8 DiscID;
	u8 Version;
	u8 AudioStreaming;			//01 = Enable it. 00 = Don't
	u8 StreamBufSize;			//For the AudioEnable. (always 00?)
	u8 unused_1[18];
	u32 DVDMagicWord;			//0xC2339F3D
	char GameName[992];			//String Name of Game, rarely > 32 chars
	u32 DMonitorOffset;			//offset of debug monitor (dh.bin)?
	u32 DMonitorLoadAd;			//addr(?) to load debug monitor?
	u8 unused_2[24];
	u32 DOLOffset;				//offset of main executable DOL (bootfile)
	u32 FSTOffset;				//offset of the FST ("fst.bin")
	u32 FSTSize;				//size of FST
	u32 MaxFSTSize;				//maximum size of FST (usually same as FSTSize)*
	u32 UserPos;				//user position(?)
	u32 UserLength;				//user length(?)
	u32 unknown;				//(?)
	u32 unused_3;
} dvdinfo_t;

dvdinfo_t *gcm_disk = NULL;

u32 gamecube_game_count;
u32 second_gamecube_game;
u32 selected_gamecube_game;
u32 second_gamecube_disc_offset = 0;


s32 show_GC_gameselection()
{
	u32 pressed;
	u32 pressedGC;	
	u32 cover_status;
	u32 old_cover_status;

	// Remove "Launching Game" from screen
	clearscreen(xfb, 0, 149 , 319, 24, 0x00800080);
	
	write_font(185, 180,true, "Select game:");

	while (true)
	{
		if (selected_gamecube_game == second_gamecube_game)
		{
			if (gcm_disk[selected_gamecube_game].DiscID > 0)
			{
				print_status("%s Disc %u*", gcm_disk[selected_gamecube_game].GameName, gcm_disk[selected_gamecube_game].DiscID + 1);
			} else
			{
				print_status("%s*",gcm_disk[selected_gamecube_game].GameName);
			}
		} else
		{
			if (gcm_disk[selected_gamecube_game].DiscID > 0)
			{
				print_status("%s Disc %u", gcm_disk[selected_gamecube_game].GameName, gcm_disk[selected_gamecube_game].DiscID + 1);
			} else
			{
				print_status("%s", gcm_disk[selected_gamecube_game].GameName);
			}
		}

		pressed = 0;
		pressedGC = 0;
		
		bwDVD_GetCoverStatus(&old_cover_status);
		
		waitforbuttonpress(&pressed, &pressedGC, &cover_status);
			
		if (pressed == WPAD_BUTTON_LEFT || pressedGC == PAD_BUTTON_LEFT)
		{
			if (selected_gamecube_game == 0)
			{
				selected_gamecube_game = gamecube_game_count-1;
			} else
			{
				selected_gamecube_game--;
			}
		}

		if (pressed == WPAD_BUTTON_RIGHT || pressedGC == PAD_BUTTON_RIGHT)
		{
			if (selected_gamecube_game == gamecube_game_count-1)
			{
				selected_gamecube_game = 0;
			} else
			{
				selected_gamecube_game++;
			}
		}

		if (pressed == WPAD_BUTTON_A || pressedGC == PAD_BUTTON_A)
		{
			break;
		}
			
		if (pressed == WPAD_BUTTON_1 || pressedGC == PAD_TRIGGER_Z)
		{
			if (second_gamecube_game == selected_gamecube_game)
			{
				// Unselect 2nd game
				second_gamecube_game = -1;
				second_gamecube_disc_offset = 0;
			} else
			{
				second_gamecube_game = selected_gamecube_game;
			}
		}

		if (pressed == WPAD_BUTTON_B || pressedGC == PAD_BUTTON_B || (cover_status != old_cover_status))
		{
			return -1;
		}
	}	
	return 0;
}



s32 load_MIOS()
{		
	u32 i;
	int ret;
	char *discid;

	// Load gamecube games from sd card via DML
	if (Sneek_Mode == true || wbfsdevice == 3)
	{
		__setappvideo_GC((char *)0x80000000);			
		WPAD_Shutdown();

		ret = setstreaming();

		if (ret == -2)
		{
			ret = bwDVD_LowRequestError();
			
			if (ret == 0x052402)
			{
				// Would be too annoying to display this everytime
			} else
			{
				print_status("Streaming error: %08x", ret);
			}
			wait(3);
		} else
		{
			print_status("Audio streaming set");
			wait(2);
		}
	
		bwDVD_LowStopMotor(0, 0);
		bwDVD_LowClose();
		
		set_videomode();
		
		ret = ES_GetTicketViews(BC, &view, 1);
		if (ret != 0)
		{
			print_status("BC ticket views error");
			wait(3);
			return -1;
		}

		// Tell DML to boot the game from sd card
		*(u32 *)0x80001800 = 0xB002D105;
		DCFlushRange((void *)(0x80001800), 4);
		ICInvalidateRange((void *)(0x80001800), 4);			
		
		*(volatile unsigned int *)0xCC003024 |= 7;
		
		ret = ES_LaunchTitle(BC, &view);
		
		return -102;
	}
		
	// Prevent bugs due to previously booting GC homebrew
	memset((void *)0x807FFFE0, 0, 32);

	if (gamecubemodeselect != 0)
	{
		u32 gamecubedollength;
			
		if (gamecubemodeselect == 1)	// Use internal loader
		{
			u32 gameOffsetTable[65];
			gameOffsetTable[0] = 0;

			if (strncmp((const char *) 0x80000000, "COBRAM", 6) == 0
			||  strncmp((const char *) 0x80000000, "GGCOSD", 6) == 0
			||  strncmp((const char *) 0x80000000, "GCOPDV", 6) == 0
			||  strncmp((const char *) 0x80000000, "RGCOSD", 6) == 0)
			{
				dvddone = 0;
				ret = bwDVD_LowUnencryptedRead(&(gameOffsetTable[1]), sizeof(u32)*64, 64 >> 2,__dvd_readidcb);
				strcpy(dvderror, "Offset Table");
				DVD_CHECK();

				u32 type;
				dvddone = 0;
				ret = bwDVD_LowUnencryptedRead(&type,sizeof(type), 4 >> 2,__dvd_readidcb);
				strcpy(dvderror, "Read Type");
				DVD_CHECK();

				gamecube_game_count = 1;
				for (i = 1; i < 65; i++) 
				{
					if (gameOffsetTable[i] != 0) 
					{
						if (type != 0x44564439) // 'DVD9'
						{
							// Not DVD9 offsets need to convert the offset.
							gameOffsetTable[i] = gameOffsetTable[i] >> 2;
						}
						gamecube_game_count++;
					} else 
					{
						break;
					}
				}
				
				if (gcm_disk != NULL)
				{
					free(gcm_disk);
				}
				gcm_disk = memalign(32, sizeof(dvdinfo_t) * (gamecube_game_count + 1));
				if (gcm_disk == NULL)
				{
					return -1;
				}
				
				for (i = 0;i < gamecube_game_count;i++)
				{
					print_status("Reading game info #%d", i);
					
					dvddone = 0;
					ret = bwDVD_LowUnencryptedRead(&(gcm_disk[i]), sizeof(dvdinfo_t), gameOffsetTable[i],__dvd_readidcb);
					strcpy(dvderror, "Game info");
					DVD_CHECK();
				
					/* preparations for banner/icon loading
					dvddone = 0;
					ret = bwDVD_LowUnencryptedRead(..., gcm_disk[i].FSTSize, gameOffsetTable[i] + gcm_disk[i].FSTOffset >> 2,__dvd_readidcb);
					strcpy(dvderror, "Game info");
					DVD_CHECK();
					*/				
				}
				
				if (gamecube_game_count > 1)
				{
					selected_gamecube_game = 1;
				} else
				{
					selected_gamecube_game = 0;
				}
				second_gamecube_game = -1;

				ret = show_GC_gameselection();
				
				if (ret < 0)		// Abort loading
				{
					if (second_gamecube_game != -1)
					{
						second_gamecube_disc_offset = gameOffsetTable[second_gamecube_game];
					}
					return -101;
				}
				
				discid = (char *)&gcm_disk[selected_gamecube_game];
				__setappvideo_GC(discid);

				if (second_gamecube_game == -1 && memcmp((void *)discid, "GNHE", 4) != 0)	// Don't auto detect 2nd disc for Action Replay
				{
					// second disc auto detect
					for (i=1;i<gamecube_game_count;i++)
					{
						if (i != selected_gamecube_game
						&& gcm_disk[selected_gamecube_game].ConsoleID == gcm_disk[i].ConsoleID
						&& gcm_disk[selected_gamecube_game].GamecodeA == gcm_disk[i].GamecodeA
						&& gcm_disk[selected_gamecube_game].GamecodeB == gcm_disk[i].GamecodeB
						&& gcm_disk[selected_gamecube_game].CountryCode == gcm_disk[i].CountryCode
						&& gcm_disk[selected_gamecube_game].MakerCodeA == gcm_disk[i].MakerCodeA
						&& gcm_disk[selected_gamecube_game].MakerCodeB == gcm_disk[i].MakerCodeB)
						{
							second_gamecube_game = i;
							break;
						}						
					}
					if (second_gamecube_game == -1)
					{
						//second_gamecube_game = selected_gamecube_game;
					}
				}
			} else
			{
				// Single game disc
				selected_gamecube_game = 0;
				second_gamecube_game = 0;					
				__setappvideo_GC((char *)0x80000000);
				discid = (char *)0x80000000;
			}

			gamecubedollength = gcbackuplauncher_dol_size;
			memcpy((void *) 0x80800000, gcbackuplauncher_dol, gamecubedollength);
			
			// Pass options to GC loader
			memset((void *)0x807FFF00, 0, 0x0100);
			strcpy((char *)0x807FFF00, "GAMECUBELOADER01");
			
			*(u32 *)0x807FFF20 = gameOffsetTable[selected_gamecube_game];
			
			if (second_gamecube_disc_offset == 0)
			{
				if (second_gamecube_game != -1)
				{
					*(u32 *)0x807FFF24 = gameOffsetTable[second_gamecube_game];
				} else
				{
					*(u32 *)0x807FFF24 = 0;	// Allow Action Replay on a multi game disc to boot single game discs
				}
			} else
			{
				*(u32 *)0x807FFF24 = second_gamecube_disc_offset; // Offset from a 2nd disc
			}
			*(u32 *)0x807FFF28 = gc_videoselect;

			if (CONF_GetVideo() == CONF_VIDEO_NTSC)
			{
				*(u32 *)0x807FFF2C = 0;
			} else
			{
				*(u32 *)0x807FFF2C = 1;
			}

			*(u8 *)0x807FFF30 = gc_videopatchselect;
			*(u8 *)0x807FFF31 = gchookselect;
			if (gc2ndhookselect == 8)
			{
				*(u8 *)0x807FFF32 = gchookselect;
			} else
			{
				*(u8 *)0x807FFF32 = gc2ndhookselect;
			}
			*(u8 *)0x807FFF33 = gcocarinaselect;
			*(u8 *)0x807FFF34 = gcdebuggerselect;
			*(u8 *)0x807FFF35 = gcreloaderselect;
			*(u8 *)0x807FFF36 = gchighplugin;
			*(u8 *)0x807FFF37 = audiostatusrequestfixselect;
			*(u8 *)0x807FFF38 = patchedMIOSselect;

			// Set max size for Ocarina codes
			if (gcdebuggerselect == 0)
				gccodelist = (u8 *) 0x800022A8;
			else
				gccodelist = (u8 *) 0x800028B8;
			gccodelistend = (u8 *) 0x80003000;
			maxgccodesize = (u32)gccodelistend - (u32)gccodelist;
			
			*(u32 *)0x807FFF40 = (u32)gccodelist;
			*(u32 *)0x807FFF44 = (u32)gccodelistend;

			if (gcocarinaselect == 1)
			{
#ifdef DONTTELL
				if (gchookselect == 0 && (gc2ndhookselect == 0 || gc2ndhookselect == 8))
#else
				if (gchookselect == 0)
#endif
				{
					print_status("Stupid...");
					wait(5);
				} else
				{
					print_status("Searching codes...");
					wait(1);
					memset((void *)(0x807FFF00 - maxgccodesize), 0, maxgccodesize);
					ret = prepare_storage_access();	
					if (ret >= 0)
					{
						if (load_codes(discid, maxgccodesize, (void *)(0x807FFF00 - maxgccodesize)) == 0)
						{
							print_status("Codes found. Applying");
						} else
						{
							//gcocarinaselect = 0;
							wait(2);
						}
						wait(2);
						resume_disc_loading();
					} else
					{
						gcocarinaselect = 0;
						print_status("Storage access failed...");
						wait(2);
					}
				}
			}			

			DCFlushRange((void *)(0x807FFF00 - maxgccodesize), maxgccodesize+0x100);
			ICInvalidateRange((void *)(0x807FFF00 - maxgccodesize), maxgccodesize+0x100);			
		} else
		{
			// Remove "Launching Game" and disc id from screen
			clearscreen(xfb, 0, 149 , 319, 60, 0x00800080);
			write_font(185, 149,true, "Launching GC homebrew");

			// Use external loader
			gamecubedollength = load_gamecube_dol((void *) 0x80800000);
			if (gamecubedollength == 0)	// Stop loading process if no GC homebrew is found
			{
				return -1;
			}
			
			__setappvideo_GC((char *)0x80000000);			
		}	
		
		if (gamecubedollength != 0)
		{
			strcpy((char *)0x807FFFE0, "gchomebrew dol");
			
			DCFlushRange((void *) 0x80800000, gamecubedollength);
			ICInvalidateRange((void *) 0x80800000, gamecubedollength);				
		}
	} else
	{
		// Just load MIOS
		__setappvideo_GC((char *)0x80000000);			
	}

	DCFlushRange((char *)0x807FFFE0, 32);
	ICInvalidateRange((char *)0x807FFFE0, 32);
	
	WPAD_Shutdown();

	ret = setstreaming();

	if (ret == -2)
	{
		ret = bwDVD_LowRequestError();
		
		if (ret == 0x052402)
		{
			// Would be too annoying to display this everytime
		} else
		{
			print_status("Streaming error: %08x", ret);
		}
		wait(3);
	} else
	{
		print_status("Audio streaming set");
		wait(2);
	}
	
	bwDVD_LowClose();
	
	set_videomode();

	ret = ES_GetTicketViews(BC, &view, 1);
	if (ret != 0)
	{
		print_status("BC ticket views error");
		wait(3);
		return -1;
	}

	//wait(1);		// Hopefully helps with the random crashes
	
	*(volatile unsigned int *)0xCC003024 |= 7;
	
	ret = ES_LaunchTitle(BC, &view);
	
	return -102;
}


void load_patches_wdm_and_Ocarina()
{		
	if (alternativedolselect > 2 || patchselect == 1 || use_wii_Ocarina_engine())
	{

		if (prepare_storage_access() < 0)
		{
			print_status("Storage access failed...");
			wait(3);
			
			if (use_wii_Ocarina_engine())
			{
				do_codes(false);
			}

			return;
		}

		if (use_wii_Ocarina_engine())
		{
			do_codes(true);
		}

		load_wip_patches();
		
		dolmenubuffer = NULL;
		dolmenufallback = false;
		
		if (alternativedolselect > 2)
		{
			if (load_dolmenu((char *)0x80000000) < 0)
			{
				//print_status is already in load_dolmenu
				wait(1);
				
				// for alt .dol 3 fall back to alt .dol disc, for alt .dol 4 don't
				if (alternativedolselect == 3)
				{
					dolmenufallback = true;
				}
			}
		}	
		
		resume_disc_loading();	
	}
}

s32 apploader()
{
	memset(dolname, 0, 32);
	snprintf(dolname, 32, "main");
	dolparameter = 1;
	drive_state = 0;

	toy_reset();
	wipreset();

	void (*app_init)(void (*report)(const char* fmt, ...));
	int (*app_main)(void** dst, int* size, int* offset);
	void* (*app_final)();
	void (*app_entry)(void(**init)(void (*report)(const char* fmt, ...)),
	int (**main)(), void *(**final)());
	char* buffer = (char*)appbuffer;
	
	s32 ret,i = 0;
	hookpatched = false;
	bool decrypted_disc = false;
	wip_patch_applied = false;

	ret = init_drive();
	drive_state = 1;
	
	if (ret < 0) return 0;

	memset(gameidbuffer, 0, 8);
	memcpy(gameidbuffer, (char*)0x80000000, 6);

	/* Gamecube Game */
	// 0xc2339f3d == magic word for gamecube discs
	if (*(u32 *)0x8000001c == 0xc2339f3d || wbfsdevice == 3)
	{
		if ((strncmp((const char *) 0x80000000, "COBRAM", 6) == 0
		||  strncmp((const char *) 0x80000000, "GGCOSD", 6) == 0
		||  strncmp((const char *) 0x80000000, "GCOPDV", 6) == 0
		||  strncmp((const char *) 0x80000000, "RGCOSD", 6) == 0)
		&& (gamecubemodeselect == 1))
		{
			// Remove "Launching Game" from screen for internal loader + multi game disc
			clearscreen(xfb, 0, 149 , 319, 24, 0x00800080);
			
			write_font(185, 180,true, "Select game:");
		} else
		{
			write_font(185, 180,true, "Game ID:");
			write_font(325, 180,true, "%s (GC)", gameidbuffer);
		}
		
		print_status("Loading...");
		
		wait(1);

		ret = load_MIOS();
		if (ret == -101)
		{
			return -101;
		}

		print_status("Gamecube Error");
		if (ret == -102)	// WPAD was shutdown
		{
			WPAD_Init();
			WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
		}
		return 0;
	}

	print_status("Open Partition...");

	u8 tmdios = 0;
	if ((*((u32 *) 0x80000000) == (u32)SIG_RSA_2048) || (*((u32 *) 0x80000000) == (u32)SIG_RSA_4096)) 
	{	
		// Decrypted disc
		decrypted_disc = true;
		
		dvddone = 0;
		ret = bwDVD_SetDecryption(1, __dvd_readidcb);
		strcpy(dvderror, "SetDecryption");
		DVD_CHECK();

		dvddone = 0;
		ret = bwDVD_SetOffset(SECTOR_SIZE >> 2, __dvd_readidcb);
		strcpy(dvderror, "SetOffset 1");
		DVD_CHECK();

		/* clear existing GameID */
		memset((char*)0x80000000, 0, 6);

		dvddone = 0;
		ret = bwDVD_LowRead(g_diskID,0x20, 0, __dvd_readidcb);
		strcpy(dvderror, "Disc ID 2");
		DVD_CHECK();
		
		load_patches_wdm_and_Ocarina();

		if (*(u32 *)0x80000018 != 0x5D1C9EA3)		// Wii magic word check
		{
			print_status("Error: Not a Wii/GC disc");
			return 0;
		}

		memset(gameidbuffer, 0, 8);
		memcpy(gameidbuffer, (char*)0x80000000, 6);		

		write_font(185, 180,true, "Game ID:");
		write_font(325, 180,true, "%s (decrypted)", gameidbuffer);
	} else
	{
		if (*(u32 *)0x80000018 != 0x5D1C9EA3)		// Wii magic word check
		{
			print_status("Error: Not a Wii/GC disc");
			return 0;
		}

		// 1:1 disc
		write_font(185, 180,true, "Game ID:");
		write_font(325, 180,true, gameidbuffer);

		load_patches_wdm_and_Ocarina();

		dvddone = 0;
		__dvd_gameToc = (struct _toc*)_gameTocBuffer;
		ret = bwDVD_LowUnencryptedRead(_gameTocBuffer,0x20,0x00010000,__dvd_readidcb);
		strcpy(dvderror, "Read TOC");
		DVD_CHECK();

		dvddone = 0;
		__dvd_partInfo = (struct _pinfo*)_gamePartInfo;
		ret = bwDVD_LowUnencryptedRead(_gamePartInfo,0x20,__dvd_gameToc->partInfoOff,__dvd_readidcb);
		strcpy(dvderror, "Partition info");
		DVD_CHECK();

		i = 0;
		__dvd_bootGameInfo = NULL;

		while(i<__dvd_gameToc->bootInfoCnt) {
			if(__dvd_partInfo[i].len==0) {	// 1 for update partition
				__dvd_bootGameInfo = &__dvd_partInfo[i];
			}
			i++;
		}
		
		if (__dvd_bootGameInfo == NULL)
		{
			print_status("No partition found!");
			wait(3);
			return -1;
		}

		ret = open_partition();
		
		if (ret < 0) return ret;

		drive_state = 2;
		
		tmdios = ((u8*)_gameTmdBuffer)[0x18b];
	}
	
	print_status("Apploader...");

	dvddone = 0;
	ret = bwDVD_LowRead(buffer,0x20,0x2440/4,__dvd_readidcb);
	strcpy(dvderror, "Apploader header");
	DVD_CHECK();
	DCFlushRange(buffer, 0x20);
	ICInvalidateRange(buffer, 0x20);

	// Read the 3 apploader functions into memory
	dvddone = 0;
	ret = bwDVD_LowRead((void*)0x81200000,((*(u32*)(buffer + 0x14)) + 31) & ~31,0x2460/4,__dvd_readidcb);
	strcpy(dvderror, "Apploader");
	DVD_CHECK();

	print_status("Apploader....");

/*	Before this is enabled, it needs to be confirmed that this reproduces the memory setup from the disc channel
	// Read the apploaer "trailer" into memory, this is the code the disc channel uses to boot games, it's copied to fake the memory setup, it's not used by NeoGamma
	dvddone = 0;
	ret = bwDVD_LowRead((void*)0x81300000,((*(u32*)(buffer + 0x18)) + 31) & ~31,(0x2460+(((*(u32*)(buffer + 0x14)) + 31) & ~31))/4,__dvd_readidcb);
	strcpy(dvderror, "Apploader");
	DVD_CHECK();

	print_status("Apploader.....");
*/

	/* copy game ID and channel patches (set stuff thats missing) */
	*(vu32*)0x80000020 = 0xD15EA5E;	// Boot from DVD
	*(vu32*)0x80000024 = 1; 			// Version
	*(vu32*)0x80000030 = 0; 			// Arena Low
	
	*(vu32*)0x800000EC = 0x81800000; // Dev Debugger Monitor Address
	*(vu32*)0x800000F0 = 0x01800000; // Dev Debugger Monitor Address
	*(vu32*)0x800000F8 = 0x0E7BE2C0; // Console Bus Speed
	*(vu32*)0x800000FC = 0x2B73A840; // Console CPU Speed
	*(vu32*)0xCD00643C = 0x00000000;	// 32Mhz on Bus

	memcpy((void*)0x80003180, (char*)0x80000000, 4);	// online check code, seems offline games clear it?
    *(vu32*)0x800030F0 = 0x0000001C;
    *(vu32*)0x8000318C = 0x00000000;
    *(vu32*)0x80003190 = 0x00000000;
	
	// Fix for Sam & Max
	*(vu32*)0x80003184	= 0x80000000;	// Game ID Address
	
	app_entry = (void (*)(void(**)(void (*)(const char*, ...)), int (**)(), void *(**)()))(*(u32*)(buffer + 0x10));
	app_entry(&app_init, &app_main, &app_final);
	app_init((void (*)(const char*, ...))nothing);

	//displayflags();
	__setappvideo((char *)0x80000000);

	// Set the clock
	settime(secs_to_ticks(time(NULL) - 946684800));
	
	Country_Strings_Patched = !(countrystringselect);

	void* dst_array[64];
	int len_array[64];
	int last_index = -1;
	int fststart;

	print_status("Apploader......");

	while (1)
	{
		void* dst = 0;
		int len = 0,offset = 0;
		int res = app_main(&dst, &len, &offset);
		
		if (!res){
			break;
		}
		
		last_index++;
		dst_array[last_index] = dst;
		len_array[last_index] = len;

		dvddone = 0;
		ret = bwDVD_LowRead(dst,len,offset/4<<2,__dvd_readidcb);
		strcpy(dvderror, "Apploader loop");
		DVD_CHECK();
		
		DCFlushRange(dst, len);
		ICInvalidateRange(dst, len);
	}
	print_status("Apploader.......");
	
	dst_array[last_index+1] = (void *)0x81800000;
	int j = 0;
	fststart = 0;
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

	appentrypoint = (u32)app_final();
	
	u32 doloffset = 0;
	
	if (alternativedolselect >= 2) // Load .dol from disc
	{
		getdolnames();

		// The .wdm for alt .dol disc+ is loaded earlier
		
		if (alternativedolselect == 2 || dolmenufallback)
		{
			if (createdolmenubuffer(dolcount+2) < 0)
			{
				print_status("Out of memory");
				wait(3);
				dolmenubuffer = NULL;
			} else
			{
				dolmenubuffer[0].count = dolcount+1;
				dolmenubuffer[1].parameter = 1;
				strcpy(dolmenubuffer[1].dolname, "main");
				strcpy(dolmenubuffer[1].name, "Boot: <main.dol>");
				
				for (i=0;i<dolcount;i++)
				{
					dolmenubuffer[i+2].parameter = 1;
					strcpy(dolmenubuffer[i+2].dolname, fstfilename(dolindex[i]));
					strcpy(dolmenubuffer[i+2].name, "Boot: <");
					strcat(dolmenubuffer[i+2].name, dolmenubuffer[i+2].dolname);
					strcat(dolmenubuffer[i+2].name, ">");
				}
			}
		}
	
		if (dolmenubuffer != NULL)
		{
			showdolmenu();	
			
			doloffset = getdoloffsetbyname(dolname);
		}

		if (doloffset == 0)
		{
			print_status("Booting main.dol ...");
		} else
		{
			print_status("Booting %s ...", dolname);
		}
		wait(1);
		
	} else
	if (alternativedolselect == 1) // Load .dol from storage
	{
		for (i=3;i<fststart;i++)
		{
			memset(dst_array[i], 0, len_array[i]);
		}
		
		appentrypoint = Load_Dol_from_storage();
		
		if (appentrypoint == 0)
		{
			return 0;
		}
		print_status(".dol loaded");
		wait(1);
	}
	
	if (doloffset != 0)
	{
		for (i=3;i<fststart;i++)
		{
			memset(dst_array[i], 0, len_array[i]);
		}

		appentrypoint = Load_Dol_from_disc(doloffset);

		if (appentrypoint == 0)
		{
			return 0;
		}
		print_status(".dol loaded");
		wait(1);
	} else
	{
		for (i=3;i<fststart;i++)
		{
			maindolpatches(dst_array[i], len_array[i]);
		}	
	}
	
	apply_wip_patches();

	// Ocarina stuff gameconfig.txt, important for Brawl+ for example
	app_pokevalues();

	if (hookpatched == false && use_wii_Ocarina_engine())
	{
		print_status("Hook error");
		wait(3);
	}		
	
	toy_patch_video_modes_to(rmode);

	if (decrypted_disc)
	{
		ret = DVD_Identify(&tmdios);

		if (ret < 0)
		{
			print_status("DVD_Identify failed, ret = %d", ret);
			return 0;
		}
	}

	if (((decrypted_disc == true) || (usecorrectiosoption == 0)) && (Sneek_Mode == false))
	{
		print_status("The requested IOS is: IOS = %u", tmdios);
		wait(2);
	} else
	{
		u8 ios_to_load = 0;

		if (usecorrectiosoption == 1 || Sneek_Mode == true)
		{
			ios_to_load = tmdios;
		} else // usecorrectiosoption == 2
		{
			ios_to_load = find_cIOS_with_base(tmdios);
		}

		if (ios_to_load == 0)
		{
			print_status("The requested IOS is: IOS = %u", tmdios);
			wait(2);
		} else
		{
			print_status("Loading IOS%u.", ios_to_load);
			wait(1);
			
			bwDVD_LowClose();
			WPAD_Shutdown();
			ISFS_Deinitialize();
			
			print_status("Loading IOS%u..", ios_to_load);

			block_ios_reload(false);
			
			IOS_ReloadIOS(ios_to_load);
			
			block_ios_reload(false);
			set_cIOS_stealth_mode(false);

			print_status("Loading IOS%u...", ios_to_load);
			
			ret = init_drive();
			if (ret >= 0)
			{
				ret = open_partition();
			} else			
			{
				block_ios_reload(false);
				
				if (Sneek_Mode == false)
				{
					IOS_ReloadIOS(CIOS_VERSION);
					
					block_ios_reload(false);
					set_cIOS_stealth_mode(false);
				} else
				{
					IOS_ReloadIOS(36);
				}
				
				// Reinit controls...
				PAD_Init();
				WPAD_Init();
				WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
				
				return ret;
			}		
		}
	}

	tell_cIOS_to_return_to_channel();

	block_ios_reload( ((blockiosreloadselect != 0) && (usecorrectiosoption != 1) && (decrypted_disc == false)) );
	
	if (use_wii_Ocarina_engine() || langselect != 0 || videopatchselect != 0 || viselect != 0 || countrystringselect != 0 || wip_patch_applied == true)
	{
		print_status("Warning: Patches enabled");
		wait(2);
	}	
	
	print_status("Starting Game...");
	wait(2);

	// Remove 002
	if (tmdios == CIOS_VERSION)
	{
		*(u32 *)0x80003188 = *(u32 *)0x80003140;
	} else
	{
		*(u8 *)0x80003141 = tmdios;
		*(u8 *)0x80003189 = tmdios;
		*(u16 *)0x80003142 = 0xffff;
		*(u16 *)0x8000318A = 0x0001;
	}

	DCFlushRange((void*)0x80000000, 0x3f00);
	ICInvalidateRange((void *)0x80000000, 0x3f00);

	set_videomode();

	set_cIOS_stealth_mode(true);	// Once this is done, and ES_Idenity or Open_Partition was called, there's no way to disable it anymore

	shutdown_all();
	
	*(u32*)0xCC003024 = dolparameter; /* Originally from tueidj */

/*
	if (use_wii_Ocarina_engine())
	{
		jumpentry(appentrypoint);
	} else
	{
		entry_point p_entry = (entry_point)appentrypoint;
		p_entry();
	}
*/	

	// Try to fool anti cheat protections by clearing NeoGamma's used memory before jumping to the entrypoint
	memsetloaderbuffer[0] = memsetloaderbuffer[0] | 0x80df;	// memset 0x80dfff00 till 
	memsetloaderbuffer[1] = memsetloaderbuffer[1] | 0xff00;

	memsetloaderbuffer[2] = memsetloaderbuffer[2] | 0x8120;	// 0x81200000
	memsetloaderbuffer[3] = memsetloaderbuffer[3] | 0x0000;

	memsetloaderbuffer[14] = memsetloaderbuffer[14] | (appentrypoint >> 16);
	memsetloaderbuffer[15] = memsetloaderbuffer[15] | (appentrypoint & 0xffff);

	memcpy((void *)0x80df0000, memsetloaderbuffer, sizeof(memsetloaderbuffer));
	DCFlushRange((void *)0x80df0000, 0x100);
	ICInvalidateRange((void *)0x80df0000, 0x100);
	
	appentrypoint = 0x80df0000;
	
	if (use_wii_Ocarina_engine())
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"lis %r3, 0x8000\n"
			"ori %r3, %r3, 0x18A8\n"
			"nop\n"									// We Dare checks if it finds this code in memory...
			"mtctr %r3\n"
			"bctr\n"
		);
	} else
	{
		__asm__(
			"lis %r3, appentrypoint@h\n"
			"ori %r3, %r3, appentrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}

	return 1;
}
