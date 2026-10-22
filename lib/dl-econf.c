/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "dl-econf.h"

#जर defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

संरचना ul_econf_opers ul_econf;

स्थिर स्थिरांक संरचना ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_किल्ल्या ),
	UL_DLSYM( ul_econf_opers, econf_get_String_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

पूर्णांक ul_dlopen_libeconf(शून्य)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	स्थिर पूर्णांक status = 0;
	स्थिर शून्य *dl = रिक्त;
	पूर्णांक flags = RTLD_LAZY | RTLD_LOCAL;

	जर (status)
		परत status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	परत status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
