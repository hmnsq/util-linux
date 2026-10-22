/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <dlfcn.h>

#समावेश "c.h"
#समावेश "dl-econf.h"

#अगर defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

काष्ठा ul_econf_opers ul_econf;

अटल स्थिर काष्ठा ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#अगर_घोषित HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#पूर्ण_अगर
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_कुंजियाँ ),
	UL_DLSYM( ul_econf_opers, econf_get_String_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_मूल्य ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

पूर्णांक ul_dlopen_libeconf(व्योम)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	अटल पूर्णांक status = 0;
	अटल व्योम *dl = शून्य;
	पूर्णांक flags = RTLD_LAZY | RTLD_LOCAL;

	अगर (status)
		वापस status > 0 ? 0 : -ENOSYS;

#अगर_घोषित RTLD_NODELETE
	flags |= RTLD_NODELETE;
#पूर्ण_अगर
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_आकार(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	वापस status > 0 ? 0 : -ENOSYS;
}

#पूर्ण_अगर /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
