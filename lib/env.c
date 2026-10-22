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
বাহ্য বর্ণ **environ;
#endif

স্থির বর্ণ * ধ্রুব forbid[] = {
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
	(বর্ণ *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
স্থির বর্ণ * ধ্রুব noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(বর্ণ *) 0
};


সংরচনা ul_env_list {
	বর্ণ	*name;
	বর্ণ	*মান;

	সংরচনা ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
স্থির সংরচনা ul_env_list *env_list_add( সংরচনা ul_env_list *ls0,
					 ধ্রুব বর্ণ *name, size_t namesz,
					 ধ্রুব বর্ণ *মান, size_t valsz)
{
	সংরচনা ul_env_list *ls;

	ls = calloc(1, পরিমাণ(সংরচনা ul_env_list) + namesz + valsz + 2);
	যদি (!ls)
		প্রত্যাগম ls0;

	ls->name = ((বর্ণ *) ls) + পরিমাণ(সংরচনা ul_env_list);
	ls->মান = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->মান, মান, valsz);

	ls->next = ls0;
	প্রত্যাগম ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
স্থির সংরচনা ul_env_list *env_list_add_from_string(সংরচনা ul_env_list *ls,
						    ধ্রুব বর্ণ *str)
{
	size_t namesz = 0, valsz = 0;
	ধ্রুব বর্ণ *val;

	যদি (!str || !*str)
		প্রত্যাগম ls;

	val = strchr(str, '=');
	যদি (!val)
		প্রত্যাগম ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	প্রত্যাগম env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
সংরচনা ul_env_list *env_list_add_variable(
				সংরচনা ul_env_list *ls,
				ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *মান)
{
	যদি (!name || !*name)
		প্রত্যাগম ls;

	প্রত্যাগম env_list_add(ls, name, strlen(name), মান, মান ? strlen(মান) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
সংরচনা ul_env_list *env_list_add_getenv(সংরচনা ul_env_list *ls,
				ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *dflt)
{
	ধ্রুব বর্ণ *val;

	যদি (!name)
		প্রত্যাগম ls;

	val = getenv(name);
	যদি (!val)
		val= dflt;
	যদি (val)
		ls = env_list_add_variable(ls, name, val);
	প্রত্যাগম ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
সংরচনা ul_env_list *env_list_add_getenvs(সংরচনা ul_env_list *ls, ধ্রুব বর্ণ *str)
{
	বর্ণ **all, **name;

	যদি (!str)
		প্রত্যাগম ls;

	all = ul_strv_split(str, ",");
	যদি (!all)
		প্রত্যাগম ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, রিক্ত);

	ul_strv_free(all);
	প্রত্যাগম ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
সংরচনা ul_env_list *env_list_from_fd(পূর্ণাঙ্ক fd)
{
	বর্ণ *buf = রিক্ত, *p;
	ssize_t rc = 0;
	সংরচনা ul_env_list *ls = রিক্ত;

	errno = 0;
	যদি ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		প্রত্যাগম রিক্ত;
	buf[rc] = '\0';
	p = buf;

	যাবত্ (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	মুক্ত(buf);
	প্রত্যাগম ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
পূর্ণাঙ্ক env_list_setenv(সংরচনা ul_env_list *ls, পূর্ণাঙ্ক overwrite)
{
	পূর্ণাঙ্ক rc = 0;

	যাবত্ (ls && rc == 0) {
		যদি (ls->name && ls->মান)
			rc = setenv(ls->name, ls->মান, overwrite);
		ls = ls->next;
	}
	প্রত্যাগম rc;
}

শূন্য env_list_free(সংরচনা ul_env_list *ls)
{
	যাবত্ (ls) {
		সংরচনা ul_env_list *x = ls;
		ls = ls->next;
		মুক্ত(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
শূন্য __sanitize_env(সংরচনা ul_env_list **org)
{
	বর্ণ **envp = environ;
	বর্ণ * ধ্রুব *bad;
	বর্ণ **cur;
	পূর্ণাঙ্ক last = 0;

	প্রতি (cur = envp; *cur; cur++)
		last++;

	প্রতি (cur = envp; *cur; cur++) {
		প্রতি (bad = forbid; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				যদি (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				ভঙ্গ;
			}
		}
	}

	প্রতি (cur = envp; *cur; cur++) {
		প্রতি (bad = noslash; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) != 0)
				অনুবর্ত;
			যদি (!strchr(*cur, '/'))
				অনুবর্ত;  /* OK */
			যদি (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			ভঙ্গ;
		}
	}
}

শূন্য sanitize_env(শূন্য)
{
	__sanitize_env(রিক্ত);
}

বর্ণ *safe_getenv(ধ্রুব বর্ণ *arg)
{
	যদি (is_privileged_execution())
		প্রত্যাগম রিক্ত;
#ifdef HAVE_PRCTL
	যদি (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		প্রত্যাগম রিক্ত;
#অন্যথা
#যদি (defined(linux) && defined(SYS_prctl))
	যদি (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		প্রত্যাগম রিক্ত;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
প্রত্যাগম secure_getenv(arg);
#নোচেদ্যদি HAVE___SECURE_GETENV
	প্রত্যাগম __secure_getenv(arg);
#অন্যথা
	প্রত্যাগম getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
পূর্ণাঙ্ক main(শূন্য)
{
	বর্ণ *ধ্রুব *bad;
	বর্ণ copy[32];
	বর্ণ *p;
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;
	সংরচনা ul_env_list *removed = রিক্ত;

	/* define env. */
	প্রতি (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		যদি (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	প্রতি (bad = forbid; *bad; bad++) {
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
	প্রতি (bad = forbid; *bad; bad++) {
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

	প্রত্যাগম retval;
}
#endif
