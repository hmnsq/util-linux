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
पूर्णांक ul_setfscreatecon_from_file(अक्षर *orig_file)
{
	जर (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		अक्षर *scontext = रिक्त;

		जर (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			परत -1;
		जर (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			परत -1;
		}
		selinux_call(freecon)(scontext);
	}
	परत 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
पूर्णांक ul_selinux_has_access(स्थिरांक अक्षर *classstr, स्थिरांक अक्षर *perm, अक्षर **user_cxt)
{
	अक्षर *user;
	पूर्णांक rc;

	जर (user_cxt)
		*user_cxt = रिक्त;

	जर (ul_dlopen_libselinux() != 0)
		परत 0;

	जर (selinux_call(getprevcon)(&user) != 0)
		परत 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, रिक्त);
	जर (rc != 0 && user_cxt)
		*user_cxt = user;
	अन्यथा
		selinux_call(freecon)(user);

	परत rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
पूर्णांक ul_selinux_get_default_context(स्थिरांक अक्षर *path, पूर्णांक st_mode, अक्षर **cxt)
{
	संरचना selabel_handle *hnd;
	संरचना selinux_opt options[SELABEL_NOPT] = {};
	पूर्णांक rc = 0;

	*cxt = रिक्त;

	जर (ul_dlopen_libselinux() != 0)
		परत -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	जर (!hnd)
		परत -त्रुटि_सं;

	जर (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -त्रुटि_सं;
	selinux_call(selabel_close)(hnd);

	परत rc;
}

#endif /* HAVE_LIBSELINUX */
