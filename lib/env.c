/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#அந்தர்பாவ <sys/prctl.h>
#இல்லையெனில்
#define PR_GET_DUMPABLE 3
#endif
#எனில் (!defined(HAVE_PRCTL) && defined(linux))
#அந்தர்பாவ <sys/syscall.h>
#endif
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "env.h"
#அந்தர்பாவ "strv.h"
#அந்தர்பாவ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
வெளி எழுத்து **environ;
#endif

நிலையான எழுத்து * மாறிலி forbid[] = {
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
	(எழுத்து *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
நிலையான எழுத்து * மாறிலி noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(எழுத்து *) 0
};


கட்டமைப்பு ul_env_list {
	எழுத்து	*name;
	எழுத்து	*மதிப்பு;

	கட்டமைப்பு ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
நிலையான கட்டமைப்பு ul_env_list *env_list_add( கட்டமைப்பு ul_env_list *ls0,
					 மாறிலி எழுத்து *name, size_t namesz,
					 மாறிலி எழுத்து *மதிப்பு, size_t valsz)
{
	கட்டமைப்பு ul_env_list *ls;

	ls = calloc(1, அளவு(கட்டமைப்பு ul_env_list) + namesz + valsz + 2);
	எனில் (!ls)
		திரும்பு ls0;

	ls->name = ((எழுத்து *) ls) + அளவு(கட்டமைப்பு ul_env_list);
	ls->மதிப்பு = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->மதிப்பு, மதிப்பு, valsz);

	ls->next = ls0;
	திரும்பு ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
நிலையான கட்டமைப்பு ul_env_list *env_list_add_from_string(கட்டமைப்பு ul_env_list *ls,
						    மாறிலி எழுத்து *str)
{
	size_t namesz = 0, valsz = 0;
	மாறிலி எழுத்து *val;

	எனில் (!str || !*str)
		திரும்பு ls;

	val = strchr(str, '=');
	எனில் (!val)
		திரும்பு ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	திரும்பு env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
கட்டமைப்பு ul_env_list *env_list_add_variable(
				கட்டமைப்பு ul_env_list *ls,
				மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு)
{
	எனில் (!name || !*name)
		திரும்பு ls;

	திரும்பு env_list_add(ls, name, strlen(name), மதிப்பு, மதிப்பு ? strlen(மதிப்பு) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
கட்டமைப்பு ul_env_list *env_list_add_getenv(கட்டமைப்பு ul_env_list *ls,
				மாறிலி எழுத்து *name, மாறிலி எழுத்து *dflt)
{
	மாறிலி எழுத்து *val;

	எனில் (!name)
		திரும்பு ls;

	val = getenv(name);
	எனில் (!val)
		val= dflt;
	எனில் (val)
		ls = env_list_add_variable(ls, name, val);
	திரும்பு ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
கட்டமைப்பு ul_env_list *env_list_add_getenvs(கட்டமைப்பு ul_env_list *ls, மாறிலி எழுத்து *str)
{
	எழுத்து **all, **name;

	எனில் (!str)
		திரும்பு ls;

	all = ul_strv_split(str, ",");
	எனில் (!all)
		திரும்பு ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, வெற்று);

	ul_strv_free(all);
	திரும்பு ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
கட்டமைப்பு ul_env_list *env_list_from_fd(முழு fd)
{
	எழுத்து *buf = வெற்று, *p;
	ssize_t rc = 0;
	கட்டமைப்பு ul_env_list *ls = வெற்று;

	errno = 0;
	எனில் ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		திரும்பு வெற்று;
	buf[rc] = '\0';
	p = buf;

	போது (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	விடுவி(buf);
	திரும்பு ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
முழு env_list_setenv(கட்டமைப்பு ul_env_list *ls, முழு overwrite)
{
	முழு rc = 0;

	போது (ls && rc == 0) {
		எனில் (ls->name && ls->மதிப்பு)
			rc = setenv(ls->name, ls->மதிப்பு, overwrite);
		ls = ls->next;
	}
	திரும்பு rc;
}

வெறுமை env_list_free(கட்டமைப்பு ul_env_list *ls)
{
	போது (ls) {
		கட்டமைப்பு ul_env_list *x = ls;
		ls = ls->next;
		விடுவி(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
வெறுமை __sanitize_env(கட்டமைப்பு ul_env_list **org)
{
	எழுத்து **envp = environ;
	எழுத்து * மாறிலி *bad;
	எழுத்து **cur;
	முழு last = 0;

	க்கு (cur = envp; *cur; cur++)
		last++;

	க்கு (cur = envp; *cur; cur++) {
		க்கு (bad = forbid; *bad; bad++) {
			எனில் (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				எனில் (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				நிறுத்து;
			}
		}
	}

	க்கு (cur = envp; *cur; cur++) {
		க்கு (bad = noslash; *bad; bad++) {
			எனில் (strncmp(*cur, *bad, strlen(*bad)) != 0)
				தொடர்;
			எனில் (!strchr(*cur, '/'))
				தொடர்;  /* OK */
			எனில் (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			நிறுத்து;
		}
	}
}

வெறுமை sanitize_env(வெறுமை)
{
	__sanitize_env(வெற்று);
}

எழுத்து *safe_getenv(மாறிலி எழுத்து *arg)
{
	எனில் (is_privileged_execution())
		திரும்பு வெற்று;
#ifdef HAVE_PRCTL
	எனில் (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		திரும்பு வெற்று;
#இல்லையெனில்
#எனில் (defined(linux) && defined(SYS_prctl))
	எனில் (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		திரும்பு வெற்று;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
திரும்பு secure_getenv(arg);
#அல்லதெனில் HAVE___SECURE_GETENV
	திரும்பு __secure_getenv(arg);
#இல்லையெனில்
	திரும்பு getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
முழு முதன்மை(வெறுமை)
{
	எழுத்து *மாறிலி *bad;
	எழுத்து copy[32];
	எழுத்து *p;
	முழு retval = EXIT_SUCCESS;
	கட்டமைப்பு ul_env_list *removed = வெற்று;

	/* define env. */
	க்கு (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		எனில் (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	க்கு (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		எனில் (p)
			*p = '\0';
		p = getenv(copy);
		எனில் (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	க்கு (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		எனில் (p)
			*p = '\0';
		p = getenv(copy);
		எனில் (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	திரும்பு retval;
}
#endif
