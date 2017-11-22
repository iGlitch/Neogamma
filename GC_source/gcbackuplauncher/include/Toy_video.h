#include <ogcsys.h>


extern GXRModeObj TVNtsc480Int; // declared somewhere inside libogc, but missing in the correct .h file
extern GXRModeObj TVPal528Prog; 
extern GXRModeObj TVPal528ProgSoft;
extern GXRModeObj TVPal528ProgUnknown;
extern GXRModeObj TVMpal480Prog;


bool compare_videomodes(GXRModeObj* mode1, GXRModeObj* mode2);
void patch_videomode(GXRModeObj* mode1, GXRModeObj* mode2);
int videomode_region(GXRModeObj* mode);
int videomode_interlaced(GXRModeObj* mode);
int videomode_480(GXRModeObj* mode);
