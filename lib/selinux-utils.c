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
পূর্ণসংখ্যা ul_setfscreatecon_from_file(অক্ষর *orig_file)
{
	যদি (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		অক্ষর *scontext = ফাঁকা;

		যদি (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ফেরত -1;
		যদি (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ফেরত -1;
		}
		selinux_call(freecon)(scontext);
	}
	ফেরত 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
পূর্ণসংখ্যা ul_selinux_has_access(ধ্রুবক অক্ষর *classstr, ধ্রুবক অক্ষর *perm, অক্ষর **user_cxt)
{
	অক্ষর *user;
	পূর্ণসংখ্যা rc;

	যদি (user_cxt)
		*user_cxt = ফাঁকা;

	যদি (ul_dlopen_libselinux() != 0)
		ফেরত 0;

	যদি (selinux_call(getprevcon)(&user) != 0)
		ফেরত 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ফাঁকা);
	যদি (rc != 0 && user_cxt)
		*user_cxt = user;
	অন্যথা
		selinux_call(freecon)(user);

	ফেরত rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
পূর্ণসংখ্যা ul_selinux_get_default_context(ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা st_mode, অক্ষর **cxt)
{
	কাঠামো selabel_handle *hnd;
	কাঠামো selinux_opt options[SELABEL_NOPT] = {};
	পূর্ণসংখ্যা rc = 0;

	*cxt = ফাঁকা;

	যদি (ul_dlopen_libselinux() != 0)
		ফেরত -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	যদি (!hnd)
		ফেরত -errno;

	যদি (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ফেরত rc;
}

#endif /* HAVE_LIBSELINUX */
