/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ಅನ್ತರ್ಭಾವ "canonicalize.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
ವರ್ಣ *ul_canonicalize_dm_name_prefixed(ಧ್ರುವ ವರ್ಣ *prefix, ಧ್ರುವ ವರ್ಣ *ptname)
{
	FILE	*f;
	size_t	sz;
	ವರ್ಣ	path[256], name[ಪರಿಮಾಣ(path) - ಪರಿಮಾಣ(_PATH_DEV_MAPPER)], *res = ರಿಕ್ತ;

	ಯದಿ (!ptname || !*ptname)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!prefix)
		prefix = "";

	snprintf(path, ಪರಿಮಾಣ(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	ಯದಿ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* read "<name>\n" from sysfs */
	ಯದಿ (fgets(name, ಪರಿಮಾಣ(name), f) && (sz = strlen(name)) > 1) {
		ಯದಿ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, ಪರಿಮಾಣ(path), _PATH_DEV_MAPPER "/%s", name);

		ಯದಿ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ಪ್ರತ್ಯಾಗಮ res;
}

ವರ್ಣ *ul_canonicalize_dm_name(ಧ್ರುವ ವರ್ಣ *ptname)
{
	ಪ್ರತ್ಯಾಗಮ ul_canonicalize_dm_name_prefixed(ರಿಕ್ತ, ptname);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_dm_devname(ವರ್ಣ *canonical, ವರ್ಣ **name)
{
	ಸಂರಚನಾ stat sb;
	ವರ್ಣ *p = strrchr(canonical, '/');

	*name = ರಿಕ್ತ;

	ಯದಿ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ಪ್ರತ್ಯಾಗಮ 0;

	*name = p + 1;
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
ವರ್ಣ *ul_absolute_path(ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	ಯದಿ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಯದಿ (!getcwd(cwd, ಪರಿಮಾಣ(cwd)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* simple clean up */
	ಯದಿ (ul_startswith(path, "./"))
		path += 2;
	ಅನ್ಯಥಾ ಯದಿ (strcmp(path, ".") == 0)
		path = ರಿಕ್ತ;

	ಯದಿ (!path || !*path)
		ಪ್ರತ್ಯಾಗಮ strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	ಯದಿ (!res)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __attribute__((nonnull(2)))
do_canonicalize(ಧ್ರುವ ವರ್ಣ *path, ವರ್ಣ **result,
		ಶೂನ್ಯ *data __attribute__((__unused__)))
{
	ವರ್ಣ *canonical, *dmname;

	*result = ರಿಕ್ತ;

	ಯದಿ (!path || !*path) {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	errno = 0;
	canonical = realpath(path, ರಿಕ್ತ);
	ಯದಿ (!canonical)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯದಿ (is_dm_devname(canonical, &dmname)) {
		ವರ್ಣ *dm = ul_canonicalize_dm_name(dmname);
		ಯದಿ (dm) {
			ಮುಕ್ತ(canonical);
			canonical = dm;
		}
	}

	ಯದಿ (canonical)
		*result = canonical;
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
ವರ್ಣ *ul_canonicalize_path(ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ *canonical = ರಿಕ್ತ;

	ಯದಿ (do_canonicalize(path, &canonical, ರಿಕ್ತ) == 1)
		ಪ್ರತ್ಯಾಗಮ strdup(path);

	ಪ್ರತ್ಯಾಗಮ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
ವರ್ಣ *ul_canonicalize_path_restricted(ಧ್ರುವ ವರ್ಣ *path)
{
	ಪ್ರತ್ಯಾಗಮ ul_restricted_path_oper(path, do_canonicalize, ರಿಕ್ತ);
}

#ifdef TEST_PROGRAM_CANONICALIZE
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ವರ್ಣ *p;

	ಯದಿ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	ಮುಕ್ತ(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	ಮುಕ್ತ(p);

	exit(EXIT_SUCCESS);
}
#endif
