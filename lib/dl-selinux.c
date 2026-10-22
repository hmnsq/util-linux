/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <dlfcn.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "dl-selinux.h"

#ఒకవేళ defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_ప్రాధాన్యత_RECOMMENDED,
		    "libselinux.so.1");

నిర్మాణం ul_selinux_opers ul_selinux;

స్థిర స్థిరరాశి నిర్మాణం ul_dlsym ul_selinux_symbols[] =
{
	UL_DLSYM( ul_selinux_opers, is_selinux_enabled ),

	UL_DLSYM( ul_selinux_opers, getfilecon ),
	UL_DLSYM( ul_selinux_opers, getfilecon_raw ),
	UL_DLSYM( ul_selinux_opers, fgetfilecon ),
	UL_DLSYM( ul_selinux_opers, lgetfilecon ),

	UL_DLSYM( ul_selinux_opers, getcon ),
	UL_DLSYM( ul_selinux_opers, getprevcon ),
	UL_DLSYM( ul_selinux_opers, getpidcon ),

	UL_DLSYM( ul_selinux_opers, getseuserbyname ),

	UL_DLSYM( ul_selinux_opers, setfilecon ),
	UL_DLSYM( ul_selinux_opers, fsetfilecon ),
	UL_DLSYM( ul_selinux_opers, setfscreatecon ),
	UL_DLSYM( ul_selinux_opers, setexeccon ),

	UL_DLSYM( ul_selinux_opers, freecon ),

	UL_DLSYM( ul_selinux_opers, selinux_check_access ),
	UL_DLSYM( ul_selinux_opers, security_compute_relabel ),
#నిర్వచితమైతే HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#అయితే_ముగింపు
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_గమ్యం_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_గమ్యం_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_సమితి ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

పూర్ణాంకం ul_dlopen_libselinux(శూన్య)
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
	స్థితి_2 = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_కొలత(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;
}

#అయితే_ముగింపు /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
