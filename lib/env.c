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
#अन्यथा
#define PR_GET_DUMPABLE 3
#endif
#जर (!defined(HAVE_PRCTL) && defined(linux))
#अन्तर्भाव <sys/syscall.h>
#endif
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "env.h"
#अन्तर्भाव "strv.h"
#अन्तर्भाव "all-io.h"

#ifndef HAVE_ENVIRON_DECL
बाह्य अक्षर **environ;
#endif

स्थिर अक्षर * स्थिरांक forbid[] = {
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
	(अक्षर *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
स्थिर अक्षर * स्थिरांक noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(अक्षर *) 0
};


संरचना ul_env_list {
	अक्षर	*name;
	अक्षर	*मूल्य;

	संरचना ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add( संरचना ul_env_list *ls0,
					 स्थिरांक अक्षर *name, माप_प्रकार namesz,
					 स्थिरांक अक्षर *मूल्य, माप_प्रकार valsz)
{
	संरचना ul_env_list *ls;

	ls = सुस्मृति(1, आकार(संरचना ul_env_list) + namesz + valsz + 2);
	जर (!ls)
		परत ls0;

	ls->name = ((अक्षर *) ls) + आकार(संरचना ul_env_list);
	ls->मूल्य = ls->name + namesz + 1;

	स_नकल(ls->name, name, namesz);
	स_नकल(ls->मूल्य, मूल्य, valsz);

	ls->next = ls0;
	परत ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add_from_string(संरचना ul_env_list *ls,
						    स्थिरांक अक्षर *str)
{
	माप_प्रकार namesz = 0, valsz = 0;
	स्थिरांक अक्षर *val;

	जर (!str || !*str)
		परत ls;

	val = म_अक्षर(str, '=');
	जर (!val)
		परत ls;
	namesz = val - str;

	val++;
	valsz = म_माप(val);

	परत env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
संरचना ul_env_list *env_list_add_variable(
				संरचना ul_env_list *ls,
				स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य)
{
	जर (!name || !*name)
		परत ls;

	परत env_list_add(ls, name, म_माप(name), मूल्य, मूल्य ? म_माप(मूल्य) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
संरचना ul_env_list *env_list_add_getenv(संरचना ul_env_list *ls,
				स्थिरांक अक्षर *name, स्थिरांक अक्षर *dflt)
{
	स्थिरांक अक्षर *val;

	जर (!name)
		परत ls;

	val = दो_पर्या(name);
	जर (!val)
		val= dflt;
	जर (val)
		ls = env_list_add_variable(ls, name, val);
	परत ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
संरचना ul_env_list *env_list_add_getenvs(संरचना ul_env_list *ls, स्थिरांक अक्षर *str)
{
	अक्षर **all, **name;

	जर (!str)
		परत ls;

	all = ul_strv_split(str, ",");
	जर (!all)
		परत ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, रिक्त);

	ul_strv_free(all);
	परत ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
संरचना ul_env_list *env_list_from_fd(पूर्णांक fd)
{
	अक्षर *buf = रिक्त, *p;
	ssize_t rc = 0;
	संरचना ul_env_list *ls = रिक्त;

	त्रुटि_सं = 0;
	जर ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		परत रिक्त;
	buf[rc] = '\0';
	p = buf;

	पर्यंत (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += म_माप(p) + 1;
		rc -= म_माप(p) + 1;
	}

	मुक्त(buf);
	परत ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
पूर्णांक env_list_setenv(संरचना ul_env_list *ls, पूर्णांक overwrite)
{
	पूर्णांक rc = 0;

	पर्यंत (ls && rc == 0) {
		जर (ls->name && ls->मूल्य)
			rc = setenv(ls->name, ls->मूल्य, overwrite);
		ls = ls->next;
	}
	परत rc;
}

शून्य env_list_free(संरचना ul_env_list *ls)
{
	पर्यंत (ls) {
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
	अक्षर **envp = environ;
	अक्षर * स्थिरांक *bad;
	अक्षर **cur;
	पूर्णांक last = 0;

	साठी (cur = envp; *cur; cur++)
		last++;

	साठी (cur = envp; *cur; cur++) {
		साठी (bad = forbid; *bad; bad++) {
			जर (म_भेदन(*cur, *bad, म_माप(*bad)) == 0) {
				जर (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				थांबा;
			}
		}
	}

	साठी (cur = envp; *cur; cur++) {
		साठी (bad = noslash; *bad; bad++) {
			जर (म_भेदन(*cur, *bad, म_माप(*bad)) != 0)
				सुरूठेवा;
			जर (!म_अक्षर(*cur, '/'))
				सुरूठेवा;  /* OK */
			जर (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			थांबा;
		}
	}
}

शून्य sanitize_env(शून्य)
{
	__sanitize_env(रिक्त);
}

अक्षर *safe_getenv(स्थिरांक अक्षर *arg)
{
	जर (is_privileged_execution())
		परत रिक्त;
#ifdef HAVE_PRCTL
	जर (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		परत रिक्त;
#अन्यथा
#जर (defined(linux) && defined(SYS_prctl))
	जर (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		परत रिक्त;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
परत secure_getenv(arg);
#किंवाजर HAVE___SECURE_GETENV
	परत __secure_getenv(arg);
#अन्यथा
	परत दो_पर्या(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
पूर्णांक main(शून्य)
{
	अक्षर *स्थिरांक *bad;
	अक्षर copy[32];
	अक्षर *p;
	पूर्णांक retval = निकास_सफल;
	संरचना ul_env_list *removed = रिक्त;

	/* define env. */
	साठी (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		जर (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	साठी (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		जर (p)
			*p = '\0';
		p = दो_पर्या(copy);
		जर (p) {
			warnx("%s was not removed", copy);
			retval = निकास_त्रुटि;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	साठी (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		जर (p)
			*p = '\0';
		p = दो_पर्या(copy);
		जर (!p) {
			warnx("%s was not restored", copy);
			retval = निकास_त्रुटि;
		}
	}

	env_list_free(removed);

	परत retval;
}
#endif
