/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "dl-selinux.h"

#ਜੇ defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_ਤਰਜੀਹ_RECOMMENDED,
		    "libselinux.so.1");

ਬਣਤਰ ul_selinux_opers ul_selinux;

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_dlsym ul_selinux_symbols[] =
{
	UL_DLSYM( ul_selinux_opers, is_selinux_enabled ),

	UL_DLSYM( ul_selinux_opers, getfilecon ),
	UL_DLSYM( ul_selinux_opers, getfilecon_raw ),
	UL_DLSYM( ul_selinux_opers, fgetfilecon ),
	UL_DLSYM( ul_selinux_opers, lgetfilecon ),

	UL_DLSYM( ul_selinux_opers, getcon ),
	UL_DLSYM( ul_selinux_opers, getprevcon ),
	UL_DLSYM( ul_selinux_opers, getpidcon ),

	UL_DLSYM( ul_selinux_opers, getseuserbyname ),

	UL_DLSYM( ul_selinux_opers, setfilecon ),
	UL_DLSYM( ul_selinux_opers, fsetfilecon ),
	UL_DLSYM( ul_selinux_opers, setfscreatecon ),
	UL_DLSYM( ul_selinux_opers, setexeccon ),

	UL_DLSYM( ul_selinux_opers, freecon ),

	UL_DLSYM( ul_selinux_opers, selinux_check_access ),
	UL_DLSYM( ul_selinux_opers, security_compute_relabel ),
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#ਜੇ_ਅੰਤ
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_ਮੰਜ਼ਿਲ_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_ਮੰਜ਼ਿਲ_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_ਸੈੱਟ ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

ਪੂਰਨ ul_dlopen_libselinux(ਖਾਲੀ)
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
	ਸਥਿਤੀ = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_ਮਾਪ(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	ਵਾਪਸ ਸਥਿਤੀ > 0 ? 0 : -ENOSYS;
}

#ਜੇ_ਅੰਤ /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
