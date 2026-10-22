/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "dl-selinux.h"
#ಅನ್ತರ್ಭಾವ "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
ಪೂರ್ಣಾಙ್ಕ ul_setfscreatecon_from_file(ವರ್ಣ *orig_file)
{
	ಯದಿ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		ವರ್ಣ *scontext = ರಿಕ್ತ;

		ಯದಿ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		selinux_call(freecon)(scontext);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
ಪೂರ್ಣಾಙ್ಕ ul_selinux_has_access(ಧ್ರುವ ವರ್ಣ *classstr, ಧ್ರುವ ವರ್ಣ *perm, ವರ್ಣ **user_cxt)
{
	ವರ್ಣ *user;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (user_cxt)
		*user_cxt = ರಿಕ್ತ;

	ಯದಿ (ul_dlopen_libselinux() != 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (selinux_call(getprevcon)(&user) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ರಿಕ್ತ);
	ಯದಿ (rc != 0 && user_cxt)
		*user_cxt = user;
	ಅನ್ಯಥಾ
		selinux_call(freecon)(user);

	ಪ್ರತ್ಯಾಗಮ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
ಪೂರ್ಣಾಙ್ಕ ul_selinux_get_default_context(ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ st_mode, ವರ್ಣ **cxt)
{
	ಸಂರಚನಾ selabel_handle *hnd;
	ಸಂರಚನಾ selinux_opt options[SELABEL_NOPT] = {};
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	*cxt = ರಿಕ್ತ;

	ಯದಿ (ul_dlopen_libselinux() != 0)
		ಪ್ರತ್ಯಾಗಮ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ಯದಿ (!hnd)
		ಪ್ರತ್ಯಾಗಮ -errno;

	ಯದಿ (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ಪ್ರತ್ಯಾಗಮ rc;
}

#endif /* HAVE_LIBSELINUX */
