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
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "closestream.h"
#ಅನ್ತರ್ಭಾವ "logindefs.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "xalloc.h"


ಸ್ಥಿರ ಶೂನ್ಯ (*logindefs_loader)(ಶೂನ್ಯ *) = ರಿಕ್ತ;
ಸ್ಥಿರ ಶೂನ್ಯ *logindefs_loader_data = ರಿಕ್ತ;

ಶೂನ್ಯ logindefs_set_loader(ಶೂನ್ಯ (*loader)(ಶೂನ್ಯ *data), ಶೂನ್ಯ *data)
{
	logindefs_loader = loader;
	logindefs_loader_data = data;
}

#ifndef HAVE_LIBECONF

ಸಂರಚನಾ item {
	ವರ್ಣ *name;		/* name of the option.  */
	ವರ್ಣ *ಮೌಲ್ಯ;		/* value of the option.  */
	ವರ್ಣ *path;		/* name of config file for this option.  */

	ಸಂರಚನಾ item *next;	/* pointer to next option.  */
};

ಸ್ಥಿರ ಸಂರಚನಾ item *list = ರಿಕ್ತ;

ಶೂನ್ಯ free_getlogindefs_data(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ item *ptr;

	ptr = list;
	ಯಾವತ್ (ptr) {
		ಸಂರಚನಾ item *tmp = ptr->next;

		ಮುಕ್ತ(ptr->path);
		ಮುಕ್ತ(ptr->name);
		ಮುಕ್ತ(ptr->ಮೌಲ್ಯ);
		ಮುಕ್ತ(ptr);
		ptr = tmp;
	}

	list = ರಿಕ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ store(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ, ಧ್ರುವ ವರ್ಣ *path)
{
	ಸಂರಚನಾ item *new = xmalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ item));

	ಯದಿ (!name)
		abort();

	new->name = xstrdup(name);
	new->ಮೌಲ್ಯ = ಮೌಲ್ಯ && *ಮೌಲ್ಯ ? xstrdup(ಮೌಲ್ಯ) : ರಿಕ್ತ;
	new->path = xstrdup(path);
	new->next = list;
	list = new;
}

ಶೂನ್ಯ logindefs_load_file(ಧ್ರುವ ವರ್ಣ *filename)
{
	FILE *f;
	ವರ್ಣ buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ;

	ಯಾವತ್ (fgets(buf, ಪರಿಮಾಣ(buf), f)) {

		ವರ್ಣ *p, *name, *data = ರಿಕ್ತ;

		ಯದಿ (*buf == '#' || *buf == '\n')
			ಅನುವರ್ತ;	/* only comment or empty line */

		p = strchr(buf, '#');
		ಯದಿ (p)
			*p = '\0';
		ಅನ್ಯಥಾ {
			size_t n = strlen(buf);
			ಯದಿ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ಯದಿ (!*buf)
			ಅನುವರ್ತ;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ಯಾವತ್ (*name && isspace((unsigned)*name))
			name++;

		/* go to the end of the name */
		data = name;
		ಯಾವತ್ (*data && !(isspace((unsigned)*data) || *data == '='))
			data++;
		ಯದಿ (data > name && *data)
			*data++ = '\0';

		ಯದಿ (!*name || data == name)
			ಅನುವರ್ತ;

		/* go to the begin of the value */
		ಯಾವತ್ (*data
		       && (isspace((unsigned)*data) || *data == '='
			   || *data == '"'))
			data++;

		/* remove space at the end of the value */
		p = data + strlen(data);
		ಯದಿ (p > data)
			p--;
		ಯಾವತ್ (p > data && (isspace((unsigned)*p) || *p == '"'))
			*p-- = '\0';

		store(name, data, filename);
	}

	fclose(f);
}

ಸ್ಥಿರ ಶೂನ್ಯ load_defaults(ಶೂನ್ಯ)
{
	ಯದಿ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);
	ಅನ್ಯಥಾ
		logindefs_load_file(_PATH_LOGINDEFS);
}

ಸ್ಥಿರ ಸಂರಚನಾ item *ಹುಡುಕು(ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ item *ptr;

	ಯದಿ (!list)
		load_defaults();

	ptr = list;
	ಯಾವತ್ (ptr != ರಿಕ್ತ) {
		ಯದಿ (c_strcasecmp(name, ptr->name) == 0)
			ಪ್ರತ್ಯಾಗಮ ptr;
		ptr = ptr->next;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಹುಡುಕು_config(ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ item *ptr;

	ptr = list;
	ಯಾವತ್ (ptr != ರಿಕ್ತ) {
		ಯದಿ (c_strcasecmp(name, ptr->name) == 0)
			ಪ್ರತ್ಯಾಗಮ ptr->path;
		ptr = ptr->next;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ getlogindefs_bool(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ dflt)
{
	ಸಂರಚನಾ item *ptr = ಹುಡುಕು(name);
	ಪ್ರತ್ಯಾಗಮ ptr && ptr->ಮೌಲ್ಯ ? (c_strcasecmp(ptr->ಮೌಲ್ಯ, "yes") == 0) : dflt;
}

unsigned long getlogindefs_ಅಂಕೆ(ಧ್ರುವ ವರ್ಣ *name, unsigned long dflt)
{
	ಸಂರಚನಾ item *ptr = ಹುಡುಕು(name);
	ವರ್ಣ *end = ರಿಕ್ತ;
	unsigned long retval;

	ಯದಿ (!ptr || !ptr->ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ dflt;

	errno = 0;
	retval = strtoul(ptr->ಮೌಲ್ಯ, &end, 0);
	ಯದಿ (end && *end == '\0' && !errno)
		ಪ್ರತ್ಯಾಗಮ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       ಹುಡುಕು_config(name), name, ptr->ಮೌಲ್ಯ);
	ಪ್ರತ್ಯಾಗಮ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ಧ್ರುವ ವರ್ಣ *getlogindefs_str(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *dflt)
{
	ಸಂರಚನಾ item *ptr = ಹುಡುಕು(name);

	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ dflt;
	ಯದಿ (!ptr->ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ "";
	ಪ್ರತ್ಯಾಗಮ ptr->ಮೌಲ್ಯ;
}

#ಅನ್ಯಥಾ /* !HAVE_LIBECONF */

#ಅನ್ತರ್ಭಾವ "dl-econf.h"

ಸ್ಥಿರ econf_file *file = ರಿಕ್ತ;

ಶೂನ್ಯ free_getlogindefs_data(ಶೂನ್ಯ)
{
	ಯದಿ (file)
		econf_call(econf_freeFile)(file);
	file = ರಿಕ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ load_defaults(ಶೂನ್ಯ)
{
	econf_err error;

	ಯದಿ (file != ರಿಕ್ತ)
		free_getlogindefs_data();

	ಯದಿ (ul_dlopen_libeconf() == 0) {
#ifdef HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ರಿಕ್ತ,
				UL_VENDORDIR_PATH, "login", "defs", "= \t", "#");
#ಅನ್ಯಥಾ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_PATH, "/etc", "login", "defs", "= \t", "#");
#endif
		ಯದಿ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ಯದಿ (logindefs_loader)
		logindefs_loader(logindefs_loader_data);

}

ಶೂನ್ಯ logindefs_load_file(ಧ್ರುವ ವರ್ಣ *filename)
{
	econf_file *file_l = ರಿಕ್ತ, *file_m = ರಿಕ್ತ;
	ವರ್ಣ *path;

	logindefs_loader = ರಿಕ್ತ; /* No recursion */

	ಯದಿ (ul_dlopen_libeconf() != 0)
		ಪ್ರತ್ಯಾಗಮ;

#ಯದಿ USE_VENDORDIR
	xasprintf(&path, _PATH_VENDORDIR"/%s", filename);

	ಯದಿ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ಯದಿ (file == ರಿಕ್ತ)
			file = file_l;
		ಅನ್ಯಥಾ ಯದಿ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ಮುಕ್ತ (path);
#endif

	xasprintf(&path, "/etc/%s", filename);

	ಯದಿ (!econf_call(econf_readFile)(&file_l, path, "= \t", "#")) {
		ಯದಿ (file == ರಿಕ್ತ)
			file = file_l;
		ಅನ್ಯಥಾ ಯದಿ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ಅನ್ಯಥಾ ಯದಿ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ಯದಿ (file == ರಿಕ್ತ)
			file = file_l;
		ಅನ್ಯಥಾ ಯದಿ (!econf_call(econf_mergeFiles)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ಮುಕ್ತ (path);
}

ಪೂರ್ಣಾಙ್ಕ getlogindefs_bool(ಧ್ರುವ ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ dflt)
{
	bool ಮೌಲ್ಯ;
	econf_err error;

	ಯದಿ (!file)
		load_defaults();

	ಯದಿ (!file)
		ಪ್ರತ್ಯಾಗಮ dflt;

	ಯದಿ ((error = econf_call(econf_get_Bool_ಮೌಲ್ಯ)(file, ರಿಕ್ತ, name, &ಮೌಲ್ಯ))) {
		ಯದಿ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಪ್ರತ್ಯಾಗಮ dflt;
	}
	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

unsigned long getlogindefs_ಅಂಕೆ(ಧ್ರುವ ವರ್ಣ *name, unsigned long dflt)
{
	uint64_t ಮೌಲ್ಯ;
	econf_err error;

	ಯದಿ (!file)
		load_defaults();

	ಯದಿ (!file)
		ಪ್ರತ್ಯಾಗಮ dflt;

	ಯದಿ ((error = econf_call(econf_get_UInt64_ಮೌಲ್ಯ)(file, ರಿಕ್ತ, name, &ಮೌಲ್ಯ))) {
		ಯದಿ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಪ್ರತ್ಯಾಗಮ dflt;
	}
	ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ಧ್ರುವ ವರ್ಣ *getlogindefs_str(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *dflt)
{
	ವರ್ಣ *ಮೌಲ್ಯ;
	econf_err error;

	ಯದಿ (!file)
		load_defaults();

	ಯದಿ (!file)
		ಪ್ರತ್ಯಾಗಮ dflt;

	ಯದಿ ((error = econf_call(econf_get_String_ಮೌಲ್ಯ)(file, ರಿಕ್ತ, name, &ಮೌಲ್ಯ))) {
		ಯದಿ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಪ್ರತ್ಯಾಗಮ dflt;
	}
	ಯದಿ (ಮೌಲ್ಯ)
		ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;

	ಪ್ರತ್ಯಾಗಮ xstrdup("");
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
ಪೂರ್ಣಾಙ್ಕ logindefs_setenv(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *conf, ಧ್ರುವ ವರ್ಣ *dflt)
{
	ಧ್ರುವ ವರ್ಣ *val = getlogindefs_str(conf, dflt);
	ಧ್ರುವ ವರ್ಣ *p;

	ಯದಿ (!val)
		ಪ್ರತ್ಯಾಗಮ -1;

	p = strchr(val, '=');
	ಯದಿ (p) {
		size_t sz = strlen(name);

		ಯದಿ (strncmp(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ಯದಿ (*val == '"')
				val++;
			ಯದಿ (!*val)
				val = dflt;
		}
	}

	ಪ್ರತ್ಯಾಗಮ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ಪೂರ್ಣಾಙ್ಕ logindefs_setenv_path(uid_t uid)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ಅನ್ಯಥಾ ಯದಿ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ರಿಕ್ತ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", PATH_DEFPATH_ಬೇರು);

	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ಪೂರ್ಣಾಙ್ಕ effective_access(ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ mode)
{
	ಪೂರ್ಣಾಙ್ಕ fd = open(path, mode);
	ಯದಿ (fd != -1)
		close(fd);
	ಪ್ರತ್ಯಾಗಮ fd == -1 ? -1 : 0;
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

ಪೂರ್ಣಾಙ್ಕ get_hushlogin_status(ಸಂರಚನಾ passwd *pwd, ಧ್ರುವ ವರ್ಣ *override_home, ಪೂರ್ಣಾಙ್ಕ force_check)
{
	ಧ್ರುವ ವರ್ಣ *files[] = { _PATH_HUSHLOGINS, _PATH_HUSHLOGIN, ರಿಕ್ತ };
	ಧ್ರುವ ವರ್ಣ *file;
	ವರ್ಣ buf[BUFSIZ];
	ಪೂರ್ಣಾಙ್ಕ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ರಿಕ್ತ);
	ಯದಿ (file) {
		ಯದಿ (!*file)
			ಪ್ರತ್ಯಾಗಮ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ರಿಕ್ತ;
	}

	ಪ್ರತಿ (i = 0; files[i]; i++) {
		ಪೂರ್ಣಾಙ್ಕ ok = 0;

		file = files[i];

		/* global hush-file */
		ಯದಿ (*file == '/') {
			ಸಂರಚನಾ stat st;
			FILE *f;

			ಯದಿ (stat(file, &st) != 0)
				ಅನುವರ್ತ;	/* file does not exist */

			ಯದಿ (st.st_size == 0)
				ಪ್ರತ್ಯಾಗಮ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ಯದಿ (!f)
				ಅನುವರ್ತ;	/* ignore errors... */

			ಯಾವತ್ (ok == 0 && fgets(buf, ಪರಿಮಾಣ(buf), f)) {
				ಯದಿ (buf[0] != '\0')
					buf[strlen(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ಯದಿ (ok)
				ಪ್ರತ್ಯಾಗಮ 1;	/* found username/shell */

			ಪ್ರತ್ಯಾಗಮ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ಯದಿ (strlen(pwd->pw_dir) + strlen(file) + 2 > ಪರಿಮಾಣ(buf))
			ಅನುವರ್ತ;

		ಯದಿ (snprintf(buf, ಪರಿಮಾಣ(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ಅನುವರ್ತ;

		ಯದಿ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ಯದಿ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ಯದಿ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				exit(EXIT_FAILURE);
			}
			ಯದಿ (ok)
				ಪ್ರತ್ಯಾಗಮ 1;	/* enabled by user */
		}
		ಅನ್ಯಥಾ {
			ಪೂರ್ಣಾಙ್ಕ rc;
			rc = effective_access(buf, O_RDONLY);
			ಯದಿ (rc == 0)
				ಪ್ರತ್ಯಾಗಮ 1;

			ಯದಿ (rc == -1 && errno == EACCES)
				ಪ್ರತ್ಯಾಗಮ -1;
		}

	}

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ifdef TEST_PROGRAM
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ವರ್ಣ *name, *type;
	close_stdout_atexit();

	ಯದಿ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_load_file(argv[1]);

	ಯದಿ (argc != 4) {	/* list all */
#ifdef HAVE_LIBECONF
		ಪೂರ್ಣಾಙ್ಕ i;
		ವರ್ಣ *ಕೀಲಿಗಳು[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ರಿಕ್ತ};

		ಪ್ರತಿ (i = 0; file && ಕೀಲಿಗಳು[i] != ರಿಕ್ತ; i++) {
			ವರ್ಣ *ಮೌಲ್ಯ = ರಿಕ್ತ;

			econf_call(econf_get_String_ಮೌಲ್ಯ)(file, ರಿಕ್ತ, ಕೀಲಿಗಳು[i], &ಮೌಲ್ಯ);
			printf ("%s: $%s: '%s'\n", argv[1], ಕೀಲಿಗಳು[i], ಮೌಲ್ಯ);
		}

		ಯದಿ (file)
			econf_call(econf_freeFile)(file);

#ಅನ್ಯಥಾ
		ಸಂರಚನಾ item *ptr;

		ಪ್ರತಿ (ptr = list; ptr; ptr = ptr->next)
			printf("%s: $%s: '%s'\n", ptr->path, ptr->name,
			       ptr->ಮೌಲ್ಯ);
#endif
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ಯದಿ (strcmp(type, "str") == 0)
		printf("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ಅನ್ಯಥಾ ಯದಿ (strcmp(type, "num") == 0)
		printf("$%s: '%lu'\n", name, getlogindefs_ಅಂಕೆ(name, 0));
	ಅನ್ಯಥಾ ಯದಿ (strcmp(type, "bool") == 0)
		printf("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif
