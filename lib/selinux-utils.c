/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "dl-selinux.h"
#அந்தர்பாவ "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
முழு ul_setfscreatecon_from_file(எழுத்து *orig_file)
{
	எனில் (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		எழுத்து *scontext = வெற்று;

		எனில் (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			திரும்பு -1;
		எனில் (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			திரும்பு -1;
		}
		selinux_call(freecon)(scontext);
	}
	திரும்பு 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
முழு ul_selinux_has_access(மாறிலி எழுத்து *classstr, மாறிலி எழுத்து *perm, எழுத்து **user_cxt)
{
	எழுத்து *user;
	முழு rc;

	எனில் (user_cxt)
		*user_cxt = வெற்று;

	எனில் (ul_dlopen_libselinux() != 0)
		திரும்பு 0;

	எனில் (selinux_call(getprevcon)(&user) != 0)
		திரும்பு 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, வெற்று);
	எனில் (rc != 0 && user_cxt)
		*user_cxt = user;
	இல்லையெனில்
		selinux_call(freecon)(user);

	திரும்பு rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
முழு ul_selinux_get_default_context(மாறிலி எழுத்து *path, முழு st_mode, எழுத்து **cxt)
{
	கட்டமைப்பு selabel_handle *hnd;
	கட்டமைப்பு selinux_opt options[SELABEL_NOPT] = {};
	முழு rc = 0;

	*cxt = வெற்று;

	எனில் (ul_dlopen_libselinux() != 0)
		திரும்பு -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	எனில் (!hnd)
		திரும்பு -errno;

	எனில் (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	திரும்பு rc;
}

#endif /* HAVE_LIBSELINUX */
