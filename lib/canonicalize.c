/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#شامل <stdio.h>
#شامل <string.h>
#شامل <ctype.h>
#شامل <unistd.h>
#شامل <errno.h>
#شامل <stdlib.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>

#شامل "canonicalize.h"
#شامل "pathnames.h"
#شامل "all-io.h"
#شامل "strutils.h"
#شامل "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
نویسه *ul_canonicalize_dm_name_prefixed(ثابت نویسه *prefix, ثابت نویسه *ptname)
{
	FILE	*f;
	size_t	sz;
	نویسه	path[256], name[اندازه(path) - اندازه(_PATH_DEV_MAPPER)], *res = پوچ;

	اگر (!ptname || !*ptname)
		بازگشت پوچ;

	اگر (!prefix)
		prefix = "";

	snprintf(path, اندازه(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	اگر (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		بازگشت پوچ;

	/* read "<name>\n" from sysfs */
	اگر (fgets(name, اندازه(name), f) && (sz = strlen(name)) > 1) {
		اگر (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, اندازه(path), _PATH_DEV_MAPPER "/%s", name);

		اگر ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	بازگشت res;
}

نویسه *ul_canonicalize_dm_name(ثابت نویسه *ptname)
{
	بازگشت ul_canonicalize_dm_name_prefixed(پوچ, ptname);
}

ایستا صحیح is_dm_devname(نویسه *canonical, نویسه **name)
{
	ساختار stat sb;
	نویسه *p = strrchr(canonical, '/');

	*name = پوچ;

	اگر (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		بازگشت 0;

	*name = p + 1;
	بازگشت 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
نویسه *ul_absolute_path(ثابت نویسه *path)
{
	نویسه cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	اگر (!ul_is_relative_path(path)) {
		errno = EINVAL;
		بازگشت پوچ;
	}
	اگر (!getcwd(cwd, اندازه(cwd)))
		بازگشت پوچ;

	/* simple clean up */
	اگر (ul_startswith(path, "./"))
		path += 2;
	وگرنه اگر (strcmp(path, ".") == 0)
		path = پوچ;

	اگر (!path || !*path)
		بازگشت strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	اگر (!res)
		بازگشت پوچ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	بازگشت res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ایستا صحیح __attribute__((nonnull(2)))
do_canonicalize(ثابت نویسه *path, نویسه **result,
		تهی *data __attribute__((__unused__)))
{
	نویسه *canonical, *dmname;

	*result = پوچ;

	اگر (!path || !*path) {
		errno = EINVAL;
		بازگشت -errno;
	}

	errno = 0;
	canonical = realpath(path, پوچ);
	اگر (!canonical)
		بازگشت 1;

	اگر (is_dm_devname(canonical, &dmname)) {
		نویسه *dm = ul_canonicalize_dm_name(dmname);
		اگر (dm) {
			آزاد(canonical);
			canonical = dm;
		}
	}

	اگر (canonical)
		*result = canonical;
	بازگشت 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
نویسه *ul_canonicalize_path(ثابت نویسه *path)
{
	نویسه *canonical = پوچ;

	اگر (do_canonicalize(path, &canonical, پوچ) == 1)
		بازگشت strdup(path);

	بازگشت canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
نویسه *ul_canonicalize_path_restricted(ثابت نویسه *path)
{
	بازگشت ul_restricted_path_oper(path, do_canonicalize, پوچ);
}

#ifdef TEST_PROGRAM_CANONICALIZE
صحیح اصلی(صحیح argc, نویسه **argv)
{
	نویسه *p;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	آزاد(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	آزاد(p);

	exit(EXIT_SUCCESS);
}
#endif
