/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#شامل <sys/types.h>
#شامل <pwd.h>
#شامل <stdlib.h>
#شامل <paths.h>
#شامل <unistd.h>
#شامل <sys/syslog.h>
#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#شامل "dl-econf.h"
#endif

#شامل "closestream.h"
#شامل "shells.h"

#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(تهی)
{
	econf_err error;
	econf_file *کلید_file = پوچ;

	اگر (ul_dlopen_libeconf() != 0)
		بازگشت پوچ;

	error = econf_call(econf_readDirs)(&کلید_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       پوچ,
			       "", /* key only */
			       "#" /* comment */);
	اگر (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		بازگشت پوچ;
	}

	بازگشت کلید_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
بیرونی تهی print_shells(FILE *out, ثابت نویسه *format)
{
#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	نویسه **کلیدها = پوچ;
	econf_file *کلید_file = open_etc_shells();

	اگر (!کلید_file)
	  بازگشت;

	error = econf_call(econf_get_کلیدها)(کلید_file, پوچ, &size, &کلیدها);
	اگر (error) {
		econf_call(econf_freeFile)(کلید_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	برای (size_t i = 0; i < size; i++) {
		fprintf(out, format, کلیدها[i]);
	}
	econf_call(econf_freeArray)(کلیدها);
	econf_call(econf_freeFile)(کلید_file);
#وگرنه
	نویسه *s;

	setusershell();
	وقتی ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
بیرونی صحیح is_known_shell(ثابت نویسه *shell_name)
{
	صحیح ret = 0;

	اگر (!shell_name)
		بازگشت 0;

#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	نویسه *val = پوچ;
	econf_err error;
	econf_file *کلید_file = open_etc_shells();

	اگر (!کلید_file)
		بازگشت 0;

	error = econf_call(econf_get_String_مقدار) (کلید_file, پوچ, shell_name, &val);
	اگر (error) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} وگرنه
		ret = 1;

	آزاد(val);
	econf_call(econf_freeFile)(کلید_file);
#وگرنه
	نویسه *s;

	setusershell();
	وقتی ((s = getusershell())) {
		اگر (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			بشکن;
		}
	}
	endusershell();
#endif
	بازگشت ret;
}
