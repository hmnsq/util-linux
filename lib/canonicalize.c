/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>

#அந்தர்பாவ "canonicalize.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
எழுத்து *ul_canonicalize_dm_name_prefixed(மாறிலி எழுத்து *prefix, மாறிலி எழுத்து *ptname)
{
	FILE	*f;
	size_t	sz;
	எழுத்து	path[256], name[அளவு(path) - அளவு(_PATH_DEV_MAPPER)], *res = வெற்று;

	எனில் (!ptname || !*ptname)
		திரும்பு வெற்று;

	எனில் (!prefix)
		prefix = "";

	snprintf(path, அளவு(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	எனில் (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		திரும்பு வெற்று;

	/* read "<name>\n" from sysfs */
	எனில் (fgets(name, அளவு(name), f) && (sz = strlen(name)) > 1) {
		எனில் (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, அளவு(path), _PATH_DEV_MAPPER "/%s", name);

		எனில் ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	திரும்பு res;
}

எழுத்து *ul_canonicalize_dm_name(மாறிலி எழுத்து *ptname)
{
	திரும்பு ul_canonicalize_dm_name_prefixed(வெற்று, ptname);
}

நிலையான முழு is_dm_devname(எழுத்து *canonical, எழுத்து **name)
{
	கட்டமைப்பு stat sb;
	எழுத்து *p = strrchr(canonical, '/');

	*name = வெற்று;

	எனில் (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		திரும்பு 0;

	*name = p + 1;
	திரும்பு 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
எழுத்து *ul_absolute_path(மாறிலி எழுத்து *path)
{
	எழுத்து cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	எனில் (!ul_is_relative_path(path)) {
		errno = EINVAL;
		திரும்பு வெற்று;
	}
	எனில் (!getcwd(cwd, அளவு(cwd)))
		திரும்பு வெற்று;

	/* simple clean up */
	எனில் (ul_startswith(path, "./"))
		path += 2;
	இல்லையெனில் எனில் (strcmp(path, ".") == 0)
		path = வெற்று;

	எனில் (!path || !*path)
		திரும்பு strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	எனில் (!res)
		திரும்பு வெற்று;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	திரும்பு res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
நிலையான முழு __attribute__((nonnull(2)))
do_canonicalize(மாறிலி எழுத்து *path, எழுத்து **result,
		வெறுமை *data __attribute__((__unused__)))
{
	எழுத்து *canonical, *dmname;

	*result = வெற்று;

	எனில் (!path || !*path) {
		errno = EINVAL;
		திரும்பு -errno;
	}

	errno = 0;
	canonical = realpath(path, வெற்று);
	எனில் (!canonical)
		திரும்பு 1;

	எனில் (is_dm_devname(canonical, &dmname)) {
		எழுத்து *dm = ul_canonicalize_dm_name(dmname);
		எனில் (dm) {
			விடுவி(canonical);
			canonical = dm;
		}
	}

	எனில் (canonical)
		*result = canonical;
	திரும்பு 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
எழுத்து *ul_canonicalize_path(மாறிலி எழுத்து *path)
{
	எழுத்து *canonical = வெற்று;

	எனில் (do_canonicalize(path, &canonical, வெற்று) == 1)
		திரும்பு strdup(path);

	திரும்பு canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
எழுத்து *ul_canonicalize_path_restricted(மாறிலி எழுத்து *path)
{
	திரும்பு ul_restricted_path_oper(path, do_canonicalize, வெற்று);
}

#ifdef TEST_PROGRAM_CANONICALIZE
முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	எழுத்து *p;

	எனில் (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	விடுவி(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	விடுவி(p);

	exit(EXIT_SUCCESS);
}
#endif
