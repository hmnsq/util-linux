/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <paths.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/syslog.h>
#જો defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#અન્તર્ભાવ "dl-econf.h"
#endif

#અન્તર્ભાવ "closestream.h"
#અન્તર્ભાવ "shells.h"

#જો defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(શૂન્ય)
{
	econf_err error;
	econf_file *ચાવી_file = ખાલી;

	જો (ul_dlopen_libeconf() != 0)
		પાછા ખાલી;

	error = econf_call(econf_readDirs)(&ચાવી_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       ખાલી,
			       "", /* key only */
			       "#" /* comment */);
	જો (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		પાછા ખાલી;
	}

	પાછા ચાવી_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
બાહ્ય શૂન્ય print_shells(FILE *out, અચલ અક્ષર *format)
{
#જો defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	અક્ષર **ચાવીઓ = ખાલી;
	econf_file *ચાવી_file = open_etc_shells();

	જો (!ચાવી_file)
	  પાછા;

	error = econf_call(econf_get_ચાવીઓ)(ચાવી_file, ખાલી, &size, &ચાવીઓ);
	જો (error) {
		econf_call(econf_freeFile)(ચાવી_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	માટે (size_t i = 0; i < size; i++) {
		fprintf(out, format, ચાવીઓ[i]);
	}
	econf_call(econf_freeArray)(ચાવીઓ);
	econf_call(econf_freeFile)(ચાવી_file);
#નહીંતર
	અક્ષર *s;

	setusershell();
	જ્યારે ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
બાહ્ય પૂર્ણાંક is_known_shell(અચલ અક્ષર *shell_name)
{
	પૂર્ણાંક ret = 0;

	જો (!shell_name)
		પાછા 0;

#જો defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	અક્ષર *val = ખાલી;
	econf_err error;
	econf_file *ચાવી_file = open_etc_shells();

	જો (!ચાવી_file)
		પાછા 0;

	error = econf_call(econf_get_String_કિંમત) (ચાવી_file, ખાલી, shell_name, &val);
	જો (error) {
		જો (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} નહીંતર
		ret = 1;

	મુક્ત(val);
	econf_call(econf_freeFile)(ચાવી_file);
#નહીંતર
	અક્ષર *s;

	setusershell();
	જ્યારે ((s = getusershell())) {
		જો (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			તોડો;
		}
	}
	endusershell();
#endif
	પાછા ret;
}
