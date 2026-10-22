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
#اگر_ختم

#شامل "closestream.h"
#شامل "shells.h"

#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(خالی)
{
	econf_err error;
	econf_file *کلید_file = تہی;

	اگر (ul_dlopen_libeconf() != 0)
		واپس تہی;

	error = econf_call(econf_readDirs)(&کلید_file,
			       راستہ_VENDORDIR,
			       "/etc",
			       "shells",
			       تہی,
			       "", /* key only */
			       "#" /* comment */);
	اگر (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		واپس تہی;
	}

	واپس کلید_file;
}
#اگر_ختم

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
خارجی خالی print_shells(FILE *برآمد, ثابت حرف *format)
{
#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t سائز = 0;
	econf_err error;
	حرف **کلیدیں = تہی;
	econf_file *کلید_file = open_etc_shells();

	اگر (!کلید_file)
	  واپس;

	error = econf_call(econf_get_کلیدیں)(کلید_file, تہی, &سائز, &کلیدیں);
	اگر (error) {
		econf_call(econf_freeFile)(کلید_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	برائے (size_t i = 0; i < سائز; i++) {
		fprintf(برآمد, format, کلیدیں[i]);
	}
	econf_call(econf_freeArray)(کلیدیں);
	econf_call(econf_freeFile)(کلید_file);
#ورنہ
	حرف *s;

	setusershell();
	جب ((s = getusershell()))
		fprintf(برآمد, format, s);
	endusershell();
#اگر_ختم
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
خارجی صحیح is_معلوم_shell(ثابت حرف *shell_name)
{
	صحیح ret = 0;

	اگر (!shell_name)
		واپس 0;

#اگر defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	حرف *val = تہی;
	econf_err error;
	econf_file *کلید_file = open_etc_shells();

	اگر (!کلید_file)
		واپس 0;

	error = econf_call(econf_get_String_قدر) (کلید_file, تہی, shell_name, &val);
	اگر (error) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ورنہ
		ret = 1;

	آزاد(val);
	econf_call(econf_freeFile)(کلید_file);
#ورنہ
	حرف *s;

	setusershell();
	جب ((s = getusershell())) {
		اگر (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			توڑو;
		}
	}
	endusershell();
#اگر_ختم
	واپس ret;
}
