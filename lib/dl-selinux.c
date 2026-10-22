/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-selinux.h"

#ಆದರೆ defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_ಆದ್ಯತೆ_RECOMMENDED,
		    "libselinux.so.1");

ರಚನೆ ul_selinux_opers ul_selinux;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym ul_selinux_symbols[] =
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#ಆದರೆ_ಅಂತ್ಯ
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_ಗಮ್ಯ_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_ಗಮ್ಯ_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_ಗಣ ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

ಪೂರ್ಣಾಂಕ ul_dlopen_libselinux(ಶೂನ್ಯ)
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
	ಸ್ಥಿತಿ_2 = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_ಗಾತ್ರ(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 > 0 ? 0 : -ENOSYS;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
