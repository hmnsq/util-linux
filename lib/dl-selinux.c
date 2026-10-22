/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-selinux.h"

#ಯದಿ defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libselinux.so.1");

ಸಂರಚನಾ ul_selinux_opers ul_selinux;

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ul_dlsym ul_selinux_symbols[] =
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
#ifdef HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#endif
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_to_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_to_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_set ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

ಪೂರ್ಣಾಙ್ಕ ul_dlopen_libselinux(ಶೂನ್ಯ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ status = 0;
	ಸ್ಥಿರ ಶೂನ್ಯ *dl = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ flags = RTLD_LAZY | RTLD_LOCAL;

	ಯದಿ (status)
		ಪ್ರತ್ಯಾಗಮ status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_SIZE(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	ಪ್ರತ್ಯಾಗಮ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
