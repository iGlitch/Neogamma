#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#ifdef GEKKO_DEBUG
void exception_init(void);
#else
#define exception_init() do { } while(0)
#endif

#endif
