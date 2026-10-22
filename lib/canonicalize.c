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
ಅಕ್ಷರ *ul_canonicalize_dm_name_prefixed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ptname)
{
	FILE	*f;
	size_t	sz;
	ಅಕ್ಷರ	ಮಾರ್ಗ[256], name[ಗಾತ್ರ(ಮಾರ್ಗ) - ಗಾತ್ರ(ಮಾರ್ಗ_DEV_MAPPER)], *res = ಖಾಲಿ;

	ಆದರೆ (!ptname || !*ptname)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಪೂರ್ವಪ್ರತ್ಯಯ = "";

	snprintf(ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ), "%s/sys/block/%s/dm/name", ಪೂರ್ವಪ್ರತ್ಯಯ, ptname);
	ಆದರೆ (!(f = fopen(ಮಾರ್ಗ, "r" UL_CLOEXECSTR)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* read "<name>\n" from sysfs */
	ಆದರೆ (fgets(name, ಗಾತ್ರ(name), f) && (sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name)) > 1) {
		ಆದರೆ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ), ಮಾರ್ಗ_DEV_MAPPER "/%s", name);

		ಆದರೆ ((ಪೂರ್ವಪ್ರತ್ಯಯ && *ಪೂರ್ವಪ್ರತ್ಯಯ) || access(ಮಾರ್ಗ, F_OK) == 0)
			res = strdup(ಮಾರ್ಗ);
	}
	fclose(f);
	ಹಿಂತಿರುಗಿ res;
}

ಅಕ್ಷರ *ul_canonicalize_dm_name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ptname)
{
	ಹಿಂತಿರುಗಿ ul_canonicalize_dm_name_prefixed(ಖಾಲಿ, ptname);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_dm_devname(ಅಕ್ಷರ *canonical, ಅಕ್ಷರ **name)
{
	ರಚನೆ stat sb;
	ಅಕ್ಷರ *p = strrchr(canonical, '/');

	*name = ಖಾಲಿ;

	ಆದರೆ (!p
	    || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p, "/dm-", 4) != 0
	    || !ಅಂಕಿಯೇ(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ಹಿಂತಿರುಗಿ 0;

	*name = p + 1;
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
ಅಕ್ಷರ *ul_absolute_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	ಆದರೆ (!ul_is_relative_ಮಾರ್ಗ(ಮಾರ್ಗ)) {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಆದರೆ (!getcwd(cwd, ಗಾತ್ರ(cwd)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* simple clean up */
	ಆದರೆ (ul_startswith(ಮಾರ್ಗ, "./"))
		ಮಾರ್ಗ += 2;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(ಮಾರ್ಗ, ".") == 0)
		ಮಾರ್ಗ = ಖಾಲಿ;

	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ strdup(cwd);

	csz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cwd);
	psz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ);

	p = res = ಸ್ಮರಣೆ_ಕೊಡು(csz + 1 + psz + 1);
	ಆದರೆ (!res)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	ಸ್ಮರಣೆ_ನಕಲು(p, ಮಾರ್ಗ, psz + 1);

	ಹಿಂತಿರುಗಿ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __attribute__((nonnull(2)))
do_canonicalize(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಅಕ್ಷರ **ಫಲಿತಾಂಶ,
		ಶೂನ್ಯ *ದತ್ತಾಂಶ __attribute__((__unused__)))
{
	ಅಕ್ಷರ *canonical, *dmname;

	*ಫಲಿತಾಂಶ = ಖಾಲಿ;

	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ) {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ -errno;
	}

	errno = 0;
	canonical = realpath(ಮಾರ್ಗ, ಖಾಲಿ);
	ಆದರೆ (!canonical)
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (is_dm_devname(canonical, &dmname)) {
		ಅಕ್ಷರ *dm = ul_canonicalize_dm_name(dmname);
		ಆದರೆ (dm) {
			ಬಿಡುಗಡೆ(canonical);
			canonical = dm;
		}
	}

	ಆದರೆ (canonical)
		*ಫಲಿತಾಂಶ = canonical;
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
ಅಕ್ಷರ *ul_canonicalize_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *canonical = ಖಾಲಿ;

	ಆದರೆ (do_canonicalize(ಮಾರ್ಗ, &canonical, ಖಾಲಿ) == 1)
		ಹಿಂತಿರುಗಿ strdup(ಮಾರ್ಗ);

	ಹಿಂತಿರುಗಿ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
ಅಕ್ಷರ *ul_canonicalize_ಮಾರ್ಗ_restricted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಹಿಂತಿರುಗಿ ul_restricted_ಮಾರ್ಗ_oper(ಮಾರ್ಗ, do_canonicalize, ಖಾಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_CANONICALIZE
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *p;

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_ಮಾರ್ಗ(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	ಬಿಡುಗಡೆ(p);

	p = ul_canonicalize_ಮಾರ್ಗ_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	ಬಿಡುಗಡೆ(p);

	ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
}
#ಆದರೆ_ಅಂತ್ಯ
