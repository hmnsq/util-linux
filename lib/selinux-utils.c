/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#شامل <stdio.h>
#شامل <string.h>
#شامل <sys/types.h>
#شامل <errno.h>

#شامل "dl-selinux.h"
#شامل "selinux-utils.h"

#اگر_تعریف_شدہ HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
صحیح ul_setfscreatecon_from_file(حرف *orig_file)
{
	اگر (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		حرف *scontext = تہی;

		اگر (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			واپس -1;
		اگر (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			واپس -1;
		}
		selinux_call(freecon)(scontext);
	}
	واپس 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
صحیح ul_selinux_has_access(ثابت حرف *classstr, ثابت حرف *perm, حرف **user_cxt)
{
	حرف *user;
	صحیح rc;

	اگر (user_cxt)
		*user_cxt = تہی;

	اگر (ul_dlopen_libselinux() != 0)
		واپس 0;

	اگر (selinux_call(getprevcon)(&user) != 0)
		واپس 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, تہی);
	اگر (rc != 0 && user_cxt)
		*user_cxt = user;
	ورنہ
		selinux_call(freecon)(user);

	واپس rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
صحیح ul_selinux_get_default_context(ثابت حرف *راستہ, صحیح st_mode, حرف **cxt)
{
	ساخت selabel_handle *hnd;
	ساخت selinux_opt options[SELABEL_NOPT] = {};
	صحیح rc = 0;

	*cxt = تہی;

	اگر (ul_dlopen_libselinux() != 0)
		واپس -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	اگر (!hnd)
		واپس -errno;

	اگر (selinux_call(selabel_lookup)(hnd, cxt, راستہ, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	واپس rc;
}

#اگر_ختم /* HAVE_LIBSELINUX */
