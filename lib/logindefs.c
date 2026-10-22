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

struct item {
	char *name;		/* name of the option.  */
	char *value;		/* value of the option.  */
	char *path;		/* name of config file for this option.  */

	struct item *next;	/* pointer to next option.  */
};

static struct item *list = NULL;

void free_getlogindefs_data(void)
{
	struct item *ptr;

	ptr = list;
	поки (ptr) {
		struct item *tmp = ptr->next;

		free(ptr->path);
		free(ptr->name);
		free(ptr->value);
		free(ptr);
		ptr = tmp;
	}

	list = NULL;
}

static void store(const char *name, const char *value, const char *path)
{
	struct item *new = xmalloc(sizeof(struct item));

	якщо (!name)
		abort();

	new->name = xstrdup(name);
	new->value = value && *value ? xstrdup(value) : NULL;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

void logindefs_load_file(const char *filename)
{
	FILE *f;
	char buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	якщо (!f)
		повернути;

	поки (fgets(buf, sizeof(buf), f)) {

		char *p, *name, *data = NULL;

		якщо (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		якщо (p)
			*p = '\0';
		інакше {
			size_t n = strlen(buf);
			якщо (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		якщо (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		поки (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		поки (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		якщо (data > name && *data)
			*data++ = '\0';

		якщо (!*name || data == name)
			continue;

		/* go to the begin of the value */
		поки (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		якщо (p > data)
			p--;
		поки (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	якщо (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	інакше
		logindefs_load_file(_PATH_LOGINDEFS);
}

static struct item *search(const char *name)
{
	struct item *ptr;

	якщо (!list)
		load_defaults();

	ptr = list;
	поки (ptr != NULL) {
		якщо (c_strcasecmp(name, ptr->name) == 0)
			повернути ptr;
		ptr = ptr->next;
	}

	повернути NULL;
}

static const char *search_config(const char *name)
{
	struct item *ptr;

	ptr = list;
	поки (ptr != NULL) {
		якщо (c_strcasecmp(name, ptr->name) == 0)
			повернути ptr->path;
		ptr = ptr->next;
	}

	повернути NULL;
}

ціле getlogindefs_bool(const char *name, ціле dflt)
{
	struct item *ptr = search(name);
	повернути ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	struct item *ptr = search(name);
	char *end = NULL;
	unsigned long retval;

	якщо (!ptr || !ptr->value)
		повернути dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	якщо (end && *end == '\0' && !errno)
		повернути retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	повернути dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
const char *getlogindefs_str(const char *name, const char *dflt)
{
	struct item *ptr = search(name);

	якщо (!ptr)
		повернути dflt;
	якщо (!ptr->value)
		повернути "";
	повернути ptr->value;
}

#інакше /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	якщо (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	якщо (file != NULL)
		free_getlogindefs_data();

	якщо (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#інакше
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		якщо (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	якщо (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const char *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	char *path;

	logindefs_loader = NULL; /* No recursion */

	якщо (ul_dlopen_libeconf() != 0)
		повернути;

#якщо USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	якщо (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		якщо (file == NULL)
			file = file_l;
		інакше якщо (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	якщо (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		якщо (file == NULL)
			file = file_l;
		інакше якщо (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} інакше якщо (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		якщо (file == NULL)
			file = file_l;
		інакше якщо (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

ціле getlogindefs_bool(const char *name, ціле dflt)
{
	bool value;
	econf_err error;

	якщо (!file)
		load_defaults();

	якщо (!file)
		повернути dflt;

	якщо ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		якщо (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		повернути dflt;
	}
	повернути value;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	якщо (!file)
		load_defaults();

	якщо (!file)
		повернути dflt;

	якщо ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		якщо (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		повернути dflt;
	}
	повернути value;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
const char *getlogindefs_str(const char *name, const char *dflt)
{
	char *value;
	econf_err error;

	якщо (!file)
		load_defaults();

	якщо (!file)
		повернути dflt;

	якщо ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		якщо (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		повернути dflt;
	}
	якщо (value)
		повернути value;

	повернути xstrdup("");
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
ціле logindefs_setenv(const char *name, const char *conf, const char *dflt)
{
	const char *val = getlogindefs_str(conf, dflt);
	const char *p;

	якщо (!val)
		повернути -1;

	p = strchr(val, '=');
	якщо (p) {
		size_t sz = strlen(name);

		якщо (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			якщо (*val == '"')
				val++;
			якщо (!*val)
				val = dflt;
		}
	}

	повернути val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ціле logindefs_setenv_path(uid_t uid)
{
	ціле rc = 0;

	якщо (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	інакше якщо ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	повернути rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ціле effective_access(const char *path, ціле mode)
{
	ціле fd = open(path, mode);
	якщо (fd != -1)
		close(fd);
	повернути fd == -1 ? -1 : 0;
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

ціле get_hushlogin_status(struct passwd *pwd, const char *override_home, ціле force_check)
{
	const char *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const char *file;
	char buf[BUFSIZ];
	ціле i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	якщо (file) {
		якщо (!*file)
			повернути 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	для (i = 0; files[i]; i++) {
		ціле ok = 0;

		file = files[i];

		/* global hush-file */
		якщо (*file == '/') {
			struct stat st;
			FILE *f;

			якщо (stat(file, &st) != 0)
				continue;	/* file does not exist */

			якщо (st.st_size == 0)
				повернути 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			якщо (!f)
				continue;	/* ignore errors... */

			поки (ok == 0 && fgets(buf, sizeof(buf), f)) {
				якщо (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			якщо (ok)
				повернути 1;	/* found username/shell */

			повернути 0;		/* ignore per-account files */
		}

		/* per-account setting */
		якщо (strlen(pwd->pw_dir) + strlen(file) + 2 > sizeof(buf))
			continue;

		якщо (snprintf(buf, sizeof(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		якщо (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			якщо (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			якщо (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			якщо (ok)
				повернути 1;	/* enabled by user */
		}
		інакше {
			ціле rc;
			rc = effective_access(buf, O_RDONLY);
			якщо (rc == 0)
				повернути 1;

			якщо (rc == -1 && errno == EACCES)
				повернути -1;
		}

	}

	повернути 0;
}
#ifdef TEST_PROGRAM
ціле main(ціле argc, char *argv[])
{
	char *name, *type;
	close_stdout_atexit();

	якщо (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	якщо (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ціле i;
		char *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		для (i = 0; file && keys[i] != NULL; i++) {
			char *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		якщо (file)
			econf_call(econf_freeFile)(file);

#інакше
		struct item *ptr;

		для (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		повернути EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	якщо (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	інакше якщо (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	інакше якщо (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	повернути EXIT_SUCCESS;
}
#endif
