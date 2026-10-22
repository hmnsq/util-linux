/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "pwdutils.h"
#अन्तर्भाव "xalloc.h"
#अन्तर्भाव "strutils.h"

संरचना passwd *xgetpwuid(uid_t uid, वर्ण **pwdbuf)
{
	संरचना passwd *pwd = रिक्त, *res = रिक्त;
	पूर्णाङ्क rc;

	प्रतिज्ञा(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, परिमाण(संरचना passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	जइ (rc != 0) {
		errno = rc;
		गच्छ failed;
	}
	जइ (!res) {
		errno = EINVAL;
		गच्छ failed;
	}
	प्रत्यागम pwd;
failed:
	मुक्त(pwd);
	मुक्त(*pwdbuf);
	प्रत्यागम रिक्त;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
संरचना passwd *xgetuserpw(ध्रुव वर्ण *str ,वर्ण **pwdbuf)
{
	संरचना passwd *pwd = रिक्त, *res = रिक्त;
	पूर्णाङ्क rc;
	uint64_t uid;

	प्रतिज्ञा(pwdbuf);
	प्रतिज्ञा(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, परिमाण(संरचना passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	जइ (rc == -ERANGE) {
		errno = ERANGE;
		गच्छ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	जइ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} अण्णहा {
		जइ (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			गच्छ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	जइ (rc != 0) {
		errno = rc;
		गच्छ failed;
	}
	जइ (!res) {
		errno = EINVAL;
		गच्छ failed;
	}
	प्रत्यागम pwd;
failed:
	मुक्त(pwd);
	मुक्त(*pwdbuf);
	प्रत्यागम रिक्त;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
संरचना group *xgetgroup(ध्रुव वर्ण *str, वर्ण **grpbuf)
{
	संरचना group *grp = रिक्त, *res = रिक्त;
	पूर्णाङ्क rc;
	uint64_t gid;

	प्रतिज्ञा(grpbuf);
	प्रतिज्ञा(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, परिमाण(संरचना group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	जइ (rc == -ERANGE) {
		errno = ERANGE;
		गच्छ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	जइ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} अण्णहा {
		जइ (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			गच्छ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	जइ (rc != 0) {
		errno = rc;
		गच्छ failed;
	}
	जइ (!res) {
		errno = EINVAL;
		गच्छ failed;
	}
	प्रत्यागम grp;
failed:
	मुक्त(grp);
	मुक्त(*grpbuf);
	प्रत्यागम रिक्त;
}

वर्ण *xgetlogin(शून्य)
{
	संरचना passwd *pw = रिक्त;
	uid_t ruid;

	/* GNU Hurd implementation has an extension where a process can exist in a
	 * non-conforming environment, and thus be outside the realms of POSIX
	 * process identifiers; on this platform, getuid() fails with a status of
	 * (uid_t)(-1) and sets errno if a program is run from a non-conforming
	 * environment.
	 *
	 * http://austingroupbugs.net/view.php?id=511
	 *
	 * The same implementation is useful for other systems, since getlogin(3)
	 * shouldn't be used as actual identification.
	 */
	errno = 0;
	ruid = getuid();

	जइ (errno == 0)
		pw = getpwuid(ruid);
	जइ (pw && pw->pw_name && *pw->pw_name)
		प्रत्यागम xstrdup(pw->pw_name);

	प्रत्यागम रिक्त;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
संरचना group *ul_getgrp_str(ध्रुव वर्ण *str, gid_t *result)
{
	पूर्णाङ्क rc;
	uint64_t अङ्कः = 0;
	संरचना group *gr;

	जइ (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &अङ्कः, 10);
	जइ (rc == -ERANGE || अङ्कः > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		प्रत्यागम रिक्त;
	}

	/* maybe an actual name */
	जइ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		जइ (gr && result)
			*result = gr->gr_gid;
		अण्णहा जइ (!gr && !errno)
			errno = EINVAL;
		प्रत्यागम gr;
	}

	जइ (result)
		*result = (gid_t) अङ्कः;

	errno = 0;
	gr = getgrgid((gid_t) अङ्कः);
	जइ (!gr && !errno)
		errno = EINVAL;

	प्रत्यागम gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
संरचना passwd *ul_getuserpw_str(ध्रुव वर्ण *str, uid_t *result)
{
	पूर्णाङ्क rc;
	uint64_t अङ्कः = 0;
	संरचना passwd *pw;

	जइ (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &अङ्कः, 10);
	जइ (rc == -ERANGE || अङ्कः > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		प्रत्यागम रिक्त;
	}

	/* maybe an actual name */
	जइ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		जइ (pw && result)
			*result = pw->pw_uid;
		अण्णहा जइ (!pw && !errno)
			errno = EINVAL;
		प्रत्यागम pw;
	}

	जइ (result)
		*result = (uid_t) अङ्कः;

	errno = 0;
	pw = getpwuid((uid_t) अङ्कः);
	जइ (!pw && !errno)
		errno = EINVAL;

	प्रत्यागम pw;
}

#ifdef TEST_PROGRAM
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *buf = रिक्त;
	संरचना passwd *pwd = रिक्त;

	जइ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		प्रत्यागम EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	जइ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	मुक्त(pwd);
	मुक्त(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	मुक्त(buf);

	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
