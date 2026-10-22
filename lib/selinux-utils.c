/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "dl-selinux.h"
#অন্তর্ভাব "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
পূর্ণাঙ্ক ul_setfscreatecon_from_file(বর্ণ *orig_file)
{
	যদি (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		বর্ণ *scontext = রিক্ত;

		যদি (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			প্রত্যাগম -1;
		যদি (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			প্রত্যাগম -1;
		}
		selinux_call(freecon)(scontext);
	}
	প্রত্যাগম 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
পূর্ণাঙ্ক ul_selinux_has_access(ধ্রুব বর্ণ *classstr, ধ্রুব বর্ণ *perm, বর্ণ **user_cxt)
{
	বর্ণ *user;
	পূর্ণাঙ্ক rc;

	যদি (user_cxt)
		*user_cxt = রিক্ত;

	যদি (ul_dlopen_libselinux() != 0)
		প্রত্যাগম 0;

	যদি (selinux_call(getprevcon)(&user) != 0)
		প্রত্যাগম 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, রিক্ত);
	যদি (rc != 0 && user_cxt)
		*user_cxt = user;
	অন্যথা
		selinux_call(freecon)(user);

	প্রত্যাগম rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
পূর্ণাঙ্ক ul_selinux_get_default_context(ধ্রুব বর্ণ *path, পূর্ণাঙ্ক st_mode, বর্ণ **cxt)
{
	সংরচনা selabel_handle *hnd;
	সংরচনা selinux_opt options[SELABEL_NOPT] = {};
	পূর্ণাঙ্ক rc = 0;

	*cxt = রিক্ত;

	যদি (ul_dlopen_libselinux() != 0)
		প্রত্যাগম -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	যদি (!hnd)
		প্রত্যাগম -errno;

	যদি (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	প্রত্যাগম rc;
}

#endif /* HAVE_LIBSELINUX */
