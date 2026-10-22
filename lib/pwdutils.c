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

struct passwd *xgetpwuid(uid_t uid, char **pwdbuf)
{
	struct passwd *pwd = NULL, *res = NULL;
	整数 rc;

	assert(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, sizeof(struct passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	如果 (rc != 0) {
		errno = rc;
		goto failed;
	}
	如果 (!res) {
		errno = EINVAL;
		goto failed;
	}
	返回 pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	返回 NULL;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
struct passwd *xgetuserpw(const char *str ,char **pwdbuf)
{
	struct passwd *pwd = NULL, *res = NULL;
	整数 rc;
	uint64_t uid;

	assert(pwdbuf);
	assert(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, sizeof(struct passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	如果 (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	如果 (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} 否则 {
		如果 (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	如果 (rc != 0) {
		errno = rc;
		goto failed;
	}
	如果 (!res) {
		errno = EINVAL;
		goto failed;
	}
	返回 pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	返回 NULL;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
struct group *xgetgroup(const char *str, char **grpbuf)
{
	struct group *grp = NULL, *res = NULL;
	整数 rc;
	uint64_t gid;

	assert(grpbuf);
	assert(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, sizeof(struct group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	如果 (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	如果 (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} 否则 {
		如果 (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	如果 (rc != 0) {
		errno = rc;
		goto failed;
	}
	如果 (!res) {
		errno = EINVAL;
		goto failed;
	}
	返回 grp;
failed:
	free(grp);
	free(*grpbuf);
	返回 NULL;
}

char *xgetlogin(void)
{
	struct passwd *pw = NULL;
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

	如果 (errno == 0)
		pw = getpwuid(ruid);
	如果 (pw && pw->pw_name && *pw->pw_name)
		返回 xstrdup(pw->pw_name);

	返回 NULL;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
struct group *ul_getgrp_str(const char *str, gid_t *result)
{
	整数 rc;
	uint64_t num = 0;
	struct group *gr;

	如果 (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	如果 (rc == -ERANGE || num > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		返回 NULL;
	}

	/* maybe an actual name */
	如果 (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		如果 (gr && result)
			*result = gr->gr_gid;
		否则 如果 (!gr && !errno)
			errno = EINVAL;
		返回 gr;
	}

	如果 (result)
		*result = (gid_t) num;

	errno = 0;
	gr = getgrgid((gid_t) num);
	如果 (!gr && !errno)
		errno = EINVAL;

	返回 gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
struct passwd *ul_getuserpw_str(const char *str, uid_t *result)
{
	整数 rc;
	uint64_t num = 0;
	struct passwd *pw;

	如果 (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	如果 (rc == -ERANGE || num > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		返回 NULL;
	}

	/* maybe an actual name */
	如果 (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		如果 (pw && result)
			*result = pw->pw_uid;
		否则 如果 (!pw && !errno)
			errno = EINVAL;
		返回 pw;
	}

	如果 (result)
		*result = (uid_t) num;

	errno = 0;
	pw = getpwuid((uid_t) num);
	如果 (!pw && !errno)
		errno = EINVAL;

	返回 pw;
}

#ifdef TEST_PROGRAM
整数 主要(整数 argc, char *argv[])
{
	char *buf = NULL;
	struct passwd *pwd = NULL;

	如果 (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		返回 EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	如果 (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	free(pwd);
	free(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	free(buf);

	返回 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
