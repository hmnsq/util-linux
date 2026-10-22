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
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "closestream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "logindefs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "xalloc.h"


ਸਥਿਰ ਖਾਲੀ (*logindefs_loader)(ਖਾਲੀ *) = ਨਿਰਾ;
ਸਥਿਰ ਖਾਲੀ *logindefs_loader_ਅੰਕੜੇ = ਨਿਰਾ;

ਖਾਲੀ logindefs_ਸੈੱਟ_loader(ਖਾਲੀ (*loader)(ਖਾਲੀ *ਅੰਕੜੇ), ਖਾਲੀ *ਅੰਕੜੇ)
{
	logindefs_loader = loader;
	logindefs_loader_ਅੰਕੜੇ = ਅੰਕੜੇ;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_LIBECONF

ਬਣਤਰ ਵਸਤੂ {
	ਅੱਖਰ *name;		/* name of the option.  */
	ਅੱਖਰ *ਮੁੱਲ;		/* value of the option.  */
	ਅੱਖਰ *ਰਸਤਾ;		/* name of config file for this option.  */

	ਬਣਤਰ ਵਸਤੂ *ਅਗਲਾ;	/* pointer to next option.  */
};

ਸਥਿਰ ਬਣਤਰ ਵਸਤੂ *list = ਨਿਰਾ;

ਖਾਲੀ free_getlogindefs_ਅੰਕੜੇ(ਖਾਲੀ)
{
	ਬਣਤਰ ਵਸਤੂ *ptr;

	ptr = list;
	ਜਦੋਂ (ptr) {
		ਬਣਤਰ ਵਸਤੂ *tmp = ptr->ਅਗਲਾ;

		ਮੁਕਤ(ptr->ਰਸਤਾ);
		ਮੁਕਤ(ptr->name);
		ਮੁਕਤ(ptr->ਮੁੱਲ);
		ਮੁਕਤ(ptr);
		ptr = tmp;
	}

	list = ਨਿਰਾ;
}

ਸਥਿਰ ਖਾਲੀ store(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਬਣਤਰ ਵਸਤੂ *new = xmalloc(ਆਕਾਰ(ਬਣਤਰ ਵਸਤੂ));

	ਜੇ (!name)
		abort();

	new->name = xstrdup(name);
	new->ਮੁੱਲ = ਮੁੱਲ && *ਮੁੱਲ ? xstrdup(ਮੁੱਲ) : ਨਿਰਾ;
	new->ਰਸਤਾ = xstrdup(ਰਸਤਾ);
	new->ਅਗਲਾ = list;
	list = new;
}

ਖਾਲੀ logindefs_ਲੱਦੋ_file(ਅਚਲ ਅੱਖਰ *filename)
{
	FILE *f;
	ਅੱਖਰ buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ਜੇ (!f)
		ਵਾਪਸ;

	ਜਦੋਂ (fgets(buf, ਆਕਾਰ(buf), f)) {

		ਅੱਖਰ *p, *name, *ਅੰਕੜੇ = ਨਿਰਾ;

		ਜੇ (*buf == '#' || *buf == '\n')
			ਜਾਰੀ;	/* only comment or empty line */

		p = strchr(buf, '#');
		ਜੇ (p)
			*p = '\0';
		ਨਹੀਂਤਾਂ {
			size_t n = ਲੜੀ_ਲੰਬਾਈ(buf);
			ਜੇ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ਜੇ (!*buf)
			ਜਾਰੀ;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ਜਦੋਂ (*name && ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ)*name))
			name++;

		/* go to the end of the name */
		ਅੰਕੜੇ = name;
		ਜਦੋਂ (*ਅੰਕੜੇ && !(ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ)*ਅੰਕੜੇ) || *ਅੰਕੜੇ == '='))
			ਅੰਕੜੇ++;
		ਜੇ (ਅੰਕੜੇ > name && *ਅੰਕੜੇ)
			*ਅੰਕੜੇ++ = '\0';

		ਜੇ (!*name || ਅੰਕੜੇ == name)
			ਜਾਰੀ;

		/* go to the begin of the value */
		ਜਦੋਂ (*ਅੰਕੜੇ
		       && (ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ)*ਅੰਕੜੇ) || *ਅੰਕੜੇ == '='
			   || *ਅੰਕੜੇ == '"'))
			ਅੰਕੜੇ++;

		/* remove space at the end of the value */
		p = ਅੰਕੜੇ + ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ);
		ਜੇ (p > ਅੰਕੜੇ)
			p--;
		ਜਦੋਂ (p > ਅੰਕੜੇ && (ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ)*p) || *p == '"'))
			*p-- = '\0';

		store(name, ਅੰਕੜੇ, filename);
	}

	fclose(f);
}

ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_defaults(ਖਾਲੀ)
{
	ਜੇ (logindefs_loader)
		logindefs_loader(logindefs_loader_ਅੰਕੜੇ);
	ਨਹੀਂਤਾਂ
		logindefs_ਲੱਦੋ_file(ਰਸਤਾ_LOGINDEFS);
}

ਸਥਿਰ ਬਣਤਰ ਵਸਤੂ *ਖੋਜ(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ ਵਸਤੂ *ptr;

	ਜੇ (!list)
		ਲੱਦੋ_defaults();

	ptr = list;
	ਜਦੋਂ (ptr != ਨਿਰਾ) {
		ਜੇ (c_strcasecmp(name, ptr->name) == 0)
			ਵਾਪਸ ptr;
		ptr = ptr->ਅਗਲਾ;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਖੋਜ_config(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ ਵਸਤੂ *ptr;

	ptr = list;
	ਜਦੋਂ (ptr != ਨਿਰਾ) {
		ਜੇ (c_strcasecmp(name, ptr->name) == 0)
			ਵਾਪਸ ptr->ਰਸਤਾ;
		ptr = ptr->ਅਗਲਾ;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ getlogindefs_bool(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ dflt)
{
	ਬਣਤਰ ਵਸਤੂ *ptr = ਖੋਜ(name);
	ਵਾਪਸ ptr && ptr->ਮੁੱਲ ? (c_strcasecmp(ptr->ਮੁੱਲ, "yes") == 0) : dflt;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ getlogindefs_ਅੰਕ(ਅਚਲ ਅੱਖਰ *name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ dflt)
{
	ਬਣਤਰ ਵਸਤੂ *ptr = ਖੋਜ(name);
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ retval;

	ਜੇ (!ptr || !ptr->ਮੁੱਲ)
		ਵਾਪਸ dflt;

	errno = 0;
	retval = strtoul(ptr->ਮੁੱਲ, &ਸਿਰਾ, 0);
	ਜੇ (ਸਿਰਾ && *ਸਿਰਾ == '\0' && !errno)
		ਵਾਪਸ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       ਖੋਜ_config(name), name, ptr->ਮੁੱਲ);
	ਵਾਪਸ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ਅਚਲ ਅੱਖਰ *getlogindefs_str(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *dflt)
{
	ਬਣਤਰ ਵਸਤੂ *ptr = ਖੋਜ(name);

	ਜੇ (!ptr)
		ਵਾਪਸ dflt;
	ਜੇ (!ptr->ਮੁੱਲ)
		ਵਾਪਸ "";
	ਵਾਪਸ ptr->ਮੁੱਲ;
}

#ਨਹੀਂਤਾਂ /* !HAVE_LIBECONF */

#ਅਨ੍ਤਰ੍ਭਾਵ "dl-econf.h"

ਸਥਿਰ econf_file *file = ਨਿਰਾ;

ਖਾਲੀ free_getlogindefs_ਅੰਕੜੇ(ਖਾਲੀ)
{
	ਜੇ (file)
		econf_call(econf_freeFile)(file);
	file = ਨਿਰਾ;
}

ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_defaults(ਖਾਲੀ)
{
	econf_err error;

	ਜੇ (file != ਨਿਰਾ)
		free_getlogindefs_ਅੰਕੜੇ();

	ਜੇ (ul_dlopen_libeconf() == 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ਨਿਰਾ,
				UL_VENDORDIR_ਰਸਤਾ, "login", "defs", "= \t", "#");
#ਨਹੀਂਤਾਂ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_ਰਸਤਾ, "/etc", "login", "defs", "= \t", "#");
#ਜੇ_ਅੰਤ
		ਜੇ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ਜੇ (logindefs_loader)
		logindefs_loader(logindefs_loader_ਅੰਕੜੇ);

}

ਖਾਲੀ logindefs_ਲੱਦੋ_file(ਅਚਲ ਅੱਖਰ *filename)
{
	econf_file *file_l = ਨਿਰਾ, *file_m = ਨਿਰਾ;
	ਅੱਖਰ *ਰਸਤਾ;

	logindefs_loader = ਨਿਰਾ; /* No recursion */

	ਜੇ (ul_dlopen_libeconf() != 0)
		ਵਾਪਸ;

#ਜੇ USE_VENDORDIR
	xasprintf(&ਰਸਤਾ, ਰਸਤਾ_VENDORDIR"/%s", filename);

	ਜੇ (!econf_call(econf_readFile)(&file_l, ਰਸਤਾ, "= \t", "#")) {
		ਜੇ (file == ਨਿਰਾ)
			file = file_l;
		ਨਹੀਂਤਾਂ ਜੇ (!econf_call(econf_ਮਿਲਾਓ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ਮੁਕਤ (ਰਸਤਾ);
#ਜੇ_ਅੰਤ

	xasprintf(&ਰਸਤਾ, "/etc/%s", filename);

	ਜੇ (!econf_call(econf_readFile)(&file_l, ਰਸਤਾ, "= \t", "#")) {
		ਜੇ (file == ਨਿਰਾ)
			file = file_l;
		ਨਹੀਂਤਾਂ ਜੇ (!econf_call(econf_ਮਿਲਾਓ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ਨਹੀਂਤਾਂ ਜੇ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ਜੇ (file == ਨਿਰਾ)
			file = file_l;
		ਨਹੀਂਤਾਂ ਜੇ (!econf_call(econf_ਮਿਲਾਓ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ਮੁਕਤ (ਰਸਤਾ);
}

ਪੂਰਨ getlogindefs_bool(ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ dflt)
{
	bool ਮੁੱਲ;
	econf_err error;

	ਜੇ (!file)
		ਲੱਦੋ_defaults();

	ਜੇ (!file)
		ਵਾਪਸ dflt;

	ਜੇ ((error = econf_call(econf_get_Bool_ਮੁੱਲ)(file, ਨਿਰਾ, name, &ਮੁੱਲ))) {
		ਜੇ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ਵਾਪਸ dflt;
	}
	ਵਾਪਸ ਮੁੱਲ;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ getlogindefs_ਅੰਕ(ਅਚਲ ਅੱਖਰ *name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ dflt)
{
	uint64_t ਮੁੱਲ;
	econf_err error;

	ਜੇ (!file)
		ਲੱਦੋ_defaults();

	ਜੇ (!file)
		ਵਾਪਸ dflt;

	ਜੇ ((error = econf_call(econf_get_UInt64_ਮੁੱਲ)(file, ਨਿਰਾ, name, &ਮੁੱਲ))) {
		ਜੇ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ਵਾਪਸ dflt;
	}
	ਵਾਪਸ ਮੁੱਲ;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ਅਚਲ ਅੱਖਰ *getlogindefs_str(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *dflt)
{
	ਅੱਖਰ *ਮੁੱਲ;
	econf_err error;

	ਜੇ (!file)
		ਲੱਦੋ_defaults();

	ਜੇ (!file)
		ਵਾਪਸ dflt;

	ਜੇ ((error = econf_call(econf_get_String_ਮੁੱਲ)(file, ਨਿਰਾ, name, &ਮੁੱਲ))) {
		ਜੇ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ਵਾਪਸ dflt;
	}
	ਜੇ (ਮੁੱਲ)
		ਵਾਪਸ ਮੁੱਲ;

	ਵਾਪਸ xstrdup("");
}
#ਜੇ_ਅੰਤ /* !HAVE_LIBECONF */

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
ਪੂਰਨ logindefs_setenv(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *conf, ਅਚਲ ਅੱਖਰ *dflt)
{
	ਅਚਲ ਅੱਖਰ *val = getlogindefs_str(conf, dflt);
	ਅਚਲ ਅੱਖਰ *p;

	ਜੇ (!val)
		ਵਾਪਸ -1;

	p = strchr(val, '=');
	ਜੇ (p) {
		size_t sz = ਲੜੀ_ਲੰਬਾਈ(name);

		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ਜੇ (*val == '"')
				val++;
			ਜੇ (!*val)
				val = dflt;
		}
	}

	ਵਾਪਸ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ਪੂਰਨ logindefs_setenv_ਰਸਤਾ(uid_t uid)
{
	ਪੂਰਨ rc = 0;

	ਜੇ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ਨਹੀਂਤਾਂ ਜੇ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ਨਿਰਾ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", ਰਸਤਾ_DEFPATH_ਜੜ੍ਹ);

	ਵਾਪਸ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ਪੂਰਨ effective_access(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ ਬਹੁਲਕ)
{
	ਪੂਰਨ fd = open(ਰਸਤਾ, ਬਹੁਲਕ);
	ਜੇ (fd != -1)
		close(fd);
	ਵਾਪਸ fd == -1 ? -1 : 0;
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

ਪੂਰਨ get_hushlogin_ਸਥਿਤੀ(ਬਣਤਰ passwd *pwd, ਅਚਲ ਅੱਖਰ *override_home, ਪੂਰਨ force_check)
{
	ਅਚਲ ਅੱਖਰ *files[] = { ਰਸਤਾ_HUSHLOGINS, ਰਸਤਾ_HUSHLOGIN, ਨਿਰਾ };
	ਅਚਲ ਅੱਖਰ *file;
	ਅੱਖਰ buf[BUFSIZ];
	ਪੂਰਨ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ਨਿਰਾ);
	ਜੇ (file) {
		ਜੇ (!*file)
			ਵਾਪਸ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ਨਿਰਾ;
	}

	ਲਈ (i = 0; files[i]; i++) {
		ਪੂਰਨ ok = 0;

		file = files[i];

		/* global hush-file */
		ਜੇ (*file == '/') {
			ਬਣਤਰ stat st;
			FILE *f;

			ਜੇ (stat(file, &st) != 0)
				ਜਾਰੀ;	/* file does not exist */

			ਜੇ (st.st_size == 0)
				ਵਾਪਸ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ਜੇ (!f)
				ਜਾਰੀ;	/* ignore errors... */

			ਜਦੋਂ (ok == 0 && fgets(buf, ਆਕਾਰ(buf), f)) {
				ਜੇ (buf[0] != '\0')
					buf[ਲੜੀ_ਲੰਬਾਈ(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ਜੇ (ok)
				ਵਾਪਸ 1;	/* found username/shell */

			ਵਾਪਸ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ਜੇ (ਲੜੀ_ਲੰਬਾਈ(pwd->pw_dir) + ਲੜੀ_ਲੰਬਾਈ(file) + 2 > ਆਕਾਰ(buf))
			ਜਾਰੀ;

		ਜੇ (snprintf(buf, ਆਕਾਰ(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ਜਾਰੀ;

		ਜੇ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ਜੇ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ਜੇ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			}
			ਜੇ (ok)
				ਵਾਪਸ 1;	/* enabled by user */
		}
		ਨਹੀਂਤਾਂ {
			ਪੂਰਨ rc;
			rc = effective_access(buf, O_RDONLY);
			ਜੇ (rc == 0)
				ਵਾਪਸ 1;

			ਜੇ (rc == -1 && errno == EACCES)
				ਵਾਪਸ -1;
		}

	}

	ਵਾਪਸ 0;
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅੱਖਰ *name, *type;
	close_stdout_atexit();

	ਜੇ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_ਲੱਦੋ_file(argv[1]);

	ਜੇ (argc != 4) {	/* list all */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LIBECONF
		ਪੂਰਨ i;
		ਅੱਖਰ *ਕੁੰਜੀਆਂ[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ਨਿਰਾ};

		ਲਈ (i = 0; file && ਕੁੰਜੀਆਂ[i] != ਨਿਰਾ; i++) {
			ਅੱਖਰ *ਮੁੱਲ = ਨਿਰਾ;

			econf_call(econf_get_String_ਮੁੱਲ)(file, ਨਿਰਾ, ਕੁੰਜੀਆਂ[i], &ਮੁੱਲ);
			ਲਿਖੋ ("%s: $%s: '%s'\n", argv[1], ਕੁੰਜੀਆਂ[i], ਮੁੱਲ);
		}

		ਜੇ (file)
			econf_call(econf_freeFile)(file);

#ਨਹੀਂਤਾਂ
		ਬਣਤਰ ਵਸਤੂ *ptr;

		ਲਈ (ptr = list; ptr; ptr = ptr->ਅਗਲਾ)
			ਲਿਖੋ("%s: $%s: '%s'\n", ptr->ਰਸਤਾ, ptr->name,
			       ptr->ਮੁੱਲ);
#ਜੇ_ਅੰਤ
		ਵਾਪਸ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ਜੇ (strcmp(type, "str") == 0)
		ਲਿਖੋ("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(type, "num") == 0)
		ਲਿਖੋ("$%s: '%lu'\n", name, getlogindefs_ਅੰਕ(name, 0));
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(type, "bool") == 0)
		ਲਿਖੋ("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ
