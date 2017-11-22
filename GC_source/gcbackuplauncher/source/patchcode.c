#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <string.h>
//#include <sys/unistd.h>

#include "patchcode.h"
#include "codecmp.h"
#include "loadapp.h"

extern void patchhook(u32 address, u32 len);

// Hook found in freedom source
const u32 vigchooks[4] = {
	0x906402E4, 0x38000000, 0x900402E0, 0x909E0004
};

// Hook found by Nicksasa
const u32 GC_gxdrawdone_hooks[8] = {
	0x28000000, 0x4182FFF0, 0x7FE3FB78, 0x4BFE7CE5,
	0x8001001C, 0x83E10014, 0x38210018, 0x7C0803A6
};

// Hook found by Nicksasa
const u32 GC_OSSleepthread_hooks[6] = {
	0x7FE3FB78, 0x4BFFC449, 0x8001001C, 0x83E10014,
	0x83C10010, 0x38210018
};

// Hooks received from Crediar
const u32 hook0[11] = {
	0x38000000, 0x9815000A, 0xA0150000, 0x5400066E,	0xB0150000, 0x3AF70001, 0x2C170004, 0x3B7B0004, 0x3B5A0002, 0x3B39000C, 0x3AB5000C
};

const u32 hook1[11] = {
	0x38000000, 0x981F000A, 0xA01F0000, 0x5400066E,	0xB01F0000, 0x3AB50001, 0x2C150004, 0x3B18000C,	0x3BFF000C, 0x4180FCC0, 0x7EC3B378
};

const u32 hook2[11] = {
	0x38000000, 0x981F000A, 0xA01F0000, 0x5400066E,	0xB01F0000, 0x3AB50001, 0x2C150004, 0x3B18000C,	0x3BFF000C, 0x4180FCC8, 0x7EC3B378
};

const u32 hook3[11] = {
	0x38000000, 0x9817000A, 0xA0170000, 0x5400066E,	0xB0170000, 0x3B390001, 0x2C190004, 0x3B9C000C,	0x3AF7000C, 0x4180FD6C, 0x7F43D378
};

const u32 memset_hook[12] = {
	0x28050020,	0x5484063e,	0x38c3ffff,	0x7c872378,	0x41800090,	0x7cc030f8,	0x540307bf,	0x41820014,	0x7ca32850,	0x3463ffff,	0x9ce60001,	0x4082fff8
};


void copy_2nd_hook(void *hookoffset)
{
	memset(hookoffset, 0, 64);
	switch (hook2select)
	{
		case 0x01:
			memcpy(hookoffset, (u32 *)vigchooks, sizeof(vigchooks));
		break;

		case 0x02:
			memcpy(hookoffset, (u32 *)GC_OSSleepthread_hooks, sizeof(GC_OSSleepthread_hooks));
		break;

		case 0x03:	
			memcpy(hookoffset, (u32 *)GC_gxdrawdone_hooks, sizeof(GC_gxdrawdone_hooks));
		break;

		case 0x04:	
			memcpy(hookoffset, (u32 *)hook0, sizeof(hook0));
		break;

		case 0x05:	
			memcpy(hookoffset, (u32 *)hook1, sizeof(hook1));
		break;

		case 0x06:	
			memcpy(hookoffset, (u32 *)hook2, sizeof(hook2));
		break;

		case 0x07:	
			memcpy(hookoffset, (u32 *)hook3, sizeof(hook3));
		break;

		case 0x08:	
			memcpy(hookoffset, (u32 *)memset_hook, sizeof(memset_hook));
		break;
		
		default:
		break;
	}
}

void patchhook2(u32 address, u32 len)
{
	patchhook(address, len);
	printf("Patching hook at 0x%08x\n", address);
}

//---------------------------------------------------------------------------------
bool dogamehooks(void *addr, u32 len)
//---------------------------------------------------------------------------------
{
	void *addr_start = addr;
	void *addr_end = addr+len;
	bool hookpatched = false;

	while(addr_start < addr_end)
	{
		switch (hookselect)
		{
			case 0x01:
				if (codecmp(addr_start, (u32 *)vigchooks, sizeof(vigchooks))==0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x02:
				if (codecmp(addr_start, (u32 *)GC_OSSleepthread_hooks, sizeof(GC_OSSleepthread_hooks)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x03:	
				if (codecmp(addr_start, (u32 *)GC_gxdrawdone_hooks, sizeof(GC_gxdrawdone_hooks)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x04:	
				if (codecmp(addr_start, (u32 *)hook0, sizeof(hook0)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x05:	
				if (codecmp(addr_start, (u32 *)hook1, sizeof(hook1)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x06:	
				if (codecmp(addr_start, (u32 *)hook2, sizeof(hook2)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x07:	
				if (codecmp(addr_start, (u32 *)hook3, sizeof(hook3)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;

			case 0x08:	
				if (codecmp(addr_start, (u32 *)memset_hook, sizeof(memset_hook)) == 0)
				{
					patchhook2((u32)addr_start, len);
					hookpatched = true;
				}
			break;
		
			case 0x00:
			default:
				//hookpatched = true;
			break;

		}
		addr_start += 4;
	}
	return hookpatched;
}



