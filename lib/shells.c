/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <paths.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/syslog.h>
#ఒకవేళ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#అన్తర్భావ "dl-econf.h"
#అయితే_ముగింపు

#అన్తర్భావ "closestream.h"
#అన్తర్భావ "shells.h"

#ఒకవేళ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(శూన్య)
{
	econf_err error;
	econf_file *కీలకం_file = ఖాళీ;

	ఒకవేళ (ul_dlopen_libeconf() != 0)
		తిరిగి ఖాళీ;

	error = econf_call(econf_readDirs)(&కీలకం_file,
			       మార్గం_VENDORDIR,
			       "/etc",
			       "shells",
			       ఖాళీ,
			       "", /* key only */
			       "#" /* comment */);
	ఒకవేళ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		తిరిగి ఖాళీ;
	}

	తిరిగి కీలకం_file;
}
#అయితే_ముగింపు

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
బాహ్య శూన్య print_shells(FILE *బహిర్గతం, స్థిరరాశి అక్షరం *format)
{
#ఒకవేళ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t కొలత = 0;
	econf_err error;
	అక్షరం **కీలకాలు = ఖాళీ;
	econf_file *కీలకం_file = open_etc_shells();

	ఒకవేళ (!కీలకం_file)
	  తిరిగి;

	error = econf_call(econf_get_కీలకాలు)(కీలకం_file, ఖాళీ, &కొలత, &కీలకాలు);
	ఒకవేళ (error) {
		econf_call(econf_freeFile)(కీలకం_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	కొరకు (size_t i = 0; i < కొలత; i++) {
		fprintf(బహిర్గతం, format, కీలకాలు[i]);
	}
	econf_call(econf_freeArray)(కీలకాలు);
	econf_call(econf_freeFile)(కీలకం_file);
#లేకపోతే
	అక్షరం *s;

	setusershell();
	అయితే ((s = getusershell()))
		fprintf(బహిర్గతం, format, s);
	endusershell();
#అయితే_ముగింపు
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
బాహ్య పూర్ణాంకం is_తెలిసిన_shell(స్థిరరాశి అక్షరం *shell_name)
{
	పూర్ణాంకం ret = 0;

	ఒకవేళ (!shell_name)
		తిరిగి 0;

#ఒకవేళ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	అక్షరం *val = ఖాళీ;
	econf_err error;
	econf_file *కీలకం_file = open_etc_shells();

	ఒకవేళ (!కీలకం_file)
		తిరిగి 0;

	error = econf_call(econf_get_String_విలువ) (కీలకం_file, ఖాళీ, shell_name, &val);
	ఒకవేళ (error) {
		ఒకవేళ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} లేకపోతే
		ret = 1;

	విడుదల(val);
	econf_call(econf_freeFile)(కీలకం_file);
#లేకపోతే
	అక్షరం *s;

	setusershell();
	అయితే ((s = getusershell())) {
		ఒకవేళ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ఆపు;
		}
	}
	endusershell();
#అయితే_ముగింపు
	తిరిగి ret;
}
