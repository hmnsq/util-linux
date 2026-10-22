/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <dlfcn.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "dl-econf.h"

#ଯଦି defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

ସଂରଚନା ul_econf_opers ul_econf;

ସ୍ଥିର ଅଚଳ ସଂରଚନା ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_ଚାବିଗୁଡ଼ିକ ),
	UL_DLSYM( ul_econf_opers, econf_get_String_ମୂଲ୍ୟ ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_ମୂଲ୍ୟ ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_ମୂଲ୍ୟ ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_dlopen_libeconf(ଶୂନ୍ୟ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;
	ସ୍ଥିର ଶୂନ୍ୟ *dl = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = RTLD_LAZY | RTLD_LOCAL;

	ଯଦି (status)
		ଫେରନ୍ତୁ status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_SIZE(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	ଫେରନ୍ତୁ status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
