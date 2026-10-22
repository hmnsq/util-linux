/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_PRCTL_H
#ಅನ್ತರ್ಭಾವ <sys/prctl.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು PR_GET_DUMPABLE 3
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ (!defined(HAVE_PRCTL) && defined(linux))
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "env.h"
#ಅನ್ತರ್ಭಾವ "strv.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_ENVIRON_DECL
ಬಾಹ್ಯ ಅಕ್ಷರ **environ;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ forbid[] = {
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
	(ಅಕ್ಷರ *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ಸ್ಥಿರ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(ಅಕ್ಷರ *) 0
};


ರಚನೆ ul_env_list {
	ಅಕ್ಷರ	*name;
	ಅಕ್ಷರ	*ಮೌಲ್ಯ;

	ರಚನೆ ul_env_list *ಮುಂದಿನ;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ಸ್ಥಿರ ರಚನೆ ul_env_list *env_list_ಕೂಡಿಸು( ರಚನೆ ul_env_list *ls0,
					 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, size_t namesz,
					 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, size_t valsz)
{
	ರಚನೆ ul_env_list *ls;

	ls = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ul_env_list) + namesz + valsz + 2);
	ಆದರೆ (!ls)
		ಹಿಂತಿರುಗಿ ls0;

	ls->name = ((ಅಕ್ಷರ *) ls) + ಗಾತ್ರ(ರಚನೆ ul_env_list);
	ls->ಮೌಲ್ಯ = ls->name + namesz + 1;

	ಸ್ಮರಣೆ_ನಕಲು(ls->name, name, namesz);
	ಸ್ಮರಣೆ_ನಕಲು(ls->ಮೌಲ್ಯ, ಮೌಲ್ಯ, valsz);

	ls->ಮುಂದಿನ = ls0;
	ಹಿಂತಿರುಗಿ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ಸ್ಥಿರ ರಚನೆ ul_env_list *env_list_ಕೂಡಿಸು_from_string(ರಚನೆ ul_env_list *ls,
						    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	size_t namesz = 0, valsz = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val;

	ಆದರೆ (!str || !*str)
		ಹಿಂತಿರುಗಿ ls;

	val = strchr(str, '=');
	ಆದರೆ (!val)
		ಹಿಂತಿರುಗಿ ls;
	namesz = val - str;

	val++;
	valsz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(val);

	ಹಿಂತಿರುಗಿ env_list_ಕೂಡಿಸು(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ರಚನೆ ul_env_list *env_list_ಕೂಡಿಸು_variable(
				ರಚನೆ ul_env_list *ls,
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಆದರೆ (!name || !*name)
		ಹಿಂತಿರುಗಿ ls;

	ಹಿಂತಿರುಗಿ env_list_ಕೂಡಿಸು(ls, name, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name), ಮೌಲ್ಯ, ಮೌಲ್ಯ ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ರಚನೆ ul_env_list *env_list_ಕೂಡಿಸು_getenv(ರಚನೆ ul_env_list *ls,
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val;

	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ ls;

	val = getenv(name);
	ಆದರೆ (!val)
		val= dflt;
	ಆದರೆ (val)
		ls = env_list_ಕೂಡಿಸು_variable(ls, name, val);
	ಹಿಂತಿರುಗಿ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ರಚನೆ ul_env_list *env_list_ಕೂಡಿಸು_getenvs(ರಚನೆ ul_env_list *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಅಕ್ಷರ **all, **name;

	ಆದರೆ (!str)
		ಹಿಂತಿರುಗಿ ls;

	all = ul_strv_ವಿಭಜಿಸು(str, ",");
	ಆದರೆ (!all)
		ಹಿಂತಿರುಗಿ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_ಕೂಡಿಸು_getenv(ls, *name, ಖಾಲಿ);

	ul_strv_free(all);
	ಹಿಂತಿರುಗಿ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ರಚನೆ ul_env_list *env_list_from_fd(ಪೂರ್ಣಾಂಕ fd)
{
	ಅಕ್ಷರ *buf = ಖಾಲಿ, *p;
	ssize_t rc = 0;
	ರಚನೆ ul_env_list *ls = ಖಾಲಿ;

	errno = 0;
	ಆದರೆ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	buf[rc] = '\0';
	p = buf;

	ಆಗ (rc > 0) {
		ls = env_list_ಕೂಡಿಸು_from_string(ls, p);
		p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1;
		rc -= ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1;
	}

	ಬಿಡುಗಡೆ(buf);
	ಹಿಂತಿರುಗಿ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ಪೂರ್ಣಾಂಕ env_list_setenv(ರಚನೆ ul_env_list *ls, ಪೂರ್ಣಾಂಕ overwrite)
{
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆಗ (ls && rc == 0) {
		ಆದರೆ (ls->name && ls->ಮೌಲ್ಯ)
			rc = setenv(ls->name, ls->ಮೌಲ್ಯ, overwrite);
		ls = ls->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ rc;
}

ಶೂನ್ಯ env_list_free(ರಚನೆ ul_env_list *ls)
{
	ಆಗ (ls) {
		ರಚನೆ ul_env_list *x = ls;
		ls = ls->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
ಶೂನ್ಯ __sanitize_env(ರಚನೆ ul_env_list **org)
{
	ಅಕ್ಷರ **envp = environ;
	ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *bad;
	ಅಕ್ಷರ **cur;
	ಪೂರ್ಣಾಂಕ ಕೊನೆಯ = 0;

	ಗಾಗಿ (cur = envp; *cur; cur++)
		ಕೊನೆಯ++;

	ಗಾಗಿ (cur = envp; *cur; cur++) {
		ಗಾಗಿ (bad = forbid; *bad; bad++) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(*cur, *bad, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*bad)) == 0) {
				ಆದರೆ (org)
					*org = env_list_ಕೂಡಿಸು_from_string(*org, *cur);
				ಕೊನೆಯ = ul_remove_ನಮೂದು(envp, cur - envp, ಕೊನೆಯ);
				cur--;
				ನಿಲ್ಲಿಸು;
			}
		}
	}

	ಗಾಗಿ (cur = envp; *cur; cur++) {
		ಗಾಗಿ (bad = noslash; *bad; bad++) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(*cur, *bad, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*bad)) != 0)
				ಮುಂದುವರಿ;
			ಆದರೆ (!strchr(*cur, '/'))
				ಮುಂದುವರಿ;  /* OK */
			ಆದರೆ (org)
				*org = env_list_ಕೂಡಿಸು_from_string(*org, *cur);
			ಕೊನೆಯ = ul_remove_ನಮೂದು(envp, cur - envp, ಕೊನೆಯ);
			cur--;
			ನಿಲ್ಲಿಸು;
		}
	}
}

ಶೂನ್ಯ sanitize_env(ಶೂನ್ಯ)
{
	__sanitize_env(ಖಾಲಿ);
}

ಅಕ್ಷರ *ಸುರಕ್ಷಿತ_getenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ಆದರೆ (is_privileged_execution())
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PRCTL
	ಆದರೆ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ (defined(linux) && defined(SYS_prctl))
	ಆದರೆ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SECURE_GETENV
ಹಿಂತಿರುಗಿ secure_getenv(arg);
#ಅಥವಾಆದರೆ HAVE___SECURE_GETENV
	ಹಿಂತಿರುಗಿ __secure_getenv(arg);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ getenv(arg);
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_ENV
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *bad;
	ಅಕ್ಷರ copy[32];
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ retval = EXIT_SUCCESS;
	ರಚನೆ ul_env_list *removed = ಖಾಲಿ;

	/* define env. */
	ಗಾಗಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಆದರೆ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	ಗಾಗಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಆದರೆ (p)
			*p = '\0';
		p = getenv(copy);
		ಆದರೆ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	ಗಾಗಿ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ಆದರೆ (p)
			*p = '\0';
		p = getenv(copy);
		ಆದರೆ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ಹಿಂತಿರುಗಿ retval;
}
#ಆದರೆ_ಅಂತ್ಯ
