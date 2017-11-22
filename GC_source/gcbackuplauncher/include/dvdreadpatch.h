#ifndef _DVDREADPATCH_H_
#define _DVDREADPATCH_H_

#include <gctypes.h>
#include "gcplugin.h"

extern unsigned long second_disk_selected;
extern unsigned long second_disc_offset;

void dvd_patchread(void *addr, u32 len, gcplugin_jumptable_debug_t *jmp, bool miosplugin, bool dopatch, bool highplugin, bool debug_plugin);
#endif


