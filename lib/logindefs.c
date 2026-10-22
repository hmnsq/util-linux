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
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/syslog.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <pwd.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "closestream.h"
#অন্তর্ভাব "logindefs.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "xalloc.h"


স্থির শূন্য (*logindefs_loader)(শূন্য *) = রিক্ত;
স্থির শূন্য *logindefs_loader_data = রিক্ত;

শূন্য logindefs_set_loader(শূন্য (*loader)(শূন্য *data), শূন্য *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

সংরচনা item {
	বর্ণ *name;		/* name of the option.  */
	বর্ণ *মান;		/* value of the option.  */
	বর্ণ *path;		/* name of config file for this option.  */

	সংরচনা item *next;	/* pointer to next option.  */
};

স্থির সংরচনা item *list = রিক্ত;

শূন্য free_getlogindefs_data(শূন্য)
{
	সংরচনা item *ptr;

	ptr = list;
	যাবত্ (ptr) {
		সংরচনা item *tmp = ptr->next;

		মুক্ত(ptr->path);
		মুক্ত(ptr->name);
		মুক্ত(ptr->মান);
		মুক্ত(ptr);
		ptr = tmp;
	}

	list = রিক্ত;
}

স্থির শূন্য store(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *মান, ধ্রুব বর্ণ *path)
{
	সংরচনা item *new = xmalloc(পরিমাণ(সংরচনা item));

	যদি (!name)
		abort();

	new->name = xstrdup(name);
	new->মান = মান && *মান ? xstrdup(মান) : রিক্ত;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

শূন্য logindefs_load_file(ধ্রুব বর্ণ *filename)
{
	FILE *f;
	বর্ণ buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	যদি (!f)
		প্রত্যাগম;

	যাবত্ (fgets(buf, পরিমাণ(buf), f)) {

		বর্ণ *p, *name, *data = রিক্ত;

		যদি (*buf == '#' || *buf == '\n')
			অনুবর্ত;	/* only comment or empty line */

		p = strchr(buf, '#');
		যদি (p)
			*p = '\0';
		অন্যথা {
			size_t n = strlen(buf);
			যদি (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		যদি (!*buf)
			অনুবর্ত;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		যাবত্ (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		যাবত্ (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		যদি (data > name && *data)
			*data++ = '\0';

		যদি (!*name || data == name)
			অনুবর্ত;

		/* go to the begin of the value */
		যাবত্ (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		যদি (p > data)
			p--;
		যাবত্ (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

স্থির শূন্য load_defaults(শূন্য)
{
	যদি (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	অন্যথা
		logindefs_load_file(_PATH_LOGINDEFS);
}

স্থির সংরচনা item *অনুসন্ধান(ধ্রুব বর্ণ *name)
{
	সংরচনা item *ptr;

	যদি (!list)
		load_defaults();

	ptr = list;
	যাবত্ (ptr != রিক্ত) {
		যদি (c_strcasecmp(name, ptr->name) == 0)
			প্রত্যাগম ptr;
		ptr = ptr->next;
	}

	প্রত্যাগম রিক্ত;
}

স্থির ধ্রুব বর্ণ *অনুসন্ধান_config(ধ্রুব বর্ণ *name)
{
	সংরচনা item *ptr;

	ptr = list;
	যাবত্ (ptr != রিক্ত) {
		যদি (c_strcasecmp(name, ptr->name) == 0)
			প্রত্যাগম ptr->path;
		ptr = ptr->next;
	}

	প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক getlogindefs_bool(ধ্রুব বর্ণ *name, পূর্ণাঙ্ক dflt)
{
	সংরচনা item *ptr = অনুসন্ধান(name);
	প্রত্যাগম ptr && ptr->মান ? (c_strcasecmp(ptr->মান, "yes") == 0) : dflt;
}

unsigned long getlogindefs_অঙ্ক(ধ্রুব বর্ণ *name, unsigned long dflt)
{
	সংরচনা item *ptr = অনুসন্ধান(name);
	বর্ণ *end = রিক্ত;
	unsigned long retval;

	যদি (!ptr || !ptr->মান)
		প্রত্যাগম dflt;

	errno = 0;
	retval = strtoul(ptr->মান, &end, 0);
	যদি (end && *end == '\0' && !errno)
		প্রত্যাগম retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       অনুসন্ধান_config(name), name, ptr->মান);
	প্রত্যাগম dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ধ্রুব বর্ণ *getlogindefs_str(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *dflt)
{
	সংরচনা item *ptr = অনুসন্ধান(name);

	যদি (!ptr)
		প্রত্যাগম dflt;
	যদি (!ptr->মান)
		প্রত্যাগম "";
	প্রত্যাগম ptr->মান;
}

#অন্যথা /* !HAVE_LIBECONF */

#অন্তর্ভাব "dl-econf.h"

স্থির econf_file *file = রিক্ত;

শূন্য free_getlogindefs_data(শূন্য)
{
	যদি (file)
		econf_call(econf_freeFile)(file);
	file = রিক্ত;
}

স্থির শূন্য load_defaults(শূন্য)
{
	econf_err error;

	যদি (file != রিক্ত)
		free_getlogindefs_data();

	যদি (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, রিক্ত,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#অন্যথা
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		যদি (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	যদি (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

শূন্য logindefs_load_file(ধ্রুব বর্ণ *filename)
{
	econf_file *file_l = রিক্ত, *file_m = রিক্ত;
	বর্ণ *path;

	logindefs_loader = রিক্ত; /* No recursion */

	যদি (ul_dlopen_libeconf() != 0)
		প্রত্যাগম;

#যদি USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	যদি (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		যদি (file == রিক্ত)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	মুক্ত (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	যদি (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		যদি (file == রিক্ত)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} অন্যথা যদি (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		যদি (file == রিক্ত)
			file = file_l;
		অন্যথা যদি (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	মুক্ত (path);
}

পূর্ণাঙ্ক getlogindefs_bool(ধ্রুব বর্ণ *name, পূর্ণাঙ্ক dflt)
{
	bool মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		প্রত্যাগম dflt;

	যদি ((error = econf_call(econf_get_Bool_মান)(file, রিক্ত, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		প্রত্যাগম dflt;
	}
	প্রত্যাগম মান;
}

unsigned long getlogindefs_অঙ্ক(ধ্রুব বর্ণ *name, unsigned long dflt)
{
	uint64_t মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		প্রত্যাগম dflt;

	যদি ((error = econf_call(econf_get_UInt64_মান)(file, রিক্ত, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		প্রত্যাগম dflt;
	}
	প্রত্যাগম মান;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ধ্রুব বর্ণ *getlogindefs_str(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *dflt)
{
	বর্ণ *মান;
	econf_err error;

	যদি (!file)
		load_defaults();

	যদি (!file)
		প্রত্যাগম dflt;

	যদি ((error = econf_call(econf_get_String_মান)(file, রিক্ত, name, &মান))) {
		যদি (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		প্রত্যাগম dflt;
	}
	যদি (মান)
		প্রত্যাগম মান;

	প্রত্যাগম xstrdup("");
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
পূর্ণাঙ্ক logindefs_setenv(ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *conf, ধ্রুব বর্ণ *dflt)
{
	ধ্রুব বর্ণ *val = getlogindefs_str(conf, dflt);
	ধ্রুব বর্ণ *p;

	যদি (!val)
		প্রত্যাগম -1;

	p = strchr(val, '=');
	যদি (p) {
		size_t sz = strlen(name);

		যদি (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			যদি (*val == '"')
				val++;
			যদি (!*val)
				val = dflt;
		}
	}

	প্রত্যাগম val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
পূর্ণাঙ্ক logindefs_setenv_path(uid_t uid)
{
	পূর্ণাঙ্ক rc = 0;

	যদি (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	অন্যথা যদি ((rc = logindefs_setenv("PATH", "ENV_SUPATH", রিক্ত)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_মূল);

	প্রত্যাগম rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
পূর্ণাঙ্ক effective_access(ধ্রুব বর্ণ *path, পূর্ণাঙ্ক mode)
{
	পূর্ণাঙ্ক fd = open(path, mode);
	যদি (fd != -1)
		close(fd);
	প্রত্যাগম fd == -1 ? -1 : 0;
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

পূর্ণাঙ্ক get_hushlogin_status(সংরচনা passwd *pwd, ধ্রুব বর্ণ *override_home, পূর্ণাঙ্ক force_check)
{
	ধ্রুব বর্ণ *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, রিক্ত };
	ধ্রুব বর্ণ *file;
	বর্ণ buf[BUFSIZ];
	পূর্ণাঙ্ক i;

	file = getlogindefs_str("HUSHLOGIN_FILE", রিক্ত);
	যদি (file) {
		যদি (!*file)
			প্রত্যাগম 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = রিক্ত;
	}

	প্রতি (i = 0; files[i]; i++) {
		পূর্ণাঙ্ক ok = 0;

		file = files[i];

		/* global hush-file */
		যদি (*file == '/') {
			সংরচনা stat st;
			FILE *f;

			যদি (stat(file, &st) != 0)
				অনুবর্ত;	/* file does not exist */

			যদি (st.st_size == 0)
				প্রত্যাগম 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			যদি (!f)
				অনুবর্ত;	/* ignore errors... */

			যাবত্ (ok == 0 && fgets(buf, পরিমাণ(buf), f)) {
				যদি (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			যদি (ok)
				প্রত্যাগম 1;	/* found username/shell */

			প্রত্যাগম 0;		/* ignore per-account files */
		}

		/* per-account setting */
		যদি (strlen(pwd->pw_dir) + strlen(file) + 2 > পরিমাণ(buf))
			অনুবর্ত;

		যদি (snprintf(buf, পরিমাণ(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			অনুবর্ত;

		যদি (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			যদি (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			যদি (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			যদি (ok)
				প্রত্যাগম 1;	/* enabled by user */
		}
		অন্যথা {
			পূর্ণাঙ্ক rc;
			rc = effective_access(buf, O_RDONLY);
			যদি (rc == 0)
				প্রত্যাগম 1;

			যদি (rc == -1 && errno == EACCES)
				প্রত্যাগম -1;
		}

	}

	প্রত্যাগম 0;
}
#ifdef TEST_PROGRAM
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	বর্ণ *name, *type;
	close_stdout_atexit();

	যদি (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	যদি (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		পূর্ণাঙ্ক i;
		বর্ণ *চাবিগুলি[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				রিক্ত};

		প্রতি (i = 0; file && চাবিগুলি[i] != রিক্ত; i++) {
			বর্ণ *মান = রিক্ত;

			econf_call(econf_get_String_মান)(file, রিক্ত, চাবিগুলি[i], &মান);
			printf ("%s: $%s: '%s'\n", argv[1], চাবিগুলি[i], মান);
		}

		যদি (file)
			econf_call(econf_freeFile)(file);

#অন্যথা
		সংরচনা item *ptr;

		প্রতি (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->মান);
#endif
		প্রত্যাগম EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	যদি (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	অন্যথা যদি (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_অঙ্ক(name, 0));
	অন্যথা যদি (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	প্রত্যাগম EXIT_SUCCESS;
}
#endif
