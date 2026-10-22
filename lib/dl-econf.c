/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "dl-econf.h"

#यदि defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_प्राथमिकता_RECOMMENDED,
		    "libeconf.so.0");

संरचना ul_econf_opers ul_econf;

स्थिर ध्रुव संरचना ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#यदि_परिभाषितम् HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#यदि_अन्तः
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_कुञ्चिकाः ),
	UL_DLSYM( ul_econf_opers, econf_get_String_मानम् ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_मानम् ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_मानम् ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

पूर्णाङ्क ul_dlopen_libeconf(शून्य)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	स्थिर पूर्णाङ्क स्थितिः = 0;
	स्थिर शून्य *dl = रिक्त;
	पूर्णाङ्क flags = RTLD_LAZY | RTLD_LOCAL;

	यदि (स्थितिः)
		प्रत्यागम स्थितिः > 0 ? 0 : -ENOSYS;

#यदि_परिभाषितम् RTLD_NODELETE
	flags |= RTLD_NODELETE;
#यदि_अन्तः
	स्थितिः = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_आकार(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	प्रत्यागम स्थितिः > 0 ? 0 : -ENOSYS;
}

#यदि_अन्तः /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
