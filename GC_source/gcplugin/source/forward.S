/* Copyright 2009 WiiGator. */

#ifdef RELOAD_SUPPORT
/** Macro for declaring one orig function, the code will be patched to jump into game code. */
.macro decl_fn name1 name2
.globl \name1
\name1:
	nop
	blr

.globl \name2
\name2:
	nop
	blr
.endm

/** Orig function declaration when Reloader is activated.
 * There are 2 functions to needed to be able to switch
 * between apploader and game code.
 */
#define DECL_FN(name) \
	decl_fn name##1 name##2

#else

.macro decl_fn name
.globl \name
\name:
	nop
	blr
.endm

/** Orig function declaration when Reloader is deactivated. */
#define DECL_FN(name) \
	decl_fn name##1
#endif

/* List of functions which are replaced. */
DECL_FN(dvd_read_orig)
DECL_FN(memset_orig)
#ifdef GEKKO_DEBUG
DECL_FN(dvd_audio_status_orig)
DECL_FN(dvd_read_audio_orig)
//DECL_FN(dvd_read_id_orig)
DECL_FN(dvd_report_error_orig)
DECL_FN(dvd_stop_motor_orig)
DECL_FN(dvd_reset_orig)
DECL_FN(stream_write_orig)
#endif


#ifdef GEKKO_DEBUG
#ifdef REPLACE_DVD_CALLBACK
.globl dvd_callback_entry1

dvd_callback_entry1:
	/* r3 has DVD command result value. */
	/* r12 points to callback function. */
	mr		%r4, %r12
	b		dvd_callback_replacement1

#ifdef RELOAD_SUPPORT
.globl dvd_callback_entry2

dvd_callback_entry2:
	/* r3 has DVD command result value. */
	/* r12 points to callback function. */
	mr		%r4, %r12
	b		dvd_callback_replacement2
#endif
#endif
#endif

