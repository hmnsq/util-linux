/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#如果 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#如果 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	如果 (ul_dlopen_libeconf() != 0)
		返回 NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	如果 (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		返回 NULL;
	}

	返回 key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const char *format)
{
#如果 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	char **keys = NULL;
	econf_file *key_file = open_etc_shells();

	如果 (!key_file)
	  返回;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	如果 (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	为 (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#否则
	char *s;

	setusershell();
	当 ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern 整数 is_known_shell(const char *shell_name)
{
	整数 ret = 0;

	如果 (!shell_name)
		返回 0;

#如果 defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	char *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	如果 (!key_file)
		返回 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	如果 (error) {
		如果 (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} 否则
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#否则
	char *s;

	setusershell();
	当 ((s = getusershell())) {
		如果 (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			break;
		}
	}
	endusershell();
#endif
	返回 ret;
}
