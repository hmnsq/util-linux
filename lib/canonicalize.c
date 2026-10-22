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
অক্ষর *ul_canonicalize_dm_name_prefixed(ধ্রুবক অক্ষর *prefix, ধ্রুবক অক্ষর *ptname)
{
	FILE	*f;
	size_t	sz;
	অক্ষর	path[256], name[আকার(path) - আকার(_PATH_DEV_MAPPER)], *res = ফাঁকা;

	যদি (!ptname || !*ptname)
		ফেরত ফাঁকা;

	যদি (!prefix)
		prefix = "";

	snprintf(path, আকার(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	যদি (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ফেরত ফাঁকা;

	/* read "<name>\n" from sysfs */
	যদি (fgets(name, আকার(name), f) && (sz = strlen(name)) > 1) {
		যদি (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, আকার(path), _PATH_DEV_MAPPER "/%s", name);

		যদি ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ফেরত res;
}

অক্ষর *ul_canonicalize_dm_name(ধ্রুবক অক্ষর *ptname)
{
	ফেরত ul_canonicalize_dm_name_prefixed(ফাঁকা, ptname);
}

স্থির পূর্ণসংখ্যা is_dm_devname(অক্ষর *canonical, অক্ষর **name)
{
	কাঠামো stat sb;
	অক্ষর *p = strrchr(canonical, '/');

	*name = ফাঁকা;

	যদি (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ফেরত 0;

	*name = p + 1;
	ফেরত 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
অক্ষর *ul_absolute_path(ধ্রুবক অক্ষর *path)
{
	অক্ষর cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	যদি (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ফেরত ফাঁকা;
	}
	যদি (!getcwd(cwd, আকার(cwd)))
		ফেরত ফাঁকা;

	/* simple clean up */
	যদি (ul_startswith(path, "./"))
		path += 2;
	অন্যথা যদি (strcmp(path, ".") == 0)
		path = ফাঁকা;

	যদি (!path || !*path)
		ফেরত strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	যদি (!res)
		ফেরত ফাঁকা;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ফেরত res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
স্থির পূর্ণসংখ্যা __attribute__((nonnull(2)))
do_canonicalize(ধ্রুবক অক্ষর *path, অক্ষর **ফলাফল,
		শূন্য *তথ্য __attribute__((__unused__)))
{
	অক্ষর *canonical, *dmname;

	*ফলাফল = ফাঁকা;

	যদি (!path || !*path) {
		errno = EINVAL;
		ফেরত -errno;
	}

	errno = 0;
	canonical = realpath(path, ফাঁকা);
	যদি (!canonical)
		ফেরত 1;

	যদি (is_dm_devname(canonical, &dmname)) {
		অক্ষর *dm = ul_canonicalize_dm_name(dmname);
		যদি (dm) {
			মুক্ত(canonical);
			canonical = dm;
		}
	}

	যদি (canonical)
		*ফলাফল = canonical;
	ফেরত 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
অক্ষর *ul_canonicalize_path(ধ্রুবক অক্ষর *path)
{
	অক্ষর *canonical = ফাঁকা;

	যদি (do_canonicalize(path, &canonical, ফাঁকা) == 1)
		ফেরত strdup(path);

	ফেরত canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
অক্ষর *ul_canonicalize_path_restricted(ধ্রুবক অক্ষর *path)
{
	ফেরত ul_restricted_path_oper(path, do_canonicalize, ফাঁকা);
}

#ifdef TEST_PROGRAM_CANONICALIZE
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	অক্ষর *p;

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
