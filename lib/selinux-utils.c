/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include "dl-selinux.h"
#include "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
ကိန်းပြည့် ul_setfscreatecon_from_file(var_na *orig_file)
{
	အကယ်၍ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		var_na *scontext = NULL;

		အကယ်၍ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ပြန်ပေး -1;
		အကယ်၍ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ပြန်ပေး -1;
		}
		selinux_call(freecon)(scontext);
	}
	ပြန်ပေး 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
ကိန်းပြည့် ul_selinux_has_access(const var_na *classstr, const var_na *perm, var_na **user_cxt)
{
	var_na *user;
	ကိန်းပြည့် rc;

	အကယ်၍ (user_cxt)
		*user_cxt = NULL;

	အကယ်၍ (ul_dlopen_libselinux() != 0)
		ပြန်ပေး 0;

	အကယ်၍ (selinux_call(getprevcon)(&user) != 0)
		ပြန်ပေး 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, NULL);
	အကယ်၍ (rc != 0 && user_cxt)
		*user_cxt = user;
	မဟုတ်ပါက
		selinux_call(freecon)(user);

	ပြန်ပေး rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
ကိန်းပြည့် ul_selinux_get_default_context(const var_na *path, ကိန်းပြည့် st_mode, var_na **cxt)
{
	sa_nracanaa selabel_handle *hnd;
	sa_nracanaa selinux_opt options[SELABEL_NOPT] = {};
	ကိန်းပြည့် rc = 0;

	*cxt = NULL;

	အကယ်၍ (ul_dlopen_libselinux() != 0)
		ပြန်ပေး -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	အကယ်၍ (!hnd)
		ပြန်ပေး -errno;

	အကယ်၍ (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ပြန်ပေး rc;
}

#endif /* HAVE_LIBSELINUX */
