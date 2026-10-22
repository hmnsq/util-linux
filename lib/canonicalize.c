/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#شامل <stdio.h>
#شامل <string.h>
#شامل <ctype.h>
#شامل <unistd.h>
#شامل <errno.h>
#شامل <stdlib.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>

#شامل "canonicalize.h"
#شامل "pathnames.h"
#شامل "all-io.h"
#شامل "strutils.h"
#شامل "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
حرف *ul_canonicalize_dm_name_prefixed(ثابت حرف *سابقہ, ثابت حرف *ptname)
{
	FILE	*f;
	size_t	sz;
	حرف	راستہ[256], name[حجم(راستہ) - حجم(راستہ_DEV_MAPPER)], *res = تہی;

	اگر (!ptname || !*ptname)
		واپس تہی;

	اگر (!سابقہ)
		سابقہ = "";

	snprintf(راستہ, حجم(راستہ), "%s/sys/block/%s/dm/name", سابقہ, ptname);
	اگر (!(f = fopen(راستہ, "r" UL_CLOEXECSTR)))
		واپس تہی;

	/* read "<name>\n" from sysfs */
	اگر (fgets(name, حجم(name), f) && (sz = لڑی_لمبائی(name)) > 1) {
		اگر (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(راستہ, حجم(راستہ), راستہ_DEV_MAPPER "/%s", name);

		اگر ((سابقہ && *سابقہ) || access(راستہ, F_OK) == 0)
			res = strdup(راستہ);
	}
	fclose(f);
	واپس res;
}

حرف *ul_canonicalize_dm_name(ثابت حرف *ptname)
{
	واپس ul_canonicalize_dm_name_prefixed(تہی, ptname);
}

ساکن صحیح is_dm_devname(حرف *canonical, حرف **name)
{
	ساخت stat sb;
	حرف *p = strrchr(canonical, '/');

	*name = تہی;

	اگر (!p
	    || لڑی_موازنہ(p, "/dm-", 4) != 0
	    || !ہندسہ_ہے(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		واپس 0;

	*name = p + 1;
	واپس 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
حرف *ul_absolute_راستہ(ثابت حرف *راستہ)
{
	حرف cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	اگر (!ul_is_relative_راستہ(راستہ)) {
		errno = EINVAL;
		واپس تہی;
	}
	اگر (!getcwd(cwd, حجم(cwd)))
		واپس تہی;

	/* simple clean up */
	اگر (ul_startswith(راستہ, "./"))
		راستہ += 2;
	ورنہ اگر (strcmp(راستہ, ".") == 0)
		راستہ = تہی;

	اگر (!راستہ || !*راستہ)
		واپس strdup(cwd);

	csz = لڑی_لمبائی(cwd);
	psz = لڑی_لمبائی(راستہ);

	p = res = حافظہ_دو(csz + 1 + psz + 1);
	اگر (!res)
		واپس تہی;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	حافظہ_نقل(p, راستہ, psz + 1);

	واپس res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ساکن صحیح __attribute__((nonnull(2)))
do_canonicalize(ثابت حرف *راستہ, حرف **نتیجہ,
		خالی *ڈیٹا __attribute__((__unused__)))
{
	حرف *canonical, *dmname;

	*نتیجہ = تہی;

	اگر (!راستہ || !*راستہ) {
		errno = EINVAL;
		واپس -errno;
	}

	errno = 0;
	canonical = realpath(راستہ, تہی);
	اگر (!canonical)
		واپس 1;

	اگر (is_dm_devname(canonical, &dmname)) {
		حرف *dm = ul_canonicalize_dm_name(dmname);
		اگر (dm) {
			آزاد(canonical);
			canonical = dm;
		}
	}

	اگر (canonical)
		*نتیجہ = canonical;
	واپس 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
حرف *ul_canonicalize_راستہ(ثابت حرف *راستہ)
{
	حرف *canonical = تہی;

	اگر (do_canonicalize(راستہ, &canonical, تہی) == 1)
		واپس strdup(راستہ);

	واپس canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
حرف *ul_canonicalize_راستہ_restricted(ثابت حرف *راستہ)
{
	واپس ul_restricted_راستہ_oper(راستہ, do_canonicalize, تہی);
}

#اگر_تعریف_شدہ TEST_پروگرام_CANONICALIZE
صحیح مین(صحیح argc, حرف **argv)
{
	حرف *p;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		باہر_نکلو(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_راستہ(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	آزاد(p);

	p = ul_canonicalize_راستہ_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	آزاد(p);

	باہر_نکلو(EXIT_SUCCESS);
}
#اگر_ختم
