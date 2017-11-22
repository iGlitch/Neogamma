#ifndef __PATCHCODE_H__
#define __PATCHCODE_H__

//---------------------------------------------------------------------------------
void copy_2nd_hook(void *hookoffset);

bool dogamehooks(void *addr, u32 len);

#endif
