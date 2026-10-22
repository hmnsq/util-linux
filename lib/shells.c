/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <paths.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/syslog.h>
#എങ്കിൽ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#അന്തര്ഭാവ "dl-econf.h"
#endif

#അന്തര്ഭാവ "closestream.h"
#അന്തര്ഭാവ "shells.h"

#എങ്കിൽ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(ശൂന്യം)
{
	econf_err error;
	econf_file *താക്കോൽ_file = ശൂന്യ;

	എങ്കിൽ (ul_dlopen_libeconf() != 0)
		തിരികെ ശൂന്യ;

	error = econf_call(econf_readDirs)(&താക്കോൽ_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       ശൂന്യ,
			       "", /* key only */
			       "#" /* comment */);
	എങ്കിൽ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		തിരികെ ശൂന്യ;
	}

	തിരികെ താക്കോൽ_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
ബാഹ്യ ശൂന്യം print_shells(FILE *out, സ്ഥിരം അക്ഷരം *format)
{
#എങ്കിൽ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	അക്ഷരം **താക്കോലുകൾ = ശൂന്യ;
	econf_file *താക്കോൽ_file = open_etc_shells();

	എങ്കിൽ (!താക്കോൽ_file)
	  തിരികെ;

	error = econf_call(econf_get_താക്കോലുകൾ)(താക്കോൽ_file, ശൂന്യ, &size, &താക്കോലുകൾ);
	എങ്കിൽ (error) {
		econf_call(econf_freeFile)(താക്കോൽ_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	വേണ്ടി (size_t i = 0; i < size; i++) {
		fprintf(out, format, താക്കോലുകൾ[i]);
	}
	econf_call(econf_freeArray)(താക്കോലുകൾ);
	econf_call(econf_freeFile)(താക്കോൽ_file);
#അല്ലെങ്കിൽ
	അക്ഷരം *s;

	setusershell();
	ആകുമ്പോൾ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
ബാഹ്യ പൂർണ്ണസംഖ്യ is_known_shell(സ്ഥിരം അക്ഷരം *shell_name)
{
	പൂർണ്ണസംഖ്യ ret = 0;

	എങ്കിൽ (!shell_name)
		തിരികെ 0;

#എങ്കിൽ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	അക്ഷരം *val = ശൂന്യ;
	econf_err error;
	econf_file *താക്കോൽ_file = open_etc_shells();

	എങ്കിൽ (!താക്കോൽ_file)
		തിരികെ 0;

	error = econf_call(econf_get_String_മൂല്യം) (താക്കോൽ_file, ശൂന്യ, shell_name, &val);
	എങ്കിൽ (error) {
		എങ്കിൽ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} അല്ലെങ്കിൽ
		ret = 1;

	സ്വതന്ത്രം(val);
	econf_call(econf_freeFile)(താക്കോൽ_file);
#അല്ലെങ്കിൽ
	അക്ഷരം *s;

	setusershell();
	ആകുമ്പോൾ ((s = getusershell())) {
		എങ്കിൽ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			നിർത്തുക;
		}
	}
	endusershell();
#endif
	തിരികെ ret;
}
