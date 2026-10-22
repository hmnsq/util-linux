/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <dlfcn.h>

#include "c.h"
#include "dl-cryptsetup.h"

#만약 defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_PRIORITY_SUGGESTED,
		    "libcryptsetup.so.12");

struct ul_cryptsetup_opers ul_cryptsetup;

static const struct ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_set_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_data_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_load ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_key_size ),
#ifdef HAVE_CRYPT_ACTIVATE_BY_SIGNED_KEY
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_key ),
#endif
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_key ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_key_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

정수 ul_dlopen_libcryptsetup(void)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	static 정수 status = 0;
	static void *dl = NULL;
	정수 flags = RTLD_LAZY | RTLD_LOCAL;

	만약 (status)
		반환 status > 0 ? 0 : -ENOSYS;

#ifdef RTLD_NODELETE
	flags |= RTLD_NODELETE;
#endif
#ifdef RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#endif
	status = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_SIZE(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	반환 status > 0 ? 0 : -ENOSYS;
}

#endif /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
