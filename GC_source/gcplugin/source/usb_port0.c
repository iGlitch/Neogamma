
/*---------------------------------------------------------------------------------------------
 * USB Gecko Development Kit - http://www.usbgecko.com
 * --------------------------------------------------------------------------------------------
 * 
 *
 * usb.c - V1.2 functions for the USB Gecko adapter (www.usbgecko.com).
 * Now works for Wii Mode - use WIIMODE define in usb.h to set
 * Copyright (c) 2008 - Nuke - <wiinuke@gmail.com>
 * 
 *---------------------------------------------------------------------------------------------*/
	 
#include "types.h"				// If not using libogc need types
#include "usb.h"
	

/*---------------------------------------------------------------------------------------------*
    Name:           usb_sendbyte
    Description:	Send byte to Gamecube/Wii over EXI memory card port
*----------------------------------------------------------------------------------------------*/ 
static int __usb_sendbyte(char sendbyte) 
{
	int32_t i;
	exi_chan0sr = 0x000000D0;
	exi_chan0data = 0xB0000000 | (sendbyte << 20);
	exi_chan0cr = 0x19;
	while ((exi_chan0cr) & 1);
	i = exi_chan0data;
	exi_chan0sr = 0;
	if (i & 0x04000000) {
		return 1;
	}
	return 0;
}



#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version have enough memory. */
/*---------------------------------------------------------------------------------------------*
    Name:           usb_receivebyte
    Description:	Receive byte from Gamecube/Wii over EXI memory card port
*----------------------------------------------------------------------------------------------*/ 
static int __usb_receivebyte(char *receivebyte) 
{
	int32_t i = 0;
	exi_chan0sr = 0x000000D0;
	exi_chan0data = 0xA0000000;
	exi_chan0cr = 0x19;
	while ((exi_chan0cr) & 1);
	i = exi_chan0data;
	exi_chan0sr = 0;
	if (i & 0x08000000) {
		*receivebyte = (i >> 16) & 0xff;
		return 1;
	}
	return 0;
}
#endif



/*---------------------------------------------------------------------------------------------*
    Name:           usb_checksendstatus
    Description:	Chesk the FIFO is ready to send
*----------------------------------------------------------------------------------------------*/ 
static int __usb_checksendstatus() 
{
	int32_t i = 0;
	exi_chan0sr = 0x000000D0;
	exi_chan0data = 0xC0000000;
	exi_chan0cr = 0x19;
	while ((exi_chan0cr) & 1);
	i = exi_chan0data;
	exi_chan0sr = 0x0;
	if (i & 0x04000000) {
		return 1;
	}
	return 0;
}


#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version have enough memory. */

/*---------------------------------------------------------------------------------------------*
    Name:           usb_checkreceivestatus
    Description:	Check the FIFO is ready to receive
*----------------------------------------------------------------------------------------------*/ 
/*
static int __usb_checkreceivestatus() 
{
	int32_t i = 0;
	exi_chan0sr = 0x000000D0;
	exi_chan0data = 0xD0000000;
	exi_chan0cr = 0x19;
	while ((exi_chan0cr) & 1);
	i = exi_chan0data;
	exi_chan0sr = 0x0;
	if (i & 0x04000000) {
		return 1;
	}
	return 0;
}*/


#endif	/*  */
	

#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version have enough memory. */
/*---------------------------------------------------------------------------------------------*
    Name:           usb_sendbuffer
    Description:	Simple buffer send routine
*----------------------------------------------------------------------------------------------*/ 
/*
void usb_sendbuffer(const void *buffer, int size) 
{
	char *sendbyte = (char *) buffer;

	int32_t bytesleft = size;
	int32_t returnvalue;
	while (bytesleft > 0)
		 {
		returnvalue = __usb_sendbyte(*sendbyte);
		if (returnvalue) {
			sendbyte++;
			bytesleft--;
		}
		}
}*/


/*---------------------------------------------------------------------------------------------*
    Name:           usb_receivebuffer
    Description:	Simple buffer receive routine
*----------------------------------------------------------------------------------------------*/ 
/*
void usb_receivebuffer(void *buffer, int size) 
{
	char *receivebyte = (char *) buffer;

	int32_t bytesleft = size;
	int32_t returnvalue;
	while (bytesleft > 0)
		 {
		returnvalue = __usb_receivebyte(receivebyte);
		if (returnvalue) {
			receivebyte++;
			bytesleft--;
		}
		}
}
*/

#endif	/*  */
	

/*---------------------------------------------------------------------------------------------*
    Name:           usb_sendbuffersafe
    Description:	Simple buffer send routine with fifo check (use for large transfers)
*----------------------------------------------------------------------------------------------*/ 
void usb_sendbuffersafe(const void *buffer, int size) 
{
	char *sendbyte = (char *) buffer;

	int32_t bytesleft = size;
	int32_t returnvalue;
	while (bytesleft > 0)
		 {
		if (__usb_checksendstatus()) {
			returnvalue = __usb_sendbyte(*sendbyte);
			if (returnvalue) {
				sendbyte++;
				bytesleft--;
			}
		}
		}
}


#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version have enough memory. */

/*---------------------------------------------------------------------------------------------*
    Name:           usb_receivebuffersafe
    Description:	Simple buffer receive routine with fifo check (use for large transfers)
*----------------------------------------------------------------------------------------------*/ 
/*
void usb_receivebuffersafe(void *buffer, int size) 
{
	char *receivebyte = (char *) buffer;

	int32_t bytesleft = size;
	int32_t returnvalue;
	while (bytesleft > 0)
		 {
		if (__usb_checkreceivestatus()) {
			returnvalue = __usb_receivebyte(receivebyte);
			if (returnvalue) {
				receivebyte++;
				bytesleft--;
			}
		}
		}
}
*/


#endif	/*  */
	

/*---------------------------------------------------------------------------------------------*
    Name:           usb_checkgecko
    Description:	Chesk the Gecko is connected
*----------------------------------------------------------------------------------------------*/ 
int usb_checkgecko() 
{
	int32_t i = 0;
	exi_chan0sr = 0x000000D0;
	exi_chan0data = 0x90000000;
	exi_chan0cr = 0x19;
	while ((exi_chan0cr) & 1);
	i = exi_chan0data;
	exi_chan0sr = 0x0;
	if (i == 0x04700000) {
		return 1;
	}
	return 0;
}



#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version have enough memory. */
/*---------------------------------------------------------------------------------------------*
    Name:           usb_flush
    Description:	Flushes the FIFO, Use at the start of your program to avoid trash
*----------------------------------------------------------------------------------------------*/ 
void usb_flush() 
{
	char tempbyte;

	while (__usb_receivebyte(&tempbyte));
}
#endif


