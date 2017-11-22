#include <ogcsys.h>

#define Patchmode_none 0
#define Patchmode_patch_normal 1		// secure video mode patches
#define Patchmode_patch_576 2			// patches 480<->576 video modes too
#define Patchmode_patch_brute 3			// patches all video modes


void toy_reset();
void toy_register_apploader_file_loading(void *Address, u32 Size);
void toy_patch_video_modes();
void set_rmode(GXRModeObj* rmode);



