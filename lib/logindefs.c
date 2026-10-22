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
	пока (ptr) {
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

	если (!name)
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
	если (!f)
		вернуть;

	пока (fgets(buf, sizeof(buf), f)) {

		char *p, *name, *data = NULL;

		если (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		если (p)
			*p = '\0';
		иначе {
			size_t n = strlen(buf);
			если (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		если (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		пока (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		пока (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		если (data > name && *data)
			*data++ = '\0';

		если (!*name || data == name)
			continue;

		/* go to the begin of the value */
		пока (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		если (p > data)
			p--;
		пока (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	если (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	иначе
		logindefs_load_file(_PATH_LOGINDEFS);
}

static struct item *search(const char *name)
{
	struct item *ptr;

	если (!list)
		load_defaults();

	ptr = list;
	пока (ptr != NULL) {
		если (c_strcasecmp(name, ptr->name) == 0)
			вернуть ptr;
		ptr = ptr->next;
	}

	вернуть NULL;
}

static const char *search_config(const char *name)
{
	struct item *ptr;

	ptr = list;
	пока (ptr != NULL) {
		если (c_strcasecmp(name, ptr->name) == 0)
			вернуть ptr->path;
		ptr = ptr->next;
	}

	вернуть NULL;
}

целое getlogindefs_bool(const char *name, целое dflt)
{
	struct item *ptr = search(name);
	вернуть ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	struct item *ptr = search(name);
	char *end = NULL;
	unsigned long retval;

	если (!ptr || !ptr->value)
		вернуть dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	если (end && *end == '\0' && !errno)
		вернуть retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	вернуть dflt;
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

	если (!ptr)
		вернуть dflt;
	если (!ptr->value)
		вернуть "";
	вернуть ptr->value;
}

#иначе /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	если (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	если (file != NULL)
		free_getlogindefs_data();

	если (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#иначе
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		если (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	если (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const char *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	char *path;

	logindefs_loader = NULL; /* No recursion */

	если (ul_dlopen_libeconf() != 0)
		вернуть;

#если USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	если (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		если (file == NULL)
			file = file_l;
		иначе если (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	если (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		если (file == NULL)
			file = file_l;
		иначе если (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} иначе если (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		если (file == NULL)
			file = file_l;
		иначе если (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

целое getlogindefs_bool(const char *name, целое dflt)
{
	bool value;
	econf_err error;

	если (!file)
		load_defaults();

	если (!file)
		вернуть dflt;

	если ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		если (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		вернуть dflt;
	}
	вернуть value;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	если (!file)
		load_defaults();

	если (!file)
		вернуть dflt;

	если ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		если (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		вернуть dflt;
	}
	вернуть value;
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

	если (!file)
		load_defaults();

	если (!file)
		вернуть dflt;

	если ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		если (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		вернуть dflt;
	}
	если (value)
		вернуть value;

	вернуть xstrdup("");
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
целое logindefs_setenv(const char *name, const char *conf, const char *dflt)
{
	const char *val = getlogindefs_str(conf, dflt);
	const char *p;

	если (!val)
		вернуть -1;

	p = strchr(val, '=');
	если (p) {
		size_t sz = strlen(name);

		если (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			если (*val == '"')
				val++;
			если (!*val)
				val = dflt;
		}
	}

	вернуть val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
целое logindefs_setenv_path(uid_t uid)
{
	целое rc = 0;

	если (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	иначе если ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	вернуть rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
целое effective_access(const char *path, целое mode)
{
	целое fd = open(path, mode);
	если (fd != -1)
		close(fd);
	вернуть fd == -1 ? -1 : 0;
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

целое get_hushlogin_status(struct passwd *pwd, const char *override_home, целое force_check)
{
	const char *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const char *file;
	char buf[BUFSIZ];
	целое i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	если (file) {
		если (!*file)
			вернуть 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	для (i = 0; files[i]; i++) {
		целое ok = 0;

		file = files[i];

		/* global hush-file */
		если (*file == '/') {
			struct stat st;
			FILE *f;

			если (stat(file, &st) != 0)
				continue;	/* file does not exist */

			если (st.st_size == 0)
				вернуть 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			если (!f)
				continue;	/* ignore errors... */

			пока (ok == 0 && fgets(buf, sizeof(buf), f)) {
				если (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			если (ok)
				вернуть 1;	/* found username/shell */

			вернуть 0;		/* ignore per-account files */
		}

		/* per-account setting */
		если (strlen(pwd->pw_dir) + strlen(file) + 2 > sizeof(buf))
			continue;

		если (snprintf(buf, sizeof(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		если (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			если (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			если (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			если (ok)
				вернуть 1;	/* enabled by user */
		}
		иначе {
			целое rc;
			rc = effective_access(buf, O_RDONLY);
			если (rc == 0)
				вернуть 1;

			если (rc == -1 && errno == EACCES)
				вернуть -1;
		}

	}

	вернуть 0;
}
#ifdef TEST_PROGRAM
целое основной(целое argc, char *argv[])
{
	char *name, *type;
	close_stdout_atexit();

	если (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	если (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		целое i;
		char *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		для (i = 0; file && keys[i] != NULL; i++) {
			char *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		если (file)
			econf_call(econf_freeFile)(file);

#иначе
		struct item *ptr;

		для (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		вернуть EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	если (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	иначе если (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	иначе если (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	вернуть EXIT_SUCCESS;
}
#endif
