/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#यदि_परिभाषितम् HAVE_SYS_PRCTL_H
#अन्तर्भाव <sys/prctl.h>
#अन्यथा
#परिभाषय PR_GET_DUMPABLE 3
#यदि_अन्तः
#यदि (!defined(HAVE_PRCTL) && defined(linux))
#अन्तर्भाव <sys/syscall.h>
#यदि_अन्तः
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "env.h"
#अन्तर्भाव "strv.h"
#अन्तर्भाव "all-io.h"

#यदि_अपरिभाषितम् HAVE_ENVIRON_DECL
बाह्य वर्ण **environ;
#यदि_अन्तः

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

	संरचना ul_env_list *अग्र;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_सङ्कलय( संरचना ul_env_list *ls0,
					 ध्रुव वर्ण *name, size_t namesz,
					 ध्रुव वर्ण *मानम्, size_t valsz)
{
	संरचना ul_env_list *ls;

	ls = शून्यस्मृतिं_देहि(1, परिमाण(संरचना ul_env_list) + namesz + valsz + 2);
	यदि (!ls)
		प्रत्यागम ls0;

	ls->name = ((वर्ण *) ls) + परिमाण(संरचना ul_env_list);
	ls->मानम् = ls->name + namesz + 1;

	स्मृतिं_प्रतिलिख(ls->name, name, namesz);
	स्मृतिं_प्रतिलिख(ls->मानम्, मानम्, valsz);

	ls->अग्र = ls0;
	प्रत्यागम ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_सङ्कलय_from_string(संरचना ul_env_list *ls,
						    ध्रुव वर्ण *str)
{
	size_t namesz = 0, valsz = 0;
	ध्रुव वर्ण *val;

	यदि (!str || !*str)
		प्रत्यागम ls;

	val = strchr(str, '=');
	यदि (!val)
		प्रत्यागम ls;
	namesz = val - str;

	val++;
	valsz = वाक्यदैर्घ्यम्(val);

	प्रत्यागम env_list_सङ्कलय(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
संरचना ul_env_list *env_list_सङ्कलय_variable(
				संरचना ul_env_list *ls,
				ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्)
{
	यदि (!name || !*name)
		प्रत्यागम ls;

	प्रत्यागम env_list_सङ्कलय(ls, name, वाक्यदैर्घ्यम्(name), मानम्, मानम् ? वाक्यदैर्घ्यम्(मानम्) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
संरचना ul_env_list *env_list_सङ्कलय_getenv(संरचना ul_env_list *ls,
				ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	ध्रुव वर्ण *val;

	यदि (!name)
		प्रत्यागम ls;

	val = getenv(name);
	यदि (!val)
		val= dflt;
	यदि (val)
		ls = env_list_सङ्कलय_variable(ls, name, val);
	प्रत्यागम ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
संरचना ul_env_list *env_list_सङ्कलय_getenvs(संरचना ul_env_list *ls, ध्रुव वर्ण *str)
{
	वर्ण **all, **name;

	यदि (!str)
		प्रत्यागम ls;

	all = ul_strv_विभज(str, ",");
	यदि (!all)
		प्रत्यागम ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_सङ्कलय_getenv(ls, *name, रिक्त);

	ul_strv_free(all);
	प्रत्यागम ls;
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
	यदि ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		प्रत्यागम रिक्त;
	buf[rc] = '\0';
	p = buf;

	यावत् (rc > 0) {
		ls = env_list_सङ्कलय_from_string(ls, p);
		p += वाक्यदैर्घ्यम्(p) + 1;
		rc -= वाक्यदैर्घ्यम्(p) + 1;
	}

	मुक्त(buf);
	प्रत्यागम ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
पूर्णाङ्क env_list_setenv(संरचना ul_env_list *ls, पूर्णाङ्क overwrite)
{
	पूर्णाङ्क rc = 0;

	यावत् (ls && rc == 0) {
		यदि (ls->name && ls->मानम्)
			rc = setenv(ls->name, ls->मानम्, overwrite);
		ls = ls->अग्र;
	}
	प्रत्यागम rc;
}

शून्य env_list_free(संरचना ul_env_list *ls)
{
	यावत् (ls) {
		संरचना ul_env_list *x = ls;
		ls = ls->अग्र;
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
	पूर्णाङ्क चरमः = 0;

	प्रति (cur = envp; *cur; cur++)
		चरमः++;

	प्रति (cur = envp; *cur; cur++) {
		प्रति (bad = forbid; *bad; bad++) {
			यदि (वाक्ये_तुलय(*cur, *bad, वाक्यदैर्घ्यम्(*bad)) == 0) {
				यदि (org)
					*org = env_list_सङ्कलय_from_string(*org, *cur);
				चरमः = ul_remove_प्रविष्टिः(envp, cur - envp, चरमः);
				cur--;
				भङ्ग;
			}
		}
	}

	प्रति (cur = envp; *cur; cur++) {
		प्रति (bad = noslash; *bad; bad++) {
			यदि (वाक्ये_तुलय(*cur, *bad, वाक्यदैर्घ्यम्(*bad)) != 0)
				अनुवर्त;
			यदि (!strchr(*cur, '/'))
				अनुवर्त;  /* OK */
			यदि (org)
				*org = env_list_सङ्कलय_from_string(*org, *cur);
			चरमः = ul_remove_प्रविष्टिः(envp, cur - envp, चरमः);
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
	यदि (is_privileged_execution())
		प्रत्यागम रिक्त;
#यदि_परिभाषितम् HAVE_PRCTL
	यदि (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		प्रत्यागम रिक्त;
#अन्यथा
#यदि (defined(linux) && defined(SYS_prctl))
	यदि (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		प्रत्यागम रिक्त;
#यदि_अन्तः
#यदि_अन्तः
#यदि_परिभाषितम् HAVE_SECURE_GETENV
प्रत्यागम secure_getenv(arg);
#नोचेद्यदि HAVE___SECURE_GETENV
	प्रत्यागम __secure_getenv(arg);
#अन्यथा
	प्रत्यागम getenv(arg);
#यदि_अन्तः
}

#यदि_परिभाषितम् TEST_क्रमादेशः_ENV
पूर्णाङ्क मुख्यम्(शून्य)
{
	वर्ण *ध्रुव *bad;
	वर्ण copy[32];
	वर्ण *p;
	पूर्णाङ्क retval = EXIT_SUCCESS;
	संरचना ul_env_list *removed = रिक्त;

	/* define env. */
	प्रति (bad = forbid; *bad; bad++) {
		वाक्यं_प्रतिलिख(copy, *bad);
		p = strchr(copy, '=');
		यदि (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	प्रति (bad = forbid; *bad; bad++) {
		वाक्यं_प्रतिलिख(copy, *bad);
		p = strchr(copy, '=');
		यदि (p)
			*p = '\0';
		p = getenv(copy);
		यदि (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	प्रति (bad = forbid; *bad; bad++) {
		वाक्यं_प्रतिलिख(copy, *bad);
		p = strchr(copy, '=');
		यदि (p)
			*p = '\0';
		p = getenv(copy);
		यदि (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	प्रत्यागम retval;
}
#यदि_अन्तः
