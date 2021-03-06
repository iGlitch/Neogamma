#ifndef _WRITE_BRANCH_H_
#define _WRITE_BRANCH_H_

#include "types.h"

void writebranch(void *sourceAddr,void *destAddr);
//void writebranch_with_link(void *sourceAddr,void *destAddr);
void install_replacement(u32 *replacedFn, u32 *replacementFn, u32 *keptFn);
#ifdef ACTION_REPLAY
void writeabsolutebranch(void *sourceAddr,void *destAddr);
#endif
uint32_t *get_bl_target(uint32_t *src);

#endif
