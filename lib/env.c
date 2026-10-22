/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_PRCTL_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/prctl.h>
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PR_GET_DUMPABLE 3
#ਜੇ_ਅੰਤ
#ਜੇ (!defined(HAVE_PRCTL) && defined(linux))
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syscall.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "env.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strv.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_ENVIRON_DECL
ਬਾਹਰੀ ਅੱਖਰ **environ;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅੱਖਰ * ਅਚਲ forbid[] = {
	"BASH_ENV=",    /* GNU creeping featurism strikes again... */
	"ENV=",
	"HOME=",
	"IFS=",
	"KRB_CONF=",
	"LD_",	  /* anything with the LD_ prefix */
	"LIBPATH=",
	"MAIL=",
	"NLSPATH=",
	"PATH=",
	"SHELL=",
	"SHLIB_PATH=",
	(ਅੱਖਰ *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ਸਥਿਰ ਅੱਖਰ * ਅਚਲ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(ਅੱਖਰ *) 0
};


ਬਣਤਰ ul_env_list {
	ਅੱਖਰ	*name;
	ਅੱਖਰ	*ਮੁੱਲ;

	ਬਣਤਰ ul_env_list *ਅਗਲਾ;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ਸਥਿਰ ਬਣਤਰ ul_env_list *env_list_ਜੋੜੋ( ਬਣਤਰ ul_env_list *ls0,
					 ਅਚਲ ਅੱਖਰ *name, size_t namesz,
					 ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, size_t valsz)
{
	ਬਣਤਰ ul_env_list *ls;

	ls = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ul_env_list) + namesz + valsz + 2);
	ਜੇ (!ls)
		ਵਾਪਸ ls0;

	ls->name = ((ਅੱਖਰ *) ls) + ਆਕਾਰ(ਬਣਤਰ ul_env_list);
	ls->ਮੁੱਲ = ls->name + namesz + 1;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ls->name, name, namesz);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(ls->ਮੁੱਲ, ਮੁੱਲ, valsz);

	ls->ਅਗਲਾ = ls0;
	ਵਾਪਸ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ਸਥਿਰ ਬਣਤਰ ul_env_list *env_list_ਜੋੜੋ_from_string(ਬਣਤਰ ul_env_list *ls,
						    ਅਚਲ ਅੱਖਰ *str)
{
	size_t namesz = 0, valsz = 0;
	ਅਚਲ ਅੱਖਰ *val;

	ਜੇ (!str || !*str)
		ਵਾਪਸ ls;

	val = strchr(str, '=');
	ਜੇ (!val)
		ਵਾਪਸ ls;
	namesz = val - str;

	val++;
	valsz = ਲੜੀ_ਲੰਬਾਈ(val);

	ਵਾਪਸ env_list_ਜੋੜੋ(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ਬਣਤਰ ul_env_list *env_list_ਜੋੜੋ_variable(
				ਬਣਤਰ ul_env_list *ls,
				ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਜੇ (!name || !*name)
		ਵਾਪਸ ls;

	ਵਾਪਸ env_list_ਜੋੜੋ(ls, name, ਲੜੀ_ਲੰਬਾਈ(name), ਮੁੱਲ, ਮੁੱਲ ? ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ਬਣਤਰ ul_env_list *env_list_ਜੋੜੋ_getenv(ਬਣਤਰ ul_env_list *ls,
				ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *dflt)
{
	ਅਚਲ ਅੱਖਰ *val;

	ਜੇ (!name)
		ਵਾਪਸ ls;

	val = getenv(name);
	ਜੇ (!val)
		val= dflt;
	ਜੇ (val)
		ls = env_list_ਜੋੜੋ_variable(ls, name, val);
	ਵਾਪਸ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ਬਣਤਰ ul_env_list *env_list_ਜੋੜੋ_getenvs(ਬਣਤਰ ul_env_list *ls, ਅਚਲ ਅੱਖਰ *str)
{
	ਅੱਖਰ **all, **name;

	ਜੇ (!str)
		ਵਾਪਸ ls;

	all = ul_strv_ਵੰਡੋ(str, ",");
	ਜੇ (!all)
		ਵਾਪਸ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_ਜੋੜੋ_getenv(ls, *name, ਨਿਰਾ);

	ul_strv_free(all);
	ਵਾਪਸ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ਬਣਤਰ ul_env_list *env_list_from_fd(ਪੂਰਨ fd)
{
	ਅੱਖਰ *buf = ਨਿਰਾ, *p;
	ssize_t rc = 0;
	ਬਣਤਰ ul_env_list *ls = ਨਿਰਾ;

	errno = 0;
	ਜੇ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ਵਾਪਸ ਨਿਰਾ;
	buf[rc] = '\0';
	p = buf;

	ਜਦੋਂ (rc > 0) {
		ls = env_list_ਜੋੜੋ_from_string(ls, p);
		p += ਲੜੀ_ਲੰਬਾਈ(p) + 1;
		rc -= ਲੜੀ_ਲੰਬਾਈ(p) + 1;
	}

	ਮੁਕਤ(buf);
	ਵਾਪਸ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ਪੂਰਨ env_list_setenv(ਬਣਤਰ ul_env_list *ls, ਪੂਰਨ overwrite)
{
	ਪੂਰਨ rc = 0;

	ਜਦੋਂ (ls && rc == 0) {
		ਜੇ (ls->name && ls->ਮੁੱਲ)
			rc = setenv(ls->name, ls->ਮੁੱਲ, overwrite);
		ls = ls->ਅਗਲਾ;
	}
	ਵਾਪਸ rc;
}

ਖਾਲੀ env_list_free(ਬਣਤਰ ul_env_list *ls)
{
	ਜਦੋਂ (ls) {
		ਬਣਤਰ ul_env_list *x = ls;
		ls = ls->ਅਗਲਾ;
		ਮੁਕਤ(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
ਖਾਲੀ __sanitize_env(ਬਣਤਰ ul_env_list **org)
{
	ਅੱਖਰ **envp = environ;
	ਅੱਖਰ * ਅਚਲ *bad;
	ਅੱਖਰ **cur;
	ਪੂਰਨ ਆਖਰੀ = 0;

	ਲਈ (cur = envp; *cur; cur++)
		ਆਖਰੀ++;

	ਲਈ (cur = envp; *cur; cur++) {
		ਲਈ (bad = forbid; *bad; bad++) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(*cur, *bad, ਲੜੀ_ਲੰਬਾਈ(*bad)) == 0) {
				ਜੇ (org)
					*org = env_list_ਜੋੜੋ_from_string(*org, *cur);
				ਆਖਰੀ = ul_remove_ਇੰਦਰਾਜ(envp, cur - envp, ਆਖਰੀ);
				cur--;
				ਰੋਕੋ;
			}
		}
	}

	ਲਈ (cur = envp; *cur; cur++) {
		ਲਈ (bad = noslash; *bad; bad++) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(*cur, *bad, ਲੜੀ_ਲੰਬਾਈ(*bad)) != 0)
				ਜਾਰੀ;
			ਜੇ (!strchr(*cur, '/'))
				ਜਾਰੀ;  /* OK */
			ਜੇ (org)
				*org = env_list_ਜੋੜੋ_from_string(*org, *cur);
			ਆਖਰੀ = ul_remove_ਇੰਦਰਾਜ(envp, cur - envp, ਆਖਰੀ);
			cur--;
			ਰੋਕੋ;
		}
	}
}

ਖਾਲੀ sanitize_env(ਖਾਲੀ)
{
	__sanitize_env(ਨਿਰਾ);
}

ਅੱਖਰ *ਸੁਰੱਖਿਅਤ_getenv(ਅਚਲ ਅੱਖਰ *arg)
{
	ਜੇ (is_privileged_execution())
		ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PRCTL
	ਜੇ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ਵਾਪਸ ਨਿਰਾ;
#ਨਹੀਂਤਾਂ
#ਜੇ (defined(linux) && defined(SYS_prctl))
	ਜੇ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SECURE_GETENV
ਵਾਪਸ secure_getenv(arg);
#ਜਾਂਜੇ HAVE___SECURE_GETENV
	ਵਾਪਸ __secure_getenv(arg);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ getenv(arg);
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_ENV
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
	ਅੱਖਰ *ਅਚਲ *bad;
	ਅੱਖਰ copy[32];
	ਅੱਖਰ *p;
	ਪੂਰਨ retval = EXIT_SUCCESS;
	ਬਣਤਰ ul_env_list *removed = ਨਿਰਾ;

	/* define env. */
	ਲਈ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ਜੇ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	ਲਈ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ਜੇ (p)
			*p = '\0';
		p = getenv(copy);
		ਜੇ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	ਲਈ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ਜੇ (p)
			*p = '\0';
		p = getenv(copy);
		ਜੇ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ਵਾਪਸ retval;
}
#ਜੇ_ਅੰਤ
