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
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/syslog.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <pwd.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "closestream.h"
#અન્તર્ભાવ "logindefs.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "xalloc.h"


સ્થિર શૂન્ય (*logindefs_loader)(શૂન્ય *) = ખાલી;
સ્થિર શૂન્ય *logindefs_loader_data = ખાલી;

શૂન્ય logindefs_set_loader(શૂન્ય (*loader)(શૂન્ય *data), શૂન્ય *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

સંરચના item {
	અક્ષર *name;		/* name of the option.  */
	અક્ષર *કિંમત;		/* value of the option.  */
	અક્ષર *path;		/* name of config file for this option.  */

	સંરચના item *next;	/* pointer to next option.  */
};

સ્થિર સંરચના item *list = ખાલી;

શૂન્ય free_getlogindefs_data(શૂન્ય)
{
	સંરચના item *ptr;

	ptr = list;
	જ્યારે (ptr) {
		સંરચના item *tmp = ptr->next;

		મુક્ત(ptr->path);
		મુક્ત(ptr->name);
		મુક્ત(ptr->કિંમત);
		મુક્ત(ptr);
		ptr = tmp;
	}

	list = ખાલી;
}

સ્થિર શૂન્ય store(અચલ અક્ષર *name, અચલ અક્ષર *કિંમત, અચલ અક્ષર *path)
{
	સંરચના item *new = xmalloc(કદ(સંરચના item));

	જો (!name)
		abort();

	new->name = xstrdup(name);
	new->કિંમત = કિંમત && *કિંમત ? xstrdup(કિંમત) : ખાલી;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

શૂન્ય logindefs_load_file(અચલ અક્ષર *filename)
{
	FILE *f;
	અક્ષર buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	જો (!f)
		પાછા;

	જ્યારે (fgets(buf, કદ(buf), f)) {

		અક્ષર *p, *name, *data = ખાલી;

		જો (*buf == '#' || *buf == '\n')
			ચાલુ;	/* only comment or empty line */

		p = strchr(buf, '#');
		જો (p)
			*p = '\0';
		નહીંતર {
			size_t n = strlen(buf);
			જો (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		જો (!*buf)
			ચાલુ;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		જ્યારે (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		જ્યારે (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		જો (data > name && *data)
			*data++ = '\0';

		જો (!*name || data == name)
			ચાલુ;

		/* go to the begin of the value */
		જ્યારે (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		જો (p > data)
			p--;
		જ્યારે (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

સ્થિર શૂન્ય load_defaults(શૂન્ય)
{
	જો (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	નહીંતર
		logindefs_load_file(_PATH_LOGINDEFS);
}

સ્થિર સંરચના item *શોધ(અચલ અક્ષર *name)
{
	સંરચના item *ptr;

	જો (!list)
		load_defaults();

	ptr = list;
	જ્યારે (ptr != ખાલી) {
		જો (c_strcasecmp(name, ptr->name) == 0)
			પાછા ptr;
		ptr = ptr->next;
	}

	પાછા ખાલી;
}

સ્થિર અચલ અક્ષર *શોધ_config(અચલ અક્ષર *name)
{
	સંરચના item *ptr;

	ptr = list;
	જ્યારે (ptr != ખાલી) {
		જો (c_strcasecmp(name, ptr->name) == 0)
			પાછા ptr->path;
		ptr = ptr->next;
	}

	પાછા ખાલી;
}

પૂર્ણાંક getlogindefs_bool(અચલ અક્ષર *name, પૂર્ણાંક dflt)
{
	સંરચના item *ptr = શોધ(name);
	પાછા ptr && ptr->કિંમત ? (c_strcasecmp(ptr->કિંમત, "yes") == 0) : dflt;
}

unsigned long getlogindefs_અંક(અચલ અક્ષર *name, unsigned long dflt)
{
	સંરચના item *ptr = શોધ(name);
	અક્ષર *end = ખાલી;
	unsigned long retval;

	જો (!ptr || !ptr->કિંમત)
		પાછા dflt;

	errno = 0;
	retval = strtoul(ptr->કિંમત, &end, 0);
	જો (end && *end == '\0' && !errno)
		પાછા retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       શોધ_config(name), name, ptr->કિંમત);
	પાછા dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
અચલ અક્ષર *getlogindefs_str(અચલ અક્ષર *name, અચલ અક્ષર *dflt)
{
	સંરચના item *ptr = શોધ(name);

	જો (!ptr)
		પાછા dflt;
	જો (!ptr->કિંમત)
		પાછા "";
	પાછા ptr->કિંમત;
}

#નહીંતર /* !HAVE_LIBECONF */

#અન્તર્ભાવ "dl-econf.h"

સ્થિર econf_file *file = ખાલી;

શૂન્ય free_getlogindefs_data(શૂન્ય)
{
	જો (file)
		econf_call(econf_freeFile)(file);
	file = ખાલી;
}

સ્થિર શૂન્ય load_defaults(શૂન્ય)
{
	econf_err error;

	જો (file != ખાલી)
		free_getlogindefs_data();

	જો (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ખાલી,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#નહીંતર
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		જો (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	જો (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

શૂન્ય logindefs_load_file(અચલ અક્ષર *filename)
{
	econf_file *file_l = ખાલી, *file_m = ખાલી;
	અક્ષર *path;

	logindefs_loader = ખાલી; /* No recursion */

	જો (ul_dlopen_libeconf() != 0)
		પાછા;

#જો USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	જો (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		જો (file == ખાલી)
			file = file_l;
		નહીંતર જો (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	મુક્ત (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	જો (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		જો (file == ખાલી)
			file = file_l;
		નહીંતર જો (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} નહીંતર જો (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		જો (file == ખાલી)
			file = file_l;
		નહીંતર જો (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	મુક્ત (path);
}

પૂર્ણાંક getlogindefs_bool(અચલ અક્ષર *name, પૂર્ણાંક dflt)
{
	bool કિંમત;
	econf_err error;

	જો (!file)
		load_defaults();

	જો (!file)
		પાછા dflt;

	જો ((error = econf_call(econf_get_Bool_કિંમત)(file, ખાલી, name, &કિંમત))) {
		જો (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		પાછા dflt;
	}
	પાછા કિંમત;
}

unsigned long getlogindefs_અંક(અચલ અક્ષર *name, unsigned long dflt)
{
	uint64_t કિંમત;
	econf_err error;

	જો (!file)
		load_defaults();

	જો (!file)
		પાછા dflt;

	જો ((error = econf_call(econf_get_UInt64_કિંમત)(file, ખાલી, name, &કિંમત))) {
		જો (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		પાછા dflt;
	}
	પાછા કિંમત;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
અચલ અક્ષર *getlogindefs_str(અચલ અક્ષર *name, અચલ અક્ષર *dflt)
{
	અક્ષર *કિંમત;
	econf_err error;

	જો (!file)
		load_defaults();

	જો (!file)
		પાછા dflt;

	જો ((error = econf_call(econf_get_String_કિંમત)(file, ખાલી, name, &કિંમત))) {
		જો (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		પાછા dflt;
	}
	જો (કિંમત)
		પાછા કિંમત;

	પાછા xstrdup("");
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
પૂર્ણાંક logindefs_setenv(અચલ અક્ષર *name, અચલ અક્ષર *conf, અચલ અક્ષર *dflt)
{
	અચલ અક્ષર *val = getlogindefs_str(conf, dflt);
	અચલ અક્ષર *p;

	જો (!val)
		પાછા -1;

	p = strchr(val, '=');
	જો (p) {
		size_t sz = strlen(name);

		જો (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			જો (*val == '"')
				val++;
			જો (!*val)
				val = dflt;
		}
	}

	પાછા val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
પૂર્ણાંક logindefs_setenv_path(uid_t uid)
{
	પૂર્ણાંક rc = 0;

	જો (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	નહીંતર જો ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ખાલી)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_મૂળ);

	પાછા rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
પૂર્ણાંક effective_access(અચલ અક્ષર *path, પૂર્ણાંક mode)
{
	પૂર્ણાંક fd = open(path, mode);
	જો (fd != -1)
		close(fd);
	પાછા fd == -1 ? -1 : 0;
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

પૂર્ણાંક get_hushlogin_status(સંરચના passwd *pwd, અચલ અક્ષર *override_home, પૂર્ણાંક force_check)
{
	અચલ અક્ષર *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, ખાલી };
	અચલ અક્ષર *file;
	અક્ષર buf[BUFSIZ];
	પૂર્ણાંક i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ખાલી);
	જો (file) {
		જો (!*file)
			પાછા 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ખાલી;
	}

	માટે (i = 0; files[i]; i++) {
		પૂર્ણાંક ok = 0;

		file = files[i];

		/* global hush-file */
		જો (*file == '/') {
			સંરચના stat st;
			FILE *f;

			જો (stat(file, &st) != 0)
				ચાલુ;	/* file does not exist */

			જો (st.st_size == 0)
				પાછા 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			જો (!f)
				ચાલુ;	/* ignore errors... */

			જ્યારે (ok == 0 && fgets(buf, કદ(buf), f)) {
				જો (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			જો (ok)
				પાછા 1;	/* found username/shell */

			પાછા 0;		/* ignore per-account files */
		}

		/* per-account setting */
		જો (strlen(pwd->pw_dir) + strlen(file) + 2 > કદ(buf))
			ચાલુ;

		જો (snprintf(buf, કદ(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ચાલુ;

		જો (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			જો (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			જો (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			જો (ok)
				પાછા 1;	/* enabled by user */
		}
		નહીંતર {
			પૂર્ણાંક rc;
			rc = effective_access(buf, O_RDONLY);
			જો (rc == 0)
				પાછા 1;

			જો (rc == -1 && errno == EACCES)
				પાછા -1;
		}

	}

	પાછા 0;
}
#ifdef TEST_PROGRAM
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અક્ષર *name, *type;
	close_stdout_atexit();

	જો (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	જો (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		પૂર્ણાંક i;
		અક્ષર *ચાવીઓ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ખાલી};

		માટે (i = 0; file && ચાવીઓ[i] != ખાલી; i++) {
			અક્ષર *કિંમત = ખાલી;

			econf_call(econf_get_String_કિંમત)(file, ખાલી, ચાવીઓ[i], &કિંમત);
			printf ("%s: $%s: '%s'\n", argv[1], ચાવીઓ[i], કિંમત);
		}

		જો (file)
			econf_call(econf_freeFile)(file);

#નહીંતર
		સંરચના item *ptr;

		માટે (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->કિંમત);
#endif
		પાછા EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	જો (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	નહીંતર જો (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_અંક(name, 0));
	નહીંતર જો (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	પાછા EXIT_SUCCESS;
}
#endif
