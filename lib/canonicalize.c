/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>

#അന്തര്ഭാവ "canonicalize.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
അക്ഷരം *ul_canonicalize_dm_name_prefixed(സ്ഥിരം അക്ഷരം *prefix, സ്ഥിരം അക്ഷരം *ptname)
{
	FILE	*f;
	size_t	sz;
	അക്ഷരം	path[256], name[വലുപ്പം(path) - വലുപ്പം(_PATH_DEV_MAPPER)], *res = ശൂന്യ;

	എങ്കിൽ (!ptname || !*ptname)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!prefix)
		prefix = "";

	snprintf(path, വലുപ്പം(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	എങ്കിൽ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		തിരികെ ശൂന്യ;

	/* read "<name>\n" from sysfs */
	എങ്കിൽ (fgets(name, വലുപ്പം(name), f) && (sz = strlen(name)) > 1) {
		എങ്കിൽ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, വലുപ്പം(path), _PATH_DEV_MAPPER "/%s", name);

		എങ്കിൽ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	തിരികെ res;
}

അക്ഷരം *ul_canonicalize_dm_name(സ്ഥിരം അക്ഷരം *ptname)
{
	തിരികെ ul_canonicalize_dm_name_prefixed(ശൂന്യ, ptname);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ is_dm_devname(അക്ഷരം *canonical, അക്ഷരം **name)
{
	ഘടന stat sb;
	അക്ഷരം *p = strrchr(canonical, '/');

	*name = ശൂന്യ;

	എങ്കിൽ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		തിരികെ 0;

	*name = p + 1;
	തിരികെ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
അക്ഷരം *ul_absolute_path(സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	എങ്കിൽ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		തിരികെ ശൂന്യ;
	}
	എങ്കിൽ (!getcwd(cwd, വലുപ്പം(cwd)))
		തിരികെ ശൂന്യ;

	/* simple clean up */
	എങ്കിൽ (ul_startswith(path, "./"))
		path += 2;
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(path, ".") == 0)
		path = ശൂന്യ;

	എങ്കിൽ (!path || !*path)
		തിരികെ strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	എങ്കിൽ (!res)
		തിരികെ ശൂന്യ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	തിരികെ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ __attribute__((nonnull(2)))
do_canonicalize(സ്ഥിരം അക്ഷരം *path, അക്ഷരം **result,
		ശൂന്യം *data __attribute__((__unused__)))
{
	അക്ഷരം *canonical, *dmname;

	*result = ശൂന്യ;

	എങ്കിൽ (!path || !*path) {
		errno = EINVAL;
		തിരികെ -errno;
	}

	errno = 0;
	canonical = realpath(path, ശൂന്യ);
	എങ്കിൽ (!canonical)
		തിരികെ 1;

	എങ്കിൽ (is_dm_devname(canonical, &dmname)) {
		അക്ഷരം *dm = ul_canonicalize_dm_name(dmname);
		എങ്കിൽ (dm) {
			സ്വതന്ത്രം(canonical);
			canonical = dm;
		}
	}

	എങ്കിൽ (canonical)
		*result = canonical;
	തിരികെ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
അക്ഷരം *ul_canonicalize_path(സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം *canonical = ശൂന്യ;

	എങ്കിൽ (do_canonicalize(path, &canonical, ശൂന്യ) == 1)
		തിരികെ strdup(path);

	തിരികെ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
അക്ഷരം *ul_canonicalize_path_restricted(സ്ഥിരം അക്ഷരം *path)
{
	തിരികെ ul_restricted_path_oper(path, do_canonicalize, ശൂന്യ);
}

#ifdef TEST_PROGRAM_CANONICALIZE
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	അക്ഷരം *p;

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	സ്വതന്ത്രം(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	സ്വതന്ത്രം(p);

	exit(EXIT_SUCCESS);
}
#endif
