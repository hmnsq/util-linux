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

	सचे (!ptname || !*ptname)
		पटिनिवत्त रिक्त;

	सचे (!prefix)
		prefix = "";

	snprintf(path, परिमाण(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	सचे (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		पटिनिवत्त रिक्त;

	/* read "<name>\n" from sysfs */
	सचे (fgets(name, परिमाण(name), f) && (sz = strlen(name)) > 1) {
		सचे (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, परिमाण(path), _PATH_DEV_MAPPER "/%s", name);

		सचे ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	पटिनिवत्त res;
}

वर्ण *ul_canonicalize_dm_name(ध्रुव वर्ण *ptname)
{
	पटिनिवत्त ul_canonicalize_dm_name_prefixed(रिक्त, ptname);
}

स्थिर पूर्णाङ्क is_dm_devname(वर्ण *canonical, वर्ण **name)
{
	संरचना stat sb;
	वर्ण *p = strrchr(canonical, '/');

	*name = रिक्त;

	सचे (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		पटिनिवत्त 0;

	*name = p + 1;
	पटिनिवत्त 1;
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

	सचे (!ul_is_relative_path(path)) {
		errno = EINVAL;
		पटिनिवत्त रिक्त;
	}
	सचे (!getcwd(cwd, परिमाण(cwd)))
		पटिनिवत्त रिक्त;

	/* simple clean up */
	सचे (ul_startswith(path, "./"))
		path += 2;
	अञ्ञथा सचे (strcmp(path, ".") == 0)
		path = रिक्त;

	सचे (!path || !*path)
		पटिनिवत्त strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	सचे (!res)
		पटिनिवत्त रिक्त;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	पटिनिवत्त res;
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

	सचे (!path || !*path) {
		errno = EINVAL;
		पटिनिवत्त -errno;
	}

	errno = 0;
	canonical = realpath(path, रिक्त);
	सचे (!canonical)
		पटिनिवत्त 1;

	सचे (is_dm_devname(canonical, &dmname)) {
		वर्ण *dm = ul_canonicalize_dm_name(dmname);
		सचे (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	सचे (canonical)
		*result = canonical;
	पटिनिवत्त 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
वर्ण *ul_canonicalize_path(ध्रुव वर्ण *path)
{
	वर्ण *canonical = रिक्त;

	सचे (do_canonicalize(path, &canonical, रिक्त) == 1)
		पटिनिवत्त strdup(path);

	पटिनिवत्त canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
वर्ण *ul_canonicalize_path_restricted(ध्रुव वर्ण *path)
{
	पटिनिवत्त ul_restricted_path_oper(path, do_canonicalize, रिक्त);
}

#ifdef TEST_PROGRAM_CANONICALIZE
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *p;

	सचे (argc < 2) {
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
