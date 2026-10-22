/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>

#অন্তর্ভাৱ "canonicalize.h"
#অন্তর্ভাৱ "pathnames.h"
#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
ৱর্ণ *ul_canonicalize_dm_name_prefixed(ধ্রুৱ ৱর্ণ *prefix, ধ্রুৱ ৱর্ণ *ptname)
{
	FILE	*f;
	size_t	sz;
	ৱর্ণ	path[256], name[পরিমাণ(path) - পরিমাণ(_PATH_DEV_MAPPER)], *res = রিক্ত;

	যদি (!ptname || !*ptname)
		ঘূৰাই রিক্ত;

	যদি (!prefix)
		prefix = "";

	snprintf(path, পরিমাণ(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	যদি (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ঘূৰাই রিক্ত;

	/* read "<name>\n" from sysfs */
	যদি (fgets(name, পরিমাণ(name), f) && (sz = strlen(name)) > 1) {
		যদি (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, পরিমাণ(path), _PATH_DEV_MAPPER "/%s", name);

		যদি ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ঘূৰাই res;
}

ৱর্ণ *ul_canonicalize_dm_name(ধ্রুৱ ৱর্ণ *ptname)
{
	ঘূৰাই ul_canonicalize_dm_name_prefixed(রিক্ত, ptname);
}

স্থির পূর্ণাঙ্ক is_dm_devname(ৱর্ণ *canonical, ৱর্ণ **name)
{
	সংরচনা stat sb;
	ৱর্ণ *p = strrchr(canonical, '/');

	*name = রিক্ত;

	যদি (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ঘূৰাই 0;

	*name = p + 1;
	ঘূৰাই 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
ৱর্ণ *ul_absolute_path(ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	যদি (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ঘূৰাই রিক্ত;
	}
	যদি (!getcwd(cwd, পরিমাণ(cwd)))
		ঘূৰাই রিক্ত;

	/* simple clean up */
	যদি (ul_startswith(path, "./"))
		path += 2;
	অন্যথা যদি (strcmp(path, ".") == 0)
		path = রিক্ত;

	যদি (!path || !*path)
		ঘূৰাই strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	যদি (!res)
		ঘূৰাই রিক্ত;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ঘূৰাই res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
স্থির পূর্ণাঙ্ক __attribute__((nonnull(2)))
do_canonicalize(ধ্রুৱ ৱর্ণ *path, ৱর্ণ **result,
		শূন্য *data __attribute__((__unused__)))
{
	ৱর্ণ *canonical, *dmname;

	*result = রিক্ত;

	যদি (!path || !*path) {
		errno = EINVAL;
		ঘূৰাই -errno;
	}

	errno = 0;
	canonical = realpath(path, রিক্ত);
	যদি (!canonical)
		ঘূৰাই 1;

	যদি (is_dm_devname(canonical, &dmname)) {
		ৱর্ণ *dm = ul_canonicalize_dm_name(dmname);
		যদি (dm) {
			মুক্ত(canonical);
			canonical = dm;
		}
	}

	যদি (canonical)
		*result = canonical;
	ঘূৰাই 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
ৱর্ণ *ul_canonicalize_path(ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ *canonical = রিক্ত;

	যদি (do_canonicalize(path, &canonical, রিক্ত) == 1)
		ঘূৰাই strdup(path);

	ঘূৰাই canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
ৱর্ণ *ul_canonicalize_path_restricted(ধ্রুৱ ৱর্ণ *path)
{
	ঘূৰাই ul_restricted_path_oper(path, do_canonicalize, রিক্ত);
}

#ifdef TEST_PROGRAM_CANONICALIZE
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	ৱর্ণ *p;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	মুক্ত(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	মুক্ত(p);

	exit(EXIT_SUCCESS);
}
#endif
