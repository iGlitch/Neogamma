#ifndef _DVD_H_
#define _DVD_H_

typedef void interrupt_callback_t(int result);

void dvd_read_replacement1(
	void *buffer,
	unsigned long size,
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_audio_config_replacement1(
	unsigned long activate,
	unsigned long size,
	interrupt_callback_t *callback_fn);

void dvd_read_audio_replacement1(
	unsigned long subcmd,
	unsigned long length,
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_seek_replacement1(
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_reset_replacement1(void);

void dvd_read_id_replacement1(
	void *buffer,
	interrupt_callback_t *callback_fn);

void dvd_callback_replacement1(
	int result,
	interrupt_callback_t *callback_fn);

void dvd_stop_motor_replacement1(
	interrupt_callback_t *function);

void dvd_report_error_replacement1(
	interrupt_callback_t *function);

void dvd_audio_status_replacement1(
	u32 subcmd,
	interrupt_callback_t *function);

#ifdef ACTION_REPLAY
void ar_dvd_read_replacement1(
	unsigned long offset,
	void *buffer,
	unsigned long size);

void ar_dvd_read_disk_id_replacement1(void *buffer);
#endif

#ifdef RELOAD_SUPPORT
void dvd_read_replacement2(
	void *buffer,
	unsigned long size,
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_audio_config_replacement2(
	unsigned long activate,
	unsigned long size,
	interrupt_callback_t *callback_fn);

void dvd_read_audio_replacement2(
	unsigned long subcmd,
	unsigned long length,
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_seek_replacement2(
	unsigned long offset,
	interrupt_callback_t *callback_fn);

void dvd_reset_replacement2(void);

void dvd_read_id_replacement2(
	void *buffer,
	interrupt_callback_t *callback_fn);

void dvd_callback_replacement2(
	int result,
	interrupt_callback_t *callback_fn);

void dvd_stop_motor_replacement2(
	interrupt_callback_t *function);

void dvd_report_error_replacement2(
	interrupt_callback_t *function);

void dvd_audio_status_replacement2(
	u32 subcmd,
	interrupt_callback_t *function);

#endif



#endif
