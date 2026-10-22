/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <dlfcn.h>

#समावेश "c.h"
#समावेश "dl-cryptsetup.h"

#अगर defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

काष्ठा ul_cryptsetup_opers ul_cryptsetup;

अटल स्थिर काष्ठा ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_तथ्य_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_कुंजी_आकार ),
#अगर_घोषित HAVE_CRYPT_ACTIVATE_BY_SIGNED_कुंजी
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_कुंजी ),
#पूर्ण_अगर
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_कुंजी ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_कुंजी_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

पूर्णांक ul_dlopen_libcryptsetup(व्योम)
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
#अगर_घोषित RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#पूर्ण_अगर
	status = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_आकार(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	वापस status > 0 ? 0 : -ENOSYS;
}

#पूर्ण_अगर /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
