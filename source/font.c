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

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <string.h>
#include <ogc/color.h>
#include "font.h"
#include "arial16.h"
#include <stdarg.h>



u16 back_framewidth = 640;

typedef struct
{
  unsigned short font_type, first_char, last_char, subst_char, ascent_units, descent_units, widest_char_width,
                 leading_space, cell_width, cell_height;
  u32 texture_size;
  unsigned short texture_format, texture_columns, texture_rows, texture_width, texture_height, offset_charwidth;
  u32 offset_tile, size_tile;
} FONT_HEADER;

int font_offset[256], font_size[256], fheight;
static u8 fontWork[ 0x20000 ] __attribute__((aligned(32)));
static u8 fontFont[ 0x40000 ] __attribute__((aligned(32)));
extern void __SYS_ReadROM(void *buf,u32 len,u32 offset);
extern u32 *xfb;


static void __yay0_decode(u8 *s, u8 *d)
{
	int i, j, k, p, q, cnt;

	i = *(u32 *)(s + 4);	  
	j = *(u32 *)(s + 8);	 
	k = *(u32 *)(s + 12);	 

	q = 0;					
	cnt = 0;				
	p = 16;					

	u32 r22 = 0, r5;
	
	do
	{
		if(cnt == 0)
		{
			r22 = *(u32 *)(s + p);
			p += 4;
			cnt = 32; 
		}

		if(r22 & 0x80000000)
		{
			*(u8 *)(d + q) = *(u8 *)(s + k);
			k++, q++;
		}

		else
		{

			int r26 = *(unsigned short *)(s + j);
			j += 2;

			int r25 = q - (r26 & 0xfff);
			int r30 = r26 >> 12;
			if(r30 == 0)
			{
				r5 = *(u8 *)(s + k);
				k++;
				r30 = r5 + 18;
			}
			else r30 += 2;
			u8 *pt = ((u8*)d) + r25;
			int i;
			for(i=0; i<r30; i++)
			{
				*(u8 *)(d + q) = *(u8 *)(pt - 1);
				q++, pt++;
			}
		}
		r22 <<= 1;
		cnt--;

	} while(q < i);
}

void untile(u8 *dst, u8 *src, int xres, int yres)
{
	int x, y;
	int t=0;
	for (y = 0; y < yres; y += 8)
		for (x = 0; x < xres; x += 8)
		{
			t = !t;
			int iy, ix;
			for (iy = 0; iy < 8; ++iy, src+=2)
			{
				u8 *d = dst + (y + iy) * xres + x;
				for (ix = 0; ix < 2; ++ix)
				{
					int v = src[ix];
					*d++ = ((v>>6)&3);
					*d++ = ((v>>4)&3);
					*d++ = ((v>>2)&3);
					*d++ = ((v)&3);
				}
			}
		}
}



void init_font(void)
{
        int i;

      //  __SYS_ReadROM((unsigned char *)&fontFont,0x3000,0x1FCF00);
        memcpy((unsigned char *)&fontFont, Arial16,Arial16_size);
		__yay0_decode((unsigned char *)&fontFont, (unsigned char *)&fontWork);
        FONT_HEADER *fnt;

        fnt = ( FONT_HEADER * )&fontWork;

        untile((unsigned char*)&fontFont, (unsigned char*)&fontWork[fnt->offset_tile], fnt->texture_width, fnt->texture_height);

        for (i=0; i<256; ++i)
        {
                int c = i;

                if ((c < fnt->first_char) || (c > fnt->last_char)) c = fnt->subst_char;
                else c -= fnt->first_char;

                font_size[i] = ((unsigned char*)fnt)[fnt->offset_charwidth + c];

                int r = c / fnt->texture_columns;
                c %= fnt->texture_columns;
                font_offset[i] = (r * fnt->cell_height) * fnt->texture_width + (c * fnt->cell_width);
        }
        
        fheight = fnt->cell_height;
}

#define TRANSPARENCY (COLOR_BLACK)

unsigned int blit_lookup[4] = {COLOR_BLACK, 0x6d896d77, 0xb584b57b, COLOR_WHITE};
unsigned int blit_lookup2[4] = {COLOR_BLACK, 0x6d896d77, 0xb584b57b, COLOR_MEDGRAY};
unsigned int blit_lookup_inv[4] = {COLOR_WHITE, 0xb584b57b, 0x6d896d77, 0x258e2573};


static void __blitchar(int x, int y, u8 c, u32 *lookup)
{
	u8 *fnt = ((u8*)fontFont) + font_offset[c];
	int ay, ax;
	u32 llookup;

	for (ay=0; ay<fheight; ++ay)
	{
		int h = (ay + y) * 320;

		for (ax=0; ax<font_size[c]; ax++)
		{
			int v0 = fnt[ax];
			int p = h + (( ax + x ) >> 1);
			u32 o = xfb[p];

			llookup = lookup[v0];

			if ((v0== 0) && (llookup == TRANSPARENCY))
				llookup = o;

			if ((ax+x) & 1)
			{
				o &= ~0x00FFFFFF;
				o |= llookup & 0x00FFFFFF;
			}
			else
			{
				o &= ~0xFF000000;
				o |= llookup & 0xFF000000;
			}

			xfb[p] = o;
		}
		
		fnt += 512;
	}
}

void write_font(int x, int y, bool white, const char *Format, ...)
{
	char Buffer[256];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);

	int i = 0;
	while (i<strlen(Buffer) && (x+font_size[(u8)Buffer[i]] < back_framewidth))
	{
		if (white)
		{
			__blitchar(x, y,Buffer[i], blit_lookup);
		} else
		{
			__blitchar(x, y,Buffer[i], blit_lookup2);
		}
		x += font_size[(u8)Buffer[i]];
		i++;
	}
}

void write_font_max_length(int x, int y, const char *string, int maxlen)
{
   int end = back_framewidth;
   if (x + maxlen < back_framewidth)
   {
		end = x + maxlen;
   } else
   {
		end = back_framewidth;
   }
   
   while (*string && (x+font_size[(u8)*string] < end))
   {
         __blitchar(x, y, *string, blit_lookup);
         x += font_size[(u8)*string];
         string++;
   }
}

void write_font_centered(int minx, int maxx, int y, const char *Format, ...)
{
	char Buffer[256];
	va_list args;

	va_start(args, Format);
	vsprintf(Buffer, Format, args);

	va_end(args);

	const char *string = (char *)Buffer;

	int x;
	const char *ostring;
	ostring = string;
	int length = 0;
	while (*string)
	{
		length+=font_size[(u8)*string];
		string++;
	}
	string = ostring;
	if (minx+length > maxx)
	{
		x = minx;
	} else
	{
		x = minx + (maxx-minx-length) /2;
	}
	if (maxx > back_framewidth)
	{
		maxx = back_framewidth;
	}
   
	while (*string && (x+font_size[(u8)*string] < maxx))
	{
		__blitchar(x, y, *string, blit_lookup);
		x += font_size[(u8)*string];
		string++;
	}
}

