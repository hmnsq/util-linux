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
	もし (rc != 0) {
		errno = rc;
		goto failed;
	}
	もし (!res) {
		errno = EINVAL;
		goto failed;
	}
	戻る pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	戻る NULL;
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
	もし (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	もし (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} 否则 {
		もし (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	もし (rc != 0) {
		errno = rc;
		goto failed;
	}
	もし (!res) {
		errno = EINVAL;
		goto failed;
	}
	戻る pwd;
failed:
	free(pwd);
	free(*pwdbuf);
	戻る NULL;
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
	もし (rc == -ERANGE) {
		errno = ERANGE;
		goto failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	もし (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} 否则 {
		もし (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			goto failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	もし (rc != 0) {
		errno = rc;
		goto failed;
	}
	もし (!res) {
		errno = EINVAL;
		goto failed;
	}
	戻る grp;
failed:
	free(grp);
	free(*grpbuf);
	戻る NULL;
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

	もし (errno == 0)
		pw = getpwuid(ruid);
	もし (pw && pw->pw_name && *pw->pw_name)
		戻る xstrdup(pw->pw_name);

	戻る NULL;
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

	もし (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	もし (rc == -ERANGE || num > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		戻る NULL;
	}

	/* maybe an actual name */
	もし (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		もし (gr && result)
			*result = gr->gr_gid;
		否则 もし (!gr && !errno)
			errno = EINVAL;
		戻る gr;
	}

	もし (result)
		*result = (gid_t) num;

	errno = 0;
	gr = getgrgid((gid_t) num);
	もし (!gr && !errno)
		errno = EINVAL;

	戻る gr;
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

	もし (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	もし (rc == -ERANGE || num > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		戻る NULL;
	}

	/* maybe an actual name */
	もし (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		もし (pw && result)
			*result = pw->pw_uid;
		否则 もし (!pw && !errno)
			errno = EINVAL;
		戻る pw;
	}

	もし (result)
		*result = (uid_t) num;

	errno = 0;
	pw = getpwuid((uid_t) num);
	もし (!pw && !errno)
		errno = EINVAL;

	戻る pw;
}

#ifdef TEST_PROGRAM
整数 メインページ(整数 argc, char *argv[])
{
	char *buf = NULL;
	struct passwd *pwd = NULL;

	もし (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		戻る EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	もし (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	free(pwd);
	free(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	free(buf);

	戻る EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
