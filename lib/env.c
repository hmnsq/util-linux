/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#اگر_تعریف_شدہ HAVE_SYS_PRCTL_H
#شامل <sys/prctl.h>
#ورنہ
#تعریف_کرو PR_GET_DUMPABLE 3
#اگر_ختم
#اگر (!defined(HAVE_PRCTL) && defined(linux))
#شامل <sys/syscall.h>
#اگر_ختم
#شامل <unistd.h>
#شامل <sys/types.h>

#شامل "env.h"
#شامل "strv.h"
#شامل "all-io.h"

#اگر_غیر_تعریف_شدہ HAVE_ENVIRON_DECL
خارجی حرف **environ;
#اگر_ختم

ساکن حرف * ثابت forbid[] = {
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
ساکن حرف * ثابت noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(حرف *) 0
};


ساخت ul_env_list {
	حرف	*name;
	حرف	*قدر;

	ساخت ul_env_list *اگلا;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ساکن ساخت ul_env_list *env_list_جوڑو( ساخت ul_env_list *ls0,
					 ثابت حرف *name, size_t namesz,
					 ثابت حرف *قدر, size_t valsz)
{
	ساخت ul_env_list *ls;

	ls = صفر_حافظہ_دو(1, حجم(ساخت ul_env_list) + namesz + valsz + 2);
	اگر (!ls)
		واپس ls0;

	ls->name = ((حرف *) ls) + حجم(ساخت ul_env_list);
	ls->قدر = ls->name + namesz + 1;

	حافظہ_نقل(ls->name, name, namesz);
	حافظہ_نقل(ls->قدر, قدر, valsz);

	ls->اگلا = ls0;
	واپس ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ساکن ساخت ul_env_list *env_list_جوڑو_from_string(ساخت ul_env_list *ls,
						    ثابت حرف *str)
{
	size_t namesz = 0, valsz = 0;
	ثابت حرف *val;

	اگر (!str || !*str)
		واپس ls;

	val = strchr(str, '=');
	اگر (!val)
		واپس ls;
	namesz = val - str;

	val++;
	valsz = لڑی_لمبائی(val);

	واپس env_list_جوڑو(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ساخت ul_env_list *env_list_جوڑو_variable(
				ساخت ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *قدر)
{
	اگر (!name || !*name)
		واپس ls;

	واپس env_list_جوڑو(ls, name, لڑی_لمبائی(name), قدر, قدر ? لڑی_لمبائی(قدر) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ساخت ul_env_list *env_list_جوڑو_getenv(ساخت ul_env_list *ls,
				ثابت حرف *name, ثابت حرف *dflt)
{
	ثابت حرف *val;

	اگر (!name)
		واپس ls;

	val = getenv(name);
	اگر (!val)
		val= dflt;
	اگر (val)
		ls = env_list_جوڑو_variable(ls, name, val);
	واپس ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ساخت ul_env_list *env_list_جوڑو_getenvs(ساخت ul_env_list *ls, ثابت حرف *str)
{
	حرف **all, **name;

	اگر (!str)
		واپس ls;

	all = ul_strv_بانٹو(str, ",");
	اگر (!all)
		واپس ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_جوڑو_getenv(ls, *name, تہی);

	ul_strv_free(all);
	واپس ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ساخت ul_env_list *env_list_from_fd(صحیح fd)
{
	حرف *buf = تہی, *p;
	ssize_t rc = 0;
	ساخت ul_env_list *ls = تہی;

	errno = 0;
	اگر ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		واپس تہی;
	buf[rc] = '\0';
	p = buf;

	جب (rc > 0) {
		ls = env_list_جوڑو_from_string(ls, p);
		p += لڑی_لمبائی(p) + 1;
		rc -= لڑی_لمبائی(p) + 1;
	}

	آزاد(buf);
	واپس ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
صحیح env_list_setenv(ساخت ul_env_list *ls, صحیح overwrite)
{
	صحیح rc = 0;

	جب (ls && rc == 0) {
		اگر (ls->name && ls->قدر)
			rc = setenv(ls->name, ls->قدر, overwrite);
		ls = ls->اگلا;
	}
	واپس rc;
}

خالی env_list_free(ساخت ul_env_list *ls)
{
	جب (ls) {
		ساخت ul_env_list *x = ls;
		ls = ls->اگلا;
		آزاد(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
خالی __sanitize_env(ساخت ul_env_list **org)
{
	حرف **envp = environ;
	حرف * ثابت *bad;
	حرف **cur;
	صحیح آخری = 0;

	برائے (cur = envp; *cur; cur++)
		آخری++;

	برائے (cur = envp; *cur; cur++) {
		برائے (bad = forbid; *bad; bad++) {
			اگر (لڑی_موازنہ(*cur, *bad, لڑی_لمبائی(*bad)) == 0) {
				اگر (org)
					*org = env_list_جوڑو_from_string(*org, *cur);
				آخری = ul_remove_اندراج(envp, cur - envp, آخری);
				cur--;
				توڑو;
			}
		}
	}

	برائے (cur = envp; *cur; cur++) {
		برائے (bad = noslash; *bad; bad++) {
			اگر (لڑی_موازنہ(*cur, *bad, لڑی_لمبائی(*bad)) != 0)
				جاری;
			اگر (!strchr(*cur, '/'))
				جاری;  /* OK */
			اگر (org)
				*org = env_list_جوڑو_from_string(*org, *cur);
			آخری = ul_remove_اندراج(envp, cur - envp, آخری);
			cur--;
			توڑو;
		}
	}
}

خالی sanitize_env(خالی)
{
	__sanitize_env(تہی);
}

حرف *محفوظ_getenv(ثابت حرف *arg)
{
	اگر (is_privileged_execution())
		واپس تہی;
#اگر_تعریف_شدہ HAVE_PRCTL
	اگر (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		واپس تہی;
#ورنہ
#اگر (defined(linux) && defined(SYS_prctl))
	اگر (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		واپس تہی;
#اگر_ختم
#اگر_ختم
#اگر_تعریف_شدہ HAVE_SECURE_GETENV
واپس secure_getenv(arg);
#وگرنہ_اگر HAVE___SECURE_GETENV
	واپس __secure_getenv(arg);
#ورنہ
	واپس getenv(arg);
#اگر_ختم
}

#اگر_تعریف_شدہ TEST_پروگرام_ENV
صحیح مین(خالی)
{
	حرف *ثابت *bad;
	حرف copy[32];
	حرف *p;
	صحیح retval = EXIT_SUCCESS;
	ساخت ul_env_list *removed = تہی;

	/* define env. */
	برائے (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	برائے (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		p = getenv(copy);
		اگر (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	برائے (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		اگر (p)
			*p = '\0';
		p = getenv(copy);
		اگر (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	واپس retval;
}
#اگر_ختم
