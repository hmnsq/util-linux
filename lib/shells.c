/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#만약 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#만약 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	만약 (ul_dlopen_libeconf() != 0)
		반환 NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	만약 (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		반환 NULL;
	}

	반환 key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const char *format)
{
#만약 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	char **keys = NULL;
	econf_file *key_file = open_etc_shells();

	만약 (!key_file)
	  반환;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	만약 (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	위해 (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#아니면
	char *s;

	setusershell();
	동안 ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern 정수 is_known_shell(const char *shell_name)
{
	정수 ret = 0;

	만약 (!shell_name)
		반환 0;

#만약 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	char *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	만약 (!key_file)
		반환 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	만약 (error) {
		만약 (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} 아니면
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#아니면
	char *s;

	setusershell();
	동안 ((s = getusershell())) {
		만약 (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			break;
		}
	}
	endusershell();
#endif
	반환 ret;
}
