#ifndef _DVD_H_
#define _DVD_H_

/** Sector size of DVD-R. Minimum size for a transfer. All transfers must be a multiple of this value. */
#define SECTOR_SIZE 0x800

/** Check for DVD errors. */
#define DVD_CHECK() \
	if (rv != 0) { \
		unsigned int err; \
		\
		debug_printf("Failed to read DVD %d.\n", __LINE__); \
		printf("   Failed to read DVD %d.\n", __LINE__); \
		err = dvd_get_error(); \
		debug_printf("Error 0x%x %s\n", err, dvd_get_error_message(err)); \
		printf("   Error 0x%x %s\n", err, dvd_get_error_message(err)); \
		sleep(5); \
		printf("   Halting\n"); \
		while (true); \
	}

extern unsigned long base_offset;

unsigned long ack_cover_interrupt(void);
unsigned long get_cover_status(void);
void dvd_reset(void);
unsigned int dvd_read(void *outbuf, int len, unsigned long long offset);
unsigned int dvd_read_id(void);
void dvd_motor_off(void);
unsigned int dvd_get_error(void);
const char *dvd_get_error_message(unsigned int err);
void wii_dvd_reset_unlock(void);
s32 dvd_set_streaming(void);
void dvd_disable_streaming(void);
s32 dvd_RequestAudioStatus(u32 subcmd);

extern bool backuploading;
#endif
