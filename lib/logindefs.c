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
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/syslog.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <pwd.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "closestream.h"
#അന്തര്ഭാവ "logindefs.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "xalloc.h"


സ്ഥിതം ശൂന്യം (*logindefs_loader)(ശൂന്യം *) = ശൂന്യ;
സ്ഥിതം ശൂന്യം *logindefs_loader_data = ശൂന്യ;

ശൂന്യം logindefs_set_loader(ശൂന്യം (*loader)(ശൂന്യം *data), ശൂന്യം *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

ഘടന item {
	അക്ഷരം *name;		/* name of the option.  */
	അക്ഷരം *മൂല്യം;		/* value of the option.  */
	അക്ഷരം *path;		/* name of config file for this option.  */

	ഘടന item *next;	/* pointer to next option.  */
};

സ്ഥിതം ഘടന item *list = ശൂന്യ;

ശൂന്യം free_getlogindefs_data(ശൂന്യം)
{
	ഘടന item *ptr;

	ptr = list;
	ആകുമ്പോൾ (ptr) {
		ഘടന item *tmp = ptr->next;

		സ്വതന്ത്രം(ptr->path);
		സ്വതന്ത്രം(ptr->name);
		സ്വതന്ത്രം(ptr->മൂല്യം);
		സ്വതന്ത്രം(ptr);
		ptr = tmp;
	}

	list = ശൂന്യ;
}

സ്ഥിതം ശൂന്യം store(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം, സ്ഥിരം അക്ഷരം *path)
{
	ഘടന item *new = xmalloc(വലുപ്പം(ഘടന item));

	എങ്കിൽ (!name)
		abort();

	new->name = xstrdup(name);
	new->മൂല്യം = മൂല്യം && *മൂല്യം ? xstrdup(മൂല്യം) : ശൂന്യ;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

ശൂന്യം logindefs_load_file(സ്ഥിരം അക്ഷരം *filename)
{
	FILE *f;
	അക്ഷരം buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	എങ്കിൽ (!f)
		തിരികെ;

	ആകുമ്പോൾ (fgets(buf, വലുപ്പം(buf), f)) {

		അക്ഷരം *p, *name, *data = ശൂന്യ;

		എങ്കിൽ (*buf == '#' || *buf == '\n')
			തുടരുക;	/* only comment or empty line */

		p = strchr(buf, '#');
		എങ്കിൽ (p)
			*p = '\0';
		അല്ലെങ്കിൽ {
			size_t n = strlen(buf);
			എങ്കിൽ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		എങ്കിൽ (!*buf)
			തുടരുക;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ആകുമ്പോൾ (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		ആകുമ്പോൾ (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		എങ്കിൽ (data > name && *data)
			*data++ = '\0';

		എങ്കിൽ (!*name || data == name)
			തുടരുക;

		/* go to the begin of the value */
		ആകുമ്പോൾ (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		എങ്കിൽ (p > data)
			p--;
		ആകുമ്പോൾ (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

സ്ഥിതം ശൂന്യം load_defaults(ശൂന്യം)
{
	എങ്കിൽ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	അല്ലെങ്കിൽ
		logindefs_load_file(_PATH_LOGINDEFS);
}

സ്ഥിതം ഘടന item *തിരയൽ(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന item *ptr;

	എങ്കിൽ (!list)
		load_defaults();

	ptr = list;
	ആകുമ്പോൾ (ptr != ശൂന്യ) {
		എങ്കിൽ (c_strcasecmp(name, ptr->name) == 0)
			തിരികെ ptr;
		ptr = ptr->next;
	}

	തിരികെ ശൂന്യ;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *തിരയൽ_config(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന item *ptr;

	ptr = list;
	ആകുമ്പോൾ (ptr != ശൂന്യ) {
		എങ്കിൽ (c_strcasecmp(name, ptr->name) == 0)
			തിരികെ ptr->path;
		ptr = ptr->next;
	}

	തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ getlogindefs_bool(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ dflt)
{
	ഘടന item *ptr = തിരയൽ(name);
	തിരികെ ptr && ptr->മൂല്യം ? (c_strcasecmp(ptr->മൂല്യം, "yes") == 0) : dflt;
}

unsigned long getlogindefs_അക്കം(സ്ഥിരം അക്ഷരം *name, unsigned long dflt)
{
	ഘടന item *ptr = തിരയൽ(name);
	അക്ഷരം *end = ശൂന്യ;
	unsigned long retval;

	എങ്കിൽ (!ptr || !ptr->മൂല്യം)
		തിരികെ dflt;

	errno = 0;
	retval = strtoul(ptr->മൂല്യം, &end, 0);
	എങ്കിൽ (end && *end == '\0' && !errno)
		തിരികെ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       തിരയൽ_config(name), name, ptr->മൂല്യം);
	തിരികെ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
സ്ഥിരം അക്ഷരം *getlogindefs_str(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *dflt)
{
	ഘടന item *ptr = തിരയൽ(name);

	എങ്കിൽ (!ptr)
		തിരികെ dflt;
	എങ്കിൽ (!ptr->മൂല്യം)
		തിരികെ "";
	തിരികെ ptr->മൂല്യം;
}

#അല്ലെങ്കിൽ /* !HAVE_LIBECONF */

#അന്തര്ഭാവ "dl-econf.h"

സ്ഥിതം econf_file *file = ശൂന്യ;

ശൂന്യം free_getlogindefs_data(ശൂന്യം)
{
	എങ്കിൽ (file)
		econf_call(econf_freeFile)(file);
	file = ശൂന്യ;
}

സ്ഥിതം ശൂന്യം load_defaults(ശൂന്യം)
{
	econf_err error;

	എങ്കിൽ (file != ശൂന്യ)
		free_getlogindefs_data();

	എങ്കിൽ (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ശൂന്യ,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#അല്ലെങ്കിൽ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		എങ്കിൽ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	എങ്കിൽ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

ശൂന്യം logindefs_load_file(സ്ഥിരം അക്ഷരം *filename)
{
	econf_file *file_l = ശൂന്യ, *file_m = ശൂന്യ;
	അക്ഷരം *path;

	logindefs_loader = ശൂന്യ; /* No recursion */

	എങ്കിൽ (ul_dlopen_libeconf() != 0)
		തിരികെ;

#എങ്കിൽ USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	എങ്കിൽ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		എങ്കിൽ (file == ശൂന്യ)
			file = file_l;
		അല്ലെങ്കിൽ എങ്കിൽ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	സ്വതന്ത്രം (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	എങ്കിൽ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		എങ്കിൽ (file == ശൂന്യ)
			file = file_l;
		അല്ലെങ്കിൽ എങ്കിൽ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} അല്ലെങ്കിൽ എങ്കിൽ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		എങ്കിൽ (file == ശൂന്യ)
			file = file_l;
		അല്ലെങ്കിൽ എങ്കിൽ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	സ്വതന്ത്രം (path);
}

പൂർണ്ണസംഖ്യ getlogindefs_bool(സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ dflt)
{
	bool മൂല്യം;
	econf_err error;

	എങ്കിൽ (!file)
		load_defaults();

	എങ്കിൽ (!file)
		തിരികെ dflt;

	എങ്കിൽ ((error = econf_call(econf_get_Bool_മൂല്യം)(file, ശൂന്യ, name, &മൂല്യം))) {
		എങ്കിൽ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		തിരികെ dflt;
	}
	തിരികെ മൂല്യം;
}

unsigned long getlogindefs_അക്കം(സ്ഥിരം അക്ഷരം *name, unsigned long dflt)
{
	uint64_t മൂല്യം;
	econf_err error;

	എങ്കിൽ (!file)
		load_defaults();

	എങ്കിൽ (!file)
		തിരികെ dflt;

	എങ്കിൽ ((error = econf_call(econf_get_UInt64_മൂല്യം)(file, ശൂന്യ, name, &മൂല്യം))) {
		എങ്കിൽ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		തിരികെ dflt;
	}
	തിരികെ മൂല്യം;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
സ്ഥിരം അക്ഷരം *getlogindefs_str(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *dflt)
{
	അക്ഷരം *മൂല്യം;
	econf_err error;

	എങ്കിൽ (!file)
		load_defaults();

	എങ്കിൽ (!file)
		തിരികെ dflt;

	എങ്കിൽ ((error = econf_call(econf_get_String_മൂല്യം)(file, ശൂന്യ, name, &മൂല്യം))) {
		എങ്കിൽ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		തിരികെ dflt;
	}
	എങ്കിൽ (മൂല്യം)
		തിരികെ മൂല്യം;

	തിരികെ xstrdup("");
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
പൂർണ്ണസംഖ്യ logindefs_setenv(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *conf, സ്ഥിരം അക്ഷരം *dflt)
{
	സ്ഥിരം അക്ഷരം *val = getlogindefs_str(conf, dflt);
	സ്ഥിരം അക്ഷരം *p;

	എങ്കിൽ (!val)
		തിരികെ -1;

	p = strchr(val, '=');
	എങ്കിൽ (p) {
		size_t sz = strlen(name);

		എങ്കിൽ (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			എങ്കിൽ (*val == '"')
				val++;
			എങ്കിൽ (!*val)
				val = dflt;
		}
	}

	തിരികെ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
പൂർണ്ണസംഖ്യ logindefs_setenv_path(uid_t uid)
{
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	അല്ലെങ്കിൽ എങ്കിൽ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ശൂന്യ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_വേര്);

	തിരികെ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
പൂർണ്ണസംഖ്യ effective_access(സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ mode)
{
	പൂർണ്ണസംഖ്യ fd = open(path, mode);
	എങ്കിൽ (fd != -1)
		close(fd);
	തിരികെ fd == -1 ? -1 : 0;
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

പൂർണ്ണസംഖ്യ get_hushlogin_status(ഘടന passwd *pwd, സ്ഥിരം അക്ഷരം *override_home, പൂർണ്ണസംഖ്യ force_check)
{
	സ്ഥിരം അക്ഷരം *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, ശൂന്യ };
	സ്ഥിരം അക്ഷരം *file;
	അക്ഷരം buf[BUFSIZ];
	പൂർണ്ണസംഖ്യ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ശൂന്യ);
	എങ്കിൽ (file) {
		എങ്കിൽ (!*file)
			തിരികെ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ശൂന്യ;
	}

	വേണ്ടി (i = 0; files[i]; i++) {
		പൂർണ്ണസംഖ്യ ok = 0;

		file = files[i];

		/* global hush-file */
		എങ്കിൽ (*file == '/') {
			ഘടന stat st;
			FILE *f;

			എങ്കിൽ (stat(file, &st) != 0)
				തുടരുക;	/* file does not exist */

			എങ്കിൽ (st.st_size == 0)
				തിരികെ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			എങ്കിൽ (!f)
				തുടരുക;	/* ignore errors... */

			ആകുമ്പോൾ (ok == 0 && fgets(buf, വലുപ്പം(buf), f)) {
				എങ്കിൽ (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			എങ്കിൽ (ok)
				തിരികെ 1;	/* found username/shell */

			തിരികെ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		എങ്കിൽ (strlen(pwd->pw_dir) + strlen(file) + 2 > വലുപ്പം(buf))
			തുടരുക;

		എങ്കിൽ (snprintf(buf, വലുപ്പം(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			തുടരുക;

		എങ്കിൽ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			എങ്കിൽ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			എങ്കിൽ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			എങ്കിൽ (ok)
				തിരികെ 1;	/* enabled by user */
		}
		അല്ലെങ്കിൽ {
			പൂർണ്ണസംഖ്യ rc;
			rc = effective_access(buf, O_RDONLY);
			എങ്കിൽ (rc == 0)
				തിരികെ 1;

			എങ്കിൽ (rc == -1 && errno == EACCES)
				തിരികെ -1;
		}

	}

	തിരികെ 0;
}
#ifdef TEST_PROGRAM
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	അക്ഷരം *name, *type;
	close_stdout_atexit();

	എങ്കിൽ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	എങ്കിൽ (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		പൂർണ്ണസംഖ്യ i;
		അക്ഷരം *താക്കോലുകൾ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ശൂന്യ};

		വേണ്ടി (i = 0; file && താക്കോലുകൾ[i] != ശൂന്യ; i++) {
			അക്ഷരം *മൂല്യം = ശൂന്യ;

			econf_call(econf_get_String_മൂല്യം)(file, ശൂന്യ, താക്കോലുകൾ[i], &മൂല്യം);
			printf ("%s: $%s: '%s'\n", argv[1], താക്കോലുകൾ[i], മൂല്യം);
		}

		എങ്കിൽ (file)
			econf_call(econf_freeFile)(file);

#അല്ലെങ്കിൽ
		ഘടന item *ptr;

		വേണ്ടി (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->മൂല്യം);
#endif
		തിരികെ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	എങ്കിൽ (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_അക്കം(name, 0));
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	തിരികെ EXIT_SUCCESS;
}
#endif
