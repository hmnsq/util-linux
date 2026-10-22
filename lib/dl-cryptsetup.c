/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-cryptsetup.h"

#ಆದರೆ defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_ಆದ್ಯತೆ_SUGGESTED,
		    "libcryptsetup.so.12");

ರಚನೆ ul_cryptsetup_opers ul_cryptsetup;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_ಗಣ_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ಗಣ_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_ದತ್ತಾಂಶ_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ತುಂಬು ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_ಕೀಲಿ_ಗಾತ್ರ ),
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_CRYPT_ACTIVATE_BY_SIGNED_ಕೀಲಿ
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_ಕೀಲಿ ),
#ಆದರೆ_ಅಂತ್ಯ
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_ಕೀಲಿ ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_ಕೀಲಿ_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

ಪೂರ್ಣಾಂಕ ul_dlopen_libcryptsetup(ಶೂನ್ಯ)
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ_2 = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_ಗಾತ್ರ(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 > 0 ? 0 : -ENOSYS;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
