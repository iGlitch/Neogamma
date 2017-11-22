/* Code from libogc. */

//#if defined(MIOS_PLUGIN) || defined(HIGH_PLUGIN) /* Only MIOS and high version has enough memory. */
	.globl DCInvalidateRange
DCInvalidateRange:
	cmplwi %r4, 0   # zero or negative size?
	blelr
	clrlwi. %r5, %r3, 27  # check for lower bits set in address
	beq 1f
	addi %r4, %r4, 0x20 
1:
	addi %r4, %r4, 0x1f
	srwi %r4, %r4, 5
	mtctr %r4
2:
	dcbi %r0, %r3
	addi %r3, %r3, 0x20
	bdnz 2b
	blr
//#endif
/*
	.globl DCFlushRange
DCFlushRange:
	cmplwi %r4, 0   # zero or negative size?
	blelr
	clrlwi. %r5, %r3, 27  # check for lower bits set in address
	beq 1f
	addi %r4, %r4, 0x20 
1:
	addi %r4, %r4, 0x1f
	srwi %r4, %r4, 5
	mtctr %r4
2:
	dcbf %r0, %r3
	addi %r3, %r3, 0x20
	bdnz 2b
	sc
	blr

	.globl ICInvalidateRange
ICInvalidateRange:
	cmplwi %r4, 0   # zero or negative size?
	blelr
	clrlwi. %r5, %r3, 27  # check for lower bits set in address
	beq 1f
	addi %r4, %r4, 0x20 
1:
	addi %r4, %r4, 0x1f
	srwi %r4, %r4, 5
	mtctr %r4
2:
	icbi %r0, %r3
	addi %r3, %r3, 0x20
	bdnz 2b
	sync
	isync
	blr
*/
	.globl DCFlushRange_and_ICInvalidateRange
DCFlushRange_and_ICInvalidateRange:
	cmplwi %r4, 0   # zero or negative size?
	blelr
	clrlwi. %r5, %r3, 27  # check for lower bits set in address
	beq 1f
	addi %r4, %r4, 0x20 
1:
	addi %r4, %r4, 0x1f
	srwi %r4, %r4, 5
	mtctr %r4
2:
	dcbf %r0, %r3
	icbi %r0, %r3
	addi %r3, %r3, 0x20
	bdnz 2b
	sc
	sync
	isync
	blr


