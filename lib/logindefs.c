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

فراغ logindefs_سېټ_loader(فراغ (*loader)(فراغ *معلومات), فراغ *معلومات)
{
	logindefs_loader = loader;
	logindefs_loader_معلومات = معلومات;
}

#که_نه_تعریف_شوی HAVE_LIBECONF

بنية توکی {
	حرف *name;		/* name of the option.  */
	حرف *ارزښت;		/* value of the option.  */
	حرف *لار;		/* name of config file for this option.  */

	بنية توکی *بل;	/* pointer to next option.  */
};

ساكن بنية توکی *list = عدم;

فراغ free_getlogindefs_معلومات(فراغ)
{
	بنية توکی *ptr;

	ptr = list;
	طالما (ptr) {
		بنية توکی *tmp = ptr->بل;

		حرر(ptr->لار);
		حرر(ptr->name);
		حرر(ptr->ارزښت);
		حرر(ptr);
		ptr = tmp;
	}

	list = عدم;
}

ساكن فراغ store(ثابت حرف *name, ثابت حرف *ارزښت, ثابت حرف *لار)
{
	بنية توکی *new = xmalloc(حجم(بنية توکی));

	إذا (!name)
		abort();

	new->name = xstrdup(name);
	new->ارزښت = ارزښت && *ارزښت ? xstrdup(ارزښت) : عدم;
	new->لار = xstrdup(لار);
	new->بل = list;
	list = new;
}

فراغ logindefs_بارول_file(ثابت حرف *filename)
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
			size_t n = مزي_اوږدوالی(buf);
			إذا (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		إذا (!*buf)
			استمر;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		طالما (*name && که_تشه((بینښه)*name))
			name++;

		/* go to the end of the name */
		معلومات = name;
		طالما (*معلومات && !(که_تشه((بینښه)*معلومات) || *معلومات == '='))
			معلومات++;
		إذا (معلومات > name && *معلومات)
			*معلومات++ = '\0';

		إذا (!*name || معلومات == name)
			استمر;

		/* go to the begin of the value */
		طالما (*معلومات
		       && (که_تشه((بینښه)*معلومات) || *معلومات == '='
			   || *معلومات == '"'))
			معلومات++;

		/* remove space at the end of the value */
		p = معلومات + مزي_اوږدوالی(معلومات);
		إذا (p > معلومات)
			p--;
		طالما (p > معلومات && (که_تشه((بینښه)*p) || *p == '"'))
			*p-- = '\0';

		store(name, معلومات, filename);
	}

	fclose(f);
}

ساكن فراغ بارول_defaults(فراغ)
{
	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_معلومات);
	وإلا
		logindefs_بارول_file(لار_LOGINDEFS);
}

ساكن بنية توکی *پلټنه(ثابت حرف *name)
{
	بنية توکی *ptr;

	إذا (!list)
		بارول_defaults();

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr;
		ptr = ptr->بل;
	}

	أرجع عدم;
}

ساكن ثابت حرف *پلټنه_config(ثابت حرف *name)
{
	بنية توکی *ptr;

	ptr = list;
	طالما (ptr != عدم) {
		إذا (c_strcasecmp(name, ptr->name) == 0)
			أرجع ptr->لار;
		ptr = ptr->بل;
	}

	أرجع عدم;
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	بنية توکی *ptr = پلټنه(name);
	أرجع ptr && ptr->ارزښت ? (c_strcasecmp(ptr->ارزښت, "yes") == 0) : dflt;
}

بینښه اوږد getlogindefs_عدد(ثابت حرف *name, بینښه اوږد dflt)
{
	بنية توکی *ptr = پلټنه(name);
	حرف *پای_ټکی = عدم;
	بینښه اوږد retval;

	إذا (!ptr || !ptr->ارزښت)
		أرجع dflt;

	errno = 0;
	retval = strtoul(ptr->ارزښت, &پای_ټکی, 0);
	إذا (پای_ټکی && *پای_ټکی == '\0' && !errno)
		أرجع retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       پلټنه_config(name), name, ptr->ارزښت);
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
	بنية توکی *ptr = پلټنه(name);

	إذا (!ptr)
		أرجع dflt;
	إذا (!ptr->ارزښت)
		أرجع "";
	أرجع ptr->ارزښت;
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

ساكن فراغ بارول_defaults(فراغ)
{
	econf_err error;

	إذا (file != عدم)
		free_getlogindefs_معلومات();

	إذا (ul_dlopen_libeconf() == 0) {
#که_تعریف_شوی HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, عدم,
				UL_VENDORDIR_لار, "login", "defs", "= \t", "#");
#وإلا
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_لار, "/etc", "login", "defs", "= \t", "#");
#د_که_پای
		إذا (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	إذا (logindefs_loader)
		logindefs_loader(logindefs_loader_معلومات);

}

فراغ logindefs_بارول_file(ثابت حرف *filename)
{
	econf_file *file_l = عدم, *file_m = عدم;
	حرف *لار;

	logindefs_loader = عدم; /* No recursion */

	إذا (ul_dlopen_libeconf() != 0)
		أرجع;

#إذا USE_VENDORDIR
	xasprintf(&لار, لار_VENDORDIR"/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, لار, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_یوځای_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (لار);
#د_که_پای

	xasprintf(&لار, "/etc/%s", filename);

	إذا (!econf_call(econf_readFile)(&file_l, لار, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_یوځای_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} وإلا إذا (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		إذا (file == عدم)
			file = file_l;
		وإلا إذا (!econf_call(econf_یوځای_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	حرر (لار);
}

صحيح getlogindefs_bool(ثابت حرف *name, صحيح dflt)
{
	bool ارزښت;
	econf_err error;

	إذا (!file)
		بارول_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_Bool_ارزښت)(file, عدم, name, &ارزښت))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع ارزښت;
}

بینښه اوږد getlogindefs_عدد(ثابت حرف *name, بینښه اوږد dflt)
{
	uint64_t ارزښت;
	econf_err error;

	إذا (!file)
		بارول_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_UInt64_ارزښت)(file, عدم, name, &ارزښت))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	أرجع ارزښت;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ثابت حرف *getlogindefs_str(ثابت حرف *name, ثابت حرف *dflt)
{
	حرف *ارزښت;
	econf_err error;

	إذا (!file)
		بارول_defaults();

	إذا (!file)
		أرجع dflt;

	إذا ((error = econf_call(econf_get_String_ارزښت)(file, عدم, name, &ارزښت))) {
		إذا (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		أرجع dflt;
	}
	إذا (ارزښت)
		أرجع ارزښت;

	أرجع xstrdup("");
}
#د_که_پای /* !HAVE_LIBECONF */

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
		size_t sz = مزي_اوږدوالی(name);

		إذا (مزي_پرتله(val, name, sz) == 0 && *(p + 1)) {
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
صحيح logindefs_setenv_لار(uid_t uid)
{
	صحيح rc = 0;

	إذا (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	وإلا إذا ((rc = logindefs_setenv("PATH", "ENV_SUPATH", عدم)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", لار_DEFPATH_ریښه);

	أرجع rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
صحيح effective_access(ثابت حرف *لار, صحيح ډېرپېښ)
{
	صحيح fd = open(لار, ډېرپېښ);
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
	ثابت حرف *files[] = { لار_HUSHLOGINS, لار_HUSHLOGIN, عدم };
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
					buf[مزي_اوږدوالی(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			إذا (ok)
				أرجع 1;	/* found username/shell */

			أرجع 0;		/* ignore per-account files */
		}

		/* per-account setting */
		إذا (مزي_اوږدوالی(pwd->pw_dir) + مزي_اوږدوالی(file) + 2 > حجم(buf))
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
				وتل(EXIT_FAILURE);
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
#که_تعریف_شوی TEST_پروګرام
صحيح اصلي(صحيح argc, حرف *argv[])
{
	حرف *name, *type;
	close_stdout_atexit();

	إذا (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_بارول_file(argv[1]);

	إذا (argc != 4) {	/* list all */
#که_تعریف_شوی HAVE_LIBECONF
		صحيح i;
		حرف *کلیګانې[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				عدم};

		لكل (i = 0; file && کلیګانې[i] != عدم; i++) {
			حرف *ارزښت = عدم;

			econf_call(econf_get_String_ارزښت)(file, عدم, کلیګانې[i], &ارزښت);
			چاپ_کړه ("%s: $%s: '%s'\n", argv[1], کلیګانې[i], ارزښت);
		}

		إذا (file)
			econf_call(econf_freeFile)(file);

#وإلا
		بنية توکی *ptr;

		لكل (ptr = list; ptr; ptr = ptr->بل)
			چاپ_کړه("%s: $%s: '%s'\n", ptr->لار, ptr->name,
			       ptr->ارزښت);
#د_که_پای
		أرجع EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	إذا (strcmp(type, "str") == 0)
		چاپ_کړه("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	وإلا إذا (strcmp(type, "num") == 0)
		چاپ_کړه("$%s: '%lu'\n", name, getlogindefs_عدد(name, 0));
	وإلا إذا (strcmp(type, "bool") == 0)
		چاپ_کړه("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	أرجع EXIT_SUCCESS;
}
#د_که_پای
