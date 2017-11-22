#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <fat.h>
#include <wiiuse/wpad.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <dirent.h>

#include "easywbfs.h"
#include "wbfs.h"
#include "tools.h"
#include "config.h"

s32 Compare_discHdr(const void *a, const void *b)
{
	struct discHdr *hdr1 = (struct discHdr *)a;
	struct discHdr *hdr2 = (struct discHdr *)b;

	char *s1 = hdr1->title;
	char *s2 = hdr2->title;

	s32 ret;
	
	while (*s1 && *s2)
	{
		ret = toupper(*(u8*)s1) - toupper(*(u8*)s2);
		if (ret != 0)
		{
			return ret;
		}
		s1++;
		s2++;
	}
	return 0;
}

s32 Compare_Sneek_game_list_entry(const void *a, const void *b)
{
	Sneek_game_list_entry *hdr1 = (Sneek_game_list_entry *)a;
	Sneek_game_list_entry *hdr2 = (Sneek_game_list_entry *)b;

	char *s1 = hdr1->title;
	char *s2 = hdr2->title;

	s32 ret;
	
	while (*s1 && *s2)
	{
		ret = toupper(*(u8*)s1) - toupper(*(u8*)s2);
		if (ret != 0)
		{
			return ret;
		}
		s1++;
		s2++;
	}
	return 0;
}

s32 Compare_DML_game_list_entry(const void *a, const void *b)
{
	DML_game_list_entry *hdr1 = (DML_game_list_entry *)a;
	DML_game_list_entry *hdr2 = (DML_game_list_entry *)b;

	char *s1 = hdr1->title;
	char *s2 = hdr2->title;

	s32 ret;
	
	while (*s1 && *s2)
	{
		ret = toupper(*(u8*)s1) - toupper(*(u8*)s2);
		if (ret != 0)
		{
			return ret;
		}
		s1++;
		s2++;
	}
	return 0;
}

struct discHdr *gameList[2] = { NULL , NULL };
u32 gameCnt[2] = { 0 , 0 };
bool done[2] = { false , false};
u32 DML_GameCount = 0;

struct discHdr *getGameHeader(u32 index)
{
	if (wbfsdevice == 0 || index >= gameCnt[wbfsdevice-1] || done[wbfsdevice-1] == false) return NULL;
	
	return &gameList[wbfsdevice-1][index];
}

s32 get_DML_game_list()
{
	DISC_INTERFACE storage = __io_wiisd;
	u32 temp_DML_GameCount = 0;
	FILE *fp;
	u32 i = 0;
	DML_GameCount = 0;
	s32 ret;
	static bool game_list_allocated = false;
	
	ret = storage.startup();
	if (ret < 0) 
	{
		return -1;
	}
	ret = fatMountSimple("fat", &storage);
	if (ret < 0) 
	{
		storage.shutdown();
		return -2;
	}
	
	static char name_buffer[64] ATTRIBUTE_ALIGN(32);;
    DIR *sdir;
    DIR *s2dir;
    struct dirent *entry;

	// 1st count the number of games
	sdir = opendir("fat:/games");
	do
	{
		entry = readdir(sdir);
		if (entry)
		{
			sprintf(name_buffer, "fat:/games/%s", entry->d_name);
			if (strlen(entry->d_name) != 6)
			{
				continue;
			}
			s2dir =  opendir(name_buffer);
			if (s2dir)
			{
				sprintf(name_buffer, "fat:/games/%s/game.iso", entry->d_name);
				fp = fopen(name_buffer, "rb");
				if (fp)
				{
					fseek(fp, 0, SEEK_END);
					if (ftell(fp) > 1000000)
					{
						temp_DML_GameCount++;
					}
					fclose(fp);
				}
				closedir(s2dir);
			}
		}
	} while (entry);
	closedir(sdir);
	
	if (temp_DML_GameCount == 0)
	{
		fatUnmount("fat");
		storage.shutdown();
		return -3;
	}
	
	if (game_list_allocated)
	{
		free(DML_game_list);
	}
	
	DML_game_list = (DML_game_list_entry *)allocate_memory(sizeof(DML_game_list_entry) * temp_DML_GameCount);
	if (DML_game_list == NULL)
	{
		fatUnmount("fat");
		storage.shutdown();
		return -4;
	}
	game_list_allocated = true;

	// Build the game list
	sdir = opendir("fat:/games");
	do
	{
		entry = readdir(sdir);
		if (entry)
		{
			sprintf(name_buffer, "fat:/games/%s", entry->d_name);
			if (strlen(entry->d_name) != 6)
			{
				continue;
			}
			s2dir = opendir(name_buffer);
			if (s2dir)
			{
				sprintf(name_buffer, "fat:/games/%s/game.iso", entry->d_name);
				fp = fopen(name_buffer, "rb");
				if (fp)
				{
					fseek(fp, 0, SEEK_END);
					if (ftell(fp) > 1000000)
					{
						memcpy(DML_game_list[i].discid, entry->d_name, 6);
						fseek(fp, 0x20, SEEK_SET);
						ret = fread(DML_game_list[i].title, 1, 0x40, fp);
						if (ret == 0x40)
						{						
							i++;
						}
					}
					fclose(fp);
				}
				closedir(s2dir);
			}
		}
	} while (entry);
	closedir(sdir);
	
	DML_GameCount = i;

	fatUnmount("fat");
	storage.shutdown();

	if (DML_GameCount == 0)
	{
		return -3;
	}

	qsort(DML_game_list, DML_GameCount, sizeof(DML_game_list_entry), Compare_DML_game_list_entry);
	
	return 0;
}

s32 DML_select_game()
{
	s32 ret;
	print_status("Writing boot.bin");
	wait(2);
	
	DISC_INTERFACE storage = __io_wiisd;
	FILE *fp;
	static char name_buffer[64] ATTRIBUTE_ALIGN(32);

	ret = storage.startup();
	if (ret < 0) 
	{
		print_status("Storage error");
		wait(3);
		return -1;
	}
	ret = fatMountSimple("sd", &storage);
	if (ret < 0) 
	{
		print_status("FAT error");
		wait(3);
		storage.shutdown();
		return -1;
	}

	sprintf(name_buffer, "sd:/games/boot.bin");
	fp = fopen(name_buffer, "wb");
	if (!fp)
	{
		print_status("File error");
		fatUnmount("sd");
		storage.shutdown();
		return -1;
	}
	
	fwrite(DML_game_list[gameselected].discid, 1, 6, fp);
	fclose(fp);

	fatUnmount("sd");
	storage.shutdown();

	return 0;
}


s32 Sneek_build_sorted_game_list()
{
	int i;
	int j = 0;
	u32 temp_Sneek_GameCount = 0;
	
	for (i=0;i < Sneek_GameCount;i++)
	{
		if ( *(u32 *)(Sneek_DICfg->GameInfo[i]+0x18) == 0x5D1C9EA3 || *(u32 *)(Sneek_DICfg->GameInfo[i]+0x1C) == 0xC2339F3D)	// Only add confirmed Wii and GC titles to the list
		{
			temp_Sneek_GameCount++;
		}	
	}
	
	Sneek_game_list = (Sneek_game_list_entry *)allocate_memory(sizeof(Sneek_game_list_entry) * temp_Sneek_GameCount);
	if (Sneek_game_list == NULL)
	{
		return -1;
	}
	
	for (i=0;i < Sneek_GameCount;i++)
	{
		if ( *(u32 *)(Sneek_DICfg->GameInfo[i]+0x18) == 0x5D1C9EA3 || *(u32 *)(Sneek_DICfg->GameInfo[i]+0x1C) == 0xC2339F3D)	// Only add confirmed Wii and GC titles to the list
		{
			Sneek_game_list[j].index = i;
			memcpy((void *)(Sneek_game_list[j].title), (void *)(Sneek_DICfg->GameInfo[i]+0x20), 0x40);
			Sneek_game_list[j].title[0x3f] = 0;
			j++;
		}
	}
	
	free(Sneek_DICfg);
	
	Sneek_GameCount = temp_Sneek_GameCount;
	
	qsort(Sneek_game_list, Sneek_GameCount, sizeof(Sneek_game_list_entry), Compare_Sneek_game_list_entry);
	
	return 0;
}

char *getGameTitle(u32 index)
{
	if (Sneek_Mode == true)
	{
		return (char *)(Sneek_game_list[index].title);
	}

	if (wbfsdevice == 3)
	{
		if (DML_GameCount == 0)
		{
			return "Press A to search games";
		} else
		{
			return (char *)(DML_game_list[index].title);
		}
	}

	if (wbfsdevice == 0)
	{
		return "This shouldn't happen";
	}
	if (done[wbfsdevice-1] == false )
	{
		return "Press A to mount WBFS";
	}
	if (index >= gameCnt[wbfsdevice-1])
	{
		return "Error mounting WBFS";
	}
	
	return getGameHeader(index)->title;
}

s32 getGameCount()
{
	if (Sneek_Mode == true)
	{
		return Sneek_GameCount;
	}
	
	if (wbfsdevice == 3)
	{
		return DML_GameCount;
	}

	if (done[wbfsdevice-1] == false)
	{
		return 0;
	}
	return gameCnt[wbfsdevice-1];
}

s32 Try_WBFS_Init()
{
	if (wbfsdevice == 0) return 0;
	if (done[wbfsdevice-1]) return 0;
	s32 ret;

	if (wbfsdevice == 1)
	{
		ret = WBFS_Init(WBFS_DEVICE_USB, 1);
	} else
	{
		ret = WBFS_Init(WBFS_DEVICE_SDHC, 1);
	}
	return ret;
}

s32 initGameList()
{
	if (wbfsdevice == 0) return 0;
	if (done[wbfsdevice-1]) return 0;
	struct discHdr *buffer = NULL;

	u32 cnt, len;
	s32 ret;

	gameCnt[wbfsdevice-1] = 0;
	
	/* Free memory */
	if (gameList[wbfsdevice-1])
		free(gameList[wbfsdevice-1]);

	ret = WBFS_Open();
	if (ret < 0)
	{
		//printf("Error: WBFS_Open fail\n");
		return ret;
	}

	/* Get list length */
	ret = WBFS_GetCount(&cnt);
	if (ret < 0)
	{
		//printf("Error: WBFS_GetCount fail\n");
		return -6;
	}
	
	if (cnt == 0)
	{
		// No games
		return -7;
	}

	/* Buffer length */
	len = sizeof(struct discHdr) * cnt;

	/* Allocate memory */
	buffer = (struct discHdr *)allocate_memory(len);
	if (!buffer)
	{
		return -8;
	}
	/* Clear buffer */
	memset(buffer, 0, len);

	/* Get header list */
	ret = WBFS_GetHeaders(buffer, cnt, sizeof(struct discHdr));
	if (ret < 0)
	{
		//printf("Error: WBFS_GetHeaders fail\n");
		free(buffer);
		return -9;
	}		

	qsort(buffer, cnt, sizeof(struct discHdr), Compare_discHdr);

	/* Set values */
	gameList[wbfsdevice-1] = buffer;
	gameCnt[wbfsdevice-1]  = cnt;
	done[wbfsdevice-1] = true;

	return 0;
}

