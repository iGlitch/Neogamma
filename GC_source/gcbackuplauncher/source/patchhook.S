.text
.set r0,0;   .set sp,1;   .set r2,2; .set r3,3;   .set r4,4
.set r5,5;   .set r6,6;   .set r7,7;   .set r8,8;   .set r9,9
.set r10,10; .set r11,11; .set r12,12; .set r13,13; .set r14,14
.set r15,15; .set r16,16; .set r17,17; .set r18,18; .set r19,19
.set r20,20; .set r21,21; .set r22,22; .set r23,23; .set r24,24
.set r25,25; .set r26,26; .set r27,27; .set r28,28; .set r29,29
.set r30,30; .set r31,31


.globl patchhook		# r3 address
patchhook:
	mtctr	r4
	lis	r6, 0x4E80
	ori	r6, r6, 0x0020	# blr
findblr:
	lwz	r5, 0(r3)
	cmpw	r6, r5
	beq	writebranch
	addi	r3, r3, 4	# next word
	bdnz	findblr		# loop length
	b	exit		# stop unhooked game hanging

writebranch:
	lis     r4, 0x8000	# 800018A0 hook location (source)
	ori     r4, r4, 0x18A8
	subf    r4, r3, r4	# subtract r3 from r4 and place in r4
	lis     r5, 0x3FF	
	ori     r5, r5, 0xFFFF	# 0x3FFFFFF
	and     r4, r4, r5
	lis     r5, 0x4800	# 0x48000000
	or      r4, r4, r5
	stw     r4, 0(r3)	# result in r3
	dcbf    r0, r3		# data cache block flush
	icbi    r0, r3
exit:
	blr                     # return
	


