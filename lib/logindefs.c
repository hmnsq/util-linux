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
ساكن فراغ *logindefs_loader_معلومات = عدم;

فراغ logindefs_سيٽ_loader(فراغ (*loader)(فراغ *معلومات), فراغ *معلومات)
{
	logindefs_loader = loader;
	logindefs_loader_معلومات = معلومات;
}

#جيڪڏهن_وصف_نه_ٿيل HAVE_LIBECONF

بنية شي {
	حرف *name;		/* name of the option.  */
	حرف *قدر;		/* value of the option.  */
	حرف *رستو;		/* name of config file for this option.  */

	بنية شي *ايندڙ;	/* pointer to next option.  */
};

ساكن بنية شي *list = عدم;

فراغ free_getlogindefs_معلومات(فراغ)
{
	بنية شي *ptr;

	ptr = list;
	طالما (ptr) {
		بنية شي *tmp = ptr->ايندڙ;

		حرر(ptr->رستو);
		حرر(ptr->name);
		حرر(ptr->قدر);
		حرر(ptr);
		ptr = tmp;
	}

	list = عدم;
}

ساكن فراغ store(ثابت حرف *name, ثابت حرف *قدر, ثابت حرف *رستو)
{
	بنية شي *new = xmalloc(حجم(بنية شي));

	إذا (!name)
		abort();

	new->name = xstrdup(name);
	new->قدر = قدر && *قدر ? xstrdup(قدر) : عدم;
	new->رستو = xstrdup(رستو);
	new->ايندڙ = list;
	list = new;
}

فراغ logindefs_ڀريو_file(ثابت حرف *filename)
{
	FILE *f;
	حرف buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع;

	طالما (fgets(buf, حجم(buf), f)) {

		حرف *p, *name, *معلومات = عدم;

		إذا (*buf == '#' || *buf == '\n')
			استمر;	/* only comment or empty line */

		p = strchr(buf, '#');
		إذا (p)
			*p = '\0';
		وإلا {
			size_t n = لڙي_ڊيگهه(buf);
			إذا (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		إذا (!*buf)
			استمر;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		طالما (*name && خال_آهي((بنانشان)*name))
			name++;

		/* go to the end of the name */
		معلومات = name;
		طالما (*معلومات && !(خال_آهي((بنانشان)*معلومات) || *معلومات == '='))
			معلومات++;
		إذا (معلومات > name && *معلومات)
			*معلومات++ = '\0';

		إذا (!*name || معلومات == name)
			استمر;

		/* go to the begin of the value */
		طالما (*معلومات
		       && (خال_آهي((بنانشان)*معلومات) || *معلومات == '='
			   || *معلومات == '"'))
			معلومات++;

		/* remove space at the end of the value */
		p = معلومات + لڙي_ڊيگهه(معلومات);
		إذا (p > معلومات)
			p--;
		طالما (p > معلومات && (خال_آهي((بنانشان)*p) || *p == '"'))
			*p-- = '\0';

		store(name, معلومات, filename);
	}

	fclose(f);
}

ساكن فراغ ڀريو_defaults(فراغ)
{
	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_معلومات);
	وإلا
		logindefs_ڀريو_file(رستو_LOGINDEFS);
}

ساكن بنية شي *ڳولا(ثابت حرف *name)
{
	بنية شي *ptr;

	إذا (!list)
		ڀريو_defaults();

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr;
		ptr = ptr->ايندڙ;
	}

	أرجع عدم;
}

ساكن ثابت حرف *ڳولا_config(ثابت حرف *name)
{
	بنية شي *ptr;

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr->رستو;
		ptr = ptr->ايندڙ;
	}

	أرجع عدم;
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	بنية شي *ptr = ڳولا(name);
	أرجع ptr && ptr->قدر ? (c_strcasecmp(ptr->قدر, "yes") == 0) : dflt;
}

بنانشان ڊگهو getlogindefs_انگ(ثابت حرف *name, بنانشان ڊگهو dflt)
{
	بنية شي *ptr = ڳولا(name);
	حرف *پڇاڙي = عدم;
	بنانشان ڊگهو retval;

	إذا (!ptr || !ptr->قدر)
		أرجع dflt;

	errno = 0;
	retval = strtoul(ptr->قدر, &پڇاڙي, 0);
	إذا (پڇاڙي && *پڇاڙي == '\0' && !errno)
		أرجع retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       ڳولا_config(name), name, ptr->قدر);
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
	بنية شي *ptr = ڳولا(name);

	إذا (!ptr)
		أرجع dflt;
	إذا (!ptr->قدر)
		أرجع "";
	أرجع ptr->قدر;
}

#وإلا /* !HAVE_LIBECONF */

#ضمن "dl-econf.h"

ساكن econf_file *file = عدم;

فراغ free_getlogindefs_معلومات(فراغ)
{
	إذا (file)
		econf_call(econf_freeFile)(file);
	file = عدم;
}

ساكن فراغ ڀريو_defaults(فراغ)
{
	econf_err error;

	إذا (file != عدم)
		free_getlogindefs_معلومات();

	إذا (ul_dlopen_libeconf() == 0) {
#جيڪڏهن_وصف_ٿيل HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, عدم,
				UL_VENDORDIR_رستو, "login", "defs", "= \t", "#");
#وإلا
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_رستو, "/etc", "login", "defs", "= \t", "#");
#جيڪڏهن_پورو
		إذا (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_معلومات);

}

فراغ logindefs_ڀريو_file(ثابت حرف *filename)
{
	econf_file *file_l = عدم, *file_m = عدم;
	حرف *رستو;

	logindefs_loader = عدم; /* No recursion */

	إذا (ul_dlopen_libeconf() != 0)
		أرجع;

#إذا USE_VENDORDIR
	xasprintf(&رستو, رستو_VENDORDIR"/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, رستو, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملايو_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (رستو);
#جيڪڏهن_پورو

	xasprintf(&رستو, "/etc/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, رستو, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملايو_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} وإلا إذا (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_ملايو_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (رستو);
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	bool قدر;
	econf_err error;

	إذا (!file)
		ڀريو_defaults();

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

بنانشان ڊگهو getlogindefs_انگ(ثابت حرف *name, بنانشان ڊگهو dflt)
{
	uint64_t قدر;
	econf_err error;

	إذا (!file)
		ڀريو_defaults();

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
		ڀريو_defaults();

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
#جيڪڏهن_پورو /* !HAVE_LIBECONF */

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
		size_t sz = لڙي_ڊيگهه(name);

		إذا (لڙي_ڀيٽ(val, name, sz) == 0 && *(p + 1)) {
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
صحيح logindefs_setenv_رستو(uid_t uid)
{
	صحيح rc = 0;

	إذا (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	وإلا إذا ((rc = logindefs_setenv("PATH", "ENV_SUPATH", عدم)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", رستو_DEFPATH_پاڙ);

	أرجع rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحيح effective_access(ثابت حرف *رستو, صحيح گهڻائي)
{
	صحيح fd = open(رستو, گهڻائي);
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
	ثابت حرف *files[] = { رستو_HUSHLOGINS, رستو_HUSHLOGIN, عدم };
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
					buf[لڙي_ڊيگهه(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			إذا (ok)
				أرجع 1;	/* found username/shell */

			أرجع 0;		/* ignore per-account files */
		}

		/* per-account setting */
		إذا (لڙي_ڊيگهه(pwd->pw_dir) + لڙي_ڊيگهه(file) + 2 > حجم(buf))
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
				ٻاهر_نڪرو(EXIT_FAILURE);
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
#جيڪڏهن_وصف_ٿيل TEST_پروگرام
صحيح مکيه(صحيح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	إذا (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_ڀريو_file(argv[1]);

	إذا (argc != 4) {	/* list all */
#جيڪڏهن_وصف_ٿيل HAVE_LIBECONF
		صحيح i;
		حرف *ڪنجيون[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				عدم};

		لكل (i = 0; file && ڪنجيون[i] != عدم; i++) {
			حرف *قدر = عدم;

			econf_call(econf_get_String_قدر)(file, عدم, ڪنجيون[i], &قدر);
			لکو ("%s: $%s: '%s'\n", argv[1], ڪنجيون[i], قدر);
		}

		إذا (file)
			econf_call(econf_freeFile)(file);

#وإلا
		بنية شي *ptr;

		لكل (ptr = list; ptr; ptr = ptr->ايندڙ)
			لکو("%s: $%s: '%s'\n", ptr->رستو, ptr->name,
			       ptr->قدر);
#جيڪڏهن_پورو
		أرجع EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	إذا (strcmp(type, "str") == 0)
		لکو("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	وإلا إذا (strcmp(type, "num") == 0)
		لکو("$%s: '%lu'\n", name, getlogindefs_انگ(name, 0));
	وإلا إذا (strcmp(type, "bool") == 0)
		لکو("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	أرجع EXIT_SUCCESS;
}
#جيڪڏهن_پورو
