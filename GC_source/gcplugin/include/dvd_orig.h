#ifndef _DVD_ORIG_H_
#define _DVD_ORIG_H_


void dvd_read_orig1(void *buffer, unsigned long size, unsigned long offset, interrupt_callback_t *function);
#ifdef RELOAD_SUPPORT
void dvd_read_orig2(void *buffer, unsigned long size, unsigned long offset,	interrupt_callback_t *function);
#endif
/*
void dvd_read_id_orig1(void *buffer, interrupt_callback_t *function);
#ifdef RELOAD_SUPPORT
void dvd_read_id_orig2(void *buffer, interrupt_callback_t *function);
#endif
*/

#ifdef GEKKO_DEBUG
void dvd_report_error_orig1(interrupt_callback_t *function);
#ifdef RELOAD_SUPPORT
void dvd_report_error_orig2(interrupt_callback_t *function);
#endif

void dvd_reset_orig1(void);
#ifdef RELOAD_SUPPORT
void dvd_reset_orig2(void);
#endif

void dvd_stop_motor_orig1(	interrupt_callback_t *function);
#ifdef RELOAD_SUPPORT
void dvd_stop_motor_orig2(	interrupt_callback_t *function);
#endif

void dvd_audio_status_orig1(u32 subcmd,	interrupt_callback_t *function);
#ifdef RELOAD_SUPPORT
void dvd_audio_status_orig2(u32 subcmd, interrupt_callback_t *function);
#endif

void dvd_read_audio_orig1(unsigned long subcmd, unsigned long length, unsigned long offset, interrupt_callback_t *callback_fn);
#ifdef RELOAD_SUPPORT
void dvd_read_audio_orig2(unsigned long subcmd, unsigned long length, unsigned long offset, interrupt_callback_t *callback_fn);
#endif
#endif




#endif
