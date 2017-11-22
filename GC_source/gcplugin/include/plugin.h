#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "types.h"
#include "gcplugin.h"

#ifdef GEKKO_DEBUG
extern gcplugin_jumptable_debug_t jumptable[];
extern gcplugin_jumptable_debug_t jumptable2;
#else
#ifdef LOW_PLUGIN
extern gcplugin_jumptable_low_t jumptable[];
extern gcplugin_jumptable_low_t jumptable2;
#endif
#ifdef HIGH_PLUGIN
extern gcplugin_jumptable_high_t jumptable[];
extern gcplugin_jumptable_high_t jumptable2;
#endif
#endif

/** Base offset for read (set by gcbackuplauncher). */
extern unsigned long base_offset;
extern unsigned long base_offset_2nd_disc;

#ifdef RELOAD_SUPPORT
extern u8 reloader_enabled;
//extern u8 dummy1;
//extern u8 dummy2;
extern u8 jmpNr;
extern u8 reloader_status;
#endif

extern u8 audio_stream_fix;
extern u8 mios_mode;

#ifdef GEKKO_DEBUG
extern u8 backuplaunching;
#endif

#ifdef DONTTELL
extern u8 hooks;
extern u32 hook[16];
#endif

#ifdef GEKKO_DEBUG
void printDVDRegs(void);
void hexDump(unsigned long *buffer, unsigned long size);
#endif

extern u32 elf_base_offset;
extern u32 elf_offset;

#endif
