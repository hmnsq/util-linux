/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <dlfcn.h>

#شامل "c.h"
#شامل "dl-econf.h"

#اگر defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_ترجیح_RECOMMENDED,
		    "libeconf.so.0");

ساخت ul_econf_opers ul_econf;

ساکن ثابت ساخت ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_ملاؤ_Files ),
#اگر_تعریف_شدہ HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#اگر_ختم
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_کلیدیں ),
	UL_DLSYM( ul_econf_opers, econf_get_String_قدر ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_قدر ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_قدر ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

صحیح ul_dlopen_libeconf(خالی)
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
	حالت_2 = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_سائز(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	واپس حالت_2 > 0 ? 0 : -ENOSYS;
}

#اگر_ختم /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
