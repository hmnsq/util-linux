/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-systemd.h"

#ಆದರೆ defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_ಆದ್ಯತೆ_RECOMMENDED,
		    "libsystemd.so.0");

ರಚನೆ ul_systemd_opers ul_systemd;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#ಆದರೆ HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#ಆದರೆ_ಅಂತ್ಯ

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_ಮುಂದಿನ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_ಹಿಂದಿನ_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_ದತ್ತಾಂಶ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_ಕೂಡಿಸು_ಹೊಂದಿಕೆ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_ಬಾಲ ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#ಆದರೆ HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_syspath ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_property_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_systemd_opers, sd_device_ref ),
	UL_DLSYM( ul_systemd_opers, sd_device_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_action ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_devname ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_is_initialized ),

	UL_DLSYM( ul_systemd_opers, sd_device_monitor_new ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_get_event ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_ಆರಂಭ ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_filter_ಕೂಡಿಸು_ಹೊಂದಿಕೆ_subsystem_devtype ),

	UL_DLSYM( ul_systemd_opers, sd_event_ಕೂಡಿಸು_ಸಮಯ_relative ),
	UL_DLSYM( ul_systemd_opers, sd_event_loop ),
	UL_DLSYM( ul_systemd_opers, sd_event_exit ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#ಆದರೆ_ಅಂತ್ಯ
};

ಪೂರ್ಣಾಂಕ ul_dlopen_libsystemd(ಶೂನ್ಯ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;
	ಸ್ಥಿರ ಶೂನ್ಯ *dl = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ flags = RTLD_LAZY | RTLD_LOCAL;

	ಆದರೆ (ಸ್ಥಿತಿ_2)
		ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 > 0 ? 0 : -ENOSYS;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ_2 = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_ಗಾತ್ರ(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 > 0 ? 0 : -ENOSYS;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
