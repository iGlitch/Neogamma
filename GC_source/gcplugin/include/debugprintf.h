#ifndef _DEBUG_PRINTF_H_
#define _DEBUG_PRINTF_H_

#ifdef GEKKO_DEBUG
void debug_printf(const char *fmt, ...);
#else
#define debug_printf(x...) do { } while(0)
#endif

#endif
