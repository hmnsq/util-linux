/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ضمن <sys/types.h>
#ضمن <pwd.h>
#ضمن <stdlib.h>
#ضمن <paths.h>
#ضمن <unistd.h>
#ضمن <sys/syslog.h>
#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ضمن "dl-econf.h"
#د_که_پای

#ضمن "closestream.h"
#ضمن "shells.h"

#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(فراغ)
{
	econf_err error;
	econf_file *کلی_file = عدم;

	إذا (ul_dlopen_libeconf() != 0)
		أرجع عدم;

	error = econf_call(econf_readDirs)(&کلی_file,
			       لار_VENDORDIR,
			       "/etc",
			       "shells",
			       عدم,
			       "", /* key only */
			       "#" /* comment */);
	إذا (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		أرجع عدم;
	}

	أرجع کلی_file;
}
#د_که_پای

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
خارجي فراغ print_shells(FILE *بهر, ثابت حرف *format)
{
#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t اندازه = 0;
	econf_err error;
	حرف **کلیګانې = عدم;
	econf_file *کلی_file = open_etc_shells();

	إذا (!کلی_file)
	  أرجع;

	error = econf_call(econf_get_کلیګانې)(کلی_file, عدم, &اندازه, &کلیګانې);
	إذا (error) {
		econf_call(econf_freeFile)(کلی_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	لكل (size_t i = 0; i < اندازه; i++) {
		fprintf(بهر, format, کلیګانې[i]);
	}
	econf_call(econf_freeArray)(کلیګانې);
	econf_call(econf_freeFile)(کلی_file);
#وإلا
	حرف *s;

	setusershell();
	طالما ((s = getusershell()))
		fprintf(بهر, format, s);
	endusershell();
#د_که_پای
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
خارجي صحيح is_معلوم_shell(ثابت حرف *shell_name)
{
	صحيح ret = 0;

	إذا (!shell_name)
		أرجع 0;

#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	حرف *val = عدم;
	econf_err error;
	econf_file *کلی_file = open_etc_shells();

	إذا (!کلی_file)
		أرجع 0;

	error = econf_call(econf_get_String_ارزښت) (کلی_file, عدم, shell_name, &val);
	إذا (error) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} وإلا
		ret = 1;

	حرر(val);
	econf_call(econf_freeFile)(کلی_file);
#وإلا
	حرف *s;

	setusershell();
	طالما ((s = getusershell())) {
		إذا (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			اكسر;
		}
	}
	endusershell();
#د_که_پای
	أرجع ret;
}
