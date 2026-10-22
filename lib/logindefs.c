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
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/syslog.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "closestream.h"
#ଅନ୍ତର୍ଭାଵ "logindefs.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "xalloc.h"


ସ୍ଥିର ଶୂନ୍ୟ (*logindefs_loader)(ଶୂନ୍ୟ *) = ଖାଲି;
ସ୍ଥିର ଶୂନ୍ୟ *logindefs_loader_data = ଖାଲି;

ଶୂନ୍ୟ logindefs_set_loader(ଶୂନ୍ୟ (*loader)(ଶୂନ୍ୟ *data), ଶୂନ୍ୟ *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

ସଂରଚନା item {
	ଅକ୍ଷର *name;		/* name of the option.  */
	ଅକ୍ଷର *ମୂଲ୍ୟ;		/* value of the option.  */
	ଅକ୍ଷର *path;		/* name of config file for this option.  */

	ସଂରଚନା item *next;	/* pointer to next option.  */
};

ସ୍ଥିର ସଂରଚନା item *list = ଖାଲି;

ଶୂନ୍ୟ free_getlogindefs_data(ଶୂନ୍ୟ)
{
	ସଂରଚନା item *ptr;

	ptr = list;
	ଯେତେବେଳେ (ptr) {
		ସଂରଚନା item *tmp = ptr->next;

		ମୁକ୍ତ(ptr->path);
		ମୁକ୍ତ(ptr->name);
		ମୁକ୍ତ(ptr->ମୂଲ୍ୟ);
		ମୁକ୍ତ(ptr);
		ptr = tmp;
	}

	list = ଖାଲି;
}

ସ୍ଥିର ଶୂନ୍ୟ store(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ, ଅଚଳ ଅକ୍ଷର *path)
{
	ସଂରଚନା item *new = xmalloc(ଆକାର(ସଂରଚନା item));

	ଯଦି (!name)
		abort();

	new->name = xstrdup(name);
	new->ମୂଲ୍ୟ = ମୂଲ୍ୟ && *ମୂଲ୍ୟ ? xstrdup(ମୂଲ୍ୟ) : ଖାଲି;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

ଶୂନ୍ୟ logindefs_load_file(ଅଚଳ ଅକ୍ଷର *filename)
{
	FILE *f;
	ଅକ୍ଷର buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ଯଦି (!f)
		ଫେରନ୍ତୁ;

	ଯେତେବେଳେ (fgets(buf, ଆକାର(buf), f)) {

		ଅକ୍ଷର *p, *name, *data = ଖାଲି;

		ଯଦି (*buf == '#' || *buf == '\n')
			ଜାରି;	/* only comment or empty line */

		p = strchr(buf, '#');
		ଯଦି (p)
			*p = '\0';
		ଅନ୍ୟଥା {
			size_t n = strlen(buf);
			ଯଦି (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ଯଦି (!*buf)
			ଜାରି;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ଯେତେବେଳେ (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		ଯେତେବେଳେ (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		ଯଦି (data > name && *data)
			*data++ = '\0';

		ଯଦି (!*name || data == name)
			ଜାରି;

		/* go to the begin of the value */
		ଯେତେବେଳେ (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		ଯଦି (p > data)
			p--;
		ଯେତେବେଳେ (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

ସ୍ଥିର ଶୂନ୍ୟ load_defaults(ଶୂନ୍ୟ)
{
	ଯଦି (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	ଅନ୍ୟଥା
		logindefs_load_file(_PATH_LOGINDEFS);
}

ସ୍ଥିର ସଂରଚନା item *ସନ୍ଧାନ(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା item *ptr;

	ଯଦି (!list)
		load_defaults();

	ptr = list;
	ଯେତେବେଳେ (ptr != ଖାଲି) {
		ଯଦି (c_strcasecmp(name, ptr->name) == 0)
			ଫେରନ୍ତୁ ptr;
		ptr = ptr->next;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ସନ୍ଧାନ_config(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା item *ptr;

	ptr = list;
	ଯେତେବେଳେ (ptr != ଖାଲି) {
		ଯଦି (c_strcasecmp(name, ptr->name) == 0)
			ଫେରନ୍ତୁ ptr->path;
		ptr = ptr->next;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା getlogindefs_bool(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dflt)
{
	ସଂରଚନା item *ptr = ସନ୍ଧାନ(name);
	ଫେରନ୍ତୁ ptr && ptr->ମୂଲ୍ୟ ? (c_strcasecmp(ptr->ମୂଲ୍ୟ, "yes") == 0) : dflt;
}

unsigned long getlogindefs_ଅଙ୍କ(ଅଚଳ ଅକ୍ଷର *name, unsigned long dflt)
{
	ସଂରଚନା item *ptr = ସନ୍ଧାନ(name);
	ଅକ୍ଷର *end = ଖାଲି;
	unsigned long retval;

	ଯଦି (!ptr || !ptr->ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ dflt;

	errno = 0;
	retval = strtoul(ptr->ମୂଲ୍ୟ, &end, 0);
	ଯଦି (end && *end == '\0' && !errno)
		ଫେରନ୍ତୁ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       ସନ୍ଧାନ_config(name), name, ptr->ମୂଲ୍ୟ);
	ଫେରନ୍ତୁ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ଅଚଳ ଅକ୍ଷର *getlogindefs_str(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *dflt)
{
	ସଂରଚନା item *ptr = ସନ୍ଧାନ(name);

	ଯଦି (!ptr)
		ଫେରନ୍ତୁ dflt;
	ଯଦି (!ptr->ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ "";
	ଫେରନ୍ତୁ ptr->ମୂଲ୍ୟ;
}

#ଅନ୍ୟଥା /* !HAVE_LIBECONF */

#ଅନ୍ତର୍ଭାଵ "dl-econf.h"

ସ୍ଥିର econf_file *file = ଖାଲି;

ଶୂନ୍ୟ free_getlogindefs_data(ଶୂନ୍ୟ)
{
	ଯଦି (file)
		econf_call(econf_freeFile)(file);
	file = ଖାଲି;
}

ସ୍ଥିର ଶୂନ୍ୟ load_defaults(ଶୂନ୍ୟ)
{
	econf_err error;

	ଯଦି (file != ଖାଲି)
		free_getlogindefs_data();

	ଯଦି (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ଖାଲି,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#ଅନ୍ୟଥା
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		ଯଦି (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ଯଦି (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

ଶୂନ୍ୟ logindefs_load_file(ଅଚଳ ଅକ୍ଷର *filename)
{
	econf_file *file_l = ଖାଲି, *file_m = ଖାଲି;
	ଅକ୍ଷର *path;

	logindefs_loader = ଖାଲି; /* No recursion */

	ଯଦି (ul_dlopen_libeconf() != 0)
		ଫେରନ୍ତୁ;

#ଯଦି USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	ଯଦି (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ଯଦି (file == ଖାଲି)
			file = file_l;
		ଅନ୍ୟଥା ଯଦି (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ମୁକ୍ତ (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	ଯଦି (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ଯଦି (file == ଖାଲି)
			file = file_l;
		ଅନ୍ୟଥା ଯଦି (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ଅନ୍ୟଥା ଯଦି (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ଯଦି (file == ଖାଲି)
			file = file_l;
		ଅନ୍ୟଥା ଯଦି (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ମୁକ୍ତ (path);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା getlogindefs_bool(ଅଚଳ ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dflt)
{
	bool ମୂଲ୍ୟ;
	econf_err error;

	ଯଦି (!file)
		load_defaults();

	ଯଦି (!file)
		ଫେରନ୍ତୁ dflt;

	ଯଦି ((error = econf_call(econf_get_Bool_ମୂଲ୍ୟ)(file, ଖାଲି, name, &ମୂଲ୍ୟ))) {
		ଯଦି (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ଫେରନ୍ତୁ dflt;
	}
	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

unsigned long getlogindefs_ଅଙ୍କ(ଅଚଳ ଅକ୍ଷର *name, unsigned long dflt)
{
	uint64_t ମୂଲ୍ୟ;
	econf_err error;

	ଯଦି (!file)
		load_defaults();

	ଯଦି (!file)
		ଫେରନ୍ତୁ dflt;

	ଯଦି ((error = econf_call(econf_get_UInt64_ମୂଲ୍ୟ)(file, ଖାଲି, name, &ମୂଲ୍ୟ))) {
		ଯଦି (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ଫେରନ୍ତୁ dflt;
	}
	ଫେରନ୍ତୁ ମୂଲ୍ୟ;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ଅଚଳ ଅକ୍ଷର *getlogindefs_str(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *dflt)
{
	ଅକ୍ଷର *ମୂଲ୍ୟ;
	econf_err error;

	ଯଦି (!file)
		load_defaults();

	ଯଦି (!file)
		ଫେରନ୍ତୁ dflt;

	ଯଦି ((error = econf_call(econf_get_String_ମୂଲ୍ୟ)(file, ଖାଲି, name, &ମୂଲ୍ୟ))) {
		ଯଦି (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ଫେରନ୍ତୁ dflt;
	}
	ଯଦି (ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ ମୂଲ୍ୟ;

	ଫେରନ୍ତୁ xstrdup("");
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା logindefs_setenv(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *conf, ଅଚଳ ଅକ୍ଷର *dflt)
{
	ଅଚଳ ଅକ୍ଷର *val = getlogindefs_str(conf, dflt);
	ଅଚଳ ଅକ୍ଷର *p;

	ଯଦି (!val)
		ଫେରନ୍ତୁ -1;

	p = strchr(val, '=');
	ଯଦି (p) {
		size_t sz = strlen(name);

		ଯଦି (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ଯଦି (*val == '"')
				val++;
			ଯଦି (!*val)
				val = dflt;
		}
	}

	ଫେରନ୍ତୁ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା logindefs_setenv_path(uid_t uid)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ଅନ୍ୟଥା ଯଦି ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ଖାଲି)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_ମୂଳ);

	ଫେରନ୍ତୁ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା effective_access(ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(path, mode);
	ଯଦି (fd != -1)
		close(fd);
	ଫେରନ୍ତୁ fd == -1 ? -1 : 0;
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_hushlogin_status(ସଂରଚନା passwd *pwd, ଅଚଳ ଅକ୍ଷର *override_home, ପୂର୍ଣ୍ଣସଂଖ୍ୟା force_check)
{
	ଅଚଳ ଅକ୍ଷର *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, ଖାଲି };
	ଅଚଳ ଅକ୍ଷର *file;
	ଅକ୍ଷର buf[BUFSIZ];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ଖାଲି);
	ଯଦି (file) {
		ଯଦି (!*file)
			ଫେରନ୍ତୁ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ଖାଲି;
	}

	ପାଇଁ (i = 0; files[i]; i++) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ok = 0;

		file = files[i];

		/* global hush-file */
		ଯଦି (*file == '/') {
			ସଂରଚନା stat st;
			FILE *f;

			ଯଦି (stat(file, &st) != 0)
				ଜାରି;	/* file does not exist */

			ଯଦି (st.st_size == 0)
				ଫେରନ୍ତୁ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ଯଦି (!f)
				ଜାରି;	/* ignore errors... */

			ଯେତେବେଳେ (ok == 0 && fgets(buf, ଆକାର(buf), f)) {
				ଯଦି (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ଯଦି (ok)
				ଫେରନ୍ତୁ 1;	/* found username/shell */

			ଫେରନ୍ତୁ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ଯଦି (strlen(pwd->pw_dir) + strlen(file) + 2 > ଆକାର(buf))
			ଜାରି;

		ଯଦି (snprintf(buf, ଆକାର(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ଜାରି;

		ଯଦି (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ଯଦି (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ଯଦି (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			ଯଦି (ok)
				ଫେରନ୍ତୁ 1;	/* enabled by user */
		}
		ଅନ୍ୟଥା {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
			rc = effective_access(buf, O_RDONLY);
			ଯଦି (rc == 0)
				ଫେରନ୍ତୁ 1;

			ଯଦି (rc == -1 && errno == EACCES)
				ଫେରନ୍ତୁ -1;
		}

	}

	ଫେରନ୍ତୁ 0;
}
#ifdef TEST_PROGRAM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅକ୍ଷର *name, *type;
	close_stdout_atexit();

	ଯଦି (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	ଯଦି (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		ଅକ୍ଷର *ଚାବିଗୁଡ଼ିକ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ଖାଲି};

		ପାଇଁ (i = 0; file && ଚାବିଗୁଡ଼ିକ[i] != ଖାଲି; i++) {
			ଅକ୍ଷର *ମୂଲ୍ୟ = ଖାଲି;

			econf_call(econf_get_String_ମୂଲ୍ୟ)(file, ଖାଲି, ଚାବିଗୁଡ଼ିକ[i], &ମୂଲ୍ୟ);
			printf ("%s: $%s: '%s'\n", argv[1], ଚାବିଗୁଡ଼ିକ[i], ମୂଲ୍ୟ);
		}

		ଯଦି (file)
			econf_call(econf_freeFile)(file);

#ଅନ୍ୟଥା
		ସଂରଚନା item *ptr;

		ପାଇଁ (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->ମୂଲ୍ୟ);
#endif
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ଯଦି (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ଅନ୍ୟଥା ଯଦି (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_ଅଙ୍କ(name, 0));
	ଅନ୍ୟଥା ଯଦି (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif
