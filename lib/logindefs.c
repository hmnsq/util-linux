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
	طالما (ptr) {
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

	𐤀𐤌 (!name)
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
	𐤀𐤌 (!f)
		أرجع;

	طالما (fgets(buf, حجم(buf), f)) {

		حرف *p, *name, *data = عدم;

		𐤀𐤌 (*buf == '#' || *buf == '\n')
			استمر;	/* only comment or empty line */

		p = strchr(buf, '#');
		𐤀𐤌 (p)
			*p = '\0';
		وإلا {
			size_t n = strlen(buf);
			𐤀𐤌 (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		𐤀𐤌 (!*buf)
			استمر;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		طالما (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		طالما (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		𐤀𐤌 (data > name && *data)
			*data++ = '\0';

		𐤀𐤌 (!*name || data == name)
			استمر;

		/* go to the begin of the value */
		طالما (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		𐤀𐤌 (p > data)
			p--;
		طالما (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

ساكن فراغ load_defaults(فراغ)
{
	𐤀𐤌 (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	وإلا
		logindefs_load_file(_PATH_LOGINDEFS);
}

ساكن بنية item *search(ثابت حرف *name)
{
	بنية item *ptr;

	𐤀𐤌 (!list)
		load_defaults();

	ptr = list;
	طالما (ptr != عدم) {
		𐤀𐤌 (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr;
		ptr = ptr->next;
	}

	أرجع عدم;
}

ساكن ثابت حرف *search_config(ثابت حرف *name)
{
	بنية item *ptr;

	ptr = list;
	طالما (ptr != عدم) {
		𐤀𐤌 (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr->path;
		ptr = ptr->next;
	}

	أرجع عدم;
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	بنية item *ptr = search(name);
	أرجع ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(ثابت حرف *name, unsigned long dflt)
{
	بنية item *ptr = search(name);
	حرف *end = عدم;
	unsigned long retval;

	𐤀𐤌 (!ptr || !ptr->value)
		أرجع dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	𐤀𐤌 (end && *end == '\0' && !errno)
		أرجع retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	أرجع dflt;
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

	𐤀𐤌 (!ptr)
		أرجع dflt;
	𐤀𐤌 (!ptr->value)
		أرجع "";
	أرجع ptr->value;
}

#وإلا /* !HAVE_LIBECONF */

#ضمن "dl-econf.h"

ساكن econf_file *file = عدم;

فراغ free_getlogindefs_data(فراغ)
{
	𐤀𐤌 (file)
		econf_call(econf_freeFile)(file);
	file = عدم;
}

ساكن فراغ load_defaults(فراغ)
{
	econf_err error;

	𐤀𐤌 (file != عدم)
		free_getlogindefs_data();

	𐤀𐤌 (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, عدم,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#وإلا
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		𐤀𐤌 (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	𐤀𐤌 (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

فراغ logindefs_load_file(ثابت حرف *filename)
{
	econf_file *file_l = عدم, *file_m = عدم;
	حرف *path;

	logindefs_loader = عدم; /* No recursion */

	𐤀𐤌 (ul_dlopen_libeconf() != 0)
		أرجع;

#𐤀𐤌 USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	𐤀𐤌 (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		𐤀𐤌 (file == عدم)
			file = file_l;
		وإلا 𐤀𐤌 (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	𐤀𐤌 (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		𐤀𐤌 (file == عدم)
			file = file_l;
		وإلا 𐤀𐤌 (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} وإلا 𐤀𐤌 (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		𐤀𐤌 (file == عدم)
			file = file_l;
		وإلا 𐤀𐤌 (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
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

	𐤀𐤌 (!file)
		load_defaults();

	𐤀𐤌 (!file)
		أرجع dflt;

	𐤀𐤌 ((error = econf_call(econf_getBoolValue)(file, عدم, name, &value))) {
		𐤀𐤌 (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع value;
}

unsigned long getlogindefs_num(ثابت حرف *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	𐤀𐤌 (!file)
		load_defaults();

	𐤀𐤌 (!file)
		أرجع dflt;

	𐤀𐤌 ((error = econf_call(econf_getUInt64Value)(file, عدم, name, &value))) {
		𐤀𐤌 (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع value;
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

	𐤀𐤌 (!file)
		load_defaults();

	𐤀𐤌 (!file)
		أرجع dflt;

	𐤀𐤌 ((error = econf_call(econf_getStringValue)(file, عدم, name, &value))) {
		𐤀𐤌 (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	𐤀𐤌 (value)
		أرجع value;

	أرجع xstrdup("");
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

	𐤀𐤌 (!val)
		أرجع -1;

	p = strchr(val, '=');
	𐤀𐤌 (p) {
		size_t sz = strlen(name);

		𐤀𐤌 (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			𐤀𐤌 (*val == '"')
				val++;
			𐤀𐤌 (!*val)
				val = dflt;
		}
	}

	أرجع val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
صحيح logindefs_setenv_path(uid_t uid)
{
	صحيح rc = 0;

	𐤀𐤌 (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	وإلا 𐤀𐤌 ((rc = logindefs_setenv("PATH", "ENV_SUPATH", عدم)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	أرجع rc;
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
	𐤀𐤌 (fd != -1)
		close(fd);
	أرجع fd == -1 ? -1 : 0;
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
	𐤀𐤌 (file) {
		𐤀𐤌 (!*file)
			أرجع 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = عدم;
	}

	لكل (i = 0; files[i]; i++) {
		صحيح ok = 0;

		file = files[i];

		/* global hush-file */
		𐤀𐤌 (*file == '/') {
			بنية stat st;
			FILE *f;

			𐤀𐤌 (stat(file, &st) != 0)
				استمر;	/* file does not exist */

			𐤀𐤌 (st.st_size == 0)
				أرجع 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			𐤀𐤌 (!f)
				استمر;	/* ignore errors... */

			طالما (ok == 0 && fgets(buf, حجم(buf), f)) {
				𐤀𐤌 (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			𐤀𐤌 (ok)
				أرجع 1;	/* found username/shell */

			أرجع 0;		/* ignore per-account files */
		}

		/* per-account setting */
		𐤀𐤌 (strlen(pwd->pw_dir) + strlen(file) + 2 > حجم(buf))
			استمر;

		𐤀𐤌 (snprintf(buf, حجم(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			استمر;

		𐤀𐤌 (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			𐤀𐤌 (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			𐤀𐤌 (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			𐤀𐤌 (ok)
				أرجع 1;	/* enabled by user */
		}
		وإلا {
			صحيح rc;
			rc = effective_access(buf, O_RDONLY);
			𐤀𐤌 (rc == 0)
				أرجع 1;

			𐤀𐤌 (rc == -1 && errno == EACCES)
				أرجع -1;
		}

	}

	أرجع 0;
}
#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	𐤀𐤌 (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	𐤀𐤌 (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		صحيح i;
		حرف *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				عدم};

		لكل (i = 0; file && keys[i] != عدم; i++) {
			حرف *value = عدم;

			econf_call(econf_getStringValue)(file, عدم, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		𐤀𐤌 (file)
			econf_call(econf_freeFile)(file);

#وإلا
		بنية item *ptr;

		لكل (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		أرجع EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	𐤀𐤌 (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	وإلا 𐤀𐤌 (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	وإلا 𐤀𐤌 (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	أرجع EXIT_SUCCESS;
}
#endif
