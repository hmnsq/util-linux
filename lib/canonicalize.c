/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>

#అన్తర్భావ "canonicalize.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
అక్షరం *ul_canonicalize_dm_name_prefixed(స్థిరరాశి అక్షరం *ఉపసర్గ, స్థిరరాశి అక్షరం *ptname)
{
	FILE	*f;
	size_t	sz;
	అక్షరం	మార్గం[256], name[పరిమాణం(మార్గం) - పరిమాణం(మార్గం_DEV_MAPPER)], *res = ఖాళీ;

	ఒకవేళ (!ptname || !*ptname)
		తిరిగి ఖాళీ;

	ఒకవేళ (!ఉపసర్గ)
		ఉపసర్గ = "";

	snprintf(మార్గం, పరిమాణం(మార్గం), "%s/sys/block/%s/dm/name", ఉపసర్గ, ptname);
	ఒకవేళ (!(f = fopen(మార్గం, "r" UL_CLOEXECSTR)))
		తిరిగి ఖాళీ;

	/* read "<name>\n" from sysfs */
	ఒకవేళ (fgets(name, పరిమాణం(name), f) && (sz = అక్షరశ్రేణి_పొడవు(name)) > 1) {
		ఒకవేళ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(మార్గం, పరిమాణం(మార్గం), మార్గం_DEV_MAPPER "/%s", name);

		ఒకవేళ ((ఉపసర్గ && *ఉపసర్గ) || access(మార్గం, F_OK) == 0)
			res = strdup(మార్గం);
	}
	fclose(f);
	తిరిగి res;
}

అక్షరం *ul_canonicalize_dm_name(స్థిరరాశి అక్షరం *ptname)
{
	తిరిగి ul_canonicalize_dm_name_prefixed(ఖాళీ, ptname);
}

స్థిర పూర్ణాంకం is_dm_devname(అక్షరం *canonical, అక్షరం **name)
{
	నిర్మాణం stat sb;
	అక్షరం *p = strrchr(canonical, '/');

	*name = ఖాళీ;

	ఒకవేళ (!p
	    || అక్షరశ్రేణి_పోలిక(p, "/dm-", 4) != 0
	    || !అంకెనా(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		తిరిగి 0;

	*name = p + 1;
	తిరిగి 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
అక్షరం *ul_absolute_మార్గం(స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	ఒకవేళ (!ul_is_relative_మార్గం(మార్గం)) {
		errno = EINVAL;
		తిరిగి ఖాళీ;
	}
	ఒకవేళ (!getcwd(cwd, పరిమాణం(cwd)))
		తిరిగి ఖాళీ;

	/* simple clean up */
	ఒకవేళ (ul_startswith(మార్గం, "./"))
		మార్గం += 2;
	లేకపోతే ఒకవేళ (strcmp(మార్గం, ".") == 0)
		మార్గం = ఖాళీ;

	ఒకవేళ (!మార్గం || !*మార్గం)
		తిరిగి strdup(cwd);

	csz = అక్షరశ్రేణి_పొడవు(cwd);
	psz = అక్షరశ్రేణి_పొడవు(మార్గం);

	p = res = జ్ఞాపకం_ఇవ్వు(csz + 1 + psz + 1);
	ఒకవేళ (!res)
		తిరిగి ఖాళీ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	జ్ఞాపకం_నకలు(p, మార్గం, psz + 1);

	తిరిగి res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
స్థిర పూర్ణాంకం __attribute__((nonnull(2)))
do_canonicalize(స్థిరరాశి అక్షరం *మార్గం, అక్షరం **ఫలితం,
		శూన్య *దత్తాంశం __attribute__((__unused__)))
{
	అక్షరం *canonical, *dmname;

	*ఫలితం = ఖాళీ;

	ఒకవేళ (!మార్గం || !*మార్గం) {
		errno = EINVAL;
		తిరిగి -errno;
	}

	errno = 0;
	canonical = realpath(మార్గం, ఖాళీ);
	ఒకవేళ (!canonical)
		తిరిగి 1;

	ఒకవేళ (is_dm_devname(canonical, &dmname)) {
		అక్షరం *dm = ul_canonicalize_dm_name(dmname);
		ఒకవేళ (dm) {
			విడుదల(canonical);
			canonical = dm;
		}
	}

	ఒకవేళ (canonical)
		*ఫలితం = canonical;
	తిరిగి 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
అక్షరం *ul_canonicalize_మార్గం(స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం *canonical = ఖాళీ;

	ఒకవేళ (do_canonicalize(మార్గం, &canonical, ఖాళీ) == 1)
		తిరిగి strdup(మార్గం);

	తిరిగి canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
అక్షరం *ul_canonicalize_మార్గం_restricted(స్థిరరాశి అక్షరం *మార్గం)
{
	తిరిగి ul_restricted_మార్గం_oper(మార్గం, do_canonicalize, ఖాళీ);
}

#నిర్వచితమైతే TEST_కార్యక్రమం_CANONICALIZE
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	అక్షరం *p;

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		నిష్క్రమించు(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_మార్గం(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	విడుదల(p);

	p = ul_canonicalize_మార్గం_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	విడుదల(p);

	నిష్క్రమించు(EXIT_SUCCESS);
}
#అయితే_ముగింపు
