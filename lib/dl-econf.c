/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <dlfcn.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "dl-econf.h"

#ఒకవేళ defined(HAVE_LIBECONF) && defined(USE_DLOPEN_ECONF)

UL_ELF_NOTE_DLOPEN("econf",
		    "Support for libeconf",
		    UL_ELF_NOTE_DLOPEN_ప్రాధాన్యత_RECOMMENDED,
		    "libeconf.so.0");

నిర్మాణం ul_econf_opers ul_econf;

స్థిర స్థిరరాశి నిర్మాణం ul_dlsym ul_econf_symbols[] =
{
	UL_DLSYM( ul_econf_opers, econf_readFile ),
	UL_DLSYM( ul_econf_opers, econf_విలీనం_Files ),
#నిర్వచితమైతే HAVE_ECONF_READCONFIG
	UL_DLSYM( ul_econf_opers, econf_readConfig ),
#అయితే_ముగింపు
	UL_DLSYM( ul_econf_opers, econf_readDirs ),
	UL_DLSYM( ul_econf_opers, econf_get_కీలకాలు ),
	UL_DLSYM( ul_econf_opers, econf_get_String_విలువ ),
	UL_DLSYM( ul_econf_opers, econf_get_Bool_విలువ ),
	UL_DLSYM( ul_econf_opers, econf_get_UInt64_విలువ ),
	UL_DLSYM( ul_econf_opers, econf_errString ),

	UL_DLSYM( ul_econf_opers, econf_freeFile ),
	UL_DLSYM( ul_econf_opers, econf_freeArray ),
};

పూర్ణాంకం ul_dlopen_libeconf(శూన్య)
{
	/* 0 = not tried, 1 = loaded, -1 = failed */
	స్థిర పూర్ణాంకం స్థితి_2 = 0;
	స్థిర శూన్య *dl = ఖాళీ;
	పూర్ణాంకం flags = RTLD_LAZY | RTLD_LOCAL;

	ఒకవేళ (స్థితి_2)
		తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;

#నిర్వచితమైతే RTLD_NODELETE
	flags |= RTLD_NODELETE;
#అయితే_ముగింపు
	స్థితి_2 = ul_dlopen_symbols("libeconf.so.0", flags,
				   ul_econf_symbols,
				   ARRAY_కొలత(ul_econf_symbols),
				   &ul_econf, &dl) == 0 ? 1 : -1;

	తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;
}

#అయితే_ముగింపు /* HAVE_LIBECONF && USE_DLOPEN_ECONF */
