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
حرف *ul_canonicalize_dm_name_prefixed(ثابت حرف *پیشوند, ثابت حرف *ptname)
{
	FILE	*f;
	size_t	sz;
	حرف	مسیر[256], name[حجم(مسیر) - حجم(مسیر_DEV_MAPPER)], *res = عدم;

	إذا (!ptname || !*ptname)
		أرجع عدم;

	إذا (!پیشوند)
		پیشوند = "";

	snprintf(مسیر, حجم(مسیر), "%s/sys/block/%s/dm/name", پیشوند, ptname);
	إذا (!(f = fopen(مسیر, "r" UL_CLOEXECSTR)))
		أرجع عدم;

	/* read "<name>\n" from sysfs */
	إذا (fgets(name, حجم(name), f) && (sz = طول_رشته(name)) > 1) {
		إذا (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(مسیر, حجم(مسیر), مسیر_DEV_MAPPER "/%s", name);

		إذا ((پیشوند && *پیشوند) || access(مسیر, F_OK) == 0)
			res = strdup(مسیر);
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
	    || مقایسه_رشته(p, "/dm-", 4) != 0
	    || !آیا_رقم(*(p + 4))
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
حرف *ul_absolute_مسیر(ثابت حرف *مسیر)
{
	حرف cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	إذا (!ul_is_relative_مسیر(مسیر)) {
		errno = EINVAL;
		أرجع عدم;
	}
	إذا (!getcwd(cwd, حجم(cwd)))
		أرجع عدم;

	/* simple clean up */
	إذا (ul_startswith(مسیر, "./"))
		مسیر += 2;
	وإلا إذا (strcmp(مسیر, ".") == 0)
		مسیر = عدم;

	إذا (!مسیر || !*مسیر)
		أرجع strdup(cwd);

	csz = طول_رشته(cwd);
	psz = طول_رشته(مسیر);

	p = res = حافظه_بده(csz + 1 + psz + 1);
	إذا (!res)
		أرجع عدم;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	کپی_حافظه(p, مسیر, psz + 1);

	أرجع res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
ساكن صحيح __attribute__((nonnull(2)))
do_canonicalize(ثابت حرف *مسیر, حرف **نتیجه,
		فراغ *داده __attribute__((__unused__)))
{
	حرف *canonical, *dmname;

	*نتیجه = عدم;

	إذا (!مسیر || !*مسیر) {
		errno = EINVAL;
		أرجع -errno;
	}

	errno = 0;
	canonical = realpath(مسیر, عدم);
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
		*نتیجه = canonical;
	أرجع 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
حرف *ul_canonicalize_مسیر(ثابت حرف *مسیر)
{
	حرف *canonical = عدم;

	إذا (do_canonicalize(مسیر, &canonical, عدم) == 1)
		أرجع strdup(مسیر);

	أرجع canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
حرف *ul_canonicalize_مسیر_restricted(ثابت حرف *مسیر)
{
	أرجع ul_restricted_مسیر_oper(مسیر, do_canonicalize, عدم);
}

#اگر_تعریف_شده TEST_برنامه_CANONICALIZE
صحيح اصلی(صحيح argc, حرف **argv)
{
	حرف *p;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		خروج(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_مسیر(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	حرر(p);

	p = ul_canonicalize_مسیر_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	حرر(p);

	خروج(EXIT_SUCCESS);
}
#پایان_اگر
