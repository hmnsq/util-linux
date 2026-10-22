/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#ᮚᮦᮔ᮪ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#ᮚᮦᮔ᮪ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	ᮚᮦᮔ᮪ (ul_dlopen_libeconf() != 0)
		ᮝᮜᮤ NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	ᮚᮦᮔ᮪ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ᮝᮜᮤ NULL;
	}

	ᮝᮜᮤ key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const var_na *format)
{
#ᮚᮦᮔ᮪ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	var_na **keys = NULL;
	econf_file *key_file = open_etc_shells();

	ᮚᮦᮔ᮪ (!key_file)
	  ᮝᮜᮤ;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	ᮚᮦᮔ᮪ (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	for (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#ᮜᮤᮚ
	var_na *s;

	setusershell();
	ᮞᮨᮓᮨᮜ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern puur_naa_nka is_known_shell(const var_na *shell_name)
{
	puur_naa_nka ret = 0;

	ᮚᮦᮔ᮪ (!shell_name)
		ᮝᮜᮤ 0;

#ᮚᮦᮔ᮪ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	var_na *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	ᮚᮦᮔ᮪ (!key_file)
		ᮝᮜᮤ 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	ᮚᮦᮔ᮪ (error) {
		ᮚᮦᮔ᮪ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ᮜᮤᮚ
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#ᮜᮤᮚ
	var_na *s;

	setusershell();
	ᮞᮨᮓᮨᮜ ((s = getusershell())) {
		ᮚᮦᮔ᮪ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			bha_nga;
		}
	}
	endusershell();
#endif
	ᮝᮜᮤ ret;
}
