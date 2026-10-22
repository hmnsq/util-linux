/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <dlfcn.h>

#समावेश "c.h"
#समावेश "dl-selinux.h"

#अगर defined(HAVE_LIBSELINUX) && defined(USE_DLOPEN_SELINUX)

UL_ELF_NOTE_DLOPEN("selinux",
		    "Support for SELinux",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_RECOMMENDED,
		    "libselinux.so.1");

काष्ठा ul_selinux_opers ul_selinux;

अटल स्थिर काष्ठा ul_dlsym ul_selinux_symbols[] =
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
#अगर_घोषित HAVE_SECURITY_GET_INITIAL_CONTEXT
	UL_DLSYM( ul_selinux_opers, security_get_initial_context ),
#पूर्ण_अगर
	UL_DLSYM( ul_selinux_opers, selinux_file_context_cmp ),
	UL_DLSYM( ul_selinux_opers, selinux_trans_to_raw_context ),
	UL_DLSYM( ul_selinux_opers, string_to_security_class ),

	UL_DLSYM( ul_selinux_opers, get_default_context_with_level ),

	UL_DLSYM( ul_selinux_opers, selabel_open ),
	UL_DLSYM( ul_selinux_opers, selabel_lookup ),
	UL_DLSYM( ul_selinux_opers, selabel_close ),

	UL_DLSYM( ul_selinux_opers, context_new ),
	UL_DLSYM( ul_selinux_opers, context_type_set ),
	UL_DLSYM( ul_selinux_opers, context_str ),
	UL_DLSYM( ul_selinux_opers, context_free ),
};

पूर्णांक ul_dlopen_libselinux(व्योम)
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
	status = ul_dlopen_symbols("libselinux.so.1", flags,
				   ul_selinux_symbols,
				   ARRAY_आकार(ul_selinux_symbols),
				   &ul_selinux, &dl) == 0 ? 1 : -1;

	वापस status > 0 ? 0 : -ENOSYS;
}

#पूर्ण_अगर /* HAVE_LIBSELINUX && USE_DLOPEN_SELINUX */
