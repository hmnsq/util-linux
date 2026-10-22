/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "pwdutils.h"
#অন্তর্ভাব "xalloc.h"
#অন্তর্ভাব "strutils.h"

কাঠামো passwd *xgetpwuid(uid_t uid, অক্ষর **pwdbuf)
{
	কাঠামো passwd *pwd = ফাঁকা, *res = ফাঁকা;
	পূর্ণসংখ্যা rc;

	প্রতিজ্ঞা(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, আকার(কাঠামো passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	যদি (rc != 0) {
		errno = rc;
		যাও failed;
	}
	যদি (!res) {
		errno = EINVAL;
		যাও failed;
	}
	ফেরত pwd;
failed:
	মুক্ত(pwd);
	মুক্ত(*pwdbuf);
	ফেরত ফাঁকা;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
কাঠামো passwd *xgetuserpw(ধ্রুবক অক্ষর *str ,অক্ষর **pwdbuf)
{
	কাঠামো passwd *pwd = ফাঁকা, *res = ফাঁকা;
	পূর্ণসংখ্যা rc;
	uint64_t uid;

	প্রতিজ্ঞা(pwdbuf);
	প্রতিজ্ঞা(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, আকার(কাঠামো passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	যদি (rc == -ERANGE) {
		errno = ERANGE;
		যাও failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	যদি (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} অন্যথা {
		যদি (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			যাও failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	যদি (rc != 0) {
		errno = rc;
		যাও failed;
	}
	যদি (!res) {
		errno = EINVAL;
		যাও failed;
	}
	ফেরত pwd;
failed:
	মুক্ত(pwd);
	মুক্ত(*pwdbuf);
	ফেরত ফাঁকা;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
কাঠামো group *xgetgroup(ধ্রুবক অক্ষর *str, অক্ষর **grpbuf)
{
	কাঠামো group *grp = ফাঁকা, *res = ফাঁকা;
	পূর্ণসংখ্যা rc;
	uint64_t gid;

	প্রতিজ্ঞা(grpbuf);
	প্রতিজ্ঞা(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, আকার(কাঠামো group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	যদি (rc == -ERANGE) {
		errno = ERANGE;
		যাও failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	যদি (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} অন্যথা {
		যদি (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			যাও failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	যদি (rc != 0) {
		errno = rc;
		যাও failed;
	}
	যদি (!res) {
		errno = EINVAL;
		যাও failed;
	}
	ফেরত grp;
failed:
	মুক্ত(grp);
	মুক্ত(*grpbuf);
	ফেরত ফাঁকা;
}

অক্ষর *xgetlogin(শূন্য)
{
	কাঠামো passwd *pw = ফাঁকা;
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

	যদি (errno == 0)
		pw = getpwuid(ruid);
	যদি (pw && pw->pw_name && *pw->pw_name)
		ফেরত xstrdup(pw->pw_name);

	ফেরত ফাঁকা;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
কাঠামো group *ul_getgrp_str(ধ্রুবক অক্ষর *str, gid_t *ফলাফল)
{
	পূর্ণসংখ্যা rc;
	uint64_t অঙ্ক = 0;
	কাঠামো group *gr;

	যদি (ফলাফল)
		*ফলাফল = (gid_t) -1;

	rc = ul_strtou64(str, &অঙ্ক, 10);
	যদি (rc == -ERANGE || অঙ্ক > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ফেরত ফাঁকা;
	}

	/* maybe an actual name */
	যদি (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		যদি (gr && ফলাফল)
			*ফলাফল = gr->gr_gid;
		অন্যথা যদি (!gr && !errno)
			errno = EINVAL;
		ফেরত gr;
	}

	যদি (ফলাফল)
		*ফলাফল = (gid_t) অঙ্ক;

	errno = 0;
	gr = getgrgid((gid_t) অঙ্ক);
	যদি (!gr && !errno)
		errno = EINVAL;

	ফেরত gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
কাঠামো passwd *ul_getuserpw_str(ধ্রুবক অক্ষর *str, uid_t *ফলাফল)
{
	পূর্ণসংখ্যা rc;
	uint64_t অঙ্ক = 0;
	কাঠামো passwd *pw;

	যদি (ফলাফল)
		*ফলাফল = (uid_t) -1;

	rc = ul_strtou64(str, &অঙ্ক, 10);
	যদি (rc == -ERANGE || অঙ্ক > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ফেরত ফাঁকা;
	}

	/* maybe an actual name */
	যদি (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		যদি (pw && ফলাফল)
			*ফলাফল = pw->pw_uid;
		অন্যথা যদি (!pw && !errno)
			errno = EINVAL;
		ফেরত pw;
	}

	যদি (ফলাফল)
		*ফলাফল = (uid_t) অঙ্ক;

	errno = 0;
	pw = getpwuid((uid_t) অঙ্ক);
	যদি (!pw && !errno)
		errno = EINVAL;

	ফেরত pw;
}

#ifdef TEST_PROGRAM
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	অক্ষর *buf = ফাঁকা;
	কাঠামো passwd *pwd = ফাঁকা;

	যদি (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ফেরত EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	যদি (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	মুক্ত(pwd);
	মুক্ত(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	মুক্ত(buf);

	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
