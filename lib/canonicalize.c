/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "canonicalize.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
ਅੱਖਰ *ul_canonicalize_dm_name_prefixed(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਅਚਲ ਅੱਖਰ *ptname)
{
	FILE	*f;
	size_t	sz;
	ਅੱਖਰ	ਰਸਤਾ[256], name[ਆਕਾਰ(ਰਸਤਾ) - ਆਕਾਰ(ਰਸਤਾ_DEV_MAPPER)], *res = ਨਿਰਾ;

	ਜੇ (!ptname || !*ptname)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!ਅਗੇਤਰ)
		ਅਗੇਤਰ = "";

	snprintf(ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ), "%s/sys/block/%s/dm/name", ਅਗੇਤਰ, ptname);
	ਜੇ (!(f = fopen(ਰਸਤਾ, "r" UL_CLOEXECSTR)))
		ਵਾਪਸ ਨਿਰਾ;

	/* read "<name>\n" from sysfs */
	ਜੇ (fgets(name, ਆਕਾਰ(name), f) && (sz = ਲੜੀ_ਲੰਬਾਈ(name)) > 1) {
		ਜੇ (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ), ਰਸਤਾ_DEV_MAPPER "/%s", name);

		ਜੇ ((ਅਗੇਤਰ && *ਅਗੇਤਰ) || access(ਰਸਤਾ, F_OK) == 0)
			res = strdup(ਰਸਤਾ);
	}
	fclose(f);
	ਵਾਪਸ res;
}

ਅੱਖਰ *ul_canonicalize_dm_name(ਅਚਲ ਅੱਖਰ *ptname)
{
	ਵਾਪਸ ul_canonicalize_dm_name_prefixed(ਨਿਰਾ, ptname);
}

ਸਥਿਰ ਪੂਰਨ is_dm_devname(ਅੱਖਰ *canonical, ਅੱਖਰ **name)
{
	ਬਣਤਰ stat sb;
	ਅੱਖਰ *p = strrchr(canonical, '/');

	*name = ਨਿਰਾ;

	ਜੇ (!p
	    || ਲੜੀ_ਤੁਲਨਾ(p, "/dm-", 4) != 0
	    || !ਅੰਕ_ਹੈ(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		ਵਾਪਸ 0;

	*name = p + 1;
	ਵਾਪਸ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
ਅੱਖਰ *ul_absolute_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	ਜੇ (!ul_is_relative_ਰਸਤਾ(ਰਸਤਾ)) {
		errno = EINVAL;
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ (!getcwd(cwd, ਆਕਾਰ(cwd)))
		ਵਾਪਸ ਨਿਰਾ;

	/* simple clean up */
	ਜੇ (ul_startswith(ਰਸਤਾ, "./"))
		ਰਸਤਾ += 2;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(ਰਸਤਾ, ".") == 0)
		ਰਸਤਾ = ਨਿਰਾ;

	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ)
		ਵਾਪਸ strdup(cwd);

	csz = ਲੜੀ_ਲੰਬਾਈ(cwd);
	psz = ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ);

	p = res = ਸਮ੍ਰਿਤੀ_ਦਿਓ(csz + 1 + psz + 1);
	ਜੇ (!res)
		ਵਾਪਸ ਨਿਰਾ;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(p, ਰਸਤਾ, psz + 1);

	ਵਾਪਸ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ਸਥਿਰ ਪੂਰਨ __attribute__((nonnull(2)))
do_canonicalize(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅੱਖਰ **ਨਤੀਜਾ,
		ਖਾਲੀ *ਅੰਕੜੇ __attribute__((__unused__)))
{
	ਅੱਖਰ *canonical, *dmname;

	*ਨਤੀਜਾ = ਨਿਰਾ;

	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ) {
		errno = EINVAL;
		ਵਾਪਸ -errno;
	}

	errno = 0;
	canonical = realpath(ਰਸਤਾ, ਨਿਰਾ);
	ਜੇ (!canonical)
		ਵਾਪਸ 1;

	ਜੇ (is_dm_devname(canonical, &dmname)) {
		ਅੱਖਰ *dm = ul_canonicalize_dm_name(dmname);
		ਜੇ (dm) {
			ਮੁਕਤ(canonical);
			canonical = dm;
		}
	}

	ਜੇ (canonical)
		*ਨਤੀਜਾ = canonical;
	ਵਾਪਸ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
ਅੱਖਰ *ul_canonicalize_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *canonical = ਨਿਰਾ;

	ਜੇ (do_canonicalize(ਰਸਤਾ, &canonical, ਨਿਰਾ) == 1)
		ਵਾਪਸ strdup(ਰਸਤਾ);

	ਵਾਪਸ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
ਅੱਖਰ *ul_canonicalize_ਰਸਤਾ_restricted(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਵਾਪਸ ul_restricted_ਰਸਤਾ_oper(ਰਸਤਾ, do_canonicalize, ਨਿਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_CANONICALIZE
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *p;

	ਜੇ (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_ਰਸਤਾ(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	ਮੁਕਤ(p);

	p = ul_canonicalize_ਰਸਤਾ_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	ਮੁਕਤ(p);

	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
}
#ਜੇ_ਅੰਤ
