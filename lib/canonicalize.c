/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ضمن <unistd.h>
#ضمن <errno.h>
#ضمن <stdlib.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>

#ضمن "canonicalize.h"
#ضمن "pathnames.h"
#ضمن "all-io.h"
#ضمن "strutils.h"
#ضمن "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
حرف *ul_canonicalize_dm_name_prefixed(ثابت حرف *prefix, ثابت حرف *ptname)
{
	FILE	*f;
	size_t	sz;
	حرف	path[256], name[حجم(path) - حجم(_PATH_DEV_MAPPER)], *res = عدم;

	إذا (!ptname || !*ptname)
		أرجع عدم;

	إذا (!prefix)
		prefix = "";

	snprintf(path, حجم(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	إذا (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		أرجع عدم;

	/* read "<name>\n" from sysfs */
	إذا (fgets(name, حجم(name), f) && (sz = strlen(name)) > 1) {
		إذا (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, حجم(path), _PATH_DEV_MAPPER "/%s", name);

		إذا ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	أرجع res;
}

حرف *ul_canonicalize_dm_name(ثابت حرف *ptname)
{
	أرجع ul_canonicalize_dm_name_prefixed(عدم, ptname);
}

ساكن صحيح is_dm_devname(حرف *canonical, حرف **name)
{
	بنية stat sb;
	حرف *p = strrchr(canonical, '/');

	*name = عدم;

	إذا (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		أرجع 0;

	*name = p + 1;
	أرجع 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
حرف *ul_absolute_path(ثابت حرف *path)
{
	حرف cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	إذا (!ul_is_relative_path(path)) {
		errno = EINVAL;
		أرجع عدم;
	}
	إذا (!getcwd(cwd, حجم(cwd)))
		أرجع عدم;

	/* simple clean up */
	إذا (ul_startswith(path, "./"))
		path += 2;
	وإلا إذا (strcmp(path, ".") == 0)
		path = عدم;

	إذا (!path || !*path)
		أرجع strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	إذا (!res)
		أرجع عدم;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	أرجع res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ساكن صحيح __attribute__((nonnull(2)))
do_canonicalize(ثابت حرف *path, حرف **result,
		فراغ *data __attribute__((__unused__)))
{
	حرف *canonical, *dmname;

	*result = عدم;

	إذا (!path || !*path) {
		errno = EINVAL;
		أرجع -errno;
	}

	errno = 0;
	canonical = realpath(path, عدم);
	إذا (!canonical)
		أرجع 1;

	إذا (is_dm_devname(canonical, &dmname)) {
		حرف *dm = ul_canonicalize_dm_name(dmname);
		إذا (dm) {
			حرر(canonical);
			canonical = dm;
		}
	}

	إذا (canonical)
		*result = canonical;
	أرجع 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
حرف *ul_canonicalize_path(ثابت حرف *path)
{
	حرف *canonical = عدم;

	إذا (do_canonicalize(path, &canonical, عدم) == 1)
		أرجع strdup(path);

	أرجع canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
حرف *ul_canonicalize_path_restricted(ثابت حرف *path)
{
	أرجع ul_restricted_path_oper(path, do_canonicalize, عدم);
}

#ifdef TEST_PROGRAM_CANONICALIZE
صحيح main(صحيح argc, حرف **argv)
{
	حرف *p;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	حرر(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	حرر(p);

	exit(EXIT_SUCCESS);
}
#endif
