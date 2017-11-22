#ifndef _PLUGIN_PROTECTOR_H_
#define _PLUGIN_PROTECTOR_H_

void memset_replacement1(void *addr, int c, unsigned long size);
void memset_orig1(void *addr, int c, unsigned long size);

#ifdef RELOAD_SUPPORT
void memset_replacement2(void *addr, int c, unsigned long size);
void memset_orig2(void *addr, int c, unsigned long size);
#endif

#endif
