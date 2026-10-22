/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <dlfcn.h>

#ضمن "c.h"
#ضمن "dl-systemd.h"

#إذا defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_ترجيح_RECOMMENDED,
		    "libsystemd.so.0");

بنية ul_systemd_opers ul_systemd;

ساكن ثابت بنية ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#إذا HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#جيڪڏهن_پورو

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_ايندڙ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_اڳيون_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_معلومات ),
	UL_DLSYM( ul_systemd_opers, sd_journal_جوڙيو_ميلاپ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_پڇ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#إذا HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
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
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_شروعات ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_filter_جوڙيو_ميلاپ_subsystem_devtype ),

	UL_DLSYM( ul_systemd_opers, sd_event_جوڙيو_وقت_relative ),
	UL_DLSYM( ul_systemd_opers, sd_event_loop ),
	UL_DLSYM( ul_systemd_opers, sd_event_exit ),
#جيڪڏهن_پورو
#إذا HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#جيڪڏهن_پورو
};

صحيح ul_dlopen_libsystemd(فراغ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساكن صحيح حالت = 0;
	ساكن فراغ *dl = عدم;
	صحيح flags = RTLD_LAZY | RTLD_LOCAL;

	إذا (حالت)
		أرجع حالت > 0 ? 0 : -ENOSYS;

#جيڪڏهن_وصف_ٿيل RTLD_NODELETE
	flags |= RTLD_NODELETE;
#جيڪڏهن_پورو
	حالت = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_سائيز(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	أرجع حالت > 0 ? 0 : -ENOSYS;
}

#جيڪڏهن_پورو /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
