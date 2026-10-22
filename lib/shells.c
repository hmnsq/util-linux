/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <paths.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/syslog.h>
#ଯଦି defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ଅନ୍ତର୍ଭାଵ "dl-econf.h"
#endif

#ଅନ୍ତର୍ଭାଵ "closestream.h"
#ଅନ୍ତର୍ଭାଵ "shells.h"

#ଯଦି defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(ଶୂନ୍ୟ)
{
	econf_err error;
	econf_file *ଚାବି_file = ଖାଲି;

	ଯଦି (ul_dlopen_libeconf() != 0)
		ଫେରନ୍ତୁ ଖାଲି;

	error = econf_call(econf_readDirs)(&ଚାବି_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       ଖାଲି,
			       "", /* key only */
			       "#" /* comment */);
	ଯଦି (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ ଚାବି_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
ବାହ୍ୟ ଶୂନ୍ୟ print_shells(FILE *out, ଅଚଳ ଅକ୍ଷର *format)
{
#ଯଦି defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	ଅକ୍ଷର **ଚାବିଗୁଡ଼ିକ = ଖାଲି;
	econf_file *ଚାବି_file = open_etc_shells();

	ଯଦି (!ଚାବି_file)
	  ଫେରନ୍ତୁ;

	error = econf_call(econf_get_ଚାବିଗୁଡ଼ିକ)(ଚାବି_file, ଖାଲି, &size, &ଚାବିଗୁଡ଼ିକ);
	ଯଦି (error) {
		econf_call(econf_freeFile)(ଚାବି_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	ପାଇଁ (size_t i = 0; i < size; i++) {
		fprintf(out, format, ଚାବିଗୁଡ଼ିକ[i]);
	}
	econf_call(econf_freeArray)(ଚାବିଗୁଡ଼ିକ);
	econf_call(econf_freeFile)(ଚାବି_file);
#ଅନ୍ୟଥା
	ଅକ୍ଷର *s;

	setusershell();
	ଯେତେବେଳେ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_known_shell(ଅଚଳ ଅକ୍ଷର *shell_name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

	ଯଦି (!shell_name)
		ଫେରନ୍ତୁ 0;

#ଯଦି defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	ଅକ୍ଷର *val = ଖାଲି;
	econf_err error;
	econf_file *ଚାବି_file = open_etc_shells();

	ଯଦି (!ଚାବି_file)
		ଫେରନ୍ତୁ 0;

	error = econf_call(econf_get_String_ମୂଲ୍ୟ) (ଚାବି_file, ଖାଲି, shell_name, &val);
	ଯଦି (error) {
		ଯଦି (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ଅନ୍ୟଥା
		ret = 1;

	ମୁକ୍ତ(val);
	econf_call(econf_freeFile)(ଚାବି_file);
#ଅନ୍ୟଥା
	ଅକ୍ଷର *s;

	setusershell();
	ଯେତେବେଳେ ((s = getusershell())) {
		ଯଦି (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ଭାଙ୍ଗ;
		}
	}
	endusershell();
#endif
	ଫେରନ୍ତୁ ret;
}
