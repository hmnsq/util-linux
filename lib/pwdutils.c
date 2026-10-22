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

ساخت passwd *xgetpwuid(uid_t uid, حرف **pwdbuf)
{
	ساخت passwd *pwd = تہی, *res = تہی;
	صحیح rc;

	أكد(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, حجم(ساخت passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	اگر (rc != 0) {
		errno = rc;
		جاؤ failed;
	}
	اگر (!res) {
		errno = EINVAL;
		جاؤ failed;
	}
	واپس pwd;
failed:
	آزاد(pwd);
	آزاد(*pwdbuf);
	واپس تہی;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ساخت passwd *xgetuserpw(ثابت حرف *str ,حرف **pwdbuf)
{
	ساخت passwd *pwd = تہی, *res = تہی;
	صحیح rc;
	uint64_t uid;

	أكد(pwdbuf);
	أكد(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, حجم(ساخت passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	اگر (rc == -ERANGE) {
		errno = ERANGE;
		جاؤ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	اگر (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} ورنہ {
		اگر (uid > MAX_کا_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			جاؤ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	اگر (rc != 0) {
		errno = rc;
		جاؤ failed;
	}
	اگر (!res) {
		errno = EINVAL;
		جاؤ failed;
	}
	واپس pwd;
failed:
	آزاد(pwd);
	آزاد(*pwdbuf);
	واپس تہی;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ساخت group *xgetgroup(ثابت حرف *str, حرف **grpbuf)
{
	ساخت group *grp = تہی, *res = تہی;
	صحیح rc;
	uint64_t gid;

	أكد(grpbuf);
	أكد(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, حجم(ساخت group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	اگر (rc == -ERANGE) {
		errno = ERANGE;
		جاؤ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	اگر (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} ورنہ {
		اگر (gid > MAX_کا_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			جاؤ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	اگر (rc != 0) {
		errno = rc;
		جاؤ failed;
	}
	اگر (!res) {
		errno = EINVAL;
		جاؤ failed;
	}
	واپس grp;
failed:
	آزاد(grp);
	آزاد(*grpbuf);
	واپس تہی;
}

حرف *xgetlogin(خالی)
{
	ساخت passwd *pw = تہی;
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
		واپس xstrdup(pw->pw_name);

	واپس تہی;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ساخت group *ul_getgrp_str(ثابت حرف *str, gid_t *نتیجہ)
{
	صحیح rc;
	uint64_t عدد = 0;
	ساخت group *gr;

	اگر (نتیجہ)
		*نتیجہ = (gid_t) -1;

	rc = ul_strtou64(str, &عدد, 10);
	اگر (rc == -ERANGE || عدد > MAX_کا_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		واپس تہی;
	}

	/* maybe an actual name */
	اگر (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		اگر (gr && نتیجہ)
			*نتیجہ = gr->gr_gid;
		ورنہ اگر (!gr && !errno)
			errno = EINVAL;
		واپس gr;
	}

	اگر (نتیجہ)
		*نتیجہ = (gid_t) عدد;

	errno = 0;
	gr = getgrgid((gid_t) عدد);
	اگر (!gr && !errno)
		errno = EINVAL;

	واپس gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ساخت passwd *ul_getuserpw_str(ثابت حرف *str, uid_t *نتیجہ)
{
	صحیح rc;
	uint64_t عدد = 0;
	ساخت passwd *pw;

	اگر (نتیجہ)
		*نتیجہ = (uid_t) -1;

	rc = ul_strtou64(str, &عدد, 10);
	اگر (rc == -ERANGE || عدد > MAX_کا_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		واپس تہی;
	}

	/* maybe an actual name */
	اگر (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		اگر (pw && نتیجہ)
			*نتیجہ = pw->pw_uid;
		ورنہ اگر (!pw && !errno)
			errno = EINVAL;
		واپس pw;
	}

	اگر (نتیجہ)
		*نتیجہ = (uid_t) عدد;

	errno = 0;
	pw = getpwuid((uid_t) عدد);
	اگر (!pw && !errno)
		errno = EINVAL;

	واپس pw;
}

#اگر_تعریف_شدہ TEST_پروگرام
صحیح مین(صحیح argc, حرف *argv[])
{
	حرف *buf = تہی;
	ساخت passwd *pwd = تہی;

	اگر (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		واپس EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	اگر (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	لکھو("Username: %s\n", pwd->pw_name);
	لکھو("UID:      %u\n", pwd->pw_uid);
	لکھو("HOME:     %s\n", pwd->pw_dir);
	لکھو("GECO:     %s\n", pwd->pw_gecos);

	آزاد(pwd);
	آزاد(buf);

	لکھو("Current:  %s\n", (buf = xgetlogin()));
	آزاد(buf);

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM */
