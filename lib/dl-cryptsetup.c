/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <dlfcn.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "dl-cryptsetup.h"

#എങ്കിൽ defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

ഘടന ul_cryptsetup_opers ul_cryptsetup;

സ്ഥിതം സ്ഥിരം ഘടന ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_താക്കോൽ_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_താക്കോൽ
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_താക്കോൽ ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_താക്കോൽ ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_താക്കോൽ_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

പൂർണ്ണസംഖ്യ ul_dlopen_libcryptsetup(ശൂന്യം)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	സ്ഥിതം പൂർണ്ണസംഖ്യ status = 0;
	സ്ഥിതം ശൂന്യം *dl = ശൂന്യ;
	പൂർണ്ണസംഖ്യ flags = RTLD_LAZY | RTLD_LOCAL;

	എങ്കിൽ (status)
		തിരികെ status > 0 ? 0 : -ENOSYS;

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

	തിരികെ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
