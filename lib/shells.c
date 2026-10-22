/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#ᤀᤡ᤺ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#ᤀᤡ᤺ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	ᤀᤡ᤺ (ul_dlopen_libeconf() != 0)
		ᤀᤡᤴ NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	ᤀᤡ᤺ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ᤀᤡᤴ NULL;
	}

	ᤀᤡᤴ key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const var_na *format)
{
#ᤀᤡ᤺ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	var_na **keys = NULL;
	econf_file *key_file = open_etc_shells();

	ᤀᤡ᤺ (!key_file)
	  ᤀᤡᤴ;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	ᤀᤡ᤺ (error) {
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
#ᤀᤡ᤺ᤆᤡ
	var_na *s;

	setusershell();
	ᤀᤠ᤺ ((s = getusershell()))
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

	ᤀᤡ᤺ (!shell_name)
		ᤀᤡᤴ 0;

#ᤀᤡ᤺ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	var_na *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	ᤀᤡ᤺ (!key_file)
		ᤀᤡᤴ 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	ᤀᤡ᤺ (error) {
		ᤀᤡ᤺ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ᤀᤡ᤺ᤆᤡ
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#ᤀᤡ᤺ᤆᤡ
	var_na *s;

	setusershell();
	ᤀᤠ᤺ ((s = getusershell())) {
		ᤀᤡ᤺ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			bha_nga;
		}
	}
	endusershell();
#endif
	ᤀᤡᤴ ret;
}
