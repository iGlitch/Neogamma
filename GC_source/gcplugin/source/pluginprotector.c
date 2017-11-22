/* Protect game plugin against evil memory clear hacks. */
#include "plugin.h"
#include "pluginprotector.h"
#include "debugprintf.h"

#ifdef HIGH_PLUGIN
#define GAME_PLUGIN_BASE HIGH_PLUGIN_BASE
#define GAME_PLUGIN_SIZE HIGH_PLUGIN_SIZE

//#define GAME_PLUGIN_BASE ((void*) 0x80001800)
//#define GAME_PLUGIN_SIZE 0x1800
#endif
#ifdef LOW_PLUGIN
#define GAME_PLUGIN_BASE LOW_PLUGIN_BASE
#define GAME_PLUGIN_SIZE LOW_PLUGIN_SIZE
#endif

void memset_replacement1(void *addr, int c, unsigned long size)
{
	unsigned long val;

#ifdef RELOAD_SUPPORT
	jmpNr = 0;
#endif

#ifdef SHOW_MEMSETS
	debug_printf("memset1(%x, %x, %x) caller %x\n", addr, c, size, __builtin_return_address(0));
#endif

	val = (unsigned long) addr;
	// Check if plugin would be overwritten.
	if (!((val >= ((unsigned long) GAME_PLUGIN_BASE))
		&& (val < (((unsigned long) GAME_PLUGIN_BASE) + GAME_PLUGIN_SIZE))))
	{
		memset_orig1(addr, c, size);
	} else
	{
		debug_printf("memset1(%x, %x, %x) skipped caller %x\n", addr, c, size, __builtin_return_address(0));
	}
}

#ifdef RELOAD_SUPPORT
void memset_replacement2(void *addr, int c, unsigned long size)
{
	unsigned long val;

	jmpNr = 1;

#ifdef SHOW_MEMSETS
	debug_printf("memset2(%x, %x, %x) caller %x\n", addr, c, size, __builtin_return_address(0));
#endif

	val = (unsigned long) addr;
	// Check if plugin would be overwritten.
	if (!((val >= ((unsigned long) GAME_PLUGIN_BASE))
		&& (val < (((unsigned long) GAME_PLUGIN_BASE) + GAME_PLUGIN_SIZE))))
	{
		memset_orig2(addr, c, size);
	} else
	{
		debug_printf("memset2(%x, %x, %x) skipped caller %x\n", addr, c, size, __builtin_return_address(0));
	}
}
#endif

