/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <sys/types.h>
#समावेश <errno.h>

#समावेश "dl-selinux.h"
#समावेश "selinux-utils.h"

#अगर_घोषित HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
पूर्णांक ul_setfscreatecon_from_file(अक्षर *orig_file)
{
	अगर (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		अक्षर *scontext = शून्य;

		अगर (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			वापस -1;
		अगर (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			वापस -1;
		}
		selinux_call(freecon)(scontext);
	}
	वापस 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
पूर्णांक ul_selinux_has_access(स्थिर अक्षर *classstr, स्थिर अक्षर *perm, अक्षर **user_cxt)
{
	अक्षर *user;
	पूर्णांक rc;

	अगर (user_cxt)
		*user_cxt = शून्य;

	अगर (ul_dlopen_libselinux() != 0)
		वापस 0;

	अगर (selinux_call(getprevcon)(&user) != 0)
		वापस 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, शून्य);
	अगर (rc != 0 && user_cxt)
		*user_cxt = user;
	अन्यथा
		selinux_call(freecon)(user);

	वापस rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
पूर्णांक ul_selinux_get_default_context(स्थिर अक्षर *path, पूर्णांक st_mode, अक्षर **cxt)
{
	काष्ठा selabel_handle *hnd;
	काष्ठा selinux_opt options[SELABEL_NOPT] = {};
	पूर्णांक rc = 0;

	*cxt = शून्य;

	अगर (ul_dlopen_libselinux() != 0)
		वापस -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	अगर (!hnd)
		वापस -त्रुटि_सं;

	अगर (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -त्रुटि_सं;
	selinux_call(selabel_close)(hnd);

	वापस rc;
}

#पूर्ण_अगर /* HAVE_LIBSELINUX */
