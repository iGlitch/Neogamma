#include <ogcsys.h>

u32 gameselected;
u32 wbfsdevice;


/* Disc header structure */
struct discHdr
{
	/* Game ID */
	u8 id[6];

	/* Game version */
	u16 version;

	/* Audio streaming */
	u8 streaming;
	u8 bufsize;

	/* Padding */
	u8 unused1[14];

	/* Magic word */
	u32 magic;

	/* Padding */
	u8 unused2[4];

	/* Game title */
	char title[64];

	/* Encryption/Hashing */
	u8 encryption;
	u8 h3_verify;

	/* Padding */
	u8 unused3[30];
} ATTRIBUTE_PACKED;


s32 Try_WBFS_Init();
s32 initGameList();
struct discHdr *getGameHeader(u32 index);
char *getGameTitle(u32 index);
s32 getGameCount();
void change_selected_wbfs_device(u32 device);
s32 get_DML_game_list();
s32 Sneek_build_sorted_game_list();
s32 DML_select_game();



