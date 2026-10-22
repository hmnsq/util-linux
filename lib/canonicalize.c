/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>

#અન્તર્ભાવ "canonicalize.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
અક્ષર *ul_canonicalize_dm_name_prefixed(અચલ અક્ષર *prefix, અચલ અક્ષર *ptname)
{
	FILE	*f;
	size_t	sz;
	અક્ષર	path[256], name[કદ(path) - કદ(_PATH_DEV_MAPPER)], *res = ખાલી;

	જો (!ptname || !*ptname)
		પાછા ખાલી;

	જો (!prefix)
		prefix = "";

	snprintf(path, કદ(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	જો (!(f = fopen(path, "r" UL_CLOEXECSTR)))
		પાછા ખાલી;

	/* read "<name>\n" from sysfs */
	જો (fgets(name, કદ(name), f) && (sz = strlen(name)) > 1) {
		જો (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, કદ(path), _PATH_DEV_MAPPER "/%s", name);

		જો ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	fclose(f);
	પાછા res;
}

અક્ષર *ul_canonicalize_dm_name(અચલ અક્ષર *ptname)
{
	પાછા ul_canonicalize_dm_name_prefixed(ખાલી, ptname);
}

સ્થિર પૂર્ણાંક is_dm_devname(અક્ષર *canonical, અક્ષર **name)
{
	સંરચના stat sb;
	અક્ષર *p = strrchr(canonical, '/');

	*name = ખાલી;

	જો (!p
	    || strncmp(p, "/dm-", 4) != 0
	    || !isdigit(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		પાછા 0;

	*name = p + 1;
	પાછા 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
અક્ષર *ul_absolute_path(અચલ અક્ષર *path)
{
	અક્ષર cwd[PATH_MAX], *res, *p;
	size_t psz, csz;

	જો (!ul_is_relative_path(path)) {
		errno = EINVAL;
		પાછા ખાલી;
	}
	જો (!getcwd(cwd, કદ(cwd)))
		પાછા ખાલી;

	/* simple clean up */
	જો (ul_startswith(path, "./"))
		path += 2;
	નહીંતર જો (strcmp(path, ".") == 0)
		path = ખાલી;

	જો (!path || !*path)
		પાછા strdup(cwd);

	csz = strlen(cwd);
	psz = strlen(path);

	p = res = malloc(csz + 1 + psz + 1);
	જો (!res)
		પાછા ખાલી;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	memcpy(p, path, psz + 1);

	પાછા res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
સ્થિર પૂર્ણાંક __attribute__((nonnull(2)))
do_canonicalize(અચલ અક્ષર *path, અક્ષર **result,
		શૂન્ય *data __attribute__((__unused__)))
{
	અક્ષર *canonical, *dmname;

	*result = ખાલી;

	જો (!path || !*path) {
		errno = EINVAL;
		પાછા -errno;
	}

	errno = 0;
	canonical = realpath(path, ખાલી);
	જો (!canonical)
		પાછા 1;

	જો (is_dm_devname(canonical, &dmname)) {
		અક્ષર *dm = ul_canonicalize_dm_name(dmname);
		જો (dm) {
			મુક્ત(canonical);
			canonical = dm;
		}
	}

	જો (canonical)
		*result = canonical;
	પાછા 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
અક્ષર *ul_canonicalize_path(અચલ અક્ષર *path)
{
	અક્ષર *canonical = ખાલી;

	જો (do_canonicalize(path, &canonical, ખાલી) == 1)
		પાછા strdup(path);

	પાછા canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
અક્ષર *ul_canonicalize_path_restricted(અચલ અક્ષર *path)
{
	પાછા ul_restricted_path_oper(path, do_canonicalize, ખાલી);
}

#ifdef TEST_PROGRAM_CANONICALIZE
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	અક્ષર *p;

	જો (argc < 2) {
		fprintf(stderr, "usage: %s <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	fprintf(stdout, "real:            %s\n", p);
	મુક્ત(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	fprintf(stdout, "real-restricted: %s\n", p);
	મુક્ત(p);

	exit(EXIT_SUCCESS);
}
#endif
