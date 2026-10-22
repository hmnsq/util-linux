/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#অন্তর্ভাৱ <sys/prctl.h>
#অন্যথা
#define PR_GET_DUMPABLE 3
#endif
#যদি (!defined(HAVE_PRCTL) && defined(linux))
#অন্তর্ভাৱ <sys/syscall.h>
#endif
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/types.h>

#অন্তর্ভাৱ "env.h"
#অন্তর্ভাৱ "strv.h"
#অন্তর্ভাৱ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
বাহ্য ৱর্ণ **environ;
#endif

স্থির ৱর্ণ * ধ্রুৱ forbid[] = {
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
	(ৱর্ণ *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
স্থির ৱর্ণ * ধ্রুৱ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(ৱর্ণ *) 0
};


সংরচনা ul_env_list {
	ৱর্ণ	*name;
	ৱর্ণ	*মান;

	সংরচনা ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
স্থির সংরচনা ul_env_list *env_list_add( সংরচনা ul_env_list *ls0,
					 ধ্রুৱ ৱর্ণ *name, size_t namesz,
					 ধ্রুৱ ৱর্ণ *মান, size_t valsz)
{
	সংরচনা ul_env_list *ls;

	ls = calloc(1, পরিমাণ(সংরচনা ul_env_list) + namesz + valsz + 2);
	যদি (!ls)
		ঘূৰাই ls0;

	ls->name = ((ৱর্ণ *) ls) + পরিমাণ(সংরচনা ul_env_list);
	ls->মান = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->মান, মান, valsz);

	ls->next = ls0;
	ঘূৰাই ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
স্থির সংরচনা ul_env_list *env_list_add_from_string(সংরচনা ul_env_list *ls,
						    ধ্রুৱ ৱর্ণ *str)
{
	size_t namesz = 0, valsz = 0;
	ধ্রুৱ ৱর্ণ *val;

	যদি (!str || !*str)
		ঘূৰাই ls;

	val = strchr(str, '=');
	যদি (!val)
		ঘূৰাই ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ঘূৰাই env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
সংরচনা ul_env_list *env_list_add_variable(
				সংরচনা ul_env_list *ls,
				ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *মান)
{
	যদি (!name || !*name)
		ঘূৰাই ls;

	ঘূৰাই env_list_add(ls, name, strlen(name), মান, মান ? strlen(মান) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
সংরচনা ul_env_list *env_list_add_getenv(সংরচনা ul_env_list *ls,
				ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *dflt)
{
	ধ্রুৱ ৱর্ণ *val;

	যদি (!name)
		ঘূৰাই ls;

	val = getenv(name);
	যদি (!val)
		val= dflt;
	যদি (val)
		ls = env_list_add_variable(ls, name, val);
	ঘূৰাই ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
সংরচনা ul_env_list *env_list_add_getenvs(সংরচনা ul_env_list *ls, ধ্রুৱ ৱর্ণ *str)
{
	ৱর্ণ **all, **name;

	যদি (!str)
		ঘূৰাই ls;

	all = ul_strv_split(str, ",");
	যদি (!all)
		ঘূৰাই ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, রিক্ত);

	ul_strv_free(all);
	ঘূৰাই ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
সংরচনা ul_env_list *env_list_from_fd(পূর্ণাঙ্ক fd)
{
	ৱর্ণ *buf = রিক্ত, *p;
	ssize_t rc = 0;
	সংরচনা ul_env_list *ls = রিক্ত;

	errno = 0;
	যদি ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ঘূৰাই রিক্ত;
	buf[rc] = '\0';
	p = buf;

	যেতিয়ালৈ (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	মুক্ত(buf);
	ঘূৰাই ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
পূর্ণাঙ্ক env_list_setenv(সংরচনা ul_env_list *ls, পূর্ণাঙ্ক overwrite)
{
	পূর্ণাঙ্ক rc = 0;

	যেতিয়ালৈ (ls && rc == 0) {
		যদি (ls->name && ls->মান)
			rc = setenv(ls->name, ls->মান, overwrite);
		ls = ls->next;
	}
	ঘূৰাই rc;
}

শূন্য env_list_free(সংরচনা ul_env_list *ls)
{
	যেতিয়ালৈ (ls) {
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
	ৱর্ণ **envp = environ;
	ৱর্ণ * ধ্রুৱ *bad;
	ৱর্ণ **cur;
	পূর্ণাঙ্ক last = 0;

	বাবে (cur = envp; *cur; cur++)
		last++;

	বাবে (cur = envp; *cur; cur++) {
		বাবে (bad = forbid; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				যদি (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				ভঙ্গ;
			}
		}
	}

	বাবে (cur = envp; *cur; cur++) {
		বাবে (bad = noslash; *bad; bad++) {
			যদি (strncmp(*cur, *bad, strlen(*bad)) != 0)
				অনুৱর্ত;
			যদি (!strchr(*cur, '/'))
				অনুৱর্ত;  /* OK */
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

ৱর্ণ *safe_getenv(ধ্রুৱ ৱর্ণ *arg)
{
	যদি (is_privileged_execution())
		ঘূৰাই রিক্ত;
#ifdef HAVE_PRCTL
	যদি (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ঘূৰাই রিক্ত;
#অন্যথা
#যদি (defined(linux) && defined(SYS_prctl))
	যদি (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ঘূৰাই রিক্ত;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ঘূৰাই secure_getenv(arg);
#নোচেদ্যদি HAVE___SECURE_GETENV
	ঘূৰাই __secure_getenv(arg);
#অন্যথা
	ঘূৰাই getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
পূর্ণাঙ্ক main(শূন্য)
{
	ৱর্ণ *ধ্রুৱ *bad;
	ৱর্ণ copy[32];
	ৱর্ণ *p;
	পূর্ণাঙ্ক retval = EXIT_SUCCESS;
	সংরচনা ul_env_list *removed = রিক্ত;

	/* define env. */
	বাবে (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		যদি (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	বাবে (bad = forbid; *bad; bad++) {
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
	বাবে (bad = forbid; *bad; bad++) {
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

	ঘূৰাই retval;
}
#endif
