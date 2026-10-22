/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <dlfcn.h>

#ضمن "c.h"
#ضمن "dl-econf.h"

#إذا defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_ترجیح_RECOMMENDED,
		    "libeconf.so.0");

بنية ul_econf_opers ul_econf;

ساكن ثابت بنية ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_ملاؤ_Files ),
#جے_تعریف_شدہ HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#جے_انت
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_کلیدیں ),
	UL_DLSYM( ul_econf_opers, econf_get_String_قدر ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_قدر ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_قدر ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

صحيح ul_dlopen_libeconf(فراغ)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	ساكن صحيح حالت = 0;
	ساكن فراغ *dl = عدم;
	صحيح flags = RTLD_LAZY | RTLD_LOCAL;

	إذا (حالت)
		أرجع حالت > 0 ? 0 : -ENOSYS;

#جے_تعریف_شدہ RTLD_NODELETE
	flags |= RTLD_NODELETE;
#جے_انت
	حالت = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_ناپ(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	أرجع حالت > 0 ? 0 : -ENOSYS;
}

#جے_انت /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
