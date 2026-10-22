/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "dl-cryptsetup.h"

#यदि defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

संरचना ul_cryptsetup_opers ul_cryptsetup;

स्थिर अचर संरचना ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_साँचो_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_साँचो
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_साँचो ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_साँचो ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_साँचो_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

पूर्णाङ्क ul_dlopen_libcryptsetup(शून्य)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	स्थिर पूर्णाङ्क status = 0;
	स्थिर शून्य *dl = रिक्त;
	पूर्णाङ्क flags = RTLD_LAZY | RTLD_LOCAL;

	यदि (status)
		फर्काऊ status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
#ifdef RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#endif
	status = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_SIZE(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	फर्काऊ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
