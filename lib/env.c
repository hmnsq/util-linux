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
#မဟုတ်ပါက
#define PR_GET_DUMPABLE 3
#endif
#အကယ်၍ (!defined(HAVE_PRCTL) && defined(linux))
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
	အကယ်၍ (!ls)
		ပြန်ပေး ls0;

	ls->name = ((var_na *) ls) + parimaa_na(sa_nracanaa ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	ပြန်ပေး ls;
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

	အကယ်၍ (!str || !*str)
		ပြန်ပေး ls;

	val = strchr(str, '=');
	အကယ်၍ (!val)
		ပြန်ပေး ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ပြန်ပေး env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
sa_nracanaa ul_env_list *env_list_add_variable(
				sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *value)
{
	အကယ်၍ (!name || !*name)
		ပြန်ပေး ls;

	ပြန်ပေး env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenv(sa_nracanaa ul_env_list *ls,
				const var_na *name, const var_na *dflt)
{
	const var_na *val;

	အကယ်၍ (!name)
		ပြန်ပေး ls;

	val = getenv(name);
	အကယ်၍ (!val)
		val= dflt;
	အကယ်၍ (val)
		ls = env_list_add_variable(ls, name, val);
	ပြန်ပေး ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
sa_nracanaa ul_env_list *env_list_add_getenvs(sa_nracanaa ul_env_list *ls, const var_na *str)
{
	var_na **all, **name;

	အကယ်၍ (!str)
		ပြန်ပေး ls;

	all = ul_strv_split(str, ",");
	အကယ်၍ (!all)
		ပြန်ပေး ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	ပြန်ပေး ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
sa_nracanaa ul_env_list *env_list_from_fd(ကိန်းပြည့် fd)
{
	var_na *buf = NULL, *p;
	ssize_t rc = 0;
	sa_nracanaa ul_env_list *ls = NULL;

	errno = 0;
	အကယ်၍ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ပြန်ပေး NULL;
	buf[rc] = '\0';
	p = buf;

	နေစဉ် (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	ပြန်ပေး ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ကိန်းပြည့် env_list_setenv(sa_nracanaa ul_env_list *ls, ကိန်းပြည့် overwrite)
{
	ကိန်းပြည့် rc = 0;

	နေစဉ် (ls && rc == 0) {
		အကယ်၍ (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	ပြန်ပေး rc;
}

void env_list_free(sa_nracanaa ul_env_list *ls)
{
	နေစဉ် (ls) {
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
	ကိန်းပြည့် last = 0;

	အတွက် (cur = envp; *cur; cur++)
		last++;

	အတွက် (cur = envp; *cur; cur++) {
		အတွက် (bad = forbid; *bad; bad++) {
			အကယ်၍ (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				အကယ်၍ (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				bha_nga;
			}
		}
	}

	အတွက် (cur = envp; *cur; cur++) {
		အတွက် (bad = noslash; *bad; bad++) {
			အကယ်၍ (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			အကယ်၍ (!strchr(*cur, '/'))
				continue;  /* OK */
			အကယ်၍ (org)
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
	အကယ်၍ (is_privileged_execution())
		ပြန်ပေး NULL;
#ifdef HAVE_PRCTL
	အကယ်၍ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ပြန်ပေး NULL;
#မဟုတ်ပါက
#အကယ်၍ (defined(linux) && defined(SYS_prctl))
	အကယ်၍ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ပြန်ပေး NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ပြန်ပေး secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	ပြန်ပေး __secure_getenv(arg);
#မဟုတ်ပါက
	ပြန်ပေး getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
ကိန်းပြည့် main(void)
{
	var_na *const *bad;
	var_na copy[32];
	var_na *p;
	ကိန်းပြည့် retval = EXIT_SUCCESS;
	sa_nracanaa ul_env_list *removed = NULL;

	/* define env. */
	အတွက် (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		အကယ်၍ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	အတွက် (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		အကယ်၍ (p)
			*p = '\0';
		p = getenv(copy);
		အကယ်၍ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	အတွက် (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		အကယ်၍ (p)
			*p = '\0';
		p = getenv(copy);
		အကယ်၍ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ပြန်ပေး retval;
}
#endif
