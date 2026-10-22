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
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#include "c.h"
#include "cctype.h"
#include "closestream.h"
#include "logindefs.h"
#include "nls.h"
#include "pathnames.h"
#include "xalloc.h"


static void (*logindefs_loader)(void *) = NULL;
static void *logindefs_loader_data = NULL;

void logindefs_set_loader(void (*loader)(void *data), void *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

sa_nracanaa item {
	var_na *name;		/* name of the option.  */
	var_na *value;		/* value of the option.  */
	var_na *path;		/* name of config file for this option.  */

	sa_nracanaa item *next;	/* pointer to next option.  */
};

static sa_nracanaa item *list = NULL;

void free_getlogindefs_data(void)
{
	sa_nracanaa item *ptr;

	ptr = list;
	ខណៈពេល (ptr) {
		sa_nracanaa item *tmp = ptr->next;

		free(ptr->path);
		free(ptr->name);
		free(ptr->value);
		free(ptr);
		ptr = tmp;
	}

	list = NULL;
}

static void store(const var_na *name, const var_na *value, const var_na *path)
{
	sa_nracanaa item *new = xmalloc(parimaa_na(sa_nracanaa item));

	ប្រសិនបើ (!name)
		abort();

	new->name = xstrdup(name);
	new->value = value && *value ? xstrdup(value) : NULL;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

void logindefs_load_file(const var_na *filename)
{
	FILE *f;
	var_na buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ប្រសិនបើ (!f)
		ត្រឡប់;

	ខណៈពេល (fgets(buf, parimaa_na(buf), f)) {

		var_na *p, *name, *data = NULL;

		ប្រសិនបើ (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		ប្រសិនបើ (p)
			*p = '\0';
		បើមិនដូច្នេះ {
			size_t n = strlen(buf);
			ប្រសិនបើ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ប្រសិនបើ (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ខណៈពេល (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		ខណៈពេល (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		ប្រសិនបើ (data > name && *data)
			*data++ = '\0';

		ប្រសិនបើ (!*name || data == name)
			continue;

		/* go to the begin of the value */
		ខណៈពេល (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		ប្រសិនបើ (p > data)
			p--;
		ខណៈពេល (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	ប្រសិនបើ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	បើមិនដូច្នេះ
		logindefs_load_file(_PATH_LOGINDEFS);
}

static sa_nracanaa item *search(const var_na *name)
{
	sa_nracanaa item *ptr;

	ប្រសិនបើ (!list)
		load_defaults();

	ptr = list;
	ខណៈពេល (ptr != NULL) {
		ប្រសិនបើ (c_strcasecmp(name, ptr->name) == 0)
			ត្រឡប់ ptr;
		ptr = ptr->next;
	}

	ត្រឡប់ NULL;
}

static const var_na *search_config(const var_na *name)
{
	sa_nracanaa item *ptr;

	ptr = list;
	ខណៈពេល (ptr != NULL) {
		ប្រសិនបើ (c_strcasecmp(name, ptr->name) == 0)
			ត្រឡប់ ptr->path;
		ptr = ptr->next;
	}

	ត្រឡប់ NULL;
}

ចំនួនគត់ getlogindefs_bool(const var_na *name, ចំនួនគត់ dflt)
{
	sa_nracanaa item *ptr = search(name);
	ត្រឡប់ ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	sa_nracanaa item *ptr = search(name);
	var_na *end = NULL;
	unsigned long retval;

	ប្រសិនបើ (!ptr || !ptr->value)
		ត្រឡប់ dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	ប្រសិនបើ (end && *end == '\0' && !errno)
		ត្រឡប់ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	ត្រឡប់ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
const var_na *getlogindefs_str(const var_na *name, const var_na *dflt)
{
	sa_nracanaa item *ptr = search(name);

	ប្រសិនបើ (!ptr)
		ត្រឡប់ dflt;
	ប្រសិនបើ (!ptr->value)
		ត្រឡប់ "";
	ត្រឡប់ ptr->value;
}

#បើមិនដូច្នេះ /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	ប្រសិនបើ (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	ប្រសិនបើ (file != NULL)
		free_getlogindefs_data();

	ប្រសិនបើ (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#បើមិនដូច្នេះ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		ប្រសិនបើ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ប្រសិនបើ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const var_na *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	var_na *path;

	logindefs_loader = NULL; /* No recursion */

	ប្រសិនបើ (ul_dlopen_libeconf() != 0)
		ត្រឡប់;

#ប្រសិនបើ USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	ប្រសិនបើ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ប្រសិនបើ (file == NULL)
			file = file_l;
		បើមិនដូច្នេះ ប្រសិនបើ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	ប្រសិនបើ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ប្រសិនបើ (file == NULL)
			file = file_l;
		បើមិនដូច្នេះ ប្រសិនបើ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} បើមិនដូច្នេះ ប្រសិនបើ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ប្រសិនបើ (file == NULL)
			file = file_l;
		បើមិនដូច្នេះ ប្រសិនបើ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

ចំនួនគត់ getlogindefs_bool(const var_na *name, ចំនួនគត់ dflt)
{
	bool value;
	econf_err error;

	ប្រសិនបើ (!file)
		load_defaults();

	ប្រសិនបើ (!file)
		ត្រឡប់ dflt;

	ប្រសិនបើ ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		ប្រសិនបើ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ត្រឡប់ dflt;
	}
	ត្រឡប់ value;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	ប្រសិនបើ (!file)
		load_defaults();

	ប្រសិនបើ (!file)
		ត្រឡប់ dflt;

	ប្រសិនបើ ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		ប្រសិនបើ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ត្រឡប់ dflt;
	}
	ត្រឡប់ value;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
const var_na *getlogindefs_str(const var_na *name, const var_na *dflt)
{
	var_na *value;
	econf_err error;

	ប្រសិនបើ (!file)
		load_defaults();

	ប្រសិនបើ (!file)
		ត្រឡប់ dflt;

	ប្រសិនបើ ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		ប្រសិនបើ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ត្រឡប់ dflt;
	}
	ប្រសិនបើ (value)
		ត្រឡប់ value;

	ត្រឡប់ xstrdup("");
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
ចំនួនគត់ logindefs_setenv(const var_na *name, const var_na *conf, const var_na *dflt)
{
	const var_na *val = getlogindefs_str(conf, dflt);
	const var_na *p;

	ប្រសិនបើ (!val)
		ត្រឡប់ -1;

	p = strchr(val, '=');
	ប្រសិនបើ (p) {
		size_t sz = strlen(name);

		ប្រសិនបើ (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ប្រសិនបើ (*val == '"')
				val++;
			ប្រសិនបើ (!*val)
				val = dflt;
		}
	}

	ត្រឡប់ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ចំនួនគត់ logindefs_setenv_path(uid_t uid)
{
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	បើមិនដូច្នេះ ប្រសិនបើ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	ត្រឡប់ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ចំនួនគត់ effective_access(const var_na *path, ចំនួនគត់ mode)
{
	ចំនួនគត់ fd = open(path, mode);
	ប្រសិនបើ (fd != -1)
		close(fd);
	ត្រឡប់ fd == -1 ? -1 : 0;
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

ចំនួនគត់ get_hushlogin_status(sa_nracanaa passwd *pwd, const var_na *override_home, ចំនួនគត់ force_check)
{
	const var_na *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const var_na *file;
	var_na buf[BUFSIZ];
	ចំនួនគត់ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	ប្រសិនបើ (file) {
		ប្រសិនបើ (!*file)
			ត្រឡប់ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	សម្រាប់ (i = 0; files[i]; i++) {
		ចំនួនគត់ ok = 0;

		file = files[i];

		/* global hush-file */
		ប្រសិនបើ (*file == '/') {
			sa_nracanaa stat st;
			FILE *f;

			ប្រសិនបើ (stat(file, &st) != 0)
				continue;	/* file does not exist */

			ប្រសិនបើ (st.st_size == 0)
				ត្រឡប់ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ប្រសិនបើ (!f)
				continue;	/* ignore errors... */

			ខណៈពេល (ok == 0 && fgets(buf, parimaa_na(buf), f)) {
				ប្រសិនបើ (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ប្រសិនបើ (ok)
				ត្រឡប់ 1;	/* found username/shell */

			ត្រឡប់ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ប្រសិនបើ (strlen(pwd->pw_dir) + strlen(file) + 2 > parimaa_na(buf))
			continue;

		ប្រសិនបើ (snprintf(buf, parimaa_na(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		ប្រសិនបើ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ប្រសិនបើ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ប្រសិនបើ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			ប្រសិនបើ (ok)
				ត្រឡប់ 1;	/* enabled by user */
		}
		បើមិនដូច្នេះ {
			ចំនួនគត់ rc;
			rc = effective_access(buf, O_RDONLY);
			ប្រសិនបើ (rc == 0)
				ត្រឡប់ 1;

			ប្រសិនបើ (rc == -1 && errno == EACCES)
				ត្រឡប់ -1;
		}

	}

	ត្រឡប់ 0;
}
#ifdef TEST_PROGRAM
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	var_na *name, *type;
	close_stdout_atexit();

	ប្រសិនបើ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	ប្រសិនបើ (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ចំនួនគត់ i;
		var_na *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		សម្រាប់ (i = 0; file && keys[i] != NULL; i++) {
			var_na *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		ប្រសិនបើ (file)
			econf_call(econf_freeFile)(file);

#បើមិនដូច្នេះ
		sa_nracanaa item *ptr;

		សម្រាប់ (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		ត្រឡប់ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ប្រសិនបើ (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	បើមិនដូច្នេះ ប្រសិនបើ (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ត្រឡប់ EXIT_SUCCESS;
}
#endif
