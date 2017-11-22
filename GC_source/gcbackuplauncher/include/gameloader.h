#ifndef _GAME_LOADER_H_
#define _GAME_LOADER_H_

#include <stdint.h>
#include <gctypes.h>

typedef struct {
	u8 ConsoleID;				//G = Gamecube, R = Wii.
	u8 GamecodeA;				//2 Ascii letters to indicate the GameID.
	u8 GamecodeB;				//2 Ascii letters to indicate the GameID.
	u8 CountryCode;				//J=JAP . P=PAL . E=USA . D=OoT MasterQuest
	u8 MakerCodeA;				//Eg 08 = Sega etc.
	u8 MakerCodeB;
	u8 DiscID;
	u8 Version;
	u8 AudioStreaming;			//01 = Enable it. 00 = Don't
	u8 StreamBufSize;			//For the AudioEnable. (always 00?)
	u8 unused_1[18];
	u32 DVDMagicWord;			//0xC2339F3D
	char GameName[992];			//String Name of Game, rarely > 32 chars
	u32 DMonitorOffset;			//offset of debug monitor (dh.bin)?
	u32 DMonitorLoadAd;			//addr(?) to load debug monitor?
	u8 unused_2[24];
	u32 DOLOffset;				//offset of main executable DOL (bootfile)
	u32 FSTOffset;				//offset of the FST ("fst.bin")
	u32 FSTSize;				//size of FST
	u32 MaxFSTSize;				//maximum size of FST (usually same as FSTSize)*
	u32 UserPos;				//user position(?)
	u32 UserLength;				//user length(?)
	u32 unknown;				//(?)
	u32 unused_3;
} dvdinfo_t;

void start_from_NeoGamma(void);

#endif
