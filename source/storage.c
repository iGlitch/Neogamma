#include <gccore.h>
#include <fat.h>
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>

#include "config.h"
#include "easywbfs.h"
#include "storage.h"
#include "font.h"
#include "apploader.h"
#include "tools.h"
#include "sdhc.h"
#include "libcios.h"
#include "dvd_broadway.h"


/*
storageselect
0	sd using cIOS
1	usb using cIOS
2	sd using IOS36
3	usb using IOS36
4	sd using IOS61
5	usb using IOS61
*/

extern DISC_INTERFACE __io_usbstorage;

DISC_INTERFACE storage;

void print_status(const char *Format, ...);

bool sd_access()
{
	return (storageselect == 0 || storageselect == 2 || storageselect == 4);
}

bool ios_change()
{
	return (storageselect > 1);
}

static bool storage_used = false;

void storage_shutdown()
{
	if (storage_used)
	{
		fatUnmount("fat");

		//Never shutdown usb storage, this breaks usb loading and doesn't work at all anyways
		if (sd_access())
		{
			storage.shutdown();
		}
	}
	storage_used = false;
}


s32 storage_init()
{
	int ret;
	
	if (sd_access())
	{
		storagename[0] = 'S';
		storagename[1] = 'D';
		storagename[2] = 0;
		storagename[3] = 0;
		storage = __io_wiisd;
	} else
	{
		storagename[0] = 'U';
		storagename[1] = 'S';
		storagename[2] = 'B';
		storagename[3] = 0;
		storage = __io_usbstorage;
	}

	ret = storage.startup();
	if (ret < 0) 
	{
		//printf("%s Error\n", storagename);
		print_status("%s Error", storagename);
		return ret;
	}
	ret = fatMountSimple("fat", &storage);

	if (ret < 0) 
	{
		storage_shutdown();
		//printf("FAT Error\n");
		print_status("FAT Error");
		return ret;
	}
	
	storage_used = true;

	return 0;
}

s32 resume_disc_loading()
{
	if (Sneek_Mode == true)
	{
		return 1;
	}
	
	int ret;
	if (ios_change())
	{
		storage_shutdown();
		Load_IOS(CIOS_VERSION);

		if (drive_state > 0)
		{
			ret = init_drive();
			if (ret < 0) return ret;
		}
		
		if (drive_state > 1)
		{
			ret = open_partition();
			if (ret < 0) return ret;
		}
	}
	
	if (drive_state > 0 && wbfsdevice == 2 && sd_access() && !ios_change())
	{
		storage_shutdown();
		SDHC_Init();
	}

	return 1;
}


s32 prepare_storage_access()
{
	if (Sneek_Mode == true)
	{
		return 1;
	}
	
	int ret;
	if (drive_state > 0 && wbfsdevice == 2 && (sd_access() || ios_change()))
	{
		SDHC_Close();
	}
	if (ios_change())
	{
		storage_shutdown();	
		if (drive_state > 0)
		{
			bwDVD_LowClose();
		}
		u32 ios;
		if (storageselect == 2 || storageselect == 3)
		{
			ios = 36;
		} else
		{
			ios = 61;
		}
		Load_IOS(ios);
	}
	ret = storage_init();
	if (ret < 0)
	{
		resume_disc_loading();
	}
	return ret;
}

