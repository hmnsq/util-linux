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
#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/syslog.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <pwd.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "closestream.h"
#अन्तर्भाव "logindefs.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "xalloc.h"


स्थिर शून्य (*logindefs_loader)(शून्य *) = रिक्त;
स्थिर शून्य *logindefs_loader_data = रिक्त;

शून्य logindefs_set_loader(शून्य (*loader)(शून्य *data), शून्य *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

संरचना item {
	वर्ण *name;		/* name of the option.  */
	वर्ण *मानम्;		/* value of the option.  */
	वर्ण *path;		/* name of config file for this option.  */

	संरचना item *next;	/* pointer to next option.  */
};

स्थिर संरचना item *list = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	संरचना item *ptr;

	ptr = list;
	याव (ptr) {
		संरचना item *tmp = ptr->next;

		मुक्त(ptr->path);
		मुक्त(ptr->name);
		मुक्त(ptr->मानम्);
		मुक्त(ptr);
		ptr = tmp;
	}

	list = रिक्त;
}

स्थिर शून्य store(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्, ध्रुव वर्ण *path)
{
	संरचना item *new = xmalloc(परिमाण(संरचना item));

	सचे (!name)
		abort();

	new->name = xstrdup(name);
	new->मानम् = मानम् && *मानम् ? xstrdup(मानम्) : रिक्त;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

शून्य logindefs_load_file(ध्रुव वर्ण *filename)
{
	FILE *f;
	वर्ण buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	सचे (!f)
		पटिनिवत्त;

	याव (fgets(buf, परिमाण(buf), f)) {

		वर्ण *p, *name, *data = रिक्त;

		सचे (*buf == '#' || *buf == '\n')
			अनुवर्त;	/* only comment or empty line */

		p = strchr(buf, '#');
		सचे (p)
			*p = '\0';
		अञ्ञथा {
			size_t n = strlen(buf);
			सचे (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		सचे (!*buf)
			अनुवर्त;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		याव (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		याव (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		सचे (data > name && *data)
			*data++ = '\0';

		सचे (!*name || data == name)
			अनुवर्त;

		/* go to the begin of the value */
		याव (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		सचे (p > data)
			p--;
		याव (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

स्थिर शून्य load_defaults(शून्य)
{
	सचे (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	अञ्ञथा
		logindefs_load_file(_PATH_LOGINDEFS);
}

स्थिर संरचना item *अन्वेषणम्(ध्रुव वर्ण *name)
{
	संरचना item *ptr;

	सचे (!list)
		load_defaults();

	ptr = list;
	याव (ptr != रिक्त) {
		सचे (c_strcasecmp(name, ptr->name) == 0)
			पटिनिवत्त ptr;
		ptr = ptr->next;
	}

	पटिनिवत्त रिक्त;
}

स्थिर ध्रुव वर्ण *अन्वेषणम्_config(ध्रुव वर्ण *name)
{
	संरचना item *ptr;

	ptr = list;
	याव (ptr != रिक्त) {
		सचे (c_strcasecmp(name, ptr->name) == 0)
			पटिनिवत्त ptr->path;
		ptr = ptr->next;
	}

	पटिनिवत्त रिक्त;
}

पूर्णाङ्क getlogindefs_bool(ध्रुव वर्ण *name, पूर्णाङ्क dflt)
{
	संरचना item *ptr = अन्वेषणम्(name);
	पटिनिवत्त ptr && ptr->मानम् ? (c_strcasecmp(ptr->मानम्, "yes") == 0) : dflt;
}

unsigned long getlogindefs_अङ्कः(ध्रुव वर्ण *name, unsigned long dflt)
{
	संरचना item *ptr = अन्वेषणम्(name);
	वर्ण *end = रिक्त;
	unsigned long retval;

	सचे (!ptr || !ptr->मानम्)
		पटिनिवत्त dflt;

	errno = 0;
	retval = strtoul(ptr->मानम्, &end, 0);
	सचे (end && *end == '\0' && !errno)
		पटिनिवत्त retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       अन्वेषणम्_config(name), name, ptr->मानम्);
	पटिनिवत्त dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ध्रुव वर्ण *getlogindefs_str(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	संरचना item *ptr = अन्वेषणम्(name);

	सचे (!ptr)
		पटिनिवत्त dflt;
	सचे (!ptr->मानम्)
		पटिनिवत्त "";
	पटिनिवत्त ptr->मानम्;
}

#अञ्ञथा /* !HAVE_LIBECONF */

#अन्तर्भाव "dl-econf.h"

स्थिर econf_file *file = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	सचे (file)
		econf_call(econf_freeFile)(file);
	file = रिक्त;
}

स्थिर शून्य load_defaults(शून्य)
{
	econf_err error;

	सचे (file != रिक्त)
		free_getlogindefs_data();

	सचे (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, रिक्त,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#अञ्ञथा
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		सचे (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	सचे (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

शून्य logindefs_load_file(ध्रुव वर्ण *filename)
{
	econf_file *file_l = रिक्त, *file_m = रिक्त;
	वर्ण *path;

	logindefs_loader = रिक्त; /* No recursion */

	सचे (ul_dlopen_libeconf() != 0)
		पटिनिवत्त;

#सचे USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	सचे (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		सचे (file == रिक्त)
			file = file_l;
		अञ्ञथा सचे (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	सचे (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		सचे (file == रिक्त)
			file = file_l;
		अञ्ञथा सचे (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} अञ्ञथा सचे (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		सचे (file == रिक्त)
			file = file_l;
		अञ्ञथा सचे (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
}

पूर्णाङ्क getlogindefs_bool(ध्रुव वर्ण *name, पूर्णाङ्क dflt)
{
	bool मानम्;
	econf_err error;

	सचे (!file)
		load_defaults();

	सचे (!file)
		पटिनिवत्त dflt;

	सचे ((error = econf_call(econf_get_Bool_मानम्)(file, रिक्त, name, &मानम्))) {
		सचे (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		पटिनिवत्त dflt;
	}
	पटिनिवत्त मानम्;
}

unsigned long getlogindefs_अङ्कः(ध्रुव वर्ण *name, unsigned long dflt)
{
	uint64_t मानम्;
	econf_err error;

	सचे (!file)
		load_defaults();

	सचे (!file)
		पटिनिवत्त dflt;

	सचे ((error = econf_call(econf_get_UInt64_मानम्)(file, रिक्त, name, &मानम्))) {
		सचे (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		पटिनिवत्त dflt;
	}
	पटिनिवत्त मानम्;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ध्रुव वर्ण *getlogindefs_str(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	वर्ण *मानम्;
	econf_err error;

	सचे (!file)
		load_defaults();

	सचे (!file)
		पटिनिवत्त dflt;

	सचे ((error = econf_call(econf_get_String_मानम्)(file, रिक्त, name, &मानम्))) {
		सचे (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		पटिनिवत्त dflt;
	}
	सचे (मानम्)
		पटिनिवत्त मानम्;

	पटिनिवत्त xstrdup("");
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
पूर्णाङ्क logindefs_setenv(ध्रुव वर्ण *name, ध्रुव वर्ण *conf, ध्रुव वर्ण *dflt)
{
	ध्रुव वर्ण *val = getlogindefs_str(conf, dflt);
	ध्रुव वर्ण *p;

	सचे (!val)
		पटिनिवत्त -1;

	p = strchr(val, '=');
	सचे (p) {
		size_t sz = strlen(name);

		सचे (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			सचे (*val == '"')
				val++;
			सचे (!*val)
				val = dflt;
		}
	}

	पटिनिवत्त val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
पूर्णाङ्क logindefs_setenv_path(uid_t uid)
{
	पूर्णाङ्क rc = 0;

	सचे (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	अञ्ञथा सचे ((rc = logindefs_setenv("PATH", "ENV_SUPATH", रिक्त)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_मूलम्);

	पटिनिवत्त rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
पूर्णाङ्क effective_access(ध्रुव वर्ण *path, पूर्णाङ्क mode)
{
	पूर्णाङ्क fd = open(path, mode);
	सचे (fd != -1)
		close(fd);
	पटिनिवत्त fd == -1 ? -1 : 0;
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

पूर्णाङ्क get_hushlogin_status(संरचना passwd *pwd, ध्रुव वर्ण *override_home, पूर्णाङ्क force_check)
{
	ध्रुव वर्ण *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, रिक्त };
	ध्रुव वर्ण *file;
	वर्ण buf[BUFSIZ];
	पूर्णाङ्क i;

	file = getlogindefs_str("HUSHLOGIN_FILE", रिक्त);
	सचे (file) {
		सचे (!*file)
			पटिनिवत्त 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = रिक्त;
	}

	अत्थाय (i = 0; files[i]; i++) {
		पूर्णाङ्क ok = 0;

		file = files[i];

		/* global hush-file */
		सचे (*file == '/') {
			संरचना stat st;
			FILE *f;

			सचे (stat(file, &st) != 0)
				अनुवर्त;	/* file does not exist */

			सचे (st.st_size == 0)
				पटिनिवत्त 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			सचे (!f)
				अनुवर्त;	/* ignore errors... */

			याव (ok == 0 && fgets(buf, परिमाण(buf), f)) {
				सचे (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			सचे (ok)
				पटिनिवत्त 1;	/* found username/shell */

			पटिनिवत्त 0;		/* ignore per-account files */
		}

		/* per-account setting */
		सचे (strlen(pwd->pw_dir) + strlen(file) + 2 > परिमाण(buf))
			अनुवर्त;

		सचे (snprintf(buf, परिमाण(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			अनुवर्त;

		सचे (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			सचे (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			सचे (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			सचे (ok)
				पटिनिवत्त 1;	/* enabled by user */
		}
		अञ्ञथा {
			पूर्णाङ्क rc;
			rc = effective_access(buf, O_RDONLY);
			सचे (rc == 0)
				पटिनिवत्त 1;

			सचे (rc == -1 && errno == EACCES)
				पटिनिवत्त -1;
		}

	}

	पटिनिवत्त 0;
}
#ifdef TEST_PROGRAM
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *name, *type;
	close_stdout_atexit();

	सचे (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	सचे (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		पूर्णाङ्क i;
		वर्ण *कुञ्चिकाः[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				रिक्त};

		अत्थाय (i = 0; file && कुञ्चिकाः[i] != रिक्त; i++) {
			वर्ण *मानम् = रिक्त;

			econf_call(econf_get_String_मानम्)(file, रिक्त, कुञ्चिकाः[i], &मानम्);
			printf ("%s: $%s: '%s'\n", argv[1], कुञ्चिकाः[i], मानम्);
		}

		सचे (file)
			econf_call(econf_freeFile)(file);

#अञ्ञथा
		संरचना item *ptr;

		अत्थाय (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->मानम्);
#endif
		पटिनिवत्त EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	सचे (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	अञ्ञथा सचे (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_अङ्कः(name, 0));
	अञ्ञथा सचे (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	पटिनिवत्त EXIT_SUCCESS;
}
#endif
