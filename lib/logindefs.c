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
#अन्तर्भाव <assert.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/syslog.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <pwd.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "closestream.h"
#अन्तर्भाव "logindefs.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "xalloc.h"


स्थिर शून्य (*logindefs_loader)(शून्य *) = रिक्त;
स्थिर शून्य *logindefs_loader_data = रिक्त;

शून्य logindefs_set_loader(शून्य (*loader)(शून्य *data), शून्य *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

संरचना item {
	वर्ण *name;		/* name of the option.  */
	वर्ण *मूल्य;		/* value of the option.  */
	वर्ण *path;		/* name of config file for this option.  */

	संरचना item *next;	/* pointer to next option.  */
};

स्थिर संरचना item *list = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	संरचना item *ptr;

	ptr = list;
	यावत् (ptr) {
		संरचना item *tmp = ptr->next;

		मुक्त(ptr->path);
		मुक्त(ptr->name);
		मुक्त(ptr->मूल्य);
		मुक्त(ptr);
		ptr = tmp;
	}

	list = रिक्त;
}

स्थिर शून्य store(ध्रुव वर्ण *name, ध्रुव वर्ण *मूल्य, ध्रुव वर्ण *path)
{
	संरचना item *new = xmalloc(परिमाण(संरचना item));

	यदि (!name)
		पात();

	new->name = xstrdup(name);
	new->मूल्य = मूल्य && *मूल्य ? xstrdup(मूल्य) : रिक्त;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

शून्य logindefs_load_file(ध्रुव वर्ण *filename)
{
	खाता *f;
	वर्ण buf[बफ_मान];

	f = ख_खोलो(filename, "r" UL_CLOEXECSTR);
	यदि (!f)
		प्रत्यागम;

	यावत् (ख_माला_लो(buf, परिमाण(buf), f)) {

		वर्ण *p, *name, *data = रिक्त;

		यदि (*buf == '#' || *buf == '\n')
			अनुवर्त;	/* only comment or empty line */

		p = म_अक्षर(buf, '#');
		यदि (p)
			*p = '\0';
		अन्यथा {
			माप_प्रकार n = म_माप(buf);
			यदि (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		यदि (!*buf)
			अनुवर्त;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		यावत् (*name && है_खाली((अचिन्हित)*name))
			name++;

		/* go to the end of the name */
		data = name;
		यावत् (*data && !(है_खाली((अचिन्हित)*data) || *data == '='))
			data++;
		यदि (data > name && *data)
			*data++ = '\0';

		यदि (!*name || data == name)
			अनुवर्त;

		/* go to the begin of the value */
		यावत् (*data
		       && (है_खाली((अचिन्हित)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + म_माप(data);
		यदि (p > data)
			p--;
		यावत् (p > data && (है_खाली((अचिन्हित)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	ख_बंद(f);
}

स्थिर शून्य load_defaults(शून्य)
{
	यदि (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	अन्यथा
		logindefs_load_file(_PATH_LOGINDEFS);
}

स्थिर संरचना item *खोज(ध्रुव वर्ण *name)
{
	संरचना item *ptr;

	यदि (!list)
		load_defaults();

	ptr = list;
	यावत् (ptr != रिक्त) {
		यदि (c_strcasecmp(name, ptr->name) == 0)
			प्रत्यागम ptr;
		ptr = ptr->next;
	}

	प्रत्यागम रिक्त;
}

स्थिर ध्रुव वर्ण *खोज_config(ध्रुव वर्ण *name)
{
	संरचना item *ptr;

	ptr = list;
	यावत् (ptr != रिक्त) {
		यदि (c_strcasecmp(name, ptr->name) == 0)
			प्रत्यागम ptr->path;
		ptr = ptr->next;
	}

	प्रत्यागम रिक्त;
}

पूर्णाङ्क getlogindefs_bool(ध्रुव वर्ण *name, पूर्णाङ्क dflt)
{
	संरचना item *ptr = खोज(name);
	प्रत्यागम ptr && ptr->मूल्य ? (c_strcasecmp(ptr->मूल्य, "yes") == 0) : dflt;
}

अचिन्हित दीर्घ getlogindefs_अंक(ध्रुव वर्ण *name, अचिन्हित दीर्घ dflt)
{
	संरचना item *ptr = खोज(name);
	वर्ण *end = रिक्त;
	अचिन्हित दीर्घ retval;

	यदि (!ptr || !ptr->मूल्य)
		प्रत्यागम dflt;

	त्रुटि_सं = 0;
	retval = म_से_अदीर्घ(ptr->मूल्य, &end, 0);
	यदि (end && *end == '\0' && !त्रुटि_सं)
		प्रत्यागम retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       खोज_config(name), name, ptr->मूल्य);
	प्रत्यागम dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ध्रुव वर्ण *getlogindefs_str(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	संरचना item *ptr = खोज(name);

	यदि (!ptr)
		प्रत्यागम dflt;
	यदि (!ptr->मूल्य)
		प्रत्यागम "";
	प्रत्यागम ptr->मूल्य;
}

#अन्यथा /* !HAVE_LIBECONF */

#अन्तर्भाव "dl-econf.h"

स्थिर econf_file *file = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	यदि (file)
		econf_call(econf_freeFile)(file);
	file = रिक्त;
}

स्थिर शून्य load_defaults(शून्य)
{
	econf_err error;

	यदि (file != रिक्त)
		free_getlogindefs_data();

	यदि (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, रिक्त,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#अन्यथा
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		यदि (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	यदि (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

शून्य logindefs_load_file(ध्रुव वर्ण *filename)
{
	econf_file *file_l = रिक्त, *file_m = रिक्त;
	वर्ण *path;

	logindefs_loader = रिक्त; /* No recursion */

	यदि (ul_dlopen_libeconf() != 0)
		प्रत्यागम;

#यदि USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	यदि (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		यदि (file == रिक्त)
			file = file_l;
		अन्यथा यदि (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	यदि (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		यदि (file == रिक्त)
			file = file_l;
		अन्यथा यदि (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} अन्यथा यदि (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		यदि (file == रिक्त)
			file = file_l;
		अन्यथा यदि (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
}

पूर्णाङ्क getlogindefs_bool(ध्रुव वर्ण *name, पूर्णाङ्क dflt)
{
	bool मूल्य;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		प्रत्यागम dflt;

	यदि ((error = econf_call(econf_get_Bool_मूल्य)(file, रिक्त, name, &मूल्य))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		प्रत्यागम dflt;
	}
	प्रत्यागम मूल्य;
}

अचिन्हित दीर्घ getlogindefs_अंक(ध्रुव वर्ण *name, अचिन्हित दीर्घ dflt)
{
	uint64_t मूल्य;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		प्रत्यागम dflt;

	यदि ((error = econf_call(econf_get_UInt64_मूल्य)(file, रिक्त, name, &मूल्य))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		प्रत्यागम dflt;
	}
	प्रत्यागम मूल्य;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ध्रुव वर्ण *getlogindefs_str(ध्रुव वर्ण *name, ध्रुव वर्ण *dflt)
{
	वर्ण *मूल्य;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		प्रत्यागम dflt;

	यदि ((error = econf_call(econf_get_String_मूल्य)(file, रिक्त, name, &मूल्य))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		प्रत्यागम dflt;
	}
	यदि (मूल्य)
		प्रत्यागम मूल्य;

	प्रत्यागम xstrdup("");
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
पूर्णाङ्क logindefs_setenv(ध्रुव वर्ण *name, ध्रुव वर्ण *conf, ध्रुव वर्ण *dflt)
{
	ध्रुव वर्ण *val = getlogindefs_str(conf, dflt);
	ध्रुव वर्ण *p;

	यदि (!val)
		प्रत्यागम -1;

	p = म_अक्षर(val, '=');
	यदि (p) {
		माप_प्रकार sz = म_माप(name);

		यदि (म_भेदन(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			यदि (*val == '"')
				val++;
			यदि (!*val)
				val = dflt;
		}
	}

	प्रत्यागम val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
पूर्णाङ्क logindefs_setenv_path(uid_t uid)
{
	पूर्णाङ्क rc = 0;

	यदि (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	अन्यथा यदि ((rc = logindefs_setenv("PATH", "ENV_SUPATH", रिक्त)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_मूल);

	प्रत्यागम rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
पूर्णाङ्क effective_access(ध्रुव वर्ण *path, पूर्णाङ्क mode)
{
	पूर्णाङ्क fd = खोलो(path, mode);
	यदि (fd != -1)
		बंद(fd);
	प्रत्यागम fd == -1 ? -1 : 0;
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

पूर्णाङ्क get_hushlogin_status(संरचना passwd *pwd, ध्रुव वर्ण *override_home, पूर्णाङ्क force_check)
{
	ध्रुव वर्ण *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, रिक्त };
	ध्रुव वर्ण *file;
	वर्ण buf[बफ_मान];
	पूर्णाङ्क i;

	file = getlogindefs_str("HUSHLOGIN_FILE", रिक्त);
	यदि (file) {
		यदि (!*file)
			प्रत्यागम 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = रिक्त;
	}

	प्रति (i = 0; files[i]; i++) {
		पूर्णाङ्क ok = 0;

		file = files[i];

		/* global hush-file */
		यदि (*file == '/') {
			संरचना stat st;
			खाता *f;

			यदि (stat(file, &st) != 0)
				अनुवर्त;	/* file does not exist */

			यदि (st.st_size == 0)
				प्रत्यागम 1;	/* for all accounts */

			f = ख_खोलो(file, "r" UL_CLOEXECSTR);
			यदि (!f)
				अनुवर्त;	/* ignore errors... */

			यावत् (ok == 0 && ख_माला_लो(buf, परिमाण(buf), f)) {
				यदि (buf[0] != '\0')
					buf[म_माप(buf) - 1] = '\0';
				ok = !म_भेद(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			ख_बंद(f);
			यदि (ok)
				प्रत्यागम 1;	/* found username/shell */

			प्रत्यागम 0;		/* ignore per-account files */
		}

		/* per-account setting */
		यदि (म_माप(pwd->pw_dir) + म_माप(file) + 2 > परिमाण(buf))
			अनुवर्त;

		यदि (snprintf(buf, परिमाण(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			अनुवर्त;

		यदि (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			यदि (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			यदि (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				निकास(निकास_त्रुटि);
			}
			यदि (ok)
				प्रत्यागम 1;	/* enabled by user */
		}
		अन्यथा {
			पूर्णाङ्क rc;
			rc = effective_access(buf, O_RDONLY);
			यदि (rc == 0)
				प्रत्यागम 1;

			यदि (rc == -1 && त्रुटि_सं == EACCES)
				प्रत्यागम -1;
		}

	}

	प्रत्यागम 0;
}
#ifdef TEST_PROGRAM
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *name, *type;
	close_stdout_atexit();

	यदि (argc <= 1)
		errx(निकास_त्रुटि, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	यदि (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		पूर्णाङ्क i;
		वर्ण *कुंजियाँ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				रिक्त};

		प्रति (i = 0; file && कुंजियाँ[i] != रिक्त; i++) {
			वर्ण *मूल्य = रिक्त;

			econf_call(econf_get_String_मूल्य)(file, रिक्त, कुंजियाँ[i], &मूल्य);
			म_लिखो ("%s: $%s: '%s'\n", argv[1], कुंजियाँ[i], मूल्य);
		}

		यदि (file)
			econf_call(econf_freeFile)(file);

#अन्यथा
		संरचना item *ptr;

		प्रति (ptr = list; ptr; ptr = ptr->next)
			म_लिखो("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->मूल्य);
#endif
		प्रत्यागम निकास_सफल;
	}

	type = argv[2];
	name = argv[3];

	यदि (म_भेद(type, "str") == 0)
		म_लिखो("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	अन्यथा यदि (म_भेद(type, "num") == 0)
		म_लिखो("$%s: '%lu'\n", name, getlogindefs_अंक(name, 0));
	अन्यथा यदि (म_भेद(type, "bool") == 0)
		म_लिखो("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	प्रत्यागम निकास_सफल;
}
#endif
