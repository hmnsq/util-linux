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
#không_thì
#define PR_GET_DUMPABLE 3
#endif
#nếu (!defined(HAVE_PRCTL) && defined(linux))
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
	nếu (!ls)
		trả_về ls0;

	ls->name = ((char *) ls) + sizeof(struct ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	trả_về ls;
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

	nếu (!str || !*str)
		trả_về ls;

	val = strchr(str, '=');
	nếu (!val)
		trả_về ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	trả_về env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
struct ul_env_list *env_list_add_variable(
				struct ul_env_list *ls,
				const char *name, const char *value)
{
	nếu (!name || !*name)
		trả_về ls;

	trả_về env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
struct ul_env_list *env_list_add_getenv(struct ul_env_list *ls,
				const char *name, const char *dflt)
{
	const char *val;

	nếu (!name)
		trả_về ls;

	val = getenv(name);
	nếu (!val)
		val= dflt;
	nếu (val)
		ls = env_list_add_variable(ls, name, val);
	trả_về ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
struct ul_env_list *env_list_add_getenvs(struct ul_env_list *ls, const char *str)
{
	char **all, **name;

	nếu (!str)
		trả_về ls;

	all = ul_strv_split(str, ",");
	nếu (!all)
		trả_về ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	trả_về ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
struct ul_env_list *env_list_from_fd(số_nguyên fd)
{
	char *buf = NULL, *p;
	ssize_t rc = 0;
	struct ul_env_list *ls = NULL;

	errno = 0;
	nếu ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		trả_về NULL;
	buf[rc] = '\0';
	p = buf;

	trong_khi (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	trả_về ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
số_nguyên env_list_setenv(struct ul_env_list *ls, số_nguyên overwrite)
{
	số_nguyên rc = 0;

	trong_khi (ls && rc == 0) {
		nếu (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	trả_về rc;
}

void env_list_free(struct ul_env_list *ls)
{
	trong_khi (ls) {
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
	số_nguyên last = 0;

	cho (cur = envp; *cur; cur++)
		last++;

	cho (cur = envp; *cur; cur++) {
		cho (bad = forbid; *bad; bad++) {
			nếu (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				nếu (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				break;
			}
		}
	}

	cho (cur = envp; *cur; cur++) {
		cho (bad = noslash; *bad; bad++) {
			nếu (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			nếu (!strchr(*cur, '/'))
				continue;  /* OK */
			nếu (org)
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
	nếu (is_privileged_execution())
		trả_về NULL;
#ifdef HAVE_PRCTL
	nếu (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		trả_về NULL;
#không_thì
#nếu (defined(linux) && defined(SYS_prctl))
	nếu (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		trả_về NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
trả_về secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	trả_về __secure_getenv(arg);
#không_thì
	trả_về getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
số_nguyên main(void)
{
	char *const *bad;
	char copy[32];
	char *p;
	số_nguyên retval = EXIT_SUCCESS;
	struct ul_env_list *removed = NULL;

	/* define env. */
	cho (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		nếu (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	cho (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		nếu (p)
			*p = '\0';
		p = getenv(copy);
		nếu (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	cho (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		nếu (p)
			*p = '\0';
		p = getenv(copy);
		nếu (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	trả_về retval;
}
#endif
