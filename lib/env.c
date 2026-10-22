/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#अगर_घोषित HAVE_SYS_PRCTL_H
#समावेश <sys/prctl.h>
#अन्यथा
#घोषणा PR_GET_DUMPABLE 3
#पूर्ण_अगर
#अगर (!defined(HAVE_PRCTL) && defined(linux))
#समावेश <sys/syscall.h>
#पूर्ण_अगर
#समावेश <unistd.h>
#समावेश <sys/types.h>

#समावेश "env.h"
#समावेश "strv.h"
#समावेश "all-io.h"

#अगर_अघोषित HAVE_ENVIRON_DECL
बाह्य अक्षर **environ;
#पूर्ण_अगर

अटल अक्षर * स्थिर forbid[] = {
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
अटल अक्षर * स्थिर noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(अक्षर *) 0
};


काष्ठा ul_env_list {
	अक्षर	*name;
	अक्षर	*मूल्य;

	काष्ठा ul_env_list *अग्र;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
अटल काष्ठा ul_env_list *env_list_add( काष्ठा ul_env_list *ls0,
					 स्थिर अक्षर *name, माप_प्रकार namesz,
					 स्थिर अक्षर *मूल्य, माप_प्रकार valsz)
{
	काष्ठा ul_env_list *ls;

	ls = सुस्मृति(1, माप(काष्ठा ul_env_list) + namesz + valsz + 2);
	अगर (!ls)
		वापस ls0;

	ls->name = ((अक्षर *) ls) + माप(काष्ठा ul_env_list);
	ls->मूल्य = ls->name + namesz + 1;

	स_नकल(ls->name, name, namesz);
	स_नकल(ls->मूल्य, मूल्य, valsz);

	ls->अग्र = ls0;
	वापस ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
अटल काष्ठा ul_env_list *env_list_add_from_string(काष्ठा ul_env_list *ls,
						    स्थिर अक्षर *str)
{
	माप_प्रकार namesz = 0, valsz = 0;
	स्थिर अक्षर *val;

	अगर (!str || !*str)
		वापस ls;

	val = म_अक्षर(str, '=');
	अगर (!val)
		वापस ls;
	namesz = val - str;

	val++;
	valsz = म_माप(val);

	वापस env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
काष्ठा ul_env_list *env_list_add_variable(
				काष्ठा ul_env_list *ls,
				स्थिर अक्षर *name, स्थिर अक्षर *मूल्य)
{
	अगर (!name || !*name)
		वापस ls;

	वापस env_list_add(ls, name, म_माप(name), मूल्य, मूल्य ? म_माप(मूल्य) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
काष्ठा ul_env_list *env_list_add_getenv(काष्ठा ul_env_list *ls,
				स्थिर अक्षर *name, स्थिर अक्षर *dflt)
{
	स्थिर अक्षर *val;

	अगर (!name)
		वापस ls;

	val = दो_पर्या(name);
	अगर (!val)
		val= dflt;
	अगर (val)
		ls = env_list_add_variable(ls, name, val);
	वापस ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
काष्ठा ul_env_list *env_list_add_getenvs(काष्ठा ul_env_list *ls, स्थिर अक्षर *str)
{
	अक्षर **all, **name;

	अगर (!str)
		वापस ls;

	all = ul_strv_split(str, ",");
	अगर (!all)
		वापस ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, शून्य);

	ul_strv_free(all);
	वापस ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
काष्ठा ul_env_list *env_list_from_fd(पूर्णांक fd)
{
	अक्षर *buf = शून्य, *p;
	ssize_t rc = 0;
	काष्ठा ul_env_list *ls = शून्य;

	त्रुटि_सं = 0;
	अगर ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		वापस शून्य;
	buf[rc] = '\0';
	p = buf;

	जबतक (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += म_माप(p) + 1;
		rc -= म_माप(p) + 1;
	}

	मुक्त(buf);
	वापस ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
पूर्णांक env_list_setenv(काष्ठा ul_env_list *ls, पूर्णांक overwrite)
{
	पूर्णांक rc = 0;

	जबतक (ls && rc == 0) {
		अगर (ls->name && ls->मूल्य)
			rc = setenv(ls->name, ls->मूल्य, overwrite);
		ls = ls->अग्र;
	}
	वापस rc;
}

व्योम env_list_free(काष्ठा ul_env_list *ls)
{
	जबतक (ls) {
		काष्ठा ul_env_list *x = ls;
		ls = ls->अग्र;
		मुक्त(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
व्योम __sanitize_env(काष्ठा ul_env_list **org)
{
	अक्षर **envp = environ;
	अक्षर * स्थिर *bad;
	अक्षर **cur;
	पूर्णांक last = 0;

	क्रम (cur = envp; *cur; cur++)
		last++;

	क्रम (cur = envp; *cur; cur++) {
		क्रम (bad = forbid; *bad; bad++) {
			अगर (म_भेदन(*cur, *bad, म_माप(*bad)) == 0) {
				अगर (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				अवरोध;
			}
		}
	}

	क्रम (cur = envp; *cur; cur++) {
		क्रम (bad = noslash; *bad; bad++) {
			अगर (म_भेदन(*cur, *bad, म_माप(*bad)) != 0)
				जारी;
			अगर (!म_अक्षर(*cur, '/'))
				जारी;  /* OK */
			अगर (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			अवरोध;
		}
	}
}

व्योम sanitize_env(व्योम)
{
	__sanitize_env(शून्य);
}

अक्षर *safe_getenv(स्थिर अक्षर *arg)
{
	अगर (is_privileged_execution())
		वापस शून्य;
#अगर_घोषित HAVE_PRCTL
	अगर (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		वापस शून्य;
#अन्यथा
#अगर (defined(linux) && defined(SYS_prctl))
	अगर (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		वापस शून्य;
#पूर्ण_अगर
#पूर्ण_अगर
#अगर_घोषित HAVE_SECURE_GETENV
वापस secure_getenv(arg);
#या_अगर HAVE___SECURE_GETENV
	वापस __secure_getenv(arg);
#अन्यथा
	वापस दो_पर्या(arg);
#पूर्ण_अगर
}

#अगर_घोषित TEST_PROGRAM_ENV
पूर्णांक मुख्य(व्योम)
{
	अक्षर *स्थिर *bad;
	अक्षर copy[32];
	अक्षर *p;
	पूर्णांक retval = निकास_सफल;
	काष्ठा ul_env_list *removed = शून्य;

	/* define env. */
	क्रम (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		अगर (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	क्रम (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		अगर (p)
			*p = '\0';
		p = दो_पर्या(copy);
		अगर (p) {
			warnx("%s was not removed", copy);
			retval = निकास_त्रुटि;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	क्रम (bad = forbid; *bad; bad++) {
		म_नकल(copy, *bad);
		p = म_अक्षर(copy, '=');
		अगर (p)
			*p = '\0';
		p = दो_पर्या(copy);
		अगर (!p) {
			warnx("%s was not restored", copy);
			retval = निकास_त्रुटि;
		}
	}

	env_list_free(removed);

	वापस retval;
}
#पूर्ण_अगर
