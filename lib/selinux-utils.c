/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "dl-selinux.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "selinux-utils.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
ਪੂਰਨ ul_setfscreatecon_from_file(ਅੱਖਰ *orig_file)
{
	ਜੇ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		ਅੱਖਰ *scontext = ਨਿਰਾ;

		ਜੇ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ਵਾਪਸ -1;
		ਜੇ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ਵਾਪਸ -1;
		}
		selinux_call(freecon)(scontext);
	}
	ਵਾਪਸ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
ਪੂਰਨ ul_selinux_has_access(ਅਚਲ ਅੱਖਰ *classstr, ਅਚਲ ਅੱਖਰ *perm, ਅੱਖਰ **user_cxt)
{
	ਅੱਖਰ *user;
	ਪੂਰਨ rc;

	ਜੇ (user_cxt)
		*user_cxt = ਨਿਰਾ;

	ਜੇ (ul_dlopen_libselinux() != 0)
		ਵਾਪਸ 0;

	ਜੇ (selinux_call(getprevcon)(&user) != 0)
		ਵਾਪਸ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ਨਿਰਾ);
	ਜੇ (rc != 0 && user_cxt)
		*user_cxt = user;
	ਨਹੀਂਤਾਂ
		selinux_call(freecon)(user);

	ਵਾਪਸ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
ਪੂਰਨ ul_selinux_get_default_context(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ st_mode, ਅੱਖਰ **cxt)
{
	ਬਣਤਰ selabel_handle *hnd;
	ਬਣਤਰ selinux_opt options[SELABEL_NOPT] = {};
	ਪੂਰਨ rc = 0;

	*cxt = ਨਿਰਾ;

	ਜੇ (ul_dlopen_libselinux() != 0)
		ਵਾਪਸ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ਜੇ (!hnd)
		ਵਾਪਸ -errno;

	ਜੇ (selinux_call(selabel_lookup)(hnd, cxt, ਰਸਤਾ, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* HAVE_LIBSELINUX */
