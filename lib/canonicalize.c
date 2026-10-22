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
char *ul_canonicalize_dm_name_prefixed(const char *prefix, const char *ptname)
{
	FILE	*f;
	size_t	sz;
	char	path[256], name[sizeof(path) - sizeof(_PATH_DEV_MAPPER)], *res = NULL;

	если (!ptname || !*ptname)
		вернуть NULL;

	если (!prefix)
		prefix = "";

	snprintf(path, sizeof(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	если (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		вернуть NULL;

	/* read "<name>\n" from sysfs */
	если (fgets(name, sizeof(name), f) && (sz = strlen(name)) > 1) {
		если (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, sizeof(path), _PATH_DEV_MAPPER "/%s", name);

		если ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	вернуть res;
}

char *ul_canonicalize_dm_name(const char *ptname)
{
	вернуть ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static целое is_dm_devname(char *canonical, char **name)
{
	struct stat sb;
	char *p = strrchr(canonical, '/');

	*name = NULL;

	если (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		вернуть 0;

	*name = p + 1;
	вернуть 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
char *ul_absolute_path(const char *path)
{
	char cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	если (!ul_is_relative_path(path)) {
		errno = EINVAL;
		вернуть NULL;
	}
	если (!getcwd(cwd, sizeof(cwd)))
		вернуть NULL;

	/* simple clean up */
	если (ul_startswith(path, "./"))
		path += 2;
	иначе если (strcmp(path, ".") == 0)
		path = NULL;

	если (!path || !*path)
		вернуть strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	если (!res)
		вернуть NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	вернуть res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static целое __attribute__((nonnull(2)))
do_canonicalize(const char *path, char **result,
		void *data __attribute__((__unused__)))
{
	char *canonical, *dmname;

	*result = NULL;

	если (!path || !*path) {
		errno = EINVAL;
		вернуть -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	если (!canonical)
		вернуть 1;

	если (is_dm_devname(canonical, &dmname)) {
		char *dm = ul_canonicalize_dm_name(dmname);
		если (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	если (canonical)
		*result = canonical;
	вернуть 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
char *ul_canonicalize_path(const char *path)
{
	char *canonical = NULL;

	если (do_canonicalize(path, &canonical, NULL) == 1)
		вернуть strdup(path);

	вернуть canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
char *ul_canonicalize_path_restricted(const char *path)
{
	вернуть ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
целое основной(целое argc, char **argv)
{
	char *p;

	если (argc < 2) {
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
