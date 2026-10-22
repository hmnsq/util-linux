/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#ଅନ୍ତର୍ଭାଵ <sys/prctl.h>
#ଅନ୍ୟଥା
#define PR_GET_DUMPABLE 3
#endif
#ଯଦି (!defined(HAVE_PRCTL) && defined(linux))
#ଅନ୍ତର୍ଭାଵ <sys/syscall.h>
#endif
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ "env.h"
#ଅନ୍ତର୍ଭାଵ "strv.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
ବାହ୍ୟ ଅକ୍ଷର **environ;
#endif

ସ୍ଥିର ଅକ୍ଷର * ଅଚଳ forbid[] = {
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
	(ଅକ୍ଷର *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
ସ୍ଥିର ଅକ୍ଷର * ଅଚଳ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(ଅକ୍ଷର *) 0
};


ସଂରଚନା ul_env_list {
	ଅକ୍ଷର	*name;
	ଅକ୍ଷର	*ମୂଲ୍ୟ;

	ସଂରଚନା ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
ସ୍ଥିର ସଂରଚନା ul_env_list *env_list_add( ସଂରଚନା ul_env_list *ls0,
					 ଅଚଳ ଅକ୍ଷର *name, size_t namesz,
					 ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, size_t valsz)
{
	ସଂରଚନା ul_env_list *ls;

	ls = calloc(1, ଆକାର(ସଂରଚନା ul_env_list) + namesz + valsz + 2);
	ଯଦି (!ls)
		ଫେରନ୍ତୁ ls0;

	ls->name = ((ଅକ୍ଷର *) ls) + ଆକାର(ସଂରଚନା ul_env_list);
	ls->ମୂଲ୍ୟ = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->ମୂଲ୍ୟ, ମୂଲ୍ୟ, valsz);

	ls->next = ls0;
	ଫେରନ୍ତୁ ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
ସ୍ଥିର ସଂରଚନା ul_env_list *env_list_add_from_string(ସଂରଚନା ul_env_list *ls,
						    ଅଚଳ ଅକ୍ଷର *str)
{
	size_t namesz = 0, valsz = 0;
	ଅଚଳ ଅକ୍ଷର *val;

	ଯଦି (!str || !*str)
		ଫେରନ୍ତୁ ls;

	val = strchr(str, '=');
	ଯଦି (!val)
		ଫେରନ୍ତୁ ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	ଫେରନ୍ତୁ env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
ସଂରଚନା ul_env_list *env_list_add_variable(
				ସଂରଚନା ul_env_list *ls,
				ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଯଦି (!name || !*name)
		ଫେରନ୍ତୁ ls;

	ଫେରନ୍ତୁ env_list_add(ls, name, strlen(name), ମୂଲ୍ୟ, ମୂଲ୍ୟ ? strlen(ମୂଲ୍ୟ) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
ସଂରଚନା ul_env_list *env_list_add_getenv(ସଂରଚନା ul_env_list *ls,
				ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *dflt)
{
	ଅଚଳ ଅକ୍ଷର *val;

	ଯଦି (!name)
		ଫେରନ୍ତୁ ls;

	val = getenv(name);
	ଯଦି (!val)
		val= dflt;
	ଯଦି (val)
		ls = env_list_add_variable(ls, name, val);
	ଫେରନ୍ତୁ ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
ସଂରଚନା ul_env_list *env_list_add_getenvs(ସଂରଚନା ul_env_list *ls, ଅଚଳ ଅକ୍ଷର *str)
{
	ଅକ୍ଷର **all, **name;

	ଯଦି (!str)
		ଫେରନ୍ତୁ ls;

	all = ul_strv_split(str, ",");
	ଯଦି (!all)
		ଫେରନ୍ତୁ ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, ଖାଲି);

	ul_strv_free(all);
	ଫେରନ୍ତୁ ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
ସଂରଚନା ul_env_list *env_list_from_fd(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ଅକ୍ଷର *buf = ଖାଲି, *p;
	ssize_t rc = 0;
	ସଂରଚନା ul_env_list *ls = ଖାଲି;

	errno = 0;
	ଯଦି ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		ଫେରନ୍ତୁ ଖାଲି;
	buf[rc] = '\0';
	p = buf;

	ଯେତେବେଳେ (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	ମୁକ୍ତ(buf);
	ଫେରନ୍ତୁ ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା env_list_setenv(ସଂରଚନା ul_env_list *ls, ପୂର୍ଣ୍ଣସଂଖ୍ୟା overwrite)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯେତେବେଳେ (ls && rc == 0) {
		ଯଦି (ls->name && ls->ମୂଲ୍ୟ)
			rc = setenv(ls->name, ls->ମୂଲ୍ୟ, overwrite);
		ls = ls->next;
	}
	ଫେରନ୍ତୁ rc;
}

ଶୂନ୍ୟ env_list_free(ସଂରଚନା ul_env_list *ls)
{
	ଯେତେବେଳେ (ls) {
		ସଂରଚନା ul_env_list *x = ls;
		ls = ls->next;
		ମୁକ୍ତ(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
ଶୂନ୍ୟ __sanitize_env(ସଂରଚନା ul_env_list **org)
{
	ଅକ୍ଷର **envp = environ;
	ଅକ୍ଷର * ଅଚଳ *bad;
	ଅକ୍ଷର **cur;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା last = 0;

	ପାଇଁ (cur = envp; *cur; cur++)
		last++;

	ପାଇଁ (cur = envp; *cur; cur++) {
		ପାଇଁ (bad = forbid; *bad; bad++) {
			ଯଦି (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				ଯଦି (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				ଭାଙ୍ଗ;
			}
		}
	}

	ପାଇଁ (cur = envp; *cur; cur++) {
		ପାଇଁ (bad = noslash; *bad; bad++) {
			ଯଦି (strncmp(*cur, *bad, strlen(*bad)) != 0)
				ଜାରି;
			ଯଦି (!strchr(*cur, '/'))
				ଜାରି;  /* OK */
			ଯଦି (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			ଭାଙ୍ଗ;
		}
	}
}

ଶୂନ୍ୟ sanitize_env(ଶୂନ୍ୟ)
{
	__sanitize_env(ଖାଲି);
}

ଅକ୍ଷର *safe_getenv(ଅଚଳ ଅକ୍ଷର *arg)
{
	ଯଦି (is_privileged_execution())
		ଫେରନ୍ତୁ ଖାଲି;
#ifdef HAVE_PRCTL
	ଯଦି (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ଫେରନ୍ତୁ ଖାଲି;
#ଅନ୍ୟଥା
#ଯଦି (defined(linux) && defined(SYS_prctl))
	ଯଦି (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		ଫେରନ୍ତୁ ଖାଲି;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
ଫେରନ୍ତୁ secure_getenv(arg);
#ନଚେତଯଦି HAVE___SECURE_GETENV
	ଫେରନ୍ତୁ __secure_getenv(arg);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *ଅଚଳ *bad;
	ଅକ୍ଷର copy[32];
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_SUCCESS;
	ସଂରଚନା ul_env_list *removed = ଖାଲି;

	/* define env. */
	ପାଇଁ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ଯଦି (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	ପାଇଁ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ଯଦି (p)
			*p = '\0';
		p = getenv(copy);
		ଯଦି (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	ପାଇଁ (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ଯଦି (p)
			*p = '\0';
		p = getenv(copy);
		ଯଦି (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	ଫେରନ୍ତୁ retval;
}
#endif
