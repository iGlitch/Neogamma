#include <stdio.h>
#include <stdio.h>
#include <fat.h>
#include <malloc.h>
#include <sdcard/wiisd_io.h>
#include <dirent.h>
#include <string.h>

#include "config.h"
#include "easywbfs.h"
#include "tools.h"

#define MAGICWORD "NEOGAMMACONFIG03"

void Set_Config_to_Defaults()
{
	CIOS_VERSION = 249;

	// Ocarina/Wiird
	wiihookselect = 0;
	wiiocarinaselect = 0;
	wiidebuggerselect = 0;

	gchookselect = 0;
	gcocarinaselect = 0;
	gcdebuggerselect = 0;

	// Wii
	langselect = 0;
	videoselect = 1;
	videopatchselect = 0;
	viselect = 0;
	countrystringselect = 0;
	alternativedolselect = 0;
	patchselect = 1;

	// General
	storageselect = 0;

	// Gamecube
	gamecubemodeselect = 1;
	gc_videoselect = 1;
	gc_videopatchselect = 0;
	
	// Rebooter
	recoveryselect = 0;
	regionfreeselect = 0;
	nocopyselect = 0;
	buttonskipselect = 0;
	skipupdateselect = 1;
	bendiosreloadselect = 0;
	preloaderselect = 1;
	
	// Not saved options
	configselect = 0; // Don't store config by default
	wbfsdevice = 0;
	gameselected = 0;
	
	gcreloaderselect = 1;

	gchighplugin = 0;	// Not saved option!
	audiostatusrequestfixselect = 0;	// Not saved option!
	patchedMIOSselect = 0;	// Not saved option!
	
	gc2ndhookselect = 8;
	blockiosreloadselect = 1;
	showrebooterselect = 0;
	wii2ndhookselect = 8;
	usecorrectiosoption = 2;
	unusedoption7 = 0;
}

bool MakeDir(const char *Path)
{
	// Open Target Folder
	DIR* dir = opendir(Path);

	// Already Exists?
	if (dir == NULL)
	{
		// Create
		mode_t Mode = 0777;
		mkdir(Path, Mode);

		// Re-Verify
		closedir(dir);
		dir = opendir(Path);
		if (dir == NULL) return false;
	}

	// Success
	closedir(dir);
	return true;
}

void Save_Config()
{
	if (configselect == 0)
	{
		return;
	}
	
	int ret;
	u32 filesize = 48;
	u8 *filebuff = (u8*) allocate_memory (filesize);
	
	if (filebuff == NULL)
	{
		return;
	}	

	sprintf((char *)filebuff, MAGICWORD);
	
	filebuff[16] = CIOS_VERSION;

	// Ocarina/Wiird
	filebuff[17] = wiihookselect;
	filebuff[18] = wiiocarinaselect;
	filebuff[19] = wiidebuggerselect;

	filebuff[20] = gchookselect;
	filebuff[21] = gcocarinaselect;
	filebuff[22] = gcdebuggerselect;

	// Wii
	filebuff[23] = langselect;
	filebuff[24] = videoselect;
	filebuff[25] = videopatchselect;
	filebuff[26] = viselect;
	filebuff[27] = countrystringselect;
	filebuff[28] = alternativedolselect;
	filebuff[29] = patchselect;

	// General
	filebuff[30] = storageselect;

	// Gamecube
	filebuff[31] = gamecubemodeselect;
	filebuff[32] = gc_videoselect;
	filebuff[33] = gc_videopatchselect;
	
	// Rebooter
	filebuff[34] = recoveryselect;
	filebuff[35] = regionfreeselect;
	filebuff[36] = nocopyselect;
	filebuff[37] = buttonskipselect;
	filebuff[38] = skipupdateselect;
	filebuff[39] = bendiosreloadselect;
	filebuff[40] = preloaderselect;
	
	filebuff[41] = gcreloaderselect;
	filebuff[42] = gc2ndhookselect;
	filebuff[43] = blockiosreloadselect;
	filebuff[44] = showrebooterselect;
	filebuff[45] = wii2ndhookselect;
	filebuff[46] = usecorrectiosoption;
	filebuff[47] = unusedoption7;

	if (Sneek_Mode == false)
	{	
		FILE *fp;
		char buf[128];

		ret = __io_wiisd.startup();
		if (!ret) 
		{
			return;
		}

		ret = fatMountSimple("sd", &__io_wiisd);
		if (!ret) 
		{
			__io_wiisd.shutdown();
			return;
		}
		
		if (!MakeDir("sd:/NeoGamma"))
		{
			fatUnmount("sd");
			__io_wiisd.shutdown();
			return;
		}
		
		sprintf(buf, "sd:/NeoGamma/NeoGamma.cfg");
		fp = fopen(buf, "wb");
		if (!fp)
		{
			fatUnmount("sd");
			__io_wiisd.shutdown();
			return;
		}

		fwrite(filebuff, 1, filesize, fp);

		fclose(fp);
		fatUnmount("sd");
		__io_wiisd.shutdown();
	} else
	{
		ISFS_Initialize();

		ISFS_CreateDir("/NeoGamma", 0, 3, 3, 3);

		write_file_to_nand("/NeoGamma/NeoGamma.cfg", filebuff, filesize);
	}

	free(filebuff);
}

void Load_Config()
{
	u8 *filebuff = NULL;
	u32 filesize = 0;
	int ret;

	if (Sneek_Mode == false)
	{
		FILE *fp;
		char buf[128];

		ret = __io_wiisd.startup();
		if (!ret) 
		{
			Set_Config_to_Defaults();
			return;
		}

		ret = fatMountSimple("sd", &__io_wiisd);
		if (!ret) 
		{
			__io_wiisd.shutdown();
			Set_Config_to_Defaults();
			return;
		}

		sprintf(buf, "sd:/NeoGamma/NeoGamma.cfg");
		fp = fopen(buf, "rb");
		if (!fp)
		{
			fatUnmount("sd");
			__io_wiisd.shutdown();
			Set_Config_to_Defaults();
			return;
		}

		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if (filesize != 48)
		{
			fclose(fp);
			fatUnmount("sd");
			__io_wiisd.shutdown();
			Set_Config_to_Defaults();
			return;
		}
		
		filebuff = (u8*) allocate_memory (filesize);
		
		if (!filebuff)
		{
			fclose(fp);
			fatUnmount("sd");
			__io_wiisd.shutdown();
			Set_Config_to_Defaults();
			return;
		}	
		
		ret = fread(filebuff, 1, filesize, fp);

		fclose(fp);
		fatUnmount("sd");
		__io_wiisd.shutdown();
	} else
	{
		ISFS_Initialize();
		
		ret = read_file_from_nand("/NeoGamma/NeoGamma.cfg", &filebuff, &filesize);
		if (ret < 0 || filesize != 48)
		{
			if (filebuff != NULL)
			{
				free(filebuff);
			}
			Set_Config_to_Defaults();
			return;
		}
	}
	
	if ( strncmp((char *)filebuff, MAGICWORD, 16) != 0 )
	{
		free(filebuff);
		Set_Config_to_Defaults();
		return;
	}
	
	CIOS_VERSION 				= filebuff[16];

	// Ocarina/Wiird
	wiihookselect 				= filebuff[17];
	wiiocarinaselect 			= filebuff[18];
	wiidebuggerselect 			= filebuff[19];

	gchookselect 				= filebuff[20];
	gcocarinaselect 			= filebuff[21];
	gcdebuggerselect 			= filebuff[22];

	// Wii
	langselect 					= filebuff[23];
	videoselect 				= filebuff[24];
	videopatchselect 			= filebuff[25];
	viselect 					= filebuff[26];
	countrystringselect 		= filebuff[27];
	alternativedolselect 		= filebuff[28];
	patchselect 				= filebuff[29];

	// General
	storageselect 				= filebuff[30];

	// Gamecube
	gamecubemodeselect 			= filebuff[31];
	gc_videoselect 				= filebuff[32];
	gc_videopatchselect 		= filebuff[33];
	
	// Rebooter
	recoveryselect 				= filebuff[34];
	regionfreeselect 			= filebuff[35];
	nocopyselect 				= filebuff[36];
	buttonskipselect 			= filebuff[37];
	skipupdateselect 			= filebuff[38];
	bendiosreloadselect 		= filebuff[39];
	preloaderselect 			= filebuff[40];
	
	gcreloaderselect 			= filebuff[41];
	gc2ndhookselect				= filebuff[42];
	blockiosreloadselect 		= filebuff[43];
	showrebooterselect 			= filebuff[44];
	wii2ndhookselect 			= filebuff[45];
	usecorrectiosoption 		= filebuff[46];
	unusedoption7 				= filebuff[47];
	
	free(filebuff);

	if (gcreloaderselect > 1) gcreloaderselect = 1;	// Reloader settings are only for testing
	
	if (CIOS_VERSION < 3
	|| wiihookselect > 7
	|| wii2ndhookselect > 8
	|| wiiocarinaselect > 1
	|| wiidebuggerselect > 2
	|| gchookselect > 7
	|| gc2ndhookselect > 8
	|| gcocarinaselect > 1
	|| gcdebuggerselect > 2
	|| langselect > 11
	|| videoselect > 8
	|| videopatchselect > 7
	|| viselect > 1
	|| countrystringselect > 1
	|| alternativedolselect > 4
	|| patchselect > 1
	|| storageselect > 5
	|| gamecubemodeselect > 2
	|| gc_videoselect > 7
	|| gc_videopatchselect > 3
	|| recoveryselect > 1
	|| regionfreeselect > 1
	|| nocopyselect > 1
	|| buttonskipselect > 1
	|| skipupdateselect > 1
	|| bendiosreloadselect > 1
	|| preloaderselect > 1
	|| gcreloaderselect > 1
	|| blockiosreloadselect > 1
	|| showrebooterselect > 1
	|| usecorrectiosoption > 2)
	{
		Set_Config_to_Defaults();
		return;
	}
	
	configselect			= 1;
	wbfsdevice				= 0;
	gameselected 			= 0;
}

