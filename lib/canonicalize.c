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

	ຖ້າ (!ptname || !*ptname)
		ສົ່ງຄືນ NULL;

	ຖ້າ (!prefix)
		prefix = "";

	snprintf(path, parimaa_na(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	ຖ້າ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ສົ່ງຄືນ NULL;

	/* read "<name>\n" from sysfs */
	ຖ້າ (fgets(name, parimaa_na(name), f) && (sz = strlen(name)) > 1) {
		ຖ້າ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, parimaa_na(path), _PATH_DEV_MAPPER "/%s", name);

		ຖ້າ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ສົ່ງຄືນ res;
}

var_na *ul_canonicalize_dm_name(const var_na *ptname)
{
	ສົ່ງຄືນ ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static ຈຳນວນເຕັມ is_dm_devname(var_na *canonical, var_na **name)
{
	sa_nracanaa stat sb;
	var_na *p = strrchr(canonical, '/');

	*name = NULL;

	ຖ້າ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ສົ່ງຄືນ 0;

	*name = p + 1;
	ສົ່ງຄືນ 1;
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

	ຖ້າ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ສົ່ງຄືນ NULL;
	}
	ຖ້າ (!getcwd(cwd, parimaa_na(cwd)))
		ສົ່ງຄືນ NULL;

	/* simple clean up */
	ຖ້າ (ul_startswith(path, "./"))
		path += 2;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(path, ".") == 0)
		path = NULL;

	ຖ້າ (!path || !*path)
		ສົ່ງຄືນ strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	ຖ້າ (!res)
		ສົ່ງຄືນ NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ສົ່ງຄືນ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static ຈຳນວນເຕັມ __attribute__((nonnull(2)))
do_canonicalize(const var_na *path, var_na **result,
		void *data __attribute__((__unused__)))
{
	var_na *canonical, *dmname;

	*result = NULL;

	ຖ້າ (!path || !*path) {
		errno = EINVAL;
		ສົ່ງຄືນ -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	ຖ້າ (!canonical)
		ສົ່ງຄືນ 1;

	ຖ້າ (is_dm_devname(canonical, &dmname)) {
		var_na *dm = ul_canonicalize_dm_name(dmname);
		ຖ້າ (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	ຖ້າ (canonical)
		*result = canonical;
	ສົ່ງຄືນ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
var_na *ul_canonicalize_path(const var_na *path)
{
	var_na *canonical = NULL;

	ຖ້າ (do_canonicalize(path, &canonical, NULL) == 1)
		ສົ່ງຄືນ strdup(path);

	ສົ່ງຄືນ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
var_na *ul_canonicalize_path_restricted(const var_na *path)
{
	ສົ່ງຄືນ ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	var_na *p;

	ຖ້າ (argc < 2) {
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
