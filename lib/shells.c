/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <paths.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/syslog.h>
#जर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#अन्तर्भाव "dl-econf.h"
#endif

#अन्तर्भाव "closestream.h"
#अन्तर्भाव "shells.h"

#जर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(शून्य)
{
	econf_err error;
	econf_file *किल्ली_file = रिक्त;

	जर (ul_dlopen_libeconf() != 0)
		परत रिक्त;

	error = econf_call(econf_readDirs)(&किल्ली_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       रिक्त,
			       "", /* key only */
			       "#" /* comment */);
	जर (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		परत रिक्त;
	}

	परत किल्ली_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
बाह्य शून्य print_shells(खाता *out, स्थिरांक अक्षर *format)
{
#जर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	माप_प्रकार size = 0;
	econf_err error;
	अक्षर **किल्ल्या = रिक्त;
	econf_file *किल्ली_file = open_etc_shells();

	जर (!किल्ली_file)
	  परत;

	error = econf_call(econf_get_किल्ल्या)(किल्ली_file, रिक्त, &size, &किल्ल्या);
	जर (error) {
		econf_call(econf_freeFile)(किल्ली_file);
		errx(निकास_त्रुटि,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	साठी (माप_प्रकार i = 0; i < size; i++) {
		ख_लिखो(out, format, किल्ल्या[i]);
	}
	econf_call(econf_freeArray)(किल्ल्या);
	econf_call(econf_freeFile)(किल्ली_file);
#अन्यथा
	अक्षर *s;

	setusershell();
	पर्यंत ((s = getusershell()))
		ख_लिखो(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
बाह्य पूर्णांक is_known_shell(स्थिरांक अक्षर *shell_name)
{
	पूर्णांक ret = 0;

	जर (!shell_name)
		परत 0;

#जर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	अक्षर *val = रिक्त;
	econf_err error;
	econf_file *किल्ली_file = open_etc_shells();

	जर (!किल्ली_file)
		परत 0;

	error = econf_call(econf_get_String_मूल्य) (किल्ली_file, रिक्त, shell_name, &val);
	जर (error) {
		जर (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} अन्यथा
		ret = 1;

	मुक्त(val);
	econf_call(econf_freeFile)(किल्ली_file);
#अन्यथा
	अक्षर *s;

	setusershell();
	पर्यंत ((s = getusershell())) {
		जर (*s != '#' && म_भेद(shell_name, s) == 0) {
			ret = 1;
			थांबा;
		}
	}
	endusershell();
#endif
	परत ret;
}
