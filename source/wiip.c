#include <gccore.h>
#include <string.h>
#include <stdlib.h>

u32 doltableoffset[64];
u32 doltablelength[64];
u32 doltableentries;

void wipreset()
{
	doltableentries = 0;
}

void wipregisteroffset(u32 dst, u32 len)
{
	doltableoffset[doltableentries] = dst;
	doltablelength[doltableentries] = len;
	doltableentries++;
}

bool patchu8(u32 offset, u8 newvalue, u8 oldvalue)
{
	u32 i = 0;
	u32 tempoffset = 0;
	bool tempresult = true;
	
	while ((doltablelength[i] <= offset-tempoffset) && (i+1 < doltableentries))
	{
		tempoffset+=doltablelength[i];
		i++;
	}
	if (offset-tempoffset < doltablelength[i])
	{
		if (*(u8 *)(offset-tempoffset+doltableoffset[i]) != oldvalue)
		{
			tempresult = false;
		}
		*(u8 *)(offset-tempoffset+doltableoffset[i]) = newvalue;
	}
	return tempresult;
}

bool wipparsebuffer(u8 *buffer, u32 length)
// The buffer needs a 0 at the end to properly terminate the string functions
{
	u32 pos = 0;
	u32 pos2 = 0;
	u32 offset;
	char buf[10];
	char buf2[4];
	bool tempresult = true;
	
	while (pos < length)
	{
		if ( *(char *)(buffer + pos) != '#' && *(char *)(buffer + pos) != ';' && *(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 32 && *(char *)(buffer + pos) != 0 )
		{
			memcpy(buf, (char *)(buffer + pos), 8);
			buf[8] = 0;
			offset = strtol(buf,NULL,16);

			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			pos2 = pos;
			pos += (u32)strchr((char *)(buffer + pos), 32)-(u32)(buffer + pos) + 1;
			
			while (pos < length && *(char *)(buffer + pos) != 10 && *(char *)(buffer + pos) != 13 && *(char *)(buffer + pos) != 0)
			{
				memcpy(buf, (char *)(buffer + pos), 2);
				buf[2] = 0;
				memcpy(buf2, (char *)(buffer + pos2), 2);
				buf2[2] = 0;
			
				if (!patchu8(offset, strtol(buf,NULL,16), strtol(buf2,NULL,16)))
				{
					tempresult = false;
				}
				offset++;
				pos +=2;
				pos2 +=2;
			}	
		}
		if (strchr((char *)(buffer + pos), 10) == NULL)
		{
			return tempresult;
		} else
		{
			pos += (u32)strchr((char *)(buffer + pos), 10)-(u32)(buffer + pos) + 1;
		}
	}
	return tempresult;
}
