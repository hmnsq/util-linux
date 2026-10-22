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
#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <limits.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/syslog.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <pwd.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "closestream.h"
#ضمن "logindefs.h"
#ضمن "nls.h"
#ضمن "pathnames.h"
#ضمن "xalloc.h"


ساكن فراغ (*logindefs_loader)(فراغ *) = عدم;
ساكن فراغ *logindefs_loader_data = عدم;

فراغ logindefs_set_loader(فراغ (*loader)(فراغ *data), فراغ *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

بنية item {
	حرف *name;		/* name of the option.  */
	حرف *value;		/* value of the option.  */
	حرف *path;		/* name of config file for this option.  */

	بنية item *next;	/* pointer to next option.  */
};

ساكن بنية item *list = عدم;

فراغ free_getlogindefs_data(فراغ)
{
	بنية item *ptr;

	ptr = list;
	כלעוד (ptr) {
		بنية item *tmp = ptr->next;

		حرر(ptr->path);
		حرر(ptr->name);
		حرر(ptr->value);
		حرر(ptr);
		ptr = tmp;
	}

	list = عدم;
}

ساكن فراغ store(ثابت حرف *name, ثابت حرف *value, ثابت حرف *path)
{
	بنية item *new = xmalloc(حجم(بنية item));

	אם (!name)
		abort();

	new->name = xstrdup(name);
	new->value = value && *value ? xstrdup(value) : عدم;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

فراغ logindefs_load_file(ثابت حرف *filename)
{
	FILE *f;
	حرف buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	אם (!f)
		החזר;

	כלעוד (fgets(buf, حجم(buf), f)) {

		حرف *p, *name, *data = عدم;

		אם (*buf == '#' || *buf == '\n')
			استمر;	/* only comment or empty line */

		p = strchr(buf, '#');
		אם (p)
			*p = '\0';
		אחרת {
			size_t n = strlen(buf);
			אם (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		אם (!*buf)
			استمر;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		כלעוד (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		כלעוד (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		אם (data > name && *data)
			*data++ = '\0';

		אם (!*name || data == name)
			استمر;

		/* go to the begin of the value */
		כלעוד (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		אם (p > data)
			p--;
		כלעוד (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

ساكن فراغ load_defaults(فراغ)
{
	אם (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	אחרת
		logindefs_load_file(_PATH_LOGINDEFS);
}

ساكن بنية item *search(ثابت حرف *name)
{
	بنية item *ptr;

	אם (!list)
		load_defaults();

	ptr = list;
	כלעוד (ptr != عدم) {
		אם (c_strcasecmp(name, ptr->name) == 0)
			החזר ptr;
		ptr = ptr->next;
	}

	החזר عدم;
}

ساكن ثابت حرف *search_config(ثابت حرف *name)
{
	بنية item *ptr;

	ptr = list;
	כלעוד (ptr != عدم) {
		אם (c_strcasecmp(name, ptr->name) == 0)
			החזר ptr->path;
		ptr = ptr->next;
	}

	החזר عدم;
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	بنية item *ptr = search(name);
	החזר ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(ثابت حرف *name, unsigned long dflt)
{
	بنية item *ptr = search(name);
	حرف *end = عدم;
	unsigned long retval;

	אם (!ptr || !ptr->value)
		החזר dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	אם (end && *end == '\0' && !errno)
		החזר retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	החזר dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	بنية item *ptr = search(name);

	אם (!ptr)
		החזר dflt;
	אם (!ptr->value)
		החזר "";
	החזר ptr->value;
}

#אחרת /* !HAVE_LIBECONF */

#ضمن "dl-econf.h"

ساكن econf_file *file = عدم;

فراغ free_getlogindefs_data(فراغ)
{
	אם (file)
		econf_call(econf_freeFile)(file);
	file = عدم;
}

ساكن فراغ load_defaults(فراغ)
{
	econf_err error;

	אם (file != عدم)
		free_getlogindefs_data();

	אם (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, عدم,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#אחרת
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		אם (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	אם (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

فراغ logindefs_load_file(ثابت حرف *filename)
{
	econf_file *file_l = عدم, *file_m = عدم;
	حرف *path;

	logindefs_loader = عدم; /* No recursion */

	אם (ul_dlopen_libeconf() != 0)
		החזר;

#אם USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	אם (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		אם (file == عدم)
			file = file_l;
		אחרת אם (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	אם (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		אם (file == عدم)
			file = file_l;
		אחרת אם (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} אחרת אם (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		אם (file == عدم)
			file = file_l;
		אחרת אם (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (path);
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	bool value;
	econf_err error;

	אם (!file)
		load_defaults();

	אם (!file)
		החזר dflt;

	אם ((error = econf_call(econf_getBoolValue)(file, عدم, name, &value))) {
		אם (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		החזר dflt;
	}
	החזר value;
}

unsigned long getlogindefs_num(ثابت حرف *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	אם (!file)
		load_defaults();

	אם (!file)
		החזר dflt;

	אם ((error = econf_call(econf_getUInt64Value)(file, عدم, name, &value))) {
		אם (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		החזר dflt;
	}
	החזר value;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	حرف *value;
	econf_err error;

	אם (!file)
		load_defaults();

	אם (!file)
		החזר dflt;

	אם ((error = econf_call(econf_getStringValue)(file, عدم, name, &value))) {
		אם (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		החזר dflt;
	}
	אם (value)
		החזר value;

	החזר xstrdup("");
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
صحيح logindefs_setenv(ثابت حرف *name, ثابت حرف *conf, ثابت حرف *dflt)
{
	ثابت حرف *val = getlogindefs_str(conf, dflt);
	ثابت حرف *p;

	אם (!val)
		החזר -1;

	p = strchr(val, '=');
	אם (p) {
		size_t sz = strlen(name);

		אם (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			אם (*val == '"')
				val++;
			אם (!*val)
				val = dflt;
		}
	}

	החזר val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
صحيح logindefs_setenv_path(uid_t uid)
{
	صحيح rc = 0;

	אם (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	אחרת אם ((rc = logindefs_setenv("PATH", "ENV_SUPATH", عدم)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	החזר rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحيح effective_access(ثابت حرف *path, صحيح mode)
{
	صحيح fd = open(path, mode);
	אם (fd != -1)
		close(fd);
	החזר fd == -1 ? -1 : 0;
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

صحيح get_hushlogin_status(بنية passwd *pwd, ثابت حرف *override_home, صحيح force_check)
{
	ثابت حرف *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, عدم };
	ثابت حرف *file;
	حرف buf[BUFSIZ];
	صحيح i;

	file = getlogindefs_str("HUSHLOGIN_FILE", عدم);
	אם (file) {
		אם (!*file)
			החזר 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = عدم;
	}

	עבור (i = 0; files[i]; i++) {
		صحيح ok = 0;

		file = files[i];

		/* global hush-file */
		אם (*file == '/') {
			بنية stat st;
			FILE *f;

			אם (stat(file, &st) != 0)
				استمر;	/* file does not exist */

			אם (st.st_size == 0)
				החזר 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			אם (!f)
				استمر;	/* ignore errors... */

			כלעוד (ok == 0 && fgets(buf, حجم(buf), f)) {
				אם (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			אם (ok)
				החזר 1;	/* found username/shell */

			החזר 0;		/* ignore per-account files */
		}

		/* per-account setting */
		אם (strlen(pwd->pw_dir) + strlen(file) + 2 > حجم(buf))
			استمر;

		אם (snprintf(buf, حجم(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			استمر;

		אם (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			אם (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			אם (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			אם (ok)
				החזר 1;	/* enabled by user */
		}
		אחרת {
			صحيح rc;
			rc = effective_access(buf, O_RDONLY);
			אם (rc == 0)
				החזר 1;

			אם (rc == -1 && errno == EACCES)
				החזר -1;
		}

	}

	החזר 0;
}
#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	אם (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	אם (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		صحيح i;
		حرف *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				عدم};

		עבור (i = 0; file && keys[i] != عدم; i++) {
			حرف *value = عدم;

			econf_call(econf_getStringValue)(file, عدم, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		אם (file)
			econf_call(econf_freeFile)(file);

#אחרת
		بنية item *ptr;

		עבור (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		החזר EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	אם (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	אחרת אם (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	אחרת אם (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	החזר EXIT_SUCCESS;
}
#endif
