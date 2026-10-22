/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <dlfcn.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "dl-cryptsetup.h"

#যদি defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

কাঠামো ul_cryptsetup_opers ul_cryptsetup;

স্থির ধ্রুবক কাঠামো ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_তথ্য_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_চাবি_আকার ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_চাবি
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_চাবি ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_চাবি ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_চাবি_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

পূর্ণসংখ্যা ul_dlopen_libcryptsetup(শূন্য)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	স্থির পূর্ণসংখ্যা status = 0;
	স্থির শূন্য *dl = ফাঁকা;
	পূর্ণসংখ্যা flags = RTLD_LAZY | RTLD_LOCAL;

	যদি (status)
		ফেরত status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
#ifdef RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#endif
	status = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_আকার(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	ফেরত status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
