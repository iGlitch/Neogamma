/*-------------------------------------------------------------

libcios.c -- Custom IOS library

Copyright (C) 2008 (waninkoko)

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <sys/unistd.h>
#include <wiiuse/wpad.h>

#include "libcios.h"
#include "config.h"
#include "tools.h"
#include "sha1.h"

#define CERTS_SIZE	0xA00

/* Variables */
static const char  certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";

s32 GetTMD(u64 TicketID, signed_blob **Output, u32 *Length)
{
	signed_blob* TMD = NULL;

	u32 TMD_Length;
	s32 ret;

	/* Retrieve TMD length */
	ret = ES_GetStoredTMDSize(TicketID, &TMD_Length);
	if (ret < 0)
		return ret;

	/* Allocate memory */
	TMD = (signed_blob*)memalign(32, (TMD_Length+31)&(~31));
	if (!TMD)
		return IPC_ENOMEM;

	/* Retrieve TMD */
	ret = ES_GetStoredTMD(TicketID, TMD, TMD_Length);
	if (ret < 0)
	{
		free(TMD);
		return ret;
	}

	/* Set values */
	*Output = TMD;
	*Length = TMD_Length;

	return 0;
}


s32 Load_IOS(u32 version)
{
	signed_blob *TMD = NULL;
	tmd *t = NULL;
	u32 TMD_size = 0;

	// Get tmd to determine the version of the IOS
	int ret = GetTMD((((u64)(1) << 32) | (version)), &TMD, &TMD_size);
	
	if (ret == 0)
	{
		t = (tmd*)SIGNATURE_PAYLOAD(TMD);
		if (t->title_version == 65280)
		{
			version = 36;
		}
		free(TMD);
	}
	
	WPAD_Shutdown();
	/* Load Custom IOS */

	block_ios_reload(false);
	ISFS_Deinitialize();
	
	ret = IOS_ReloadIOS(version);
	
	block_ios_reload(false);
	set_cIOS_stealth_mode(false);

	PAD_Init();
	WPAD_Init();
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	return ret;
}

void block_ios_reload(bool enable)
{
	if (Sneek_Mode == true)
	{
		return;
	}
	
	int es_fd = IOS_Open("/dev/es", 0);
	
	if (es_fd < 0)
	{
		return;
	}

	static ioctlv vector[0x08] ATTRIBUTE_ALIGN(32);

	static int mode ATTRIBUTE_ALIGN(32);
    static int ios ATTRIBUTE_ALIGN(32);
	if (enable)
	{
		mode = 2;
	} else
	{
		mode = 0;
	}
	
	ios = CIOS_VERSION;
	
	vector[0].data = &mode;
	vector[0].len = 4;
	vector[1].data = &ios;
	vector[1].len = 4;

	s32 ret = IOS_Ioctlv(es_fd, 0xA0, 2, 0, vector);

	// print_status will crash the wii if it's executed before loading the graphics!
	if (enable)
	{
		if (ret < 0)
		{
			print_status("IOS Reload block error");
			wait(3);
		} else
		{
			print_status("Blocking IOS reloads");
			wait(1);
		}
	}

	IOS_Close(es_fd);
}

void set_cIOS_stealth_mode(bool enable)		// The ability to switch this was introduced in d2x rev8
{
	if (Sneek_Mode == true)
	{
		return;
	}
	
	int mload_fd = IOS_Open("/dev/mload", 0);
	
	if (mload_fd < 0)
	{
		return;
	}

	static ioctlv vector[0x08] ATTRIBUTE_ALIGN(32);

	static int mode ATTRIBUTE_ALIGN(32);
 	if (enable)
	{
		mode = 1;
	} else
	{
		mode = 0;
	}
	
	vector[0].data = &mode;
	vector[0].len = 4;

	IOS_Ioctlv(mload_fd, 0x4D4C44E0, 1, 0, vector);

	IOS_Close(mload_fd);
}

void tell_cIOS_to_return_to_channel()
{
    if (TITLE_UPPER(old_title_id) > 1 && TITLE_LOWER(old_title_id) > 2)	// Don't change anything for system menu or no title id
	{
		static u64 sm_title_id  ATTRIBUTE_ALIGN(32);
		sm_title_id = old_title_id; // title id to be launched in place of the system menu

		int ret;
		signed_blob *buf = NULL;
		u32 filesize;
		
		// Check if the title exists NeoGamma wants the cIOS to return to
		ret = GetTMD(sm_title_id, &buf, &filesize);
		if (buf != NULL)
		{
			free(buf);
		}

		if (ret < 0)
		{
			return;
		}
		
		static ioctlv vector[0x08] ATTRIBUTE_ALIGN(32);

		vector[0].data = &sm_title_id;
		vector[0].len = 8;

		int es_fd = IOS_Open("/dev/es", 0);
		if (es_fd < 0)
		{
			print_status("Couldn't open ES module(2)");
			wait(5);	
		}
		
		ret = IOS_Ioctlv(es_fd, 0xA1, 1, 0, vector);

		IOS_Close(es_fd);
		
		if (ret < 0)
		{
			//print_status("ret = %d", ret);
			//wait(5);	
		}
	}
}




/*
 * __CIOS_GetCerts() - Reads the system certificates
 */

s32 __CIOS_GetCerts(signed_blob **out, u32 *len)
{
	signed_blob *p_certs = NULL;
	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, ISFS_OPEN_READ);
	if (fd < 0)
		return fd;

	/* Allocate memory */
	p_certs = (signed_blob *)memalign(32, CERTS_SIZE);
	if (!p_certs) {
		ret = IPC_ENOMEM;
		goto err;
	}

	/* Read certificates */
	ret = IOS_Read(fd, p_certs, CERTS_SIZE);
	if (ret < 0)
		goto err;

	/* Set values */
	*out = p_certs;
	*len = CERTS_SIZE;

	ret = 0;
	goto out;

err:
	/* Free memory */
	if (p_certs)
		free(p_certs);

out:
	/* Close file */
	if (fd >= 0)
		IOS_Close(fd);

	return ret;
}
