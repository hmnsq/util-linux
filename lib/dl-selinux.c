/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "dl-selinux.h"

#यदि defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_प्राथमिकता_RECOMMENDED,
		    "libselinux.so.1");

संरचना ul_selinux_opers ul_selinux;

स्थिर ध्रुव संरचना ul_dlsym ul_selinux_symbols[] =
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
#यदि_परिभाषितम् HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#यदि_अन्तः
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_गन्तव्यम्_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_गन्तव्यम्_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_समुच्चयः ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

पूर्णाङ्क ul_dlopen_libselinux(शून्य)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	स्थिर पूर्णाङ्क स्थितिः = 0;
	स्थिर शून्य *dl = रिक्त;
	पूर्णाङ्क flags = RTLD_LAZY | RTLD_LOCAL;

	यदि (स्थितिः)
		प्रत्यागम स्थितिः > 0 ? 0 : -ENOSYS;

#यदि_परिभाषितम् RTLD_NODELETE
	flags |= RTLD_NODELETE;
#यदि_अन्तः
	स्थितिः = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_आकार(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	प्रत्यागम स्थितिः > 0 ? 0 : -ENOSYS;
}

#यदि_अन्तः /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
