/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ضمن <unistd.h>
#ضمن <errno.h>
#ضمن <stdlib.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>

#ضمن "canonicalize.h"
#ضمن "pathnames.h"
#ضمن "all-io.h"
#ضمن "strutils.h"
#ضمن "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
حرف *ul_canonicalize_dm_name_prefixed(ثابت حرف *اگیتر, ثابت حرف *ptname)
{
	FILE	*f;
	size_t	sz;
	حرف	رستہ[256], name[حجم(رستہ) - حجم(رستہ_DEV_MAPPER)], *res = عدم;

	إذا (!ptname || !*ptname)
		أرجع عدم;

	إذا (!اگیتر)
		اگیتر = "";

	snprintf(رستہ, حجم(رستہ), "%s/sys/block/%s/dm/name", اگیتر, ptname);
	إذا (!(f = fopen(رستہ, "r" UL_CLOEXECSTR)))
		أرجع عدم;

	/* read "<name>\n" from sysfs */
	إذا (fgets(name, حجم(name), f) && (sz = لڑی_لمبائی(name)) > 1) {
		إذا (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(رستہ, حجم(رستہ), رستہ_DEV_MAPPER "/%s", name);

		إذا ((اگیتر && *اگیتر) || access(رستہ, F_OK) == 0)
			res = strdup(رستہ);
	}
	fclose(f);
	أرجع res;
}

حرف *ul_canonicalize_dm_name(ثابت حرف *ptname)
{
	أرجع ul_canonicalize_dm_name_prefixed(عدم, ptname);
}

ساكن صحيح is_dm_devname(حرف *canonical, حرف **name)
{
	بنية stat sb;
	حرف *p = strrchr(canonical, '/');

	*name = عدم;

	إذا (!p
	    || لڑی_مقابلہ(p, "/dm-", 4) != 0
	    || !ہندسہ_اے(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		أرجع 0;

	*name = p + 1;
	أرجع 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
حرف *ul_absolute_رستہ(ثابت حرف *رستہ)
{
	حرف cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	إذا (!ul_is_relative_رستہ(رستہ)) {
		errno = EINVAL;
		أرجع عدم;
	}
	إذا (!getcwd(cwd, حجم(cwd)))
		أرجع عدم;

	/* simple clean up */
	إذا (ul_startswith(رستہ, "./"))
		رستہ += 2;
	وإلا إذا (strcmp(رستہ, ".") == 0)
		رستہ = عدم;

	إذا (!رستہ || !*رستہ)
		أرجع strdup(cwd);

	csz = لڑی_لمبائی(cwd);
	psz = لڑی_لمبائی(رستہ);

	p = res = حافظہ_دیو(csz + 1 + psz + 1);
	إذا (!res)
		أرجع عدم;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	حافظہ_نقل(p, رستہ, psz + 1);

	أرجع res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ساكن صحيح __attribute__((nonnull(2)))
do_canonicalize(ثابت حرف *رستہ, حرف **نتیجہ,
		فراغ *انکڑے __attribute__((__unused__)))
{
	حرف *canonical, *dmname;

	*نتیجہ = عدم;

	إذا (!رستہ || !*رستہ) {
		errno = EINVAL;
		أرجع -errno;
	}

	errno = 0;
	canonical = realpath(رستہ, عدم);
	إذا (!canonical)
		أرجع 1;

	إذا (is_dm_devname(canonical, &dmname)) {
		حرف *dm = ul_canonicalize_dm_name(dmname);
		إذا (dm) {
			حرر(canonical);
			canonical = dm;
		}
	}

	إذا (canonical)
		*نتیجہ = canonical;
	أرجع 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
حرف *ul_canonicalize_رستہ(ثابت حرف *رستہ)
{
	حرف *canonical = عدم;

	إذا (do_canonicalize(رستہ, &canonical, عدم) == 1)
		أرجع strdup(رستہ);

	أرجع canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
حرف *ul_canonicalize_رستہ_restricted(ثابت حرف *رستہ)
{
	أرجع ul_restricted_رستہ_oper(رستہ, do_canonicalize, عدم);
}

#جے_تعریف_شدہ TEST_پروگرام_CANONICALIZE
صحيح مکھ(صحيح argc, حرف **argv)
{
	حرف *p;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		باہر_نکلو(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_رستہ(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	حرر(p);

	p = ul_canonicalize_رستہ_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	حرر(p);

	باہر_نکلو(EXIT_SUCCESS);
}
#جے_انت
