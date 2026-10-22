/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <paths.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/syslog.h>
#ಯದಿ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ಅನ್ತರ್ಭಾವ "dl-econf.h"
#endif

#ಅನ್ತರ್ಭಾವ "closestream.h"
#ಅನ್ತರ್ಭಾವ "shells.h"

#ಯದಿ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(ಶೂನ್ಯ)
{
	econf_err error;
	econf_file *ಕೀಲಿ_file = ರಿಕ್ತ;

	ಯದಿ (ul_dlopen_libeconf() != 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	error = econf_call(econf_readDirs)(&ಕೀಲಿ_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       ರಿಕ್ತ,
			       "", /* key only */
			       "#" /* comment */);
	ಯದಿ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
ಬಾಹ್ಯ ಶೂನ್ಯ print_shells(FILE *out, ಧ್ರುವ ವರ್ಣ *format)
{
#ಯದಿ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	ವರ್ಣ **ಕೀಲಿಗಳು = ರಿಕ್ತ;
	econf_file *ಕೀಲಿ_file = open_etc_shells();

	ಯದಿ (!ಕೀಲಿ_file)
	  ಪ್ರತ್ಯಾಗಮ;

	error = econf_call(econf_get_ಕೀಲಿಗಳು)(ಕೀಲಿ_file, ರಿಕ್ತ, &size, &ಕೀಲಿಗಳು);
	ಯದಿ (error) {
		econf_call(econf_freeFile)(ಕೀಲಿ_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	ಪ್ರತಿ (size_t i = 0; i < size; i++) {
		fprintf(out, format, ಕೀಲಿಗಳು[i]);
	}
	econf_call(econf_freeArray)(ಕೀಲಿಗಳು);
	econf_call(econf_freeFile)(ಕೀಲಿ_file);
#ಅನ್ಯಥಾ
	ವರ್ಣ *s;

	setusershell();
	ಯಾವತ್ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ is_known_shell(ಧ್ರುವ ವರ್ಣ *shell_name)
{
	ಪೂರ್ಣಾಙ್ಕ ret = 0;

	ಯದಿ (!shell_name)
		ಪ್ರತ್ಯಾಗಮ 0;

#ಯದಿ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	ವರ್ಣ *val = ರಿಕ್ತ;
	econf_err error;
	econf_file *ಕೀಲಿ_file = open_etc_shells();

	ಯದಿ (!ಕೀಲಿ_file)
		ಪ್ರತ್ಯಾಗಮ 0;

	error = econf_call(econf_get_String_ಮೌಲ್ಯ) (ಕೀಲಿ_file, ರಿಕ್ತ, shell_name, &val);
	ಯದಿ (error) {
		ಯದಿ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ಅನ್ಯಥಾ
		ret = 1;

	ಮುಕ್ತ(val);
	econf_call(econf_freeFile)(ಕೀಲಿ_file);
#ಅನ್ಯಥಾ
	ವರ್ಣ *s;

	setusershell();
	ಯಾವತ್ ((s = getusershell())) {
		ಯದಿ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ಭಙ್ಗ;
		}
	}
	endusershell();
#endif
	ಪ್ರತ್ಯಾಗಮ ret;
}
