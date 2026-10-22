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
#инако
#define PR_GET_DUMPABLE 3
#endif
#ако (!defined(HAVE_PRCTL) && defined(linux))
#include <sys/syscall.h>
#endif
#include <unistd.h>
#include <sys/types.h>

#include "env.h"
#include "strv.h"
#include "all-io.h"

#ifndef HAVE_ENVIRON_DECL
extern char **environ;
#endif

static char * const forbid[] = {
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
	(char *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
static char * const noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(char *) 0
};


struct ul_env_list {
	char	*name;
	char	*value;

	struct ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
static struct ul_env_list *env_list_add( struct ul_env_list *ls0,
					 const char *name, size_t namesz,
					 const char *value, size_t valsz)
{
	struct ul_env_list *ls;

	ls = calloc(1, sizeof(struct ul_env_list) + namesz + valsz + 2);
	ако (!ls)
		врати ls0;

	ls->name = ((char *) ls) + sizeof(struct ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	врати ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
static struct ul_env_list *env_list_add_from_string(struct ul_env_list *ls,
						    const char *str)
{
	size_t namesz = 0, valsz = 0;
	const char *val;

	ако (!str || !*str)
		врати ls;

	val = strchr(str, '=');
	ако (!val)
		врати ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	врати env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
struct ul_env_list *env_list_add_variable(
				struct ul_env_list *ls,
				const char *name, const char *value)
{
	ако (!name || !*name)
		врати ls;

	врати env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
struct ul_env_list *env_list_add_getenv(struct ul_env_list *ls,
				const char *name, const char *dflt)
{
	const char *val;

	ако (!name)
		врати ls;

	val = getenv(name);
	ако (!val)
		val= dflt;
	ако (val)
		ls = env_list_add_variable(ls, name, val);
	врати ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
struct ul_env_list *env_list_add_getenvs(struct ul_env_list *ls, const char *str)
{
	char **all, **name;

	ако (!str)
		врати ls;

	all = ul_strv_split(str, ",");
	ако (!all)
		врати ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	врати ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
struct ul_env_list *env_list_from_fd(цео fd)
{
	char *buf = NULL, *p;
	ssize_t rc = 0;
	struct ul_env_list *ls = NULL;

	errno = 0;
	ако ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		врати NULL;
	buf[rc] = '\0';
	p = buf;

	док (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	врати ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
цео env_list_setenv(struct ul_env_list *ls, цео overwrite)
{
	цео rc = 0;

	док (ls && rc == 0) {
		ако (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	врати rc;
}

void env_list_free(struct ul_env_list *ls)
{
	док (ls) {
		struct ul_env_list *x = ls;
		ls = ls->next;
		free(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
void __sanitize_env(struct ul_env_list **org)
{
	char **envp = environ;
	char * const *bad;
	char **cur;
	цео last = 0;

	за (cur = envp; *cur; cur++)
		last++;

	за (cur = envp; *cur; cur++) {
		за (bad = forbid; *bad; bad++) {
			ако (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				ако (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				break;
			}
		}
	}

	за (cur = envp; *cur; cur++) {
		за (bad = noslash; *bad; bad++) {
			ако (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			ако (!strchr(*cur, '/'))
				continue;  /* OK */
			ако (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			break;
		}
	}
}

void sanitize_env(void)
{
	__sanitize_env(NULL);
}

char *safe_getenv(const char *arg)
{
	ако (is_privileged_execution())
		врати NULL;
#ifdef HAVE_PRCTL
	ако (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		врати NULL;
#инако
#ако (defined(linux) && defined(SYS_prctl))
	ако (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		врати NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
врати secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	врати __secure_getenv(arg);
#инако
	врати getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
цео main(void)
{
	char *const *bad;
	char copy[32];
	char *p;
	цео retval = EXIT_SUCCESS;
	struct ul_env_list *removed = NULL;

	/* define env. */
	за (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ако (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	за (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ако (p)
			*p = '\0';
		p = getenv(copy);
		ако (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	за (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ако (p)
			*p = '\0';
		p = getenv(copy);
		ако (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	врати retval;
}
#endif
