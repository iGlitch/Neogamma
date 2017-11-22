/* Code from klibc. */

/*
 * memcpy.c
 */

#include "types.h"
#include "memcpy.h"

void *memcpy(void *dst, const void *src, size_t n)
{
	const char *p = src;
	char *q = dst;

	/** Code can be optimized for PPC. */
	while (n--) {
		*q++ = *p++;
	}

	return dst;
}
