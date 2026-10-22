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

	නම් (!ptname || !*ptname)
		ආපසු NULL;

	නම් (!prefix)
		prefix = "";

	snprintf(path, parimaa_na(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	නම් (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ආපසු NULL;

	/* read "<name>\n" from sysfs */
	නම් (fgets(name, parimaa_na(name), f) && (sz = strlen(name)) > 1) {
		නම් (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, parimaa_na(path), _PATH_DEV_MAPPER "/%s", name);

		නම් ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ආපසු res;
}

var_na *ul_canonicalize_dm_name(const var_na *ptname)
{
	ආපසු ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static පූර්ණ_සංඛ්‍යා is_dm_devname(var_na *canonical, var_na **name)
{
	sa_nracanaa stat sb;
	var_na *p = strrchr(canonical, '/');

	*name = NULL;

	නම් (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ආපසු 0;

	*name = p + 1;
	ආපසු 1;
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

	නම් (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ආපසු NULL;
	}
	නම් (!getcwd(cwd, parimaa_na(cwd)))
		ආපසු NULL;

	/* simple clean up */
	නම් (ul_startswith(path, "./"))
		path += 2;
	නැතහොත් නම් (strcmp(path, ".") == 0)
		path = NULL;

	නම් (!path || !*path)
		ආපසු strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	නම් (!res)
		ආපසු NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ආපසු res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static පූර්ණ_සංඛ්‍යා __attribute__((nonnull(2)))
do_canonicalize(const var_na *path, var_na **result,
		void *data __attribute__((__unused__)))
{
	var_na *canonical, *dmname;

	*result = NULL;

	නම් (!path || !*path) {
		errno = EINVAL;
		ආපසු -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	නම් (!canonical)
		ආපසු 1;

	නම් (is_dm_devname(canonical, &dmname)) {
		var_na *dm = ul_canonicalize_dm_name(dmname);
		නම් (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	නම් (canonical)
		*result = canonical;
	ආපසු 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
var_na *ul_canonicalize_path(const var_na *path)
{
	var_na *canonical = NULL;

	නම් (do_canonicalize(path, &canonical, NULL) == 1)
		ආපසු strdup(path);

	ආපසු canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
var_na *ul_canonicalize_path_restricted(const var_na *path)
{
	ආපසු ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	var_na *p;

	නම් (argc < 2) {
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
