/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "dl-selinux.h"
#अन्तर्भाव "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
पूर्णाङ्क ul_setfscreatecon_from_file(वर्ण *orig_file)
{
	यदि (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		वर्ण *scontext = रिक्त;

		यदि (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			प्रत्यागम -1;
		यदि (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			प्रत्यागम -1;
		}
		selinux_call(freecon)(scontext);
	}
	प्रत्यागम 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
पूर्णाङ्क ul_selinux_has_access(ध्रुव वर्ण *classstr, ध्रुव वर्ण *perm, वर्ण **user_cxt)
{
	वर्ण *user;
	पूर्णाङ्क rc;

	यदि (user_cxt)
		*user_cxt = रिक्त;

	यदि (ul_dlopen_libselinux() != 0)
		प्रत्यागम 0;

	यदि (selinux_call(getprevcon)(&user) != 0)
		प्रत्यागम 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, रिक्त);
	यदि (rc != 0 && user_cxt)
		*user_cxt = user;
	अन्यथा
		selinux_call(freecon)(user);

	प्रत्यागम rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
पूर्णाङ्क ul_selinux_get_default_context(ध्रुव वर्ण *path, पूर्णाङ्क st_mode, वर्ण **cxt)
{
	संरचना selabel_handle *hnd;
	संरचना selinux_opt options[SELABEL_NOPT] = {};
	पूर्णाङ्क rc = 0;

	*cxt = रिक्त;

	यदि (ul_dlopen_libselinux() != 0)
		प्रत्यागम -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	यदि (!hnd)
		प्रत्यागम -errno;

	यदि (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	प्रत्यागम rc;
}

#endif /* HAVE_LIBSELINUX */
