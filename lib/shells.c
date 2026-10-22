/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <paths.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syslog.h>
#ਜੇ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#ਅਨ੍ਤਰ੍ਭਾਵ "dl-econf.h"
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "closestream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "shells.h"

#ਜੇ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(ਖਾਲੀ)
{
	econf_err error;
	econf_file *ਕੁੰਜੀ_file = ਨਿਰਾ;

	ਜੇ (ul_dlopen_libeconf() != 0)
		ਵਾਪਸ ਨਿਰਾ;

	error = econf_call(econf_readDirs)(&ਕੁੰਜੀ_file,
			       ਰਸਤਾ_VENDORDIR,
			       "/etc",
			       "shells",
			       ਨਿਰਾ,
			       "", /* key only */
			       "#" /* comment */);
	ਜੇ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ ਕੁੰਜੀ_file;
}
#ਜੇ_ਅੰਤ

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
ਬਾਹਰੀ ਖਾਲੀ print_shells(FILE *ਬਾਹਰਲਾ, ਅਚਲ ਅੱਖਰ *format)
{
#ਜੇ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t ਮਾਪ = 0;
	econf_err error;
	ਅੱਖਰ **ਕੁੰਜੀਆਂ = ਨਿਰਾ;
	econf_file *ਕੁੰਜੀ_file = open_etc_shells();

	ਜੇ (!ਕੁੰਜੀ_file)
	  ਵਾਪਸ;

	error = econf_call(econf_get_ਕੁੰਜੀਆਂ)(ਕੁੰਜੀ_file, ਨਿਰਾ, &ਮਾਪ, &ਕੁੰਜੀਆਂ);
	ਜੇ (error) {
		econf_call(econf_freeFile)(ਕੁੰਜੀ_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	ਲਈ (size_t i = 0; i < ਮਾਪ; i++) {
		fprintf(ਬਾਹਰਲਾ, format, ਕੁੰਜੀਆਂ[i]);
	}
	econf_call(econf_freeArray)(ਕੁੰਜੀਆਂ);
	econf_call(econf_freeFile)(ਕੁੰਜੀ_file);
#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *s;

	setusershell();
	ਜਦੋਂ ((s = getusershell()))
		fprintf(ਬਾਹਰਲਾ, format, s);
	endusershell();
#ਜੇ_ਅੰਤ
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
ਬਾਹਰੀ ਪੂਰਨ is_ਗਿਆਤ_shell(ਅਚਲ ਅੱਖਰ *shell_name)
{
	ਪੂਰਨ ret = 0;

	ਜੇ (!shell_name)
		ਵਾਪਸ 0;

#ਜੇ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	ਅੱਖਰ *val = ਨਿਰਾ;
	econf_err error;
	econf_file *ਕੁੰਜੀ_file = open_etc_shells();

	ਜੇ (!ਕੁੰਜੀ_file)
		ਵਾਪਸ 0;

	error = econf_call(econf_get_String_ਮੁੱਲ) (ਕੁੰਜੀ_file, ਨਿਰਾ, shell_name, &val);
	ਜੇ (error) {
		ਜੇ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ਨਹੀਂਤਾਂ
		ret = 1;

	ਮੁਕਤ(val);
	econf_call(econf_freeFile)(ਕੁੰਜੀ_file);
#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *s;

	setusershell();
	ਜਦੋਂ ((s = getusershell())) {
		ਜੇ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			ਰੋਕੋ;
		}
	}
	endusershell();
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ret;
}
