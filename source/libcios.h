/*-------------------------------------------------------------

libcios.h -- Custom IOS library

Copyright (C) 2008 (waninkoko)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#ifndef _LIBCIOS_H_
#define _LIBCIOS_H_

#include <ogcsys.h>

/* Prototypes */
s32 Load_IOS(u32 version);
s32 CIOS_Identify(void);
s32 CIOS_IdentifyAsTitle(u64);
s32 __CIOS_GetCerts(signed_blob **out, u32 *len);
s32 GetTMD(u64 TicketID, signed_blob **Output, u32 *Length);
void block_ios_reload(bool enable);
void tell_cIOS_to_return_to_channel();
void set_cIOS_stealth_mode(bool enable);

#endif
