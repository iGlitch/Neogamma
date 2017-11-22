#include <ogcsys.h>
#include <gccore.h>

syssram* __SYS_LockSram(void);
u32 __SYS_UnlockSram(u32 write);
u32 __SYS_SyncSram(void);



void setSRAMvideomode(int videomode)
{
	syssram *sram;

	sram = __SYS_LockSram();

	sram->flags = (sram->flags & ~0x03) | videomode;

	__SYS_UnlockSram(TRUE);
	while(!__SYS_SyncSram());
}

u8 getSRAMlanguage()
{
	syssram *sram;
	u8 ret;

	sram = __SYS_LockSram();

	ret = sram->lang;

	__SYS_UnlockSram(TRUE);
	while(!__SYS_SyncSram());
	
	return ret;
}

u8 getSRAMflags()
{
	syssram *sram;
	u8 ret;

	sram = __SYS_LockSram();

	ret = sram->flags;

	__SYS_UnlockSram(TRUE);
	while(!__SYS_SyncSram());
	
	return ret;
}

void setSRAMflags(u8 flags)
{
	syssram *sram;

	sram = __SYS_LockSram();

	sram->flags = flags;

	__SYS_UnlockSram(TRUE);
	while(!__SYS_SyncSram());
}