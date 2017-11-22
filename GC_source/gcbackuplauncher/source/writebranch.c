#include <gctypes.h>
#include <stdint.h>
#include <ogc/cache.h>

#include "debugprintf.h"
#include "writebranch.h"

/** Write branch:
 * sourceAddr = Address we want to write the branch over
 * destAddr = The new branch we write will take us to this address
 */
void writebranch(void *sourceAddr,void *destAddr) {
	u32 temp;
	u32 temp1;

	/* Calculate branch offset: */
	temp = ((destAddr)-(sourceAddr));
	temp1 = temp & 0x03FFFFFF;

	/* Branch to new code: */
	temp = temp1 | 0x48000000;
	*(u32*)sourceAddr = temp;
	
}

/** Write branch:
 * sourceAddr = Address we want to write the branch over
 * destAddr = The new branch we write will take us to this address
 */
void writeabsolutebranch(void *sourceAddr,void *destAddr) {
	u32 *code = (u32*)sourceAddr;
	uint32_t base;

	base = (uint32_t) destAddr;

	code[0] = 0x3d800000 | (base >> 16); 			// lis     r12,0
	code[1] = 0x618c0000 | (base & 0x0000FFFF);		// ori     r12,r12,0
	code[2] = 0x7d8903a6;							// mtctr   r12
	code[3] = 0x4e800420;							// bctr
	
	DCFlushRange(sourceAddr, 32);
	ICInvalidateRange(sourceAddr, 32);
}

/** Write branch:
 * sourceAddr = Address we want to write the branch over
 * destAddr = The new branch we write will take us to this address
 */
void writebranch_with_link(void *sourceAddr,void *destAddr) {
	u32 temp;
	u32 temp1;

	/* Calculate branch offset: */
	temp = ((destAddr)-(sourceAddr));
	temp1 = temp & 0x03FFFFFF;

	/* Branch to new code: */
	temp = temp1 | 0x48000001;
	*(u32*)sourceAddr = temp;
	
}

/** Install a replacement function.
 * @param replacedFn Points to start of function which should be replaced by replacementFn.
 * @param replacementFn Points to start of replacement function.
 * @param keptFn Points to start of function which should call the original function as it were not patched.
 */
void install_replacement(u32 *replacedFn, u32 *replacementFn, u32 *keptFn) {
	/* Copy instruction which will be overwritten by patch into function which will calls original function. */
	keptFn[0] = replacedFn[0];

	/* Jump to replacement function. */
	writebranch(replacedFn, replacementFn);

	/* Jump to orignal function. */
	writebranch(&keptFn[1], &replacedFn[1]);

	/* Result:
	 * A call of replacedFn will call the replacement function.
	 * A call of keptFn will call original function as it were not patched.
	 */
}

/** Get destination addressof jump. */
uint32_t *get_bl_target(uint32_t *src)
{
	uint32_t dst;
	int16_t *offset;

	offset = ((int16_t *) src) + 1;

	debug_printf("src 0x%08x offset 0x%08x 0x%04x\n", src, offset, (uint32_t) *offset);

	dst = (uint32_t) src;
	debug_printf("dst 0x%08x\n", dst);
	dst = dst + (((int32_t) *offset) & 0xFFFFFFFE);
	debug_printf("dst 0x%08x\n", dst);

	debug_printf("Branch target 0x%08x\n", dst);
	return (uint32_t *) dst;
}


