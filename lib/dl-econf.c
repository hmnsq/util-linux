/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <dlfcn.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "dl-econf.h"

#எனில் defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

கட்டமைப்பு ul_econf_opers ul_econf;

நிலையான மாறிலி கட்டமைப்பு ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_சாவிகள் ),
	UL_DLSYM( ul_econf_opers, econf_get_String_மதிப்பு ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_மதிப்பு ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_மதிப்பு ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

முழு ul_dlopen_libeconf(வெறுமை)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	நிலையான முழு status = 0;
	நிலையான வெறுமை *dl = வெற்று;
	முழு flags = RTLD_LAZY | RTLD_LOCAL;

	எனில் (status)
		திரும்பு status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	திரும்பு status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
