#ifndef _DVDREADPATCH_H_
#define _DVDREADPATCH_H_

#include "types.h"
#include "plugin.h"

#ifdef GEKKO_DEBUG
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_debug_t *jmp, u32 dvd_offset);
#else
#ifdef LOW_PLUGIN
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_low_t *jmp, u32 dvd_offset);
#endif
#ifdef HIGH_PLUGIN
void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_high_t *jmp, u32 dvd_offset);
#endif
#endif

#endif