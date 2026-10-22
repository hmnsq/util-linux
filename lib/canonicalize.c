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
वर्ण *ul_canonicalize_dm_name_prefixed(ध्रुव वर्ण *उपसर्गः, ध्रुव वर्ण *ptname)
{
	FILE	*f;
	size_t	sz;
	वर्ण	मार्गः[256], name[परिमाण(मार्गः) - परिमाण(मार्गः_DEV_MAPPER)], *res = रिक्त;

	यदि (!ptname || !*ptname)
		प्रत्यागम रिक्त;

	यदि (!उपसर्गः)
		उपसर्गः = "";

	snprintf(मार्गः, परिमाण(मार्गः), "%s/sys/block/%s/dm/name", उपसर्गः, ptname);
	यदि (!(f = fopen(मार्गः, "r" UL_CLOEXECSTR)))
		प्रत्यागम रिक्त;

	/* read "<name>\n" from sysfs */
	यदि (fgets(name, परिमाण(name), f) && (sz = वाक्यदैर्घ्यम्(name)) > 1) {
		यदि (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(मार्गः, परिमाण(मार्गः), मार्गः_DEV_MAPPER "/%s", name);

		यदि ((उपसर्गः && *उपसर्गः) || access(मार्गः, F_OK) == 0)
			res = strdup(मार्गः);
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

	यदि (!p
	    || वाक्ये_तुलय(p, "/dm-", 4) != 0
	    || !अङ्कः_वा(*(p + 4))
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
वर्ण *ul_absolute_मार्गः(ध्रुव वर्ण *मार्गः)
{
	वर्ण cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	यदि (!ul_is_relative_मार्गः(मार्गः)) {
		errno = EINVAL;
		प्रत्यागम रिक्त;
	}
	यदि (!getcwd(cwd, परिमाण(cwd)))
		प्रत्यागम रिक्त;

	/* simple clean up */
	यदि (ul_startswith(मार्गः, "./"))
		मार्गः += 2;
	अन्यथा यदि (वाक्यं_तुलय(मार्गः, ".") == 0)
		मार्गः = रिक्त;

	यदि (!मार्गः || !*मार्गः)
		प्रत्यागम strdup(cwd);

	csz = वाक्यदैर्घ्यम्(cwd);
	psz = वाक्यदैर्घ्यम्(मार्गः);

	p = res = स्मृतिं_देहि(csz + 1 + psz + 1);
	यदि (!res)
		प्रत्यागम रिक्त;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	स्मृतिं_प्रतिलिख(p, मार्गः, psz + 1);

	प्रत्यागम res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
स्थिर पूर्णाङ्क __attribute__((nonnull(2)))
do_canonicalize(ध्रुव वर्ण *मार्गः, वर्ण **परिणाम,
		शून्य *दत्त __attribute__((__unused__)))
{
	वर्ण *canonical, *dmname;

	*परिणाम = रिक्त;

	यदि (!मार्गः || !*मार्गः) {
		errno = EINVAL;
		प्रत्यागम -errno;
	}

	errno = 0;
	canonical = realpath(मार्गः, रिक्त);
	यदि (!canonical)
		प्रत्यागम 1;

	यदि (is_dm_devname(canonical, &dmname)) {
		वर्ण *dm = ul_canonicalize_dm_name(dmname);
		यदि (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	यदि (canonical)
		*परिणाम = canonical;
	प्रत्यागम 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
वर्ण *ul_canonicalize_मार्गः(ध्रुव वर्ण *मार्गः)
{
	वर्ण *canonical = रिक्त;

	यदि (do_canonicalize(मार्गः, &canonical, रिक्त) == 1)
		प्रत्यागम strdup(मार्गः);

	प्रत्यागम canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
वर्ण *ul_canonicalize_मार्गः_restricted(ध्रुव वर्ण *मार्गः)
{
	प्रत्यागम ul_restricted_मार्गः_oper(मार्गः, do_canonicalize, रिक्त);
}

#यदि_परिभाषितम् TEST_क्रमादेशः_CANONICALIZE
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *p;

	यदि (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		निर्गच्छ(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_मार्गः(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	मुक्त(p);

	p = ul_canonicalize_मार्गः_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	मुक्त(p);

	निर्गच्छ(EXIT_SUCCESS);
}
#यदि_अन्तः
