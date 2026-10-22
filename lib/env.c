/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#ifdef HAVE_SYS_PRCTL_H
#અન્તર્ભાવ <sys/prctl.h>
#નહીંતર
#define PR_GET_DUMPABLE 3
#endif
#જો (!defined(HAVE_PRCTL) && defined(linux))
#અન્તર્ભાવ <sys/syscall.h>
#endif
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ "env.h"
#અન્તર્ભાવ "strv.h"
#અન્તર્ભાવ "all-io.h"

#ifndef HAVE_ENVIRON_DECL
બાહ્ય અક્ષર **environ;
#endif

સ્થિર અક્ષર * અચલ forbid[] = {
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
	(અક્ષર *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
સ્થિર અક્ષર * અચલ noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(અક્ષર *) 0
};


સંરચના ul_env_list {
	અક્ષર	*name;
	અક્ષર	*કિંમત;

	સંરચના ul_env_list *next;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
સ્થિર સંરચના ul_env_list *env_list_add( સંરચના ul_env_list *ls0,
					 અચલ અક્ષર *name, size_t namesz,
					 અચલ અક્ષર *કિંમત, size_t valsz)
{
	સંરચના ul_env_list *ls;

	ls = calloc(1, કદ(સંરચના ul_env_list) + namesz + valsz + 2);
	જો (!ls)
		પાછા ls0;

	ls->name = ((અક્ષર *) ls) + કદ(સંરચના ul_env_list);
	ls->કિંમત = ls->name + namesz + 1;

	memcpy(ls->name, name, namesz);
	memcpy(ls->કિંમત, કિંમત, valsz);

	ls->next = ls0;
	પાછા ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
સ્થિર સંરચના ul_env_list *env_list_add_from_string(સંરચના ul_env_list *ls,
						    અચલ અક્ષર *str)
{
	size_t namesz = 0, valsz = 0;
	અચલ અક્ષર *val;

	જો (!str || !*str)
		પાછા ls;

	val = strchr(str, '=');
	જો (!val)
		પાછા ls;
	namesz = val - str;

	val++;
	valsz = strlen(val);

	પાછા env_list_add(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
સંરચના ul_env_list *env_list_add_variable(
				સંરચના ul_env_list *ls,
				અચલ અક્ષર *name, અચલ અક્ષર *કિંમત)
{
	જો (!name || !*name)
		પાછા ls;

	પાછા env_list_add(ls, name, strlen(name), કિંમત, કિંમત ? strlen(કિંમત) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
સંરચના ul_env_list *env_list_add_getenv(સંરચના ul_env_list *ls,
				અચલ અક્ષર *name, અચલ અક્ષર *dflt)
{
	અચલ અક્ષર *val;

	જો (!name)
		પાછા ls;

	val = getenv(name);
	જો (!val)
		val= dflt;
	જો (val)
		ls = env_list_add_variable(ls, name, val);
	પાછા ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
સંરચના ul_env_list *env_list_add_getenvs(સંરચના ul_env_list *ls, અચલ અક્ષર *str)
{
	અક્ષર **all, **name;

	જો (!str)
		પાછા ls;

	all = ul_strv_split(str, ",");
	જો (!all)
		પાછા ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_add_getenv(ls, *name, ખાલી);

	ul_strv_free(all);
	પાછા ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
સંરચના ul_env_list *env_list_from_fd(પૂર્ણાંક fd)
{
	અક્ષર *buf = ખાલી, *p;
	ssize_t rc = 0;
	સંરચના ul_env_list *ls = ખાલી;

	errno = 0;
	જો ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		પાછા ખાલી;
	buf[rc] = '\0';
	p = buf;

	જ્યારે (rc > 0) {
		ls = env_list_add_from_string(ls, p);
		p += strlen(p) + 1;
		rc -= strlen(p) + 1;
	}

	મુક્ત(buf);
	પાછા ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
પૂર્ણાંક env_list_setenv(સંરચના ul_env_list *ls, પૂર્ણાંક overwrite)
{
	પૂર્ણાંક rc = 0;

	જ્યારે (ls && rc == 0) {
		જો (ls->name && ls->કિંમત)
			rc = setenv(ls->name, ls->કિંમત, overwrite);
		ls = ls->next;
	}
	પાછા rc;
}

શૂન્ય env_list_free(સંરચના ul_env_list *ls)
{
	જ્યારે (ls) {
		સંરચના ul_env_list *x = ls;
		ls = ls->next;
		મુક્ત(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
શૂન્ય __sanitize_env(સંરચના ul_env_list **org)
{
	અક્ષર **envp = environ;
	અક્ષર * અચલ *bad;
	અક્ષર **cur;
	પૂર્ણાંક last = 0;

	માટે (cur = envp; *cur; cur++)
		last++;

	માટે (cur = envp; *cur; cur++) {
		માટે (bad = forbid; *bad; bad++) {
			જો (strncmp(*cur, *bad, strlen(*bad)) == 0) {
				જો (org)
					*org = env_list_add_from_string(*org, *cur);
				last = ul_remove_entry(envp, cur - envp, last);
				cur--;
				તોડો;
			}
		}
	}

	માટે (cur = envp; *cur; cur++) {
		માટે (bad = noslash; *bad; bad++) {
			જો (strncmp(*cur, *bad, strlen(*bad)) != 0)
				ચાલુ;
			જો (!strchr(*cur, '/'))
				ચાલુ;  /* OK */
			જો (org)
				*org = env_list_add_from_string(*org, *cur);
			last = ul_remove_entry(envp, cur - envp, last);
			cur--;
			તોડો;
		}
	}
}

શૂન્ય sanitize_env(શૂન્ય)
{
	__sanitize_env(ખાલી);
}

અક્ષર *safe_getenv(અચલ અક્ષર *arg)
{
	જો (is_privileged_execution())
		પાછા ખાલી;
#ifdef HAVE_PRCTL
	જો (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		પાછા ખાલી;
#નહીંતર
#જો (defined(linux) && defined(SYS_prctl))
	જો (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		પાછા ખાલી;
#endif
#endif
#ifdef HAVE_SECURE_GETENV
પાછા secure_getenv(arg);
#અથવાજો HAVE___SECURE_GETENV
	પાછા __secure_getenv(arg);
#નહીંતર
	પાછા getenv(arg);
#endif
}

#ifdef TEST_PROGRAM_ENV
પૂર્ણાંક મુખ્ય(શૂન્ય)
{
	અક્ષર *અચલ *bad;
	અક્ષર copy[32];
	અક્ષર *p;
	પૂર્ણાંક retval = EXIT_SUCCESS;
	સંરચના ul_env_list *removed = ખાલી;

	/* define env. */
	માટે (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		જો (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	માટે (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		જો (p)
			*p = '\0';
		p = getenv(copy);
		જો (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	માટે (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		જો (p)
			*p = '\0';
		p = getenv(copy);
		જો (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	પાછા retval;
}
#endif
