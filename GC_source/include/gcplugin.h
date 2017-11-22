#ifndef _GCPLUGIN_H_
#define _GCPLUGIN_H_

#if defined(HIGH_PLUGIN) || defined(LOADER)
/** Load address of game plugin. */
#define HIGH_PLUGIN_BASE ((void*) 0x817FC000)
/** Maximum size of the plugin. */
#define HIGH_PLUGIN_SIZE 0x4000
#endif

#if defined(MIOS_PLUGIN) || defined(LOADER)
/** Load address of mios plugin. */
#define MIOS_PLUGIN_BASE ((void*) 0x80f80000)
//#define LOW_PLUGIN_BASE ((void*) 0x80001800)
//#define HIGH_PLUGIN_BASE ((void*) 0x817FC000)
#endif

#if defined(LOW_PLUGIN) || defined(LOADER)
/** Load address of plugin in reserved exception area. */
#define LOW_PLUGIN_BASE ((void*) 0x80001800)
/** Maximum size of the plugin. */
#define LOW_PLUGIN_SIZE 0x1800
#endif

#if defined(GEKKO_DEBUG) || defined(LOADER)
typedef struct
{
	void *dvd_read_replacement;
	void *dvd_read_id_replacement;
	void *dvd_seek_replacement;

	void *dvd_audio_config_replacement;
	void *dvd_audio_status_replacement;
	void *dvd_read_audio_replacement;

	void *memset_replacement;
	void *memset_orig;
	void *dvd_read_orig;

	void *ar_dvd_read_replacement;
	void *ar_dvd_read_disk_id_replacement;
	void (* ar_dvd_wait_for_last_cmd)(void);

	void *dvd_audio_status_orig;
	void *dvd_read_audio_orig;

	void *dvd_report_error_replacement;	
	void *dvd_report_error_orig;
	void *dvd_reset_replacement;
	void *dvd_reset_orig;
	void *dvd_stop_motor_replacement;
	void *dvd_stop_motor_orig;

#ifdef REPLACE_DVD_CALLBACK
	void *dvd_callback_entry;
#endif

	void *stream_write_replacement;
	void *stream_write_orig;
	//void *ar_stream_write_replacement;
	//void *ar_stream_write_orig;
	void *printf_replacement;
} gcplugin_jumptable_debug_t;
#endif

#if (defined(LOW_PLUGIN) && !defined(GEKKO_DEBUG)) || defined(LOADER)
typedef struct
{
	void *dvd_read_replacement;
	void *dvd_read_id_replacement;
	void *dvd_seek_replacement;

	void *dvd_audio_config_replacement;
	void *dvd_audio_status_replacement;
	void *dvd_read_audio_replacement;

	void *memset_replacement;
	void *memset_orig;
	void *dvd_read_orig;
} gcplugin_jumptable_low_t;
#endif

#if (defined(HIGH_PLUGIN) && !defined(GEKKO_DEBUG)) || defined(LOADER)
typedef struct
{
	void *dvd_read_replacement;
	void *dvd_read_id_replacement;
	void *dvd_seek_replacement;

	void *dvd_audio_config_replacement;
	void *dvd_audio_status_replacement;
	void *dvd_read_audio_replacement;

	void *memset_replacement;
	void *memset_orig;
	void *dvd_read_orig;

	void *ar_dvd_read_replacement;
	void *ar_dvd_read_disk_id_replacement;
	void (* ar_dvd_wait_for_last_cmd)(void);
} gcplugin_jumptable_high_t;
#endif





#endif
