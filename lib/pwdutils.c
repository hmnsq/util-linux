/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "c.h"
#include "pwdutils.h"
#include "xalloc.h"
#include "strutils.h"

sa_nracanaa passwd *xgetpwuid(uid_t uid, var_na **pwdbuf)
{
	sa_nracanaa passwd *pwd = NULL, *res = NULL;
	ចំនួនគត់ rc;

	assert(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, parimaa_na(sa_nracanaa passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ប្រសិនបើ (rc != 0) {
		errno = rc;
		goto failed;
	}
	ប្រសិនបើ (!res) {
		errno = EINVAL;
		goto failed;
	}
	ត្រឡប់ pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	ត្រឡប់ NULL;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
sa_nracanaa passwd *xgetuserpw(const var_na *str ,var_na **pwdbuf)
{
	sa_nracanaa passwd *pwd = NULL, *res = NULL;
	ចំនួនគត់ rc;
	uint64_t uid;

	assert(pwdbuf);
	assert(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, parimaa_na(sa_nracanaa passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ប្រសិនបើ (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ប្រសិនបើ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ប្រសិនបើ (rc != 0) {
		errno = rc;
		goto failed;
	}
	ប្រសិនបើ (!res) {
		errno = EINVAL;
		goto failed;
	}
	ត្រឡប់ pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	ត្រឡប់ NULL;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
sa_nracanaa group *xgetgroup(const var_na *str, var_na **grpbuf)
{
	sa_nracanaa group *grp = NULL, *res = NULL;
	ចំនួនគត់ rc;
	uint64_t gid;

	assert(grpbuf);
	assert(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, parimaa_na(sa_nracanaa group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ប្រសិនបើ (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ប្រសិនបើ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ប្រសិនបើ (rc != 0) {
		errno = rc;
		goto failed;
	}
	ប្រសិនបើ (!res) {
		errno = EINVAL;
		goto failed;
	}
	ត្រឡប់ grp;
failed:
	free(grp);
	free(*grpbuf);
	ត្រឡប់ NULL;
}

var_na *xgetlogin(void)
{
	sa_nracanaa passwd *pw = NULL;
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

	ប្រសិនបើ (errno == 0)
		pw = getpwuid(ruid);
	ប្រសិនបើ (pw && pw->pw_name && *pw->pw_name)
		ត្រឡប់ xstrdup(pw->pw_name);

	ត្រឡប់ NULL;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
sa_nracanaa group *ul_getgrp_str(const var_na *str, gid_t *result)
{
	ចំនួនគត់ rc;
	uint64_t num = 0;
	sa_nracanaa group *gr;

	ប្រសិនបើ (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	ប្រសិនបើ (rc == -ERANGE || num > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ត្រឡប់ NULL;
	}

	/* maybe an actual name */
	ប្រសិនបើ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ប្រសិនបើ (gr && result)
			*result = gr->gr_gid;
		បើមិនដូច្នេះ ប្រសិនបើ (!gr && !errno)
			errno = EINVAL;
		ត្រឡប់ gr;
	}

	ប្រសិនបើ (result)
		*result = (gid_t) num;

	errno = 0;
	gr = getgrgid((gid_t) num);
	ប្រសិនបើ (!gr && !errno)
		errno = EINVAL;

	ត្រឡប់ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
sa_nracanaa passwd *ul_getuserpw_str(const var_na *str, uid_t *result)
{
	ចំនួនគត់ rc;
	uint64_t num = 0;
	sa_nracanaa passwd *pw;

	ប្រសិនបើ (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	ប្រសិនបើ (rc == -ERANGE || num > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ត្រឡប់ NULL;
	}

	/* maybe an actual name */
	ប្រសិនបើ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ប្រសិនបើ (pw && result)
			*result = pw->pw_uid;
		បើមិនដូច្នេះ ប្រសិនបើ (!pw && !errno)
			errno = EINVAL;
		ត្រឡប់ pw;
	}

	ប្រសិនបើ (result)
		*result = (uid_t) num;

	errno = 0;
	pw = getpwuid((uid_t) num);
	ប្រសិនបើ (!pw && !errno)
		errno = EINVAL;

	ត្រឡប់ pw;
}

#ifdef TEST_PROGRAM
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	var_na *buf = NULL;
	sa_nracanaa passwd *pwd = NULL;

	ប្រសិនបើ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ត្រឡប់ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ប្រសិនបើ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	free(pwd);
	free(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	free(buf);

	ត្រឡប់ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
