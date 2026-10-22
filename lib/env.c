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
#यदि (!defined(HAVE_PRCTL) && defined(linux))
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

स्थिर वर्ण * अचर forbid[] = {
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
स्थिर वर्ण * अचर noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(वर्ण *) 0
};


संरचना ul_env_list {
	वर्ण	*name;
	वर्ण	*मान;

	संरचना ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add( संरचना ul_env_list *ls0,
					 अचर वर्ण *name, माप_प्रकार namesz,
					 अचर वर्ण *मान, माप_प्रकार valsz)
{
	संरचना ul_env_list *ls;

	ls = सुस्मृति(1, परिमाण(संरचना ul_env_list) + namesz + valsz + 2);
	यदि (!ls)
		फर्काऊ ls0;

	ls->name = ((वर्ण *) ls) + परिमाण(संरचना ul_env_list);
	ls->मान = ls->name + namesz + 1;

	स_नकल(ls->name, name, namesz);
	स_नकल(ls->मान, मान, valsz);

	ls->next = ls0;
	फर्काऊ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
स्थिर संरचना ul_env_list *env_list_add_from_string(संरचना ul_env_list *ls,
						    अचर वर्ण *str)
{
	माप_प्रकार namesz = 0, valsz = 0;
	अचर वर्ण *val;

	यदि (!str || !*str)
		फर्काऊ ls;

	val = म_अक्षर(str, '=');
	यदि (!val)
		फर्काऊ ls;
	namesz = val - str;

	val++;
	valsz = म_माप(val);

	फर्काऊ env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
संरचना ul_env_list *env_list_add_variable(
				संरचना ul_env_list *ls,
				अचर वर्ण *name, अचर वर्ण *मान)
{
	यदि (!name || !*name)
		फर्काऊ ls;

	फर्काऊ env_list_add(ls, name, म_माप(name), मान, मान ? म_माप(मान) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
संरचना ul_env_list *env_list_add_getenv(संरचना ul_env_list *ls,
				अचर वर्ण *name, अचर वर्ण *dflt)
{
	अचर वर्ण *val;

	यदि (!name)
		फर्काऊ ls;

	val = दो_पर्या(name);
	यदि (!val)
		val= dflt;
	यदि (val)
		ls = env_list_add_variable(ls, name, val);
	फर्काऊ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
संरचना ul_env_list *env_list_add_getenvs(संरचना ul_env_list *ls, अचर वर्ण *str)
{
	वर्ण **all, **name;

	यदि (!str)
		फर्काऊ ls;

	all = ul_strv_split(str, ",");
	यदि (!all)
		फर्काऊ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, रिक्त);

	ul_strv_free(all);
	फर्काऊ ls;
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

	त्रुटि_सं = 0;
	यदि ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		फर्काऊ रिक्त;
	buf[rc] = '\0';
	p = buf;

	जबसम्म (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += म_माप(p) + 1;
		rc -= म_माप(p) + 1;
	}

	मुक्त(buf);
	फर्काऊ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
पूर्णाङ्क env_list_setenv(संरचना ul_env_list *ls, पूर्णाङ्क overwrite)
{
	पूर्णाङ्क rc = 0;

	जबसम्म (ls && rc == 0) {
		यदि (ls->name && ls->मान)
			rc = setenv(ls->name, ls->मान, overwrite);
		ls = ls->next;
	}
	फर्काऊ rc;
}

शून्य env_list_free(संरचना ul_env_list *ls)
{
	जबसम्म (ls) {
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
	वर्ण * अचर *bad;
	वर्ण **cur;
	पूर्णाङ्क last = 0;

	लागि (cur = envp; *cur; cur++)
		last++;

	लागि (cur = envp; *cur; cur++) {
		लागि (bad = forbid; *bad; bad++) {
			यदि (म_भेदन(*cur, *bad, म_माप(*bad)) == 0) {
				यदि (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				रोक;
			}
		}
	}

	लागि (cur = envp; *cur; cur++) {
		लागि (bad = noslash; *bad; bad++) {
			यदि (म_भेदन(*cur, *bad, म_माप(*bad)) != 0)
				जारी;
			यदि (!म_अक्षर(*cur, '/'))
				जारी;  /* OK */
			यदि (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			रोक;
		}
	}
}

शून्य sanitize_env(शून्य)
{
	__sanitize_env(रिक्त);
}

वर्ण *safe_getenv(अचर वर्ण *arg)
{
	यदि (is_privileged_execution())
		फर्काऊ रिक्त;
#ifdef HAVE_PRCTL
	यदि (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		फर्काऊ रिक्त;
#अन्यथा
#यदि (defined(linux) && defined(SYS_prctl))
	यदि (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		फर्काऊ रिक्त;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
फर्काऊ secure_getenv(arg);
#अथवायदि HAVE___SECURE_GETENV
	फर्काऊ __secure_getenv(arg);
#अन्यथा
	फर्काऊ दो_पर्या(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
पूर्णाङ्क main(शून्य)
{
	वर्ण *अचर *bad;
	वर्ण copy[32];
	वर्ण *p;
	पूर्णाङ्क retval = निकास_सफल;
	संरचना ul_env_list *removed = रिक्त;

	/* define env. */
	लागि (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		यदि (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	लागि (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		यदि (p)
			*p = '\0';
		p = दो_पर्या(copy);
		यदि (p) {
			warnx("%s was not removed", copy);
			retval = निकास_त्रुटि;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	लागि (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		यदि (p)
			*p = '\0';
		p = दो_पर्या(copy);
		यदि (!p) {
			warnx("%s was not restored", copy);
			retval = निकास_त्रुटि;
		}
	}

	env_list_free(removed);

	फर्काऊ retval;
}
#endif
