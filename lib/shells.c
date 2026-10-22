/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <paths.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/syslog.h>
#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#অন্তর্ভাৱ "dl-econf.h"
#endif

#অন্তর্ভাৱ "closestream.h"
#অন্তর্ভাৱ "shells.h"

#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(শূন্য)
{
	econf_err error;
	econf_file *চাবি_file = রিক্ত;

	যদি (ul_dlopen_libeconf() != 0)
		ঘূৰাই রিক্ত;

	error = econf_call(econf_readDirs)(&চাবি_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       রিক্ত,
			       "", /* key only */
			       "#" /* comment */);
	যদি (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই চাবি_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
বাহ্য শূন্য print_shells(FILE *out, ধ্রুৱ ৱর্ণ *format)
{
#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	ৱর্ণ **চাবিগুলি = রিক্ত;
	econf_file *চাবি_file = open_etc_shells();

	যদি (!চাবি_file)
	  ঘূৰাই;

	error = econf_call(econf_get_চাবিগুলি)(চাবি_file, রিক্ত, &size, &চাবিগুলি);
	যদি (error) {
		econf_call(econf_freeFile)(চাবি_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	বাবে (size_t i = 0; i < size; i++) {
		fprintf(out, format, চাবিগুলি[i]);
	}
	econf_call(econf_freeArray)(চাবিগুলি);
	econf_call(econf_freeFile)(চাবি_file);
#অন্যথা
	ৱর্ণ *s;

	setusershell();
	যেতিয়ালৈ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
বাহ্য পূর্ণাঙ্ক is_known_shell(ধ্রুৱ ৱর্ণ *shell_name)
{
	পূর্ণাঙ্ক ret = 0;

	যদি (!shell_name)
		ঘূৰাই 0;

#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	ৱর্ণ *val = রিক্ত;
	econf_err error;
	econf_file *চাবি_file = open_etc_shells();

	যদি (!চাবি_file)
		ঘূৰাই 0;

	error = econf_call(econf_get_String_মান) (চাবি_file, রিক্ত, shell_name, &val);
	যদি (error) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} অন্যথা
		ret = 1;

	মুক্ত(val);
	econf_call(econf_freeFile)(চাবি_file);
#অন্যথা
	ৱর্ণ *s;

	setusershell();
	যেতিয়ালৈ ((s = getusershell())) {
		যদি (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ভঙ্গ;
		}
	}
	endusershell();
#endif
	ঘূৰাই ret;
}
