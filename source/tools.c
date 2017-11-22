/*
 *  Copyright (C) 2010 WiiPower
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
#include <time.h>
#include <gccore.h>
#include <ogcsys.h>
#include <sys/unistd.h>
#include <wiiuse/wpad.h>
//#include <ogc/lwp_watchdog.h>
//#include <time.h>
#include <malloc.h>
#include <stdarg.h>

#include "tools.h"
#include "storage.h"
#include "geckomenu.h"
#include "font.h"
#include "background.h"
#include "libcios.h"
#include "sha1.h"
#include "dvd_broadway.h"
#include "config.h"

extern s32 __IOS_ShutdownSubsystems();
extern void __exception_closeall();

u32 *xfb = NULL;
static GXRModeObj *rmode = NULL;
u32 screenheight;

bool use_wii_Ocarina_engine()
{
	return (
(wiihookselect != 0 || (wii2ndhookselect > 0 && wii2ndhookselect < 8)) 
&&
(wiiocarinaselect != 0 || wiidebuggerselect != 0) 
 
 );
}

void init_video()
{
	/* Init Video, Pad, Fonts */
	VIDEO_Init();
	init_font();

	rmode = VIDEO_GetPreferredMode(NULL);
	
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);

	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	VIDEO_SetNextFramebuffer(xfb);

	VIDEO_SetBlack(FALSE);

	VIDEO_Flush();

	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
    VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	
	screenheight = rmode->xfbHeight;
}

s32 read_file_from_nand(char *filepath, u8 **buffer, u32 *filesize)
{
	s32 Fd;
	int ret;

	if (buffer == NULL)
	{
		//printf("NULL Pointer\n");
		return -1;
	}

	Fd = ISFS_Open(filepath, ISFS_OPEN_READ);
	if (Fd < 0)
	{
		//printf("ISFS_Open %s failed %d\n", filepath, Fd);
		return Fd;
	}

	fstats *status;
	status = allocate_memory(sizeof(fstats));
	if (status == NULL)
	{
		//printf("Out of memory for status\n");
		return -1;
	}
	
	ret = ISFS_GetFileStats(Fd, status);
	if (ret < 0)
	{
		//printf("ISFS_GetFileStats failed %d\n", ret);
		ISFS_Close(Fd);
		free(status);
		return -1;
	}
	
	*buffer = allocate_memory(status->file_length);
	if (*buffer == NULL)
	{
		//printf("Out of memory for buffer\n");
		ISFS_Close(Fd);
		free(status);
		return -1;
	}
		
	ret = ISFS_Read(Fd, *buffer, status->file_length);
	if (ret < 0)
	{
		//printf("ISFS_Read failed %d\n", ret);
		ISFS_Close(Fd);
		free(status);
		free(*buffer);
		return ret;
	}
	
	ISFS_Close(Fd);

	*filesize = status->file_length;
	free(status);

	if (*filesize > 0)
	{
		DCFlushRange(*buffer, *filesize);
		ICInvalidateRange(*buffer, *filesize);
	}

	return 0;
}

s32 write_file_to_nand(char *filepath, u8 *buffer, u32 filesize)
{
	s32 Fd;
	int ret;

	if (buffer == NULL || filesize == 0)
	{
		//printf("NULL Pointer\n");
		return -1;
	}

	ISFS_Delete(filepath);
	
	ret = ISFS_CreateFile(filepath, 0, 3, 3, 3);
	if (ret < 0)
	{
		//printf("ISFS_CreateFile failed %d\n", ret);
		return ret;
	}

	Fd = ISFS_Open(filepath, ISFS_OPEN_RW);
	if (Fd < 0)
	{
		//printf("ISFS_Open %s failed %d\n", filepath, Fd);
		return Fd;
	}

	ret = ISFS_Write(Fd, buffer, filesize);
	if (ret < 0)
	{
		//printf("ISFS_Write failed %d\n", ret);
		ISFS_Close(Fd);
		return ret;
	}
	
	ISFS_Close(Fd);

	return 0;
}



#define info_number 35

static u32 hashes[info_number][5] = {
{0x20e60607, 0x4e02c484, 0x2bbc5758, 0xee2b40fc, 0x35a68b0a},		// cIOSrev13a
{0x620c57c7, 0xd155b67f, 0xa451e2ba, 0xfb5534d7, 0xaa457878}, 		// cIOSrev13b
{0x3c968e54, 0x9e915458, 0x9ecc3bda, 0x16d0a0d4, 0x8cac7917},		// cIOS37 rev18
{0xe811bca8, 0xe1df1e93, 0x779c40e6, 0x2006e807, 0xd4403b97},		// cIOS38 rev18
{0x697676f0, 0x7a133b19, 0x881f512f, 0x2017b349, 0x6243c037},		// cIOS57 rev18
{0x34ec540b, 0xd1fb5a5e, 0x4ae7f069, 0xd0a39b9a, 0xb1a1445f},		// cIOS60 rev18
{0xd98a4dd9, 0xff426ddb, 0x1afebc55, 0x30f75489, 0x40b27ade},		// cIOS70 rev18
{0x0a49cd80, 0x6f8f87ff, 0xac9a10aa, 0xefec9c1d, 0x676965b9},		// cIOS37 rev19
{0x09179764, 0xeecf7f2e, 0x7631e504, 0x13b4b7aa, 0xca5fc1ab},		// cIOS38 rev19
{0x6010d5cf, 0x396415b7, 0x3c3915e9, 0x83ded6e3, 0x8f418d54},		// cIOS57 rev19
{0x589d6c4f, 0x6bcbd80a, 0xe768f258, 0xc53a322c, 0xd143f8cd},		// cIOS60 rev19
{0x8969e0bf, 0x7f9b2391, 0x31ecfd88, 0x1c6f76eb, 0xf9418fe6},		// cIOS70 rev19
{0x30aeadfe, 0x8b6ea668, 0x446578c7, 0x91f0832e, 0xb33c08ac},		// cIOS36 rev20
{0xba0461a2, 0xaa26eed0, 0x482c1a7a, 0x59a97d94, 0xa607773e},		// cIOS37 rev20
{0xb694a33e, 0xf5040583, 0x0d540460, 0x2a450f3c, 0x69a68148},		// cIOS38 rev20
{0xf6058710, 0xfe78a2d8, 0x44e6397f, 0x14a61501, 0x66c352cf},		// cIOS53 rev20
{0xfa07fb10, 0x52ffb607, 0xcf1fc572, 0xf94ce42e, 0xa2f5b523},		// cIOS55 rev20
{0xe30acf09, 0xbcc32544, 0x490aec18, 0xc276cee6, 0x5e5f6bab},		// cIOS56 rev20
{0x595ef1a3, 0x57d0cd99, 0x21b6bf6b, 0x432f6342, 0x605ae60d},		// cIOS57 rev20
{0x687a2698, 0x3efe5a08, 0xc01f6ae3, 0x3d8a1637, 0xadab6d48},		// cIOS60 rev20
{0xea6610e4, 0xa6beae66, 0x887be72d, 0x5da3415b, 0xa470523c},		// cIOS61 rev20
{0x64e1af0e, 0xf7167fd7, 0x0c696306, 0xa2035b2d, 0x6047c736},		// cIOS70 rev20
{0x0df93ca9, 0x833cf61f, 0xb3b79277, 0xf4c93cd2, 0xcd8eae17},		// cIOS80 rev20
{0x074dfb39, 0x90a5da61, 0x67488616, 0x68ccb747, 0x3a5b59b3}, 		// cIOS36 rev21
{0x6956a016, 0x59542728, 0x8d2efade, 0xad8ed01e, 0xe7f9a780}, 		// cIOS37 rev21
{0xdc8b23e6, 0x9d95fefe, 0xac10668a, 0x6891a729, 0x2bdfbca0}, 		// cIOS38 rev21
{0xaa2cdd40, 0xd628bc2e, 0x96335184, 0x1b51404c, 0x6592b992}, 		// cIOS53 rev21
{0x4a3d6d15, 0x014f5216, 0x84d65ffe, 0x6daa0114, 0x973231cf}, 		// cIOS55 rev21
{0xca883eb0, 0x3fe8e45c, 0x97cc140c, 0x2e2d7533, 0x5b369ba5}, 		// cIOS56 rev21
{0x469831dc, 0x918acc3e, 0x81b58a9a, 0x4493dc2c, 0xaa5e57a0}, 		// cIOS57 rev21
{0xe5af138b, 0x029201c7, 0x0c1241e7, 0x9d6a5d43, 0x37a1456a}, 		// cIOS58 rev21
{0x0fdee208, 0xf1d031d3, 0x6fedb797, 0xede8d534, 0xd3b77838}, 		// cIOS60 rev21
{0xaf588570, 0x13955a32, 0x001296aa, 0x5f30e37f, 0x0be91316}, 		// cIOS61 rev21
{0x50deaba2, 0x9328755c, 0x7c2deac8, 0x385ecb49, 0x65ea3b2b}, 		// cIOS70 rev21
{0x811b6a0b, 0xe26b9419, 0x7ffd4930, 0xdccd6ed3, 0x6ea2cdd2}, 		// cIOS80 rev21

};

static char infos[info_number][24] = {
{"cIOS rev13a"},
{"cIOS rev13b"},
{"cIOS37rev18"},
{"cIOS38rev18"},
{"cIOS57rev18"},
{"cIOS60rev18"},
{"cIOS70rev18"},
{"cIOS37rev19"},
{"cIOS38rev19"},
{"cIOS57rev19"},
{"cIOS60rev19"},
{"cIOS70rev19"},
{"cIOS36rev20"},
{"cIOS37rev20"},
{"cIOS38rev20"},
{"cIOS53rev20"},
{"cIOS55rev20"},
{"cIOS56rev20"},
{"cIOS57rev20"},
{"cIOS60rev20"},
{"cIOS61rev20"},
{"cIOS70rev20"},
{"cIOS80rev20"},
{"cIOS36rev21"},
{"cIOS37rev21"},
{"cIOS38rev21"},
{"cIOS53rev21"},
{"cIOS55rev21"},
{"cIOS56rev21"},
{"cIOS57rev21"},
{"cIOS58rev21"},
{"cIOS60rev21"},
{"cIOS61rev21"},
{"cIOS70rev21"},
{"cIOS80rev21"},
};	
	
s32 brute_tmd(tmd *p_tmd) 
{
	u16 fill;
	for(fill=0; fill<65535; fill++) 
	{
		p_tmd->fill3=fill;
		sha1 hash;
		SHA1((u8 *)p_tmd, TMD_SIZE(p_tmd), hash);;
		  
		if (hash[0]==0) 
		{
			return 0;
		}
	}
	return -1;
}	

void identify_IOS(u8 ios_slot, u8 *ios_base, u32 *ios_revision, char *ios_string)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);
	u8 *buffer = NULL;
	u32 filesize;
	signed_blob *TMD = NULL;
	tmd *t = NULL;
	u32 TMD_size = 0;
	u32 i;
	iosinfo_t *iosinfo = NULL;

	u8 temp_ios_base = 0;
	u32 temp_ios_revision = 0;
	
	// Backup in case the other methods fail
	if (ios_string != NULL)
	{
		if (ios_slot == IOS_GetVersion())
		{
			sprintf(ios_string, "IOS%u (Rev %u)", IOS_GetVersion(), IOS_GetRevision());
		} else
		{
			sprintf(ios_string, "IOS%u", ios_slot);
		}
	}

	ISFS_Initialize();
	
	sprintf(filepath, "/title/%08x/%08x/content/title.tmd", 0x00000001, ios_slot);
	s32 ret = read_file_from_nand(filepath, (void *)(&TMD), &TMD_size);
	
	if (ret >= 0)
	{
		// Try to identify the cIOS by the info put in by the installer/ModMii
		sprintf(filepath, "/title/%08x/%08x/content/%08x.app", 0x00000001, ios_slot, *(u8 *)((u32)TMD+0x1E7));
		ret = read_file_from_nand(filepath, &buffer, &filesize);
		
		//ISFS_Deinitialize();	It is executed now before an IOS Reload, this fixes a weird freezing in Castlevania Judgment when a wii mote is used
		
		iosinfo = (iosinfo_t *)(buffer);
		if (ret >= 0 && iosinfo != NULL && iosinfo->magicword == 0x1ee7c105 && iosinfo->magicversion == 1)
		{
			temp_ios_base = iosinfo->baseios;
			temp_ios_revision = iosinfo->version;
	
			if (ios_string != NULL)
			{
				sprintf(ios_string, "%s%uv%u%s (%u)", iosinfo->name, iosinfo->baseios, iosinfo->version, iosinfo->versionstring, ios_slot);				
				// Example: "d2x56v5beta2 (249)"
			}
			if (buffer != 0)
			{
				free(buffer);
			}
		} else
		{	
			// Crappy hash method
			t = (tmd*)SIGNATURE_PAYLOAD(TMD);
			t->title_id = ((u64)(1) << 32) | 249;	// The hashes were made with the cIOS installed as IOS249
			brute_tmd(t);		

			sha1 hash;
			SHA1((u8 *)TMD, TMD_size, hash);;

			for (i = 0;i < info_number;i++)
			{
				if (memcmp((void *)hash, (u32 *)&hashes[i], sizeof(sha1)) == 0)
				{
					switch (i)
					{
						case 0:
						case 1:
						case 3:						
						case 8:						
						case 14:						
						case 25:						
							temp_ios_base = 38;
						break;
						
						case 2:
						case 7:
						case 13:
						case 24:
							temp_ios_base = 37;
						break;

						case 4:
						case 9:
						case 18:
						case 29:
							temp_ios_base = 57;
						break;

						case 5:
						case 10:
						case 19:
						case 31:
							temp_ios_base = 60;
						break;
						
						case 6:
						case 11:
						case 21:
						case 33:
							temp_ios_base = 70;
						break;
						
						case 12:
						case 23:
							temp_ios_base = 36;
						break;

						case 15:
						case 26:
							temp_ios_base = 53;
						break;

						case 16:
						case 27:
							temp_ios_base = 55;
						break;

						case 17:
						case 28:
							temp_ios_base = 56;
						break;

						case 20:
						case 32:
							temp_ios_base = 61;
						break;
						
						case 22:
						case 34:
							temp_ios_base = 80;
						break;

						case 30:
							temp_ios_base = 58;
						break;
					}
					
					if (ios_string != NULL)
					{
						sprintf(ios_string, "%s (%u)", (char *)&infos[i], ios_slot);				
					}
				}
			}
		}
		free(TMD);
	}
	
	if (ios_base != NULL)
	{
		*ios_base = temp_ios_base;
	}
	
	if (ios_revision != NULL)
	{
		*ios_revision = temp_ios_revision;		// Only gets a value if a cIOS is identified
	}
}


u8 find_cIOS_with_base(u8 requested_ios_base)
{
	u64 *buf;
	s32 i, ret;
	u32 tcnt = 0;
	
	u8 current_ios_base;
	u32 current_ios_revision;
	
	u8 temp_ios_base;
	u32 temp_ios_revision;
	
	identify_IOS(CIOS_VERSION, &current_ios_base, &current_ios_revision, NULL);	
	
	if (requested_ios_base == current_ios_base || current_ios_base == 0)
	{
		//print_status("current_ios_base = %u", current_ios_base);
		//wait(2);
		return 0; // Using the best cIOS already or couldn't identify the cIOS
	}
	
	//Get stored IOS versions.
	ret = ES_GetNumTitles(&tcnt);
	if(ret < 0)
	{
		print_status("ES_GetNumTitles: Error! (result = %d)", ret);
		wait(2);
		return 0;
	}
	buf = memalign(32, sizeof(u64) * tcnt);
	if (buf == NULL)
	{
		print_status("Out of memory(find cIOS)");
		wait(2);
		return 0;
	}
	ret = ES_GetTitles(buf, tcnt);
	if(ret < 0)
	{
		print_status("ES_GetTitles: Error! (result = %d)", ret);
		wait(2);
		return 0;
	}

	for (i = 0; i < tcnt; i++)
	{
		if (*((u32 *)(&(buf[i]))) == 1 && (u32)buf[i] >= 240 && (u32)buf[i] < 254) // Only check IOS240-253
		{
			identify_IOS((u8)buf[i], &temp_ios_base, &temp_ios_revision, NULL);	
			
			if (temp_ios_revision == current_ios_revision && temp_ios_base == requested_ios_base)
			{
				u8 temp_result = (u8)buf[i];
				free(buf);
				return temp_result;
			}
		}
	}
	free(buf);
	return 0;
}



char ios_info[64];
	
void show_banner()
{
	static bool first_run = true;
	
	DrawBackground(rmode); // Show banner
	VIDEO_WaitVSync();

	if (first_run)
	{
		u32 position;
		if (screenheight == 480)
		{
			position = screenheight - 60;
		} else
		{
			position = screenheight - 36;
		}
		drawicon(xfb, 18, position - 3, 32/2, 27,0); // Show Gecko icon

		if (Sneek_Mode == false)
		{
			identify_IOS(CIOS_VERSION, NULL, NULL, ios_info);
			
			write_font(68, position, true, "NeoGamma R9 b56, %s", ios_info);
		} else
		{
			write_font(68, position, true, "NeoGamma R9 b56, Sneek+DI");
		}

		first_run = false;
	}	
	VIDEO_WaitVSync();
}

void wait(u32 s)
{
	time_t t;
	t = time(NULL) + s; 
	while (time(NULL) < t)
	{
		// ;
		Verify_Flags(); // Standby and Reset
	}
}

void clear_status()
{
	clearscreen(xfb, 0, 210, 320, 24, 0x00800080);
}

void print_status(const char *Format, ...)
{
	char Buffer[256];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);

	clear_status();
	write_font(185, 210,true, Buffer);
	VIDEO_WaitVSync();
}

void *allocate_memory(u32 size)
{
	return memalign(32, (size+31)&(~31) );
}

void Verify_Flags()
{
	if (Power_Flag)
	{
		WPAD_Shutdown();
		STM_ShutdownToStandby();
	}
	if (Reset_Flag)
	{
		WPAD_Shutdown();
		STM_RebootSystem();
	}
}

void waitforbuttonpress(u32 *out, u32 *outGC, u32 *outCover)
{
	u32 pressed = 0;
	u32 pressedGC = 0;
	u32 cover_status;
	u32 old_cover_status;
	bwDVD_GetCoverStatus(&old_cover_status);

	while (true)
	{
		Verify_Flags(); // Standby and Reset

		WPAD_ScanPads();
		pressed = WPAD_ButtonsDown(0) | WPAD_ButtonsDown(1) | WPAD_ButtonsDown(2) | WPAD_ButtonsDown(3);

		PAD_ScanPads();
		pressedGC = PAD_ButtonsDown(0) | PAD_ButtonsDown(1) | PAD_ButtonsDown(2) | PAD_ButtonsDown(3);

		bwDVD_GetCoverStatus(&cover_status);
		
		if (pressed || pressedGC || (cover_status != old_cover_status)) 
		{
			if (pressedGC)
			{
				// Without waiting you can't select anything
				usleep (20000);
			}
			if (out) *out = pressed;
			if (outGC) *outGC = pressedGC;
			if (outCover) *outCover = cover_status;
			return;
		}
	}
}


void shutdown_all()
{
	storage_shutdown();
	WPAD_Shutdown();
	IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();
}

u32 be32(const u8 *data)
{
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}
