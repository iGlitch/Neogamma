#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>

#include "sdhc.h"
#include "usbstorage.h"
//#include "utils.h"
//#include "video.h"
#include "wbfs.h"
//#include "wdvd.h"

#include "libwbfs/libwbfs.h"

//X Constants XX
#define MAX_NB_SECTORS	32

//X WBFS HDD XX
static wbfs_t *hdd = NULL;

//X WBFS callbacks XX
static rw_sector_callback_t readCallback  = NULL;
static rw_sector_callback_t writeCallback = NULL;

//X Variables XX
static u32 nb_sectors, sector_size;

/*
void __WBFS_Spinner(s32 x, s32 max)
{
	static time_t start;
	static u32 expected;

	f32 percent, size;
	u32 d, h, m, s;

	//X First time XX
	if (!x) {
		start    = time(0);
		expected = 300;
	}

	//X Elapsed time XX
	d = time(0) - start;

	if (x != max) {
		//X Expected time XX
		if (d)
			expected = (expected * 3 + d * max / x) / 4;

		//X Remaining time XX
		d = (expected > d) ? (expected - d) : 0;
	}

	//X Calculate time values XX
	h =  d / 3600;
	m = (d / 60) % 60;
	s =  d % 60;

	//X Calculate percentage/size XX
	percent = (x * 100.0) / max;
	size    = (hdd->wii_sec_sz / GB_SIZE) * max;

	Con_ClearLine();

	//X Show progress XX
	if (x != max) {
		printf("    %.2f%% of %.2fGB (%c) ETA: %d:%02d:%02d\r", percent, size, "/|\\-"[(x / 10) % 4], h, m, s);
		fflush(stdout);
	} else
		printf("    %.2fGB copied in %d:%02d:%02d\n", size, h, m, s);
}

s32 __WBFS_ReadDVD(void *fp, u32 lba, u32 len, void *iobuf)
{
	void *buffer = NULL;

	u64 offset;
	u32 mod, size;
	s32 ret;

	//X Calculate offset XX
	offset = ((u64)lba) << 2;

	//X Calcualte sizes XX
	mod  = len % 32;
	size = len - mod;

	//X Read aligned data XX
	if (size) {
		ret = WDVD_UnencryptedRead(iobuf, size, offset);
		if (ret < 0)
			goto out;
	}

	//X Read non-aligned data XX
	if (mod) {
		//X Allocate memory XX
		buffer = allocate_memory(0x20);
		if (!buffer)
			return -1;

		//X Read data XX
		ret = WDVD_UnencryptedRead(buffer, 0x20, offset + size);
		if (ret < 0)
			goto out;

		//X Copy data XX
		memcpy(iobuf + size, buffer, mod);
	}

	//X Success XX
	ret = 0;

out:
	//X Free memory XX
	if (buffer)
		free(buffer);

	return ret;
}
*/
s32 __WBFS_ReadUSB(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	//X Do reads XX
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		//X Read sectors is too big XX
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		//X USB read XX
		ret = USBStorage_ReadSectors(lba + cnt, sectors, ptr);
		if (ret < 0)
			return ret;

		//X Increment counter XX
		cnt += sectors;
	}

	return 0;
}
/*
s32 __WBFS_WriteUSB(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	//X Do writes XX
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		//X Write sectors is too big XX
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		//X USB write XX
		ret = USBStorage_WriteSectors(lba + cnt, sectors, ptr);
		if (ret < 0)
			return ret;

		//X Increment counter XX
		cnt += sectors;
	}

	return 0;
}
*/
s32 __WBFS_ReadSDHC(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	//X Do reads XX
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		//X Read sectors is too big XX
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		//X SDHC read XX
		ret = SDHC_ReadSectors(lba + cnt, sectors, ptr);
		if (!ret)
			return -1;

		//X Increment counter XX
		cnt += sectors;
	}

	return 0;
}
/*
s32 __WBFS_WriteSDHC(void *fp, u32 lba, u32 count, void *iobuf)
{
	u32 cnt = 0;
	s32 ret;

	//X Do writes XX
	while (cnt < count) {
		void *ptr     = ((u8 *)iobuf) + (cnt * sector_size);
		u32   sectors = (count - cnt);

		//X Write sectors is too big XX
		if (sectors > MAX_NB_SECTORS)
			sectors = MAX_NB_SECTORS;

		//X SDHC write XX
		ret = SDHC_WriteSectors(lba + cnt, sectors, ptr);
		if (!ret)
			return -1;

		//X Increment counter XX
		cnt += sectors;
	}

	return 0;
}
*/

bool WBFS_isalive(u32 device)
{
	switch (device) 
	{
		case WBFS_DEVICE_USB: 
			// TODO find a way to check if usb is alive
			break;

		case WBFS_DEVICE_SDHC: 
			if (!SDHC_Init())
			{
				return false;
			}
			break;
		
		default:
			return false;
	}
	return true;
}	



s32 WBFS_Init(u32 device, u32 timeout)
{
	u32 cnt;
	s32 ret = -1;

	//X Wrong timeout XX
	if (!timeout)
		return -1;

	//X Try to mount device XX
	for (cnt = 0; cnt < timeout; cnt++) 
	{
		switch (device) 
		{
			case WBFS_DEVICE_USB: 
				//X Initialize USB storage XX
				ret = USBStorage_Init(&nb_sectors, &sector_size);

				if (ret >= 0) 
				{
					//X Setup callbacks XX
					readCallback  = __WBFS_ReadUSB;
					//writeCallback = __WBFS_WriteUSB;

					//X Device info XX
					//nb_sectors = USBStorage_GetCapacity(&sector_size);
					
					goto out;
				}
				break;
				
			case WBFS_DEVICE_SDHC: 
				//X Initialize SDHC XX
				ret = SDHC_Init();

				if (ret >= 0) 
				{
					//X Setup callbacks XX
					readCallback  = __WBFS_ReadSDHC;
					//writeCallback = __WBFS_WriteSDHC;

					//X Device info XX
					nb_sectors  = 0;
					sector_size = SDHC_SECTOR_SIZE;

					goto out;
				} else
				{
					ret = -1;
				}
				break;

			default:
				return -1;
		}

		//X Sleep 1 second XX
		sleep(1);
	}

out:
	return ret;
}

s32 WBFS_Open(void)
{
	//X Close hard disk XX
	if (hdd)
		wbfs_close(hdd);

	s32 ret;
	//X Open hard disk XX
	ret = wbfs_open_hd(readCallback, writeCallback, NULL, sector_size, nb_sectors, 0, &hdd);
	
	if (ret < 0)
	{
		return ret;
	}
	
	if (!hdd)
	{
		// Partition found but pointer is NULL...
		return -5;
	}

	return 0;
}

/*
s32 WBFS_Format(u32 lba, u32 size)
{
	wbfs_t *partition = NULL;

	//X Reset partition XX
	partition = wbfs_open_partition(readCallback, writeCallback, NULL, sector_size, size, lba, 1);
	if (!partition)
		return -1;

	//X Free memory XX
	wbfs_close(partition);

	return 0;
}
*/
s32 WBFS_GetCount(u32 *count)
{
	//X No device open XX
	if (!hdd)
		return -1;

	//X Get list length XX
	*count = wbfs_count_discs(hdd);

	return 0;
}

s32 WBFS_GetHeaders(void *outbuf, u32 cnt, u32 len)
{
	u32 idx, size;
	s32 ret;

	//X No device open XX
	if (!hdd)
		return -1;

	for (idx = 0; idx < cnt; idx++) {
		u8 *ptr = ((u8 *)outbuf) + (idx * len);

		//X Get header XX
		ret = wbfs_get_disc_info(hdd, idx, ptr, len, &size);
		if (ret < 0)
			return ret;
	}

	return 0;
}

s32 WBFS_CheckGame(u8 *discid)
{
	wbfs_disc_t *disc = NULL;

	//X Try to open game disc XX
	disc = wbfs_open_disc(hdd, discid);
	if (disc) {
		//X Close disc XX
		wbfs_close_disc(disc);

		return 1;
	}

	return 0;
}
/*
s32 WBFS_AddGame(void)
{
	s32 ret;

	//X No device open XX
	if (!hdd)
		return -1;

	//X Add game to device XX
	ret = wbfs_add_disc(hdd, __WBFS_ReadDVD, NULL, __WBFS_Spinner, ALL_PARTITIONS, 0);
	if (ret < 0)
		return ret;

	return 0;
}

s32 WBFS_RemoveGame(u8 *discid)
{
	s32 ret;

	//X No device open XX
	if (!hdd)
		return -1;

	//X Remove game from device XX
	ret = wbfs_rm_disc(hdd, discid);
	if (ret < 0)
		return ret;

	return 0;
}

s32 WBFS_GameSize(u8 *discid, f32 *size)
{
	wbfs_disc_t *disc = NULL;

	u32 sectors;

	//X No device open XX
	if (!hdd)
		return -1;

	//X Open disc XX
	disc = wbfs_open_disc(hdd, discid);
	if (!disc)
		return -2;

	//X Get game size in sectors XX
	sectors = wbfs_sector_used(hdd, disc->header);

	//X Close disc XX
	wbfs_close_disc(disc);

	//X Copy value XX
	*size = (hdd->wbfs_sec_sz / GB_SIZE) * sectors;

	return 0;
}

s32 WBFS_DiskSpace(f32 *used, f32 *free)
{
	f32 ssize;
	u32 cnt;

	//X No device open XX
	if (!hdd)
		return -1;

	//X Count used blocks XX
	cnt = wbfs_count_usedblocks(hdd);

	//X Sector size in GB XX
	ssize = hdd->wbfs_sec_sz / GB_SIZE;

	//X Copy values XX
	*free = ssize * cnt;
	*used = ssize * (hdd->n_wbfs_sec - cnt);

	return 0;
}
*/
