/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#समावेश <sys/types.h>
#समावेश <pwd.h>
#समावेश <stdlib.h>
#समावेश <paths.h>
#समावेश <unistd.h>
#समावेश <sys/syslog.h>
#अगर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#समावेश "dl-econf.h"
#पूर्ण_अगर

#समावेश "closestream.h"
#समावेश "shells.h"

#अगर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(व्योम)
{
	econf_err error;
	econf_file *कुंजी_file = शून्य;

	अगर (ul_dlopen_libeconf() != 0)
		वापस शून्य;

	error = econf_call(econf_readDirs)(&कुंजी_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       शून्य,
			       "", /* key only */
			       "#" /* comment */);
	अगर (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		वापस शून्य;
	}

	वापस कुंजी_file;
}
#पूर्ण_अगर

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
बाह्य व्योम print_shells(खाता *out, स्थिर अक्षर *format)
{
#अगर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	माप_प्रकार आकार = 0;
	econf_err error;
	अक्षर **कुंजियाँ = शून्य;
	econf_file *कुंजी_file = open_etc_shells();

	अगर (!कुंजी_file)
	  वापस;

	error = econf_call(econf_get_कुंजियाँ)(कुंजी_file, शून्य, &आकार, &कुंजियाँ);
	अगर (error) {
		econf_call(econf_freeFile)(कुंजी_file);
		errx(निकास_त्रुटि,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	क्रम (माप_प्रकार i = 0; i < आकार; i++) {
		ख_लिखो(out, format, कुंजियाँ[i]);
	}
	econf_call(econf_freeArray)(कुंजियाँ);
	econf_call(econf_freeFile)(कुंजी_file);
#अन्यथा
	अक्षर *s;

	setusershell();
	जबतक ((s = getusershell()))
		ख_लिखो(out, format, s);
	endusershell();
#पूर्ण_अगर
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
बाह्य पूर्णांक is_known_shell(स्थिर अक्षर *shell_name)
{
	पूर्णांक ret = 0;

	अगर (!shell_name)
		वापस 0;

#अगर defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	अक्षर *val = शून्य;
	econf_err error;
	econf_file *कुंजी_file = open_etc_shells();

	अगर (!कुंजी_file)
		वापस 0;

	error = econf_call(econf_get_String_मूल्य) (कुंजी_file, शून्य, shell_name, &val);
	अगर (error) {
		अगर (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} अन्यथा
		ret = 1;

	मुक्त(val);
	econf_call(econf_freeFile)(कुंजी_file);
#अन्यथा
	अक्षर *s;

	setusershell();
	जबतक ((s = getusershell())) {
		अगर (*s != '#' && म_भेद(shell_name, s) == 0) {
			ret = 1;
			अवरोध;
		}
	}
	endusershell();
#पूर्ण_अगर
	वापस ret;
}
