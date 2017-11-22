/* Copyright 2009 WiiGator. */
/* Based on code from GCOS. */
#include "exception.h"
#include "context.h"
#include "debugprintf.h"
#include "processor.h"
#include "plugin.h"

#ifdef GEKKO_DEBUG

/** Set breakpoint. */
#define SET_IABR(addr) \
	__asm__("mtspr 1010, %1\n"::"r"(0), "r"((addr) | 2));

/** Set data breakpoint. */
#define SET_DABR(addr, flags) \
	__asm__("mtspr 1013, %1\n"::"r"(0), "r"(((addr) & 0xFFFFFFF8) | ((flags) & 0x7)))

void exception_handler_default();
void exception_set_handler(int exception, void (*handler)(int, struct context_s*));

const char* exception_name[15] =
{
	"System Reset", "Machine Check", "DSI", "ISI", "Interrupt", "Alignment", "Program", "Floating Point", "Decrementer", "System Call", "Trace", "Performance", "IABR", "Reserved", "Thermal"
};
#endif

void exception_init(void)
{
#ifdef GEKKO_DEBUG
#ifndef MIOS_PLUGIN /* MIOS have already a debug handler, which prints correct register values. */
	static int initialized;

	/* Install exception handlers for debug purpose. */
	if (!initialized) {
		int i;

		//debug_printf("Initializing exception handlers.\n");
		for (i = 0; i < 15; ++i)
		{
			/* Don't overwrite interrupt, floating point handler and decrmenter, because this is normally used by the game.*/
			if ((i != 4) && (i != 7) && (i != 8)) {
				exception_set_handler(i, exception_handler_default);
			}
		}
		initialized = -1;
	}
#endif
#endif
}

#ifdef GEKKO_DEBUG
void exception_set_handler(int exception, void (*handler)(int, struct context_s*))
{
	/* The game installs an exception handler, which will call the functions in the table at 0x80003000, if exception is recoverable. */
	((void**)0x80003000)[exception] = handler;
}

void exception_handler_default(int exception)
{
	struct context_s* c = (struct context_s*)CONTEXT_CURRENT;

	/* XXX: Caution context structure definition is not correct for all registers!
	 * Only registers R3, R4 and R5 are valid! Other registers are not saved in handler.
	 */
	debug_printf("Exception %s, Caution some GPR values are not correct!\n", exception_name[exception]);
	debug_printf("GPR00 %x GPR08 %x GPR16 %x GPR24 %x\n", c->GPR[0], c->GPR[8], c->GPR[16], c->GPR[24]);
	debug_printf("GPR01 %x GPR09 %x GPR17 %x GPR25 %x\n", c->GPR[1], c->GPR[9], c->GPR[17], c->GPR[25]);
	debug_printf("GPR02 %x GPR10 %x GPR18 %x GPR26 %x\n", c->GPR[2], c->GPR[10], c->GPR[18], c->GPR[26]);
	debug_printf("GPR03 %x GPR11 %x GPR19 %x GPR27 %x\n", c->GPR[3], c->GPR[11], c->GPR[19], c->GPR[27]);
	debug_printf("GPR04 %x GPR12 %x GPR20 %x GPR28 %x\n", c->GPR[4], c->GPR[12], c->GPR[20], c->GPR[28]);
	debug_printf("GPR05 %x GPR13 %x GPR21 %x GPR29 %x\n", c->GPR[5], c->GPR[13], c->GPR[21], c->GPR[29]);
	debug_printf("GPR06 %x GPR14 %x GPR22 %x GPR30 %x\n", c->GPR[6], c->GPR[14], c->GPR[22], c->GPR[30]);
	debug_printf("GPR07 %x GPR15 %x GPR23 %x GPR31 %x\n", c->GPR[7], c->GPR[15], c->GPR[23], c->GPR[31]);
	//debug_printf("Exception %x\n", exception);
	debug_printf("LR %x SRR0 %x %x\n", c->LR, c->SRR0, c->SRR1);
	debug_printf("DAR: %x DSISR %x\n", mfspr(19), mfspr(18));

	while (1);
}
#endif
