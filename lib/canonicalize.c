/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>

#অন্তর্ভাব "canonicalize.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
বর্ণ *ul_canonicalize_dm_name_prefixed(ধ্রুব বর্ণ *prefix, ধ্রুব বর্ণ *ptname)
{
	FILE	*f;
	size_t	sz;
	বর্ণ	path[256], name[পরিমাণ(path) - পরিমাণ(_PATH_DEV_MAPPER)], *res = রিক্ত;

	যদি (!ptname || !*ptname)
		প্রত্যাগম রিক্ত;

	যদি (!prefix)
		prefix = "";

	snprintf(path, পরিমাণ(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	যদি (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		প্রত্যাগম রিক্ত;

	/* read "<name>\n" from sysfs */
	যদি (fgets(name, পরিমাণ(name), f) && (sz = strlen(name)) > 1) {
		যদি (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, পরিমাণ(path), _PATH_DEV_MAPPER "/%s", name);

		যদি ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	প্রত্যাগম res;
}

বর্ণ *ul_canonicalize_dm_name(ধ্রুব বর্ণ *ptname)
{
	প্রত্যাগম ul_canonicalize_dm_name_prefixed(রিক্ত, ptname);
}

স্থির পূর্ণাঙ্ক is_dm_devname(বর্ণ *canonical, বর্ণ **name)
{
	সংরচনা stat sb;
	বর্ণ *p = strrchr(canonical, '/');

	*name = রিক্ত;

	যদি (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		প্রত্যাগম 0;

	*name = p + 1;
	প্রত্যাগম 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
বর্ণ *ul_absolute_path(ধ্রুব বর্ণ *path)
{
	বর্ণ cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	যদি (!ul_is_relative_path(path)) {
		errno = EINVAL;
		প্রত্যাগম রিক্ত;
	}
	যদি (!getcwd(cwd, পরিমাণ(cwd)))
		প্রত্যাগম রিক্ত;

	/* simple clean up */
	যদি (ul_startswith(path, "./"))
		path += 2;
	অন্যথা যদি (strcmp(path, ".") == 0)
		path = রিক্ত;

	যদি (!path || !*path)
		প্রত্যাগম strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	যদি (!res)
		প্রত্যাগম রিক্ত;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	প্রত্যাগম res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
স্থির পূর্ণাঙ্ক __attribute__((nonnull(2)))
do_canonicalize(ধ্রুব বর্ণ *path, বর্ণ **result,
		শূন্য *data __attribute__((__unused__)))
{
	বর্ণ *canonical, *dmname;

	*result = রিক্ত;

	যদি (!path || !*path) {
		errno = EINVAL;
		প্রত্যাগম -errno;
	}

	errno = 0;
	canonical = realpath(path, রিক্ত);
	যদি (!canonical)
		প্রত্যাগম 1;

	যদি (is_dm_devname(canonical, &dmname)) {
		বর্ণ *dm = ul_canonicalize_dm_name(dmname);
		যদি (dm) {
			মুক্ত(canonical);
			canonical = dm;
		}
	}

	যদি (canonical)
		*result = canonical;
	প্রত্যাগম 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
বর্ণ *ul_canonicalize_path(ধ্রুব বর্ণ *path)
{
	বর্ণ *canonical = রিক্ত;

	যদি (do_canonicalize(path, &canonical, রিক্ত) == 1)
		প্রত্যাগম strdup(path);

	প্রত্যাগম canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
বর্ণ *ul_canonicalize_path_restricted(ধ্রুব বর্ণ *path)
{
	প্রত্যাগম ul_restricted_path_oper(path, do_canonicalize, রিক্ত);
}

#ifdef TEST_PROGRAM_CANONICALIZE
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	বর্ণ *p;

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
