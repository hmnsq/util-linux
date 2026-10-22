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

	もし (!ptname || !*ptname)
		戻る NULL;

	もし (!prefix)
		prefix = "";

	snprintf(path, sizeof(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	もし (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		戻る NULL;

	/* read "<name>\n" from sysfs */
	もし (fgets(name, sizeof(name), f) && (sz = strlen(name)) > 1) {
		もし (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, sizeof(path), _PATH_DEV_MAPPER "/%s", name);

		もし ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	戻る res;
}

char *ul_canonicalize_dm_name(const char *ptname)
{
	戻る ul_canonicalize_dm_name_prefixed(NULL, ptname);
}

static 整数 is_dm_devname(char *canonical, char **name)
{
	struct stat sb;
	char *p = strrchr(canonical, '/');

	*name = NULL;

	もし (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		戻る 0;

	*name = p + 1;
	戻る 1;
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

	もし (!ul_is_relative_path(path)) {
		errno = EINVAL;
		戻る NULL;
	}
	もし (!getcwd(cwd, sizeof(cwd)))
		戻る NULL;

	/* simple clean up */
	もし (ul_startswith(path, "./"))
		path += 2;
	否则 もし (strcmp(path, ".") == 0)
		path = NULL;

	もし (!path || !*path)
		戻る strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	もし (!res)
		戻る NULL;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	戻る res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
static 整数 __attribute__((nonnull(2)))
do_canonicalize(const char *path, char **result,
		void *data __attribute__((__unused__)))
{
	char *canonical, *dmname;

	*result = NULL;

	もし (!path || !*path) {
		errno = EINVAL;
		戻る -errno;
	}

	errno = 0;
	canonical = realpath(path, NULL);
	もし (!canonical)
		戻る 1;

	もし (is_dm_devname(canonical, &dmname)) {
		char *dm = ul_canonicalize_dm_name(dmname);
		もし (dm) {
			free(canonical);
			canonical = dm;
		}
	}

	もし (canonical)
		*result = canonical;
	戻る 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
char *ul_canonicalize_path(const char *path)
{
	char *canonical = NULL;

	もし (do_canonicalize(path, &canonical, NULL) == 1)
		戻る strdup(path);

	戻る canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
char *ul_canonicalize_path_restricted(const char *path)
{
	戻る ul_restricted_path_oper(path, do_canonicalize, NULL);
}

#ifdef TEST_PROGRAM_CANONICALIZE
整数 メインページ(整数 argc, char **argv)
{
	char *p;

	もし (argc < 2) {
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
