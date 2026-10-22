/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <dlfcn.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "dl-cryptsetup.h"

#જો defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

સંરચના ul_cryptsetup_opers ul_cryptsetup;

સ્થિર અચલ સંરચના ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_ચાવી_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_ચાવી
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_ચાવી ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_ચાવી ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_ચાવી_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

પૂર્ણાંક ul_dlopen_libcryptsetup(શૂન્ય)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	સ્થિર પૂર્ણાંક status = 0;
	સ્થિર શૂન્ય *dl = ખાલી;
	પૂર્ણાંક flags = RTLD_LAZY | RTLD_LOCAL;

	જો (status)
		પાછા status > 0 ? 0 : -ENOSYS;

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

	પાછા status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
