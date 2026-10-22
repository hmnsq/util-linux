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
वर्ण *ul_canonicalize_dm_name_prefixed(अचर वर्ण *prefix, अचर वर्ण *ptname)
{
	खाता	*f;
	माप_प्रकार	sz;
	वर्ण	path[256], name[परिमाण(path) - परिमाण(_PATH_DEV_MAPPER)], *res = रिक्त;

	यदि (!ptname || !*ptname)
		फर्काऊ रिक्त;

	यदि (!prefix)
		prefix = "";

	snprintf(path, परिमाण(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	यदि (!(f = ख_खोलो(path, "r" UL_CLOEXECSTR)))
		फर्काऊ रिक्त;

	/* read "<name>\n" from sysfs */
	यदि (ख_माला_लो(name, परिमाण(name), f) && (sz = म_माप(name)) > 1) {
		यदि (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, परिमाण(path), _PATH_DEV_MAPPER "/%s", name);

		यदि ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	ख_बंद(f);
	फर्काऊ res;
}

वर्ण *ul_canonicalize_dm_name(अचर वर्ण *ptname)
{
	फर्काऊ ul_canonicalize_dm_name_prefixed(रिक्त, ptname);
}

स्थिर पूर्णाङ्क is_dm_devname(वर्ण *canonical, वर्ण **name)
{
	संरचना stat sb;
	वर्ण *p = म_खोजप(canonical, '/');

	*name = रिक्त;

	यदि (!p
	    || म_भेदन(p, "/dm-", 4) != 0
	    || !है_अंक(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		फर्काऊ 0;

	*name = p + 1;
	फर्काऊ 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
वर्ण *ul_absolute_path(अचर वर्ण *path)
{
	वर्ण cwd[PATH_MAX], *res, *p;
	माप_प्रकार psz, csz;

	यदि (!ul_is_relative_path(path)) {
		त्रुटि_सं = EINVAL;
		फर्काऊ रिक्त;
	}
	यदि (!getcwd(cwd, परिमाण(cwd)))
		फर्काऊ रिक्त;

	/* simple clean up */
	यदि (ul_startswith(path, "./"))
		path += 2;
	अन्यथा यदि (म_भेद(path, ".") == 0)
		path = रिक्त;

	यदि (!path || !*path)
		फर्काऊ strdup(cwd);

	csz = म_माप(cwd);
	psz = म_माप(path);

	p = res = दो_स्मृति(csz + 1 + psz + 1);
	यदि (!res)
		फर्काऊ रिक्त;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	स_नकल(p, path, psz + 1);

	फर्काऊ res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
स्थिर पूर्णाङ्क __attribute__((nonnull(2)))
do_canonicalize(अचर वर्ण *path, वर्ण **result,
		शून्य *data __attribute__((__unused__)))
{
	वर्ण *canonical, *dmname;

	*result = रिक्त;

	यदि (!path || !*path) {
		त्रुटि_सं = EINVAL;
		फर्काऊ -त्रुटि_सं;
	}

	त्रुटि_सं = 0;
	canonical = realpath(path, रिक्त);
	यदि (!canonical)
		फर्काऊ 1;

	यदि (is_dm_devname(canonical, &dmname)) {
		वर्ण *dm = ul_canonicalize_dm_name(dmname);
		यदि (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	यदि (canonical)
		*result = canonical;
	फर्काऊ 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
वर्ण *ul_canonicalize_path(अचर वर्ण *path)
{
	वर्ण *canonical = रिक्त;

	यदि (do_canonicalize(path, &canonical, रिक्त) == 1)
		फर्काऊ strdup(path);

	फर्काऊ canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
वर्ण *ul_canonicalize_path_restricted(अचर वर्ण *path)
{
	फर्काऊ ul_restricted_path_oper(path, do_canonicalize, रिक्त);
}

#ifdef TEST_PROGRAM_CANONICALIZE
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *p;

	यदि (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s <device>\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	ख_लिखो(मानक_निकास, "orig:            %s\n", argv[1]);

	p = ul_canonicalize_path(argv[1]);
	ख_लिखो(मानक_निकास, "real:            %s\n", p);
	मुक्त(p);

	p = ul_canonicalize_path_restricted(argv[1]);
	ख_लिखो(मानक_निकास, "real-restricted: %s\n", p);
	मुक्त(p);

	निकास(निकास_सफल);
}
#endif
