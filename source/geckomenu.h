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

#ifndef __GECKOMENU_H__
#define __GECKOMENU_H__


#define ROOTMENU 0
#define ABOUTMENU 1
#define OPTIONSMENU 2
#define GENERALMENU 3
#define WIIMENU 4
#define GCMENU 5
#define OCARINAMENU 6
#define REBOOTMENU 7

#define root_itemcount 8
#define about_itemcount 5

#define options_itemcount 7
#define general_itemcount 6
#define wii_itemcount 10
#define gc_itemcount 9

#ifdef DONTTELL
#define ocarina_itemcount 10
#else
#define ocarina_itemcount 9
#endif

#define rebooter_itemcount 9

u32 currentmenu;	// 0 ROOT
s32 rootmenu_item;
u32 menufreeze;

void clearmenu();

u32 doprogress(u32 progstate, u32 noelements);
void drawmenu(u32 menuid);
void drawselected(u32 menuidpos);
void processwpad();
void clearscreen(u32 *framebuffer, u16 xscreen, u16 yscreen, u16 width, u16 height, u32 color);
void drawicon(u32 *framebuffer, u16 xscreen, u16 yscreen, u16 width, u16 height, u32 gicon);
u32 CvtRGB (u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2);


#endif // __GECKOLOAD_H__
