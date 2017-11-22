/*******************************************************************************
 * Toy.c
 *
 * Copyright (c) 2009 WiiPower
 *
 * Distributed under the terms of the GNU General Public License (v3)
 * See http://www.gnu.org/licenses/gpl-3.0.txt for more info.
 *
 * Description:
 * -----------
 *
 ******************************************************************************/


#include <ogcsys.h>
#include <stdlib.h>
#include <dirent.h>


#include "Toy.h"
#include "Toy_video.h"
#include "loadapp.h"


void* 				Address_Array[64];			//TODO: variable size
int					Section_Size_Array[64];		//TODO: variable size
int 				last_index;
int 				maindol_len;
GXRModeObj* 		videomodes[50];
unsigned int		videomodecount;
int 				contentcounter;
char 				Disc_ID[8];
unsigned int		toy_Video_Mode;
void*				apploader_header_address;
bool				loadapploaderfromdisc;



void toy_reset()
{
	videomodecount = 0;
	last_index = -1;
	maindol_len = 0;
	contentcounter = 0;
}

void toy_register_apploader_file_loading(void *Address, u32 Size)
{
	// Heavily relies on toy_reset being executed
	
	last_index = last_index + 1;
	Address_Array[last_index] = Address;
	Section_Size_Array[last_index] = Size;

	// find the addresses of the video mode objects
	u8 *Addr				= (u8 *)Address;
	while (Size >= sizeof(GXRModeObj))
	{
		if (videomode_region((GXRModeObj*)Addr) != -1)
		{
			videomodes[videomodecount] = (GXRModeObj*)Addr;
			videomodecount++;
			Addr += sizeof(GXRModeObj);
			Size -= sizeof(GXRModeObj);			
		} else
		{
			Addr += 4;
			Size -= 4;
		}
	}
}

GXRModeObj* vmode = NULL;

void set_rmode(GXRModeObj* rmode)
{
	vmode = rmode;
}

void toy_patch_video_modes()
{
	if (vmode == NULL)
		return;
		
	// Note that the video mode patches for progressive video modes with video mode wii or disc will fail
	// Select the video mode to use directly in this case
	
	// Patch video modes from the main.dol
	bool patch;
	if (videopatchmode > 0)
	{
		unsigned int i;
		for (i=0;i < videomodecount;i++)
		{
			patch = false;
			if (videopatchmode == 3)
			{
				patch = true;
			} else
			{
				if (videomode_interlaced(videomodes[i]) == videomode_interlaced(vmode))
				{
					if (videopatchmode == 2)
					{
						patch = true;
					} else
					{
						if (videomode_480(videomodes[i]) == videomode_480(vmode))
						{
							patch = true;
						}
					}				
				}
			}
			if (patch == true)
			{
				patch_videomode(videomodes[i], vmode);
				DCFlushRange(&videomodes[i], sizeof(GXRModeObj));
				ICInvalidateRange(&videomodes[i], sizeof(GXRModeObj));
			}
		}
	}		
}
