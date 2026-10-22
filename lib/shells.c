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
#پایان_اگر

#ضمن "closestream.h"
#ضمن "shells.h"

#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(فراغ)
{
	econf_err error;
	econf_file *کلید_file = عدم;

	إذا (ul_dlopen_libeconf() != 0)
		أرجع عدم;

	error = econf_call(econf_readDirs)(&کلید_file,
			       مسیر_VENDORDIR,
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

	أرجع کلید_file;
}
#پایان_اگر

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
خارجي فراغ print_shells(FILE *بیرون, ثابت حرف *format)
{
#إذا defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t اندازه = 0;
	econf_err error;
	حرف **کلیدها = عدم;
	econf_file *کلید_file = open_etc_shells();

	إذا (!کلید_file)
	  أرجع;

	error = econf_call(econf_get_کلیدها)(کلید_file, عدم, &اندازه, &کلیدها);
	إذا (error) {
		econf_call(econf_freeFile)(کلید_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	لكل (size_t i = 0; i < اندازه; i++) {
		fprintf(بیرون, format, کلیدها[i]);
	}
	econf_call(econf_freeArray)(کلیدها);
	econf_call(econf_freeFile)(کلید_file);
#وإلا
	حرف *s;

	setusershell();
	طالما ((s = getusershell()))
		fprintf(بیرون, format, s);
	endusershell();
#پایان_اگر
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
	econf_file *کلید_file = open_etc_shells();

	إذا (!کلید_file)
		أرجع 0;

	error = econf_call(econf_get_String_مقدار) (کلید_file, عدم, shell_name, &val);
	إذا (error) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} وإلا
		ret = 1;

	حرر(val);
	econf_call(econf_freeFile)(کلید_file);
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
#پایان_اگر
	أرجع ret;
}
