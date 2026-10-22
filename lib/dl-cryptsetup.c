/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <dlfcn.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "dl-cryptsetup.h"

#ଯଦି defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

ସଂରଚନା ul_cryptsetup_opers ul_cryptsetup;

ସ୍ଥିର ଅଚଳ ସଂରଚନା ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_ଚାବି_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_ଚାବି
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_ଚାବି ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_ଚାବି ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_ଚାବି_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_dlopen_libcryptsetup(ଶୂନ୍ୟ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;
	ସ୍ଥିର ଶୂନ୍ୟ *dl = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = RTLD_LAZY | RTLD_LOCAL;

	ଯଦି (status)
		ଫେରନ୍ତୁ status > 0 ? 0 : -ENOSYS;

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

	ଫେରନ୍ତୁ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
