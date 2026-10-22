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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
ಪೂರ್ಣಾಂಕ ul_setfscreatecon_from_file(ಅಕ್ಷರ *orig_file)
{
	ಆದರೆ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		ಅಕ್ಷರ *scontext = ಖಾಲಿ;

		ಆದರೆ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ಹಿಂತಿರುಗಿ -1;
		}
		selinux_call(freecon)(scontext);
	}
	ಹಿಂತಿರುಗಿ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
ಪೂರ್ಣಾಂಕ ul_selinux_has_access(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *classstr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *perm, ಅಕ್ಷರ **user_cxt)
{
	ಅಕ್ಷರ *user;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (user_cxt)
		*user_cxt = ಖಾಲಿ;

	ಆದರೆ (ul_dlopen_libselinux() != 0)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (selinux_call(getprevcon)(&user) != 0)
		ಹಿಂತಿರುಗಿ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ಖಾಲಿ);
	ಆದರೆ (rc != 0 && user_cxt)
		*user_cxt = user;
	ಇಲ್ಲದಿದ್ದರೆ
		selinux_call(freecon)(user);

	ಹಿಂತಿರುಗಿ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
ಪೂರ್ಣಾಂಕ ul_selinux_get_default_context(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ st_mode, ಅಕ್ಷರ **cxt)
{
	ರಚನೆ selabel_handle *hnd;
	ರಚನೆ selinux_opt options[SELABEL_NOPT] = {};
	ಪೂರ್ಣಾಂಕ rc = 0;

	*cxt = ಖಾಲಿ;

	ಆದರೆ (ul_dlopen_libselinux() != 0)
		ಹಿಂತಿರುಗಿ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ಆದರೆ (!hnd)
		ಹಿಂತಿರುಗಿ -errno;

	ಆದರೆ (selinux_call(selabel_lookup)(hnd, cxt, ಮಾರ್ಗ, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ_ಅಂತ್ಯ /* HAVE_LIBSELINUX */
