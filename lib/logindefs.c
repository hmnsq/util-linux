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
#شامل <assert.h>
#شامل <ctype.h>
#شامل <errno.h>
#شامل <limits.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/syslog.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <pwd.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "closestream.h"
#شامل "logindefs.h"
#شامل "nls.h"
#شامل "pathnames.h"
#شامل "xalloc.h"


ایستا تهی (*logindefs_loader)(تهی *) = پوچ;
ایستا تهی *logindefs_loader_data = پوچ;

تهی logindefs_set_loader(تهی (*loader)(تهی *data), تهی *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

ساختار item {
	نویسه *name;		/* name of the option.  */
	نویسه *مقدار;		/* value of the option.  */
	نویسه *path;		/* name of config file for this option.  */

	ساختار item *next;	/* pointer to next option.  */
};

ایستا ساختار item *list = پوچ;

تهی free_getlogindefs_data(تهی)
{
	ساختار item *ptr;

	ptr = list;
	وقتی (ptr) {
		ساختار item *tmp = ptr->next;

		آزاد(ptr->path);
		آزاد(ptr->name);
		آزاد(ptr->مقدار);
		آزاد(ptr);
		ptr = tmp;
	}

	list = پوچ;
}

ایستا تهی store(ثابت نویسه *name, ثابت نویسه *مقدار, ثابت نویسه *path)
{
	ساختار item *new = xmalloc(اندازه(ساختار item));

	اگر (!name)
		abort();

	new->name = xstrdup(name);
	new->مقدار = مقدار && *مقدار ? xstrdup(مقدار) : پوچ;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

تهی logindefs_load_file(ثابت نویسه *filename)
{
	FILE *f;
	نویسه buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	اگر (!f)
		بازگشت;

	وقتی (fgets(buf, اندازه(buf), f)) {

		نویسه *p, *name, *data = پوچ;

		اگر (*buf == '#' || *buf == '\n')
			ادامه;	/* only comment or empty line */

		p = strchr(buf, '#');
		اگر (p)
			*p = '\0';
		وگرنه {
			size_t n = strlen(buf);
			اگر (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		اگر (!*buf)
			ادامه;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		وقتی (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		وقتی (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		اگر (data > name && *data)
			*data++ = '\0';

		اگر (!*name || data == name)
			ادامه;

		/* go to the begin of the value */
		وقتی (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		اگر (p > data)
			p--;
		وقتی (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

ایستا تهی load_defaults(تهی)
{
	اگر (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	وگرنه
		logindefs_load_file(_PATH_LOGINDEFS);
}

ایستا ساختار item *جستجو(ثابت نویسه *name)
{
	ساختار item *ptr;

	اگر (!list)
		load_defaults();

	ptr = list;
	وقتی (ptr != پوچ) {
		اگر (c_strcasecmp(name, ptr->name) == 0)
			بازگشت ptr;
		ptr = ptr->next;
	}

	بازگشت پوچ;
}

ایستا ثابت نویسه *جستجو_config(ثابت نویسه *name)
{
	ساختار item *ptr;

	ptr = list;
	وقتی (ptr != پوچ) {
		اگر (c_strcasecmp(name, ptr->name) == 0)
			بازگشت ptr->path;
		ptr = ptr->next;
	}

	بازگشت پوچ;
}

صحیح getlogindefs_bool(ثابت نویسه *name, صحیح dflt)
{
	ساختار item *ptr = جستجو(name);
	بازگشت ptr && ptr->مقدار ? (c_strcasecmp(ptr->مقدار, "yes") == 0) : dflt;
}

unsigned long getlogindefs_رقم(ثابت نویسه *name, unsigned long dflt)
{
	ساختار item *ptr = جستجو(name);
	نویسه *end = پوچ;
	unsigned long retval;

	اگر (!ptr || !ptr->مقدار)
		بازگشت dflt;

	errno = 0;
	retval = strtoul(ptr->مقدار, &end, 0);
	اگر (end && *end == '\0' && !errno)
		بازگشت retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       جستجو_config(name), name, ptr->مقدار);
	بازگشت dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت نویسه *getlogindefs_str(ثابت نویسه *name, ثابت نویسه *dflt)
{
	ساختار item *ptr = جستجو(name);

	اگر (!ptr)
		بازگشت dflt;
	اگر (!ptr->مقدار)
		بازگشت "";
	بازگشت ptr->مقدار;
}

#وگرنه /* !HAVE_LIBECONF */

#شامل "dl-econf.h"

ایستا econf_file *file = پوچ;

تهی free_getlogindefs_data(تهی)
{
	اگر (file)
		econf_call(econf_freeFile)(file);
	file = پوچ;
}

ایستا تهی load_defaults(تهی)
{
	econf_err error;

	اگر (file != پوچ)
		free_getlogindefs_data();

	اگر (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, پوچ,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#وگرنه
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		اگر (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	اگر (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

تهی logindefs_load_file(ثابت نویسه *filename)
{
	econf_file *file_l = پوچ, *file_m = پوچ;
	نویسه *path;

	logindefs_loader = پوچ; /* No recursion */

	اگر (ul_dlopen_libeconf() != 0)
		بازگشت;

#اگر USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	اگر (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		اگر (file == پوچ)
			file = file_l;
		وگرنه اگر (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	آزاد (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	اگر (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		اگر (file == پوچ)
			file = file_l;
		وگرنه اگر (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} وگرنه اگر (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		اگر (file == پوچ)
			file = file_l;
		وگرنه اگر (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	آزاد (path);
}

صحیح getlogindefs_bool(ثابت نویسه *name, صحیح dflt)
{
	bool مقدار;
	econf_err error;

	اگر (!file)
		load_defaults();

	اگر (!file)
		بازگشت dflt;

	اگر ((error = econf_call(econf_get_Bool_مقدار)(file, پوچ, name, &مقدار))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		بازگشت dflt;
	}
	بازگشت مقدار;
}

unsigned long getlogindefs_رقم(ثابت نویسه *name, unsigned long dflt)
{
	uint64_t مقدار;
	econf_err error;

	اگر (!file)
		load_defaults();

	اگر (!file)
		بازگشت dflt;

	اگر ((error = econf_call(econf_get_UInt64_مقدار)(file, پوچ, name, &مقدار))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		بازگشت dflt;
	}
	بازگشت مقدار;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت نویسه *getlogindefs_str(ثابت نویسه *name, ثابت نویسه *dflt)
{
	نویسه *مقدار;
	econf_err error;

	اگر (!file)
		load_defaults();

	اگر (!file)
		بازگشت dflt;

	اگر ((error = econf_call(econf_get_String_مقدار)(file, پوچ, name, &مقدار))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		بازگشت dflt;
	}
	اگر (مقدار)
		بازگشت مقدار;

	بازگشت xstrdup("");
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
صحیح logindefs_setenv(ثابت نویسه *name, ثابت نویسه *conf, ثابت نویسه *dflt)
{
	ثابت نویسه *val = getlogindefs_str(conf, dflt);
	ثابت نویسه *p;

	اگر (!val)
		بازگشت -1;

	p = strchr(val, '=');
	اگر (p) {
		size_t sz = strlen(name);

		اگر (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			اگر (*val == '"')
				val++;
			اگر (!*val)
				val = dflt;
		}
	}

	بازگشت val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
صحیح logindefs_setenv_path(uid_t uid)
{
	صحیح rc = 0;

	اگر (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	وگرنه اگر ((rc = logindefs_setenv("PATH", "ENV_SUPATH", پوچ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_ریشه);

	بازگشت rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحیح effective_access(ثابت نویسه *path, صحیح mode)
{
	صحیح fd = open(path, mode);
	اگر (fd != -1)
		close(fd);
	بازگشت fd == -1 ? -1 : 0;
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

صحیح get_hushlogin_status(ساختار passwd *pwd, ثابت نویسه *override_home, صحیح force_check)
{
	ثابت نویسه *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, پوچ };
	ثابت نویسه *file;
	نویسه buf[BUFSIZ];
	صحیح i;

	file = getlogindefs_str("HUSHLOGIN_FILE", پوچ);
	اگر (file) {
		اگر (!*file)
			بازگشت 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = پوچ;
	}

	برای (i = 0; files[i]; i++) {
		صحیح ok = 0;

		file = files[i];

		/* global hush-file */
		اگر (*file == '/') {
			ساختار stat st;
			FILE *f;

			اگر (stat(file, &st) != 0)
				ادامه;	/* file does not exist */

			اگر (st.st_size == 0)
				بازگشت 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			اگر (!f)
				ادامه;	/* ignore errors... */

			وقتی (ok == 0 && fgets(buf, اندازه(buf), f)) {
				اگر (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			اگر (ok)
				بازگشت 1;	/* found username/shell */

			بازگشت 0;		/* ignore per-account files */
		}

		/* per-account setting */
		اگر (strlen(pwd->pw_dir) + strlen(file) + 2 > اندازه(buf))
			ادامه;

		اگر (snprintf(buf, اندازه(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ادامه;

		اگر (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			اگر (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			اگر (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			اگر (ok)
				بازگشت 1;	/* enabled by user */
		}
		وگرنه {
			صحیح rc;
			rc = effective_access(buf, O_RDONLY);
			اگر (rc == 0)
				بازگشت 1;

			اگر (rc == -1 && errno == EACCES)
				بازگشت -1;
		}

	}

	بازگشت 0;
}
#ifdef TEST_PROGRAM
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	نویسه *name, *type;
	close_stdout_atexit();

	اگر (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	اگر (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		صحیح i;
		نویسه *کلیدها[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				پوچ};

		برای (i = 0; file && کلیدها[i] != پوچ; i++) {
			نویسه *مقدار = پوچ;

			econf_call(econf_get_String_مقدار)(file, پوچ, کلیدها[i], &مقدار);
			printf ("%s: $%s: '%s'\n", argv[1], کلیدها[i], مقدار);
		}

		اگر (file)
			econf_call(econf_freeFile)(file);

#وگرنه
		ساختار item *ptr;

		برای (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->مقدار);
#endif
		بازگشت EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	اگر (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	وگرنه اگر (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_رقم(name, 0));
	وگرنه اگر (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	بازگشت EXIT_SUCCESS;
}
#endif
