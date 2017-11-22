
u32 CIOS_VERSION;

u8 langselect;
u8 viselect;
u8 recoveryselect;
u8 regionfreeselect;
u8 nocopyselect;
u8 buttonskipselect;
u8 countrystringselect;
u8 videoselect;
u8 alternativedolselect;
u8 gamecubemodeselect;
u8 patchselect;
u8 storageselect;
u8 skipupdateselect;
u8 bendiosreloadselect;
u8 preloaderselect;

u8 wiihookselect;
u8 wiiocarinaselect;
u8 wiidebuggerselect;
u8 gchookselect;
u8 gcocarinaselect;
u8 gcdebuggerselect;

void *gccodelist;		// Not saved!
void *gccodelistend;	// Not saved!
u32 maxgccodesize; 		// Not saved!

u8 gc_videoselect;
u8 gc_videopatchselect;

u8 videopatchselect;

u8 configselect;

u32 drivedate;	// Read from the drive itself
u64 old_title_id; // Read at startup

u8 gcreloaderselect;

u8 gchighplugin;

u8 gc2ndhookselect;
u8 blockiosreloadselect;
u8 showrebooterselect;
u8 wii2ndhookselect;
u8 usecorrectiosoption;
u8 unusedoption7;

u8 audiostatusrequestfixselect;
u8 patchedMIOSselect;

void Load_Config();
void Save_Config();


u32 Sneek_GameCount __attribute__((aligned(32)));
extern u32 DML_GameCount;

bool Sneek_Mode;

typedef struct
{
        u32             SlotID;
        u32             Region;
        u32             Gamecount;
        u32             Config;
        u8              GameInfo[][0x80];
} Sneek_DIConfig;

Sneek_DIConfig *Sneek_DICfg __attribute__((aligned(32)));

typedef struct 
{
	u32 index;
	char title[0x40];
} Sneek_game_list_entry;

Sneek_game_list_entry *Sneek_game_list;

typedef struct 
{
	char discid[6];
	char title[0x40];
} DML_game_list_entry;

DML_game_list_entry *DML_game_list;

