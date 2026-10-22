/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <paths.h>
#include <unistd.h>
#include <sys/syslog.h>
#if defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
#include "dl-econf.h"
#endif

#include "closestream.h"
#include "shells.h"

#if defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
econf_file *open_etc_shells(void)
{
	econf_err error;
	econf_file *सुसि_file = NULL;

	if (ul_dlopen_libeconf() != 0)
		return NULL;

	error = econf_call(econf_readDirs)(&सुसि_file,
			       _PATH_VENDORDIR,
			       "/etc",
			       "shells",
			       NULL,
			       "", /* key only */
			       "#" /* comment */);
	if (error) {
		syslog(LOG_ALERT,
		       _("Cannot parse shells files: %s"),
		       econf_call(econf_errString)(error));
		return NULL;
	}

	return सुसि_file;
}
#endif

/*
 *  print_shells () -- /etc/shells is outputted to stdout.
 */
extern void इलिम्बाग_shells(FILE *out, const char *format)
{
#if defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	size_t size = 0;
	econf_err error;
	char **मगा_सुसि = NULL;
	econf_file *सुसि_file = open_etc_shells();

	if (!सुसि_file)
	  return;

	error = econf_call(econf_get_मगा_सुसि)(सुसि_file, NULL, &size, &मगा_सुसि);
	if (error) {
		econf_call(econf_freeFile)(सुसि_file);
		errx(EXIT_FAILURE,
		  _("Cannot evaluate entries in shells files: %s"),
		  econf_call(econf_errString)(error));
	}

	for (size_t i = 0; i < size; i++) {
		fprintf(out, format, मगा_सुसि[i]);
	}
	econf_call(econf_freeArray)(मगा_सुसि);
	econf_call(econf_freeFile)(सुसि_file);
#else
	char *s;

	setusershell();
	while ((s = getusershell()))
		fprintf(out, format, s);
	endusershell();
#endif
}


/*
 *  is_known_shell() -- if the given shell appears in /etc/shells
 *  or vendor defined files.
 *  Return 1 if found and return 0 if not found.
 */
extern int is_known_shell(const char *shell_name)
{
	int ret = 0;

	if (!shell_name)
		return 0;

#if defined (HAVE_LIBECONF) && defined (USE_VENDORDIR)
	char *val = NULL;
	econf_err error;
	econf_file *सुसि_file = open_etc_shells();

	if (!सुसि_file)
		return 0;

	error = econf_call(econf_get_String_हालागा) (सुसि_file, NULL, shell_name, &val);
	if (error) {
		if (error != ECONF_NOKEY)
			syslog(LOG_ALERT,
			       _("Cannot evaluate entries in shells files: %s"),
			       econf_call(econf_errString)(error));
	} else
		ret = 1;

	free(val);
	econf_call(econf_freeFile)(सुसि_file);
#else
	char *s;

	setusershell();
	while ((s = getusershell())) {
		if (*s != '#' && strcmp(shell_name, s) == 0) {
			ret = 1;
			break;
		}
	}
	endusershell();
#endif
	return ret;
}
