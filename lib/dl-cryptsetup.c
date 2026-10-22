/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <dlfcn.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "dl-cryptsetup.h"

#ఒకవేళ defined(HAVE_CRYPTSETUP) && defined(USE_DLOPEN_CRYPTSETUP)

UL_ELF_NOTE_DLOPEN("cryptsetup",
		    "Support for dm-verity devices",
		    UL_ELF_NOTE_DLOPEN_ప్రాధాన్యత_SUGGESTED,
		    "libcryptsetup.so.12");

నిర్మాణం ul_cryptsetup_opers ul_cryptsetup;

స్థిర స్థిరరాశి నిర్మాణం ul_dlsym ul_cryptsetup_symbols[] =
{
	UL_DLSYM( ul_cryptsetup_opers, crypt_సమితి_debug_level ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_సమితి_log_callback ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_దత్తాంశం_device ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_ఎక్కించు ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_volume_కీలకం_కొలత ),
#నిర్వచితమైతే HAVE_CRYPT_ACTIVATE_BY_SIGNED_కీలకం
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_signed_కీలకం ),
#అయితే_ముగింపు
	UL_DLSYM( ul_cryptsetup_opers, crypt_activate_by_volume_కీలకం ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_free ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_init_by_name ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_get_verity_info ),
	UL_DLSYM( ul_cryptsetup_opers, crypt_volume_కీలకం_get ),

	UL_DLSYM( ul_cryptsetup_opers, crypt_deactivate_by_name ),
};

పూర్ణాంకం ul_dlopen_libcryptsetup(శూన్య)
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
#నిర్వచితమైతే RTLD_DEEPBIND
	flags |= RTLD_DEEPBIND;
#అయితే_ముగింపు
	స్థితి_2 = ul_dlopen_symbols("libcryptsetup.so.12", flags,
				   ul_cryptsetup_symbols,
				   ARRAY_కొలత(ul_cryptsetup_symbols),
				   &ul_cryptsetup, &dl) == 0 ? 1 : -1;

	తిరిగి స్థితి_2 > 0 ? 0 : -ENOSYS;
}

#అయితే_ముగింపు /* HAVE_CRYPTSETUP && USE_DLOPEN_CRYPTSETUP */
