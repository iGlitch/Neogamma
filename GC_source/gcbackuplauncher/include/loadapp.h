#ifndef _LOADAPP_H_
#define _LOADAPP_H_

#define APPLOADER_ADDRESS 0x81200000
#define APPLOADER_HEADER_OFFSET 0x2440
#define APPLOADER_OFFSET 0x2460
#define GAMECODE_ADDRESS 0x80000000

/** Start address of MIOS v5. */
#define MIOS_BASE ((void *) 0x812fffe0)
#define MIOS_SIZE 0x00032000	// Estimate 200 KB as max MIOS size
//#define MIOS_SIZE 0x0002659c
/** Entry point for MIOS v5. */
#define MIOS_ENTRY 0x81300200
/** Entry point variable. */
#define MIOS_PATCH_ADDR ((void *) 0x800037fcUL)
/** Entry point for GC HomeBrew Launcher code (patched into MIOS). */
#define GC_HOMEBREW_LAUNCHER_ENTRY 0x81000000UL

void load_app();
u32 appentrypoint;

extern u32 hookselect;
extern u32 hook2select;
extern u32 ocarinaselect;
extern u32 debuggerselect;

extern u32 patched_read;
extern bool patched_readMIOS;
extern bool patched_seek;
extern bool patched_readID;
extern bool patched_readIDMIOS;
extern bool patched_readaudio;
extern bool patched_audioconfig;
extern bool patched_audiostatus;

// Only for debugging:
extern bool patched_reporterror;
extern bool patched_reset;
extern bool patched_stop_motor;
extern bool patched_stream_write;

extern u32 streaming;
extern u8 reloader;
extern u32 videopatchmode;

bool use_high_plugin;
bool use_debug_plugin;
bool use_patched_MIOS;
bool dopatch__GXSetVAT;
bool audio_status_request_fix;
bool audio_32K_fix;

void *codelist;
void *codelistend;

#endif
