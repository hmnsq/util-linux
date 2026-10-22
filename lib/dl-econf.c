/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <dlfcn.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "dl-econf.h"

#જો defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

સંરચના ul_econf_opers ul_econf;

સ્થિર અચલ સંરચના ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_ચાવીઓ ),
	UL_DLSYM( ul_econf_opers, econf_get_String_કિંમત ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_કિંમત ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_કિંમત ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

પૂર્ણાંક ul_dlopen_libeconf(શૂન્ય)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	સ્થિર પૂર્ણાંક status = 0;
	સ્થિર શૂન્ય *dl = ખાલી;
	પૂર્ણાંક flags = RTLD_LAZY | RTLD_LOCAL;

	જો (status)
		પાછા status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	પાછા status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
