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
#شامل <assert.h>
#شامل <ctype.h>
#شامل <errno.h>
#شامل <limits.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/syslog.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <pwd.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "closestream.h"
#شامل "logindefs.h"
#شامل "nls.h"
#شامل "pathnames.h"
#شامل "xalloc.h"


ساکن خالی (*logindefs_loader)(خالی *) = تہی;
ساکن خالی *logindefs_loader_ڈیٹا = تہی;

خالی logindefs_سیٹ_loader(خالی (*loader)(خالی *ڈیٹا), خالی *ڈیٹا)
{
	logindefs_loader = loader;
	logindefs_loader_ڈیٹا = ڈیٹا;
}

#اگر_غیر_تعریف_شدہ HAVE_LIBECONF

ساخت شے {
	حرف *name;		/* name of the option.  */
	حرف *قدر;		/* value of the option.  */
	حرف *راستہ;		/* name of config file for this option.  */

	ساخت شے *اگلا;	/* pointer to next option.  */
};

ساکن ساخت شے *list = تہی;

خالی free_getlogindefs_ڈیٹا(خالی)
{
	ساخت شے *ptr;

	ptr = list;
	جب (ptr) {
		ساخت شے *tmp = ptr->اگلا;

		آزاد(ptr->راستہ);
		آزاد(ptr->name);
		آزاد(ptr->قدر);
		آزاد(ptr);
		ptr = tmp;
	}

	list = تہی;
}

ساکن خالی store(ثابت حرف *name, ثابت حرف *قدر, ثابت حرف *راستہ)
{
	ساخت شے *new = xmalloc(حجم(ساخت شے));

	اگر (!name)
		abort();

	new->name = xstrdup(name);
	new->قدر = قدر && *قدر ? xstrdup(قدر) : تہی;
	new->راستہ = xstrdup(راستہ);
	new->اگلا = list;
	list = new;
}

خالی logindefs_بھرو_file(ثابت حرف *filename)
{
	FILE *f;
	حرف buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	اگر (!f)
		واپس;

	جب (fgets(buf, حجم(buf), f)) {

		حرف *p, *name, *ڈیٹا = تہی;

		اگر (*buf == '#' || *buf == '\n')
			جاری;	/* only comment or empty line */

		p = strchr(buf, '#');
		اگر (p)
			*p = '\0';
		ورنہ {
			size_t n = لڑی_لمبائی(buf);
			اگر (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		اگر (!*buf)
			جاری;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		جب (*name && خلا_ہے((غیرعلامتی)*name))
			name++;

		/* go to the end of the name */
		ڈیٹا = name;
		جب (*ڈیٹا && !(خلا_ہے((غیرعلامتی)*ڈیٹا) || *ڈیٹا == '='))
			ڈیٹا++;
		اگر (ڈیٹا > name && *ڈیٹا)
			*ڈیٹا++ = '\0';

		اگر (!*name || ڈیٹا == name)
			جاری;

		/* go to the begin of the value */
		جب (*ڈیٹا
		       && (خلا_ہے((غیرعلامتی)*ڈیٹا) || *ڈیٹا == '='
			   || *ڈیٹا == '"'))
			ڈیٹا++;

		/* remove space at the end of the value */
		p = ڈیٹا + لڑی_لمبائی(ڈیٹا);
		اگر (p > ڈیٹا)
			p--;
		جب (p > ڈیٹا && (خلا_ہے((غیرعلامتی)*p) || *p == '"'))
			*p-- = '\0';

		store(name, ڈیٹا, filename);
	}

	fclose(f);
}

ساکن خالی بھرو_defaults(خالی)
{
	اگر (logindefs_loader)
		logindefs_loader(logindefs_loader_ڈیٹا);
	ورنہ
		logindefs_بھرو_file(راستہ_LOGINDEFS);
}

ساکن ساخت شے *تلاش(ثابت حرف *name)
{
	ساخت شے *ptr;

	اگر (!list)
		بھرو_defaults();

	ptr = list;
	جب (ptr != تہی) {
		اگر (c_strcasecmp(name, ptr->name) == 0)
			واپس ptr;
		ptr = ptr->اگلا;
	}

	واپس تہی;
}

ساکن ثابت حرف *تلاش_config(ثابت حرف *name)
{
	ساخت شے *ptr;

	ptr = list;
	جب (ptr != تہی) {
		اگر (c_strcasecmp(name, ptr->name) == 0)
			واپس ptr->راستہ;
		ptr = ptr->اگلا;
	}

	واپس تہی;
}

صحیح getlogindefs_bool(ثابت حرف *name, صحیح dflt)
{
	ساخت شے *ptr = تلاش(name);
	واپس ptr && ptr->قدر ? (c_strcasecmp(ptr->قدر, "yes") == 0) : dflt;
}

غیرعلامتی طویل getlogindefs_عدد(ثابت حرف *name, غیرعلامتی طویل dflt)
{
	ساخت شے *ptr = تلاش(name);
	حرف *انتہا = تہی;
	غیرعلامتی طویل retval;

	اگر (!ptr || !ptr->قدر)
		واپس dflt;

	errno = 0;
	retval = strtoul(ptr->قدر, &انتہا, 0);
	اگر (انتہا && *انتہا == '\0' && !errno)
		واپس retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       تلاش_config(name), name, ptr->قدر);
	واپس dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	ساخت شے *ptr = تلاش(name);

	اگر (!ptr)
		واپس dflt;
	اگر (!ptr->قدر)
		واپس "";
	واپس ptr->قدر;
}

#ورنہ /* !HAVE_LIBECONF */

#شامل "dl-econf.h"

ساکن econf_file *file = تہی;

خالی free_getlogindefs_ڈیٹا(خالی)
{
	اگر (file)
		econf_call(econf_freeFile)(file);
	file = تہی;
}

ساکن خالی بھرو_defaults(خالی)
{
	econf_err error;

	اگر (file != تہی)
		free_getlogindefs_ڈیٹا();

	اگر (ul_dlopen_libeconf() == 0) {
#اگر_تعریف_شدہ HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, تہی,
				UL_VENDORDIR_راستہ, "login", "defs", "= \t", "#");
#ورنہ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_راستہ, "/etc", "login", "defs", "= \t", "#");
#اگر_ختم
		اگر (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	اگر (logindefs_loader)
		logindefs_loader(logindefs_loader_ڈیٹا);

}

خالی logindefs_بھرو_file(ثابت حرف *filename)
{
	econf_file *file_l = تہی, *file_m = تہی;
	حرف *راستہ;

	logindefs_loader = تہی; /* No recursion */

	اگر (ul_dlopen_libeconf() != 0)
		واپس;

#اگر USE_VENDORDIR
	xasprintf(&راستہ, راستہ_VENDORDIR"/%s", filename);

	اگر (!econf_call(econf_readFile)(&file_l, راستہ, "= \t", "#")) {
		اگر (file == تہی)
			file = file_l;
		ورنہ اگر (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	آزاد (راستہ);
#اگر_ختم

	xasprintf(&راستہ, "/etc/%s", filename);

	اگر (!econf_call(econf_readFile)(&file_l, راستہ, "= \t", "#")) {
		اگر (file == تہی)
			file = file_l;
		ورنہ اگر (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ورنہ اگر (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		اگر (file == تہی)
			file = file_l;
		ورنہ اگر (!econf_call(econf_ملاؤ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	آزاد (راستہ);
}

صحیح getlogindefs_bool(ثابت حرف *name, صحیح dflt)
{
	bool قدر;
	econf_err error;

	اگر (!file)
		بھرو_defaults();

	اگر (!file)
		واپس dflt;

	اگر ((error = econf_call(econf_get_Bool_قدر)(file, تہی, name, &قدر))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		واپس dflt;
	}
	واپس قدر;
}

غیرعلامتی طویل getlogindefs_عدد(ثابت حرف *name, غیرعلامتی طویل dflt)
{
	uint64_t قدر;
	econf_err error;

	اگر (!file)
		بھرو_defaults();

	اگر (!file)
		واپس dflt;

	اگر ((error = econf_call(econf_get_UInt64_قدر)(file, تہی, name, &قدر))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		واپس dflt;
	}
	واپس قدر;
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

	اگر (!file)
		بھرو_defaults();

	اگر (!file)
		واپس dflt;

	اگر ((error = econf_call(econf_get_String_قدر)(file, تہی, name, &قدر))) {
		اگر (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		واپس dflt;
	}
	اگر (قدر)
		واپس قدر;

	واپس xstrdup("");
}
#اگر_ختم /* !HAVE_LIBECONF */

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
صحیح logindefs_setenv(ثابت حرف *name, ثابت حرف *conf, ثابت حرف *dflt)
{
	ثابت حرف *val = getlogindefs_str(conf, dflt);
	ثابت حرف *p;

	اگر (!val)
		واپس -1;

	p = strchr(val, '=');
	اگر (p) {
		size_t sz = لڑی_لمبائی(name);

		اگر (لڑی_موازنہ(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			اگر (*val == '"')
				val++;
			اگر (!*val)
				val = dflt;
		}
	}

	واپس val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
صحیح logindefs_setenv_راستہ(uid_t uid)
{
	صحیح rc = 0;

	اگر (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ورنہ اگر ((rc = logindefs_setenv("PATH", "ENV_SUPATH", تہی)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", راستہ_DEFPATH_جڑ);

	واپس rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحیح effective_access(ثابت حرف *راستہ, صحیح منوال)
{
	صحیح fd = open(راستہ, منوال);
	اگر (fd != -1)
		close(fd);
	واپس fd == -1 ? -1 : 0;
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

صحیح get_hushlogin_حالت(ساخت passwd *pwd, ثابت حرف *override_home, صحیح force_check)
{
	ثابت حرف *files[] = { راستہ_HUSHLOGINS, راستہ_HUSHLOGIN, تہی };
	ثابت حرف *file;
	حرف buf[BUFSIZ];
	صحیح i;

	file = getlogindefs_str("HUSHLOGIN_FILE", تہی);
	اگر (file) {
		اگر (!*file)
			واپس 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = تہی;
	}

	برائے (i = 0; files[i]; i++) {
		صحیح ok = 0;

		file = files[i];

		/* global hush-file */
		اگر (*file == '/') {
			ساخت stat st;
			FILE *f;

			اگر (stat(file, &st) != 0)
				جاری;	/* file does not exist */

			اگر (st.st_size == 0)
				واپس 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			اگر (!f)
				جاری;	/* ignore errors... */

			جب (ok == 0 && fgets(buf, حجم(buf), f)) {
				اگر (buf[0] != '\0')
					buf[لڑی_لمبائی(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			اگر (ok)
				واپس 1;	/* found username/shell */

			واپس 0;		/* ignore per-account files */
		}

		/* per-account setting */
		اگر (لڑی_لمبائی(pwd->pw_dir) + لڑی_لمبائی(file) + 2 > حجم(buf))
			جاری;

		اگر (snprintf(buf, حجم(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			جاری;

		اگر (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			اگر (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			اگر (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				باہر_نکلو(EXIT_FAILURE);
			}
			اگر (ok)
				واپس 1;	/* enabled by user */
		}
		ورنہ {
			صحیح rc;
			rc = effective_access(buf, O_RDONLY);
			اگر (rc == 0)
				واپس 1;

			اگر (rc == -1 && errno == EACCES)
				واپس -1;
		}

	}

	واپس 0;
}
#اگر_تعریف_شدہ TEST_پروگرام
صحیح مین(صحیح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	اگر (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_بھرو_file(argv[1]);

	اگر (argc != 4) {	/* list all */
#اگر_تعریف_شدہ HAVE_LIBECONF
		صحیح i;
		حرف *کلیدیں[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				تہی};

		برائے (i = 0; file && کلیدیں[i] != تہی; i++) {
			حرف *قدر = تہی;

			econf_call(econf_get_String_قدر)(file, تہی, کلیدیں[i], &قدر);
			لکھو ("%s: $%s: '%s'\n", argv[1], کلیدیں[i], قدر);
		}

		اگر (file)
			econf_call(econf_freeFile)(file);

#ورنہ
		ساخت شے *ptr;

		برائے (ptr = list; ptr; ptr = ptr->اگلا)
			لکھو("%s: $%s: '%s'\n", ptr->راستہ, ptr->name,
			       ptr->قدر);
#اگر_ختم
		واپس EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	اگر (strcmp(type, "str") == 0)
		لکھو("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ورنہ اگر (strcmp(type, "num") == 0)
		لکھو("$%s: '%lu'\n", name, getlogindefs_عدد(name, 0));
	ورنہ اگر (strcmp(type, "bool") == 0)
		لکھو("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	واپس EXIT_SUCCESS;
}
#اگر_ختم
