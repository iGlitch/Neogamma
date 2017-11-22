/*-------------------------------------------------------------

usbstorage_starlet.c -- USB mass storage support, inside starlet
Copyright (C) 2009 Kwiirk

If this driver is linked before libogc, this will replace the original 
usbstorage driver by svpe from libogc
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

/* IOCTL commands */
#define UMS_BASE			(('U'<<24)|('M'<<16)|('S'<<8))
#define USB_IOCTL_UMS_INIT	        (UMS_BASE+0x1)
#define USB_IOCTL_UMS_GET_CAPACITY      (UMS_BASE+0x2)
#define USB_IOCTL_UMS_READ_SECTORS      (UMS_BASE+0x3)
#define USB_IOCTL_UMS_WRITE_SECTORS	(UMS_BASE+0x4)
#define USB_IOCTL_UMS_READ_STRESS	(UMS_BASE+0x5)
#define USB_IOCTL_UMS_SET_VERBOSE	(UMS_BASE+0x6)

#define UMS_HEAPSIZE			0x8000

/* Variables */
static char fs[] ATTRIBUTE_ALIGN(32) = "/dev/usb2"; // Name of the module since cIOS rev18
static char fs2[] ATTRIBUTE_ALIGN(32) = "/dev/usb/ehc"; // Name of the module before cIOS rev18
static char fs3[] ATTRIBUTE_ALIGN(32) = "/dev/usb123"; // Name of the module in Hermes cIOS
 
static s32 hid = -1, fd = -1;
static u32 sector_size;
static u32 nb_sectors;

#define USB_MEM2_SIZE           0x1000		// Needs to be a multiple of 4096

void *usb_buf2 = NULL;
extern void* SYS_AllocArena2MemLo(u32 size,u32 align);


inline s32 __USBStorage_isMEM2Buffer(const void *buffer)
{
	u32 high_addr = ((u32)buffer) >> 24;

	return (high_addr == 0x90) || (high_addr == 0xD0);
}


s32 USBStorage_GetCapacity()
{
	if (fd > 0) {
		s32 ret;

		ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_GET_CAPACITY, ":i", &sector_size);

/*		if (ret && _sector_size)
			*_sector_size = sector_size;
*/
		return ret;
	}

	return IPC_ENOENT;
}

s32 USBStorage_Init(u32 *out_nb_sectors, u32 *out_sector_size)
{
	/* Already open */
	if (fd > 0)
		return 0;

	/* Create heap */
	if (hid < 0) {
		hid = iosCreateHeap(UMS_HEAPSIZE);
		if (hid < 0)
			return IPC_ENOMEM; 
	}

	/* Open USB device */
	fd = IOS_Open(fs, 0);
	if (fd < 0)
	{
		fd = IOS_Open(fs2, 0);
		if (fd < 0)
		{
			fd = IOS_Open(fs3, 0);
		}
	}

	if (fd < 0)
		return fd;

	/* Initialize USB storage */
	IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_INIT, ":");

	/* Get device capacity */
	sector_size = 0;
	nb_sectors = USBStorage_GetCapacity();

	if (nb_sectors < 16)	// For some reason ret is 1 most of the time with a cIOS with base IOS58
	{
		goto err;
	}
	
	*out_nb_sectors = nb_sectors;
		
	if (sector_size == 0)
	{
		goto err;
	}
	
	*out_sector_size = sector_size;	

	return 0;

err:
	/* Close USB device */
	if (fd > 0) {
		IOS_Close(fd);
		fd = -1;
	}

	return -1;
}

void USBStorage_Deinit(void)
{
	/* Close USB device */
	if (fd > 0) {
		IOS_Close(fd);
		fd = -1;
	}
}




// Function copied from Configurable USB Loader
// http://code.google.com/p/cfg-loader/source/browse/trunk/Source/source/usbstorage.c

s32 USBStorage_ReadSectors(u32 sector, u32 numSectors, void *buffer)
{
	u32 size;
	s32 ret = -1;

	/* Device not opened */
	if (fd < 0)
		return fd;

	/* check align and MEM1 buffer */
	if (((u32)buffer & 0x1F) || (!__USBStorage_isMEM2Buffer(buffer)))
	{
		if (usb_buf2 == NULL)
		{
			//usb_buf2 = iosAlloc(hid, USB_MEM2_SIZE);
			usb_buf2 = SYS_AllocArena2MemLo(USB_MEM2_SIZE, 32);

			if (usb_buf2 == NULL)
			{
				return IPC_ENOMEM;  // = -22
			}
		}
		
		int cnt;
		int max_sec = USB_MEM2_SIZE / sector_size;

		while (numSectors)
		{
			if (numSectors > max_sec)
			{
				cnt = max_sec;
			} else
			{
				cnt = numSectors;
			}
			size = cnt * sector_size;
			
			// This happens on some usb storage with a cIOS with base IOS58, and freezes the Wii
			if (sector >= nb_sectors)
			{
				return -1;
			}
			
			ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, cnt, usb_buf2, size);

			if (ret < 0)
				return ret;
			memcpy(buffer, usb_buf2, size);
			numSectors -= cnt;
			sector += cnt;
			buffer += size;
		}
	} else
	{
		size = sector_size * numSectors;
		/* Read data */
		ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_READ_SECTORS, "ii:d", sector, numSectors, buffer, size);
	}

	return ret;
}

/*
s32 USBStorage_WriteSectors(u32 sector, u32 numSectors, const void *buffer)
{
	void *buf = (void *)buffer;
	u32   len = (sector_size * numSectors);

	s32 ret;

	// Device not opened
	if (fd < 0)
		return fd;

	// MEM1 buffer
	if (!__USBStorage_isMEM2Buffer(buffer)) {
		// Allocate memory 
		buf = iosAlloc(hid, len);
		if (!buf)
			return IPC_ENOMEM;

		// Copy data
		memcpy(buf, buffer, len);
	}

	// Write data
	ret = IOS_IoctlvFormat(hid, fd, USB_IOCTL_UMS_WRITE_SECTORS, "ii:d", sector, numSectors, buf, len);

	// Free memory
	if (buf != buffer)
		iosFree(hid, buf);

	return ret;
}
*/
