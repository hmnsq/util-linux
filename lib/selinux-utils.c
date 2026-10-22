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
puur_naa_nka ul_setfscreatecon_from_file(var_na *orig_file)
{
	ᯀᯮᯝ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		var_na *scontext = NULL;

		ᯀᯮᯝ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ᯀᯮᯞ -1;
		ᯀᯮᯝ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ᯀᯮᯞ -1;
		}
		selinux_call(freecon)(scontext);
	}
	ᯀᯮᯞ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
puur_naa_nka ul_selinux_has_access(const var_na *classstr, const var_na *perm, var_na **user_cxt)
{
	var_na *user;
	puur_naa_nka rc;

	ᯀᯮᯝ (user_cxt)
		*user_cxt = NULL;

	ᯀᯮᯝ (ul_dlopen_libselinux() != 0)
		ᯀᯮᯞ 0;

	ᯀᯮᯝ (selinux_call(getprevcon)(&user) != 0)
		ᯀᯮᯞ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, NULL);
	ᯀᯮᯝ (rc != 0 && user_cxt)
		*user_cxt = user;
	ᯀᯮᯝᯂᯪ
		selinux_call(freecon)(user);

	ᯀᯮᯞ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
puur_naa_nka ul_selinux_get_default_context(const var_na *path, puur_naa_nka st_mode, var_na **cxt)
{
	sa_nracanaa selabel_handle *hnd;
	sa_nracanaa selinux_opt options[SELABEL_NOPT] = {};
	puur_naa_nka rc = 0;

	*cxt = NULL;

	ᯀᯮᯝ (ul_dlopen_libselinux() != 0)
		ᯀᯮᯞ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ᯀᯮᯝ (!hnd)
		ᯀᯮᯞ -errno;

	ᯀᯮᯝ (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ᯀᯮᯞ rc;
}

#endif /* HAVE_LIBSELINUX */
