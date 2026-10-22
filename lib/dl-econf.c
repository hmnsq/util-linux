/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dl-econf.h"

#ਜੇ defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_ਤਰਜੀਹ_RECOMMENDED,
		    "libeconf.so.0");

ਬਣਤਰ ul_econf_opers ul_econf;

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_ਮਿਲਾਓ_Files ),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#ਜੇ_ਅੰਤ
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_ਕੁੰਜੀਆਂ ),
	UL_DLSYM( ul_econf_opers, econf_get_String_ਮੁੱਲ ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_ਮੁੱਲ ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_ਮੁੱਲ ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

ਪੂਰਨ ul_dlopen_libeconf(ਖਾਲੀ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ਸਥਿਰ ਪੂਰਨ ਸਥਿਤੀ = 0;
	ਸਥਿਰ ਖਾਲੀ *dl = ਨਿਰਾ;
	ਪੂਰਨ flags = RTLD_LAZY | RTLD_LOCAL;

	ਜੇ (ਸਥਿਤੀ)
		ਵਾਪਸ ਸਥਿਤੀ > 0 ? 0 : -ENOSYS;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#ਜੇ_ਅੰਤ
	ਸਥਿਤੀ = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_ਮਾਪ(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	ਵਾਪਸ ਸਥਿਤੀ > 0 ? 0 : -ENOSYS;
}

#ਜੇ_ਅੰਤ /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
