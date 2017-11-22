#define Patchmode_none 0
#define Patchmode_patch_normal 1		// secure video mode patches
#define Patchmode_patch_576 2			// patches 480<->576 video modes too
#define Patchmode_patch_brute 3			// patches all video modes
#define Patchmode_videomodemenu 4
#define Patchmode_dump 5
#define Patchmode_load 6

bool mount_WBFS_disc();


void toy_reset();
bool toy_do_regular_patches(void **Address, int *Size);
void toy_register_apploader_file_loading(void *Address, u32 Size);


void toy_patch_video_modes_to(GXRModeObj* vmode);



