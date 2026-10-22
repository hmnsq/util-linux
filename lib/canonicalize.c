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

	ᬬᬾᬦ᭄ (!ptname || !*ptname)
		ᬯᬮᬶ NULL;

	ᬬᬾᬦ᭄ (!prefix)
		prefix = "";

	snprintf(path, parimaa_na(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	ᬬᬾᬦ᭄ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ᬯᬮᬶ NULL;

	/* read "<name>\n" from sysfs */
	ᬬᬾᬦ᭄ (fgets(name, parimaa_na(name), f) && (sz = strlen(name)) > 1) {
		ᬬᬾᬦ᭄ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, parimaa_na(path), _PATH_DEV_MAPPER "/%s", name);

		ᬬᬾᬦ᭄ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ᬯᬮᬶ res;
}

var_na *ul_canonicalize_dm_name(const var_na *ptname)
{
	ᬯᬮᬶ ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static puur_naa_nka is_dm_devname(var_na *canonical, var_na **name)
{
	sa_nracanaa stat sb;
	var_na *p = strrchr(canonical, '/');

	*name = NULL;

	ᬬᬾᬦ᭄ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ᬯᬮᬶ 0;

	*name = p + 1;
	ᬯᬮᬶ 1;
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

	ᬬᬾᬦ᭄ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ᬯᬮᬶ NULL;
	}
	ᬬᬾᬦ᭄ (!getcwd(cwd, parimaa_na(cwd)))
		ᬯᬮᬶ NULL;

	/* simple clean up */
	ᬬᬾᬦ᭄ (ul_startswith(path, "./"))
		path += 2;
	ᬮᬶᬬ ᬬᬾᬦ᭄ (strcmp(path, ".") == 0)
		path = NULL;

	ᬬᬾᬦ᭄ (!path || !*path)
		ᬯᬮᬶ strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	ᬬᬾᬦ᭄ (!res)
		ᬯᬮᬶ NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ᬯᬮᬶ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static puur_naa_nka __attribute__((nonnull(2)))
do_canonicalize(const var_na *path, var_na **result,
		void *data __attribute__((__unused__)))
{
	var_na *canonical, *dmname;

	*result = NULL;

	ᬬᬾᬦ᭄ (!path || !*path) {
		errno = EINVAL;
		ᬯᬮᬶ -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	ᬬᬾᬦ᭄ (!canonical)
		ᬯᬮᬶ 1;

	ᬬᬾᬦ᭄ (is_dm_devname(canonical, &dmname)) {
		var_na *dm = ul_canonicalize_dm_name(dmname);
		ᬬᬾᬦ᭄ (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	ᬬᬾᬦ᭄ (canonical)
		*result = canonical;
	ᬯᬮᬶ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
var_na *ul_canonicalize_path(const var_na *path)
{
	var_na *canonical = NULL;

	ᬬᬾᬦ᭄ (do_canonicalize(path, &canonical, NULL) == 1)
		ᬯᬮᬶ strdup(path);

	ᬯᬮᬶ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
var_na *ul_canonicalize_path_restricted(const var_na *path)
{
	ᬯᬮᬶ ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	var_na *p;

	ᬬᬾᬦ᭄ (argc < 2) {
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
