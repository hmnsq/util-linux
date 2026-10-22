/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "dl-econf.h"

#ಯದಿ defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

ಸಂರಚನಾ ul_econf_opers ul_econf;

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_ಕೀಲಿಗಳು ),
	UL_DLSYM( ul_econf_opers, econf_get_String_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_ಮೌಲ್ಯ ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

ಪೂರ್ಣಾಙ್ಕ ul_dlopen_libeconf(ಶೂನ್ಯ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ status = 0;
	ಸ್ಥಿರ ಶೂನ್ಯ *dl = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ flags = RTLD_LAZY | RTLD_LOCAL;

	ಯದಿ (status)
		ಪ್ರತ್ಯಾಗಮ status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	ಪ್ರತ್ಯಾಗಮ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
