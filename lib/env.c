/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#جيڪڏهن_وصف_ٿيل HAVE_SYS_PRCTL_H
#ضمن <sys/prctl.h>
#وإلا
#وصف_ڪريو PR_GET_DUMPABLE 3
#جيڪڏهن_پورو
#إذا (!defined(HAVE_PRCTL) && defined(linux))
#ضمن <sys/syscall.h>
#جيڪڏهن_پورو
#ضمن <unistd.h>
#ضمن <sys/types.h>

#ضمن "env.h"
#ضمن "strv.h"
#ضمن "all-io.h"

#جيڪڏهن_وصف_نه_ٿيل HAVE_ENVIRON_DECL
خارجي حرف **environ;
#جيڪڏهن_پورو

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

	بنية ul_env_list *ايندڙ;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جوڙيو( بنية ul_env_list *ls0,
					 ثابت حرف *name, size_t namesz,
					 ثابت حرف *قدر, size_t valsz)
{
	بنية ul_env_list *ls;

	ls = صفر_يادگيري_ڏيو(1, حجم(بنية ul_env_list) + namesz + valsz + 2);
	إذا (!ls)
		أرجع ls0;

	ls->name = ((حرف *) ls) + حجم(بنية ul_env_list);
	ls->قدر = ls->name + namesz + 1;

	يادگيري_نقل(ls->name, name, namesz);
	يادگيري_نقل(ls->قدر, قدر, valsz);

	ls->ايندڙ = ls0;
	أرجع ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ساكن بنية ul_env_list *env_list_جوڙيو_from_string(بنية ul_env_list *ls,
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
	valsz = لڙي_ڊيگهه(val);

	أرجع env_list_جوڙيو(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
بنية ul_env_list *env_list_جوڙيو_variable(
				بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *قدر)
{
	إذا (!name || !*name)
		أرجع ls;

	أرجع env_list_جوڙيو(ls, name, لڙي_ڊيگهه(name), قدر, قدر ? لڙي_ڊيگهه(قدر) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
بنية ul_env_list *env_list_جوڙيو_getenv(بنية ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *dflt)
{
	ثابت حرف *val;

	إذا (!name)
		أرجع ls;

	val = getenv(name);
	إذا (!val)
		val= dflt;
	إذا (val)
		ls = env_list_جوڙيو_variable(ls, name, val);
	أرجع ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
بنية ul_env_list *env_list_جوڙيو_getenvs(بنية ul_env_list *ls, ثابت حرف *str)
{
	حرف **all, **name;

	إذا (!str)
		أرجع ls;

	all = ul_strv_ورهايو(str, ",");
	إذا (!all)
		أرجع ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_جوڙيو_getenv(ls, *name, عدم);

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
		ls = env_list_جوڙيو_from_string(ls, p);
		p += لڙي_ڊيگهه(p) + 1;
		rc -= لڙي_ڊيگهه(p) + 1;
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
		ls = ls->ايندڙ;
	}
	أرجع rc;
}

فراغ env_list_free(بنية ul_env_list *ls)
{
	طالما (ls) {
		بنية ul_env_list *x = ls;
		ls = ls->ايندڙ;
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
	صحيح آخري = 0;

	لكل (cur = envp; *cur; cur++)
		آخري++;

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = forbid; *bad; bad++) {
			إذا (لڙي_ڀيٽ(*cur, *bad, لڙي_ڊيگهه(*bad)) == 0) {
				إذا (org)
					*org = env_list_جوڙيو_from_string(*org, *cur);
				آخري = ul_remove_داخلا(envp, cur - envp, آخري);
				cur--;
				اكسر;
			}
		}
	}

	لكل (cur = envp; *cur; cur++) {
		لكل (bad = noslash; *bad; bad++) {
			إذا (لڙي_ڀيٽ(*cur, *bad, لڙي_ڊيگهه(*bad)) != 0)
				استمر;
			إذا (!strchr(*cur, '/'))
				استمر;  /* OK */
			إذا (org)
				*org = env_list_جوڙيو_from_string(*org, *cur);
			آخري = ul_remove_داخلا(envp, cur - envp, آخري);
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
#جيڪڏهن_وصف_ٿيل HAVE_PRCTL
	إذا (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#وإلا
#إذا (defined(linux) && defined(SYS_prctl))
	إذا (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		أرجع عدم;
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل HAVE_SECURE_GETENV
أرجع secure_getenv(arg);
#وإلاإذا HAVE___SECURE_GETENV
	أرجع __secure_getenv(arg);
#وإلا
	أرجع getenv(arg);
#جيڪڏهن_پورو
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_ENV
صحيح مکيه(فراغ)
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
#جيڪڏهن_پورو
