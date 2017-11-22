#ifndef _STREAM_H_
#define _STREAM_H_

#ifdef GEKKO_DEBUG

/*
int stream_write_replacement1(
	void *stream,
	const char *text,
	unsigned long size);
*/
int stream_write_replacement1(const void *ptr, u32 size, u32 count, void *stream);

#ifdef RELOAD_SUPPORT
int stream_write_replacement2(const void *ptr, u32 size, u32 count, void *stream);
#endif

/*int stream_write_orig1(
	void *stream,
	const char *text,
	unsigned long size);
*/
int stream_write_orig1(const void *ptr, u32 size, u32 count, void *stream);

#ifdef RELOAD_SUPPORT
int stream_write_orig2(const void *ptr, u32 size, u32 count, void *stream);
#endif


#ifdef GEKKO_DEBUG
int printf_replacement1(const char *format, ...);
#endif
/*
#ifndef MIOS_PLUGIN
#ifdef ACTION_REPLAY
unsigned long ar_stream_write_replacement1(
	char **buffer,
	const char *format,
	void *args);

unsigned long ar_stream_write_orig1(
	char **buffer,
	const char *format,
	void *args);
#endif
#endif
*/
#endif

#endif
