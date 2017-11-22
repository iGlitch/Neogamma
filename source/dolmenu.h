#include <gccore.h>

typedef struct _test
{
	u32 count;
	char name[64];
	char dolname[32];
	u32 parameter;
	u32 parent;
} test_t;

test_t *dolmenubuffer;
bool dolmenufallback;

s32 createdolmenubuffer(u32 count);
s32 load_dolmenu(char *discid);
