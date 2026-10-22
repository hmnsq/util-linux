/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <dlfcn.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "dl-econf.h"

#যদি defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libeconf.so.0");

কাঠামো ul_econf_opers ul_econf;

স্থির ধ্রুবক কাঠামো ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_mergeFiles ),
#ifdef HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#endif
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_চাবিগুলি ),
	UL_DLSYM( ul_econf_opers, econf_get_String_মান ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_মান ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_মান ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

পূর্ণসংখ্যা ul_dlopen_libeconf(শূন্য)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	স্থির পূর্ণসংখ্যা status = 0;
	স্থির শূন্য *dl = ফাঁকা;
	পূর্ণসংখ্যা flags = RTLD_LAZY | RTLD_LOCAL;

	যদি (status)
		ফেরত status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
	status = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_আকার(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	ফেরত status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
