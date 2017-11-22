/*
config_bytes[2] hooktypes
0 no
1 VBI
2 KPAD read
3 Joypad Hook
4 GXDraw Hook
5 GXFlush Hook
6 OSSleepThread Hook
7 AXNextFrame Hook
8 Default

config_bytes[4] ocarina
0 no
1 yes

config_bytes[5] paused start
0 no
1 yes


config_bytes[7] debugger
0 no
1 yes
*/

#include <gccore.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/unistd.h>



#include "codehandler_bin.h"
//#include "codehandlerslota_bin.h"
#include "codehandleronly_bin.h"
#include "multidol.h"
#include "patchcode.h"

#include "tools.h"
#include "config.h"
#include "codes.h"
#include "font.h"
#include "easywbfs.h"
#include "storage.h"

static u8 *codelistend;
void *codelist;

u32 gameconfsize = 0;
u32 *gameconf = NULL;

void wait(u32 s);
void print_status(const char *Format, ...);

s32 load_codes(char *disc_id, u32 maxsize, u8 *buffer)
{
	u32 filesize;
	u32 ret;
	char buf[128];
	
	char discid4[5];
	char discid6[7];
	char discid7[8];

	memset(discid4, 0, 5);
	memset(discid6, 0, 7);
	memset(discid7, 0, 8);
	
	memcpy(discid4, disc_id, 4);
	memcpy(discid6, disc_id, 6);
	memcpy(discid7, disc_id, 6);
	
	discid7[6] = '1' + disc_id[6];
	
	fflush(stdout);
	
	if (Sneek_Mode == false)
	{
		FILE *fp;
		
		sprintf(buf, "fat:/NeoGamma/codes/%s.gct", discid7);
		fp = fopen(buf, "rb");

		if (!fp) 
		{
			sprintf(buf, "fat:/NeoGamma/codes/%s.gct", discid6);
			fp = fopen(buf, "rb");
		}

		if (!fp) 
		{
			sprintf(buf, "fat:/NeoGamma/codes/%s.gct", discid4);
			fp = fopen(buf, "rb");
		}

		if (!fp) 
		{
			sprintf(buf, "fat:/codes/%s.gct", discid7);
			fp = fopen(buf, "rb");
		}

		if (!fp) 
		{
			sprintf(buf, "fat:/codes/%s.gct", discid6);
			fp = fopen(buf, "rb");
		}

		if (!fp) 
		{
			sprintf(buf, "fat:/codes/%s.gct", discid4);
			fp = fopen(buf, "rb");
		}

		if (!fp) 
		{
			//printf("Failed to open %s\n", buf);
			//printf("No %s codes found\n", storagename);
			//sleep(3);
			print_status("No %s codes found", storagename);
			return -1;
		}

		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		if (filesize > maxsize)
		{
			//printf("Too many codes\n");
			print_status("Too many codes");
			return -1;
		}	
		
		ret = fread(buffer, 1, filesize, fp);
		if(ret != filesize)
		{	
			fclose(fp);
			//printf("%s Code Error\n", storagename);
			print_status("%s Code Error", storagename);
			return -1;
		}

		fclose(fp);
	} else
	{
		u8 *temp_codes_buffer = NULL;

		ISFS_Initialize();

		sprintf(buf, "/NeoGamma/codes/%s.gct", discid7);
		ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		if (ret < 0) 
		{
			sprintf(buf, "/NeoGamma/codes/%s.gct", discid6);
			ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		}

		if (ret < 0) 
		{
			sprintf(buf, "/NeoGamma/codes/%s.gct", discid4);
			ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		}

		if (ret < 0) 
		{
			sprintf(buf, "/codes/%s.gct", discid7);
			ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		}

		if (ret < 0) 
		{
			sprintf(buf, "/codes/%s.gct", discid6);
			ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		}

		if (ret < 0) 
		{
			sprintf(buf, "/codes/%s.gct", discid4);
			ret = read_file_from_nand(buf, &temp_codes_buffer, &filesize);
		}

		if (ret < 0) 
		{
			//printf("Failed to open %s\n", buf);
			//printf("No %s codes found\n", storagename);
			//sleep(3);
			print_status("No NAND codes found");
			return -1;
		}

		if (filesize > maxsize)
		{
			//printf("Too many codes\n");
			print_status("Too many codes");
			free(temp_codes_buffer);
			return -1;
		}	
		memcpy(buffer, temp_codes_buffer, filesize);
		free(temp_codes_buffer);
	}
	
	return 0;
}



//---------------------------------------------------------------------------------
void load_handler()
//---------------------------------------------------------------------------------
{
	if (use_wii_Ocarina_engine())
	{
		if (wiidebuggerselect > 0)
		{
			switch(gecko_channel)
			{
				case 0: // Slot A
					/*
					memset((void*)0x80001800,0,codehandlerslota_size);
					memcpy((void*)0x80001800,codehandlerslota,codehandlerslota_size);
					if (pausedstartoption == 0x01)
						*(u32*)0x80002774 = 1;
					memcpy((void*)0x80001CDE, &codelist, 2);
					memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
					memcpy((void*)0x80001F7E, &codelist, 2);
					memcpy((void*)0x80001F82, ((u8*) &codelist) + 2, 2);
					DCFlushRange((void*)0x80001800,codehandlerslota_size);
					break;
					*/
				case 1:	// slot B
				default:
					memset((void*)0x80001800, 0, codehandler_bin_size);
					memcpy((void*)0x80001800, codehandler_bin, codehandler_bin_size);
					
					// Paused start
					if (wiidebuggerselect == 2)
					{
						*(u32*)0x80002774 = 1;
					}
					
					memcpy((void*)0x80001CDE, &codelist, 2);
					memcpy((void*)0x80001CE2, ((u8*) &codelist) + 2, 2);
					memcpy((void*)0x80001F5A, &codelist, 2);
					memcpy((void*)0x80001F5E, ((u8*) &codelist) + 2, 2);
					
					DCFlushRange((void*)0x80001800, codehandler_bin_size);
					break;
					
				break;
			}
		}
		else
		{
			memset((void*)0x80001800,0,codehandleronly_bin_size);
			memcpy((void*)0x80001800,codehandleronly_bin,codehandleronly_bin_size);
			memcpy((void*)0x80001906, &codelist, 2);
			memcpy((void*)0x8000190A, ((u8*) &codelist) + 2, 2);
			DCFlushRange((void*)0x80001800,codehandleronly_bin_size);
		}
		
		// Multi .dol hook if a 2nd hook is selected, or hook selected and 2ndhook set to = 1st hook
		if ((wii2ndhookselect > 0 && wii2ndhookselect < 8) || (wii2ndhookselect == 8 && wiihookselect != 0))
		{			
			// Load multidol handler
			memset((void*)0x80001000,0,multidol_size);
			memcpy((void*)0x80001000,multidol,multidol_size); 
			DCFlushRange((void*)0x80001000,multidol_size);
			
			u8 temp2ndhook;
			
			if (wii2ndhookselect == 8)
			{
				temp2ndhook = wiihookselect;
			} else
			{
				temp2ndhook = wii2ndhookselect;
			}
			
			switch(temp2ndhook)
			{
				case 0x01:
					memcpy((void*)0x8000119C,viwiihooks,12);
					memcpy((void*)0x80001198,viwiihooks+3,4);
					break;
				case 0x02:
					memcpy((void*)0x8000119C,kpadhooks,12);
					memcpy((void*)0x80001198,kpadhooks+3,4);
					break;
				case 0x03:
					memcpy((void*)0x8000119C,joypadhooks,12);
					memcpy((void*)0x80001198,joypadhooks+3,4);
					break;
				case 0x04:
					memcpy((void*)0x8000119C,gxdrawhooks,12);
					memcpy((void*)0x80001198,gxdrawhooks+3,4);
					break;
				case 0x05:
					memcpy((void*)0x8000119C,gxflushhooks,12);
					memcpy((void*)0x80001198,gxflushhooks+3,4);
					break;
				case 0x06:
					memcpy((void*)0x8000119C,ossleepthreadhooks,12);
					memcpy((void*)0x80001198,ossleepthreadhooks+3,4);
					break;
				case 0x07:
					memcpy((void*)0x8000119C,axnextframehooks,12);
					memcpy((void*)0x80001198,axnextframehooks+3,4);
					break;
			}
			DCFlushRange((void*)0x80001000,multidol_size);
			ICInvalidateRange((void*)0x80001000,multidol_size);
		}
	}

	memcpy((void *)0x80001800, (void*)0x80000000, 6);
	memset(codelist, 0, (u32)codelistend - (u32)codelist);
}

//---------------------------------------------------------------------------------
void app_loadgameconfig()
//---------------------------------------------------------------------------------
{
	FILE* fp;
	s32 ret;
	u32 filesize;
	s32 gameidmatch, maxgameidmatch = -1, maxgameidmatch2 = -1;
	u32 i, numnonascii, parsebufpos;
	u32 codeaddr, codeval, codeaddr2, codeval2, codeoffset;
	u32 temp, tempoffset = 0;
	char parsebuffer[18];
	
	//if (config_bytes[2] == 8)
	//	hookset = 1;
	
	u8 *tempgameconf = NULL;
	u32 tempgameconfsize = 0;
	
	//memcpy(tempgameconf, defaultgameconfig, defaultgameconfig_size);
	//tempgameconf[defaultgameconfig_size] = '\n';
	//tempgameconfsize = defaultgameconfig_size + 1;

	if (Sneek_Mode == false)
	{
		fp = fopen("fat:/NeoGamma/gameconfig.txt", "rb");
			
		if (!fp) fp = fopen("fat:/gameconfig.txt", "rb");
		if (!fp) fp = fopen("fat:/codes/gameconfig.txt", "rb");
				
		if (fp) {
			fseek(fp, 0, SEEK_END);
			filesize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			
			tempgameconf = malloc(filesize);
			if (tempgameconf == NULL)
			{
				print_status("Out of memory");
				wait(4);
				return;
			}
			
			ret = fread((void*)tempgameconf, 1, filesize, fp);
			fclose(fp);
			if (ret != filesize)
			{
				print_status("Error reading gameconfig.txt");
				wait(4);
				return;
			}
			tempgameconfsize = filesize;
		} else
		{
			return;
		}
	} else
	{
		ISFS_Initialize();

		ret = read_file_from_nand("/NeoGamma/gameconfig.txt", &tempgameconf, &filesize);
		if (ret < 0) 
		{
			ret = read_file_from_nand("/gameconfig.txt", &tempgameconf, &filesize);
		}
		if (ret < 0) 
		{
			ret = read_file_from_nand("/codes/gameconfig.txt", &tempgameconf, &filesize);
		}

		if (ret < 0)
		{
			return;
		} else
		{
			tempgameconfsize = filesize;
		}
	}
	
	// Remove non-ASCII characters
	numnonascii = 0;
	for (i = 0; i < tempgameconfsize; i++)
	{
		if (tempgameconf[i] < 9 || tempgameconf[i] > 126)
			numnonascii++;
		else
			tempgameconf[i-numnonascii] = tempgameconf[i];
	}
	tempgameconfsize -= numnonascii;
	
	*(tempgameconf + tempgameconfsize) = 0;
	//gameconf = (tempgameconf + tempgameconfsize) + (4 - (((u32) (tempgameconf + tempgameconfsize)) % 4));
	
	for (maxgameidmatch = 0; maxgameidmatch <= 6; maxgameidmatch++)
	{
		i = 0;
		while (i < tempgameconfsize)
		{
			maxgameidmatch2 = -1;
			while (maxgameidmatch != maxgameidmatch2)
			{
				while (i != tempgameconfsize && tempgameconf[i] != ':') i++;
				if (i == tempgameconfsize) break;
				while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0)) i--;
				if (i != 0) i++;
				parsebufpos = 0;
				gameidmatch = 0;
				while (tempgameconf[i] != ':')
				{
					if (tempgameconf[i] == '?')
					{
						parsebuffer[parsebufpos] = ((char*)0x80000000)[parsebufpos];
						parsebufpos++;
						gameidmatch--;
						i++;
					}
					else if (tempgameconf[i] != 0 && tempgameconf[i] != ' ')
						parsebuffer[parsebufpos++] = tempgameconf[i++];
					else if (tempgameconf[i] == ' ')
						break;
					else
						i++;
					if (parsebufpos == 8) break;
				}
				parsebuffer[parsebufpos] = 0;
				if (strncasecmp("DEFAULT", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 7)
				{
					gameidmatch = 0;
					goto idmatch;
				}
				if (strncmp((char*)0x80000000, parsebuffer, strlen(parsebuffer)) == 0)
				{
					gameidmatch += strlen(parsebuffer);
				idmatch:
					if (gameidmatch > maxgameidmatch2)
					{
						maxgameidmatch2 = gameidmatch;
					}
				}
				while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13)) i++;
			}
			while (i != tempgameconfsize && tempgameconf[i] != ':')
			{
				parsebufpos = 0;
				while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13))
				{
					if (tempgameconf[i] != 0 && tempgameconf[i] != ' ' && tempgameconf[i] != '(' && tempgameconf[i] != ':')
						parsebuffer[parsebufpos++] = tempgameconf[i++];
					else if (tempgameconf[i] == ' ' || tempgameconf[i] == '(' || tempgameconf[i] == ':')
						break;
					else
						i++;
					if (parsebufpos == 17) break;
				}
				parsebuffer[parsebufpos] = 0;
				//if (!autobootcheck)
				{
					//if (strncasecmp("addtocodelist(", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 14)
					//{
					//	ret = sscanf(tempgameconf + i, "%x %x", &codeaddr, &codeval);
					//	if (ret == 2)
					//		addtocodelist(codeaddr, codeval);
					//}
					if (strncasecmp("codeliststart", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 13)
					{
						sscanf((char *)(tempgameconf + i), " = %x", (unsigned int *)&codelist);
					}
					if (strncasecmp("codelistend", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						sscanf((char *)(tempgameconf + i), " = %x", (unsigned int *)&codelistend);
					}
					/*
					if (strncasecmp("hooktype", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
					{
						if (hookset == 1)
						{
							ret = sscanf(tempgameconf + i, " = %u", &temp);
							if (ret == 1)
								if (temp >= 0 && temp <= 7)
									config_bytes[2] = temp;
						}
					}
					*/
					if (strncasecmp("poke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 4)
					{
						ret = sscanf((char *)tempgameconf + i, "( %x , %x", &codeaddr, &codeval);
						if (ret == 2)
						{
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 8;
							*(gameconf + (gameconfsize / 4)) = codeaddr;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval;
							gameconfsize += 4;
							DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
							ICInvalidateRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
						}
					}
					if (strncasecmp("pokeifequal", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						ret = sscanf((char *)(tempgameconf + i), "( %x , %x , %x , %x", &codeaddr, &codeval, &codeaddr2, &codeval2);
						if (ret == 4)
						{
							*(gameconf + (gameconfsize / 4)) = 0;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeaddr;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeaddr2;
							gameconfsize += 4;
							*(gameconf + (gameconfsize / 4)) = codeval2;
							gameconfsize += 4;
							DCFlushRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
							ICInvalidateRange((void *) (gameconf + (gameconfsize / 4) - 5), 20);
						}
					}
					if (strncasecmp("searchandpoke", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 13)
					{
						ret = sscanf((char *)(tempgameconf + i), "( %x%n", &codeval, &tempoffset);
						if (ret == 1)
						{
							gameconfsize += 4;
							temp = 0;
							while (ret == 1)
							{
								*(gameconf + (gameconfsize / 4)) = codeval;
								gameconfsize += 4;
								temp++;
								i += tempoffset;
								ret = sscanf((char *)(tempgameconf + i), " %x%n", &codeval, &tempoffset);
							}
							*(gameconf + (gameconfsize / 4) - temp - 1) = temp;
							ret = sscanf((char *)(tempgameconf + i), " , %x , %x , %x , %x", &codeaddr, &codeaddr2, &codeoffset, &codeval2);
							if (ret == 4)
							{
								*(gameconf + (gameconfsize / 4)) = codeaddr;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeaddr2;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeoffset;
								gameconfsize += 4;
								*(gameconf + (gameconfsize / 4)) = codeval2;
								gameconfsize += 4;
								DCFlushRange((void *) (gameconf + (gameconfsize / 4) - temp - 5), temp * 4 + 20);
								ICInvalidateRange((void *) (gameconf + (gameconfsize / 4) - temp - 5), temp * 4 + 20);
							}
							else
								gameconfsize -= temp * 4 + 4;
						}
						
					}
					/*
					if (strncasecmp("hook", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 4)
					{
						ret = sscanf(tempgameconf + i, "( %x %x %x %x %x %x %x %x", customhook, customhook + 1, customhook + 2, customhook + 3, customhook + 4, customhook + 5, customhook + 6, customhook + 7);
						if (ret >= 3)
						{
							if (hookset != 1)
								configwarn |= 4;
							config_bytes[2] = 0x08;
							customhooksize = ret * 4;
						}
					}
					if (strncasecmp("002fix", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 6)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 0x1)
								fakeiosversion = temp;
					}
					if (strncasecmp("switchios", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 9)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								willswitchios = temp;
					}
					if (strncasecmp("videomode", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 9)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
						{
							if (temp == 0)
							{
								if (config_bytes[1] != 0x00)
									configwarn |= 1;
								config_bytes[1] = 0x00;
							}
							else if (temp == 1)
							{
								if (config_bytes[1] != 0x03)
									configwarn |= 1;
								config_bytes[1] = 0x03;
							}
							else if (temp == 2)
							{
								if (config_bytes[1] != 0x01)
									configwarn |= 1;
								config_bytes[1] = 0x01;
							}
							else if (temp == 3)
							{
								if (config_bytes[1] != 0x02)
									configwarn |= 1;
								config_bytes[1] = 0x02;
							}
						}
					}
					if (strncasecmp("language", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
						{
							if (temp == 0)
							{
								if (config_bytes[0] != 0xCD)
									configwarn |= 2;
								config_bytes[0] = 0xCD;
							}
							else if (temp > 0 && temp <= 10)
							{
								if (config_bytes[0] != temp-1)
									configwarn |= 2;
								config_bytes[0] = temp-1;
							}
						}
					}
					if (strncasecmp("diagnostic", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 10)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
						{
							if (temp == 0 || temp == 1)
								diagcreate = temp;
						}
					}
					if (strncasecmp("vidtv", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 5)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								vipatchon = temp;
					}
					if (strncasecmp("fwritepatch", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								applyfwritepatch = temp;
					}
					if (strncasecmp("dumpmaindol", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								dumpmaindol = temp;
					}
					*/
				}
				/*else
				{
					
					if (strncasecmp("autoboot", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 8)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								autoboot = temp;
					}
					if (strncasecmp("autobootwait", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 12)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 255)
								autobootwait = temp;
					}
					if (strncasecmp("autoboothbc", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 11)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								autoboothbc = temp;
					}
					if (strncasecmp("autobootocarina", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 15)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								config_bytes[4] = temp;
					}
					if (strncasecmp("autobootdebugger", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 16)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								config_bytes[7] = temp;
					}
					if (strncasecmp("rebootermenuitem", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 16)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 1)
								rebooterasmenuitem = temp;
					}
					if (strncasecmp("startupios", parsebuffer, strlen(parsebuffer)) == 0 && strlen(parsebuffer) == 10)
					{
						ret = sscanf(tempgameconf + i, " = %u", &temp);
						if (ret == 1)
							if (temp >= 0 && temp <= 255)
							{
								sdio_Shutdown();
								IOS_ReloadIOS(temp);
								detectIOScapabilities();
								sd_init();
								startupiosloaded = 1;
							}
					}
					
				}*/
				if (tempgameconf[i] != ':')
				{
					while ((i != tempgameconfsize) && (tempgameconf[i] != 10 && tempgameconf[i] != 13)) i++;
					if (i != tempgameconfsize) i++;
				}
			}
			if (i != tempgameconfsize) while ((tempgameconf[i] != 10 && tempgameconf[i] != 13) && (i != 0)) i--;
		}
	}
	
	free(tempgameconf);
	//tempcodelist = ((u8 *) gameconf) + gameconfsize;
}

//---------------------------------------------------------------------------------
void app_pokevalues()
//---------------------------------------------------------------------------------
{
	u32 i, *codeaddr, *codeaddr2, *addrfound = NULL;
	
	if (gameconfsize != 0)
	{
		for (i = 0; i < gameconfsize/4; i++)
		{
			if (*(gameconf + i) == 0)
			{
				if (((u32 *) (*(gameconf + i + 1))) == NULL ||
					*((u32 *) (*(gameconf + i + 1))) == *(gameconf + i + 2))
				{
					*((u32 *) (*(gameconf + i + 3))) = *(gameconf + i + 4);
					DCFlushRange((void *) *(gameconf + i + 3), 4);
					ICInvalidateRange((void *) *(gameconf + i + 3), 4);
				}
				i += 4;
			}
			else
			{
				codeaddr = (u32 *)*(gameconf + i + *(gameconf + i) + 1);
				codeaddr2 = (u32 *)*(gameconf + i + *(gameconf + i) + 2);
				if (codeaddr == 0 && addrfound != NULL)
					codeaddr = addrfound;
				else if (codeaddr == 0 && codeaddr2 != 0)
					codeaddr = (u32 *) ((((u32) codeaddr2) >> 28) << 28);
				else if (codeaddr == 0 && codeaddr2 == 0)
				{
					i += *(gameconf + i) + 4;
					continue;
				}
				if (codeaddr2 == 0)
					codeaddr2 = codeaddr + *(gameconf + i);
				addrfound = NULL;
				while (codeaddr <= (codeaddr2 - *(gameconf + i)))
				{
					if (memcmp(codeaddr, gameconf + i + 1, (*(gameconf + i)) * 4) == 0)
					{
						*(codeaddr + ((*(gameconf + i + *(gameconf + i) + 3)) / 4)) = *(gameconf + i + *(gameconf + i) + 4);
						if (addrfound == NULL) addrfound = codeaddr;
					}
					codeaddr++;
				}
				i += *(gameconf + i) + 4;
			}
		}
	}
}

void do_codes(bool storageavailable)
{
	gameconfsize = 0;
	memset((void *)0x80001800, 0, 0x1800);
	
	s32 ret;
	char gameidbuffer[8];
	memset(gameidbuffer, 0, 8);
	
	memcpy(gameidbuffer, (char*)0x80000000, 6);

	if (wiidebuggerselect == 0)
		codelist = (u8 *) 0x800022A8;
	else
		codelist = (u8 *) 0x800028B8;
	codelistend = (u8 *) 0x80003000;

	gameconf = malloc(65536);
	if (gameconf == NULL)
	{
		print_status("Out of memory");
		wait(3);
		return;
	}

	if (storageavailable)
	{
		app_loadgameconfig();
	}
	
	if (wiiocarinaselect != 0 && storageavailable)
	{
		print_status("Searching codes...");
		wait(1);
		
		load_handler();
		
		ret = load_codes(gameidbuffer, (u32)codelistend - (u32)codelist, codelist);
		if (ret >= 0)
		{
			//printf("Codes found. Applying\n");
			print_status("Codes found. Applying");
		} else
		{
			//print_status is already in load_codes
		}
		wait(2);
	} else
	if (wiidebuggerselect != 0)
	{
		load_handler();
		print_status("Code handler loaded...");
		wait(2);		
	}
	
	DCFlushRange(codelist, (u32)codelistend - (u32)codelist);
	ICInvalidateRange(codelist, (u32)codelistend - (u32)codelist);
	DCFlushRange((void*)0x80000000, 0x3f00);
	ICInvalidateRange((void*)0x80000000, 0x3f00);
}

