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
정수 ul_setfscreatecon_from_file(char *orig_file)
{
	만약 (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		char *scontext = NULL;

		만약 (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			반환 -1;
		만약 (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			반환 -1;
		}
		selinux_call(freecon)(scontext);
	}
	반환 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
정수 ul_selinux_has_access(const char *classstr, const char *perm, char **user_cxt)
{
	char *user;
	정수 rc;

	만약 (user_cxt)
		*user_cxt = NULL;

	만약 (ul_dlopen_libselinux() != 0)
		반환 0;

	만약 (selinux_call(getprevcon)(&user) != 0)
		반환 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, NULL);
	만약 (rc != 0 && user_cxt)
		*user_cxt = user;
	아니면
		selinux_call(freecon)(user);

	반환 rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
정수 ul_selinux_get_default_context(const char *path, 정수 st_mode, char **cxt)
{
	struct selabel_handle *hnd;
	struct selinux_opt options[SELABEL_NOPT] = {};
	정수 rc = 0;

	*cxt = NULL;

	만약 (ul_dlopen_libselinux() != 0)
		반환 -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	만약 (!hnd)
		반환 -errno;

	만약 (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	반환 rc;
}

#endif /* HAVE_LIBSELINUX */
