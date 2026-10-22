/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#ಅನ್ತರ್ಭಾವ <sys/prctl.h>
#ಅನ್ಯಥಾ
#define PR_GET_DUMPABLE 3
#endif
#ಯದಿ (!defined(HAVE_PRCTL) && defined(linux))
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#endif
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "env.h"
#ಅನ್ತರ್ಭಾವ "strv.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
ಬಾಹ್ಯ ವರ್ಣ **environ;
#endif

ಸ್ಥಿರ ವರ್ಣ * ಧ್ರುವ forbid[] = {
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
	(ವರ್ಣ *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ಸ್ಥಿರ ವರ್ಣ * ಧ್ರುವ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(ವರ್ಣ *) 0
};


ಸಂರಚನಾ ul_env_list {
	ವರ್ಣ	*name;
	ವರ್ಣ	*ಮೌಲ್ಯ;

	ಸಂರಚನಾ ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ಸ್ಥಿರ ಸಂರಚನಾ ul_env_list *env_list_add( ಸಂರಚನಾ ul_env_list *ls0,
					 ಧ್ರುವ ವರ್ಣ *name, size_t namesz,
					 ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, size_t valsz)
{
	ಸಂರಚನಾ ul_env_list *ls;

	ls = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_env_list) + namesz + valsz + 2);
	ಯದಿ (!ls)
		ಪ್ರತ್ಯಾಗಮ ls0;

	ls->name = ((ವರ್ಣ *) ls) + ಪರಿಮಾಣ(ಸಂರಚನಾ ul_env_list);
	ls->ಮೌಲ್ಯ = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->ಮೌಲ್ಯ, ಮೌಲ್ಯ, valsz);

	ls->next = ls0;
	ಪ್ರತ್ಯಾಗಮ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ಸ್ಥಿರ ಸಂರಚನಾ ul_env_list *env_list_add_from_string(ಸಂರಚನಾ ul_env_list *ls,
						    ಧ್ರುವ ವರ್ಣ *str)
{
	size_t namesz = 0, valsz = 0;
	ಧ್ರುವ ವರ್ಣ *val;

	ಯದಿ (!str || !*str)
		ಪ್ರತ್ಯಾಗಮ ls;

	val = strchr(str, '=');
	ಯದಿ (!val)
		ಪ್ರತ್ಯಾಗಮ ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ಪ್ರತ್ಯಾಗಮ env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ಸಂರಚನಾ ul_env_list *env_list_add_variable(
				ಸಂರಚನಾ ul_env_list *ls,
				ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಯದಿ (!name || !*name)
		ಪ್ರತ್ಯಾಗಮ ls;

	ಪ್ರತ್ಯಾಗಮ env_list_add(ls, name, strlen(name), ಮೌಲ್ಯ, ಮೌಲ್ಯ ? strlen(ಮೌಲ್ಯ) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ಸಂರಚನಾ ul_env_list *env_list_add_getenv(ಸಂರಚನಾ ul_env_list *ls,
				ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *dflt)
{
	ಧ್ರುವ ವರ್ಣ *val;

	ಯದಿ (!name)
		ಪ್ರತ್ಯಾಗಮ ls;

	val = getenv(name);
	ಯದಿ (!val)
		val= dflt;
	ಯದಿ (val)
		ls = env_list_add_variable(ls, name, val);
	ಪ್ರತ್ಯಾಗಮ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ಸಂರಚನಾ ul_env_list *env_list_add_getenvs(ಸಂರಚನಾ ul_env_list *ls, ಧ್ರುವ ವರ್ಣ *str)
{
	ವರ್ಣ **all, **name;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ ls;

	all = ul_strv_split(str, ",");
	ಯದಿ (!all)
		ಪ್ರತ್ಯಾಗಮ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, ರಿಕ್ತ);

	ul_strv_free(all);
	ಪ್ರತ್ಯಾಗಮ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ಸಂರಚನಾ ul_env_list *env_list_from_fd(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ವರ್ಣ *buf = ರಿಕ್ತ, *p;
	ssize_t rc = 0;
	ಸಂರಚನಾ ul_env_list *ls = ರಿಕ್ತ;

	errno = 0;
	ಯದಿ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	buf[rc] = '\0';
	p = buf;

	ಯಾವತ್ (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	ಮುಕ್ತ(buf);
	ಪ್ರತ್ಯಾಗಮ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ಪೂರ್ಣಾಙ್ಕ env_list_setenv(ಸಂರಚನಾ ul_env_list *ls, ಪೂರ್ಣಾಙ್ಕ overwrite)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯಾವತ್ (ls && rc == 0) {
		ಯದಿ (ls->name && ls->ಮೌಲ್ಯ)
			rc = setenv(ls->name, ls->ಮೌಲ್ಯ, overwrite);
		ls = ls->next;
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಶೂನ್ಯ env_list_free(ಸಂರಚನಾ ul_env_list *ls)
{
	ಯಾವತ್ (ls) {
		ಸಂರಚನಾ ul_env_list *x = ls;
		ls = ls->next;
		ಮುಕ್ತ(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
ಶೂನ್ಯ __sanitize_env(ಸಂರಚನಾ ul_env_list **org)
{
	ವರ್ಣ **envp = environ;
	ವರ್ಣ * ಧ್ರುವ *bad;
	ವರ್ಣ **cur;
	ಪೂರ್ಣಾಙ್ಕ last = 0;

	ಪ್ರತಿ (cur = envp; *cur; cur++)
		last++;

	ಪ್ರತಿ (cur = envp; *cur; cur++) {
		ಪ್ರತಿ (bad = forbid; *bad; bad++) {
			ಯದಿ (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				ಯದಿ (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				ಭಙ್ಗ;
			}
		}
	}

	ಪ್ರತಿ (cur = envp; *cur; cur++) {
		ಪ್ರತಿ (bad = noslash; *bad; bad++) {
			ಯದಿ (strncmp(*cur, *bad, strlen(*bad)) != 0)
				ಅನುವರ್ತ;
			ಯದಿ (!strchr(*cur, '/'))
				ಅನುವರ್ತ;  /* OK */
			ಯದಿ (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			ಭಙ್ಗ;
		}
	}
}

ಶೂನ್ಯ sanitize_env(ಶೂನ್ಯ)
{
	__sanitize_env(ರಿಕ್ತ);
}

ವರ್ಣ *safe_getenv(ಧ್ರುವ ವರ್ಣ *arg)
{
	ಯದಿ (is_privileged_execution())
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#ifdef HAVE_PRCTL
	ಯದಿ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#ಅನ್ಯಥಾ
#ಯದಿ (defined(linux) && defined(SYS_prctl))
	ಯದಿ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ಪ್ರತ್ಯಾಗಮ secure_getenv(arg);
#ನೋಚೇದ್ಯದಿ HAVE___SECURE_GETENV
	ಪ್ರತ್ಯಾಗಮ __secure_getenv(arg);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ)
{
	ವರ್ಣ *ಧ್ರುವ *bad;
	ವರ್ಣ copy[32];
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ retval = EXIT_SUCCESS;
	ಸಂರಚನಾ ul_env_list *removed = ರಿಕ್ತ;

	/* define env. */
	ಪ್ರತಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಯದಿ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	ಪ್ರತಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಯದಿ (p)
			*p = '\0';
		p = getenv(copy);
		ಯದಿ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	ಪ್ರತಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಯದಿ (p)
			*p = '\0';
		p = getenv(copy);
		ಯದಿ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ಪ್ರತ್ಯಾಗಮ retval;
}
#endif
