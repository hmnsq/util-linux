/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-econf.h"

#ಆದರೆ defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_ಆದ್ಯತೆ_RECOMMENDED,
		    "libeconf.so.0");

ರಚನೆ ul_econf_opers ul_econf;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_ವಿಲೀನ_Files ),
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#ಆದರೆ_ಅಂತ್ಯ
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_ಕೀಲಿಗಳು ),
	UL_DLSYM( ul_econf_opers, econf_get_String_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

ಪೂರ್ಣಾಂಕ ul_dlopen_libeconf(ಶೂನ್ಯ)
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
	ಸ್ಥಿತಿ_2 = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_ಗಾತ್ರ(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2 > 0 ? 0 : -ENOSYS;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
