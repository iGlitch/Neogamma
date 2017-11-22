/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
 *  Copyright (C) 2009 WiiPower
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
#include <ogc/es.h>
#include <ogcsys.h>
#include <sys/unistd.h>
#include "apploader.h"
#include "patchcode.h"
#include "tools.h"
#include "geckomenu.h"
#include "font.h"
#include "identify.h"
#include "sysmenu.h"
#include "multidol.h"
#include "config.h"
#include "codes.h"


extern u32 *xfb;
extern const unsigned char kenobiwii[];
extern const int kenobiwii_size;
void _unstub_start();

void* 				Address_Array[64];			//TODO: variable size
int					Section_Size_Array[64];		//TODO: variable size
int 				last_index;

typedef struct _dolheader
{
	u32 text_pos[7];
	u32 data_pos[11];
	u32 text_start[7];
	u32 data_start[11];
	u32 text_size[7];
	u32 data_size[11];
	u32 bss_start;
	u32 bss_size;
	u32 entry_point;
} dolheader;

void print_status(const char *Format, ...);

/*
Replaced by bendiosreload
u8 iosReloadOrig[] = {
	0x7f, 0x06, 0xc3, 0x78,
	0x7f, 0x25, 0xcb, 0x78,
	0x38, 0x7e, 0x02, 0xc0,
	0x4c, 0xc6, 0x31, 0x82
};

u8 iosReloadPatch[] = {
	0x3b, 0x20, 0x00, 0x01,
	0x3b, 0x00, 0x00, 0xf9
};

void iosReloadFix(void *addr, u32 len)
{
	u8 *addr_start = addr;
	u8 *addr_end = addr+len-sizeof(iosReloadOrig);

	iosReloadPatch[7] = CIOS_VERSION;

	while (addr_start < addr_end)
	{
		if (memcmp(addr_start, iosReloadOrig, sizeof(iosReloadOrig))==0) 
		{
			memcpy(addr_start, iosReloadPatch, sizeof(iosReloadPatch));
			return;
		}
		addr_start += 4;
	}
}
*/


void bendiosreload(u16 version)
{
	switch (version)
	{
		case 289: // NTSC-U 3.2
			*(u32 *)0x81375D38 = 0x3B200001;
			*(u32 *)0x81375D3C = 0x3B000000 + CIOS_VERSION;
		break;		
		
		case 290: // PAL 3.2
			*(u32 *)0x81375DE0 = 0x3B200001;
			*(u32 *)0x81375DE4 = 0x3B000000 + CIOS_VERSION;
		break;
		
		case 417: // NTSC-U 4.0
			// TODO
		break;
		
		case 418: // PAL 4.0
			*(u32 *)0x8137B8C8 = 0x3B200001;
			*(u32 *)0x8137B8CC = 0x3B000000 + CIOS_VERSION;
		break;		
		
		case 448: //NTSC-J 4.1
			*(u32 *)0x8137ADD0 = 0x3B200001;
			*(u32 *)0x8137ADD4 = 0x3B000000 + CIOS_VERSION;
		break;
		
		case 449: //NTSC-U 4.1
			*(u32 *)0x8137B91C = 0x3B200001;
			*(u32 *)0x8137B920 = 0x3B000000 + CIOS_VERSION;
		break;		
		
		case 450: // PAL 4.1
			*(u32 *)0x8137B9C4 = 0x3B200001;
			*(u32 *)0x8137B9C8 = 0x3B000000 + CIOS_VERSION;
		break;

		case 480: // NTSC-J 4.2
			*(u32 *)0x8137B230 = 0x3B200001;
			*(u32 *)0x8137B234 = 0x3B000000 + CIOS_VERSION;
		break;

		case 481: // NTSC-U 4.2
			*(u32 *)0x8137BD7C = 0x3B200001;
			*(u32 *)0x8137BD80 = 0x3B000000 + CIOS_VERSION;
		break;

		case 482: // PAL 4.2
			*(u32 *)0x8137BE24 = 0x3B200001;
			*(u32 *)0x8137BE28 = 0x3B000000 + CIOS_VERSION;
		break;

		case 486: // KOR 4.2
			*(u32 *)0x8137B110 = 0x3B200001;
			*(u32 *)0x8137B114 = 0x3B000000 + CIOS_VERSION;
		break;

		default:
		break;
	}
}


void skipdiscupdates(u16 version)
{
	switch (version)
	{
		case 289: // NTSC-U 3.2
			*(u32 *)0x813791A0 = 0x60000000;
			*(u32 *)0x813791A4 = 0x60000000;
		break;		
		
		case 290: // PAL 3.2
			*(u32 *)0x813791F0 = 0x60000000;
			*(u32 *)0x813791F4 = 0x60000000;
		break;
		
		case 417: // NTSC-U 4.0
			*(u32 *)0x8137ED54 = 0x60000000;
			*(u32 *)0x8137ED58 = 0x60000000;
		break;
		
		case 418: // PAL 4.0
			*(u32 *)0x8137EE54 = 0x60000000;
			*(u32 *)0x8137EE58 = 0x60000000;
			*(u32 *)0x8137EDFC = 0x60000000;
			*(u32 *)0x8137EE00 = 0x60000000;
			*(u32 *)0x8137ED54 = 0x60000000;
			*(u32 *)0x8137ED58 = 0x60000000;
		break;		
		
		case 448: //NTSC-J 4.1
			*(u32 *)0x8137E35C = 0x60000000;
			*(u32 *)0x8137E360 = 0x60000000;
			*(u32 *)0x8137E304 = 0x60000000;
			*(u32 *)0x8137E308 = 0x60000000;
			*(u32 *)0x8137E25C = 0x60000000;
			*(u32 *)0x8137E260 = 0x60000000;
		break;
		
		case 449: //NTSC-U 4.1
			*(u32 *)0x8137EEA8 = 0x60000000;
			*(u32 *)0x8137EEAC = 0x60000000;
			*(u32 *)0x8137EE50 = 0x60000000;
			*(u32 *)0x8137EE54 = 0x60000000;
			*(u32 *)0x8137EDA8 = 0x60000000;
			*(u32 *)0x8137EDAC = 0x60000000;
		break;		
		
		case 450: // PAL 4.1
			*(u32 *)0x8137EF50 = 0x60000000;
			*(u32 *)0x8137EF54 = 0x60000000;
			*(u32 *)0x8137EEF8 = 0x60000000;
			*(u32 *)0x8137EEFC = 0x60000000;
			*(u32 *)0x8137EE50 = 0x60000000;
			*(u32 *)0x8137EE54 = 0x60000000;	
		break;

		case 480: // NTSC-J 4.2
			*(u32 *)0x8137EA70 = 0x60000000;
			*(u32 *)0x8137EA74 = 0x60000000;
			*(u32 *)0x8137EA18 = 0x60000000;
			*(u32 *)0x8137EA1C = 0x60000000;
			*(u32 *)0x8137E970 = 0x60000000;
			*(u32 *)0x8137E974 = 0x60000000;	
		break;

		case 481: // NTSC-U 4.2
			*(u32 *)0x8137F5BC = 0x60000000;
			*(u32 *)0x8137F5C0 = 0x60000000;
			*(u32 *)0x8137F564 = 0x60000000;
			*(u32 *)0x8137F568 = 0x60000000;
			*(u32 *)0x8137F4BC = 0x60000000;
			*(u32 *)0x8137F4C0 = 0x60000000;	
		break;

		case 482: // PAL 4.2
			*(u32 *)0x8137F664 = 0x60000000;
			*(u32 *)0x8137F668 = 0x60000000;
			*(u32 *)0x8137F60C = 0x60000000;
			*(u32 *)0x8137F610 = 0x60000000;
			*(u32 *)0x8137F564 = 0x60000000;
			*(u32 *)0x8137F568 = 0x60000000;	
		break;

		case 486: // KOR 4.2
			*(u32 *)0x8137E950 = 0x60000000;
			*(u32 *)0x8137E954 = 0x60000000;
			*(u32 *)0x8137E8F8 = 0x60000000;
			*(u32 *)0x8137E8FC = 0x60000000;
			*(u32 *)0x8137E850 = 0x60000000;
			*(u32 *)0x8137E854 = 0x60000000;	
		break;
		
		default:
		break;
	}
}


void region_free(u16 version)
{
	switch (version)
	{
		case 289: // NTSC-U 3.2
			*(u32 *)0x81379818 = 0x38600001;
			*(u32 *)0x81377AC4 = 0x38000001;
			*(u32 *)0x81506ECC = 0x38000001;
			*(u32 *)0x813AE7A0 = 0x48000028;
		break;		
		
		case 290: // PAL 3.2
			*(u32 *)0x813798C0 = 0x38600001;
			*(u32 *)0x81377B6C = 0x38000001;
			*(u32 *)0x81506FC8 = 0x38000001;
			*(u32 *)0x813AE89C = 0x48000028;
		break;
		
		case 417: // NTSC-U 4.0
			*(u32 *)0x8137D378 = 0x38000001;
			*(u32 *)0x8137D63C = 0x38000001;
			*(u32 *)0x8137F40C = 0x38600001;
			*(u32 *)0x8152D580 = 0x38000001;
			*(u32 *)0x813B5FF4 = 0x48000028;
		break;
		
		case 418: // PAL 4.0
			*(u32 *)0x8152D194 = 0x60000000;
			*(u32 *)0x81530640 = 0x4E800020;
			*(u32 *)0x8152D67C = 0x38000001;
			*(u32 *)0x813B60F0 = 0x48000280;
			*(u32 *)0x81530640 = 0x4E800020;
			*(u32 *)0x8137D428 = 0x60000000;
			*(u32 *)0x8137D704 = 0x4800001C;
			*(u32 *)0x8137D6E4 = 0x38000001;
			*(u32 *)0x8137D420 = 0x38000001;
			*(u32 *)0x8137F4B4 = 0x38600001;
			*(u32 *)0x8137D428 = 0x900DA5D8;
			*(u32 *)0x8137D42C = 0x38000032;
		break;		
		
		case 448: //NTSC-J 4.1
			*(u32 *)0x8137C930 = 0x60000000;
			*(u32 *)0x8137CC0C = 0x4800001C;
			*(u32 *)0x8137CBEC = 0x38000001;
			*(u32 *)0x8137C928 = 0x38000001;
			*(u32 *)0x8137E9BC = 0x38600001;

			*(u32 *)0x815560FC = 0x60000000;
			*(u32 *)0x815595A8 = 0x4E800020;
			*(u32 *)0x815565E4 = 0x38000001;
			*(u32 *)0x813B53DC = 0x48000028;
		break;
		
		case 449: //NTSC-U 4.1
			*(u32 *)0x8137D47C = 0x60000000;
			*(u32 *)0x8137D758 = 0x4800001C;
			*(u32 *)0x8137D738 = 0x38000001;
			*(u32 *)0x8137D474 = 0x38000001;
			*(u32 *)0x8137F508 = 0x38600001;

			*(u32 *)0x8152D1C0 = 0x60000000;
			*(u32 *)0x8153066C = 0x4E800020;
			*(u32 *)0x8152D6A8 = 0x38000001;
			*(u32 *)0x813B60F0 = 0x48000028;
		break;		
		
		case 450: // PAL 4.1
			*(u32 *)0x8137D524 = 0x60000000;
			*(u32 *)0x8137D800 = 0x4800001C;
			*(u32 *)0x8137D7E0 = 0x38000001;
			*(u32 *)0x8137D51C = 0x38000001;
			*(u32 *)0x8137F5B0 = 0x38600001;

			*(u32 *)0x8152D2BC = 0x60000000;
			*(u32 *)0x81530768 = 0x4E800020;
			*(u32 *)0x8152D7A4 = 0x38000001;
			*(u32 *)0x813B61EC = 0x48000028;
		break;

		case 480: // NTSC-J 4.2
			*(u32 *)0x8137D09C = 0x4800001C;
			*(u32 *)0x8137D07C = 0x38000001;
			*(u32 *)0x8137CDB8 = 0x38000001;
			*(u32 *)0x8137F138 = 0x38600001;
			*(u32 *)0x8137D8F0 = 0x60000000;

			*(u32 *)0x815569C8 = 0x60000000;
			*(u32 *)0x81559E74 = 0x4E800020;
			*(u32 *)0x81556EB0 = 0x38000001;
			*(u32 *)0x813B5B98 = 0x48000028;
		break;

		case 481: // NTSC-U 4.2
			*(u32 *)0x8137DBE8 = 0x4800001C;
			*(u32 *)0x8137DBC8 = 0x38000001;
			*(u32 *)0x8137D904 = 0x38000001;
			*(u32 *)0x8137FC84 = 0x38600001;
			*(u32 *)0x8137E43C = 0x60000000;

			*(u32 *)0x8152DA88 = 0x60000000;
			*(u32 *)0x81530F34 = 0x4E800020;
			*(u32 *)0x8152DF70 = 0x38000001;
			*(u32 *)0x813B68AC = 0x48000028;
		break;

		case 482: // PAL 4.2
			*(u32 *)0x8137DC90 = 0x4800001C;
			*(u32 *)0x8137DC70 = 0x38000001;
			*(u32 *)0x8137D9AC = 0x38000001;
			*(u32 *)0x8137FD2C = 0x38600001;
			*(u32 *)0x8137E4E4 = 0x60000000;

			*(u32 *)0x8152DB84 = 0x60000000;
			*(u32 *)0x81531030 = 0x4E800020;
			*(u32 *)0x8152E06C = 0x38000001;
			*(u32 *)0x813B69A8 = 0x48000028;
		break;

		case 486: // KOR 4.2
			*(u32 *)0x8137CF7C = 0x4800001C;
			*(u32 *)0x8137CF5C = 0x38000001;
			*(u32 *)0x8137CC98 = 0x38000001;
			*(u32 *)0x8137F018 = 0x38600001;
			*(u32 *)0x8137D7D0 = 0x60000000;

			*(u32 *)0x81506840 = 0x60000000;
			*(u32 *)0x81509CEC = 0x4E800020;
			*(u32 *)0x81506D28 = 0x38000001;
			*(u32 *)0x813B5C68 = 0x48000028;
		break;
		
		default:
		break;
	}
}


s32 identify(u64 titleid, u16 *bootapp, u16 *version)
{
	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);
	u8 *tmdBuffer = NULL;
	u32 tmdSize;
	u8 *tikBuffer = NULL;
	u32 tikSize;
	u8 *certBuffer = NULL;
	u32 certSize;
	
	int ret;

	//printf("Reading TMD...");
	//fflush(stdout);

	ISFS_Initialize();
	
	sprintf(filepath, "/title/%08x/%08x/content/title.tmd", TITLE_UPPER(titleid), TITLE_LOWER(titleid));
	ret = read_file_from_nand(filepath, &tmdBuffer, &tmdSize);
	if (ret < 0)
	{
		//printf("Reading TMD failed\n");
		return ret;
	}
	//printf("done\n");

	u16 bootindex;
	bootindex = ((tmd *)SIGNATURE_PAYLOAD((signed_blob *)tmdBuffer))->boot_index;
	tmd_content *p_cr;
	p_cr = TMD_CONTENTS(((tmd *)SIGNATURE_PAYLOAD((signed_blob *)tmdBuffer)));
	*bootapp = p_cr[bootindex].cid;

	*version = *(u16 *)(&tmdBuffer[0x1dc]);

	//printf("Reading ticket...");
	//fflush(stdout);

	sprintf(filepath, "/ticket/%08x/%08x.tik", TITLE_UPPER(titleid), TITLE_LOWER(titleid));
	ret = read_file_from_nand(filepath, &tikBuffer, &tikSize);
	if (ret < 0)
	{
		//printf("Reading ticket failed\n");
		free(tmdBuffer);
		return ret;
	}
	//printf("done\n");

	//printf("Reading certs...");
	//fflush(stdout);

	sprintf(filepath, "/sys/cert.sys");
	ret = read_file_from_nand(filepath, &certBuffer, &certSize);
	if (ret < 0)
	{
		//printf("Reading certs failed\n");
		free(tmdBuffer);
		free(tikBuffer);
		return ret;
	}
	//printf("done\n");
	
	//printf("ES_Identify...");
	//fflush(stdout);

	ret = ES_Identify((signed_blob*)certBuffer, certSize, (signed_blob*)tmdBuffer, tmdSize, (signed_blob*)tikBuffer, tikSize, NULL);
	if (ret < 0)
	{
		switch(ret)
		{
			case ES_EINVAL:
				//printf("Error! ES_Identify (ret = %d;) Data invalid!\n", ret);
				break;
			case ES_EALIGN:
				//printf("Error! ES_Identify (ret = %d;) Data not aligned!\n", ret);
				break;
			case ES_ENOTINIT:
				//printf("Error! ES_Identify (ret = %d;) ES not initialized!\n", ret);
				break;
			case ES_ENOMEM:
				//printf("Error! ES_Identify (ret = %d;) No memory!\n", ret);
				break;
			default:
				//printf("Error! ES_Identify (ret = %d)\n", ret);
				break;
		}
		free(tmdBuffer);
		free(tikBuffer);
		free(certBuffer);
		return ret;
	}
	//printf("done\n");
	
	free(tmdBuffer);
	free(tikBuffer);
	free(certBuffer);
	return 0;
}

s32 ISFS_Load_dol(u64 titleid, u16 index)
{
	last_index = -1;
	u32 i;
	s32 ret;
	s32 Fd = 0;

	dolheader *dolfile = (dolheader*)allocate_memory(sizeof(dolheader));
	if (dolfile == NULL)
	{
		print_status("Out of memory");
		return -1;
	}

	char filepath[ISFS_MAXPATH] ATTRIBUTE_ALIGN(0x20);
	
	if (preloaderselect == 1)
	{
		// Preloader moves the system menu .dol here:
		sprintf(filepath, "/title/%08x/%08x/content/1%07x.app", TITLE_UPPER(titleid), TITLE_LOWER(titleid), index);
		Fd = ISFS_Open(filepath, ISFS_OPEN_READ);
	}
	if (preloaderselect == 0 || Fd < 0)
	{
		sprintf(filepath, "/title/%08x/%08x/content/%08x.app", TITLE_UPPER(titleid), TITLE_LOWER(titleid), index);

		Fd = ISFS_Open(filepath, ISFS_OPEN_READ);
		if (Fd < 0)
		{
			print_status("ISFS_Open %d", Fd);
			free(dolfile);
			return Fd;
		}
	}

	ret = ISFS_Read(Fd, (void *)dolfile, sizeof(dolheader));
	if (ret < 0)
	{
		print_status("ISFS_Read %d", ret);
		free(dolfile);
		return ret;
	}
	
	memset((void *)dolfile->bss_start, 0, dolfile->bss_size);
	
	for (i = 0; i < 7; i++)
	{
		if(dolfile->data_start[i] < sizeof(dolheader))
			continue;
		
		ret = ISFS_Seek(Fd, dolfile->text_pos[i], 0);
		if (ret < 0)
		{
			print_status("ISFS_Seek %d", ret);
			free(dolfile);
			return ret;
		}
		ret = ISFS_Read(Fd, (void *)dolfile->text_start[i], dolfile->text_size[i]);
		if (ret < 0)
		{
			print_status("ISFS_Read %d", ret);
			free(dolfile);
			return ret;
		}
		DCFlushRange((void *)dolfile->text_start[i], dolfile->text_size[i]);
		ICInvalidateRange((void *)dolfile->text_start[i], dolfile->text_size[i]);

		last_index++;	
		Address_Array[last_index] = (void *)dolfile->text_start[i];
		Section_Size_Array[last_index] = dolfile->text_size[i];
	}

	for (i = 0; i < 11; i++)
	{
		if(dolfile->data_start[i] < sizeof(dolheader))
			continue;
				
		ret = ISFS_Seek(Fd, dolfile->data_pos[i], 0);
		if (ret < 0)
		{
			print_status("ISFS_Seek %d", ret);
			free(dolfile);
			return ret;
		}
		ret = ISFS_Read(Fd, (void *)dolfile->data_start[i], dolfile->data_size[i]);
		if (ret < 0)
		{
			print_status("ISFS_Read %d", ret);
			free(dolfile);
			return ret;
		}
		DCFlushRange((void *)dolfile->data_start[i], dolfile->data_size[i]);
		ICInvalidateRange((void *)dolfile->data_start[i], dolfile->data_size[i]);

		last_index++;	
		Address_Array[last_index] = (void *)dolfile->data_start[i];
		Section_Size_Array[last_index] = dolfile->data_size[i];
	}
	
	ISFS_Close(Fd);
	free(dolfile);
	
	return 0;
}

extern void *codelist;

void boot_menu()
{
	s32 ret;
	int i;
	u16 bootapp;
	u16 version;
	
	ocarinarebooter();

	ISFS_Initialize();

	ret = identify(0x0000000100000002ULL, &bootapp, &version);
	if (ret < 0)
	{
		print_status("Identify failed %d", ret);
		return;
	}

	ret = ES_SetUID(0x0000000100000002ULL);
	if (ret < 0)
	{
		print_status("ES_SetUID failed %d", ret);
		return;
	}
	
	ret = ISFS_Load_dol(0x0000000100000002ULL, bootapp);
	if (ret < 0)
	{
		return;
	}

	ISFS_Deinitialize();
	
	if (wiihookselect != 0)
	{
		// Load Sysmenu patch handler
		memset((void*)0x81700000,0,sysmenu_size);
		memcpy((void*)0x81700000,sysmenu,sysmenu_size);
		memcpy((void*)0x81700096, &codelist, 2);
		memcpy((void*)0x8170009A, ((u8*) &codelist) + 2, 2);
		switch(wiihookselect)
		{
			case 0x01:
				memcpy((void*)0x81700224,viwiihooks,12);
				memcpy((void*)0x81700220,viwiihooks+3,4);
				break;
			case 0x02:
				memcpy((void*)0x81700224,kpadhooks,12);
				memcpy((void*)0x81700220,kpadhooks+3,4);
				break;
			case 0x03:
				memcpy((void*)0x81700224,joypadhooks,12);
				memcpy((void*)0x81700220,joypadhooks+3,4);
				break;
			case 0x04:
				memcpy((void*)0x81700224,gxdrawhooks,12);
				memcpy((void*)0x81700220,gxdrawhooks+3,4);
				break;
			case 0x05:
				memcpy((void*)0x81700224,gxflushhooks,12);
				memcpy((void*)0x81700220,gxflushhooks+3,4);
				break;
			case 0x06:
				memcpy((void*)0x81700224,ossleepthreadhooks,12);
				memcpy((void*)0x81700220,ossleepthreadhooks+3,4);
				break;
			case 0x07:
				memcpy((void*)0x81700224,axnextframehooks,12);
				memcpy((void*)0x81700220,axnextframehooks+3,4);
				break;
			case 0x08:
				//if (customhooksize == 16)
				//{
				//	memcpy((void*)0x81700224,customhook,12);
				//	memcpy((void*)0x81700220,customhook+3,4);
				//}
				break;
		}
		DCFlushRange((void*)0x81700000,sysmenu_size);
		ICInvalidateRange((void*)0x81700000,sysmenu_size);		
	}

	if (skipupdateselect)
	{
		skipdiscupdates(version);
	}

	if (bendiosreloadselect)
	{
		bendiosreload(version);
	}

	if (regionfreeselect)
	{
		region_free(version);
	}

	for (i = 0; i <= last_index; i++)
	{
		// Multidol hook
		if (wiihookselect != 0x00)
		{
			patchmenu(Address_Array[i], Section_Size_Array[i], 6);
		}
		
		if (recoveryselect)
		{
			patchmenu(Address_Array[i], Section_Size_Array[i], 0);
		}
		// Health check
		if (buttonskipselect)
		{
			patchmenu(Address_Array[i], Section_Size_Array[i], 3);
		}
		// No copy bit remove
		if (nocopyselect)
		{
			patchmenu(Address_Array[i], Section_Size_Array[i], 4);
		}		
		
		// viwiihook
		if (wiihookselect != 0)
		{
			patchmenu(Address_Array[i], Section_Size_Array[i], 1);
		}
		
		// move dvd channel
		//patchmenu((void*)0x8132ff80, 0x380000, 5);

		DCFlushRange(Address_Array[i], Section_Size_Array[i]);
		ICInvalidateRange(Address_Array[i], Section_Size_Array[i]);
	}
	//memset((void*)0x800027E8,0,4); 


	//DCFlushRange((void*)0x80000000, 0x3f00);

	shutdown_all();
	if (wiihookselect == 0)
	{
		__asm__(
				"bl DCDisable\n"
				"bl ICDisable\n"
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, 0x0000\n"
				"ori %r4,%r4,0x3400\n"
				"mtsrr0 %r4\n"
				"rfi\n"
				);
	}
	else
	{
		__asm__(
				"lis %r3, returnpointmenu@h\n"
				"ori %r3, %r3, returnpointmenu@l\n"
				"mtlr %r3\n"
				"lis %r3, 0x8000\n"
				"ori %r3, %r3, 0x18A8\n"
				"nop\n"									// We Dare checks if it find this code in memory. If this nop wasn't here, the game wouldn't work, and yes also if the rebooter is not used
				"mtctr %r3\n"
				"bctr\n"
				"returnpointmenu:\n"
				"bl DCDisable\n"
				"bl ICDisable\n"
				"li %r3, 0\n"
				"mtsrr1 %r3\n"
				"lis %r4, 0x0000\n"
				"ori %r4,%r4,0x3400\n"
				"mtsrr0 %r4\n"
				"rfi\n"
				);
	}
}
