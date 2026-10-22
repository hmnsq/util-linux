/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "dl-selinux.h"
#ଅନ୍ତର୍ଭାଵ "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_setfscreatecon_from_file(ଅକ୍ଷର *orig_file)
{
	ଯଦି (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		ଅକ୍ଷର *scontext = ଖାଲି;

		ଯଦି (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			ଫେରନ୍ତୁ -1;
		ଯଦି (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			ଫେରନ୍ତୁ -1;
		}
		selinux_call(freecon)(scontext);
	}
	ଫେରନ୍ତୁ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_selinux_has_access(ଅଚଳ ଅକ୍ଷର *classstr, ଅଚଳ ଅକ୍ଷର *perm, ଅକ୍ଷର **user_cxt)
{
	ଅକ୍ଷର *user;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (user_cxt)
		*user_cxt = ଖାଲି;

	ଯଦି (ul_dlopen_libselinux() != 0)
		ଫେରନ୍ତୁ 0;

	ଯଦି (selinux_call(getprevcon)(&user) != 0)
		ଫେରନ୍ତୁ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ଖାଲି);
	ଯଦି (rc != 0 && user_cxt)
		*user_cxt = user;
	ଅନ୍ୟଥା
		selinux_call(freecon)(user);

	ଫେରନ୍ତୁ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_selinux_get_default_context(ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା st_mode, ଅକ୍ଷର **cxt)
{
	ସଂରଚନା selabel_handle *hnd;
	ସଂରଚନା selinux_opt options[SELABEL_NOPT] = {};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	*cxt = ଖାଲି;

	ଯଦି (ul_dlopen_libselinux() != 0)
		ଫେରନ୍ତୁ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ଯଦି (!hnd)
		ଫେରନ୍ତୁ -errno;

	ଯଦି (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	ଫେରନ୍ତୁ rc;
}

#endif /* HAVE_LIBSELINUX */
