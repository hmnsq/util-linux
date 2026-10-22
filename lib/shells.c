/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <sys/types.h>
#ضمن <pwd.h>
#ضمن <stdlib.h>
#ضمن <paths.h>
#ضمن <unistd.h>
#ضمن <sys/syslog.h>
#ܐܢ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ضمن "dl-econf.h"
#endif

#ضمن "closestream.h"
#ضمن "shells.h"

#ܐܢ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(فراغ)
{
	econf_err error;
	econf_file *key_file = عدم;

	ܐܢ (ul_dlopen_libeconf() != 0)
		ܦܢܐ عدم;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       عدم,
			       "", /* key only */
			       "#" /* comment */);
	ܐܢ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ܦܢܐ عدم;
	}

	ܦܢܐ key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
خارجي فراغ print_shells(FILE *out, ثابت حرف *format)
{
#ܐܢ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	حرف **keys = عدم;
	econf_file *key_file = open_etc_shells();

	ܐܢ (!key_file)
	  ܦܢܐ;

	error = econf_call(econf_getKeys)(key_file, عدم, &size, &keys);
	ܐܢ (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	لكل (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#ܐܚܪܢܐ
	حرف *s;

	setusershell();
	ܟܡܐ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
خارجي صحيح is_known_shell(ثابت حرف *shell_name)
{
	صحيح ret = 0;

	ܐܢ (!shell_name)
		ܦܢܐ 0;

#ܐܢ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	حرف *val = عدم;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	ܐܢ (!key_file)
		ܦܢܐ 0;

	error = econf_call(econf_getStringValue) (key_file, عدم, shell_name, &val);
	ܐܢ (error) {
		ܐܢ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ܐܚܪܢܐ
		ret = 1;

	حرر(val);
	econf_call(econf_freeFile)(key_file);
#ܐܚܪܢܐ
	حرف *s;

	setusershell();
	ܟܡܐ ((s = getusershell())) {
		ܐܢ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			اكسر;
		}
	}
	endusershell();
#endif
	ܦܢܐ ret;
}
