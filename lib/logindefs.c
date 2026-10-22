/*
 * Copyright (C) 2003, 2004, 2005 Thorsten Kukuk
 * Author: Thorsten Kukuk <kukuk@suse.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain any existing copyright
 *    notice, and this entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 *
 * 2. Redistributions in binary form must reproduce all prior and current
 *    copyright notices, this list of conditions, and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. The name of any author may not be used to endorse or promote
 *    products derived from this software without their specific prior
 *   written permission.
 */
#అన్తర్భావ <assert.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/syslog.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <pwd.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "closestream.h"
#అన్తర్భావ "logindefs.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "xalloc.h"


స్థిర శూన్య (*logindefs_loader)(శూన్య *) = ఖాళీ;
స్థిర శూన్య *logindefs_loader_దత్తాంశం = ఖాళీ;

శూన్య logindefs_సమితి_loader(శూన్య (*loader)(శూన్య *దత్తాంశం), శూన్య *దత్తాంశం)
{
	logindefs_loader = loader;
	logindefs_loader_దత్తాంశం = దత్తాంశం;
}

#నిర్వచితం_కాకపోతే HAVE_LIBECONF

నిర్మాణం అంశం {
	అక్షరం *name;		/* name of the option.  */
	అక్షరం *విలువ;		/* value of the option.  */
	అక్షరం *మార్గం;		/* name of config file for this option.  */

	నిర్మాణం అంశం *తదుపరి;	/* pointer to next option.  */
};

స్థిర నిర్మాణం అంశం *list = ఖాళీ;

శూన్య free_getlogindefs_దత్తాంశం(శూన్య)
{
	నిర్మాణం అంశం *ptr;

	ptr = list;
	అయితే (ptr) {
		నిర్మాణం అంశం *tmp = ptr->తదుపరి;

		విడుదల(ptr->మార్గం);
		విడుదల(ptr->name);
		విడుదల(ptr->విలువ);
		విడుదల(ptr);
		ptr = tmp;
	}

	list = ఖాళీ;
}

స్థిర శూన్య store(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ, స్థిరరాశి అక్షరం *మార్గం)
{
	నిర్మాణం అంశం *new = xmalloc(పరిమాణం(నిర్మాణం అంశం));

	ఒకవేళ (!name)
		abort();

	new->name = xstrdup(name);
	new->విలువ = విలువ && *విలువ ? xstrdup(విలువ) : ఖాళీ;
	new->మార్గం = xstrdup(మార్గం);
	new->తదుపరి = list;
	list = new;
}

శూన్య logindefs_ఎక్కించు_file(స్థిరరాశి అక్షరం *filename)
{
	FILE *f;
	అక్షరం buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ఒకవేళ (!f)
		తిరిగి;

	అయితే (fgets(buf, పరిమాణం(buf), f)) {

		అక్షరం *p, *name, *దత్తాంశం = ఖాళీ;

		ఒకవేళ (*buf == '#' || *buf == '\n')
			కొనసాగు;	/* only comment or empty line */

		p = strchr(buf, '#');
		ఒకవేళ (p)
			*p = '\0';
		లేకపోతే {
			size_t n = అక్షరశ్రేణి_పొడవు(buf);
			ఒకవేళ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ఒకవేళ (!*buf)
			కొనసాగు;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		అయితే (*name && ఖాళీనా((గుర్తులేని)*name))
			name++;

		/* go to the end of the name */
		దత్తాంశం = name;
		అయితే (*దత్తాంశం && !(ఖాళీనా((గుర్తులేని)*దత్తాంశం) || *దత్తాంశం == '='))
			దత్తాంశం++;
		ఒకవేళ (దత్తాంశం > name && *దత్తాంశం)
			*దత్తాంశం++ = '\0';

		ఒకవేళ (!*name || దత్తాంశం == name)
			కొనసాగు;

		/* go to the begin of the value */
		అయితే (*దత్తాంశం
		       && (ఖాళీనా((గుర్తులేని)*దత్తాంశం) || *దత్తాంశం == '='
			   || *దత్తాంశం == '"'))
			దత్తాంశం++;

		/* remove space at the end of the value */
		p = దత్తాంశం + అక్షరశ్రేణి_పొడవు(దత్తాంశం);
		ఒకవేళ (p > దత్తాంశం)
			p--;
		అయితే (p > దత్తాంశం && (ఖాళీనా((గుర్తులేని)*p) || *p == '"'))
			*p-- = '\0';

		store(name, దత్తాంశం, filename);
	}

	fclose(f);
}

స్థిర శూన్య ఎక్కించు_defaults(శూన్య)
{
	ఒకవేళ (logindefs_loader)
		logindefs_loader(logindefs_loader_దత్తాంశం);
	లేకపోతే
		logindefs_ఎక్కించు_file(మార్గం_LOGINDEFS);
}

స్థిర నిర్మాణం అంశం *శోధన(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం అంశం *ptr;

	ఒకవేళ (!list)
		ఎక్కించు_defaults();

	ptr = list;
	అయితే (ptr != ఖాళీ) {
		ఒకవేళ (c_strcasecmp(name, ptr->name) == 0)
			తిరిగి ptr;
		ptr = ptr->తదుపరి;
	}

	తిరిగి ఖాళీ;
}

స్థిర స్థిరరాశి అక్షరం *శోధన_config(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం అంశం *ptr;

	ptr = list;
	అయితే (ptr != ఖాళీ) {
		ఒకవేళ (c_strcasecmp(name, ptr->name) == 0)
			తిరిగి ptr->మార్గం;
		ptr = ptr->తదుపరి;
	}

	తిరిగి ఖాళీ;
}

పూర్ణాంకం getlogindefs_bool(స్థిరరాశి అక్షరం *name, పూర్ణాంకం dflt)
{
	నిర్మాణం అంశం *ptr = శోధన(name);
	తిరిగి ptr && ptr->విలువ ? (c_strcasecmp(ptr->విలువ, "yes") == 0) : dflt;
}

గుర్తులేని దీర్ఘ getlogindefs_అంకె(స్థిరరాశి అక్షరం *name, గుర్తులేని దీర్ఘ dflt)
{
	నిర్మాణం అంశం *ptr = శోధన(name);
	అక్షరం *చివర = ఖాళీ;
	గుర్తులేని దీర్ఘ retval;

	ఒకవేళ (!ptr || !ptr->విలువ)
		తిరిగి dflt;

	errno = 0;
	retval = strtoul(ptr->విలువ, &చివర, 0);
	ఒకవేళ (చివర && *చివర == '\0' && !errno)
		తిరిగి retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       శోధన_config(name), name, ptr->విలువ);
	తిరిగి dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
స్థిరరాశి అక్షరం *getlogindefs_str(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *dflt)
{
	నిర్మాణం అంశం *ptr = శోధన(name);

	ఒకవేళ (!ptr)
		తిరిగి dflt;
	ఒకవేళ (!ptr->విలువ)
		తిరిగి "";
	తిరిగి ptr->విలువ;
}

#లేకపోతే /* !HAVE_LIBECONF */

#అన్తర్భావ "dl-econf.h"

స్థిర econf_file *file = ఖాళీ;

శూన్య free_getlogindefs_దత్తాంశం(శూన్య)
{
	ఒకవేళ (file)
		econf_call(econf_freeFile)(file);
	file = ఖాళీ;
}

స్థిర శూన్య ఎక్కించు_defaults(శూన్య)
{
	econf_err error;

	ఒకవేళ (file != ఖాళీ)
		free_getlogindefs_దత్తాంశం();

	ఒకవేళ (ul_dlopen_libeconf() == 0) {
#నిర్వచితమైతే HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ఖాళీ,
				UL_VENDORDIR_మార్గం, "login", "defs", "= \t", "#");
#లేకపోతే
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_మార్గం, "/etc", "login", "defs", "= \t", "#");
#అయితే_ముగింపు
		ఒకవేళ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ఒకవేళ (logindefs_loader)
		logindefs_loader(logindefs_loader_దత్తాంశం);

}

శూన్య logindefs_ఎక్కించు_file(స్థిరరాశి అక్షరం *filename)
{
	econf_file *file_l = ఖాళీ, *file_m = ఖాళీ;
	అక్షరం *మార్గం;

	logindefs_loader = ఖాళీ; /* No recursion */

	ఒకవేళ (ul_dlopen_libeconf() != 0)
		తిరిగి;

#ఒకవేళ USE_VENDORDIR
	xasprintf(&మార్గం, మార్గం_VENDORDIR"/%s", filename);

	ఒకవేళ (!econf_call(econf_readFile)(&file_l, మార్గం, "= \t", "#")) {
		ఒకవేళ (file == ఖాళీ)
			file = file_l;
		లేకపోతే ఒకవేళ (!econf_call(econf_విలీనం_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	విడుదల (మార్గం);
#అయితే_ముగింపు

	xasprintf(&మార్గం, "/etc/%s", filename);

	ఒకవేళ (!econf_call(econf_readFile)(&file_l, మార్గం, "= \t", "#")) {
		ఒకవేళ (file == ఖాళీ)
			file = file_l;
		లేకపోతే ఒకవేళ (!econf_call(econf_విలీనం_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} లేకపోతే ఒకవేళ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ఒకవేళ (file == ఖాళీ)
			file = file_l;
		లేకపోతే ఒకవేళ (!econf_call(econf_విలీనం_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	విడుదల (మార్గం);
}

పూర్ణాంకం getlogindefs_bool(స్థిరరాశి అక్షరం *name, పూర్ణాంకం dflt)
{
	bool విలువ;
	econf_err error;

	ఒకవేళ (!file)
		ఎక్కించు_defaults();

	ఒకవేళ (!file)
		తిరిగి dflt;

	ఒకవేళ ((error = econf_call(econf_get_Bool_విలువ)(file, ఖాళీ, name, &విలువ))) {
		ఒకవేళ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		తిరిగి dflt;
	}
	తిరిగి విలువ;
}

గుర్తులేని దీర్ఘ getlogindefs_అంకె(స్థిరరాశి అక్షరం *name, గుర్తులేని దీర్ఘ dflt)
{
	uint64_t విలువ;
	econf_err error;

	ఒకవేళ (!file)
		ఎక్కించు_defaults();

	ఒకవేళ (!file)
		తిరిగి dflt;

	ఒకవేళ ((error = econf_call(econf_get_UInt64_విలువ)(file, ఖాళీ, name, &విలువ))) {
		ఒకవేళ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		తిరిగి dflt;
	}
	తిరిగి విలువ;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
స్థిరరాశి అక్షరం *getlogindefs_str(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *dflt)
{
	అక్షరం *విలువ;
	econf_err error;

	ఒకవేళ (!file)
		ఎక్కించు_defaults();

	ఒకవేళ (!file)
		తిరిగి dflt;

	ఒకవేళ ((error = econf_call(econf_get_String_విలువ)(file, ఖాళీ, name, &విలువ))) {
		ఒకవేళ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		తిరిగి dflt;
	}
	ఒకవేళ (విలువ)
		తిరిగి విలువ;

	తిరిగి xstrdup("");
}
#అయితే_ముగింపు /* !HAVE_LIBECONF */

/*
 * For compatibility with shadow-utils we have to support additional
 * syntax for environment variables in login.defs(5) file. The standard
 * syntax is:
 *
 *	ENV_FOO   data
 *
 * but shadow-utils supports also
 *
 *	ENV_FOO FOO=data
 *
 * the FOO= prefix has to be remove before we call setenv().
 */
పూర్ణాంకం logindefs_setenv(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *conf, స్థిరరాశి అక్షరం *dflt)
{
	స్థిరరాశి అక్షరం *val = getlogindefs_str(conf, dflt);
	స్థిరరాశి అక్షరం *p;

	ఒకవేళ (!val)
		తిరిగి -1;

	p = strchr(val, '=');
	ఒకవేళ (p) {
		size_t sz = అక్షరశ్రేణి_పొడవు(name);

		ఒకవేళ (అక్షరశ్రేణి_పోలిక(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ఒకవేళ (*val == '"')
				val++;
			ఒకవేళ (!*val)
				val = dflt;
		}
	}

	తిరిగి val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
పూర్ణాంకం logindefs_setenv_మార్గం(uid_t uid)
{
	పూర్ణాంకం rc = 0;

	ఒకవేళ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	లేకపోతే ఒకవేళ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ఖాళీ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", మార్గం_DEFPATH_మూలం);

	తిరిగి rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
పూర్ణాంకం effective_access(స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం బహుళకం)
{
	పూర్ణాంకం fd = open(మార్గం, బహుళకం);
	ఒకవేళ (fd != -1)
		close(fd);
	తిరిగి fd == -1 ? -1 : 0;
}

/*
 * Check the per-account or the global hush-login setting.
 *
 * Hushed mode is enabled:
 *
 * a) if a global (e.g. /etc/hushlogins) hush file exists:
 *     1) for ALL ACCOUNTS if the file is empty
 *     2) for the current user if the username or shell is found in the file
 *
 * b) if a ~/.hushlogin file exists
 *
 * The ~/.hushlogin file is ignored if the global hush file exists.
 *
 * The HUSHLOGIN_FILE login.def variable overrides the default hush filename.
 *
 * Note that shadow-utils login(1) does not support "a1)". The "a1)" is
 * necessary if you want to use PAM for "Last login" message.
 *
 * -- Karel Zak <kzak@redhat.com> (26-Aug-2011)
 *
 *
 * The per-account check requires some explanation: As root we may not be able
 * to read the directory of the user if it is on an NFS-mounted filesystem. We
 * temporarily set our effective uid to the user-uid, making sure that we keep
 * root privileges in the real uid.
 *
 * A portable solution would require a fork(), but we rely on Linux having the
 * BSD setreuid().
 */

పూర్ణాంకం get_hushlogin_స్థితి(నిర్మాణం passwd *pwd, స్థిరరాశి అక్షరం *override_home, పూర్ణాంకం force_check)
{
	స్థిరరాశి అక్షరం *files[] = { మార్గం_HUSHLOGINS, మార్గం_HUSHLOGIN, ఖాళీ };
	స్థిరరాశి అక్షరం *file;
	అక్షరం buf[BUFSIZ];
	పూర్ణాంకం i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ఖాళీ);
	ఒకవేళ (file) {
		ఒకవేళ (!*file)
			తిరిగి 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ఖాళీ;
	}

	కొరకు (i = 0; files[i]; i++) {
		పూర్ణాంకం ok = 0;

		file = files[i];

		/* global hush-file */
		ఒకవేళ (*file == '/') {
			నిర్మాణం stat st;
			FILE *f;

			ఒకవేళ (stat(file, &st) != 0)
				కొనసాగు;	/* file does not exist */

			ఒకవేళ (st.st_size == 0)
				తిరిగి 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ఒకవేళ (!f)
				కొనసాగు;	/* ignore errors... */

			అయితే (ok == 0 && fgets(buf, పరిమాణం(buf), f)) {
				ఒకవేళ (buf[0] != '\0')
					buf[అక్షరశ్రేణి_పొడవు(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ఒకవేళ (ok)
				తిరిగి 1;	/* found username/shell */

			తిరిగి 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ఒకవేళ (అక్షరశ్రేణి_పొడవు(pwd->pw_dir) + అక్షరశ్రేణి_పొడవు(file) + 2 > పరిమాణం(buf))
			కొనసాగు;

		ఒకవేళ (snprintf(buf, పరిమాణం(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			కొనసాగు;

		ఒకవేళ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ఒకవేళ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ఒకవేళ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				నిష్క్రమించు(EXIT_FAILURE);
			}
			ఒకవేళ (ok)
				తిరిగి 1;	/* enabled by user */
		}
		లేకపోతే {
			పూర్ణాంకం rc;
			rc = effective_access(buf, O_RDONLY);
			ఒకవేళ (rc == 0)
				తిరిగి 1;

			ఒకవేళ (rc == -1 && errno == EACCES)
				తిరిగి -1;
		}

	}

	తిరిగి 0;
}
#నిర్వచితమైతే TEST_కార్యక్రమం
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	అక్షరం *name, *type;
	close_stdout_atexit();

	ఒకవేళ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_ఎక్కించు_file(argv[1]);

	ఒకవేళ (argc != 4) {	/* list all */
#నిర్వచితమైతే HAVE_LIBECONF
		పూర్ణాంకం i;
		అక్షరం *కీలకాలు[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ఖాళీ};

		కొరకు (i = 0; file && కీలకాలు[i] != ఖాళీ; i++) {
			అక్షరం *విలువ = ఖాళీ;

			econf_call(econf_get_String_విలువ)(file, ఖాళీ, కీలకాలు[i], &విలువ);
			రాయి ("%s: $%s: '%s'\n", argv[1], కీలకాలు[i], విలువ);
		}

		ఒకవేళ (file)
			econf_call(econf_freeFile)(file);

#లేకపోతే
		నిర్మాణం అంశం *ptr;

		కొరకు (ptr = list; ptr; ptr = ptr->తదుపరి)
			రాయి("%s: $%s: '%s'\n", ptr->మార్గం, ptr->name,
			       ptr->విలువ);
#అయితే_ముగింపు
		తిరిగి EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ఒకవేళ (strcmp(type, "str") == 0)
		రాయి("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	లేకపోతే ఒకవేళ (strcmp(type, "num") == 0)
		రాయి("$%s: '%lu'\n", name, getlogindefs_అంకె(name, 0));
	లేకపోతే ఒకవేళ (strcmp(type, "bool") == 0)
		రాయి("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు
