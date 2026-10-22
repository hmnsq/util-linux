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
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/syslog.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <pwd.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "closestream.h"
#অন্তর্ভাব "logindefs.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "xalloc.h"


স্থির শূন্য (*logindefs_loader)(শূন্য *) = ফাঁকা;
স্থির শূন্য *logindefs_loader_তথ্য = ফাঁকা;

শূন্য logindefs_set_loader(শূন্য (*loader)(শূন্য *তথ্য), শূন্য *তথ্য)
{
	logindefs_loader = loader;
	logindefs_loader_তথ্য = তথ্য;
}

#ifndef HAVE_LIBECONF

কাঠামো item {
	অক্ষর *name;		/* name of the option.  */
	অক্ষর *মান;		/* value of the option.  */
	অক্ষর *path;		/* name of config file for this option.  */

	কাঠামো item *পরবর্তী;	/* pointer to next option.  */
};

স্থির কাঠামো item *list = ফাঁকা;

শূন্য free_getlogindefs_তথ্য(শূন্য)
{
	কাঠামো item *ptr;

	ptr = list;
	যখন (ptr) {
		কাঠামো item *tmp = ptr->পরবর্তী;

		মুক্ত(ptr->path);
		মুক্ত(ptr->name);
		মুক্ত(ptr->মান);
		মুক্ত(ptr);
		ptr = tmp;
	}

	list = ফাঁকা;
}

স্থির শূন্য store(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান, ধ্রুবক অক্ষর *path)
{
	কাঠামো item *new = xmalloc(আকার(কাঠামো item));

	যদি (!name)
		abort();

	new->name = xstrdup(name);
	new->মান = মান && *মান ? xstrdup(মান) : ফাঁকা;
	new->path = xstrdup(path);
	new->পরবর্তী = list;
	list = new;
}

শূন্য logindefs_load_file(ধ্রুবক অক্ষর *filename)
{
	FILE *f;
	অক্ষর buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	যদি (!f)
		ফেরত;

	যখন (fgets(buf, আকার(buf), f)) {

		অক্ষর *p, *name, *তথ্য = ফাঁকা;

		যদি (*buf == '#' || *buf == '\n')
			চালু;	/* only comment or empty line */

		p = strchr(buf, '#');
		যদি (p)
			*p = '\0';
		অন্যথা {
			size_t n = strlen(buf);
			যদি (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		যদি (!*buf)
			চালু;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		যখন (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		তথ্য = name;
		যখন (*তথ্য && !(isspace((unsigned)*তথ্য) || *তথ্য == '='))
			তথ্য++;
		যদি (তথ্য > name && *তথ্য)
			*তথ্য++ = '\0';

		যদি (!*name || তথ্য == name)
			চালু;

		/* go to the begin of the value */
		যখন (*তথ্য
		       && (isspace((unsigned)*তথ্য) || *তথ্য == '='
			   || *তথ্য == '"'))
			তথ্য++;

		/* remove space at the end of the value */
		p = তথ্য + strlen(তথ্য);
		যদি (p > তথ্য)
			p--;
		যখন (p > তথ্য && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, তথ্য, filename);
	}

	fclose(f);
}

স্থির শূন্য load_defaults(শূন্য)
{
	যদি (logindefs_loader)
		logindefs_loader(logindefs_loader_তথ্য);
	অন্যথা
		logindefs_load_file(_PATH_LOGINDEFS);
}

স্থির কাঠামো item *অনুসন্ধান(ধ্রুবক অক্ষর *name)
{
	কাঠামো item *ptr;

	যদি (!list)
		load_defaults();

	ptr = list;
	যখন (ptr != ফাঁকা) {
		যদি (c_strcasecmp(name, ptr->name) == 0)
			ফেরত ptr;
		ptr = ptr->পরবর্তী;
	}

	ফেরত ফাঁকা;
}

স্থির ধ্রুবক অক্ষর *অনুসন্ধান_config(ধ্রুবক অক্ষর *name)
{
	কাঠামো item *ptr;

	ptr = list;
	যখন (ptr != ফাঁকা) {
		যদি (c_strcasecmp(name, ptr->name) == 0)
			ফেরত ptr->path;
		ptr = ptr->পরবর্তী;
	}

	ফেরত ফাঁকা;
}

পূর্ণসংখ্যা getlogindefs_bool(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা dflt)
{
	কাঠামো item *ptr = অনুসন্ধান(name);
	ফেরত ptr && ptr->মান ? (c_strcasecmp(ptr->মান, "yes") == 0) : dflt;
}

unsigned long getlogindefs_অঙ্ক(ধ্রুবক অক্ষর *name, unsigned long dflt)
{
	কাঠামো item *ptr = অনুসন্ধান(name);
	অক্ষর *end = ফাঁকা;
	unsigned long retval;

	যদি (!ptr || !ptr->মান)
		ফেরত dflt;

	errno = 0;
	retval = strtoul(ptr->মান, &end, 0);
	যদি (end && *end == '\0' && !errno)
		ফেরত retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       অনুসন্ধান_config(name), name, ptr->মান);
	ফেরত dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ধ্রুবক অক্ষর *getlogindefs_str(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *dflt)
{
	কাঠামো item *ptr = অনুসন্ধান(name);

	যদি (!ptr)
		ফেরত dflt;
	যদি (!ptr->মান)
		ফেরত "";
	ফেরত ptr->মান;
}

#অন্যথা /* !HAVE_LIBECONF */

#অন্তর্ভাব "dl-econf.h"

স্থির econf_file *file = ফাঁকা;

শূন্য free_getlogindefs_তথ্য(শূন্য)
{
	যদি (file)
		econf_call(econf_freeFile)(file);
	file = ফাঁকা;
}

স্থির শূন্য load_defaults(শূন্য)
{
	econf_err error;

	যদি (file != ফাঁকা)
		free_getlogindefs_তথ্য();

	যদি (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ফাঁকা,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#অন্যথা
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		যদি (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	যদি (logindefs_loader)
		logindefs_loader(logindefs_loader_তথ্য);

}

শূন্য logindefs_load_file(ধ্রুবক অক্ষর *filename)
{
	econf_file *file_l = ফাঁকা, *file_m = ফাঁকা;
	অক্ষর *path;

	logindefs_loader = ফাঁকা; /* No recursion */

	যদি (ul_dlopen_libeconf() != 0)
		ফেরত;

#যদি USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	যদি (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		যদি (file == ফাঁকা)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	মুক্ত (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	যদি (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		যদি (file == ফাঁকা)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} অন্যথা যদি (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		যদি (file == ফাঁকা)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	মুক্ত (path);
}

পূর্ণসংখ্যা getlogindefs_bool(ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা dflt)
{
	bool মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		ফেরত dflt;

	যদি ((error = econf_call(econf_get_Bool_মান)(file, ফাঁকা, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ফেরত dflt;
	}
	ফেরত মান;
}

unsigned long getlogindefs_অঙ্ক(ধ্রুবক অক্ষর *name, unsigned long dflt)
{
	uint64_t মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		ফেরত dflt;

	যদি ((error = econf_call(econf_get_UInt64_মান)(file, ফাঁকা, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ফেরত dflt;
	}
	ফেরত মান;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ধ্রুবক অক্ষর *getlogindefs_str(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *dflt)
{
	অক্ষর *মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		ফেরত dflt;

	যদি ((error = econf_call(econf_get_String_মান)(file, ফাঁকা, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ফেরত dflt;
	}
	যদি (মান)
		ফেরত মান;

	ফেরত xstrdup("");
}
#endif /* !HAVE_LIBECONF */

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
পূর্ণসংখ্যা logindefs_setenv(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *conf, ধ্রুবক অক্ষর *dflt)
{
	ধ্রুবক অক্ষর *val = getlogindefs_str(conf, dflt);
	ধ্রুবক অক্ষর *p;

	যদি (!val)
		ফেরত -1;

	p = strchr(val, '=');
	যদি (p) {
		size_t sz = strlen(name);

		যদি (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			যদি (*val == '"')
				val++;
			যদি (!*val)
				val = dflt;
		}
	}

	ফেরত val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
পূর্ণসংখ্যা logindefs_setenv_path(uid_t uid)
{
	পূর্ণসংখ্যা rc = 0;

	যদি (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	অন্যথা যদি ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ফাঁকা)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_মূল);

	ফেরত rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
পূর্ণসংখ্যা effective_access(ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা mode)
{
	পূর্ণসংখ্যা fd = open(path, mode);
	যদি (fd != -1)
		close(fd);
	ফেরত fd == -1 ? -1 : 0;
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

পূর্ণসংখ্যা get_hushlogin_status(কাঠামো passwd *pwd, ধ্রুবক অক্ষর *override_home, পূর্ণসংখ্যা force_check)
{
	ধ্রুবক অক্ষর *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, ফাঁকা };
	ধ্রুবক অক্ষর *file;
	অক্ষর buf[BUFSIZ];
	পূর্ণসংখ্যা i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ফাঁকা);
	যদি (file) {
		যদি (!*file)
			ফেরত 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ফাঁকা;
	}

	জন্য (i = 0; files[i]; i++) {
		পূর্ণসংখ্যা ok = 0;

		file = files[i];

		/* global hush-file */
		যদি (*file == '/') {
			কাঠামো stat st;
			FILE *f;

			যদি (stat(file, &st) != 0)
				চালু;	/* file does not exist */

			যদি (st.st_size == 0)
				ফেরত 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			যদি (!f)
				চালু;	/* ignore errors... */

			যখন (ok == 0 && fgets(buf, আকার(buf), f)) {
				যদি (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			যদি (ok)
				ফেরত 1;	/* found username/shell */

			ফেরত 0;		/* ignore per-account files */
		}

		/* per-account setting */
		যদি (strlen(pwd->pw_dir) + strlen(file) + 2 > আকার(buf))
			চালু;

		যদি (snprintf(buf, আকার(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			চালু;

		যদি (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			যদি (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			যদি (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			যদি (ok)
				ফেরত 1;	/* enabled by user */
		}
		অন্যথা {
			পূর্ণসংখ্যা rc;
			rc = effective_access(buf, O_RDONLY);
			যদি (rc == 0)
				ফেরত 1;

			যদি (rc == -1 && errno == EACCES)
				ফেরত -1;
		}

	}

	ফেরত 0;
}
#ifdef TEST_PROGRAM
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	অক্ষর *name, *type;
	close_stdout_atexit();

	যদি (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	যদি (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		পূর্ণসংখ্যা i;
		অক্ষর *চাবিগুলি[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ফাঁকা};

		জন্য (i = 0; file && চাবিগুলি[i] != ফাঁকা; i++) {
			অক্ষর *মান = ফাঁকা;

			econf_call(econf_get_String_মান)(file, ফাঁকা, চাবিগুলি[i], &মান);
			printf ("%s: $%s: '%s'\n", argv[1], চাবিগুলি[i], মান);
		}

		যদি (file)
			econf_call(econf_freeFile)(file);

#অন্যথা
		কাঠামো item *ptr;

		জন্য (ptr = list; ptr; ptr = ptr->পরবর্তী)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->মান);
#endif
		ফেরত EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	যদি (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	অন্যথা যদি (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_অঙ্ক(name, 0));
	অন্যথা যদি (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ফেরত EXIT_SUCCESS;
}
#endif
