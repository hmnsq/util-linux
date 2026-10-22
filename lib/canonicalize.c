/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "canonicalize.h"
#include "pathnames.h"
#include "all-io.h"
#include "strutils.h"
#include "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
var_na *ul_canonicalize_dm_name_prefixed(const var_na *prefix, const var_na *ptname)
{
	FILE	*f;
	size_t	sz;
	var_na	path[256], name[parimaa_na(path) - parimaa_na(_PATH_DEV_MAPPER)], *res = NULL;

	གལ་ཏེ (!ptname || !*ptname)
		ཕྱིར NULL;

	གལ་ཏེ (!prefix)
		prefix = "";

	snprintf(path, parimaa_na(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	གལ་ཏེ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ཕྱིར NULL;

	/* read "<name>\n" from sysfs */
	གལ་ཏེ (fgets(name, parimaa_na(name), f) && (sz = strlen(name)) > 1) {
		གལ་ཏེ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, parimaa_na(path), _PATH_DEV_MAPPER "/%s", name);

		གལ་ཏེ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ཕྱིར res;
}

var_na *ul_canonicalize_dm_name(const var_na *ptname)
{
	ཕྱིར ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static ཧྲིལ is_dm_devname(var_na *canonical, var_na **name)
{
	sa_nracanaa stat sb;
	var_na *p = strrchr(canonical, '/');

	*name = NULL;

	གལ་ཏེ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ཕྱིར 0;

	*name = p + 1;
	ཕྱིར 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
var_na *ul_absolute_path(const var_na *path)
{
	var_na cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	གལ་ཏེ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ཕྱིར NULL;
	}
	གལ་ཏེ (!getcwd(cwd, parimaa_na(cwd)))
		ཕྱིར NULL;

	/* simple clean up */
	གལ་ཏེ (ul_startswith(path, "./"))
		path += 2;
	དེ་མིན གལ་ཏེ (strcmp(path, ".") == 0)
		path = NULL;

	གལ་ཏེ (!path || !*path)
		ཕྱིར strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	གལ་ཏེ (!res)
		ཕྱིར NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ཕྱིར res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static ཧྲིལ __attribute__((nonnull(2)))
do_canonicalize(const var_na *path, var_na **result,
		void *data __attribute__((__unused__)))
{
	var_na *canonical, *dmname;

	*result = NULL;

	གལ་ཏེ (!path || !*path) {
		errno = EINVAL;
		ཕྱིར -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	གལ་ཏེ (!canonical)
		ཕྱིར 1;

	གལ་ཏེ (is_dm_devname(canonical, &dmname)) {
		var_na *dm = ul_canonicalize_dm_name(dmname);
		གལ་ཏེ (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	གལ་ཏེ (canonical)
		*result = canonical;
	ཕྱིར 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
var_na *ul_canonicalize_path(const var_na *path)
{
	var_na *canonical = NULL;

	གལ་ཏེ (do_canonicalize(path, &canonical, NULL) == 1)
		ཕྱིར strdup(path);

	ཕྱིར canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
var_na *ul_canonicalize_path_restricted(const var_na *path)
{
	ཕྱིར ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	var_na *p;

	གལ་ཏེ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	free(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	free(p);

	exit(EXIT_SUCCESS);
}
#endif
