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
#समावेश <assert.h>
#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <limits.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/syslog.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <pwd.h>

#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "closestream.h"
#समावेश "logindefs.h"
#समावेश "nls.h"
#समावेश "pathnames.h"
#समावेश "xalloc.h"


अटल व्योम (*logindefs_loader)(व्योम *) = शून्य;
अटल व्योम *logindefs_loader_तथ्य = शून्य;

व्योम logindefs_set_loader(व्योम (*loader)(व्योम *तथ्य), व्योम *तथ्य)
{
	logindefs_loader = loader;
	logindefs_loader_तथ्य = तथ्य;
}

#अगर_अघोषित HAVE_LIBECONF

काष्ठा item {
	अक्षर *name;		/* name of the option.  */
	अक्षर *मूल्य;		/* value of the option.  */
	अक्षर *path;		/* name of config file for this option.  */

	काष्ठा item *अग्र;	/* pointer to next option.  */
};

अटल काष्ठा item *list = शून्य;

व्योम free_getlogindefs_तथ्य(व्योम)
{
	काष्ठा item *ptr;

	ptr = list;
	जबतक (ptr) {
		काष्ठा item *tmp = ptr->अग्र;

		मुक्त(ptr->path);
		मुक्त(ptr->name);
		मुक्त(ptr->मूल्य);
		मुक्त(ptr);
		ptr = tmp;
	}

	list = शून्य;
}

अटल व्योम store(स्थिर अक्षर *name, स्थिर अक्षर *मूल्य, स्थिर अक्षर *path)
{
	काष्ठा item *new = xmalloc(माप(काष्ठा item));

	अगर (!name)
		पात();

	new->name = xstrdup(name);
	new->मूल्य = मूल्य && *मूल्य ? xstrdup(मूल्य) : शून्य;
	new->path = xstrdup(path);
	new->अग्र = list;
	list = new;
}

व्योम logindefs_load_file(स्थिर अक्षर *filename)
{
	खाता *f;
	अक्षर buf[बफ_मान];

	f = ख_खोलो(filename, "r" UL_CLOEXECSTR);
	अगर (!f)
		वापस;

	जबतक (ख_माला_लो(buf, माप(buf), f)) {

		अक्षर *p, *name, *तथ्य = शून्य;

		अगर (*buf == '#' || *buf == '\n')
			जारी;	/* only comment or empty line */

		p = म_अक्षर(buf, '#');
		अगर (p)
			*p = '\0';
		अन्यथा {
			माप_प्रकार n = म_माप(buf);
			अगर (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		अगर (!*buf)
			जारी;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		जबतक (*name && है_खाली((अचिन्हित)*name))
			name++;

		/* go to the end of the name */
		तथ्य = name;
		जबतक (*तथ्य && !(है_खाली((अचिन्हित)*तथ्य) || *तथ्य == '='))
			तथ्य++;
		अगर (तथ्य > name && *तथ्य)
			*तथ्य++ = '\0';

		अगर (!*name || तथ्य == name)
			जारी;

		/* go to the begin of the value */
		जबतक (*तथ्य
		       && (है_खाली((अचिन्हित)*तथ्य) || *तथ्य == '='
			   || *तथ्य == '"'))
			तथ्य++;

		/* remove space at the end of the value */
		p = तथ्य + म_माप(तथ्य);
		अगर (p > तथ्य)
			p--;
		जबतक (p > तथ्य && (है_खाली((अचिन्हित)*p) || *p == '"'))
			*p-- = '\0';

		store(name, तथ्य, filename);
	}

	ख_बंद(f);
}

अटल व्योम load_defaults(व्योम)
{
	अगर (logindefs_loader)
		logindefs_loader(logindefs_loader_तथ्य);
	अन्यथा
		logindefs_load_file(_PATH_LOGINDEFS);
}

अटल काष्ठा item *खोज(स्थिर अक्षर *name)
{
	काष्ठा item *ptr;

	अगर (!list)
		load_defaults();

	ptr = list;
	जबतक (ptr != शून्य) {
		अगर (c_strcasecmp(name, ptr->name) == 0)
			वापस ptr;
		ptr = ptr->अग्र;
	}

	वापस शून्य;
}

अटल स्थिर अक्षर *खोज_config(स्थिर अक्षर *name)
{
	काष्ठा item *ptr;

	ptr = list;
	जबतक (ptr != शून्य) {
		अगर (c_strcasecmp(name, ptr->name) == 0)
			वापस ptr->path;
		ptr = ptr->अग्र;
	}

	वापस शून्य;
}

पूर्णांक getlogindefs_bool(स्थिर अक्षर *name, पूर्णांक dflt)
{
	काष्ठा item *ptr = खोज(name);
	वापस ptr && ptr->मूल्य ? (c_strcasecmp(ptr->मूल्य, "yes") == 0) : dflt;
}

अचिन्हित दीर्घ getlogindefs_अंक(स्थिर अक्षर *name, अचिन्हित दीर्घ dflt)
{
	काष्ठा item *ptr = खोज(name);
	अक्षर *end = शून्य;
	अचिन्हित दीर्घ retval;

	अगर (!ptr || !ptr->मूल्य)
		वापस dflt;

	त्रुटि_सं = 0;
	retval = म_से_अदीर्घ(ptr->मूल्य, &end, 0);
	अगर (end && *end == '\0' && !त्रुटि_सं)
		वापस retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       खोज_config(name), name, ptr->मूल्य);
	वापस dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
स्थिर अक्षर *getlogindefs_str(स्थिर अक्षर *name, स्थिर अक्षर *dflt)
{
	काष्ठा item *ptr = खोज(name);

	अगर (!ptr)
		वापस dflt;
	अगर (!ptr->मूल्य)
		वापस "";
	वापस ptr->मूल्य;
}

#अन्यथा /* !HAVE_LIBECONF */

#समावेश "dl-econf.h"

अटल econf_file *file = शून्य;

व्योम free_getlogindefs_तथ्य(व्योम)
{
	अगर (file)
		econf_call(econf_freeFile)(file);
	file = शून्य;
}

अटल व्योम load_defaults(व्योम)
{
	econf_err error;

	अगर (file != शून्य)
		free_getlogindefs_तथ्य();

	अगर (ul_dlopen_libeconf() == 0) {
#अगर_घोषित HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, शून्य,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#अन्यथा
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#पूर्ण_अगर
		अगर (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	अगर (logindefs_loader)
		logindefs_loader(logindefs_loader_तथ्य);

}

व्योम logindefs_load_file(स्थिर अक्षर *filename)
{
	econf_file *file_l = शून्य, *file_m = शून्य;
	अक्षर *path;

	logindefs_loader = शून्य; /* No recursion */

	अगर (ul_dlopen_libeconf() != 0)
		वापस;

#अगर USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	अगर (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		अगर (file == शून्य)
			file = file_l;
		अन्यथा अगर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
#पूर्ण_अगर

	xasprintf(&path, "/etc/%s", filename);

	अगर (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		अगर (file == शून्य)
			file = file_l;
		अन्यथा अगर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} अन्यथा अगर (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		अगर (file == शून्य)
			file = file_l;
		अन्यथा अगर (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	मुक्त (path);
}

पूर्णांक getlogindefs_bool(स्थिर अक्षर *name, पूर्णांक dflt)
{
	bool मूल्य;
	econf_err error;

	अगर (!file)
		load_defaults();

	अगर (!file)
		वापस dflt;

	अगर ((error = econf_call(econf_get_Bool_मूल्य)(file, शून्य, name, &मूल्य))) {
		अगर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		वापस dflt;
	}
	वापस मूल्य;
}

अचिन्हित दीर्घ getlogindefs_अंक(स्थिर अक्षर *name, अचिन्हित दीर्घ dflt)
{
	uint64_t मूल्य;
	econf_err error;

	अगर (!file)
		load_defaults();

	अगर (!file)
		वापस dflt;

	अगर ((error = econf_call(econf_get_UInt64_मूल्य)(file, शून्य, name, &मूल्य))) {
		अगर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		वापस dflt;
	}
	वापस मूल्य;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
स्थिर अक्षर *getlogindefs_str(स्थिर अक्षर *name, स्थिर अक्षर *dflt)
{
	अक्षर *मूल्य;
	econf_err error;

	अगर (!file)
		load_defaults();

	अगर (!file)
		वापस dflt;

	अगर ((error = econf_call(econf_get_String_मूल्य)(file, शून्य, name, &मूल्य))) {
		अगर (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		वापस dflt;
	}
	अगर (मूल्य)
		वापस मूल्य;

	वापस xstrdup("");
}
#पूर्ण_अगर /* !HAVE_LIBECONF */

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
पूर्णांक logindefs_setenv(स्थिर अक्षर *name, स्थिर अक्षर *conf, स्थिर अक्षर *dflt)
{
	स्थिर अक्षर *val = getlogindefs_str(conf, dflt);
	स्थिर अक्षर *p;

	अगर (!val)
		वापस -1;

	p = म_अक्षर(val, '=');
	अगर (p) {
		माप_प्रकार sz = म_माप(name);

		अगर (म_भेदन(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			अगर (*val == '"')
				val++;
			अगर (!*val)
				val = dflt;
		}
	}

	वापस val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
पूर्णांक logindefs_setenv_path(uid_t uid)
{
	पूर्णांक rc = 0;

	अगर (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	अन्यथा अगर ((rc = logindefs_setenv("PATH", "ENV_SUPATH", शून्य)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_मूल);

	वापस rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
पूर्णांक effective_access(स्थिर अक्षर *path, पूर्णांक mode)
{
	पूर्णांक fd = खोलो(path, mode);
	अगर (fd != -1)
		बंद(fd);
	वापस fd == -1 ? -1 : 0;
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

पूर्णांक get_hushlogin_status(काष्ठा passwd *pwd, स्थिर अक्षर *override_home, पूर्णांक force_check)
{
	स्थिर अक्षर *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, शून्य };
	स्थिर अक्षर *file;
	अक्षर buf[बफ_मान];
	पूर्णांक i;

	file = getlogindefs_str("HUSHLOGIN_FILE", शून्य);
	अगर (file) {
		अगर (!*file)
			वापस 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = शून्य;
	}

	क्रम (i = 0; files[i]; i++) {
		पूर्णांक ok = 0;

		file = files[i];

		/* global hush-file */
		अगर (*file == '/') {
			काष्ठा stat st;
			खाता *f;

			अगर (stat(file, &st) != 0)
				जारी;	/* file does not exist */

			अगर (st.st_size == 0)
				वापस 1;	/* for all accounts */

			f = ख_खोलो(file, "r" UL_CLOEXECSTR);
			अगर (!f)
				जारी;	/* ignore errors... */

			जबतक (ok == 0 && ख_माला_लो(buf, माप(buf), f)) {
				अगर (buf[0] != '\0')
					buf[म_माप(buf) - 1] = '\0';
				ok = !म_भेद(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			ख_बंद(f);
			अगर (ok)
				वापस 1;	/* found username/shell */

			वापस 0;		/* ignore per-account files */
		}

		/* per-account setting */
		अगर (म_माप(pwd->pw_dir) + म_माप(file) + 2 > माप(buf))
			जारी;

		अगर (snprintf(buf, माप(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			जारी;

		अगर (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			अगर (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			अगर (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				निकास(निकास_त्रुटि);
			}
			अगर (ok)
				वापस 1;	/* enabled by user */
		}
		अन्यथा {
			पूर्णांक rc;
			rc = effective_access(buf, O_RDONLY);
			अगर (rc == 0)
				वापस 1;

			अगर (rc == -1 && त्रुटि_सं == EACCES)
				वापस -1;
		}

	}

	वापस 0;
}
#अगर_घोषित TEST_PROGRAM
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *name, *type;
	close_stdout_atexit();

	अगर (argc <= 1)
		errx(निकास_त्रुटि, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	अगर (argc != 4) {	/* list all */
#अगर_घोषित HAVE_LIBECONF
		पूर्णांक i;
		अक्षर *कुंजियाँ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				शून्य};

		क्रम (i = 0; file && कुंजियाँ[i] != शून्य; i++) {
			अक्षर *मूल्य = शून्य;

			econf_call(econf_get_String_मूल्य)(file, शून्य, कुंजियाँ[i], &मूल्य);
			म_लिखो ("%s: $%s: '%s'\n", argv[1], कुंजियाँ[i], मूल्य);
		}

		अगर (file)
			econf_call(econf_freeFile)(file);

#अन्यथा
		काष्ठा item *ptr;

		क्रम (ptr = list; ptr; ptr = ptr->अग्र)
			म_लिखो("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->मूल्य);
#पूर्ण_अगर
		वापस निकास_सफल;
	}

	type = argv[2];
	name = argv[3];

	अगर (म_भेद(type, "str") == 0)
		म_लिखो("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	अन्यथा अगर (म_भेद(type, "num") == 0)
		म_लिखो("$%s: '%lu'\n", name, getlogindefs_अंक(name, 0));
	अन्यथा अगर (म_भेद(type, "bool") == 0)
		म_लिखो("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	वापस निकास_सफल;
}
#पूर्ण_अगर
