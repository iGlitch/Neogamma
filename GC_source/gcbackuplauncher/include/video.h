#ifndef _VIDEO_H_
#define _VIDEO_H_

#define VIDEO_MODE_NTSC 0
#define VIDEO_MODE_PAL 1
#define VIDEO_MODE_PAL60 2
#define VIDEO_MODE_NTSC480P 3
#define VIDEO_MODE_PAL480P 4


void set_game_videomode(int mode);

//extern uint32_t GXPal528IntDf[15];
//extern uint32_t GXMpal480IntDf[15];
//extern uint32_t GXNtsc480Int[15];
//extern uint32_t GXNtsc480IntDf[15];
//extern uint32_t GXEurgb60Hz480IntDf[15];

#endif
