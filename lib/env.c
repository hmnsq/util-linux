/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#که_تعریف_شوی HAVE_SYS_PRCTL_H
#ضمن <sys/prctl.h>
#وإلا
#تعریف_کړه PR_GET_DUMPABLE 3
#د_که_پای
#إذا (!defined(HAVE_PRCTL) && defined(linux))
#ضمن <sys/syscall.h>
#د_که_پای
#ضمن <unistd.h>
#ضمن <sys/types.h>

#ضمن "env.h"
#ضمن "strv.h"
#ضمن "all-io.h"

#که_نه_تعریف_شوی HAVE_ENVIRON_DECL
خارجي حرف **environ;
#د_که_پای

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
	حرف	*ارزښت;

	بنية ul_env_list *بل;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جمع_کړه( بنية ul_env_list *ls0,
					 ثابت حرف *name, size_t namesz,
					 ثابت حرف *ارزښت, size_t valsz)
{
	بنية ul_env_list *ls;

	ls = حافظه_صفر_ورکړه(1, حجم(بنية ul_env_list) + namesz + valsz + 2);
	إذا (!ls)
		أرجع ls0;

	ls->name = ((حرف *) ls) + حجم(بنية ul_env_list);
	ls->ارزښت = ls->name + namesz + 1;

	حافظه_نقل(ls->name, name, namesz);
	حافظه_نقل(ls->ارزښت, ارزښت, valsz);

	ls->بل = ls0;
	أرجع ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جمع_کړه_from_string(بنية ul_env_list *ls,
						    ثابت حرف *str)
{
	size_t namesz = 0, valsz = 0;
	ثابت حرف *val;

	إذا (!str || !*str)
		أرجع ls;

	val = strchr(str, '=');
	إذا (!val)
		أرجع ls;
	namesz = val - str;

	val++;
	valsz = مزي_اوږدوالی(val);

	أرجع env_list_جمع_کړه(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
بنية ul_env_list *env_list_جمع_کړه_variable(
				بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *ارزښت)
{
	إذا (!name || !*name)
		أرجع ls;

	أرجع env_list_جمع_کړه(ls, name, مزي_اوږدوالی(name), ارزښت, ارزښت ? مزي_اوږدوالی(ارزښت) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
بنية ul_env_list *env_list_جمع_کړه_getenv(بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *dflt)
{
	ثابت حرف *val;

	إذا (!name)
		أرجع ls;

	val = getenv(name);
	إذا (!val)
		val= dflt;
	إذا (val)
		ls = env_list_جمع_کړه_variable(ls, name, val);
	أرجع ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
بنية ul_env_list *env_list_جمع_کړه_getenvs(بنية ul_env_list *ls, ثابت حرف *str)
{
	حرف **all, **name;

	إذا (!str)
		أرجع ls;

	all = ul_strv_وېشل(str, ",");
	إذا (!all)
		أرجع ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_جمع_کړه_getenv(ls, *name, عدم);

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
	إذا ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		أرجع عدم;
	buf[rc] = '\0';
	p = buf;

	طالما (rc > 0) {
		ls = env_list_جمع_کړه_from_string(ls, p);
		p += مزي_اوږدوالی(p) + 1;
		rc -= مزي_اوږدوالی(p) + 1;
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
		إذا (ls->name && ls->ارزښت)
			rc = setenv(ls->name, ls->ارزښت, overwrite);
		ls = ls->بل;
	}
	أرجع rc;
}

فراغ env_list_free(بنية ul_env_list *ls)
{
	طالما (ls) {
		بنية ul_env_list *x = ls;
		ls = ls->بل;
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
	صحيح وروستی = 0;

	لكل (cur = envp; *cur; cur++)
		وروستی++;

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = forbid; *bad; bad++) {
			إذا (مزي_پرتله(*cur, *bad, مزي_اوږدوالی(*bad)) == 0) {
				إذا (org)
					*org = env_list_جمع_کړه_from_string(*org, *cur);
				وروستی = ul_remove_ثبت(envp, cur - envp, وروستی);
				cur--;
				اكسر;
			}
		}
	}

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = noslash; *bad; bad++) {
			إذا (مزي_پرتله(*cur, *bad, مزي_اوږدوالی(*bad)) != 0)
				استمر;
			إذا (!strchr(*cur, '/'))
				استمر;  /* OK */
			إذا (org)
				*org = env_list_جمع_کړه_from_string(*org, *cur);
			وروستی = ul_remove_ثبت(envp, cur - envp, وروستی);
			cur--;
			اكسر;
		}
	}
}

فراغ sanitize_env(فراغ)
{
	__sanitize_env(عدم);
}

حرف *خوندي_getenv(ثابت حرف *arg)
{
	إذا (is_privileged_execution())
		أرجع عدم;
#که_تعریف_شوی HAVE_PRCTL
	إذا (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#وإلا
#إذا (defined(linux) && defined(SYS_prctl))
	إذا (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#د_که_پای
#د_که_پای
#که_تعریف_شوی HAVE_SECURE_GETENV
أرجع secure_getenv(arg);
#وإلاإذا HAVE___SECURE_GETENV
	أرجع __secure_getenv(arg);
#وإلا
	أرجع getenv(arg);
#د_که_پای
}

#که_تعریف_شوی TEST_پروګرام_ENV
صحيح اصلي(فراغ)
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
		إذا (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	لكل (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		إذا (p)
			*p = '\0';
		p = getenv(copy);
		إذا (p) {
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
		إذا (p)
			*p = '\0';
		p = getenv(copy);
		إذا (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	أرجع retval;
}
#د_که_پای
