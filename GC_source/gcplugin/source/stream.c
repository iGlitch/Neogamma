/* Copyright 2009 WiiGator. */
#include "types.h"
#include "usb.h"
#include "debugprintf.h"
#include "exception.h"
#include "strnlen.h"
#include "snprintf.h"
#include "plugin.h"
#include "stream.h"
#include "cache.h"

#ifdef GEKKO_DEBUG

/*int stream_write_replacement1(
	void *stream,
	const char *text,
	unsigned long size)
*/
int stream_write_replacement1(const void *ptr, u32 size, u32 count, void *stream)
{
	//jmpNr = 0;

	exception_init();

	debug_printf("%s", (char *)ptr);

	return stream_write_orig1(ptr, size, count, stream);
}

#ifdef RELOAD_SUPPORT
int stream_write_replacement2(const void *ptr, u32 size, u32 count, void *stream)
{
	//jmpNr = 1;

	exception_init();

	debug_printf("%s", (char *)ptr);

	return stream_write_orig2(ptr, size, count, stream);
}
#endif

#ifndef MIOS_PLUGIN /* AR is can only use game plugin. */
#ifdef ACTION_REPLAY
/*
unsigned long ar_stream_write_replacement1(
	char **buffer,
	const char *format,
	void *args)
{
	static char usbgecko_connected;
	static char usbgecko_checked;
	const char *dst;
	unsigned long len;

	exception_init();

	dst = *buffer;

	if (!usbgecko_checked) {
		usbgecko_connected = usb_checkgecko();
		usbgecko_checked = 1;
	}
	len = ar_stream_write_orig1(buffer, format, args);
	if (usbgecko_connected) {

		usb_sendbuffersafe(dst, len);
	}
	return len;
}
*/
#endif
#endif

#ifdef GEKKO_DEBUG
/** Replace printf function in MIOS. */
int printf_replacement1(const char *format, ...)
{
/* MIOS patch extraction
	u32 patchoffset = 0x803221d8;
	debug_printf("Memory setup:\n");
	u32 i;
	for (i=0;i < 16;i++)
	{
		debug_printf("0x%08x, 0x%08x, 0x%08x, 0x%08x,\n", *(u32 *)(patchoffset+i*16), *(u32 *)(patchoffset+i*16+4), *(u32 *)(patchoffset+i*16+8), *(u32 *)(patchoffset+i*16+12));
	}
	debug_printf("\n");
*/
	char buf[128];
	int len;
	va_list ap;
	static char usbgecko_connected;
	static char usbgecko_checked;

	exception_init();

	/* XXX: Installation of exception handlers is too early for MIOS. */
	// exception_init();
#ifdef RELOAD_SUPPORT
	//jmpNr = 0;
#endif

	//debug_printf("printf_replacement called by 0x%x\n", __builtin_return_address(0));

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	len = strnlen(buf, sizeof(buf));
	
	if (!usbgecko_checked) {
		usbgecko_connected = usb_checkgecko();
		usbgecko_checked = 1;
		if (usbgecko_connected)
		{
			usb_flush();
		}
	}
	if (usbgecko_connected) {
		usb_sendbuffersafe(buf, len);
	}

	/* success: */
	return len;
}
#endif

#endif

