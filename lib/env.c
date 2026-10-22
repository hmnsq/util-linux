/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#ifdef HAVE_SYS_PRCTL_H
#অন্তর্ভাব <sys/prctl.h>
#অন্যথা
#define PR_GET_DUMPABLE 3
#endif
#যদি (!defined(HAVE_PRCTL) && defined(linux))
#অন্তর্ভাব <sys/syscall.h>
#endif
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "env.h"
#অন্তর্ভাব "strv.h"
#অন্তর্ভাব "all-io.h"

#ifndef HAVE_ENVIRON_DECL
বাহ্যিক অক্ষর **environ;
#endif

স্থির অক্ষর * ধ্রুবক forbid[] = {
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
	(অক্ষর *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
স্থির অক্ষর * ধ্রুবক noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(অক্ষর *) 0
};


কাঠামো ul_env_list {
	অক্ষর	*name;
	অক্ষর	*মান;

	কাঠামো ul_env_list *পরবর্তী;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
স্থির কাঠামো ul_env_list *env_list_add( কাঠামো ul_env_list *ls0,
					 ধ্রুবক অক্ষর *name, size_t namesz,
					 ধ্রুবক অক্ষর *মান, size_t valsz)
{
	কাঠামো ul_env_list *ls;

	ls = calloc(1, আকার(কাঠামো ul_env_list) + namesz + valsz + 2);
	যদি (!ls)
		ফেরত ls0;

	ls->name = ((অক্ষর *) ls) + আকার(কাঠামো ul_env_list);
	ls->মান = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->মান, মান, valsz);

	ls->পরবর্তী = ls0;
	ফেরত ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
স্থির কাঠামো ul_env_list *env_list_add_from_string(কাঠামো ul_env_list *ls,
						    ধ্রুবক অক্ষর *str)
{
	size_t namesz = 0, valsz = 0;
	ধ্রুবক অক্ষর *val;

	যদি (!str || !*str)
		ফেরত ls;

	val = strchr(str, '=');
	যদি (!val)
		ফেরত ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ফেরত env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
কাঠামো ul_env_list *env_list_add_variable(
				কাঠামো ul_env_list *ls,
				ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান)
{
	যদি (!name || !*name)
		ফেরত ls;

	ফেরত env_list_add(ls, name, strlen(name), মান, মান ? strlen(মান) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
কাঠামো ul_env_list *env_list_add_getenv(কাঠামো ul_env_list *ls,
				ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *dflt)
{
	ধ্রুবক অক্ষর *val;

	যদি (!name)
		ফেরত ls;

	val = getenv(name);
	যদি (!val)
		val= dflt;
	যদি (val)
		ls = env_list_add_variable(ls, name, val);
	ফেরত ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
কাঠামো ul_env_list *env_list_add_getenvs(কাঠামো ul_env_list *ls, ধ্রুবক অক্ষর *str)
{
	অক্ষর **all, **name;

	যদি (!str)
		ফেরত ls;

	all = ul_strv_split(str, ",");
	যদি (!all)
		ফেরত ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, ফাঁকা);

	ul_strv_free(all);
	ফেরত ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
কাঠামো ul_env_list *env_list_from_fd(পূর্ণসংখ্যা fd)
{
	অক্ষর *buf = ফাঁকা, *p;
	ssize_t rc = 0;
	কাঠামো ul_env_list *ls = ফাঁকা;

	errno = 0;
	যদি ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ফেরত ফাঁকা;
	buf[rc] = '\0';
	p = buf;

	যখন (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	মুক্ত(buf);
	ফেরত ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
পূর্ণসংখ্যা env_list_setenv(কাঠামো ul_env_list *ls, পূর্ণসংখ্যা overwrite)
{
	পূর্ণসংখ্যা rc = 0;

	যখন (ls && rc == 0) {
		যদি (ls->name && ls->মান)
			rc = setenv(ls->name, ls->মান, overwrite);
		ls = ls->পরবর্তী;
	}
	ফেরত rc;
}

শূন্য env_list_free(কাঠামো ul_env_list *ls)
{
	যখন (ls) {
		কাঠামো ul_env_list *x = ls;
		ls = ls->পরবর্তী;
		মুক্ত(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
শূন্য __sanitize_env(কাঠামো ul_env_list **org)
{
	অক্ষর **envp = environ;
	অক্ষর * ধ্রুবক *bad;
	অক্ষর **cur;
	পূর্ণসংখ্যা last = 0;

	জন্য (cur = envp; *cur; cur++)
		last++;

	জন্য (cur = envp; *cur; cur++) {
		জন্য (bad = forbid; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				যদি (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				বিরতি;
			}
		}
	}

	জন্য (cur = envp; *cur; cur++) {
		জন্য (bad = noslash; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) != 0)
				চালু;
			যদি (!strchr(*cur, '/'))
				চালু;  /* OK */
			যদি (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			বিরতি;
		}
	}
}

শূন্য sanitize_env(শূন্য)
{
	__sanitize_env(ফাঁকা);
}

অক্ষর *safe_getenv(ধ্রুবক অক্ষর *arg)
{
	যদি (is_privileged_execution())
		ফেরত ফাঁকা;
#ifdef HAVE_PRCTL
	যদি (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ফেরত ফাঁকা;
#অন্যথা
#যদি (defined(linux) && defined(SYS_prctl))
	যদি (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ফেরত ফাঁকা;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ফেরত secure_getenv(arg);
#নতুবা_যদি HAVE___SECURE_GETENV
	ফেরত __secure_getenv(arg);
#অন্যথা
	ফেরত getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
পূর্ণসংখ্যা প্রধান(শূন্য)
{
	অক্ষর *ধ্রুবক *bad;
	অক্ষর copy[32];
	অক্ষর *p;
	পূর্ণসংখ্যা retval = EXIT_SUCCESS;
	কাঠামো ul_env_list *removed = ফাঁকা;

	/* define env. */
	জন্য (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		যদি (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	জন্য (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		যদি (p)
			*p = '\0';
		p = getenv(copy);
		যদি (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	জন্য (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		যদি (p)
			*p = '\0';
		p = getenv(copy);
		যদি (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ফেরত retval;
}
#endif
