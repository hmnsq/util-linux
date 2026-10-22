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
	နေစဉ် (ptr) {
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

	အကယ်၍ (!name)
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
	အကယ်၍ (!f)
		ပြန်ပေး;

	နေစဉ် (fgets(buf, parimaa_na(buf), f)) {

		var_na *p, *name, *data = NULL;

		အကယ်၍ (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		အကယ်၍ (p)
			*p = '\0';
		မဟုတ်ပါက {
			size_t n = strlen(buf);
			အကယ်၍ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		အကယ်၍ (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		နေစဉ် (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		နေစဉ် (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		အကယ်၍ (data > name && *data)
			*data++ = '\0';

		အကယ်၍ (!*name || data == name)
			continue;

		/* go to the begin of the value */
		နေစဉ် (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		အကယ်၍ (p > data)
			p--;
		နေစဉ် (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	အကယ်၍ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	မဟုတ်ပါက
		logindefs_load_file(_PATH_LOGINDEFS);
}

static sa_nracanaa item *search(const var_na *name)
{
	sa_nracanaa item *ptr;

	အကယ်၍ (!list)
		load_defaults();

	ptr = list;
	နေစဉ် (ptr != NULL) {
		အကယ်၍ (c_strcasecmp(name, ptr->name) == 0)
			ပြန်ပေး ptr;
		ptr = ptr->next;
	}

	ပြန်ပေး NULL;
}

static const var_na *search_config(const var_na *name)
{
	sa_nracanaa item *ptr;

	ptr = list;
	နေစဉ် (ptr != NULL) {
		အကယ်၍ (c_strcasecmp(name, ptr->name) == 0)
			ပြန်ပေး ptr->path;
		ptr = ptr->next;
	}

	ပြန်ပေး NULL;
}

ကိန်းပြည့် getlogindefs_bool(const var_na *name, ကိန်းပြည့် dflt)
{
	sa_nracanaa item *ptr = search(name);
	ပြန်ပေး ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	sa_nracanaa item *ptr = search(name);
	var_na *end = NULL;
	unsigned long retval;

	အကယ်၍ (!ptr || !ptr->value)
		ပြန်ပေး dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	အကယ်၍ (end && *end == '\0' && !errno)
		ပြန်ပေး retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	ပြန်ပေး dflt;
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

	အကယ်၍ (!ptr)
		ပြန်ပေး dflt;
	အကယ်၍ (!ptr->value)
		ပြန်ပေး "";
	ပြန်ပေး ptr->value;
}

#မဟုတ်ပါက /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	အကယ်၍ (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	အကယ်၍ (file != NULL)
		free_getlogindefs_data();

	အကယ်၍ (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#မဟုတ်ပါက
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		အကယ်၍ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	အကယ်၍ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const var_na *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	var_na *path;

	logindefs_loader = NULL; /* No recursion */

	အကယ်၍ (ul_dlopen_libeconf() != 0)
		ပြန်ပေး;

#အကယ်၍ USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	အကယ်၍ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		အကယ်၍ (file == NULL)
			file = file_l;
		မဟုတ်ပါက အကယ်၍ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	အကယ်၍ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		အကယ်၍ (file == NULL)
			file = file_l;
		မဟုတ်ပါက အကယ်၍ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} မဟုတ်ပါက အကယ်၍ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		အကယ်၍ (file == NULL)
			file = file_l;
		မဟုတ်ပါက အကယ်၍ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

ကိန်းပြည့် getlogindefs_bool(const var_na *name, ကိန်းပြည့် dflt)
{
	bool value;
	econf_err error;

	အကယ်၍ (!file)
		load_defaults();

	အကယ်၍ (!file)
		ပြန်ပေး dflt;

	အကယ်၍ ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		အကယ်၍ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပြန်ပေး dflt;
	}
	ပြန်ပေး value;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	အကယ်၍ (!file)
		load_defaults();

	အကယ်၍ (!file)
		ပြန်ပေး dflt;

	အကယ်၍ ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		အကယ်၍ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပြန်ပေး dflt;
	}
	ပြန်ပေး value;
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

	အကယ်၍ (!file)
		load_defaults();

	အကယ်၍ (!file)
		ပြန်ပေး dflt;

	အကယ်၍ ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		အကယ်၍ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပြန်ပေး dflt;
	}
	အကယ်၍ (value)
		ပြန်ပေး value;

	ပြန်ပေး xstrdup("");
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
ကိန်းပြည့် logindefs_setenv(const var_na *name, const var_na *conf, const var_na *dflt)
{
	const var_na *val = getlogindefs_str(conf, dflt);
	const var_na *p;

	အကယ်၍ (!val)
		ပြန်ပေး -1;

	p = strchr(val, '=');
	အကယ်၍ (p) {
		size_t sz = strlen(name);

		အကယ်၍ (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			အကယ်၍ (*val == '"')
				val++;
			အကယ်၍ (!*val)
				val = dflt;
		}
	}

	ပြန်ပေး val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ကိန်းပြည့် logindefs_setenv_path(uid_t uid)
{
	ကိန်းပြည့် rc = 0;

	အကယ်၍ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	မဟုတ်ပါက အကယ်၍ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	ပြန်ပေး rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ကိန်းပြည့် effective_access(const var_na *path, ကိန်းပြည့် mode)
{
	ကိန်းပြည့် fd = open(path, mode);
	အကယ်၍ (fd != -1)
		close(fd);
	ပြန်ပေး fd == -1 ? -1 : 0;
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

ကိန်းပြည့် get_hushlogin_status(sa_nracanaa passwd *pwd, const var_na *override_home, ကိန်းပြည့် force_check)
{
	const var_na *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const var_na *file;
	var_na buf[BUFSIZ];
	ကိန်းပြည့် i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	အကယ်၍ (file) {
		အကယ်၍ (!*file)
			ပြန်ပေး 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	အတွက် (i = 0; files[i]; i++) {
		ကိန်းပြည့် ok = 0;

		file = files[i];

		/* global hush-file */
		အကယ်၍ (*file == '/') {
			sa_nracanaa stat st;
			FILE *f;

			အကယ်၍ (stat(file, &st) != 0)
				continue;	/* file does not exist */

			အကယ်၍ (st.st_size == 0)
				ပြန်ပေး 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			အကယ်၍ (!f)
				continue;	/* ignore errors... */

			နေစဉ် (ok == 0 && fgets(buf, parimaa_na(buf), f)) {
				အကယ်၍ (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			အကယ်၍ (ok)
				ပြန်ပေး 1;	/* found username/shell */

			ပြန်ပေး 0;		/* ignore per-account files */
		}

		/* per-account setting */
		အကယ်၍ (strlen(pwd->pw_dir) + strlen(file) + 2 > parimaa_na(buf))
			continue;

		အကယ်၍ (snprintf(buf, parimaa_na(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		အကယ်၍ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			အကယ်၍ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			အကယ်၍ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			အကယ်၍ (ok)
				ပြန်ပေး 1;	/* enabled by user */
		}
		မဟုတ်ပါက {
			ကိန်းပြည့် rc;
			rc = effective_access(buf, O_RDONLY);
			အကယ်၍ (rc == 0)
				ပြန်ပေး 1;

			အကယ်၍ (rc == -1 && errno == EACCES)
				ပြန်ပေး -1;
		}

	}

	ပြန်ပေး 0;
}
#ifdef TEST_PROGRAM
ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	var_na *name, *type;
	close_stdout_atexit();

	အကယ်၍ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	အကယ်၍ (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ကိန်းပြည့် i;
		var_na *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		အတွက် (i = 0; file && keys[i] != NULL; i++) {
			var_na *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		အကယ်၍ (file)
			econf_call(econf_freeFile)(file);

#မဟုတ်ပါက
		sa_nracanaa item *ptr;

		အတွက် (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		ပြန်ပေး EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	အကယ်၍ (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	မဟုတ်ပါက အကယ်၍ (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	မဟုတ်ပါက အကယ်၍ (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ပြန်ပေး EXIT_SUCCESS;
}
#endif
