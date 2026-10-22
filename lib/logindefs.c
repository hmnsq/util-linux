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
#ضمن <assert.h>
#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <limits.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/syslog.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <pwd.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "closestream.h"
#ضمن "logindefs.h"
#ضمن "nls.h"
#ضمن "pathnames.h"
#ضمن "xalloc.h"


ساكن فراغ (*logindefs_loader)(فراغ *) = عدم;
ساكن فراغ *logindefs_loader_انکڑے = عدم;

فراغ logindefs_سیٹ_loader(فراغ (*loader)(فراغ *انکڑے), فراغ *انکڑے)
{
	logindefs_loader = loader;
	logindefs_loader_انکڑے = انکڑے;
}

#جے_بنا_تعریف HAVE_LIBECONF

بنية شے {
	حرف *name;		/* name of the option.  */
	حرف *قدر;		/* value of the option.  */
	حرف *رستہ;		/* name of config file for this option.  */

	بنية شے *اگلا;	/* pointer to next option.  */
};

ساكن بنية شے *list = عدم;

فراغ free_getlogindefs_انکڑے(فراغ)
{
	بنية شے *ptr;

	ptr = list;
	طالما (ptr) {
		بنية شے *tmp = ptr->اگلا;

		حرر(ptr->رستہ);
		حرر(ptr->name);
		حرر(ptr->قدر);
		حرر(ptr);
		ptr = tmp;
	}

	list = عدم;
}

ساكن فراغ store(ثابت حرف *name, ثابت حرف *قدر, ثابت حرف *رستہ)
{
	بنية شے *new = xmalloc(حجم(بنية شے));

	إذا (!name)
		abort();

	new->name = xstrdup(name);
	new->قدر = قدر && *قدر ? xstrdup(قدر) : عدم;
	new->رستہ = xstrdup(رستہ);
	new->اگلا = list;
	list = new;
}

فراغ logindefs_لدو_file(ثابت حرف *filename)
{
	FILE *f;
	حرف buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع;

	طالما (fgets(buf, حجم(buf), f)) {

		حرف *p, *name, *انکڑے = عدم;

		إذا (*buf == '#' || *buf == '\n')
			استمر;	/* only comment or empty line */

		p = strchr(buf, '#');
		إذا (p)
			*p = '\0';
		وإلا {
			size_t n = لڑی_لمبائی(buf);
			إذا (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		إذا (!*buf)
			استمر;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		طالما (*name && خلا_اے((بنانشانی)*name))
			name++;

		/* go to the end of the name */
		انکڑے = name;
		طالما (*انکڑے && !(خلا_اے((بنانشانی)*انکڑے) || *انکڑے == '='))
			انکڑے++;
		إذا (انکڑے > name && *انکڑے)
			*انکڑے++ = '\0';

		إذا (!*name || انکڑے == name)
			استمر;

		/* go to the begin of the value */
		طالما (*انکڑے
		       && (خلا_اے((بنانشانی)*انکڑے) || *انکڑے == '='
			   || *انکڑے == '"'))
			انکڑے++;

		/* remove space at the end of the value */
		p = انکڑے + لڑی_لمبائی(انکڑے);
		إذا (p > انکڑے)
			p--;
		طالما (p > انکڑے && (خلا_اے((بنانشانی)*p) || *p == '"'))
			*p-- = '\0';

		store(name, انکڑے, filename);
	}

	fclose(f);
}

ساكن فراغ لدو_defaults(فراغ)
{
	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_انکڑے);
	وإلا
		logindefs_لدو_file(رستہ_LOGINDEFS);
}

ساكن بنية شے *تلاش(ثابت حرف *name)
{
	بنية شے *ptr;

	إذا (!list)
		لدو_defaults();

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr;
		ptr = ptr->اگلا;
	}

	أرجع عدم;
}

ساكن ثابت حرف *تلاش_config(ثابت حرف *name)
{
	بنية شے *ptr;

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr->رستہ;
		ptr = ptr->اگلا;
	}

	أرجع عدم;
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	بنية شے *ptr = تلاش(name);
	أرجع ptr && ptr->قدر ? (c_strcasecmp(ptr->قدر, "yes") == 0) : dflt;
}

بنانشانی لما getlogindefs_عدد(ثابت حرف *name, بنانشانی لما dflt)
{
	بنية شے *ptr = تلاش(name);
	حرف *سرا = عدم;
	بنانشانی لما retval;

	إذا (!ptr || !ptr->قدر)
		أرجع dflt;

	errno = 0;
	retval = strtoul(ptr->قدر, &سرا, 0);
	إذا (سرا && *سرا == '\0' && !errno)
		أرجع retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       تلاش_config(name), name, ptr->قدر);
	أرجع dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	بنية شے *ptr = تلاش(name);

	إذا (!ptr)
		أرجع dflt;
	إذا (!ptr->قدر)
		أرجع "";
	أرجع ptr->قدر;
}

#وإلا /* !HAVE_LIBECONF */

#ضمن "dl-econf.h"

ساكن econf_file *file = عدم;

فراغ free_getlogindefs_انکڑے(فراغ)
{
	إذا (file)
		econf_call(econf_freeFile)(file);
	file = عدم;
}

ساكن فراغ لدو_defaults(فراغ)
{
	econf_err error;

	إذا (file != عدم)
		free_getlogindefs_انکڑے();

	إذا (ul_dlopen_libeconf() == 0) {
#جے_تعریف_شدہ HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, عدم,
				UL_VENDORDIR_رستہ, "login", "defs", "= \t", "#");
#وإلا
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_رستہ, "/etc", "login", "defs", "= \t", "#");
#جے_انت
		إذا (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_انکڑے);

}

فراغ logindefs_لدو_file(ثابت حرف *filename)
{
	econf_file *file_l = عدم, *file_m = عدم;
	حرف *رستہ;

	logindefs_loader = عدم; /* No recursion */

	إذا (ul_dlopen_libeconf() != 0)
		أرجع;

#إذا USE_VENDORDIR
	xasprintf(&رستہ, رستہ_VENDORDIR"/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, رستہ, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (رستہ);
#جے_انت

	xasprintf(&رستہ, "/etc/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, رستہ, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} وإلا إذا (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (رستہ);
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	bool قدر;
	econf_err error;

	إذا (!file)
		لدو_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_Bool_قدر)(file, عدم, name, &قدر))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع قدر;
}

بنانشانی لما getlogindefs_عدد(ثابت حرف *name, بنانشانی لما dflt)
{
	uint64_t قدر;
	econf_err error;

	إذا (!file)
		لدو_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_UInt64_قدر)(file, عدم, name, &قدر))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع قدر;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	حرف *قدر;
	econf_err error;

	إذا (!file)
		لدو_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_String_قدر)(file, عدم, name, &قدر))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	إذا (قدر)
		أرجع قدر;

	أرجع xstrdup("");
}
#جے_انت /* !HAVE_LIBECONF */

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
صحيح logindefs_setenv(ثابت حرف *name, ثابت حرف *conf, ثابت حرف *dflt)
{
	ثابت حرف *val = getlogindefs_str(conf, dflt);
	ثابت حرف *p;

	إذا (!val)
		أرجع -1;

	p = strchr(val, '=');
	إذا (p) {
		size_t sz = لڑی_لمبائی(name);

		إذا (لڑی_مقابلہ(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			إذا (*val == '"')
				val++;
			إذا (!*val)
				val = dflt;
		}
	}

	أرجع val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
صحيح logindefs_setenv_رستہ(uid_t uid)
{
	صحيح rc = 0;

	إذا (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	وإلا إذا ((rc = logindefs_setenv("PATH", "ENV_SUPATH", عدم)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", رستہ_DEFPATH_جڑ);

	أرجع rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحيح effective_access(ثابت حرف *رستہ, صحيح منوال)
{
	صحيح fd = open(رستہ, منوال);
	إذا (fd != -1)
		close(fd);
	أرجع fd == -1 ? -1 : 0;
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

صحيح get_hushlogin_حالت(بنية passwd *pwd, ثابت حرف *override_home, صحيح force_check)
{
	ثابت حرف *files[] = { رستہ_HUSHLOGINS, رستہ_HUSHLOGIN, عدم };
	ثابت حرف *file;
	حرف buf[BUFSIZ];
	صحيح i;

	file = getlogindefs_str("HUSHLOGIN_FILE", عدم);
	إذا (file) {
		إذا (!*file)
			أرجع 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = عدم;
	}

	لكل (i = 0; files[i]; i++) {
		صحيح ok = 0;

		file = files[i];

		/* global hush-file */
		إذا (*file == '/') {
			بنية stat st;
			FILE *f;

			إذا (stat(file, &st) != 0)
				استمر;	/* file does not exist */

			إذا (st.st_size == 0)
				أرجع 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			إذا (!f)
				استمر;	/* ignore errors... */

			طالما (ok == 0 && fgets(buf, حجم(buf), f)) {
				إذا (buf[0] != '\0')
					buf[لڑی_لمبائی(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			إذا (ok)
				أرجع 1;	/* found username/shell */

			أرجع 0;		/* ignore per-account files */
		}

		/* per-account setting */
		إذا (لڑی_لمبائی(pwd->pw_dir) + لڑی_لمبائی(file) + 2 > حجم(buf))
			استمر;

		إذا (snprintf(buf, حجم(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			استمر;

		إذا (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			إذا (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			إذا (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				باہر_نکلو(EXIT_FAILURE);
			}
			إذا (ok)
				أرجع 1;	/* enabled by user */
		}
		وإلا {
			صحيح rc;
			rc = effective_access(buf, O_RDONLY);
			إذا (rc == 0)
				أرجع 1;

			إذا (rc == -1 && errno == EACCES)
				أرجع -1;
		}

	}

	أرجع 0;
}
#جے_تعریف_شدہ TEST_پروگرام
صحيح مکھ(صحيح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	إذا (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_لدو_file(argv[1]);

	إذا (argc != 4) {	/* list all */
#جے_تعریف_شدہ HAVE_LIBECONF
		صحيح i;
		حرف *کلیدیں[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				عدم};

		لكل (i = 0; file && کلیدیں[i] != عدم; i++) {
			حرف *قدر = عدم;

			econf_call(econf_get_String_قدر)(file, عدم, کلیدیں[i], &قدر);
			لکھو ("%s: $%s: '%s'\n", argv[1], کلیدیں[i], قدر);
		}

		إذا (file)
			econf_call(econf_freeFile)(file);

#وإلا
		بنية شے *ptr;

		لكل (ptr = list; ptr; ptr = ptr->اگلا)
			لکھو("%s: $%s: '%s'\n", ptr->رستہ, ptr->name,
			       ptr->قدر);
#جے_انت
		أرجع EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	إذا (strcmp(type, "str") == 0)
		لکھو("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	وإلا إذا (strcmp(type, "num") == 0)
		لکھو("$%s: '%lu'\n", name, getlogindefs_عدد(name, 0));
	وإلا إذا (strcmp(type, "bool") == 0)
		لکھو("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	أرجع EXIT_SUCCESS;
}
#جے_انت
