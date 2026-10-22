/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <dlfcn.h>

#شامل "c.h"
#شامل "dl-cryptsetup.h"

#اگر defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_ترجیح_SUGGESTED,
		    "libcryptsetup.so.12");

ساخت ul_cryptsetup_opers ul_cryptsetup;

ساکن ثابت ساخت ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_سیٹ_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_سیٹ_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_ڈیٹا_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_بھرو ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_کلید_سائز ),
#اگر_تعریف_شدہ HAVE_CRYPT_ACTIVATE_BY_SIGNED_کلید
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_کلید ),
#اگر_ختم
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_کلید ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_کلید_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

صحیح ul_dlopen_libcryptsetup(خالی)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساکن صحیح حالت_2 = 0;
	ساکن خالی *dl = تہی;
	صحیح flags = RTLD_LAZY | RTLD_LOCAL;

	اگر (حالت_2)
		واپس حالت_2 > 0 ? 0 : -ENOSYS;

#اگر_تعریف_شدہ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#اگر_ختم
#اگر_تعریف_شدہ RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#اگر_ختم
	حالت_2 = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_سائز(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	واپس حالت_2 > 0 ? 0 : -ENOSYS;
}

#اگر_ختم /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
