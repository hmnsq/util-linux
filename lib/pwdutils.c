/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <stdlib.h>
#ضمن <assert.h>
#ضمن <unistd.h>

#ضمن "c.h"
#ضمن "pwdutils.h"
#ضمن "xalloc.h"
#ضمن "strutils.h"

بنية passwd *xgetpwuid(uid_t uid, حرف **pwdbuf)
{
	بنية passwd *pwd = عدم, *res = عدم;
	صحيح rc;

	أكد(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, حجم(بنية passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	𐡄𐡍 (rc != 0) {
		errno = rc;
		اذهب failed;
	}
	𐡄𐡍 (!res) {
		errno = EINVAL;
		اذهب failed;
	}
	أرجع pwd;
failed:
	حرر(pwd);
	حرر(*pwdbuf);
	أرجع عدم;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
بنية passwd *xgetuserpw(ثابت حرف *str ,حرف **pwdbuf)
{
	بنية passwd *pwd = عدم, *res = عدم;
	صحيح rc;
	uint64_t uid;

	أكد(pwdbuf);
	أكد(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, حجم(بنية passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	𐡄𐡍 (rc == -ERANGE) {
		errno = ERANGE;
		اذهب failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	𐡄𐡍 (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} وإلا {
		𐡄𐡍 (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			اذهب failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	𐡄𐡍 (rc != 0) {
		errno = rc;
		اذهب failed;
	}
	𐡄𐡍 (!res) {
		errno = EINVAL;
		اذهب failed;
	}
	أرجع pwd;
failed:
	حرر(pwd);
	حرر(*pwdbuf);
	أرجع عدم;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
بنية group *xgetgroup(ثابت حرف *str, حرف **grpbuf)
{
	بنية group *grp = عدم, *res = عدم;
	صحيح rc;
	uint64_t gid;

	أكد(grpbuf);
	أكد(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, حجم(بنية group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	𐡄𐡍 (rc == -ERANGE) {
		errno = ERANGE;
		اذهب failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	𐡄𐡍 (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} وإلا {
		𐡄𐡍 (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			اذهب failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	𐡄𐡍 (rc != 0) {
		errno = rc;
		اذهب failed;
	}
	𐡄𐡍 (!res) {
		errno = EINVAL;
		اذهب failed;
	}
	أرجع grp;
failed:
	حرر(grp);
	حرر(*grpbuf);
	أرجع عدم;
}

حرف *xgetlogin(فراغ)
{
	بنية passwd *pw = عدم;
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

	𐡄𐡍 (errno == 0)
		pw = getpwuid(ruid);
	𐡄𐡍 (pw && pw->pw_name && *pw->pw_name)
		أرجع xstrdup(pw->pw_name);

	أرجع عدم;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
بنية group *ul_getgrp_str(ثابت حرف *str, gid_t *result)
{
	صحيح rc;
	uint64_t num = 0;
	بنية group *gr;

	𐡄𐡍 (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	𐡄𐡍 (rc == -ERANGE || num > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		أرجع عدم;
	}

	/* maybe an actual name */
	𐡄𐡍 (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		𐡄𐡍 (gr && result)
			*result = gr->gr_gid;
		وإلا 𐡄𐡍 (!gr && !errno)
			errno = EINVAL;
		أرجع gr;
	}

	𐡄𐡍 (result)
		*result = (gid_t) num;

	errno = 0;
	gr = getgrgid((gid_t) num);
	𐡄𐡍 (!gr && !errno)
		errno = EINVAL;

	أرجع gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
بنية passwd *ul_getuserpw_str(ثابت حرف *str, uid_t *result)
{
	صحيح rc;
	uint64_t num = 0;
	بنية passwd *pw;

	𐡄𐡍 (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &num, 10);
	𐡄𐡍 (rc == -ERANGE || num > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		أرجع عدم;
	}

	/* maybe an actual name */
	𐡄𐡍 (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		𐡄𐡍 (pw && result)
			*result = pw->pw_uid;
		وإلا 𐡄𐡍 (!pw && !errno)
			errno = EINVAL;
		أرجع pw;
	}

	𐡄𐡍 (result)
		*result = (uid_t) num;

	errno = 0;
	pw = getpwuid((uid_t) num);
	𐡄𐡍 (!pw && !errno)
		errno = EINVAL;

	أرجع pw;
}

#ifdef TEST_PROGRAM
صحيح main(صحيح argc, حرف *argv[])
{
	حرف *buf = عدم;
	بنية passwd *pwd = عدم;

	𐡄𐡍 (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		أرجع EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	𐡄𐡍 (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	حرر(pwd);
	حرر(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	حرر(buf);

	أرجع EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
