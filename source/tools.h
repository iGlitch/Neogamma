/*
 *  Copyright (C) 2008 Nuke (wiinuke@gmail.com)
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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#define TITLE_UPPER(x)		((u32)((x) >> 32))
#define TITLE_LOWER(x)		((u32)(x))

bool Power_Flag;
bool Reset_Flag;
	
// Function prototypes
void init_video();
s32 read_file_from_nand(char *filepath, u8 **buffer, u32 *filesize);
s32 write_file_to_nand(char *filepath, u8 *buffer, u32 filesize);
void show_banner();
void wait(u32 s);
void print_status(const char *Format, ...);
void *allocate_memory(u32 size);
void waitforbuttonpress(u32 *out, u32 *outGC, u32 *outCover);
void Verify_Flags();
void shutdown_all();
u32 be32(const u8 *data);
bool use_wii_Ocarina_engine();
u8 find_cIOS_with_base(u8 requested_ios_base);

typedef struct _iosinfo_t {
	u32 magicword; //0x1ee7c105
	u32 magicversion; // 1
	u32 version; // Example: 5
	u32 baseios; // Example: 56
	char name[0x10]; // Example: d2x
	char versionstring[0x10]; // Example: beta2
} __attribute__((packed)) iosinfo_t;

#endif // __TOOLS_H__

