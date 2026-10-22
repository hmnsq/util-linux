/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#དེ་མིན
#define PR_GET_DUMPABLE 3
#endif
#གལ་སྲིད (!defined(HAVE_PRCTL) && defined(linux))
#include <sys/syscall.h>
#endif
#include <unistd.h>
#include <sys/types.h>

#include "env.h"
#include "strv.h"
#include "all-io.h"

#ifndef HAVE_ENVIRON_DECL
extern var_na **environ;
#endif

static var_na * const forbid[] = {
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
	(var_na *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
static var_na * const noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(var_na *) 0
};


sa_nracanaa ul_env_list {
	var_na	*name;
	var_na	*value;

	sa_nracanaa ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
static sa_nracanaa ul_env_list *env_list_add( sa_nracanaa ul_env_list *ls0,
					 const var_na *name, size_t namesz,
					 const var_na *value, size_t valsz)
{
	sa_nracanaa ul_env_list *ls;

	ls = calloc(1, parimaa_na(sa_nracanaa ul_env_list) + namesz + valsz + 2);
	གལ་སྲིད (!ls)
		ལོག ls0;

	ls->name = ((var_na *) ls) + parimaa_na(sa_nracanaa ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	ལོག ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
static sa_nracanaa ul_env_list *env_list_add_from_string(sa_nracanaa ul_env_list *ls,
						    const var_na *str)
{
	size_t namesz = 0, valsz = 0;
	const var_na *val;

	གལ་སྲིད (!str || !*str)
		ལོག ls;

	val = strchr(str, '=');
	གལ་སྲིད (!val)
		ལོག ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ལོག env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
sa_nracanaa ul_env_list *env_list_add_variable(
				sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *value)
{
	གལ་སྲིད (!name || !*name)
		ལོག ls;

	ལོག env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenv(sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *dflt)
{
	const var_na *val;

	གལ་སྲིད (!name)
		ལོག ls;

	val = getenv(name);
	གལ་སྲིད (!val)
		val= dflt;
	གལ་སྲིད (val)
		ls = env_list_add_variable(ls, name, val);
	ལོག ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenvs(sa_nracanaa ul_env_list *ls, const var_na *str)
{
	var_na **all, **name;

	གལ་སྲིད (!str)
		ལོག ls;

	all = ul_strv_split(str, ",");
	གལ་སྲིད (!all)
		ལོག ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	ལོག ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
sa_nracanaa ul_env_list *env_list_from_fd(ཧྲིལ fd)
{
	var_na *buf = NULL, *p;
	ssize_t rc = 0;
	sa_nracanaa ul_env_list *ls = NULL;

	errno = 0;
	གལ་སྲིད ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ལོག NULL;
	buf[rc] = '\0';
	p = buf;

	སྐབས (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	ལོག ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ཧྲིལ env_list_setenv(sa_nracanaa ul_env_list *ls, ཧྲིལ overwrite)
{
	ཧྲིལ rc = 0;

	སྐབས (ls && rc == 0) {
		གལ་སྲིད (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	ལོག rc;
}

void env_list_free(sa_nracanaa ul_env_list *ls)
{
	སྐབས (ls) {
		sa_nracanaa ul_env_list *x = ls;
		ls = ls->next;
		free(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
void __sanitize_env(sa_nracanaa ul_env_list **org)
{
	var_na **envp = environ;
	var_na * const *bad;
	var_na **cur;
	ཧྲིལ last = 0;

	དོན་ལུ (cur = envp; *cur; cur++)
		last++;

	དོན་ལུ (cur = envp; *cur; cur++) {
		དོན་ལུ (bad = forbid; *bad; bad++) {
			གལ་སྲིད (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				གལ་སྲིད (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				bha_nga;
			}
		}
	}

	དོན་ལུ (cur = envp; *cur; cur++) {
		དོན་ལུ (bad = noslash; *bad; bad++) {
			གལ་སྲིད (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			གལ་སྲིད (!strchr(*cur, '/'))
				continue;  /* OK */
			གལ་སྲིད (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			bha_nga;
		}
	}
}

void sanitize_env(void)
{
	__sanitize_env(NULL);
}

var_na *safe_getenv(const var_na *arg)
{
	གལ་སྲིད (is_privileged_execution())
		ལོག NULL;
#ifdef HAVE_PRCTL
	གལ་སྲིད (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ལོག NULL;
#དེ་མིན
#གལ་སྲིད (defined(linux) && defined(SYS_prctl))
	གལ་སྲིད (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ལོག NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ལོག secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	ལོག __secure_getenv(arg);
#དེ་མིན
	ལོག getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
ཧྲིལ main(void)
{
	var_na *const *bad;
	var_na copy[32];
	var_na *p;
	ཧྲིལ retval = EXIT_SUCCESS;
	sa_nracanaa ul_env_list *removed = NULL;

	/* define env. */
	དོན་ལུ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		གལ་སྲིད (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	དོན་ལུ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		གལ་སྲིད (p)
			*p = '\0';
		p = getenv(copy);
		གལ་སྲིད (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	དོན་ལུ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		གལ་སྲིད (p)
			*p = '\0';
		p = getenv(copy);
		གལ་སྲིད (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ལོག retval;
}
#endif
