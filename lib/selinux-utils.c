/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <sys/types.h>
#ضمن <errno.h>

#ضمن "dl-selinux.h"
#ضمن "selinux-utils.h"

#اگر_تعریف_شده HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
صحيح ul_setfscreatecon_from_file(حرف *orig_file)
{
	إذا (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		حرف *scontext = عدم;

		إذا (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			أرجع -1;
		إذا (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			أرجع -1;
		}
		selinux_call(freecon)(scontext);
	}
	أرجع 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
صحيح ul_selinux_has_access(ثابت حرف *classstr, ثابت حرف *perm, حرف **user_cxt)
{
	حرف *user;
	صحيح rc;

	إذا (user_cxt)
		*user_cxt = عدم;

	إذا (ul_dlopen_libselinux() != 0)
		أرجع 0;

	إذا (selinux_call(getprevcon)(&user) != 0)
		أرجع 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, عدم);
	إذا (rc != 0 && user_cxt)
		*user_cxt = user;
	وإلا
		selinux_call(freecon)(user);

	أرجع rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
صحيح ul_selinux_get_default_context(ثابت حرف *مسیر, صحيح st_mode, حرف **cxt)
{
	بنية selabel_handle *hnd;
	بنية selinux_opt options[SELABEL_NOPT] = {};
	صحيح rc = 0;

	*cxt = عدم;

	إذا (ul_dlopen_libselinux() != 0)
		أرجع -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	إذا (!hnd)
		أرجع -errno;

	إذا (selinux_call(selabel_lookup)(hnd, cxt, مسیر, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	أرجع rc;
}

#پایان_اگر /* HAVE_LIBSELINUX */
