/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <paths.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/syslog.h>
#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#अन्तर्भाव "dl-econf.h"
#यदि_अन्तः

#अन्तर्भाव "closestream.h"
#अन्तर्भाव "shells.h"

#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(शून्य)
{
	econf_err error;
	econf_file *कुञ्चिका_file = रिक्त;

	यदि (ul_dlopen_libeconf() != 0)
		प्रत्यागम रिक्त;

	error = econf_call(econf_readDirs)(&कुञ्चिका_file,
			       मार्गः_VENDORDIR,
			       "/etc",
			       "shells",
			       रिक्त,
			       "", /* key only */
			       "#" /* comment */);
	यदि (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		प्रत्यागम रिक्त;
	}

	प्रत्यागम कुञ्चिका_file;
}
#यदि_अन्तः

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
बाह्य शून्य print_shells(FILE *बहिर्गमनम्, ध्रुव वर्ण *format)
{
#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t आकार = 0;
	econf_err error;
	वर्ण **कुञ्चिकाः = रिक्त;
	econf_file *कुञ्चिका_file = open_etc_shells();

	यदि (!कुञ्चिका_file)
	  प्रत्यागम;

	error = econf_call(econf_get_कुञ्चिकाः)(कुञ्चिका_file, रिक्त, &आकार, &कुञ्चिकाः);
	यदि (error) {
		econf_call(econf_freeFile)(कुञ्चिका_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	प्रति (size_t i = 0; i < आकार; i++) {
		fprintf(बहिर्गमनम्, format, कुञ्चिकाः[i]);
	}
	econf_call(econf_freeArray)(कुञ्चिकाः);
	econf_call(econf_freeFile)(कुञ्चिका_file);
#अन्यथा
	वर्ण *s;

	setusershell();
	यावत् ((s = getusershell()))
		fprintf(बहिर्गमनम्, format, s);
	endusershell();
#यदि_अन्तः
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
बाह्य पूर्णाङ्क is_ज्ञातम्_shell(ध्रुव वर्ण *shell_name)
{
	पूर्णाङ्क ret = 0;

	यदि (!shell_name)
		प्रत्यागम 0;

#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	वर्ण *val = रिक्त;
	econf_err error;
	econf_file *कुञ्चिका_file = open_etc_shells();

	यदि (!कुञ्चिका_file)
		प्रत्यागम 0;

	error = econf_call(econf_get_String_मानम्) (कुञ्चिका_file, रिक्त, shell_name, &val);
	यदि (error) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} अन्यथा
		ret = 1;

	मुक्त(val);
	econf_call(econf_freeFile)(कुञ्चिका_file);
#अन्यथा
	वर्ण *s;

	setusershell();
	यावत् ((s = getusershell())) {
		यदि (*s != '#' && वाक्यं_तुलय(shell_name, s) == 0) {
			ret = 1;
			भङ्ग;
		}
	}
	endusershell();
#यदि_अन्तः
	प्रत्यागम ret;
}
