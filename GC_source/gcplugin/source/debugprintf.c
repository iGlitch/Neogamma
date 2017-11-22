#include "snprintf.h"
#include "usb.h"
#include "strnlen.h"

#ifdef GEKKO_DEBUG

void debug_printf(const char *fmt, ...) {
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
		usbgecko_connected = usb_checkgecko();
		usbgecko_checked = 1;
		if (usbgecko_connected)
		{
			usb_flush();
		}
	}
	if (usbgecko_connected)
	{
		len = strnlen(buf, sizeof(buf));
		usb_sendbuffersafe(buf, len);
	}
}

#endif
