#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <stdarg.h>

#include "debugprintf.h"
#include "loadapp.h"

void debug_printf(const char *fmt, ...)
{
//#ifdef GEKKO_DEBUG
	if (use_debug_plugin == false)
	{
		return;
	}
	static u32 GEKKO_CHANNEL = 1;

	char buf[256];
	int len;
	va_list ap;
	static char usbgecko_connected;
	static char usbgecko_checked;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (!usbgecko_checked)
	{
		usbgecko_connected = usb_isgeckoalive(1);
		if (usbgecko_connected)
		{
			GEKKO_CHANNEL = 1;
		} else
		{
			usbgecko_connected = usb_isgeckoalive(0);
			GEKKO_CHANNEL = 0;
		}
		
		usbgecko_checked = 1;
		if (usbgecko_connected)
		{
			usb_flush(GEKKO_CHANNEL);
		}
	}
	if (usbgecko_connected)
	{
		len = strnlen(buf, sizeof(buf));
		usb_sendbuffer(GEKKO_CHANNEL, buf, len);
	}
	
//#endif
}
