/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#ຖ້າ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#ຖ້າ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *key_file = NULL;

	ຖ້າ (ul_dlopen_libeconf() != 0)
		ສົ່ງຄືນ NULL;

	error = econf_call(econf_readDirs)(&key_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	ຖ້າ (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		ສົ່ງຄືນ NULL;
	}

	ສົ່ງຄືນ key_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void print_shells(FILE *out, const var_na *format)
{
#ຖ້າ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	var_na **keys = NULL;
	econf_file *key_file = open_etc_shells();

	ຖ້າ (!key_file)
	  ສົ່ງຄືນ;

	error = econf_call(econf_getKeys)(key_file, NULL, &size, &keys);
	ຖ້າ (error) {
		econf_call(econf_freeFile)(key_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	ສຳລັບ (size_t i = 0; i < size; i++) {
		fprintf(out, format, keys[i]);
	}
	econf_call(econf_freeArray)(keys);
	econf_call(econf_freeFile)(key_file);
#ບໍ່ດັ່ງນັ້ນ
	var_na *s;

	setusershell();
	ໃນຂະນະທີ່ ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern ຈຳນວນເຕັມ is_known_shell(const var_na *shell_name)
{
	ຈຳນວນເຕັມ ret = 0;

	ຖ້າ (!shell_name)
		ສົ່ງຄືນ 0;

#ຖ້າ defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	var_na *val = NULL;
	econf_err error;
	econf_file *key_file = open_etc_shells();

	ຖ້າ (!key_file)
		ສົ່ງຄືນ 0;

	error = econf_call(econf_getStringValue) (key_file, NULL, shell_name, &val);
	ຖ້າ (error) {
		ຖ້າ (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} ບໍ່ດັ່ງນັ້ນ
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(key_file);
#ບໍ່ດັ່ງນັ້ນ
	var_na *s;

	setusershell();
	ໃນຂະນະທີ່ ((s = getusershell())) {
		ຖ້າ (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			bha_nga;
		}
	}
	endusershell();
#endif
	ສົ່ງຄືນ ret;
}
