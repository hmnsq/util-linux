/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>

#अन्तर्भाव "canonicalize.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
वर्ण *ul_canonicalize_dm_name_prefixed(ध्रुव वर्ण *prefix, ध्रुव वर्ण *ptname)
{
	FILE	*f;
	size_t	sz;
	वर्ण	path[256], name[परिमाण(path) - परिमाण(_PATH_DEV_MAPPER)], *res = रिक्त;

	जइ (!ptname || !*ptname)
		प्रत्यागम रिक्त;

	जइ (!prefix)
		prefix = "";

	snprintf(path, परिमाण(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	जइ (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		प्रत्यागम रिक्त;

	/* read "<name>\n" from sysfs */
	जइ (fgets(name, परिमाण(name), f) && (sz = strlen(name)) > 1) {
		जइ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, परिमाण(path), _PATH_DEV_MAPPER "/%s", name);

		जइ ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	प्रत्यागम res;
}

वर्ण *ul_canonicalize_dm_name(ध्रुव वर्ण *ptname)
{
	प्रत्यागम ul_canonicalize_dm_name_prefixed(रिक्त, ptname);
}

स्थिर पूर्णाङ्क is_dm_devname(वर्ण *canonical, वर्ण **name)
{
	संरचना stat sb;
	वर्ण *p = strrchr(canonical, '/');

	*name = रिक्त;

	जइ (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		प्रत्यागम 0;

	*name = p + 1;
	प्रत्यागम 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
वर्ण *ul_absolute_path(ध्रुव वर्ण *path)
{
	वर्ण cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	जइ (!ul_is_relative_path(path)) {
		errno = EINVAL;
		प्रत्यागम रिक्त;
	}
	जइ (!getcwd(cwd, परिमाण(cwd)))
		प्रत्यागम रिक्त;

	/* simple clean up */
	जइ (ul_startswith(path, "./"))
		path += 2;
	अण्णहा जइ (strcmp(path, ".") == 0)
		path = रिक्त;

	जइ (!path || !*path)
		प्रत्यागम strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	जइ (!res)
		प्रत्यागम रिक्त;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	प्रत्यागम res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
स्थिर पूर्णाङ्क __attribute__((nonnull(2)))
do_canonicalize(ध्रुव वर्ण *path, वर्ण **result,
		शून्य *data __attribute__((__unused__)))
{
	वर्ण *canonical, *dmname;

	*result = रिक्त;

	जइ (!path || !*path) {
		errno = EINVAL;
		प्रत्यागम -errno;
	}

	errno = 0;
	canonical = realpath(path, रिक्त);
	जइ (!canonical)
		प्रत्यागम 1;

	जइ (is_dm_devname(canonical, &dmname)) {
		वर्ण *dm = ul_canonicalize_dm_name(dmname);
		जइ (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	जइ (canonical)
		*result = canonical;
	प्रत्यागम 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
वर्ण *ul_canonicalize_path(ध्रुव वर्ण *path)
{
	वर्ण *canonical = रिक्त;

	जइ (do_canonicalize(path, &canonical, रिक्त) == 1)
		प्रत्यागम strdup(path);

	प्रत्यागम canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
वर्ण *ul_canonicalize_path_restricted(ध्रुव वर्ण *path)
{
	प्रत्यागम ul_restricted_path_oper(path, do_canonicalize, रिक्त);
}

#ifdef TEST_PROGRAM_CANONICALIZE
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *p;

	जइ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	मुक्त(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	मुक्त(p);

	exit(EXIT_SUCCESS);
}
#endif
