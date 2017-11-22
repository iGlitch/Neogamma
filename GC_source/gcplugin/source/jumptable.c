/* Copyright 2009 WiiGator. */
#include "plugin.h"
#include "dvd.h"

#include "dvd_orig.h"

#include "stream.h"
#include "pluginprotector.h"

#ifdef GEKKO_DEBUG
// Debug plugin
gcplugin_jumptable_debug_t jumptable[] = {
	{
	.dvd_read_replacement = dvd_read_replacement1,
	.dvd_read_id_replacement = dvd_read_id_replacement1,
	.dvd_seek_replacement = dvd_seek_replacement1,

	.dvd_audio_config_replacement = dvd_audio_config_replacement1,
	.dvd_audio_status_replacement = dvd_audio_status_replacement1,
	.dvd_read_audio_replacement = dvd_read_audio_replacement1,

	.memset_replacement = memset_replacement1,
	.memset_orig = memset_orig1,
	.dvd_read_orig = dvd_read_orig1,

#ifdef ACTION_REPLAY
	.ar_dvd_read_replacement = ar_dvd_read_replacement1,
	.ar_dvd_read_disk_id_replacement = ar_dvd_read_disk_id_replacement1,
#endif

#ifdef GEKKO_DEBUG
	.dvd_audio_status_orig = dvd_audio_status_orig1,
	.dvd_read_audio_orig = dvd_read_audio_orig1,

	.dvd_report_error_replacement = dvd_report_error_replacement1,
	.dvd_report_error_orig = dvd_report_error_orig1,
	.dvd_reset_replacement = dvd_reset_replacement1,
	.dvd_reset_orig = dvd_reset_orig1,
	.dvd_stop_motor_replacement = dvd_stop_motor_replacement1,
	.dvd_stop_motor_orig = dvd_stop_motor_orig1,
#ifdef REPLACE_DVD_CALLBACK
	.dvd_callback_entry = dvd_callback_replacement1,
#endif
	.stream_write_replacement = stream_write_replacement1,
	.stream_write_orig = stream_write_orig1,
	.printf_replacement = printf_replacement1
#ifndef MIOS_PLUGIN
#ifdef ACTION_REPLAY
	//.ar_stream_write_replacement = ar_stream_write_replacement1,
	//.ar_stream_write_orig = ar_stream_write_orig1,
#endif
#endif
#ifdef RELOAD_SUPPORT
#endif
#endif
	},
#ifdef RELOAD_SUPPORT
	{
	.dvd_read_replacement = dvd_read_replacement2,
	.dvd_read_id_replacement = dvd_read_id_replacement2,
	.dvd_seek_replacement = dvd_seek_replacement2,

	.dvd_audio_config_replacement = dvd_audio_config_replacement2,
	.dvd_audio_status_replacement = dvd_audio_status_replacement2,
	.dvd_read_audio_replacement = dvd_read_audio_replacement2,

	.memset_replacement = memset_replacement2,
	.memset_orig = memset_orig2,
	.dvd_read_orig = dvd_read_orig2,

#ifdef GEKKO_DEBUG
	.dvd_audio_status_orig = dvd_audio_status_orig2,
	.dvd_read_audio_orig = dvd_read_audio_orig2,

	.dvd_report_error_replacement = dvd_report_error_replacement2,
	.dvd_report_error_orig = dvd_report_error_orig2,
	.dvd_reset_replacement = dvd_reset_replacement2,
	.dvd_reset_orig = dvd_reset_orig2,
	.dvd_stop_motor_replacement = dvd_stop_motor_replacement2,
	.dvd_stop_motor_orig = dvd_stop_motor_orig2,
#ifdef REPLACE_DVD_CALLBACK
	.dvd_callback_entry = dvd_callback_replacement2,
#endif
	.stream_write_replacement = stream_write_replacement2,
	.stream_write_orig = stream_write_orig2
#endif
	}
#endif
};
#else
// Regular plugins
#ifdef LOW_PLUGIN
gcplugin_jumptable_low_t jumptable[] = {
#endif
#ifdef HIGH_PLUGIN
gcplugin_jumptable_high_t jumptable[] = {
#endif
	{
	.dvd_read_replacement = dvd_read_replacement1,
	.dvd_read_id_replacement = dvd_read_id_replacement1,
	.dvd_seek_replacement = dvd_seek_replacement1,

	.dvd_audio_config_replacement = dvd_audio_config_replacement1,
	.dvd_audio_status_replacement = dvd_audio_status_replacement1,
	.dvd_read_audio_replacement = dvd_read_audio_replacement1,

	.memset_replacement = memset_replacement1,
	.memset_orig = memset_orig1,
	.dvd_read_orig = dvd_read_orig1,
	
#ifdef ACTION_REPLAY
	.ar_dvd_read_replacement = ar_dvd_read_replacement1,
	.ar_dvd_read_disk_id_replacement = ar_dvd_read_disk_id_replacement1
#endif
	},
#ifdef RELOAD_SUPPORT
	{
	.dvd_read_replacement = dvd_read_replacement2,
	.dvd_read_id_replacement = dvd_read_id_replacement2,
	.dvd_seek_replacement = dvd_seek_replacement2,

	.dvd_audio_config_replacement = dvd_audio_config_replacement2,
	.dvd_audio_status_replacement = dvd_audio_status_replacement2,
	.dvd_read_audio_replacement = dvd_read_audio_replacement2,

	.memset_replacement = memset_replacement2,
	.memset_orig = memset_orig2,
	.dvd_read_orig = dvd_read_orig2
	}
#endif
};
#endif



unsigned long base_offset = 0x11111111;
unsigned long base_offset_2nd_disc = 0x22222222;

#ifdef RELOAD_SUPPORT
u8 reloader_enabled = 0x33;
//u8 dummy1 = 0x55;
//u8 dummy2 = 0x66;
/** Current jumptable used. */
u8 jmpNr = 0;
u8 reloader_status = 0;
#endif

u8 audio_stream_fix = 0x44;
u8 mios_mode = 0x45;

#ifdef GEKKO_DEBUG
u8 backuplaunching = 0x46;
#endif

#ifdef DONTTELL
u8 hooks = 0x48;
u32 hook[16] = {	0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555,
					0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555, 0x55555555 };
#endif

u32 elf_base_offset = 0;
u32 elf_offset = 0;
