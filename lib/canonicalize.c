/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

#ଅନ୍ତର୍ଭାଵ "canonicalize.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
ଅକ୍ଷର *ul_canonicalize_dm_name_prefixed(ଅଚଳ ଅକ୍ଷର *prefix, ଅଚଳ ଅକ୍ଷର *ptname)
{
	FILE	*f;
	size_t	sz;
	ଅକ୍ଷର	path[256], name[ଆକାର(path) - ଆକାର(_PATH_DEV_MAPPER)], *res = ଖାଲି;

	ଯଦି (!ptname || !*ptname)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!prefix)
		prefix = "";

	snprintf(path, ଆକାର(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	ଯଦି (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		ଫେରନ୍ତୁ ଖାଲି;

	/* read "<name>\n" from sysfs */
	ଯଦି (fgets(name, ଆକାର(name), f) && (sz = strlen(name)) > 1) {
		ଯଦି (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, ଆକାର(path), _PATH_DEV_MAPPER "/%s", name);

		ଯଦି ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	ଫେରନ୍ତୁ res;
}

ଅକ୍ଷର *ul_canonicalize_dm_name(ଅଚଳ ଅକ୍ଷର *ptname)
{
	ଫେରନ୍ତୁ ul_canonicalize_dm_name_prefixed(ଖାଲି, ptname);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_dm_devname(ଅକ୍ଷର *canonical, ଅକ୍ଷର **name)
{
	ସଂରଚନା stat sb;
	ଅକ୍ଷର *p = strrchr(canonical, '/');

	*name = ଖାଲି;

	ଯଦି (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ଫେରନ୍ତୁ 0;

	*name = p + 1;
	ଫେରନ୍ତୁ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
ଅକ୍ଷର *ul_absolute_path(ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	ଯଦି (!ul_is_relative_path(path)) {
		errno = EINVAL;
		ଫେରନ୍ତୁ ଖାଲି;
	}
	ଯଦି (!getcwd(cwd, ଆକାର(cwd)))
		ଫେରନ୍ତୁ ଖାଲି;

	/* simple clean up */
	ଯଦି (ul_startswith(path, "./"))
		path += 2;
	ଅନ୍ୟଥା ଯଦି (strcmp(path, ".") == 0)
		path = ଖାଲି;

	ଯଦି (!path || !*path)
		ଫେରନ୍ତୁ strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	ଯଦି (!res)
		ଫେରନ୍ତୁ ଖାଲି;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	ଫେରନ୍ତୁ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __attribute__((nonnull(2)))
do_canonicalize(ଅଚଳ ଅକ୍ଷର *path, ଅକ୍ଷର **result,
		ଶୂନ୍ୟ *data __attribute__((__unused__)))
{
	ଅକ୍ଷର *canonical, *dmname;

	*result = ଖାଲି;

	ଯଦି (!path || !*path) {
		errno = EINVAL;
		ଫେରନ୍ତୁ -errno;
	}

	errno = 0;
	canonical = realpath(path, ଖାଲି);
	ଯଦି (!canonical)
		ଫେରନ୍ତୁ 1;

	ଯଦି (is_dm_devname(canonical, &dmname)) {
		ଅକ୍ଷର *dm = ul_canonicalize_dm_name(dmname);
		ଯଦି (dm) {
			ମୁକ୍ତ(canonical);
			canonical = dm;
		}
	}

	ଯଦି (canonical)
		*result = canonical;
	ଫେରନ୍ତୁ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
ଅକ୍ଷର *ul_canonicalize_path(ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *canonical = ଖାଲି;

	ଯଦି (do_canonicalize(path, &canonical, ଖାଲି) == 1)
		ଫେରନ୍ତୁ strdup(path);

	ଫେରନ୍ତୁ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
ଅକ୍ଷର *ul_canonicalize_path_restricted(ଅଚଳ ଅକ୍ଷର *path)
{
	ଫେରନ୍ତୁ ul_restricted_path_oper(path, do_canonicalize, ଖାଲି);
}

#ifdef TEST_PROGRAM_CANONICALIZE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *p;

	ଯଦି (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	ମୁକ୍ତ(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	ମୁକ୍ତ(p);

	exit(EXIT_SUCCESS);
}
#endif
