/*
 * environ[] array cleanup code and getenv() wrappers
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#నిర్వచితమైతే HAVE_SYS_PRCTL_H
#అన్తర్భావ <sys/prctl.h>
#లేకపోతే
#నిర్వచించు PR_GET_DUMPABLE 3
#అయితే_ముగింపు
#ఒకవేళ (!defined(HAVE_PRCTL) && defined(linux))
#అన్తర్భావ <sys/syscall.h>
#అయితే_ముగింపు
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "env.h"
#అన్తర్భావ "strv.h"
#అన్తర్భావ "all-io.h"

#నిర్వచితం_కాకపోతే HAVE_ENVIRON_DECL
బాహ్య అక్షరం **environ;
#అయితే_ముగింపు

స్థిర అక్షరం * స్థిరరాశి forbid[] = {
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
	(అక్షరం *) 0
};

/* these are allowed, but with no slashes inside
   (to work around security problems in GNU gettext) */
స్థిర అక్షరం * స్థిరరాశి noslash[] = {
	"LANG=",
	"LANGUAGE=",
	"LC_",	  /* anything with the LC_ prefix */
	(అక్షరం *) 0
};


నిర్మాణం ul_env_list {
	అక్షరం	*name;
	అక్షరం	*విలువ;

	నిర్మాణం ul_env_list *తదుపరి;
};

/*
 * Saves to the list and returns a pointer to the new head of the list.
 */
స్థిర నిర్మాణం ul_env_list *env_list_కూడు( నిర్మాణం ul_env_list *ls0,
					 స్థిరరాశి అక్షరం *name, size_t namesz,
					 స్థిరరాశి అక్షరం *విలువ, size_t valsz)
{
	నిర్మాణం ul_env_list *ls;

	ls = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం ul_env_list) + namesz + valsz + 2);
	ఒకవేళ (!ls)
		తిరిగి ls0;

	ls->name = ((అక్షరం *) ls) + పరిమాణం(నిర్మాణం ul_env_list);
	ls->విలువ = ls->name + namesz + 1;

	జ్ఞాపకం_నకలు(ls->name, name, namesz);
	జ్ఞాపకం_నకలు(ls->విలువ, విలువ, valsz);

	ls->తదుపరి = ls0;
	తిరిగి ls;
}

/*
 * Saves the @str (with the name=value string) to the @ls and returns a pointer
 * to the new head of the list.
 */
స్థిర నిర్మాణం ul_env_list *env_list_కూడు_from_string(నిర్మాణం ul_env_list *ls,
						    స్థిరరాశి అక్షరం *str)
{
	size_t namesz = 0, valsz = 0;
	స్థిరరాశి అక్షరం *val;

	ఒకవేళ (!str || !*str)
		తిరిగి ls;

	val = strchr(str, '=');
	ఒకవేళ (!val)
		తిరిగి ls;
	namesz = val - str;

	val++;
	valsz = అక్షరశ్రేణి_పొడవు(val);

	తిరిగి env_list_కూడు(ls, str, namesz, val, valsz);
}

/*
 * Saves the @name and @value to @ls0 and returns a pointer to the new head of
 * the list.
 */
నిర్మాణం ul_env_list *env_list_కూడు_variable(
				నిర్మాణం ul_env_list *ls,
				స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ)
{
	ఒకవేళ (!name || !*name)
		తిరిగి ls;

	తిరిగి env_list_కూడు(ls, name, అక్షరశ్రేణి_పొడవు(name), విలువ, విలువ ? అక్షరశ్రేణి_పొడవు(విలువ) : 0);
}

/*
 * Calls getenv() and adds the result to the list.
 */
నిర్మాణం ul_env_list *env_list_కూడు_getenv(నిర్మాణం ul_env_list *ls,
				స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *dflt)
{
	స్థిరరాశి అక్షరం *val;

	ఒకవేళ (!name)
		తిరిగి ls;

	val = getenv(name);
	ఒకవేళ (!val)
		val= dflt;
	ఒకవేళ (val)
		ls = env_list_కూడు_variable(ls, name, val);
	తిరిగి ls;
}

/*
 * Calls getenv() for each name (comma-separated) in @str and adds the results
 * to the list.
 */
నిర్మాణం ul_env_list *env_list_కూడు_getenvs(నిర్మాణం ul_env_list *ls, స్థిరరాశి అక్షరం *str)
{
	అక్షరం **all, **name;

	ఒకవేళ (!str)
		తిరిగి ls;

	all = ul_strv_విభజించు(str, ",");
	ఒకవేళ (!all)
		తిరిగి ls;

	UL_STRV_FOREACH(name, all)
		ls = env_list_కూడు_getenv(ls, *name, ఖాళీ);

	ul_strv_free(all);
	తిరిగి ls;
}

/*
 * Use env_list_from_fd() to read environment from @fd.
 *
 * @fd must be /proc/<pid>/environ file.
*/
నిర్మాణం ul_env_list *env_list_from_fd(పూర్ణాంకం fd)
{
	అక్షరం *buf = ఖాళీ, *p;
	ssize_t rc = 0;
	నిర్మాణం ul_env_list *ls = ఖాళీ;

	errno = 0;
	ఒకవేళ ((rc = ul_read_all_alloc(fd, &buf)) < 1)
		తిరిగి ఖాళీ;
	buf[rc] = '\0';
	p = buf;

	అయితే (rc > 0) {
		ls = env_list_కూడు_from_string(ls, p);
		p += అక్షరశ్రేణి_పొడవు(p) + 1;
		rc -= అక్షరశ్రేణి_పొడవు(p) + 1;
	}

	విడుదల(buf);
	తిరిగి ls;
}

/*
 * Use setenv() for all stuff in @ls.
 */
పూర్ణాంకం env_list_setenv(నిర్మాణం ul_env_list *ls, పూర్ణాంకం overwrite)
{
	పూర్ణాంకం rc = 0;

	అయితే (ls && rc == 0) {
		ఒకవేళ (ls->name && ls->విలువ)
			rc = setenv(ls->name, ls->విలువ, overwrite);
		ls = ls->తదుపరి;
	}
	తిరిగి rc;
}

శూన్య env_list_free(నిర్మాణం ul_env_list *ls)
{
	అయితే (ls) {
		నిర్మాణం ul_env_list *x = ls;
		ls = ls->తదుపరి;
		విడుదల(x);
	}
}

/*
 * Removes unwanted variables from environ[]. If @org is not NULL than stores
 * unwnated variables to the list.
 */
శూన్య __sanitize_env(నిర్మాణం ul_env_list **org)
{
	అక్షరం **envp = environ;
	అక్షరం * స్థిరరాశి *bad;
	అక్షరం **cur;
	పూర్ణాంకం చివరి = 0;

	కొరకు (cur = envp; *cur; cur++)
		చివరి++;

	కొరకు (cur = envp; *cur; cur++) {
		కొరకు (bad = forbid; *bad; bad++) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(*cur, *bad, అక్షరశ్రేణి_పొడవు(*bad)) == 0) {
				ఒకవేళ (org)
					*org = env_list_కూడు_from_string(*org, *cur);
				చివరి = ul_remove_నమోదు(envp, cur - envp, చివరి);
				cur--;
				ఆపు;
			}
		}
	}

	కొరకు (cur = envp; *cur; cur++) {
		కొరకు (bad = noslash; *bad; bad++) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(*cur, *bad, అక్షరశ్రేణి_పొడవు(*bad)) != 0)
				కొనసాగు;
			ఒకవేళ (!strchr(*cur, '/'))
				కొనసాగు;  /* OK */
			ఒకవేళ (org)
				*org = env_list_కూడు_from_string(*org, *cur);
			చివరి = ul_remove_నమోదు(envp, cur - envp, చివరి);
			cur--;
			ఆపు;
		}
	}
}

శూన్య sanitize_env(శూన్య)
{
	__sanitize_env(ఖాళీ);
}

అక్షరం *సురక్షిత_getenv(స్థిరరాశి అక్షరం *arg)
{
	ఒకవేళ (is_privileged_execution())
		తిరిగి ఖాళీ;
#నిర్వచితమైతే HAVE_PRCTL
	ఒకవేళ (prctl(PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		తిరిగి ఖాళీ;
#లేకపోతే
#ఒకవేళ (defined(linux) && defined(SYS_prctl))
	ఒకవేళ (syscall(SYS_prctl, PR_GET_DUMPABLE, 0, 0, 0, 0) == 0)
		తిరిగి ఖాళీ;
#అయితే_ముగింపు
#అయితే_ముగింపు
#నిర్వచితమైతే HAVE_SECURE_GETENV
తిరిగి secure_getenv(arg);
#లేదావేళ HAVE___SECURE_GETENV
	తిరిగి __secure_getenv(arg);
#లేకపోతే
	తిరిగి getenv(arg);
#అయితే_ముగింపు
}

#నిర్వచితమైతే TEST_కార్యక్రమం_ENV
పూర్ణాంకం ప్రధాన(శూన్య)
{
	అక్షరం *స్థిరరాశి *bad;
	అక్షరం copy[32];
	అక్షరం *p;
	పూర్ణాంకం retval = EXIT_SUCCESS;
	నిర్మాణం ul_env_list *removed = ఖాళీ;

	/* define env. */
	కొరకు (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ఒకవేళ (p)
			*p = '\0';
		setenv(copy, "abc", 1);
	}

	/* removed */
	__sanitize_env(&removed);

	/* check removal */
	కొరకు (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ఒకవేళ (p)
			*p = '\0';
		p = getenv(copy);
		ఒకవేళ (p) {
			warnx("%s was not removed", copy);
			retval = EXIT_FAILURE;
		}
	}

	/* restore removed */
	env_list_setenv(removed, 0);

	/* check restore */
	కొరకు (bad = forbid; *bad; bad++) {
		strcpy(copy, *bad);
		p = strchr(copy, '=');
		ఒకవేళ (p)
			*p = '\0';
		p = getenv(copy);
		ఒకవేళ (!p) {
			warnx("%s was not restored", copy);
			retval = EXIT_FAILURE;
		}
	}

	env_list_free(removed);

	తిరిగి retval;
}
#అయితే_ముగింపు
