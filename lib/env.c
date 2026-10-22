/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#ifdef HAVE_SYS_PRCTL_H
#अन्तर्भाव <sys/prctl.h>
#अञ्ञथा
#define PR_GET_DUMPABLE 3
#endif
#सचे (!defined(HAVE_PRCTL) && defined(linux))
#अन्तर्भाव <sys/syscall.h>
#endif
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "env.h"
#अन्तर्भाव "strv.h"
#अन्तर्भाव "all-io.h"

#ifndef HAVE_ENVIRON_DECL
बाह्य वर्ण **environ;
#endif

स्थिर वर्ण * ध्रुव forbid[] = {
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
	(वर्ण *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
स्थिर वर्ण * ध्रुव noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(वर्ण *) 0
};


संरचना ul_env_list {
	वर्ण	*name;
	वर्ण	*मानम्;

	संरचना ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add( संरचना ul_env_list *ls0,
					 ध्रुव वर्ण *name, size_t namesz,
					 ध्रुव वर्ण *मानम्, size_t valsz)
{
	संरचना ul_env_list *ls;

	ls = calloc(1, परिमाण(संरचना ul_env_list) + namesz + valsz + 2);
	सचे (!ls)
		पटिनिवत्त ls0;

	ls->name = ((वर्ण *) ls) + परिमाण(संरचना ul_env_list);
	ls->मानम् = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->मानम्, मानम्, valsz);

	ls->next = ls0;
	पटिनिवत्त ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add_from_string(संरचना ul_env_list *ls,
						    ध्रुव वर्ण *str)
{
	size_t namesz = 0, valsz = 0;
	ध्रुव वर्ण *val;

	सचे (!str || !*str)
		पटिनिवत्त ls;

	val = strchr(str, '=');
	सचे (!val)
		पटिनिवत्त ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	पटिनिवत्त env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
संरचना ul_env_list *env_list_add_variable(
				संरचना ul_env_list *ls,
				ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्)
{
	सचे (!name || !*name)
		पटिनिवत्त ls;

	पटिनिवत्त env_list_add(ls, name, strlen(name), मानम्, मानम् ? strlen(मानम्) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
संरचना ul_env_list *env_list_add_getenv(संरचना ul_env_list *ls,
				ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	ध्रुव वर्ण *val;

	सचे (!name)
		पटिनिवत्त ls;

	val = getenv(name);
	सचे (!val)
		val= dflt;
	सचे (val)
		ls = env_list_add_variable(ls, name, val);
	पटिनिवत्त ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
संरचना ul_env_list *env_list_add_getenvs(संरचना ul_env_list *ls, ध्रुव वर्ण *str)
{
	वर्ण **all, **name;

	सचे (!str)
		पटिनिवत्त ls;

	all = ul_strv_split(str, ",");
	सचे (!all)
		पटिनिवत्त ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, रिक्त);

	ul_strv_free(all);
	पटिनिवत्त ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
संरचना ul_env_list *env_list_from_fd(पूर्णाङ्क fd)
{
	वर्ण *buf = रिक्त, *p;
	ssize_t rc = 0;
	संरचना ul_env_list *ls = रिक्त;

	errno = 0;
	सचे ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		पटिनिवत्त रिक्त;
	buf[rc] = '\0';
	p = buf;

	याव (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	मुक्त(buf);
	पटिनिवत्त ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
पूर्णाङ्क env_list_setenv(संरचना ul_env_list *ls, पूर्णाङ्क overwrite)
{
	पूर्णाङ्क rc = 0;

	याव (ls && rc == 0) {
		सचे (ls->name && ls->मानम्)
			rc = setenv(ls->name, ls->मानम्, overwrite);
		ls = ls->next;
	}
	पटिनिवत्त rc;
}

शून्य env_list_free(संरचना ul_env_list *ls)
{
	याव (ls) {
		संरचना ul_env_list *x = ls;
		ls = ls->next;
		मुक्त(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
शून्य __sanitize_env(संरचना ul_env_list **org)
{
	वर्ण **envp = environ;
	वर्ण * ध्रुव *bad;
	वर्ण **cur;
	पूर्णाङ्क last = 0;

	अत्थाय (cur = envp; *cur; cur++)
		last++;

	अत्थाय (cur = envp; *cur; cur++) {
		अत्थाय (bad = forbid; *bad; bad++) {
			सचे (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				सचे (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				भङ्ग;
			}
		}
	}

	अत्थाय (cur = envp; *cur; cur++) {
		अत्थाय (bad = noslash; *bad; bad++) {
			सचे (strncmp(*cur, *bad, strlen(*bad)) != 0)
				अनुवर्त;
			सचे (!strchr(*cur, '/'))
				अनुवर्त;  /* OK */
			सचे (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			भङ्ग;
		}
	}
}

शून्य sanitize_env(शून्य)
{
	__sanitize_env(रिक्त);
}

वर्ण *safe_getenv(ध्रुव वर्ण *arg)
{
	सचे (is_privileged_execution())
		पटिनिवत्त रिक्त;
#ifdef HAVE_PRCTL
	सचे (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		पटिनिवत्त रिक्त;
#अञ्ञथा
#सचे (defined(linux) && defined(SYS_prctl))
	सचे (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		पटिनिवत्त रिक्त;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
पटिनिवत्त secure_getenv(arg);
#नोचेद्यदि HAVE___SECURE_GETENV
	पटिनिवत्त __secure_getenv(arg);
#अञ्ञथा
	पटिनिवत्त getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
पूर्णाङ्क main(शून्य)
{
	वर्ण *ध्रुव *bad;
	वर्ण copy[32];
	वर्ण *p;
	पूर्णाङ्क retval = EXIT_SUCCESS;
	संरचना ul_env_list *removed = रिक्त;

	/* define env. */
	अत्थाय (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		सचे (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	अत्थाय (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		सचे (p)
			*p = '\0';
		p = getenv(copy);
		सचे (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	अत्थाय (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		सचे (p)
			*p = '\0';
		p = getenv(copy);
		सचे (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	पटिनिवत्त retval;
}
#endif
