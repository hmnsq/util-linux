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
	वर्ण *मान;		/* value of the option.  */
	वर्ण *path;		/* name of config file for this option.  */

	संरचना item *next;	/* pointer to next option.  */
};

स्थिर संरचना item *list = रिक्त;

शून्य free_getlogindefs_data(शून्य)
{
	संरचना item *ptr;

	ptr = list;
	जबसम्म (ptr) {
		संरचना item *tmp = ptr->next;

		मुक्त(ptr->path);
		मुक्त(ptr->name);
		मुक्त(ptr->मान);
		मुक्त(ptr);
		ptr = tmp;
	}

	list = रिक्त;
}

स्थिर शून्य store(अचर वर्ण *name, अचर वर्ण *मान, अचर वर्ण *path)
{
	संरचना item *new = xmalloc(परिमाण(संरचना item));

	यदि (!name)
		पात();

	new->name = xstrdup(name);
	new->मान = मान && *मान ? xstrdup(मान) : रिक्त;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

शून्य logindefs_load_file(अचर वर्ण *filename)
{
	खाता *f;
	वर्ण buf[बफ_मान];

	f = ख_खोलो(filename, "r" UL_CLOEXECSTR);
	यदि (!f)
		फर्काऊ;

	जबसम्म (ख_माला_लो(buf, परिमाण(buf), f)) {

		वर्ण *p, *name, *data = रिक्त;

		यदि (*buf == '#' || *buf == '\n')
			जारी;	/* only comment or empty line */

		p = म_अक्षर(buf, '#');
		यदि (p)
			*p = '\0';
		अन्यथा {
			माप_प्रकार n = म_माप(buf);
			यदि (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		यदि (!*buf)
			जारी;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		जबसम्म (*name && है_खाली((अचिन्हित)*name))
			name++;

		/* go to the end of the name */
		data = name;
		जबसम्म (*data && !(है_खाली((अचिन्हित)*data) || *data == '='))
			data++;
		यदि (data > name && *data)
			*data++ = '\0';

		यदि (!*name || data == name)
			जारी;

		/* go to the begin of the value */
		जबसम्म (*data
		       && (है_खाली((अचिन्हित)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + म_माप(data);
		यदि (p > data)
			p--;
		जबसम्म (p > data && (है_खाली((अचिन्हित)*p) || *p == '"'))
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

स्थिर संरचना item *खोज(अचर वर्ण *name)
{
	संरचना item *ptr;

	यदि (!list)
		load_defaults();

	ptr = list;
	जबसम्म (ptr != रिक्त) {
		यदि (c_strcasecmp(name, ptr->name) == 0)
			फर्काऊ ptr;
		ptr = ptr->next;
	}

	फर्काऊ रिक्त;
}

स्थिर अचर वर्ण *खोज_config(अचर वर्ण *name)
{
	संरचना item *ptr;

	ptr = list;
	जबसम्म (ptr != रिक्त) {
		यदि (c_strcasecmp(name, ptr->name) == 0)
			फर्काऊ ptr->path;
		ptr = ptr->next;
	}

	फर्काऊ रिक्त;
}

पूर्णाङ्क getlogindefs_bool(अचर वर्ण *name, पूर्णाङ्क dflt)
{
	संरचना item *ptr = खोज(name);
	फर्काऊ ptr && ptr->मान ? (c_strcasecmp(ptr->मान, "yes") == 0) : dflt;
}

अचिन्हित दीर्घ getlogindefs_अङ्क(अचर वर्ण *name, अचिन्हित दीर्घ dflt)
{
	संरचना item *ptr = खोज(name);
	वर्ण *end = रिक्त;
	अचिन्हित दीर्घ retval;

	यदि (!ptr || !ptr->मान)
		फर्काऊ dflt;

	त्रुटि_सं = 0;
	retval = म_से_अदीर्घ(ptr->मान, &end, 0);
	यदि (end && *end == '\0' && !त्रुटि_सं)
		फर्काऊ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       खोज_config(name), name, ptr->मान);
	फर्काऊ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
अचर वर्ण *getlogindefs_str(अचर वर्ण *name, अचर वर्ण *dflt)
{
	संरचना item *ptr = खोज(name);

	यदि (!ptr)
		फर्काऊ dflt;
	यदि (!ptr->मान)
		फर्काऊ "";
	फर्काऊ ptr->मान;
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

शून्य logindefs_load_file(अचर वर्ण *filename)
{
	econf_file *file_l = रिक्त, *file_m = रिक्त;
	वर्ण *path;

	logindefs_loader = रिक्त; /* No recursion */

	यदि (ul_dlopen_libeconf() != 0)
		फर्काऊ;

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

पूर्णाङ्क getlogindefs_bool(अचर वर्ण *name, पूर्णाङ्क dflt)
{
	bool मान;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		फर्काऊ dflt;

	यदि ((error = econf_call(econf_get_Bool_मान)(file, रिक्त, name, &मान))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		फर्काऊ dflt;
	}
	फर्काऊ मान;
}

अचिन्हित दीर्घ getlogindefs_अङ्क(अचर वर्ण *name, अचिन्हित दीर्घ dflt)
{
	uint64_t मान;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		फर्काऊ dflt;

	यदि ((error = econf_call(econf_get_UInt64_मान)(file, रिक्त, name, &मान))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		फर्काऊ dflt;
	}
	फर्काऊ मान;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
अचर वर्ण *getlogindefs_str(अचर वर्ण *name, अचर वर्ण *dflt)
{
	वर्ण *मान;
	econf_err error;

	यदि (!file)
		load_defaults();

	यदि (!file)
		फर्काऊ dflt;

	यदि ((error = econf_call(econf_get_String_मान)(file, रिक्त, name, &मान))) {
		यदि (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		फर्काऊ dflt;
	}
	यदि (मान)
		फर्काऊ मान;

	फर्काऊ xstrdup("");
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
पूर्णाङ्क logindefs_setenv(अचर वर्ण *name, अचर वर्ण *conf, अचर वर्ण *dflt)
{
	अचर वर्ण *val = getlogindefs_str(conf, dflt);
	अचर वर्ण *p;

	यदि (!val)
		फर्काऊ -1;

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

	फर्काऊ val ? setenv(name, val, 1) : -1;
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
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_जरा);

	फर्काऊ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
पूर्णाङ्क effective_access(अचर वर्ण *path, पूर्णाङ्क mode)
{
	पूर्णाङ्क fd = खोलो(path, mode);
	यदि (fd != -1)
		बंद(fd);
	फर्काऊ fd == -1 ? -1 : 0;
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

पूर्णाङ्क get_hushlogin_status(संरचना passwd *pwd, अचर वर्ण *override_home, पूर्णाङ्क force_check)
{
	अचर वर्ण *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, रिक्त };
	अचर वर्ण *file;
	वर्ण buf[बफ_मान];
	पूर्णाङ्क i;

	file = getlogindefs_str("HUSHLOGIN_FILE", रिक्त);
	यदि (file) {
		यदि (!*file)
			फर्काऊ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = रिक्त;
	}

	लागि (i = 0; files[i]; i++) {
		पूर्णाङ्क ok = 0;

		file = files[i];

		/* global hush-file */
		यदि (*file == '/') {
			संरचना stat st;
			खाता *f;

			यदि (stat(file, &st) != 0)
				जारी;	/* file does not exist */

			यदि (st.st_size == 0)
				फर्काऊ 1;	/* for all accounts */

			f = ख_खोलो(file, "r" UL_CLOEXECSTR);
			यदि (!f)
				जारी;	/* ignore errors... */

			जबसम्म (ok == 0 && ख_माला_लो(buf, परिमाण(buf), f)) {
				यदि (buf[0] != '\0')
					buf[म_माप(buf) - 1] = '\0';
				ok = !म_भेद(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			ख_बंद(f);
			यदि (ok)
				फर्काऊ 1;	/* found username/shell */

			फर्काऊ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		यदि (म_माप(pwd->pw_dir) + म_माप(file) + 2 > परिमाण(buf))
			जारी;

		यदि (snprintf(buf, परिमाण(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			जारी;

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
				फर्काऊ 1;	/* enabled by user */
		}
		अन्यथा {
			पूर्णाङ्क rc;
			rc = effective_access(buf, O_RDONLY);
			यदि (rc == 0)
				फर्काऊ 1;

			यदि (rc == -1 && त्रुटि_सं == EACCES)
				फर्काऊ -1;
		}

	}

	फर्काऊ 0;
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
		वर्ण *साँचोहरू[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				रिक्त};

		लागि (i = 0; file && साँचोहरू[i] != रिक्त; i++) {
			वर्ण *मान = रिक्त;

			econf_call(econf_get_String_मान)(file, रिक्त, साँचोहरू[i], &मान);
			म_लिखो ("%s: $%s: '%s'\n", argv[1], साँचोहरू[i], मान);
		}

		यदि (file)
			econf_call(econf_freeFile)(file);

#अन्यथा
		संरचना item *ptr;

		लागि (ptr = list; ptr; ptr = ptr->next)
			म_लिखो("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->मान);
#endif
		फर्काऊ निकास_सफल;
	}

	type = argv[2];
	name = argv[3];

	यदि (म_भेद(type, "str") == 0)
		म_लिखो("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	अन्यथा यदि (म_भेद(type, "num") == 0)
		म_लिखो("$%s: '%lu'\n", name, getlogindefs_अङ्क(name, 0));
	अन्यथा यदि (म_भेद(type, "bool") == 0)
		म_लिखो("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	फर्काऊ निकास_सफल;
}
#endif
