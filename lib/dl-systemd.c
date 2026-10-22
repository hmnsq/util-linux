/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <dlfcn.h>

#समावेश "c.h"
#समावेश "dl-systemd.h"

#अगर defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libsystemd.so.0");

काष्ठा ul_systemd_opers ul_systemd;

अटल स्थिर काष्ठा ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#अगर HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#पूर्ण_अगर

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_अग्र ),
	UL_DLSYM( ul_systemd_opers, sd_journal_previous_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_तथ्य ),
	UL_DLSYM( ul_systemd_opers, sd_journal_add_match ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_पुच्छ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#अगर HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_syspath ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_property_मूल्य ),
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
#पूर्ण_अगर
#अगर HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#पूर्ण_अगर
};

पूर्णांक ul_dlopen_libsystemd(व्योम)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	अटल पूर्णांक status = 0;
	अटल व्योम *dl = शून्य;
	पूर्णांक flags = RTLD_LAZY | RTLD_LOCAL;

	अगर (status)
		वापस status > 0 ? 0 : -ENOSYS;

#अगर_घोषित RTLD_NODELETE
	flags |= RTLD_NODELETE;
#पूर्ण_अगर
	status = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_आकार(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	वापस status > 0 ? 0 : -ENOSYS;
}

#पूर्ण_अगर /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
