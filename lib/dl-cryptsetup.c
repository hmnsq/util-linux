/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <dlfcn.h>

#ضمن "c.h"
#ضمن "dl-cryptsetup.h"

#إذا defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_ترجيح_SUGGESTED,
		    "libcryptsetup.so.12");

بنية ul_cryptsetup_opers ul_cryptsetup;

ساكن ثابت بنية ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_سيٽ_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_سيٽ_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_معلومات_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ڀريو ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_ڪنجي_سائيز ),
#جيڪڏهن_وصف_ٿيل HAVE_CRYPT_ACTIVATE_BY_SIGNED_ڪنجي
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_ڪنجي ),
#جيڪڏهن_پورو
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_ڪنجي ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_ڪنجي_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

صحيح ul_dlopen_libcryptsetup(فراغ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساكن صحيح حالت = 0;
	ساكن فراغ *dl = عدم;
	صحيح flags = RTLD_LAZY | RTLD_LOCAL;

	إذا (حالت)
		أرجع حالت > 0 ? 0 : -ENOSYS;

#جيڪڏهن_وصف_ٿيل RTLD_NODELETE
	flags |= RTLD_NODELETE;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#جيڪڏهن_پورو
	حالت = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_سائيز(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	أرجع حالت > 0 ? 0 : -ENOSYS;
}

#جيڪڏهن_پورو /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
