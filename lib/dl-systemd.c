/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <dlfcn.h>

#شامل "c.h"
#شامل "dl-systemd.h"

#اگر defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_ترجیح_RECOMMENDED,
		    "libsystemd.so.0");

ساخت ul_systemd_opers ul_systemd;

ساکن ثابت ساخت ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#اگر HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#اگر_ختم

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_اگلا ),
	UL_DLSYM( ul_systemd_opers, sd_journal_پچھلا_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_ڈیٹا ),
	UL_DLSYM( ul_systemd_opers, sd_journal_جوڑو_مطابقت ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_دم ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#اگر HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_syspath ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_property_قدر ),
	UL_DLSYM( ul_systemd_opers, sd_device_ref ),
	UL_DLSYM( ul_systemd_opers, sd_device_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_action ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_devname ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_is_initialized ),

	UL_DLSYM( ul_systemd_opers, sd_device_monitor_new ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_get_event ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_آغاز ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_filter_جوڑو_مطابقت_subsystem_devtype ),

	UL_DLSYM( ul_systemd_opers, sd_event_جوڑو_وقت_relative ),
	UL_DLSYM( ul_systemd_opers, sd_event_loop ),
	UL_DLSYM( ul_systemd_opers, sd_event_exit ),
#اگر_ختم
#اگر HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#اگر_ختم
};

صحیح ul_dlopen_libsystemd(خالی)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساکن صحیح حالت_2 = 0;
	ساکن خالی *dl = تہی;
	صحیح flags = RTLD_LAZY | RTLD_LOCAL;

	اگر (حالت_2)
		واپس حالت_2 > 0 ? 0 : -ENOSYS;

#اگر_تعریف_شدہ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#اگر_ختم
	حالت_2 = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_سائز(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	واپس حالت_2 > 0 ? 0 : -ENOSYS;
}

#اگر_ختم /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
