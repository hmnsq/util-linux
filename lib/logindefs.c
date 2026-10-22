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
	ၶၢဝ်း (ptr) {
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

	ပေႃး (!name)
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
	ပေႃး (!f)
		ပွၵ်ႈ;

	ၶၢဝ်း (fgets(buf, parimaa_na(buf), f)) {

		var_na *p, *name, *data = NULL;

		ပေႃး (*buf == '#' || *buf == '\n')
			continue;	/* only comment or empty line */

		p = strchr(buf, '#');
		ပေႃး (p)
			*p = '\0';
		ပေႃးဢမ်ႇ {
			size_t n = strlen(buf);
			ပေႃး (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ပေႃး (!*buf)
			continue;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ၶၢဝ်း (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		ၶၢဝ်း (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		ပေႃး (data > name && *data)
			*data++ = '\0';

		ပေႃး (!*name || data == name)
			continue;

		/* go to the begin of the value */
		ၶၢဝ်း (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		ပေႃး (p > data)
			p--;
		ၶၢဝ်း (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

static void load_defaults(void)
{
	ပေႃး (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	ပေႃးဢမ်ႇ
		logindefs_load_file(_PATH_LOGINDEFS);
}

static sa_nracanaa item *search(const var_na *name)
{
	sa_nracanaa item *ptr;

	ပေႃး (!list)
		load_defaults();

	ptr = list;
	ၶၢဝ်း (ptr != NULL) {
		ပေႃး (c_strcasecmp(name, ptr->name) == 0)
			ပွၵ်ႈ ptr;
		ptr = ptr->next;
	}

	ပွၵ်ႈ NULL;
}

static const var_na *search_config(const var_na *name)
{
	sa_nracanaa item *ptr;

	ptr = list;
	ၶၢဝ်း (ptr != NULL) {
		ပေႃး (c_strcasecmp(name, ptr->name) == 0)
			ပွၵ်ႈ ptr->path;
		ptr = ptr->next;
	}

	ပွၵ်ႈ NULL;
}

puur_naa_nka getlogindefs_bool(const var_na *name, puur_naa_nka dflt)
{
	sa_nracanaa item *ptr = search(name);
	ပွၵ်ႈ ptr && ptr->value ? (c_strcasecmp(ptr->value, "yes") == 0) : dflt;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	sa_nracanaa item *ptr = search(name);
	var_na *end = NULL;
	unsigned long retval;

	ပေႃး (!ptr || !ptr->value)
		ပွၵ်ႈ dflt;

	errno = 0;
	retval = strtoul(ptr->value, &end, 0);
	ပေႃး (end && *end == '\0' && !errno)
		ပွၵ်ႈ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       search_config(name), name, ptr->value);
	ပွၵ်ႈ dflt;
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

	ပေႃး (!ptr)
		ပွၵ်ႈ dflt;
	ပေႃး (!ptr->value)
		ပွၵ်ႈ "";
	ပွၵ်ႈ ptr->value;
}

#ပေႃးဢမ်ႇ /* !HAVE_LIBECONF */

#include "dl-econf.h"

static econf_file *file = NULL;

void free_getlogindefs_data(void)
{
	ပေႃး (file)
		econf_call(econf_freeFile)(file);
	file = NULL;
}

static void load_defaults(void)
{
	econf_err error;

	ပေႃး (file != NULL)
		free_getlogindefs_data();

	ပေႃး (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, NULL,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#ပေႃးဢမ်ႇ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		ပေႃး (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ပေႃး (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

void logindefs_load_file(const var_na *filename)
{
	econf_file *file_l = NULL, *file_m = NULL;
	var_na *path;

	logindefs_loader = NULL; /* No recursion */

	ပေႃး (ul_dlopen_libeconf() != 0)
		ပွၵ်ႈ;

#ပေႃး USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	ပေႃး (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ပေႃး (file == NULL)
			file = file_l;
		ပေႃးဢမ်ႇ ပေႃး (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	ပေႃး (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ပေႃး (file == NULL)
			file = file_l;
		ပေႃးဢမ်ႇ ပေႃး (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ပေႃးဢမ်ႇ ပေႃး (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ပေႃး (file == NULL)
			file = file_l;
		ပေႃးဢမ်ႇ ပေႃး (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	free (path);
}

puur_naa_nka getlogindefs_bool(const var_na *name, puur_naa_nka dflt)
{
	bool value;
	econf_err error;

	ပေႃး (!file)
		load_defaults();

	ပေႃး (!file)
		ပွၵ်ႈ dflt;

	ပေႃး ((error = econf_call(econf_getBoolValue)(file, NULL, name, &value))) {
		ပေႃး (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပွၵ်ႈ dflt;
	}
	ပွၵ်ႈ value;
}

unsigned long getlogindefs_num(const var_na *name, unsigned long dflt)
{
	uint64_t value;
	econf_err error;

	ပေႃး (!file)
		load_defaults();

	ပေႃး (!file)
		ပွၵ်ႈ dflt;

	ပေႃး ((error = econf_call(econf_getUInt64Value)(file, NULL, name, &value))) {
		ပေႃး (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပွၵ်ႈ dflt;
	}
	ပွၵ်ႈ value;
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

	ပေႃး (!file)
		load_defaults();

	ပေႃး (!file)
		ပွၵ်ႈ dflt;

	ပေႃး ((error = econf_call(econf_getStringValue)(file, NULL, name, &value))) {
		ပေႃး (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ပွၵ်ႈ dflt;
	}
	ပေႃး (value)
		ပွၵ်ႈ value;

	ပွၵ်ႈ xstrdup("");
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
puur_naa_nka logindefs_setenv(const var_na *name, const var_na *conf, const var_na *dflt)
{
	const var_na *val = getlogindefs_str(conf, dflt);
	const var_na *p;

	ပေႃး (!val)
		ပွၵ်ႈ -1;

	p = strchr(val, '=');
	ပေႃး (p) {
		size_t sz = strlen(name);

		ပေႃး (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ပေႃး (*val == '"')
				val++;
			ပေႃး (!*val)
				val = dflt;
		}
	}

	ပွၵ်ႈ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
puur_naa_nka logindefs_setenv_path(uid_t uid)
{
	puur_naa_nka rc = 0;

	ပေႃး (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ပေႃးဢမ်ႇ ပေႃး ((rc = logindefs_setenv("PATH", "ENV_SUPATH", NULL)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", _PATH_DEFPATH_ROOT);

	ပွၵ်ႈ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
puur_naa_nka effective_access(const var_na *path, puur_naa_nka mode)
{
	puur_naa_nka fd = open(path, mode);
	ပေႃး (fd != -1)
		close(fd);
	ပွၵ်ႈ fd == -1 ? -1 : 0;
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

puur_naa_nka get_hushlogin_status(sa_nracanaa passwd *pwd, const var_na *override_home, puur_naa_nka force_check)
{
	const var_na *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, NULL };
	const var_na *file;
	var_na buf[BUFSIZ];
	puur_naa_nka i;

	file = getlogindefs_str("HUSHLOGIN_FILE", NULL);
	ပေႃး (file) {
		ပေႃး (!*file)
			ပွၵ်ႈ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = NULL;
	}

	for (i = 0; files[i]; i++) {
		puur_naa_nka ok = 0;

		file = files[i];

		/* global hush-file */
		ပေႃး (*file == '/') {
			sa_nracanaa stat st;
			FILE *f;

			ပေႃး (stat(file, &st) != 0)
				continue;	/* file does not exist */

			ပေႃး (st.st_size == 0)
				ပွၵ်ႈ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ပေႃး (!f)
				continue;	/* ignore errors... */

			ၶၢဝ်း (ok == 0 && fgets(buf, parimaa_na(buf), f)) {
				ပေႃး (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ပေႃး (ok)
				ပွၵ်ႈ 1;	/* found username/shell */

			ပွၵ်ႈ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ပေႃး (strlen(pwd->pw_dir) + strlen(file) + 2 > parimaa_na(buf))
			continue;

		ပေႃး (snprintf(buf, parimaa_na(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			continue;

		ပေႃး (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ပေႃး (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ပေႃး (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			ပေႃး (ok)
				ပွၵ်ႈ 1;	/* enabled by user */
		}
		ပေႃးဢမ်ႇ {
			puur_naa_nka rc;
			rc = effective_access(buf, O_RDONLY);
			ပေႃး (rc == 0)
				ပွၵ်ႈ 1;

			ပေႃး (rc == -1 && errno == EACCES)
				ပွၵ်ႈ -1;
		}

	}

	ပွၵ်ႈ 0;
}
#ifdef TEST_PROGRAM
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	var_na *name, *type;
	close_stdout_atexit();

	ပေႃး (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	ပေႃး (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		puur_naa_nka i;
		var_na *keys[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				NULL};

		for (i = 0; file && keys[i] != NULL; i++) {
			var_na *value = NULL;

			econf_call(econf_getStringValue)(file, NULL, keys[i], &value);
			printf ("%s: $%s: '%s'\n", argv[1], keys[i], value);
		}

		ပေႃး (file)
			econf_call(econf_freeFile)(file);

#ပေႃးဢမ်ႇ
		sa_nracanaa item *ptr;

		for (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->value);
#endif
		ပွၵ်ႈ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ပေႃး (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ပေႃးဢမ်ႇ ပေႃး (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_num(name, 0));
	ပေႃးဢမ်ႇ ပေႃး (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ပွၵ်ႈ EXIT_SUCCESS;
}
#endif
