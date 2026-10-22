/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <paths.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sys/syslog.h>
#எனில் defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#அந்தர்பாவ "dl-econf.h"
#endif

#அந்தர்பாவ "closestream.h"
#அந்தர்பாவ "shells.h"

#எனில் defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(வெறுமை)
{
	econf_err error;
	econf_file *சாவி_file = வெற்று;

	எனில் (ul_dlopen_libeconf() != 0)
		திரும்பு வெற்று;

	error = econf_call(econf_readDirs)(&சாவி_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       வெற்று,
			       "", /* key only */
			       "#" /* comment */);
	எனில் (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		திரும்பு வெற்று;
	}

	திரும்பு சாவி_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
வெளி வெறுமை print_shells(FILE *out, மாறிலி எழுத்து *format)
{
#எனில் defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	எழுத்து **சாவிகள் = வெற்று;
	econf_file *சாவி_file = open_etc_shells();

	எனில் (!சாவி_file)
	  திரும்பு;

	error = econf_call(econf_get_சாவிகள்)(சாவி_file, வெற்று, &size, &சாவிகள்);
	எனில் (error) {
		econf_call(econf_freeFile)(சாவி_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	க்கு (size_t i = 0; i < size; i++) {
		fprintf(out, format, சாவிகள்[i]);
	}
	econf_call(econf_freeArray)(சாவிகள்);
	econf_call(econf_freeFile)(சாவி_file);
#இல்லையெனில்
	எழுத்து *s;

	setusershell();
	போது ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
வெளி முழு is_known_shell(மாறிலி எழுத்து *shell_name)
{
	முழு ret = 0;

	எனில் (!shell_name)
		திரும்பு 0;

#எனில் defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	எழுத்து *val = வெற்று;
	econf_err error;
	econf_file *சாவி_file = open_etc_shells();

	எனில் (!சாவி_file)
		திரும்பு 0;

	error = econf_call(econf_get_String_மதிப்பு) (சாவி_file, வெற்று, shell_name, &val);
	எனில் (error) {
		எனில் (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} இல்லையெனில்
		ret = 1;

	விடுவி(val);
	econf_call(econf_freeFile)(சாவி_file);
#இல்லையெனில்
	எழுத்து *s;

	setusershell();
	போது ((s = getusershell())) {
		எனில் (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			நிறுத்து;
		}
	}
	endusershell();
#endif
	திரும்பு ret;
}
