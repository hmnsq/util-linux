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
		    UL_ELF_NOTE_DLOPEN_اولویت_SUGGESTED,
		    "libcryptsetup.so.12");

بنية ul_cryptsetup_opers ul_cryptsetup;

ساكن ثابت بنية ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_مجموعه_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_مجموعه_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_داده_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_بارگیری ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_کلید_اندازه ),
#اگر_تعریف_شده HAVE_CRYPT_ACTIVATE_BY_SIGNED_کلید
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_کلید ),
#پایان_اگر
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_کلید ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_کلید_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

صحيح ul_dlopen_libcryptsetup(فراغ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساكن صحيح وضعیت = 0;
	ساكن فراغ *dl = عدم;
	صحيح flags = RTLD_LAZY | RTLD_LOCAL;

	إذا (وضعیت)
		أرجع وضعیت > 0 ? 0 : -ENOSYS;

#اگر_تعریف_شده RTLD_NODELETE
	flags |= RTLD_NODELETE;
#پایان_اگر
#اگر_تعریف_شده RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#پایان_اگر
	وضعیت = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_اندازه(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	أرجع وضعیت > 0 ? 0 : -ENOSYS;
}

#پایان_اگر /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
