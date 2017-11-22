#include <stdint.h>
#include <gctypes.h>

#include "codecmp.h"

/** Instruction type lwz rD, d(rA). */
#define INSN_TYPE_LWZ 32
/** Instruction type stw rS, d(rA). */
#define INSN_TYPE_STW 36
/** Instruction b or bl. */
#define INSN_TYPE_B 18
/** Instruction bc. */
#define INSN_TYPE_BC 16
/** This register is used to access fast global variables. */
#define SDA_REG1 2
/** This register is used to access fast global variables. */
#define SDA_REG2 13

/** Compare code and ignore offsets used for r2 or r13. */
int codecmp(void *checkedCode, const u32 *condition, u32 size)
{
	u32 *code;
	int i;
	int numberOfInstructions;

	code = (u32 *) checkedCode;
	numberOfInstructions = size/sizeof(*condition);

	for (i = 0; i < numberOfInstructions; i++) {
		u32 type;
		u32 rA;

		if (condition[i] == 0) {
			continue;
		}

		type = (condition[i] >> (31 - 5)) & 0x1f;
		rA = (condition[i] >> (31 - 15)) & 0x1f;

		if (((type == INSN_TYPE_LWZ) || (type == INSN_TYPE_STW))
			||((rA == SDA_REG1) || (rA == SDA_REG2))) {
			/* Don't compare offset, because offset can change each time a program is compiled. */
			if ((code[i] & 0xFFFF0000) != (condition[i] & 0xFFFF0000)) {
				/* Code is not identical. */
				return -1;
			}
		} else if (type == INSN_TYPE_BC) {
			/* Ignore offset for branches. */
			if ((code[i] & 0xFFFF0003) != (condition[i] & 0xFFFF0003)) {
				/* Code is not identical. */
				return -1;
			}
		} else if (type == INSN_TYPE_B) {
			/* Ignore offset for branches. */
			if ((code[i] & 0xFC000003) != (condition[i] & 0xFC000003)) {
				/* Code is not identical. */
				return -1;
			}
		} else if (code[i] != condition[i]) {
			/* Code is not identical. */
			return -1;
		}
	}
	return 0;
}


