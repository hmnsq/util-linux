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
	སྐབས (ptr) {
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

	གལ་ཏེ (!name)
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
	གལ་ཏེ (!f)
		ཕྱིར;

	སྐབས (fgets(buf, parimaa_na(buf), f)) {

		var_na *p, *name, *data = NULL;

		གལ་ཏེ (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		གལ་ཏེ (p)
			*p = '\0';
		དེ་མིན {
			size_t n = strlen(buf);
			གལ་ཏེ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		གལ་ཏེ (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		སྐབས (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		སྐབས (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		གལ་ཏེ (data > name && *data)
			*data++ = '\0';

		གལ་ཏེ (!*name || data == name)
			continue;

		/* go to the begin of the value */
		སྐབས (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		གལ་ཏེ (p > data)
			p--;
		སྐབས (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	གལ་ཏེ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	དེ་མིན
		logindefs_load_file(_PATH_LOGINDEFS);
}

static sa_nracanaa item *search(const var_na *name)
{
	sa_nracanaa item *ptr;

	གལ་ཏེ (!list)
		load_defaults();

	ptr = list;
	སྐབས (ptr != NULL) {
		གལ་ཏེ (c_strcasecmp(name, ptr->name) == 0)
			ཕྱིར ptr;
		ptr = ptr->next;
	}

	ཕྱིར NULL;
}

static const var_na *search_config(const var_na *name)
{
	sa_nracanaa item *ptr;

	ptr = list;
	སྐབས (ptr != NULL) {
		གལ་ཏེ (c_strcasecmp(name, ptr->name) == 0)
			ཕྱིར ptr->path;
		ptr = ptr->next;
	}

	ཕྱིར NULL;
}

ཧྲིལ getlogindefs_bool(const var_na *name, ཧྲིལ dflt)
{
	sa_nracanaa item *ptr = search(name);
	ཕྱིར ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	sa_nracanaa item *ptr = search(name);
	var_na *end = NULL;
	unsigned long retval;

	གལ་ཏེ (!ptr || !ptr->value)
		ཕྱིར dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	གལ་ཏེ (end && *end == '\0' && !errno)
		ཕྱིར retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	ཕྱིར dflt;
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

	གལ་ཏེ (!ptr)
		ཕྱིར dflt;
	གལ་ཏེ (!ptr->value)
		ཕྱིར "";
	ཕྱིར ptr->value;
}

#དེ་མིན /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	གལ་ཏེ (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	གལ་ཏེ (file != NULL)
		free_getlogindefs_data();

	གལ་ཏེ (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#དེ་མིན
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		གལ་ཏེ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	གལ་ཏེ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const var_na *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	var_na *path;

	logindefs_loader = NULL; /* No recursion */

	གལ་ཏེ (ul_dlopen_libeconf() != 0)
		ཕྱིར;

#གལ་ཏེ USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	གལ་ཏེ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		གལ་ཏེ (file == NULL)
			file = file_l;
		དེ་མིན གལ་ཏེ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	གལ་ཏེ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		གལ་ཏེ (file == NULL)
			file = file_l;
		དེ་མིན གལ་ཏེ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} དེ་མིན གལ་ཏེ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		གལ་ཏེ (file == NULL)
			file = file_l;
		དེ་མིན གལ་ཏེ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

ཧྲིལ getlogindefs_bool(const var_na *name, ཧྲིལ dflt)
{
	bool value;
	econf_err error;

	གལ་ཏེ (!file)
		load_defaults();

	གལ་ཏེ (!file)
		ཕྱིར dflt;

	གལ་ཏེ ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		གལ་ཏེ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ཕྱིར dflt;
	}
	ཕྱིར value;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	གལ་ཏེ (!file)
		load_defaults();

	གལ་ཏེ (!file)
		ཕྱིར dflt;

	གལ་ཏེ ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		གལ་ཏེ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ཕྱིར dflt;
	}
	ཕྱིར value;
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

	གལ་ཏེ (!file)
		load_defaults();

	གལ་ཏེ (!file)
		ཕྱིར dflt;

	གལ་ཏེ ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		གལ་ཏེ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ཕྱིར dflt;
	}
	གལ་ཏེ (value)
		ཕྱིར value;

	ཕྱིར xstrdup("");
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
ཧྲིལ logindefs_setenv(const var_na *name, const var_na *conf, const var_na *dflt)
{
	const var_na *val = getlogindefs_str(conf, dflt);
	const var_na *p;

	གལ་ཏེ (!val)
		ཕྱིར -1;

	p = strchr(val, '=');
	གལ་ཏེ (p) {
		size_t sz = strlen(name);

		གལ་ཏེ (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			གལ་ཏེ (*val == '"')
				val++;
			གལ་ཏེ (!*val)
				val = dflt;
		}
	}

	ཕྱིར val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ཧྲིལ logindefs_setenv_path(uid_t uid)
{
	ཧྲིལ rc = 0;

	གལ་ཏེ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	དེ་མིན གལ་ཏེ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	ཕྱིར rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ཧྲིལ effective_access(const var_na *path, ཧྲིལ mode)
{
	ཧྲིལ fd = open(path, mode);
	གལ་ཏེ (fd != -1)
		close(fd);
	ཕྱིར fd == -1 ? -1 : 0;
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

ཧྲིལ get_hushlogin_status(sa_nracanaa passwd *pwd, const var_na *override_home, ཧྲིལ force_check)
{
	const var_na *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const var_na *file;
	var_na buf[BUFSIZ];
	ཧྲིལ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	གལ་ཏེ (file) {
		གལ་ཏེ (!*file)
			ཕྱིར 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	ཆེད་དུ (i = 0; files[i]; i++) {
		ཧྲིལ ok = 0;

		file = files[i];

		/* global hush-file */
		གལ་ཏེ (*file == '/') {
			sa_nracanaa stat st;
			FILE *f;

			གལ་ཏེ (stat(file, &st) != 0)
				continue;	/* file does not exist */

			གལ་ཏེ (st.st_size == 0)
				ཕྱིར 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			གལ་ཏེ (!f)
				continue;	/* ignore errors... */

			སྐབས (ok == 0 && fgets(buf, parimaa_na(buf), f)) {
				གལ་ཏེ (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			གལ་ཏེ (ok)
				ཕྱིར 1;	/* found username/shell */

			ཕྱིར 0;		/* ignore per-account files */
		}

		/* per-account setting */
		གལ་ཏེ (strlen(pwd->pw_dir) + strlen(file) + 2 > parimaa_na(buf))
			continue;

		གལ་ཏེ (snprintf(buf, parimaa_na(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		གལ་ཏེ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			གལ་ཏེ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			གལ་ཏེ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			གལ་ཏེ (ok)
				ཕྱིར 1;	/* enabled by user */
		}
		དེ་མིན {
			ཧྲིལ rc;
			rc = effective_access(buf, O_RDONLY);
			གལ་ཏེ (rc == 0)
				ཕྱིར 1;

			གལ་ཏེ (rc == -1 && errno == EACCES)
				ཕྱིར -1;
		}

	}

	ཕྱིར 0;
}
#ifdef TEST_PROGRAM
ཧྲིལ གཙོ(ཧྲིལ argc, var_na *argv[])
{
	var_na *name, *type;
	close_stdout_atexit();

	གལ་ཏེ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	གལ་ཏེ (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ཧྲིལ i;
		var_na *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		ཆེད་དུ (i = 0; file && keys[i] != NULL; i++) {
			var_na *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		གལ་ཏེ (file)
			econf_call(econf_freeFile)(file);

#དེ་མིན
		sa_nracanaa item *ptr;

		ཆེད་དུ (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		ཕྱིར EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	གལ་ཏེ (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	དེ་མིན གལ་ཏེ (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	དེ་མིན གལ་ཏེ (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ཕྱིར EXIT_SUCCESS;
}
#endif
