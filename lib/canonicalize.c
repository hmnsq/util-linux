/*
 * canonicalize.c -- canonicalize pathname by removing symlinks
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2009-2013 Karel Zak <kzak@redhat.com>
 */
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <ctype.h>
#समावेश <unistd.h>
#समावेश <errno.h>
#समावेश <stdlib.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>

#समावेश "canonicalize.h"
#समावेश "pathnames.h"
#समावेश "all-io.h"
#समावेश "strutils.h"
#समावेश "fileutils.h"

/*
 * Converts private "dm-N" names to "/dev/mapper/<name>"
 *
 * Since 2.6.29 (patch 784aae735d9b0bba3f8b9faef4c8b30df3bf0128) kernel sysfs
 * provides the real DM device names in /sys/block/<ptname>/dm/name
 *
 * The @prefix allows /sys to be mounted or stored outside the system root
 * (/prefix/sys/...).
 */
अक्षर *ul_canonicalize_dm_name_prefixed(स्थिर अक्षर *prefix, स्थिर अक्षर *ptname)
{
	खाता	*f;
	माप_प्रकार	sz;
	अक्षर	path[256], name[माप(path) - माप(_PATH_DEV_MAPPER)], *res = शून्य;

	अगर (!ptname || !*ptname)
		वापस शून्य;

	अगर (!prefix)
		prefix = "";

	snprintf(path, माप(path), "%s/sys/block/%s/dm/name", prefix, ptname);
	अगर (!(f = ख_खोलो(path, "r" UL_CLOEXECSTR)))
		वापस शून्य;

	/* read "<name>\n" from sysfs */
	अगर (ख_माला_लो(name, माप(name), f) && (sz = म_माप(name)) > 1) {
		अगर (name[sz - 1] == '\n')
			name[sz - 1] = '\0';

		snprintf(path, माप(path), _PATH_DEV_MAPPER "/%s", name);

		अगर ((prefix && *prefix) || access(path, F_OK) == 0)
			res = strdup(path);
	}
	ख_बंद(f);
	वापस res;
}

अक्षर *ul_canonicalize_dm_name(स्थिर अक्षर *ptname)
{
	वापस ul_canonicalize_dm_name_prefixed(शून्य, ptname);
}

अटल पूर्णांक is_dm_devname(अक्षर *canonical, अक्षर **name)
{
	काष्ठा stat sb;
	अक्षर *p = म_खोजप(canonical, '/');

	*name = शून्य;

	अगर (!p
	    || म_भेदन(p, "/dm-", 4) != 0
	    || !है_अंक(*(p + 4))
	    || stat(canonical, &sb) != 0
	    || !S_ISBLK(sb.st_mode))
		वापस 0;

	*name = p + 1;
	वापस 1;
}

/*
 * This function does not canonicalize the path! It just prepends CWD before a
 * relative path. If the path is no relative than returns NULL. The path does
 * not have to exist.
 */
अक्षर *ul_absolute_path(स्थिर अक्षर *path)
{
	अक्षर cwd[PATH_MAX], *res, *p;
	माप_प्रकार psz, csz;

	अगर (!ul_is_relative_path(path)) {
		त्रुटि_सं = EINVAL;
		वापस शून्य;
	}
	अगर (!getcwd(cwd, माप(cwd)))
		वापस शून्य;

	/* simple clean up */
	अगर (ul_startswith(path, "./"))
		path += 2;
	अन्यथा अगर (म_भेद(path, ".") == 0)
		path = शून्य;

	अगर (!path || !*path)
		वापस strdup(cwd);

	csz = म_माप(cwd);
	psz = म_माप(path);

	p = res = दो_स्मृति(csz + 1 + psz + 1);
	अगर (!res)
		वापस शून्य;

	p = mempcpy(p, cwd, csz);
	*p++ = '/';
	स_नकल(p, path, psz + 1);

	वापस res;
}

/*
 * Returns: <0 on error, 1 is cannot be canonicalized (errno is set); 0 on success
 */
अटल पूर्णांक __attribute__((nonnull(2)))
do_canonicalize(स्थिर अक्षर *path, अक्षर **परिणाम,
		व्योम *तथ्य __attribute__((__unused__)))
{
	अक्षर *canonical, *dmname;

	*परिणाम = शून्य;

	अगर (!path || !*path) {
		त्रुटि_सं = EINVAL;
		वापस -त्रुटि_सं;
	}

	त्रुटि_सं = 0;
	canonical = realpath(path, शून्य);
	अगर (!canonical)
		वापस 1;

	अगर (is_dm_devname(canonical, &dmname)) {
		अक्षर *dm = ul_canonicalize_dm_name(dmname);
		अगर (dm) {
			मुक्त(canonical);
			canonical = dm;
		}
	}

	अगर (canonical)
		*परिणाम = canonical;
	वापस 0;
}

/*
 * Always returns a newly allocated string or NULL in case of an error. An
 * unreachable path is not an error (!), and in this case, it just duplicates
 * @path.
 */
अक्षर *ul_canonicalize_path(स्थिर अक्षर *path)
{
	अक्षर *canonical = शून्य;

	अगर (do_canonicalize(path, &canonical, शून्य) == 1)
		वापस strdup(path);

	वापस canonical;
}

/*
 * Drop permissions (e.g., suid) and canonicalize the path. If the path is
 * unreadable (for example, due to missing permissions), it returns NULL.
 */
अक्षर *ul_canonicalize_path_restricted(स्थिर अक्षर *path)
{
	वापस ul_restricted_path_oper(path, do_canonicalize, शून्य);
}

#अगर_घोषित TEST_PROGRAM_CANONICALIZE
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *p;

	अगर (argc < 2) {
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
#पूर्ण_अगर
