/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#شامل <stdlib.h>
#شامل <assert.h>
#شامل <unistd.h>

#شامل "c.h"
#شامل "pwdutils.h"
#شامل "xalloc.h"
#شامل "strutils.h"

ساختار passwd *xgetpwuid(uid_t uid, نویسه **pwdbuf)
{
	ساختار passwd *pwd = پوچ, *res = پوچ;
	صحیح rc;

	أكد(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, اندازه(ساختار passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	اگر (rc != 0) {
		errno = rc;
		برو failed;
	}
	اگر (!res) {
		errno = EINVAL;
		برو failed;
	}
	بازگشت pwd;
failed:
	آزاد(pwd);
	آزاد(*pwdbuf);
	بازگشت پوچ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ساختار passwd *xgetuserpw(ثابت نویسه *str ,نویسه **pwdbuf)
{
	ساختار passwd *pwd = پوچ, *res = پوچ;
	صحیح rc;
	uint64_t uid;

	أكد(pwdbuf);
	أكد(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, اندازه(ساختار passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	اگر (rc == -ERANGE) {
		errno = ERANGE;
		برو failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	اگر (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} وگرنه {
		اگر (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			برو failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	اگر (rc != 0) {
		errno = rc;
		برو failed;
	}
	اگر (!res) {
		errno = EINVAL;
		برو failed;
	}
	بازگشت pwd;
failed:
	آزاد(pwd);
	آزاد(*pwdbuf);
	بازگشت پوچ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ساختار group *xgetgroup(ثابت نویسه *str, نویسه **grpbuf)
{
	ساختار group *grp = پوچ, *res = پوچ;
	صحیح rc;
	uint64_t gid;

	أكد(grpbuf);
	أكد(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, اندازه(ساختار group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	اگر (rc == -ERANGE) {
		errno = ERANGE;
		برو failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	اگر (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} وگرنه {
		اگر (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			برو failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	اگر (rc != 0) {
		errno = rc;
		برو failed;
	}
	اگر (!res) {
		errno = EINVAL;
		برو failed;
	}
	بازگشت grp;
failed:
	آزاد(grp);
	آزاد(*grpbuf);
	بازگشت پوچ;
}

نویسه *xgetlogin(تهی)
{
	ساختار passwd *pw = پوچ;
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

	اگر (errno == 0)
		pw = getpwuid(ruid);
	اگر (pw && pw->pw_name && *pw->pw_name)
		بازگشت xstrdup(pw->pw_name);

	بازگشت پوچ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ساختار group *ul_getgrp_str(ثابت نویسه *str, gid_t *result)
{
	صحیح rc;
	uint64_t رقم = 0;
	ساختار group *gr;

	اگر (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &رقم, 10);
	اگر (rc == -ERANGE || رقم > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		بازگشت پوچ;
	}

	/* maybe an actual name */
	اگر (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		اگر (gr && result)
			*result = gr->gr_gid;
		وگرنه اگر (!gr && !errno)
			errno = EINVAL;
		بازگشت gr;
	}

	اگر (result)
		*result = (gid_t) رقم;

	errno = 0;
	gr = getgrgid((gid_t) رقم);
	اگر (!gr && !errno)
		errno = EINVAL;

	بازگشت gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ساختار passwd *ul_getuserpw_str(ثابت نویسه *str, uid_t *result)
{
	صحیح rc;
	uint64_t رقم = 0;
	ساختار passwd *pw;

	اگر (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &رقم, 10);
	اگر (rc == -ERANGE || رقم > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		بازگشت پوچ;
	}

	/* maybe an actual name */
	اگر (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		اگر (pw && result)
			*result = pw->pw_uid;
		وگرنه اگر (!pw && !errno)
			errno = EINVAL;
		بازگشت pw;
	}

	اگر (result)
		*result = (uid_t) رقم;

	errno = 0;
	pw = getpwuid((uid_t) رقم);
	اگر (!pw && !errno)
		errno = EINVAL;

	بازگشت pw;
}

#ifdef TEST_PROGRAM
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	نویسه *buf = پوچ;
	ساختار passwd *pwd = پوچ;

	اگر (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		بازگشت EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	اگر (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	آزاد(pwd);
	آزاد(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	آزاد(buf);

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
