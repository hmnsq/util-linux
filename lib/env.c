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
#아니면
#define PR_GET_DUMPABLE 3
#endif
#만약 (!defined(HAVE_PRCTL) && defined(linux))
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
	만약 (!ls)
		반환 ls0;

	ls->name = ((char *) ls) + sizeof(struct ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	반환 ls;
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

	만약 (!str || !*str)
		반환 ls;

	val = strchr(str, '=');
	만약 (!val)
		반환 ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	반환 env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
struct ul_env_list *env_list_add_variable(
				struct ul_env_list *ls,
				const char *name, const char *value)
{
	만약 (!name || !*name)
		반환 ls;

	반환 env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
struct ul_env_list *env_list_add_getenv(struct ul_env_list *ls,
				const char *name, const char *dflt)
{
	const char *val;

	만약 (!name)
		반환 ls;

	val = getenv(name);
	만약 (!val)
		val= dflt;
	만약 (val)
		ls = env_list_add_variable(ls, name, val);
	반환 ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
struct ul_env_list *env_list_add_getenvs(struct ul_env_list *ls, const char *str)
{
	char **all, **name;

	만약 (!str)
		반환 ls;

	all = ul_strv_split(str, ",");
	만약 (!all)
		반환 ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	반환 ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
struct ul_env_list *env_list_from_fd(정수 fd)
{
	char *buf = NULL, *p;
	ssize_t rc = 0;
	struct ul_env_list *ls = NULL;

	errno = 0;
	만약 ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		반환 NULL;
	buf[rc] = '\0';
	p = buf;

	동안 (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	반환 ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
정수 env_list_setenv(struct ul_env_list *ls, 정수 overwrite)
{
	정수 rc = 0;

	동안 (ls && rc == 0) {
		만약 (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	반환 rc;
}

void env_list_free(struct ul_env_list *ls)
{
	동안 (ls) {
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
	정수 last = 0;

	위해 (cur = envp; *cur; cur++)
		last++;

	위해 (cur = envp; *cur; cur++) {
		위해 (bad = forbid; *bad; bad++) {
			만약 (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				만약 (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				break;
			}
		}
	}

	위해 (cur = envp; *cur; cur++) {
		위해 (bad = noslash; *bad; bad++) {
			만약 (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			만약 (!strchr(*cur, '/'))
				continue;  /* OK */
			만약 (org)
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
	만약 (is_privileged_execution())
		반환 NULL;
#ifdef HAVE_PRCTL
	만약 (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		반환 NULL;
#아니면
#만약 (defined(linux) && defined(SYS_prctl))
	만약 (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		반환 NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
반환 secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	반환 __secure_getenv(arg);
#아니면
	반환 getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
정수 main(void)
{
	char *const *bad;
	char copy[32];
	char *p;
	정수 retval = EXIT_SUCCESS;
	struct ul_env_list *removed = NULL;

	/* define env. */
	위해 (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		만약 (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	위해 (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		만약 (p)
			*p = '\0';
		p = getenv(copy);
		만약 (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	위해 (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		만약 (p)
			*p = '\0';
		p = getenv(copy);
		만약 (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	반환 retval;
}
#endif
