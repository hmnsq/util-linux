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
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/syslog.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <pwd.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "closestream.h"
#அந்தர்பாவ "logindefs.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "xalloc.h"


நிலையான வெறுமை (*logindefs_loader)(வெறுமை *) = வெற்று;
நிலையான வெறுமை *logindefs_loader_data = வெற்று;

வெறுமை logindefs_set_loader(வெறுமை (*loader)(வெறுமை *data), வெறுமை *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

கட்டமைப்பு item {
	எழுத்து *name;		/* name of the option.  */
	எழுத்து *மதிப்பு;		/* value of the option.  */
	எழுத்து *path;		/* name of config file for this option.  */

	கட்டமைப்பு item *next;	/* pointer to next option.  */
};

நிலையான கட்டமைப்பு item *list = வெற்று;

வெறுமை free_getlogindefs_data(வெறுமை)
{
	கட்டமைப்பு item *ptr;

	ptr = list;
	போது (ptr) {
		கட்டமைப்பு item *tmp = ptr->next;

		விடுவி(ptr->path);
		விடுவி(ptr->name);
		விடுவி(ptr->மதிப்பு);
		விடுவி(ptr);
		ptr = tmp;
	}

	list = வெற்று;
}

நிலையான வெறுமை store(மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு, மாறிலி எழுத்து *path)
{
	கட்டமைப்பு item *new = xmalloc(அளவு(கட்டமைப்பு item));

	எனில் (!name)
		abort();

	new->name = xstrdup(name);
	new->மதிப்பு = மதிப்பு && *மதிப்பு ? xstrdup(மதிப்பு) : வெற்று;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

வெறுமை logindefs_load_file(மாறிலி எழுத்து *filename)
{
	FILE *f;
	எழுத்து buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	எனில் (!f)
		திரும்பு;

	போது (fgets(buf, அளவு(buf), f)) {

		எழுத்து *p, *name, *data = வெற்று;

		எனில் (*buf == '#' || *buf == '\n')
			தொடர்;	/* only comment or empty line */

		p = strchr(buf, '#');
		எனில் (p)
			*p = '\0';
		இல்லையெனில் {
			size_t n = strlen(buf);
			எனில் (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		எனில் (!*buf)
			தொடர்;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		போது (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		போது (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		எனில் (data > name && *data)
			*data++ = '\0';

		எனில் (!*name || data == name)
			தொடர்;

		/* go to the begin of the value */
		போது (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		எனில் (p > data)
			p--;
		போது (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

நிலையான வெறுமை load_defaults(வெறுமை)
{
	எனில் (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	இல்லையெனில்
		logindefs_load_file(_PATH_LOGINDEFS);
}

நிலையான கட்டமைப்பு item *தேடல்(மாறிலி எழுத்து *name)
{
	கட்டமைப்பு item *ptr;

	எனில் (!list)
		load_defaults();

	ptr = list;
	போது (ptr != வெற்று) {
		எனில் (c_strcasecmp(name, ptr->name) == 0)
			திரும்பு ptr;
		ptr = ptr->next;
	}

	திரும்பு வெற்று;
}

நிலையான மாறிலி எழுத்து *தேடல்_config(மாறிலி எழுத்து *name)
{
	கட்டமைப்பு item *ptr;

	ptr = list;
	போது (ptr != வெற்று) {
		எனில் (c_strcasecmp(name, ptr->name) == 0)
			திரும்பு ptr->path;
		ptr = ptr->next;
	}

	திரும்பு வெற்று;
}

முழு getlogindefs_bool(மாறிலி எழுத்து *name, முழு dflt)
{
	கட்டமைப்பு item *ptr = தேடல்(name);
	திரும்பு ptr && ptr->மதிப்பு ? (c_strcasecmp(ptr->மதிப்பு, "yes") == 0) : dflt;
}

unsigned long getlogindefs_இலக்கம்(மாறிலி எழுத்து *name, unsigned long dflt)
{
	கட்டமைப்பு item *ptr = தேடல்(name);
	எழுத்து *end = வெற்று;
	unsigned long retval;

	எனில் (!ptr || !ptr->மதிப்பு)
		திரும்பு dflt;

	errno = 0;
	retval = strtoul(ptr->மதிப்பு, &end, 0);
	எனில் (end && *end == '\0' && !errno)
		திரும்பு retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       தேடல்_config(name), name, ptr->மதிப்பு);
	திரும்பு dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
மாறிலி எழுத்து *getlogindefs_str(மாறிலி எழுத்து *name, மாறிலி எழுத்து *dflt)
{
	கட்டமைப்பு item *ptr = தேடல்(name);

	எனில் (!ptr)
		திரும்பு dflt;
	எனில் (!ptr->மதிப்பு)
		திரும்பு "";
	திரும்பு ptr->மதிப்பு;
}

#இல்லையெனில் /* !HAVE_LIBECONF */

#அந்தர்பாவ "dl-econf.h"

நிலையான econf_file *file = வெற்று;

வெறுமை free_getlogindefs_data(வெறுமை)
{
	எனில் (file)
		econf_call(econf_freeFile)(file);
	file = வெற்று;
}

நிலையான வெறுமை load_defaults(வெறுமை)
{
	econf_err error;

	எனில் (file != வெற்று)
		free_getlogindefs_data();

	எனில் (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, வெற்று,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#இல்லையெனில்
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		எனில் (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	எனில் (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

வெறுமை logindefs_load_file(மாறிலி எழுத்து *filename)
{
	econf_file *file_l = வெற்று, *file_m = வெற்று;
	எழுத்து *path;

	logindefs_loader = வெற்று; /* No recursion */

	எனில் (ul_dlopen_libeconf() != 0)
		திரும்பு;

#எனில் USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	எனில் (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		எனில் (file == வெற்று)
			file = file_l;
		இல்லையெனில் எனில் (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	விடுவி (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	எனில் (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		எனில் (file == வெற்று)
			file = file_l;
		இல்லையெனில் எனில் (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} இல்லையெனில் எனில் (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		எனில் (file == வெற்று)
			file = file_l;
		இல்லையெனில் எனில் (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	விடுவி (path);
}

முழு getlogindefs_bool(மாறிலி எழுத்து *name, முழு dflt)
{
	bool மதிப்பு;
	econf_err error;

	எனில் (!file)
		load_defaults();

	எனில் (!file)
		திரும்பு dflt;

	எனில் ((error = econf_call(econf_get_Bool_மதிப்பு)(file, வெற்று, name, &மதிப்பு))) {
		எனில் (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		திரும்பு dflt;
	}
	திரும்பு மதிப்பு;
}

unsigned long getlogindefs_இலக்கம்(மாறிலி எழுத்து *name, unsigned long dflt)
{
	uint64_t மதிப்பு;
	econf_err error;

	எனில் (!file)
		load_defaults();

	எனில் (!file)
		திரும்பு dflt;

	எனில் ((error = econf_call(econf_get_UInt64_மதிப்பு)(file, வெற்று, name, &மதிப்பு))) {
		எனில் (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		திரும்பு dflt;
	}
	திரும்பு மதிப்பு;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
மாறிலி எழுத்து *getlogindefs_str(மாறிலி எழுத்து *name, மாறிலி எழுத்து *dflt)
{
	எழுத்து *மதிப்பு;
	econf_err error;

	எனில் (!file)
		load_defaults();

	எனில் (!file)
		திரும்பு dflt;

	எனில் ((error = econf_call(econf_get_String_மதிப்பு)(file, வெற்று, name, &மதிப்பு))) {
		எனில் (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		திரும்பு dflt;
	}
	எனில் (மதிப்பு)
		திரும்பு மதிப்பு;

	திரும்பு xstrdup("");
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
முழு logindefs_setenv(மாறிலி எழுத்து *name, மாறிலி எழுத்து *conf, மாறிலி எழுத்து *dflt)
{
	மாறிலி எழுத்து *val = getlogindefs_str(conf, dflt);
	மாறிலி எழுத்து *p;

	எனில் (!val)
		திரும்பு -1;

	p = strchr(val, '=');
	எனில் (p) {
		size_t sz = strlen(name);

		எனில் (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			எனில் (*val == '"')
				val++;
			எனில் (!*val)
				val = dflt;
		}
	}

	திரும்பு val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
முழு logindefs_setenv_path(uid_t uid)
{
	முழு rc = 0;

	எனில் (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	இல்லையெனில் எனில் ((rc = logindefs_setenv("PATH", "ENV_SUPATH", வெற்று)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_வேர்);

	திரும்பு rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
முழு effective_access(மாறிலி எழுத்து *path, முழு mode)
{
	முழு fd = open(path, mode);
	எனில் (fd != -1)
		close(fd);
	திரும்பு fd == -1 ? -1 : 0;
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

முழு get_hushlogin_status(கட்டமைப்பு passwd *pwd, மாறிலி எழுத்து *override_home, முழு force_check)
{
	மாறிலி எழுத்து *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, வெற்று };
	மாறிலி எழுத்து *file;
	எழுத்து buf[BUFSIZ];
	முழு i;

	file = getlogindefs_str("HUSHLOGIN_FILE", வெற்று);
	எனில் (file) {
		எனில் (!*file)
			திரும்பு 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = வெற்று;
	}

	க்கு (i = 0; files[i]; i++) {
		முழு ok = 0;

		file = files[i];

		/* global hush-file */
		எனில் (*file == '/') {
			கட்டமைப்பு stat st;
			FILE *f;

			எனில் (stat(file, &st) != 0)
				தொடர்;	/* file does not exist */

			எனில் (st.st_size == 0)
				திரும்பு 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			எனில் (!f)
				தொடர்;	/* ignore errors... */

			போது (ok == 0 && fgets(buf, அளவு(buf), f)) {
				எனில் (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			எனில் (ok)
				திரும்பு 1;	/* found username/shell */

			திரும்பு 0;		/* ignore per-account files */
		}

		/* per-account setting */
		எனில் (strlen(pwd->pw_dir) + strlen(file) + 2 > அளவு(buf))
			தொடர்;

		எனில் (snprintf(buf, அளவு(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			தொடர்;

		எனில் (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			எனில் (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			எனில் (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			எனில் (ok)
				திரும்பு 1;	/* enabled by user */
		}
		இல்லையெனில் {
			முழு rc;
			rc = effective_access(buf, O_RDONLY);
			எனில் (rc == 0)
				திரும்பு 1;

			எனில் (rc == -1 && errno == EACCES)
				திரும்பு -1;
		}

	}

	திரும்பு 0;
}
#ifdef TEST_PROGRAM
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	எழுத்து *name, *type;
	close_stdout_atexit();

	எனில் (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	எனில் (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		முழு i;
		எழுத்து *சாவிகள்[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				வெற்று};

		க்கு (i = 0; file && சாவிகள்[i] != வெற்று; i++) {
			எழுத்து *மதிப்பு = வெற்று;

			econf_call(econf_get_String_மதிப்பு)(file, வெற்று, சாவிகள்[i], &மதிப்பு);
			printf ("%s: $%s: '%s'\n", argv[1], சாவிகள்[i], மதிப்பு);
		}

		எனில் (file)
			econf_call(econf_freeFile)(file);

#இல்லையெனில்
		கட்டமைப்பு item *ptr;

		க்கு (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->மதிப்பு);
#endif
		திரும்பு EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	எனில் (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	இல்லையெனில் எனில் (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_இலக்கம்(name, 0));
	இல்லையெனில் எனில் (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	திரும்பு EXIT_SUCCESS;
}
#endif
