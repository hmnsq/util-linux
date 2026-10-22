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
	अक्षर *name;		/* name of the option.  */
	अक्षर *मूल्य;		/* value of the option.  */
	अक्षर *path;		/* name of config file for this option.  */

	संरचना item *next;	/* pointer to next option.  */
};

स्थिर संरचना item *list = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	संरचना item *ptr;

	ptr = list;
	पर्यंत (ptr) {
		संरचना item *tmp = ptr->next;

		मुक्त(ptr->path);
		मुक्त(ptr->name);
		मुक्त(ptr->मूल्य);
		मुक्त(ptr);
		ptr = tmp;
	}

	list = रिक्त;
}

स्थिर शून्य store(स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य, स्थिरांक अक्षर *path)
{
	संरचना item *new = xmalloc(आकार(संरचना item));

	जर (!name)
		पात();

	new->name = xstrdup(name);
	new->मूल्य = मूल्य && *मूल्य ? xstrdup(मूल्य) : रिक्त;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

शून्य logindefs_load_file(स्थिरांक अक्षर *filename)
{
	खाता *f;
	अक्षर buf[बफ_मान];

	f = ख_खोलो(filename, "r" UL_CLOEXECSTR);
	जर (!f)
		परत;

	पर्यंत (ख_माला_लो(buf, आकार(buf), f)) {

		अक्षर *p, *name, *data = रिक्त;

		जर (*buf == '#' || *buf == '\n')
			सुरूठेवा;	/* only comment or empty line */

		p = म_अक्षर(buf, '#');
		जर (p)
			*p = '\0';
		अन्यथा {
			माप_प्रकार n = म_माप(buf);
			जर (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		जर (!*buf)
			सुरूठेवा;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		पर्यंत (*name && है_खाली((अचिन्हित)*name))
			name++;

		/* go to the end of the name */
		data = name;
		पर्यंत (*data && !(है_खाली((अचिन्हित)*data) || *data == '='))
			data++;
		जर (data > name && *data)
			*data++ = '\0';

		जर (!*name || data == name)
			सुरूठेवा;

		/* go to the begin of the value */
		पर्यंत (*data
		       && (है_खाली((अचिन्हित)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + म_माप(data);
		जर (p > data)
			p--;
		पर्यंत (p > data && (है_खाली((अचिन्हित)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	ख_बंद(f);
}

स्थिर शून्य load_defaults(शून्य)
{
	जर (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	अन्यथा
		logindefs_load_file(_PATH_LOGINDEFS);
}

स्थिर संरचना item *शोध(स्थिरांक अक्षर *name)
{
	संरचना item *ptr;

	जर (!list)
		load_defaults();

	ptr = list;
	पर्यंत (ptr != रिक्त) {
		जर (c_strcasecmp(name, ptr->name) == 0)
			परत ptr;
		ptr = ptr->next;
	}

	परत रिक्त;
}

स्थिर स्थिरांक अक्षर *शोध_config(स्थिरांक अक्षर *name)
{
	संरचना item *ptr;

	ptr = list;
	पर्यंत (ptr != रिक्त) {
		जर (c_strcasecmp(name, ptr->name) == 0)
			परत ptr->path;
		ptr = ptr->next;
	}

	परत रिक्त;
}

पूर्णांक getlogindefs_bool(स्थिरांक अक्षर *name, पूर्णांक dflt)
{
	संरचना item *ptr = शोध(name);
	परत ptr && ptr->मूल्य ? (c_strcasecmp(ptr->मूल्य, "yes") == 0) : dflt;
}

अचिन्हित दीर्घ getlogindefs_अंक(स्थिरांक अक्षर *name, अचिन्हित दीर्घ dflt)
{
	संरचना item *ptr = शोध(name);
	अक्षर *end = रिक्त;
	अचिन्हित दीर्घ retval;

	जर (!ptr || !ptr->मूल्य)
		परत dflt;

	त्रुटि_सं = 0;
	retval = म_से_अदीर्घ(ptr->मूल्य, &end, 0);
	जर (end && *end == '\0' && !त्रुटि_सं)
		परत retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       शोध_config(name), name, ptr->मूल्य);
	परत dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
स्थिरांक अक्षर *getlogindefs_str(स्थिरांक अक्षर *name, स्थिरांक अक्षर *dflt)
{
	संरचना item *ptr = शोध(name);

	जर (!ptr)
		परत dflt;
	जर (!ptr->मूल्य)
		परत "";
	परत ptr->मूल्य;
}

#अन्यथा /* !HAVE_LIBECONF */

#अन्तर्भाव "dl-econf.h"

स्थिर econf_file *file = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	जर (file)
		econf_call(econf_freeFile)(file);
	file = रिक्त;
}

स्थिर शून्य load_defaults(शून्य)
{
	econf_err error;

	जर (file != रिक्त)
		free_getlogindefs_data();

	जर (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, रिक्त,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#अन्यथा
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		जर (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	जर (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

शून्य logindefs_load_file(स्थिरांक अक्षर *filename)
{
	econf_file *file_l = रिक्त, *file_m = रिक्त;
	अक्षर *path;

	logindefs_loader = रिक्त; /* No recursion */

	जर (ul_dlopen_libeconf() != 0)
		परत;

#जर USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	जर (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		जर (file == रिक्त)
			file = file_l;
		अन्यथा जर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	जर (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		जर (file == रिक्त)
			file = file_l;
		अन्यथा जर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} अन्यथा जर (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		जर (file == रिक्त)
			file = file_l;
		अन्यथा जर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
}

पूर्णांक getlogindefs_bool(स्थिरांक अक्षर *name, पूर्णांक dflt)
{
	bool मूल्य;
	econf_err error;

	जर (!file)
		load_defaults();

	जर (!file)
		परत dflt;

	जर ((error = econf_call(econf_get_Bool_मूल्य)(file, रिक्त, name, &मूल्य))) {
		जर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		परत dflt;
	}
	परत मूल्य;
}

अचिन्हित दीर्घ getlogindefs_अंक(स्थिरांक अक्षर *name, अचिन्हित दीर्घ dflt)
{
	uint64_t मूल्य;
	econf_err error;

	जर (!file)
		load_defaults();

	जर (!file)
		परत dflt;

	जर ((error = econf_call(econf_get_UInt64_मूल्य)(file, रिक्त, name, &मूल्य))) {
		जर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		परत dflt;
	}
	परत मूल्य;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
स्थिरांक अक्षर *getlogindefs_str(स्थिरांक अक्षर *name, स्थिरांक अक्षर *dflt)
{
	अक्षर *मूल्य;
	econf_err error;

	जर (!file)
		load_defaults();

	जर (!file)
		परत dflt;

	जर ((error = econf_call(econf_get_String_मूल्य)(file, रिक्त, name, &मूल्य))) {
		जर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		परत dflt;
	}
	जर (मूल्य)
		परत मूल्य;

	परत xstrdup("");
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
पूर्णांक logindefs_setenv(स्थिरांक अक्षर *name, स्थिरांक अक्षर *conf, स्थिरांक अक्षर *dflt)
{
	स्थिरांक अक्षर *val = getlogindefs_str(conf, dflt);
	स्थिरांक अक्षर *p;

	जर (!val)
		परत -1;

	p = म_अक्षर(val, '=');
	जर (p) {
		माप_प्रकार sz = म_माप(name);

		जर (म_भेदन(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			जर (*val == '"')
				val++;
			जर (!*val)
				val = dflt;
		}
	}

	परत val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
पूर्णांक logindefs_setenv_path(uid_t uid)
{
	पूर्णांक rc = 0;

	जर (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	अन्यथा जर ((rc = logindefs_setenv("PATH", "ENV_SUPATH", रिक्त)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_मूळ);

	परत rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
पूर्णांक effective_access(स्थिरांक अक्षर *path, पूर्णांक mode)
{
	पूर्णांक fd = खोलो(path, mode);
	जर (fd != -1)
		बंद(fd);
	परत fd == -1 ? -1 : 0;
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

पूर्णांक get_hushlogin_status(संरचना passwd *pwd, स्थिरांक अक्षर *override_home, पूर्णांक force_check)
{
	स्थिरांक अक्षर *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, रिक्त };
	स्थिरांक अक्षर *file;
	अक्षर buf[बफ_मान];
	पूर्णांक i;

	file = getlogindefs_str("HUSHLOGIN_FILE", रिक्त);
	जर (file) {
		जर (!*file)
			परत 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = रिक्त;
	}

	साठी (i = 0; files[i]; i++) {
		पूर्णांक ok = 0;

		file = files[i];

		/* global hush-file */
		जर (*file == '/') {
			संरचना stat st;
			खाता *f;

			जर (stat(file, &st) != 0)
				सुरूठेवा;	/* file does not exist */

			जर (st.st_size == 0)
				परत 1;	/* for all accounts */

			f = ख_खोलो(file, "r" UL_CLOEXECSTR);
			जर (!f)
				सुरूठेवा;	/* ignore errors... */

			पर्यंत (ok == 0 && ख_माला_लो(buf, आकार(buf), f)) {
				जर (buf[0] != '\0')
					buf[म_माप(buf) - 1] = '\0';
				ok = !म_भेद(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			ख_बंद(f);
			जर (ok)
				परत 1;	/* found username/shell */

			परत 0;		/* ignore per-account files */
		}

		/* per-account setting */
		जर (म_माप(pwd->pw_dir) + म_माप(file) + 2 > आकार(buf))
			सुरूठेवा;

		जर (snprintf(buf, आकार(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			सुरूठेवा;

		जर (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			जर (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			जर (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				निकास(निकास_त्रुटि);
			}
			जर (ok)
				परत 1;	/* enabled by user */
		}
		अन्यथा {
			पूर्णांक rc;
			rc = effective_access(buf, O_RDONLY);
			जर (rc == 0)
				परत 1;

			जर (rc == -1 && त्रुटि_सं == EACCES)
				परत -1;
		}

	}

	परत 0;
}
#ifdef TEST_PROGRAM
पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *name, *type;
	close_stdout_atexit();

	जर (argc <= 1)
		errx(निकास_त्रुटि, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	जर (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		पूर्णांक i;
		अक्षर *किल्ल्या[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				रिक्त};

		साठी (i = 0; file && किल्ल्या[i] != रिक्त; i++) {
			अक्षर *मूल्य = रिक्त;

			econf_call(econf_get_String_मूल्य)(file, रिक्त, किल्ल्या[i], &मूल्य);
			म_लिखो ("%s: $%s: '%s'\n", argv[1], किल्ल्या[i], मूल्य);
		}

		जर (file)
			econf_call(econf_freeFile)(file);

#अन्यथा
		संरचना item *ptr;

		साठी (ptr = list; ptr; ptr = ptr->next)
			म_लिखो("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->मूल्य);
#endif
		परत निकास_सफल;
	}

	type = argv[2];
	name = argv[3];

	जर (म_भेद(type, "str") == 0)
		म_लिखो("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	अन्यथा जर (म_भेद(type, "num") == 0)
		म_लिखो("$%s: '%lu'\n", name, getlogindefs_अंक(name, 0));
	अन्यथा जर (म_भेद(type, "bool") == 0)
		म_लिखो("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	परत निकास_सफल;
}
#endif
