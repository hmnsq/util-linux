/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include "dl-selinux.h"
#include "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
цео ul_setfscreatecon_from_file(char *orig_file)
{
	ако (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		char *scontext = NULL;

		ако (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			врати -1;
		ако (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			врати -1;
		}
		selinux_call(freecon)(scontext);
	}
	врати 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
цео ul_selinux_has_access(const char *classstr, const char *perm, char **user_cxt)
{
	char *user;
	цео rc;

	ако (user_cxt)
		*user_cxt = NULL;

	ако (ul_dlopen_libselinux() != 0)
		врати 0;

	ако (selinux_call(getprevcon)(&user) != 0)
		врати 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, NULL);
	ако (rc != 0 && user_cxt)
		*user_cxt = user;
	инако
		selinux_call(freecon)(user);

	врати rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
цео ul_selinux_get_default_context(const char *path, цео st_mode, char **cxt)
{
	struct selabel_handle *hnd;
	struct selinux_opt options[SELABEL_NOPT] = {};
	цео rc = 0;

	*cxt = NULL;

	ако (ul_dlopen_libselinux() != 0)
		врати -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ако (!hnd)
		врати -errno;

	ако (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	врати rc;
}

#endif /* HAVE_LIBSELINUX */
