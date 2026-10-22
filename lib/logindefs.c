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
	док (ptr) {
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

	ако (!name)
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
	ако (!f)
		врати;

	док (fgets(buf, sizeof(buf), f)) {

		char *p, *name, *data = NULL;

		ако (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		ако (p)
			*p = '\0';
		инако {
			size_t n = strlen(buf);
			ако (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ако (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		док (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		док (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		ако (data > name && *data)
			*data++ = '\0';

		ако (!*name || data == name)
			continue;

		/* go to the begin of the value */
		док (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		ако (p > data)
			p--;
		док (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	ако (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	инако
		logindefs_load_file(_PATH_LOGINDEFS);
}

static struct item *search(const char *name)
{
	struct item *ptr;

	ако (!list)
		load_defaults();

	ptr = list;
	док (ptr != NULL) {
		ако (c_strcasecmp(name, ptr->name) == 0)
			врати ptr;
		ptr = ptr->next;
	}

	врати NULL;
}

static const char *search_config(const char *name)
{
	struct item *ptr;

	ptr = list;
	док (ptr != NULL) {
		ако (c_strcasecmp(name, ptr->name) == 0)
			врати ptr->path;
		ptr = ptr->next;
	}

	врати NULL;
}

цео getlogindefs_bool(const char *name, цео dflt)
{
	struct item *ptr = search(name);
	врати ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	struct item *ptr = search(name);
	char *end = NULL;
	unsigned long retval;

	ако (!ptr || !ptr->value)
		врати dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	ако (end && *end == '\0' && !errno)
		врати retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	врати dflt;
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

	ако (!ptr)
		врати dflt;
	ако (!ptr->value)
		врати "";
	врати ptr->value;
}

#инако /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	ако (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	ако (file != NULL)
		free_getlogindefs_data();

	ако (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#инако
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		ако (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ако (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const char *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	char *path;

	logindefs_loader = NULL; /* No recursion */

	ако (ul_dlopen_libeconf() != 0)
		врати;

#ако USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	ако (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ако (file == NULL)
			file = file_l;
		инако ако (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	ако (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ако (file == NULL)
			file = file_l;
		инако ако (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} инако ако (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ако (file == NULL)
			file = file_l;
		инако ако (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

цео getlogindefs_bool(const char *name, цео dflt)
{
	bool value;
	econf_err error;

	ако (!file)
		load_defaults();

	ако (!file)
		врати dflt;

	ако ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		ако (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		врати dflt;
	}
	врати value;
}

unsigned long getlogindefs_num(const char *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	ако (!file)
		load_defaults();

	ако (!file)
		врати dflt;

	ако ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		ако (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		врати dflt;
	}
	врати value;
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

	ако (!file)
		load_defaults();

	ако (!file)
		врати dflt;

	ако ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		ако (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		врати dflt;
	}
	ако (value)
		врати value;

	врати xstrdup("");
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
цео logindefs_setenv(const char *name, const char *conf, const char *dflt)
{
	const char *val = getlogindefs_str(conf, dflt);
	const char *p;

	ако (!val)
		врати -1;

	p = strchr(val, '=');
	ако (p) {
		size_t sz = strlen(name);

		ако (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ако (*val == '"')
				val++;
			ако (!*val)
				val = dflt;
		}
	}

	врати val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
цео logindefs_setenv_path(uid_t uid)
{
	цео rc = 0;

	ако (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	инако ако ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	врати rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
цео effective_access(const char *path, цео mode)
{
	цео fd = open(path, mode);
	ако (fd != -1)
		close(fd);
	врати fd == -1 ? -1 : 0;
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

цео get_hushlogin_status(struct passwd *pwd, const char *override_home, цео force_check)
{
	const char *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const char *file;
	char buf[BUFSIZ];
	цео i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	ако (file) {
		ако (!*file)
			врати 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	за (i = 0; files[i]; i++) {
		цео ok = 0;

		file = files[i];

		/* global hush-file */
		ако (*file == '/') {
			struct stat st;
			FILE *f;

			ако (stat(file, &st) != 0)
				continue;	/* file does not exist */

			ако (st.st_size == 0)
				врати 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ако (!f)
				continue;	/* ignore errors... */

			док (ok == 0 && fgets(buf, sizeof(buf), f)) {
				ако (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ако (ok)
				врати 1;	/* found username/shell */

			врати 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ако (strlen(pwd->pw_dir) + strlen(file) + 2 > sizeof(buf))
			continue;

		ако (snprintf(buf, sizeof(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		ако (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ако (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ако (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			ако (ok)
				врати 1;	/* enabled by user */
		}
		инако {
			цео rc;
			rc = effective_access(buf, O_RDONLY);
			ако (rc == 0)
				врати 1;

			ако (rc == -1 && errno == EACCES)
				врати -1;
		}

	}

	врати 0;
}
#ifdef TEST_PROGRAM
цео main(цео argc, char *argv[])
{
	char *name, *type;
	close_stdout_atexit();

	ако (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	ако (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		цео i;
		char *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		за (i = 0; file && keys[i] != NULL; i++) {
			char *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		ако (file)
			econf_call(econf_freeFile)(file);

#инако
		struct item *ptr;

		за (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		врати EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ако (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	инако ако (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	инако ако (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	врати EXIT_SUCCESS;
}
#endif
