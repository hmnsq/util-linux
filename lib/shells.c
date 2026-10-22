/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <paths.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/syslog.h>
#ಆದರೆ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ಅನ್ತರ್ಭಾವ "dl-econf.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "closestream.h"
#ಅನ್ತರ್ಭಾವ "shells.h"

#ಆದರೆ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(ಶೂನ್ಯ)
{
	econf_err error;
	econf_file *ಕೀಲಿ_file = ಖಾಲಿ;

	ಆದರೆ (ul_dlopen_libeconf() != 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	error = econf_call(econf_readDirs)(&ಕೀಲಿ_file,
			       ಮಾರ್ಗ_VENDORDIR,
			       "/etc",
			       "shells",
			       ಖಾಲಿ,
			       "", /* key only */
			       "#" /* comment */);
	ಆದರೆ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ ಕೀಲಿ_file;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
ಬಾಹ್ಯ ಶೂನ್ಯ print_shells(FILE *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format)
{
#ಆದರೆ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t ಗಾತ್ರ_2 = 0;
	econf_err error;
	ಅಕ್ಷರ **ಕೀಲಿಗಳು = ಖಾಲಿ;
	econf_file *ಕೀಲಿ_file = open_etc_shells();

	ಆದರೆ (!ಕೀಲಿ_file)
	  ಹಿಂತಿರುಗಿ;

	error = econf_call(econf_get_ಕೀಲಿಗಳು)(ಕೀಲಿ_file, ಖಾಲಿ, &ಗಾತ್ರ_2, &ಕೀಲಿಗಳು);
	ಆದರೆ (error) {
		econf_call(econf_freeFile)(ಕೀಲಿ_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	ಗಾಗಿ (size_t i = 0; i < ಗಾತ್ರ_2; i++) {
		fprintf(ಹೊರಹೋಗು, format, ಕೀಲಿಗಳು[i]);
	}
	econf_call(econf_freeArray)(ಕೀಲಿಗಳು);
	econf_call(econf_freeFile)(ಕೀಲಿ_file);
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *s;

	setusershell();
	ಆಗ ((s = getusershell()))
		fprintf(ಹೊರಹೋಗು, format, s);
	endusershell();
#ಆದರೆ_ಅಂತ್ಯ
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ is_ತಿಳಿದ_shell(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell_name)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

	ಆದರೆ (!shell_name)
		ಹಿಂತಿರುಗಿ 0;

#ಆದರೆ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	ಅಕ್ಷರ *val = ಖಾಲಿ;
	econf_err error;
	econf_file *ಕೀಲಿ_file = open_etc_shells();

	ಆದರೆ (!ಕೀಲಿ_file)
		ಹಿಂತಿರುಗಿ 0;

	error = econf_call(econf_get_String_ಮೌಲ್ಯ) (ಕೀಲಿ_file, ಖಾಲಿ, shell_name, &val);
	ಆದರೆ (error) {
		ಆದರೆ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ಇಲ್ಲದಿದ್ದರೆ
		ret = 1;

	ಬಿಡುಗಡೆ(val);
	econf_call(econf_freeFile)(ಕೀಲಿ_file);
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *s;

	setusershell();
	ಆಗ ((s = getusershell())) {
		ಆದರೆ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ನಿಲ್ಲಿಸು;
		}
	}
	endusershell();
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ret;
}
