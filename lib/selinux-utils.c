/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "dl-selinux.h"
#అన్తర్భావ "selinux-utils.h"

#నిర్వచితమైతే HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
పూర్ణాంకం ul_setfscreatecon_from_file(అక్షరం *orig_file)
{
	ఒకవేళ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		అక్షరం *scontext = ఖాళీ;

		ఒకవేళ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			తిరిగి -1;
		ఒకవేళ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			తిరిగి -1;
		}
		selinux_call(freecon)(scontext);
	}
	తిరిగి 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
పూర్ణాంకం ul_selinux_has_access(స్థిరరాశి అక్షరం *classstr, స్థిరరాశి అక్షరం *perm, అక్షరం **user_cxt)
{
	అక్షరం *user;
	పూర్ణాంకం rc;

	ఒకవేళ (user_cxt)
		*user_cxt = ఖాళీ;

	ఒకవేళ (ul_dlopen_libselinux() != 0)
		తిరిగి 0;

	ఒకవేళ (selinux_call(getprevcon)(&user) != 0)
		తిరిగి 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ఖాళీ);
	ఒకవేళ (rc != 0 && user_cxt)
		*user_cxt = user;
	లేకపోతే
		selinux_call(freecon)(user);

	తిరిగి rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
పూర్ణాంకం ul_selinux_get_default_context(స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం st_mode, అక్షరం **cxt)
{
	నిర్మాణం selabel_handle *hnd;
	నిర్మాణం selinux_opt options[SELABEL_NOPT] = {};
	పూర్ణాంకం rc = 0;

	*cxt = ఖాళీ;

	ఒకవేళ (ul_dlopen_libselinux() != 0)
		తిరిగి -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	ఒకవేళ (!hnd)
		తిరిగి -errno;

	ఒకవేళ (selinux_call(selabel_lookup)(hnd, cxt, మార్గం, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	తిరిగి rc;
}

#అయితే_ముగింపు /* HAVE_LIBSELINUX */
