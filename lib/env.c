/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ifdef HAVE_SYS_PRCTL_H
#ضمن <sys/prctl.h>
#وإلا
#define PR_GET_DUMPABLE 3
#endif
#𐤀𐤌 (!defined(HAVE_PRCTL) && defined(linux))
#ضمن <sys/syscall.h>
#endif
#ضمن <unistd.h>
#ضمن <sys/types.h>

#ضمن "env.h"
#ضمن "strv.h"
#ضمن "all-io.h"

#ifndef HAVE_ENVIRON_DECL
خارجي حرف **environ;
#endif

ساكن حرف * ثابت forbid[] = {
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
	(حرف *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ساكن حرف * ثابت noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(حرف *) 0
};


بنية ul_env_list {
	حرف	*name;
	حرف	*value;

	بنية ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_add( بنية ul_env_list *ls0,
					 ثابت حرف *name, size_t namesz,
					 ثابت حرف *value, size_t valsz)
{
	بنية ul_env_list *ls;

	ls = calloc(1, حجم(بنية ul_env_list) + namesz + valsz + 2);
	𐤀𐤌 (!ls)
		أرجع ls0;

	ls->name = ((حرف *) ls) + حجم(بنية ul_env_list);
	ls->value = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->value, value, valsz);

	ls->next = ls0;
	أرجع ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_add_from_string(بنية ul_env_list *ls,
						    ثابت حرف *str)
{
	size_t namesz = 0, valsz = 0;
	ثابت حرف *val;

	𐤀𐤌 (!str || !*str)
		أرجع ls;

	val = strchr(str, '=');
	𐤀𐤌 (!val)
		أرجع ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	أرجع env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
بنية ul_env_list *env_list_add_variable(
				بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *value)
{
	𐤀𐤌 (!name || !*name)
		أرجع ls;

	أرجع env_list_add(ls, name, strlen(name), value, value ? strlen(value) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
بنية ul_env_list *env_list_add_getenv(بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *dflt)
{
	ثابت حرف *val;

	𐤀𐤌 (!name)
		أرجع ls;

	val = getenv(name);
	𐤀𐤌 (!val)
		val= dflt;
	𐤀𐤌 (val)
		ls = env_list_add_variable(ls, name, val);
	أرجع ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
بنية ul_env_list *env_list_add_getenvs(بنية ul_env_list *ls, ثابت حرف *str)
{
	حرف **all, **name;

	𐤀𐤌 (!str)
		أرجع ls;

	all = ul_strv_split(str, ",");
	𐤀𐤌 (!all)
		أرجع ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, عدم);

	ul_strv_free(all);
	أرجع ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
بنية ul_env_list *env_list_from_fd(صحيح fd)
{
	حرف *buf = عدم, *p;
	ssize_t rc = 0;
	بنية ul_env_list *ls = عدم;

	errno = 0;
	𐤀𐤌 ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		أرجع عدم;
	buf[rc] = '\0';
	p = buf;

	طالما (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	حرر(buf);
	أرجع ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
صحيح env_list_setenv(بنية ul_env_list *ls, صحيح overwrite)
{
	صحيح rc = 0;

	طالما (ls && rc == 0) {
		𐤀𐤌 (ls->name && ls->value)
			rc = setenv(ls->name, ls->value, overwrite);
		ls = ls->next;
	}
	أرجع rc;
}

فراغ env_list_free(بنية ul_env_list *ls)
{
	طالما (ls) {
		بنية ul_env_list *x = ls;
		ls = ls->next;
		حرر(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
فراغ __sanitize_env(بنية ul_env_list **org)
{
	حرف **envp = environ;
	حرف * ثابت *bad;
	حرف **cur;
	صحيح last = 0;

	لكل (cur = envp; *cur; cur++)
		last++;

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = forbid; *bad; bad++) {
			𐤀𐤌 (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				𐤀𐤌 (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				اكسر;
			}
		}
	}

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = noslash; *bad; bad++) {
			𐤀𐤌 (strncmp(*cur, *bad, strlen(*bad)) != 0)
				استمر;
			𐤀𐤌 (!strchr(*cur, '/'))
				استمر;  /* OK */
			𐤀𐤌 (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			اكسر;
		}
	}
}

فراغ sanitize_env(فراغ)
{
	__sanitize_env(عدم);
}

حرف *safe_getenv(ثابت حرف *arg)
{
	𐤀𐤌 (is_privileged_execution())
		أرجع عدم;
#ifdef HAVE_PRCTL
	𐤀𐤌 (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#وإلا
#𐤀𐤌 (defined(linux) && defined(SYS_prctl))
	𐤀𐤌 (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
أرجع secure_getenv(arg);
#وإلاإذا HAVE___SECURE_GETENV
	أرجع __secure_getenv(arg);
#وإلا
	أرجع getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
صحيح main(فراغ)
{
	حرف *ثابت *bad;
	حرف copy[32];
	حرف *p;
	صحيح retval = EXIT_SUCCESS;
	بنية ul_env_list *removed = عدم;

	/* define env. */
	لكل (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		𐤀𐤌 (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	لكل (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		𐤀𐤌 (p)
			*p = '\0';
		p = getenv(copy);
		𐤀𐤌 (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	لكل (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		𐤀𐤌 (p)
			*p = '\0';
		p = getenv(copy);
		𐤀𐤌 (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	أرجع retval;
}
#endif
