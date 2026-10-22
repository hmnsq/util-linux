/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <sys/types.h>
#ضمن <pwd.h>
#ضمن <stdlib.h>
#ضمن <paths.h>
#ضمن <unistd.h>
#ضمن <sys/syslog.h>
#אם defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ضمن "dl-econf.h"
#endif

#ضمن "closestream.h"
#ضمن "shells.h"

#אם defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(فراغ)
{
	econf_err error;
	econf_file *key_file = عدم;

	אם (ul_dlopen_libeconf() != 0)
		החזר عدم;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       عدم,
			       "", /* key only */
			       "#" /* comment */);
	אם (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		החזר عدم;
	}

	החזר key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
خارجي فراغ print_shells(FILE *out, ثابت حرف *format)
{
#אם defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	حرف **keys = عدم;
	econf_file *key_file = open_etc_shells();

	אם (!key_file)
	  החזר;

	error = econf_call(econf_getKeys)(key_file, عدم, &size, &keys);
	אם (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	עבור (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#אחרת
	حرف *s;

	setusershell();
	כלעוד ((s = getusershell()))
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

	אם (!shell_name)
		החזר 0;

#אם defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	حرف *val = عدم;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	אם (!key_file)
		החזר 0;

	error = econf_call(econf_getStringValue) (key_file, عدم, shell_name, &val);
	אם (error) {
		אם (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} אחרת
		ret = 1;

	حرر(val);
	econf_call(econf_freeFile)(key_file);
#אחרת
	حرف *s;

	setusershell();
	כלעוד ((s = getusershell())) {
		אם (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			اكسر;
		}
	}
	endusershell();
#endif
	החזר ret;
}
