/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <dlfcn.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "dl-cryptsetup.h"

#எனில் defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

கட்டமைப்பு ul_cryptsetup_opers ul_cryptsetup;

நிலையான மாறிலி கட்டமைப்பு ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_சாவி_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_சாவி
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_சாவி ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_சாவி ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_சாவி_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

முழு ul_dlopen_libcryptsetup(வெறுமை)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	நிலையான முழு status = 0;
	நிலையான வெறுமை *dl = வெற்று;
	முழு flags = RTLD_LAZY | RTLD_LOCAL;

	எனில் (status)
		திரும்பு status > 0 ? 0 : -ENOSYS;

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

	திரும்பு status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
