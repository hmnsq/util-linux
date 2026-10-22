/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#അന്തര്ഭാവ <sys/prctl.h>
#അല്ലെങ്കിൽ
#define PR_GET_DUMPABLE 3
#endif
#എങ്കിൽ (!defined(HAVE_PRCTL) && defined(linux))
#അന്തര്ഭാവ <sys/syscall.h>
#endif
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "env.h"
#അന്തര്ഭാവ "strv.h"
#അന്തര്ഭാവ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
ബാഹ്യ അക്ഷരം **environ;
#endif

സ്ഥിതം അക്ഷരം * സ്ഥിരം forbid[] = {
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
	(അക്ഷരം *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
സ്ഥിതം അക്ഷരം * സ്ഥിരം noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(അക്ഷരം *) 0
};


ഘടന ul_env_list {
	അക്ഷരം	*name;
	അക്ഷരം	*മൂല്യം;

	ഘടന ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
സ്ഥിതം ഘടന ul_env_list *env_list_add( ഘടന ul_env_list *ls0,
					 സ്ഥിരം അക്ഷരം *name, size_t namesz,
					 സ്ഥിരം അക്ഷരം *മൂല്യം, size_t valsz)
{
	ഘടന ul_env_list *ls;

	ls = calloc(1, വലുപ്പം(ഘടന ul_env_list) + namesz + valsz + 2);
	എങ്കിൽ (!ls)
		തിരികെ ls0;

	ls->name = ((അക്ഷരം *) ls) + വലുപ്പം(ഘടന ul_env_list);
	ls->മൂല്യം = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->മൂല്യം, മൂല്യം, valsz);

	ls->next = ls0;
	തിരികെ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
സ്ഥിതം ഘടന ul_env_list *env_list_add_from_string(ഘടന ul_env_list *ls,
						    സ്ഥിരം അക്ഷരം *str)
{
	size_t namesz = 0, valsz = 0;
	സ്ഥിരം അക്ഷരം *val;

	എങ്കിൽ (!str || !*str)
		തിരികെ ls;

	val = strchr(str, '=');
	എങ്കിൽ (!val)
		തിരികെ ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	തിരികെ env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ഘടന ul_env_list *env_list_add_variable(
				ഘടന ul_env_list *ls,
				സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	എങ്കിൽ (!name || !*name)
		തിരികെ ls;

	തിരികെ env_list_add(ls, name, strlen(name), മൂല്യം, മൂല്യം ? strlen(മൂല്യം) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ഘടന ul_env_list *env_list_add_getenv(ഘടന ul_env_list *ls,
				സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *dflt)
{
	സ്ഥിരം അക്ഷരം *val;

	എങ്കിൽ (!name)
		തിരികെ ls;

	val = getenv(name);
	എങ്കിൽ (!val)
		val= dflt;
	എങ്കിൽ (val)
		ls = env_list_add_variable(ls, name, val);
	തിരികെ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ഘടന ul_env_list *env_list_add_getenvs(ഘടന ul_env_list *ls, സ്ഥിരം അക്ഷരം *str)
{
	അക്ഷരം **all, **name;

	എങ്കിൽ (!str)
		തിരികെ ls;

	all = ul_strv_split(str, ",");
	എങ്കിൽ (!all)
		തിരികെ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, ശൂന്യ);

	ul_strv_free(all);
	തിരികെ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ഘടന ul_env_list *env_list_from_fd(പൂർണ്ണസംഖ്യ fd)
{
	അക്ഷരം *buf = ശൂന്യ, *p;
	ssize_t rc = 0;
	ഘടന ul_env_list *ls = ശൂന്യ;

	errno = 0;
	എങ്കിൽ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		തിരികെ ശൂന്യ;
	buf[rc] = '\0';
	p = buf;

	ആകുമ്പോൾ (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	സ്വതന്ത്രം(buf);
	തിരികെ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
പൂർണ്ണസംഖ്യ env_list_setenv(ഘടന ul_env_list *ls, പൂർണ്ണസംഖ്യ overwrite)
{
	പൂർണ്ണസംഖ്യ rc = 0;

	ആകുമ്പോൾ (ls && rc == 0) {
		എങ്കിൽ (ls->name && ls->മൂല്യം)
			rc = setenv(ls->name, ls->മൂല്യം, overwrite);
		ls = ls->next;
	}
	തിരികെ rc;
}

ശൂന്യം env_list_free(ഘടന ul_env_list *ls)
{
	ആകുമ്പോൾ (ls) {
		ഘടന ul_env_list *x = ls;
		ls = ls->next;
		സ്വതന്ത്രം(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
ശൂന്യം __sanitize_env(ഘടന ul_env_list **org)
{
	അക്ഷരം **envp = environ;
	അക്ഷരം * സ്ഥിരം *bad;
	അക്ഷരം **cur;
	പൂർണ്ണസംഖ്യ last = 0;

	വേണ്ടി (cur = envp; *cur; cur++)
		last++;

	വേണ്ടി (cur = envp; *cur; cur++) {
		വേണ്ടി (bad = forbid; *bad; bad++) {
			എങ്കിൽ (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				എങ്കിൽ (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				നിർത്തുക;
			}
		}
	}

	വേണ്ടി (cur = envp; *cur; cur++) {
		വേണ്ടി (bad = noslash; *bad; bad++) {
			എങ്കിൽ (strncmp(*cur, *bad, strlen(*bad)) != 0)
				തുടരുക;
			എങ്കിൽ (!strchr(*cur, '/'))
				തുടരുക;  /* OK */
			എങ്കിൽ (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			നിർത്തുക;
		}
	}
}

ശൂന്യം sanitize_env(ശൂന്യം)
{
	__sanitize_env(ശൂന്യ);
}

അക്ഷരം *safe_getenv(സ്ഥിരം അക്ഷരം *arg)
{
	എങ്കിൽ (is_privileged_execution())
		തിരികെ ശൂന്യ;
#ifdef HAVE_PRCTL
	എങ്കിൽ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		തിരികെ ശൂന്യ;
#അല്ലെങ്കിൽ
#എങ്കിൽ (defined(linux) && defined(SYS_prctl))
	എങ്കിൽ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		തിരികെ ശൂന്യ;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
തിരികെ secure_getenv(arg);
#അല്ലെങ്കിലെങ്കിൽ HAVE___SECURE_GETENV
	തിരികെ __secure_getenv(arg);
#അല്ലെങ്കിൽ
	തിരികെ getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
	അക്ഷരം *സ്ഥിരം *bad;
	അക്ഷരം copy[32];
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ retval = EXIT_SUCCESS;
	ഘടന ul_env_list *removed = ശൂന്യ;

	/* define env. */
	വേണ്ടി (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		എങ്കിൽ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	വേണ്ടി (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		എങ്കിൽ (p)
			*p = '\0';
		p = getenv(copy);
		എങ്കിൽ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	വേണ്ടി (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		എങ്കിൽ (p)
			*p = '\0';
		p = getenv(copy);
		എങ്കിൽ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	തിരികെ retval;
}
#endif
