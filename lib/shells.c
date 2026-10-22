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
#endif

#अन्तर्भाव "closestream.h"
#अन्तर्भाव "shells.h"

#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(शून्य)
{
	econf_err error;
	econf_file *साँचो_file = रिक्त;

	यदि (ul_dlopen_libeconf() != 0)
		फर्काऊ रिक्त;

	error = econf_call(econf_readDirs)(&साँचो_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       रिक्त,
			       "", /* key only */
			       "#" /* comment */);
	यदि (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		फर्काऊ रिक्त;
	}

	फर्काऊ साँचो_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
बाह्य शून्य print_shells(खाता *out, अचर वर्ण *format)
{
#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	माप_प्रकार size = 0;
	econf_err error;
	वर्ण **साँचोहरू = रिक्त;
	econf_file *साँचो_file = open_etc_shells();

	यदि (!साँचो_file)
	  फर्काऊ;

	error = econf_call(econf_get_साँचोहरू)(साँचो_file, रिक्त, &size, &साँचोहरू);
	यदि (error) {
		econf_call(econf_freeFile)(साँचो_file);
		errx(निकास_त्रुटि,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	लागि (माप_प्रकार i = 0; i < size; i++) {
		ख_लिखो(out, format, साँचोहरू[i]);
	}
	econf_call(econf_freeArray)(साँचोहरू);
	econf_call(econf_freeFile)(साँचो_file);
#अन्यथा
	वर्ण *s;

	setusershell();
	जबसम्म ((s = getusershell()))
		ख_लिखो(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
बाह्य पूर्णाङ्क is_known_shell(अचर वर्ण *shell_name)
{
	पूर्णाङ्क ret = 0;

	यदि (!shell_name)
		फर्काऊ 0;

#यदि defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	वर्ण *val = रिक्त;
	econf_err error;
	econf_file *साँचो_file = open_etc_shells();

	यदि (!साँचो_file)
		फर्काऊ 0;

	error = econf_call(econf_get_String_मान) (साँचो_file, रिक्त, shell_name, &val);
	यदि (error) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} अन्यथा
		ret = 1;

	मुक्त(val);
	econf_call(econf_freeFile)(साँचो_file);
#अन्यथा
	वर्ण *s;

	setusershell();
	जबसम्म ((s = getusershell())) {
		यदि (*s != '#' && म_भेद(shell_name, s) == 0) {
			ret = 1;
			रोक;
		}
	}
	endusershell();
#endif
	फर्काऊ ret;
}
