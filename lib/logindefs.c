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


ಸ್ಥಿರ ಶೂನ್ಯ (*logindefs_loader)(ಶೂನ್ಯ *) = ಖಾಲಿ;
ಸ್ಥಿರ ಶೂನ್ಯ *logindefs_loader_ದತ್ತಾಂಶ = ಖಾಲಿ;

ಶೂನ್ಯ logindefs_ಗಣ_loader(ಶೂನ್ಯ (*loader)(ಶೂನ್ಯ *ದತ್ತಾಂಶ), ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	logindefs_loader = loader;
	logindefs_loader_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_LIBECONF

ರಚನೆ ಅಂಶ {
	ಅಕ್ಷರ *name;		/* name of the option.  */
	ಅಕ್ಷರ *ಮೌಲ್ಯ;		/* value of the option.  */
	ಅಕ್ಷರ *ಮಾರ್ಗ;		/* name of config file for this option.  */

	ರಚನೆ ಅಂಶ *ಮುಂದಿನ;	/* pointer to next option.  */
};

ಸ್ಥಿರ ರಚನೆ ಅಂಶ *list = ಖಾಲಿ;

ಶೂನ್ಯ free_getlogindefs_ದತ್ತಾಂಶ(ಶೂನ್ಯ)
{
	ರಚನೆ ಅಂಶ *ptr;

	ptr = list;
	ಆಗ (ptr) {
		ರಚನೆ ಅಂಶ *tmp = ptr->ಮುಂದಿನ;

		ಬಿಡುಗಡೆ(ptr->ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(ptr->name);
		ಬಿಡುಗಡೆ(ptr->ಮೌಲ್ಯ);
		ಬಿಡುಗಡೆ(ptr);
		ptr = tmp;
	}

	list = ಖಾಲಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ store(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ರಚನೆ ಅಂಶ *new = xmalloc(ಗಾತ್ರ(ರಚನೆ ಅಂಶ));

	ಆದರೆ (!name)
		abort();

	new->name = xstrdup(name);
	new->ಮೌಲ್ಯ = ಮೌಲ್ಯ && *ಮೌಲ್ಯ ? xstrdup(ಮೌಲ್ಯ) : ಖಾಲಿ;
	new->ಮಾರ್ಗ = xstrdup(ಮಾರ್ಗ);
	new->ಮುಂದಿನ = list;
	list = new;
}

ಶೂನ್ಯ logindefs_ತುಂಬು_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	FILE *f;
	ಅಕ್ಷರ buf[BUFSIZ];

	f = fopen(filename, "r" UL_CLOEXECSTR);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ;

	ಆಗ (fgets(buf, ಗಾತ್ರ(buf), f)) {

		ಅಕ್ಷರ *p, *name, *ದತ್ತಾಂಶ = ಖಾಲಿ;

		ಆದರೆ (*buf == '#' || *buf == '\n')
			ಮುಂದುವರಿ;	/* only comment or empty line */

		p = strchr(buf, '#');
		ಆದರೆ (p)
			*p = '\0';
		ಇಲ್ಲದಿದ್ದರೆ {
			size_t n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);
			ಆದರೆ (n && *(buf + n - 1) == '\n')
				*(buf + n - 1) = '\0';
		}

		ಆದರೆ (!*buf)
			ಮುಂದುವರಿ;	/* empty line */

		/* ignore space at begin of the line */
		name = buf;
		ಆಗ (*name && ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ)*name))
			name++;

		/* go to the end of the name */
		ದತ್ತಾಂಶ = name;
		ಆಗ (*ದತ್ತಾಂಶ && !(ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ)*ದತ್ತಾಂಶ) || *ದತ್ತಾಂಶ == '='))
			ದತ್ತಾಂಶ++;
		ಆದರೆ (ದತ್ತಾಂಶ > name && *ದತ್ತಾಂಶ)
			*ದತ್ತಾಂಶ++ = '\0';

		ಆದರೆ (!*name || ದತ್ತಾಂಶ == name)
			ಮುಂದುವರಿ;

		/* go to the begin of the value */
		ಆಗ (*ದತ್ತಾಂಶ
		       && (ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ)*ದತ್ತಾಂಶ) || *ದತ್ತಾಂಶ == '='
			   || *ದತ್ತಾಂಶ == '"'))
			ದತ್ತಾಂಶ++;

		/* remove space at the end of the value */
		p = ದತ್ತಾಂಶ + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ);
		ಆದರೆ (p > ದತ್ತಾಂಶ)
			p--;
		ಆಗ (p > ದತ್ತಾಂಶ && (ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ)*p) || *p == '"'))
			*p-- = '\0';

		store(name, ದತ್ತಾಂಶ, filename);
	}

	fclose(f);
}

ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_defaults(ಶೂನ್ಯ)
{
	ಆದರೆ (logindefs_loader)
		logindefs_loader(logindefs_loader_ದತ್ತಾಂಶ);
	ಇಲ್ಲದಿದ್ದರೆ
		logindefs_ತುಂಬು_file(ಮಾರ್ಗ_LOGINDEFS);
}

ಸ್ಥಿರ ರಚನೆ ಅಂಶ *ಹುಡುಕು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ ಅಂಶ *ptr;

	ಆದರೆ (!list)
		ತುಂಬು_defaults();

	ptr = list;
	ಆಗ (ptr != ಖಾಲಿ) {
		ಆದರೆ (c_strcasecmp(name, ptr->name) == 0)
			ಹಿಂತಿರುಗಿ ptr;
		ptr = ptr->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಹುಡುಕು_config(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ ಅಂಶ *ptr;

	ptr = list;
	ಆಗ (ptr != ಖಾಲಿ) {
		ಆದರೆ (c_strcasecmp(name, ptr->name) == 0)
			ಹಿಂತಿರುಗಿ ptr->ಮಾರ್ಗ;
		ptr = ptr->ಮುಂದಿನ;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ getlogindefs_bool(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ dflt)
{
	ರಚನೆ ಅಂಶ *ptr = ಹುಡುಕು(name);
	ಹಿಂತಿರುಗಿ ptr && ptr->ಮೌಲ್ಯ ? (c_strcasecmp(ptr->ಮೌಲ್ಯ, "yes") == 0) : dflt;
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ getlogindefs_ಅಂಕೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ dflt)
{
	ರಚನೆ ಅಂಶ *ptr = ಹುಡುಕು(name);
	ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ retval;

	ಆದರೆ (!ptr || !ptr->ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ dflt;

	errno = 0;
	retval = strtoul(ptr->ಮೌಲ್ಯ, &ಕೊನೆ, 0);
	ಆದರೆ (ಕೊನೆ && *ಕೊನೆ == '\0' && !errno)
		ಹಿಂತಿರುಗಿ retval;

	syslog(LOG_NOTICE, _("%s: %s contains invalid numerical value: %s"),
	       ಹುಡುಕು_config(name), name, ptr->ಮೌಲ್ಯ);
	ಹಿಂತಿರುಗಿ dflt;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getlogindefs_str(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt)
{
	ರಚನೆ ಅಂಶ *ptr = ಹುಡುಕು(name);

	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ dflt;
	ಆದರೆ (!ptr->ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ "";
	ಹಿಂತಿರುಗಿ ptr->ಮೌಲ್ಯ;
}

#ಇಲ್ಲದಿದ್ದರೆ /* !HAVE_LIBECONF */

#ಅನ್ತರ್ಭಾವ "dl-econf.h"

ಸ್ಥಿರ econf_file *file = ಖಾಲಿ;

ಶೂನ್ಯ free_getlogindefs_ದತ್ತಾಂಶ(ಶೂನ್ಯ)
{
	ಆದರೆ (file)
		econf_call(econf_freeFile)(file);
	file = ಖಾಲಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_defaults(ಶೂನ್ಯ)
{
	econf_err error;

	ಆದರೆ (file != ಖಾಲಿ)
		free_getlogindefs_ದತ್ತಾಂಶ();

	ಆದರೆ (ul_dlopen_libeconf() == 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ECONF_READCONFIG
		error = econf_call(econf_readConfig)(&file, ಖಾಲಿ,
				UL_VENDORDIR_ಮಾರ್ಗ, "login", "defs", "= \t", "#");
#ಇಲ್ಲದಿದ್ದರೆ
		error = econf_call(econf_readDirs)(&file,
				UL_VENDORDIR_ಮಾರ್ಗ, "/etc", "login", "defs", "= \t", "#");
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (error)
			syslog(LOG_NOTICE, _("Error reading login.defs: %s"),
			       econf_call(econf_errString)(error));
	}

	ಆದರೆ (logindefs_loader)
		logindefs_loader(logindefs_loader_ದತ್ತಾಂಶ);

}

ಶೂನ್ಯ logindefs_ತುಂಬು_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	econf_file *file_l = ಖಾಲಿ, *file_m = ಖಾಲಿ;
	ಅಕ್ಷರ *ಮಾರ್ಗ;

	logindefs_loader = ಖಾಲಿ; /* No recursion */

	ಆದರೆ (ul_dlopen_libeconf() != 0)
		ಹಿಂತಿರುಗಿ;

#ಆದರೆ USE_VENDORDIR
	xasprintf(&ಮಾರ್ಗ, ಮಾರ್ಗ_VENDORDIR"/%s", filename);

	ಆದರೆ (!econf_call(econf_readFile)(&file_l, ಮಾರ್ಗ, "= \t", "#")) {
		ಆದರೆ (file == ಖಾಲಿ)
			file = file_l;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!econf_call(econf_ವಿಲೀನ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ಬಿಡುಗಡೆ (ಮಾರ್ಗ);
#ಆದರೆ_ಅಂತ್ಯ

	xasprintf(&ಮಾರ್ಗ, "/etc/%s", filename);

	ಆದರೆ (!econf_call(econf_readFile)(&file_l, ಮಾರ್ಗ, "= \t", "#")) {
		ಆದರೆ (file == ಖಾಲಿ)
			file = file_l;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!econf_call(econf_ವಿಲೀನ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}

	/* Try original filename, could be relative */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!econf_call(econf_readFile)(&file_l, filename, "= \t", "#")) {
		ಆದರೆ (file == ಖಾಲಿ)
			file = file_l;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!econf_call(econf_ವಿಲೀನ_Files)(&file_m, file, file_l)) {
			econf_call(econf_freeFile)(file);
			file = file_m;
			econf_call(econf_freeFile)(file_l);
		}
	}
	ಬಿಡುಗಡೆ (ಮಾರ್ಗ);
}

ಪೂರ್ಣಾಂಕ getlogindefs_bool(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ dflt)
{
	bool ಮೌಲ್ಯ;
	econf_err error;

	ಆದರೆ (!file)
		ತುಂಬು_defaults();

	ಆದರೆ (!file)
		ಹಿಂತಿರುಗಿ dflt;

	ಆದರೆ ((error = econf_call(econf_get_Bool_ಮೌಲ್ಯ)(file, ಖಾಲಿ, name, &ಮೌಲ್ಯ))) {
		ಆದರೆ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಹಿಂತಿರುಗಿ dflt;
	}
	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ getlogindefs_ಅಂಕೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ dflt)
{
	uint64_t ಮೌಲ್ಯ;
	econf_err error;

	ಆದರೆ (!file)
		ತುಂಬು_defaults();

	ಆದರೆ (!file)
		ಹಿಂತಿರುಗಿ dflt;

	ಆದರೆ ((error = econf_call(econf_get_UInt64_ಮೌಲ್ಯ)(file, ಖಾಲಿ, name, &ಮೌಲ್ಯ))) {
		ಆದರೆ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಹಿಂತಿರುಗಿ dflt;
	}
	ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;
}

/*
 * Returns:
 *	@dflt		if @name not found
 *	""		(empty string) if found, but value not defined
 *	"string"	if found
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getlogindefs_str(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt)
{
	ಅಕ್ಷರ *ಮೌಲ್ಯ;
	econf_err error;

	ಆದರೆ (!file)
		ತುಂಬು_defaults();

	ಆದರೆ (!file)
		ಹಿಂತಿರುಗಿ dflt;

	ಆದರೆ ((error = econf_call(econf_get_String_ಮೌಲ್ಯ)(file, ಖಾಲಿ, name, &ಮೌಲ್ಯ))) {
		ಆದರೆ (error != ECONF_NOKEY)
			syslog(LOG_NOTICE, _("couldn't fetch %s: %s"), name,
			       econf_call(econf_errString)(error));
		ಹಿಂತಿರುಗಿ dflt;
	}
	ಆದರೆ (ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;

	ಹಿಂತಿರುಗಿ xstrdup("");
}
#ಆದರೆ_ಅಂತ್ಯ /* !HAVE_LIBECONF */

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
ಪೂರ್ಣಾಂಕ logindefs_setenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *conf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dflt)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val = getlogindefs_str(conf, dflt);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	ಆದರೆ (!val)
		ಹಿಂತಿರುಗಿ -1;

	p = strchr(val, '=');
	ಆದರೆ (p) {
		size_t sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);

		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(val, name, sz) == 0 && *(p + 1)) {
			val = p + 1;
			ಆದರೆ (*val == '"')
				val++;
			ಆದರೆ (!*val)
				val = dflt;
		}
	}

	ಹಿಂತಿರುಗಿ val ? setenv(name, val, 1) : -1;
}

/*
 * logindefs based setenv("PATH")
 */
ಪೂರ್ಣಾಂಕ logindefs_setenv_ಮಾರ್ಗ(uid_t uid)
{
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (uid)
		rc = logindefs_setenv("PATH", "ENV_PATH", _PATH_DEFPATH);

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((rc = logindefs_setenv("PATH", "ENV_SUPATH", ಖಾಲಿ)) != 0)
		rc = logindefs_setenv("PATH", "ENV_ROOTPATH", ಮಾರ್ಗ_DEFPATH_ಬೇರು);

	ಹಿಂತಿರುಗಿ rc;
}

/*
 * We need to check the effective UID/GID. For example, $HOME could be on a
 * root-squashed NFS or on an NFS with UID mapping, and access(2) uses the
 * real UID/GID.  Then open(2) seems as the surest solution.
 * -- kzak@redhat.com (10-Apr-2009)
 */
ಪೂರ್ಣಾಂಕ effective_access(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ fd = open(ಮಾರ್ಗ, ಬಹುಲಕ);
	ಆದರೆ (fd != -1)
		close(fd);
	ಹಿಂತಿರುಗಿ fd == -1 ? -1 : 0;
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

ಪೂರ್ಣಾಂಕ get_hushlogin_ಸ್ಥಿತಿ(ರಚನೆ passwd *pwd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *override_home, ಪೂರ್ಣಾಂಕ force_check)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *files[] = { ಮಾರ್ಗ_HUSHLOGINS, ಮಾರ್ಗ_HUSHLOGIN, ಖಾಲಿ };
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file;
	ಅಕ್ಷರ buf[BUFSIZ];
	ಪೂರ್ಣಾಂಕ i;

	file = getlogindefs_str("HUSHLOGIN_FILE", ಖಾಲಿ);
	ಆದರೆ (file) {
		ಆದರೆ (!*file)
			ಹಿಂತಿರುಗಿ 0;	/* empty HUSHLOGIN_FILE defined */

		files[0] = file;
		files[1] = ಖಾಲಿ;
	}

	ಗಾಗಿ (i = 0; files[i]; i++) {
		ಪೂರ್ಣಾಂಕ ok = 0;

		file = files[i];

		/* global hush-file */
		ಆದರೆ (*file == '/') {
			ರಚನೆ stat st;
			FILE *f;

			ಆದರೆ (stat(file, &st) != 0)
				ಮುಂದುವರಿ;	/* file does not exist */

			ಆದರೆ (st.st_size == 0)
				ಹಿಂತಿರುಗಿ 1;	/* for all accounts */

			f = fopen(file, "r" UL_CLOEXECSTR);
			ಆದರೆ (!f)
				ಮುಂದುವರಿ;	/* ignore errors... */

			ಆಗ (ok == 0 && fgets(buf, ಗಾತ್ರ(buf), f)) {
				ಆದರೆ (buf[0] != '\0')
					buf[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf) - 1] = '\0';
				ok = !strcmp(buf, *buf == '/' ? pwd->pw_shell :
								pwd->pw_name);
			}
			fclose(f);
			ಆದರೆ (ok)
				ಹಿಂತಿರುಗಿ 1;	/* found username/shell */

			ಹಿಂತಿರುಗಿ 0;		/* ignore per-account files */
		}

		/* per-account setting */
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(pwd->pw_dir) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(file) + 2 > ಗಾತ್ರ(buf))
			ಮುಂದುವರಿ;

		ಆದರೆ (snprintf(buf, ಗಾತ್ರ(buf), "%s/%s", override_home ?: pwd->pw_dir, file) < 0)
			ಮುಂದುವರಿ;

		ಆದರೆ (force_check) {
			uid_t ruid = getuid();
			gid_t egid = getegid();

			ಆದರೆ (setregid(-1, pwd->pw_gid) == 0 &&
			    setreuid(0, pwd->pw_uid) == 0)
				ok = effective_access(buf, O_RDONLY) == 0;

			ಆದರೆ (setuid(0) != 0 ||
			    setreuid(ruid, 0) != 0 ||
			    setregid(-1, egid) != 0) {
				syslog(LOG_ALERT, _("hush login status: restore original IDs failed"));
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			}
			ಆದರೆ (ok)
				ಹಿಂತಿರುಗಿ 1;	/* enabled by user */
		}
		ಇಲ್ಲದಿದ್ದರೆ {
			ಪೂರ್ಣಾಂಕ rc;
			rc = effective_access(buf, O_RDONLY);
			ಆದರೆ (rc == 0)
				ಹಿಂತಿರುಗಿ 1;

			ಆದರೆ (rc == -1 && errno == EACCES)
				ಹಿಂತಿರುಗಿ -1;
		}

	}

	ಹಿಂತಿರುಗಿ 0;
}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಅಕ್ಷರ *name, *type;
	close_stdout_atexit();

	ಆದರೆ (argc <= 1)
		errx(EXIT_FAILURE, "usage: %s <filename> "
		     "[<str|num|bool> <valname>]", argv[0]);

	logindefs_ತುಂಬು_file(argv[1]);

	ಆದರೆ (argc != 4) {	/* list all */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LIBECONF
		ಪೂರ್ಣಾಂಕ i;
		ಅಕ್ಷರ *ಕೀಲಿಗಳು[] = {"END", "EMPTY", "CRAZY3", "CRAZY2", "CRAZY1",
				"BOOLEAN", "NUMBER", "STRING", "HELLO_WORLD",
				ಖಾಲಿ};

		ಗಾಗಿ (i = 0; file && ಕೀಲಿಗಳು[i] != ಖಾಲಿ; i++) {
			ಅಕ್ಷರ *ಮೌಲ್ಯ = ಖಾಲಿ;

			econf_call(econf_get_String_ಮೌಲ್ಯ)(file, ಖಾಲಿ, ಕೀಲಿಗಳು[i], &ಮೌಲ್ಯ);
			ಬರೆ ("%s: $%s: '%s'\n", argv[1], ಕೀಲಿಗಳು[i], ಮೌಲ್ಯ);
		}

		ಆದರೆ (file)
			econf_call(econf_freeFile)(file);

#ಇಲ್ಲದಿದ್ದರೆ
		ರಚನೆ ಅಂಶ *ptr;

		ಗಾಗಿ (ptr = list; ptr; ptr = ptr->ಮುಂದಿನ)
			ಬರೆ("%s: $%s: '%s'\n", ptr->ಮಾರ್ಗ, ptr->name,
			       ptr->ಮೌಲ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	type = argv[2];
	name = argv[3];

	ಆದರೆ (strcmp(type, "str") == 0)
		ಬರೆ("$%s: '%s'\n", name, getlogindefs_str(name, "DEFAULT"));
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(type, "num") == 0)
		ಬರೆ("$%s: '%lu'\n", name, getlogindefs_ಅಂಕೆ(name, 0));
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(type, "bool") == 0)
		ಬರೆ("$%s: '%s'\n", name,
		       getlogindefs_bool(name, 0) ? "Y" : "N");

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ
