/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <dlfcn.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "dl-econf.h"

#എങ്കിൽ defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

ഘടന ul_econf_opers ul_econf;

സ്ഥിതം സ്ഥിരം ഘടന ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_താക്കോലുകൾ ),
	UL_DLSYM( ul_econf_opers, econf_get_String_മൂല്യം ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_മൂല്യം ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_മൂല്യം ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

പൂർണ്ണസംഖ്യ ul_dlopen_libeconf(ശൂന്യം)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	സ്ഥിതം പൂർണ്ണസംഖ്യ status = 0;
	സ്ഥിതം ശൂന്യം *dl = ശൂന്യ;
	പൂർണ്ണസംഖ്യ flags = RTLD_LAZY | RTLD_LOCAL;

	എങ്കിൽ (status)
		തിരികെ status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	തിരികെ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
