/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#ifdef HAVE_SYS_PRCTL_H
#شامل <sys/prctl.h>
#وگرنه
#define PR_GET_DUMPABLE 3
#endif
#اگر (!defined(HAVE_PRCTL) && defined(linux))
#شامل <sys/syscall.h>
#endif
#شامل <unistd.h>
#شامل <sys/types.h>

#شامل "env.h"
#شامل "strv.h"
#شامل "all-io.h"

#ifndef HAVE_ENVIRON_DECL
بیرونی نویسه **environ;
#endif

ایستا نویسه * ثابت forbid[] = {
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
	(نویسه *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ایستا نویسه * ثابت noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(نویسه *) 0
};


ساختار ul_env_list {
	نویسه	*name;
	نویسه	*مقدار;

	ساختار ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ایستا ساختار ul_env_list *env_list_add( ساختار ul_env_list *ls0,
					 ثابت نویسه *name, size_t namesz,
					 ثابت نویسه *مقدار, size_t valsz)
{
	ساختار ul_env_list *ls;

	ls = calloc(1, اندازه(ساختار ul_env_list) + namesz + valsz + 2);
	اگر (!ls)
		بازگشت ls0;

	ls->name = ((نویسه *) ls) + اندازه(ساختار ul_env_list);
	ls->مقدار = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->مقدار, مقدار, valsz);

	ls->next = ls0;
	بازگشت ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ایستا ساختار ul_env_list *env_list_add_from_string(ساختار ul_env_list *ls,
						    ثابت نویسه *str)
{
	size_t namesz = 0, valsz = 0;
	ثابت نویسه *val;

	اگر (!str || !*str)
		بازگشت ls;

	val = strchr(str, '=');
	اگر (!val)
		بازگشت ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	بازگشت env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ساختار ul_env_list *env_list_add_variable(
				ساختار ul_env_list *ls,
				ثابت نویسه *name, ثابت نویسه *مقدار)
{
	اگر (!name || !*name)
		بازگشت ls;

	بازگشت env_list_add(ls, name, strlen(name), مقدار, مقدار ? strlen(مقدار) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ساختار ul_env_list *env_list_add_getenv(ساختار ul_env_list *ls,
				ثابت نویسه *name, ثابت نویسه *dflt)
{
	ثابت نویسه *val;

	اگر (!name)
		بازگشت ls;

	val = getenv(name);
	اگر (!val)
		val= dflt;
	اگر (val)
		ls = env_list_add_variable(ls, name, val);
	بازگشت ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ساختار ul_env_list *env_list_add_getenvs(ساختار ul_env_list *ls, ثابت نویسه *str)
{
	نویسه **all, **name;

	اگر (!str)
		بازگشت ls;

	all = ul_strv_split(str, ",");
	اگر (!all)
		بازگشت ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, پوچ);

	ul_strv_free(all);
	بازگشت ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ساختار ul_env_list *env_list_from_fd(صحیح fd)
{
	نویسه *buf = پوچ, *p;
	ssize_t rc = 0;
	ساختار ul_env_list *ls = پوچ;

	errno = 0;
	اگر ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		بازگشت پوچ;
	buf[rc] = '\0';
	p = buf;

	وقتی (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	آزاد(buf);
	بازگشت ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
صحیح env_list_setenv(ساختار ul_env_list *ls, صحیح overwrite)
{
	صحیح rc = 0;

	وقتی (ls && rc == 0) {
		اگر (ls->name && ls->مقدار)
			rc = setenv(ls->name, ls->مقدار, overwrite);
		ls = ls->next;
	}
	بازگشت rc;
}

تهی env_list_free(ساختار ul_env_list *ls)
{
	وقتی (ls) {
		ساختار ul_env_list *x = ls;
		ls = ls->next;
		آزاد(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
تهی __sanitize_env(ساختار ul_env_list **org)
{
	نویسه **envp = environ;
	نویسه * ثابت *bad;
	نویسه **cur;
	صحیح last = 0;

	برای (cur = envp; *cur; cur++)
		last++;

	برای (cur = envp; *cur; cur++) {
		برای (bad = forbid; *bad; bad++) {
			اگر (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				اگر (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				بشکن;
			}
		}
	}

	برای (cur = envp; *cur; cur++) {
		برای (bad = noslash; *bad; bad++) {
			اگر (strncmp(*cur, *bad, strlen(*bad)) != 0)
				ادامه;
			اگر (!strchr(*cur, '/'))
				ادامه;  /* OK */
			اگر (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			بشکن;
		}
	}
}

تهی sanitize_env(تهی)
{
	__sanitize_env(پوچ);
}

نویسه *safe_getenv(ثابت نویسه *arg)
{
	اگر (is_privileged_execution())
		بازگشت پوچ;
#ifdef HAVE_PRCTL
	اگر (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		بازگشت پوچ;
#وگرنه
#اگر (defined(linux) && defined(SYS_prctl))
	اگر (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		بازگشت پوچ;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
بازگشت secure_getenv(arg);
#وگرنهاگر HAVE___SECURE_GETENV
	بازگشت __secure_getenv(arg);
#وگرنه
	بازگشت getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
صحیح اصلی(تهی)
{
	نویسه *ثابت *bad;
	نویسه copy[32];
	نویسه *p;
	صحیح retval = EXIT_SUCCESS;
	ساختار ul_env_list *removed = پوچ;

	/* define env. */
	برای (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	برای (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		p = getenv(copy);
		اگر (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	برای (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		p = getenv(copy);
		اگر (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	بازگشت retval;
}
#endif
