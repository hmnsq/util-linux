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
#інакше
#define PR_GET_DUMPABLE 3
#endif
#якщо (!defined(HAVE_PRCTL) && defined(linux))
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
	якщо (!ls)
		повернути ls0;

	ls->name = ((char *) ls) + sizeof(struct ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	повернути ls;
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

	якщо (!str || !*str)
		повернути ls;

	val = strchr(str, '=');
	якщо (!val)
		повернути ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	повернути env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
struct ul_env_list *env_list_add_variable(
				struct ul_env_list *ls,
				const char *name, const char *value)
{
	якщо (!name || !*name)
		повернути ls;

	повернути env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
struct ul_env_list *env_list_add_getenv(struct ul_env_list *ls,
				const char *name, const char *dflt)
{
	const char *val;

	якщо (!name)
		повернути ls;

	val = getenv(name);
	якщо (!val)
		val= dflt;
	якщо (val)
		ls = env_list_add_variable(ls, name, val);
	повернути ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
struct ul_env_list *env_list_add_getenvs(struct ul_env_list *ls, const char *str)
{
	char **all, **name;

	якщо (!str)
		повернути ls;

	all = ul_strv_split(str, ",");
	якщо (!all)
		повернути ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, NULL);

	ul_strv_free(all);
	повернути ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
struct ul_env_list *env_list_from_fd(ціле fd)
{
	char *buf = NULL, *p;
	ssize_t rc = 0;
	struct ul_env_list *ls = NULL;

	errno = 0;
	якщо ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		повернути NULL;
	buf[rc] = '\0';
	p = buf;

	поки (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	free(buf);
	повернути ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ціле env_list_setenv(struct ul_env_list *ls, ціле overwrite)
{
	ціле rc = 0;

	поки (ls && rc == 0) {
		якщо (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	повернути rc;
}

void env_list_free(struct ul_env_list *ls)
{
	поки (ls) {
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
	ціле last = 0;

	для (cur = envp; *cur; cur++)
		last++;

	для (cur = envp; *cur; cur++) {
		для (bad = forbid; *bad; bad++) {
			якщо (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				якщо (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				break;
			}
		}
	}

	для (cur = envp; *cur; cur++) {
		для (bad = noslash; *bad; bad++) {
			якщо (strncmp(*cur, *bad, strlen(*bad)) != 0)
				continue;
			якщо (!strchr(*cur, '/'))
				continue;  /* OK */
			якщо (org)
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
	якщо (is_privileged_execution())
		повернути NULL;
#ifdef HAVE_PRCTL
	якщо (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		повернути NULL;
#інакше
#якщо (defined(linux) && defined(SYS_prctl))
	якщо (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		повернути NULL;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
повернути secure_getenv(arg);
#elif HAVE___SECURE_GETENV
	повернути __secure_getenv(arg);
#інакше
	повернути getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
ціле main(void)
{
	char *const *bad;
	char copy[32];
	char *p;
	ціле retval = EXIT_SUCCESS;
	struct ul_env_list *removed = NULL;

	/* define env. */
	для (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		якщо (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	для (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		якщо (p)
			*p = '\0';
		p = getenv(copy);
		якщо (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	для (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		якщо (p)
			*p = '\0';
		p = getenv(copy);
		якщо (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	повернути retval;
}
#endif
