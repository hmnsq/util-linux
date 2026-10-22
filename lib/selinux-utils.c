/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "dl-selinux.h"
#અન્તર્ભાવ "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
પૂર્ણાંક ul_setfscreatecon_from_file(અક્ષર *orig_file)
{
	જો (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		અક્ષર *scontext = ખાલી;

		જો (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			પાછા -1;
		જો (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			પાછા -1;
		}
		selinux_call(freecon)(scontext);
	}
	પાછા 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
પૂર્ણાંક ul_selinux_has_access(અચલ અક્ષર *classstr, અચલ અક્ષર *perm, અક્ષર **user_cxt)
{
	અક્ષર *user;
	પૂર્ણાંક rc;

	જો (user_cxt)
		*user_cxt = ખાલી;

	જો (ul_dlopen_libselinux() != 0)
		પાછા 0;

	જો (selinux_call(getprevcon)(&user) != 0)
		પાછા 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ખાલી);
	જો (rc != 0 && user_cxt)
		*user_cxt = user;
	નહીંતર
		selinux_call(freecon)(user);

	પાછા rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
પૂર્ણાંક ul_selinux_get_default_context(અચલ અક્ષર *path, પૂર્ણાંક st_mode, અક્ષર **cxt)
{
	સંરચના selabel_handle *hnd;
	સંરચના selinux_opt options[SELABEL_NOPT] = {};
	પૂર્ણાંક rc = 0;

	*cxt = ખાલી;

	જો (ul_dlopen_libselinux() != 0)
		પાછા -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	જો (!hnd)
		પાછા -errno;

	જો (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	પાછા rc;
}

#endif /* HAVE_LIBSELINUX */
