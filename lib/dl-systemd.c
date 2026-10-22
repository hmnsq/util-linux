/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <dlfcn.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "dl-systemd.h"

#ఒకవేళ defined(HAVE_LIBSYSTEMD) && defined(USE_DLOPEN_SYSTEMD)

UL_ELF_NOTE_DLOPEN("systemd",
		    "Support for systemd",
		    UL_ELF_NOTE_DLOPEN_ప్రాధాన్యత_RECOMMENDED,
		    "libsystemd.so.0");

నిర్మాణం ul_systemd_opers ul_systemd;

స్థిర స్థిరరాశి నిర్మాణం ul_dlsym ul_systemd_symbols[] =
{
	UL_DLSYM( ul_systemd_opers, sd_booted ),
	UL_DLSYM( ul_systemd_opers, sd_listen_fds ),

	UL_DLSYM( ul_systemd_opers, sd_get_sessions ),
#ఒకవేళ HAVE_DECL_SD_SESSION_GET_USERNAME
	UL_DLSYM( ul_systemd_opers, sd_session_get_username ),
	UL_DLSYM( ul_systemd_opers, sd_session_get_tty ),
#అయితే_ముగింపు

	UL_DLSYM( ul_systemd_opers, sd_journal_open ),
	UL_DLSYM( ul_systemd_opers, sd_journal_open_directory ),
	UL_DLSYM( ul_systemd_opers, sd_journal_close ),
	UL_DLSYM( ul_systemd_opers, sd_journal_తదుపరి ),
	UL_DLSYM( ul_systemd_opers, sd_journal_మునుపటి_skip ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_realtime_usec ),
	UL_DLSYM( ul_systemd_opers, sd_journal_get_దత్తాంశం ),
	UL_DLSYM( ul_systemd_opers, sd_journal_కూడు_సరిపోలిక ),
	UL_DLSYM( ul_systemd_opers, sd_journal_flush_matches ),
	UL_DLSYM( ul_systemd_opers, sd_journal_seek_తోక ),
	UL_DLSYM( ul_systemd_opers, sd_journal_sendv ),

#ఒకవేళ HAVE_DECL_SD_DEVICE_NEW_FROM_SYSPATH
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_syspath ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_property_విలువ ),
	UL_DLSYM( ul_systemd_opers, sd_device_ref ),
	UL_DLSYM( ul_systemd_opers, sd_device_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_action ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_devname ),
	UL_DLSYM( ul_systemd_opers, sd_device_get_is_initialized ),

	UL_DLSYM( ul_systemd_opers, sd_device_monitor_new ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_unref ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_get_event ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_ప్రారంభం ),
	UL_DLSYM( ul_systemd_opers, sd_device_monitor_filter_కూడు_సరిపోలిక_subsystem_devtype ),

	UL_DLSYM( ul_systemd_opers, sd_event_కూడు_సమయం_relative ),
	UL_DLSYM( ul_systemd_opers, sd_event_loop ),
	UL_DLSYM( ul_systemd_opers, sd_event_exit ),
#అయితే_ముగింపు
#ఒకవేళ HAVE_DECL_SD_DEVICE_OPEN
	UL_DLSYM( ul_systemd_opers, sd_device_open ),
	UL_DLSYM( ul_systemd_opers, sd_device_new_from_devname ),
#అయితే_ముగింపు
};

పూర్ణాంకం ul_dlopen_libsystemd(శూన్య)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	స్థిర పూర్ణాంకం స్థితి_2 = 0;
	స్థిర శూన్య *dl = ఖాళీ;
	పూర్ణాంకం flags = RTLD_LAZY | RTLD_LOCAL;

	ఒకవేళ (స్థితి_2)
		తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;

#నిర్వచితమైతే RTLD_NODELETE
	flags |= RTLD_NODELETE;
#అయితే_ముగింపు
	స్థితి_2 = ul_dlopen_symbols("libsystemd.so.0", flags,
				   ul_systemd_symbols,
				   ARRAY_కొలత(ul_systemd_symbols),
				   &ul_systemd, &dl) == 0 ? 1 : -1;

	తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;
}

#అయితే_ముగింపు /* HAVE_LIBSYSTEMD && USE_DLOPEN_SYSTEMD */
