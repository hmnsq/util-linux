/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "dl-systemd.h"

#सचे defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libsystemd.so.0");

संरचना ul_systemd_opers ul_systemd;

स्थिर ध्रुव संरचना ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#सचे HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#endif

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_next ),
	UL_DLSYM( ul_systemd_opers, sd_journal_previous_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_data ),
	UL_DLSYM( ul_systemd_opers, sd_journal_add_match ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_tail ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#सचे HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_syspath ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_property_मानम् ),
	UL_DLSYM( ul_systemd_opers, sd_device_ref ),
	UL_DLSYM( ul_systemd_opers, sd_device_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_action ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_devname ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_is_initialized ),

	UL_DLSYM( ul_systemd_opers, sd_device_monitor_new ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_get_event ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_start ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_filter_add_match_subsystem_devtype ),

	UL_DLSYM( ul_systemd_opers, sd_event_add_time_relative ),
	UL_DLSYM( ul_systemd_opers, sd_event_loop ),
	UL_DLSYM( ul_systemd_opers, sd_event_exit ),
#endif
#सचे HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#endif
};

पूर्णाङ्क ul_dlopen_libsystemd(शून्य)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	स्थिर पूर्णाङ्क status = 0;
	स्थिर शून्य *dl = रिक्त;
	पूर्णाङ्क flags = RTLD_LAZY | RTLD_LOCAL;

	सचे (status)
		पटिनिवत्त status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_SIZE(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	पटिनिवत्त status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
