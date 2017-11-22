#ifndef _CACHE_H_
#define _CACHE_H_

#include "types.h"

//#ifdef HIGH_PLUGIN /* Only high version has enough memory. */
void DCInvalidateRange(void *startaddress,u32 len);
//#endif
void DCFlushRange_and_ICInvalidateRange(void *startaddress,u32 len);
//void ICInvalidateRange(void *startaddress,u32 len);


#endif
