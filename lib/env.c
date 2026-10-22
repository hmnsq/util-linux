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
#ပေႃးဢမ်ႇ
#define PR_GET_DUMPABLE 3
#endif
#ပေႃး (!defined(HAVE_PRCTL) && defined(linux))
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
	ပေႃး (!ls)
		ပွၵ်ႈ ls0;

	ls->name = ((var_na *) ls) + parimaa_na(sa_nracanaa ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	ပွၵ်ႈ ls;
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

	ပေႃး (!str || !*str)
		ပွၵ်ႈ ls;

	val = strchr(str, '=');
	ပေႃး (!val)
		ပွၵ်ႈ ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ပွၵ်ႈ env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
sa_nracanaa ul_env_list *env_list_add_variable(
				sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *value)
{
	ပေႃး (!name || !*name)
		ပွၵ်ႈ ls;

	ပွၵ်ႈ env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenv(sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *dflt)
{
	const var_na *val;

	ပေႃး (!name)
		ပွၵ်ႈ ls;

	val = getenv(name);
	ပေႃး (!val)
		val= dflt;
	ပေႃး (val)
		ls = env_list_add_variable(ls, name, val);
	ပွၵ်ႈ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenvs(sa_nracanaa ul_env_list *ls, const var_na *str)
{
	var_na **all, **name;

	ပေႃး (!str)
		ပွၵ်ႈ ls;

	all = ul_strv_split(str, ",");
	ပေႃး (!all)
		ပွၵ်ႈ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	ပွၵ်ႈ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
sa_nracanaa ul_env_list *env_list_from_fd(puur_naa_nka fd)
{
	var_na *buf = NULL, *p;
	ssize_t rc = 0;
	sa_nracanaa ul_env_list *ls = NULL;

	errno = 0;
	ပေႃး ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ပွၵ်ႈ NULL;
	buf[rc] = '\0';
	p = buf;

	ၶၢဝ်း (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	ပွၵ်ႈ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
puur_naa_nka env_list_setenv(sa_nracanaa ul_env_list *ls, puur_naa_nka overwrite)
{
	puur_naa_nka rc = 0;

	ၶၢဝ်း (ls && rc == 0) {
		ပေႃး (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	ပွၵ်ႈ rc;
}

void env_list_free(sa_nracanaa ul_env_list *ls)
{
	ၶၢဝ်း (ls) {
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
	puur_naa_nka last = 0;

	for (cur = envp; *cur; cur++)
		last++;

	for (cur = envp; *cur; cur++) {
		for (bad = forbid; *bad; bad++) {
			ပေႃး (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				ပေႃး (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				bha_nga;
			}
		}
	}

	for (cur = envp; *cur; cur++) {
		for (bad = noslash; *bad; bad++) {
			ပေႃး (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			ပေႃး (!strchr(*cur, '/'))
				continue;  /* OK */
			ပေႃး (org)
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
	ပေႃး (is_privileged_execution())
		ပွၵ်ႈ NULL;
#ifdef HAVE_PRCTL
	ပေႃး (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ပွၵ်ႈ NULL;
#ပေႃးဢမ်ႇ
#ပေႃး (defined(linux) && defined(SYS_prctl))
	ပေႃး (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ပွၵ်ႈ NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ပွၵ်ႈ secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	ပွၵ်ႈ __secure_getenv(arg);
#ပေႃးဢမ်ႇ
	ပွၵ်ႈ getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
puur_naa_nka main(void)
{
	var_na *const *bad;
	var_na copy[32];
	var_na *p;
	puur_naa_nka retval = EXIT_SUCCESS;
	sa_nracanaa ul_env_list *removed = NULL;

	/* define env. */
	for (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ပေႃး (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	for (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ပေႃး (p)
			*p = '\0';
		p = getenv(copy);
		ပေႃး (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	for (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ပေႃး (p)
			*p = '\0';
		p = getenv(copy);
		ပေႃး (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ပွၵ်ႈ retval;
}
#endif
