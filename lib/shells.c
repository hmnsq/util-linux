/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#အကယ်၍ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#အကယ်၍ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	အကယ်၍ (ul_dlopen_libeconf() != 0)
		ပြန်ပေး NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	အကယ်၍ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ပြန်ပေး NULL;
	}

	ပြန်ပေး key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const var_na *format)
{
#အကယ်၍ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	var_na **keys = NULL;
	econf_file *key_file = open_etc_shells();

	အကယ်၍ (!key_file)
	  ပြန်ပေး;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	အကယ်၍ (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	အတွက် (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#မဟုတ်ပါက
	var_na *s;

	setusershell();
	နေစဉ် ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern ကိန်းပြည့် is_known_shell(const var_na *shell_name)
{
	ကိန်းပြည့် ret = 0;

	အကယ်၍ (!shell_name)
		ပြန်ပေး 0;

#အကယ်၍ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	var_na *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	အကယ်၍ (!key_file)
		ပြန်ပေး 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	အကယ်၍ (error) {
		အကယ်၍ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} မဟုတ်ပါက
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#မဟုတ်ပါက
	var_na *s;

	setusershell();
	နေစဉ် ((s = getusershell())) {
		အကယ်၍ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			bha_nga;
		}
	}
	endusershell();
#endif
	ပြန်ပေး ret;
}
