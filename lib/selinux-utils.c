/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [January 2021]
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "dl-selinux.h"
#അന്തര്ഭാവ "selinux-utils.h"

#ifdef HAVE_LIBSELINUX

/* set the SELinux security context used for _creating_ a new file system object
 *
 * returns 0 on success,
 *     or <0 on error
 */
പൂർണ്ണസംഖ്യ ul_setfscreatecon_from_file(അക്ഷരം *orig_file)
{
	എങ്കിൽ (ul_dlopen_libselinux() == 0
	    && selinux_call(is_selinux_enabled)() > 0) {
		അക്ഷരം *scontext = ശൂന്യ;

		എങ്കിൽ (selinux_call(getfilecon)(orig_file, &scontext) < 0)
			തിരികെ -1;
		എങ്കിൽ (selinux_call(setfscreatecon)(scontext) < 0) {
			selinux_call(freecon)(scontext);
			തിരികെ -1;
		}
		selinux_call(freecon)(scontext);
	}
	തിരികെ 0;
}

/* returns 1 if user has access to @class and @perm ("passwd", "chfn")
 *	or 0 on error,
 *	or 0 if has no access -- in this case sets @user_cxt to user-context
 */
പൂർണ്ണസംഖ്യ ul_selinux_has_access(സ്ഥിരം അക്ഷരം *classstr, സ്ഥിരം അക്ഷരം *perm, അക്ഷരം **user_cxt)
{
	അക്ഷരം *user;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (user_cxt)
		*user_cxt = ശൂന്യ;

	എങ്കിൽ (ul_dlopen_libselinux() != 0)
		തിരികെ 0;

	എങ്കിൽ (selinux_call(getprevcon)(&user) != 0)
		തിരികെ 0;

	rc = selinux_call(selinux_check_access)(user, user, classstr, perm, ശൂന്യ);
	എങ്കിൽ (rc != 0 && user_cxt)
		*user_cxt = user;
	അല്ലെങ്കിൽ
		selinux_call(freecon)(user);

	തിരികെ rc == 0 ? 1 : 0;
}

/* Gets the default context for @path and @st_mode.
 *
 * returns 0 on success,
 *     or <0 on error
 */
പൂർണ്ണസംഖ്യ ul_selinux_get_default_context(സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ st_mode, അക്ഷരം **cxt)
{
	ഘടന selabel_handle *hnd;
	ഘടന selinux_opt options[SELABEL_NOPT] = {};
	പൂർണ്ണസംഖ്യ rc = 0;

	*cxt = ശൂന്യ;

	എങ്കിൽ (ul_dlopen_libselinux() != 0)
		തിരികെ -ENOSYS;

	hnd = selinux_call(selabel_open)(SELABEL_CTX_FILE, options, SELABEL_NOPT);
	എങ്കിൽ (!hnd)
		തിരികെ -errno;

	എങ്കിൽ (selinux_call(selabel_lookup)(hnd, cxt, path, st_mode) != 0)
		rc = -errno;
	selinux_call(selabel_close)(hnd);

	തിരികെ rc;
}

#endif /* HAVE_LIBSELINUX */
