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
số_nguyên ul_setfscreatecon_from_file(char *orig_file)
{
	nếu (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		char *scontext = NULL;

		nếu (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			trả_về -1;
		nếu (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			trả_về -1;
		}
		selinux_call(freecon)(scontext);
	}
	trả_về 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
số_nguyên ul_selinux_has_access(const char *classstr, const char *perm, char **user_cxt)
{
	char *user;
	số_nguyên rc;

	nếu (user_cxt)
		*user_cxt = NULL;

	nếu (ul_dlopen_libselinux() != 0)
		trả_về 0;

	nếu (selinux_call(getprevcon)(&user) != 0)
		trả_về 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, NULL);
	nếu (rc != 0 && user_cxt)
		*user_cxt = user;
	không_thì
		selinux_call(freecon)(user);

	trả_về rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
số_nguyên ul_selinux_get_default_context(const char *path, số_nguyên st_mode, char **cxt)
{
	struct selabel_handle *hnd;
	struct selinux_opt options[SELABEL_NOPT] = {};
	số_nguyên rc = 0;

	*cxt = NULL;

	nếu (ul_dlopen_libselinux() != 0)
		trả_về -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	nếu (!hnd)
		trả_về -errno;

	nếu (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	trả_về rc;
}

#endif /* HAVE_LIBSELINUX */
