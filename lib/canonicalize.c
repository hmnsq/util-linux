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
अक्षर *ul_canonicalize_dm_name_prefixed(स्थिरांक अक्षर *prefix, स्थिरांक अक्षर *ptname)
{
	खाता	*f;
	माप_प्रकार	sz;
	अक्षर	path[256], name[आकार(path) - आकार(_PATH_DEV_MAPPER)], *res = रिक्त;

	जर (!ptname || !*ptname)
		परत रिक्त;

	जर (!prefix)
		prefix = "";

	snprintf(path, आकार(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	जर (!(f = ख_खोलो(path, "r" UL_CLOEXECSTR)))
		परत रिक्त;

	/* read "<name>\n" from sysfs */
	जर (ख_माला_लो(name, आकार(name), f) && (sz = म_माप(name)) > 1) {
		जर (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, आकार(path), _PATH_DEV_MAPPER "/%s", name);

		जर ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	ख_बंद(f);
	परत res;
}

अक्षर *ul_canonicalize_dm_name(स्थिरांक अक्षर *ptname)
{
	परत ul_canonicalize_dm_name_prefixed(रिक्त, ptname);
}

स्थिर पूर्णांक is_dm_devname(अक्षर *canonical, अक्षर **name)
{
	संरचना stat sb;
	अक्षर *p = म_खोजप(canonical, '/');

	*name = रिक्त;

	जर (!p
	    || म_भेदन(p, "/dm-", 4) != 0
	    || !है_अंक(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		परत 0;

	*name = p + 1;
	परत 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
अक्षर *ul_absolute_path(स्थिरांक अक्षर *path)
{
	अक्षर cwd[PATH_MAX], *res, *p;
	माप_प्रकार psz, csz;

	जर (!ul_is_relative_path(path)) {
		त्रुटि_सं = EINVAL;
		परत रिक्त;
	}
	जर (!getcwd(cwd, आकार(cwd)))
		परत रिक्त;

	/* simple clean up */
	जर (ul_startswith(path, "./"))
		path += 2;
	अन्यथा जर (म_भेद(path, ".") == 0)
		path = रिक्त;

	जर (!path || !*path)
		परत strdup(cwd);

	csz = म_माप(cwd);
	psz = म_माप(path);

	p = res = दो_स्मृति(csz + 1 + psz + 1);
	जर (!res)
		परत रिक्त;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	स_नकल(p, path, psz + 1);

	परत res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
स्थिर पूर्णांक __attribute__((nonnull(2)))
do_canonicalize(स्थिरांक अक्षर *path, अक्षर **result,
		शून्य *data __attribute__((__unused__)))
{
	अक्षर *canonical, *dmname;

	*result = रिक्त;

	जर (!path || !*path) {
		त्रुटि_सं = EINVAL;
		परत -त्रुटि_सं;
	}

	त्रुटि_सं = 0;
	canonical = realpath(path, रिक्त);
	जर (!canonical)
		परत 1;

	जर (is_dm_devname(canonical, &dmname)) {
		अक्षर *dm = ul_canonicalize_dm_name(dmname);
		जर (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	जर (canonical)
		*result = canonical;
	परत 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
अक्षर *ul_canonicalize_path(स्थिरांक अक्षर *path)
{
	अक्षर *canonical = रिक्त;

	जर (do_canonicalize(path, &canonical, रिक्त) == 1)
		परत strdup(path);

	परत canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
अक्षर *ul_canonicalize_path_restricted(स्थिरांक अक्षर *path)
{
	परत ul_restricted_path_oper(path, do_canonicalize, रिक्त);
}

#ifdef TEST_PROGRAM_CANONICALIZE
पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *p;

	जर (argc < 2) {
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
