/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#جے_تعریف_شدہ HAVE_SYS_PRCTL_H
#ضمن <sys/prctl.h>
#وإلا
#تعریف_کرو PR_GET_DUMPABLE 3
#جے_انت
#إذا (!defined(HAVE_PRCTL) && defined(linux))
#ضمن <sys/syscall.h>
#جے_انت
#ضمن <unistd.h>
#ضمن <sys/types.h>

#ضمن "env.h"
#ضمن "strv.h"
#ضمن "all-io.h"

#جے_بنا_تعریف HAVE_ENVIRON_DECL
خارجي حرف **environ;
#جے_انت

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
	حرف	*قدر;

	بنية ul_env_list *اگلا;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جوڑو( بنية ul_env_list *ls0,
					 ثابت حرف *name, size_t namesz,
					 ثابت حرف *قدر, size_t valsz)
{
	بنية ul_env_list *ls;

	ls = صفر_حافظہ_دیو(1, حجم(بنية ul_env_list) + namesz + valsz + 2);
	إذا (!ls)
		أرجع ls0;

	ls->name = ((حرف *) ls) + حجم(بنية ul_env_list);
	ls->قدر = ls->name + namesz + 1;

	حافظہ_نقل(ls->name, name, namesz);
	حافظہ_نقل(ls->قدر, قدر, valsz);

	ls->اگلا = ls0;
	أرجع ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جوڑو_from_string(بنية ul_env_list *ls,
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
	valsz = لڑی_لمبائی(val);

	أرجع env_list_جوڑو(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
بنية ul_env_list *env_list_جوڑو_variable(
				بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *قدر)
{
	إذا (!name || !*name)
		أرجع ls;

	أرجع env_list_جوڑو(ls, name, لڑی_لمبائی(name), قدر, قدر ? لڑی_لمبائی(قدر) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
بنية ul_env_list *env_list_جوڑو_getenv(بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *dflt)
{
	ثابت حرف *val;

	إذا (!name)
		أرجع ls;

	val = getenv(name);
	إذا (!val)
		val= dflt;
	إذا (val)
		ls = env_list_جوڑو_variable(ls, name, val);
	أرجع ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
بنية ul_env_list *env_list_جوڑو_getenvs(بنية ul_env_list *ls, ثابت حرف *str)
{
	حرف **all, **name;

	إذا (!str)
		أرجع ls;

	all = ul_strv_ونڈو(str, ",");
	إذا (!all)
		أرجع ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_جوڑو_getenv(ls, *name, عدم);

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
		ls = env_list_جوڑو_from_string(ls, p);
		p += لڑی_لمبائی(p) + 1;
		rc -= لڑی_لمبائی(p) + 1;
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
		إذا (ls->name && ls->قدر)
			rc = setenv(ls->name, ls->قدر, overwrite);
		ls = ls->اگلا;
	}
	أرجع rc;
}

فراغ env_list_free(بنية ul_env_list *ls)
{
	طالما (ls) {
		بنية ul_env_list *x = ls;
		ls = ls->اگلا;
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
	صحيح آخری = 0;

	لكل (cur = envp; *cur; cur++)
		آخری++;

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = forbid; *bad; bad++) {
			إذا (لڑی_مقابلہ(*cur, *bad, لڑی_لمبائی(*bad)) == 0) {
				إذا (org)
					*org = env_list_جوڑو_from_string(*org, *cur);
				آخری = ul_remove_اندراج(envp, cur - envp, آخری);
				cur--;
				اكسر;
			}
		}
	}

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = noslash; *bad; bad++) {
			إذا (لڑی_مقابلہ(*cur, *bad, لڑی_لمبائی(*bad)) != 0)
				استمر;
			إذا (!strchr(*cur, '/'))
				استمر;  /* OK */
			إذا (org)
				*org = env_list_جوڑو_from_string(*org, *cur);
			آخری = ul_remove_اندراج(envp, cur - envp, آخری);
			cur--;
			اكسر;
		}
	}
}

فراغ sanitize_env(فراغ)
{
	__sanitize_env(عدم);
}

حرف *محفوظ_getenv(ثابت حرف *arg)
{
	إذا (is_privileged_execution())
		أرجع عدم;
#جے_تعریف_شدہ HAVE_PRCTL
	إذا (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#وإلا
#إذا (defined(linux) && defined(SYS_prctl))
	إذا (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#جے_انت
#جے_انت
#جے_تعریف_شدہ HAVE_SECURE_GETENV
أرجع secure_getenv(arg);
#وإلاإذا HAVE___SECURE_GETENV
	أرجع __secure_getenv(arg);
#وإلا
	أرجع getenv(arg);
#جے_انت
}

#جے_تعریف_شدہ TEST_پروگرام_ENV
صحيح مکھ(فراغ)
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
#جے_انت
