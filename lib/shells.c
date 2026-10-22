/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <paths.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/syslog.h>
#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#অন্তর্ভাব "dl-econf.h"
#endif

#অন্তর্ভাব "closestream.h"
#অন্তর্ভাব "shells.h"

#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(শূন্য)
{
	econf_err error;
	econf_file *চাবি_file = ফাঁকা;

	যদি (ul_dlopen_libeconf() != 0)
		ফেরত ফাঁকা;

	error = econf_call(econf_readDirs)(&চাবি_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       ফাঁকা,
			       "", /* key only */
			       "#" /* comment */);
	যদি (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ফেরত ফাঁকা;
	}

	ফেরত চাবি_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
বাহ্যিক শূন্য print_shells(FILE *out, ধ্রুবক অক্ষর *format)
{
#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t আকার_2 = 0;
	econf_err error;
	অক্ষর **চাবিগুলি = ফাঁকা;
	econf_file *চাবি_file = open_etc_shells();

	যদি (!চাবি_file)
	  ফেরত;

	error = econf_call(econf_get_চাবিগুলি)(চাবি_file, ফাঁকা, &আকার_2, &চাবিগুলি);
	যদি (error) {
		econf_call(econf_freeFile)(চাবি_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	জন্য (size_t i = 0; i < আকার_2; i++) {
		fprintf(out, format, চাবিগুলি[i]);
	}
	econf_call(econf_freeArray)(চাবিগুলি);
	econf_call(econf_freeFile)(চাবি_file);
#অন্যথা
	অক্ষর *s;

	setusershell();
	যখন ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
বাহ্যিক পূর্ণসংখ্যা is_known_shell(ধ্রুবক অক্ষর *shell_name)
{
	পূর্ণসংখ্যা ret = 0;

	যদি (!shell_name)
		ফেরত 0;

#যদি defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	অক্ষর *val = ফাঁকা;
	econf_err error;
	econf_file *চাবি_file = open_etc_shells();

	যদি (!চাবি_file)
		ফেরত 0;

	error = econf_call(econf_get_String_মান) (চাবি_file, ফাঁকা, shell_name, &val);
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
	অক্ষর *s;

	setusershell();
	যখন ((s = getusershell())) {
		যদি (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			বিরতি;
		}
	}
	endusershell();
#endif
	ফেরত ret;
}
