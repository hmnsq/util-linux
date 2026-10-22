/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dl-cryptsetup.h"

#ਜੇ defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_ਤਰਜੀਹ_SUGGESTED,
		    "libcryptsetup.so.12");

ਬਣਤਰ ul_cryptsetup_opers ul_cryptsetup;

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_ਸੈੱਟ_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ਸੈੱਟ_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_ਅੰਕੜੇ_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ਲੱਦੋ ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_ਕੁੰਜੀ_ਮਾਪ ),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_CRYPT_ACTIVATE_BY_SIGNED_ਕੁੰਜੀ
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_ਕੁੰਜੀ ),
#ਜੇ_ਅੰਤ
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_ਕੁੰਜੀ ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_ਕੁੰਜੀ_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

ਪੂਰਨ ul_dlopen_libcryptsetup(ਖਾਲੀ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ਸਥਿਰ ਪੂਰਨ ਸਥਿਤੀ = 0;
	ਸਥਿਰ ਖਾਲੀ *dl = ਨਿਰਾ;
	ਪੂਰਨ flags = RTLD_LAZY | RTLD_LOCAL;

	ਜੇ (ਸਥਿਤੀ)
		ਵਾਪਸ ਸਥਿਤੀ > 0 ? 0 : -ENOSYS;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#ਜੇ_ਅੰਤ
	ਸਥਿਤੀ = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_ਮਾਪ(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	ਵਾਪਸ ਸਥਿਤੀ > 0 ? 0 : -ENOSYS;
}

#ਜੇ_ਅੰਤ /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
