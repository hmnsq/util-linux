/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "pwdutils.h"
#ಅನ್ತರ್ಭಾವ "xalloc.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

ಸಂರಚನಾ passwd *xgetpwuid(uid_t uid, ವರ್ಣ **pwdbuf)
{
	ಸಂರಚನಾ passwd *pwd = ರಿಕ್ತ, *res = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಪ್ರತಿಜ್ಞಾ(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ಯದಿ (rc != 0) {
		errno = rc;
		ಗಚ್ಛ failed;
	}
	ಯದಿ (!res) {
		errno = EINVAL;
		ಗಚ್ಛ failed;
	}
	ಪ್ರತ್ಯಾಗಮ pwd;
failed:
	ಮುಕ್ತ(pwd);
	ಮುಕ್ತ(*pwdbuf);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ಸಂರಚನಾ passwd *xgetuserpw(ಧ್ರುವ ವರ್ಣ *str ,ವರ್ಣ **pwdbuf)
{
	ಸಂರಚನಾ passwd *pwd = ರಿಕ್ತ, *res = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;
	uint64_t uid;

	ಪ್ರತಿಜ್ಞಾ(pwdbuf);
	ಪ್ರತಿಜ್ಞಾ(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ಯದಿ (rc == -ERANGE) {
		errno = ERANGE;
		ಗಚ್ಛ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ಯದಿ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} ಅನ್ಯಥಾ {
		ಯದಿ (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			ಗಚ್ಛ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ಯದಿ (rc != 0) {
		errno = rc;
		ಗಚ್ಛ failed;
	}
	ಯದಿ (!res) {
		errno = EINVAL;
		ಗಚ್ಛ failed;
	}
	ಪ್ರತ್ಯಾಗಮ pwd;
failed:
	ಮುಕ್ತ(pwd);
	ಮುಕ್ತ(*pwdbuf);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ಸಂರಚನಾ group *xgetgroup(ಧ್ರುವ ವರ್ಣ *str, ವರ್ಣ **grpbuf)
{
	ಸಂರಚನಾ group *grp = ರಿಕ್ತ, *res = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;
	uint64_t gid;

	ಪ್ರತಿಜ್ಞಾ(grpbuf);
	ಪ್ರತಿಜ್ಞಾ(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ಯದಿ (rc == -ERANGE) {
		errno = ERANGE;
		ಗಚ್ಛ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ಯದಿ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} ಅನ್ಯಥಾ {
		ಯದಿ (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			ಗಚ್ಛ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ಯದಿ (rc != 0) {
		errno = rc;
		ಗಚ್ಛ failed;
	}
	ಯದಿ (!res) {
		errno = EINVAL;
		ಗಚ್ಛ failed;
	}
	ಪ್ರತ್ಯಾಗಮ grp;
failed:
	ಮುಕ್ತ(grp);
	ಮುಕ್ತ(*grpbuf);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ವರ್ಣ *xgetlogin(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ passwd *pw = ರಿಕ್ತ;
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

	ಯದಿ (errno == 0)
		pw = getpwuid(ruid);
	ಯದಿ (pw && pw->pw_name && *pw->pw_name)
		ಪ್ರತ್ಯಾಗಮ xstrdup(pw->pw_name);

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ಸಂರಚನಾ group *ul_getgrp_str(ಧ್ರುವ ವರ್ಣ *str, gid_t *result)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	uint64_t ಅಂಕೆ = 0;
	ಸಂರಚನಾ group *gr;

	ಯದಿ (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &ಅಂಕೆ, 10);
	ಯದಿ (rc == -ERANGE || ಅಂಕೆ > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* maybe an actual name */
	ಯದಿ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ಯದಿ (gr && result)
			*result = gr->gr_gid;
		ಅನ್ಯಥಾ ಯದಿ (!gr && !errno)
			errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ gr;
	}

	ಯದಿ (result)
		*result = (gid_t) ಅಂಕೆ;

	errno = 0;
	gr = getgrgid((gid_t) ಅಂಕೆ);
	ಯದಿ (!gr && !errno)
		errno = EINVAL;

	ಪ್ರತ್ಯಾಗಮ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ಸಂರಚನಾ passwd *ul_getuserpw_str(ಧ್ರುವ ವರ್ಣ *str, uid_t *result)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	uint64_t ಅಂಕೆ = 0;
	ಸಂರಚನಾ passwd *pw;

	ಯದಿ (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &ಅಂಕೆ, 10);
	ಯದಿ (rc == -ERANGE || ಅಂಕೆ > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	/* maybe an actual name */
	ಯದಿ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ಯದಿ (pw && result)
			*result = pw->pw_uid;
		ಅನ್ಯಥಾ ಯದಿ (!pw && !errno)
			errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ pw;
	}

	ಯದಿ (result)
		*result = (uid_t) ಅಂಕೆ;

	errno = 0;
	pw = getpwuid((uid_t) ಅಂಕೆ);
	ಯದಿ (!pw && !errno)
		errno = EINVAL;

	ಪ್ರತ್ಯಾಗಮ pw;
}

#ifdef TEST_PROGRAM
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ವರ್ಣ *buf = ರಿಕ್ತ;
	ಸಂರಚನಾ passwd *pwd = ರಿಕ್ತ;

	ಯದಿ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ಯದಿ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	ಮುಕ್ತ(pwd);
	ಮುಕ್ತ(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	ಮುಕ್ತ(buf);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
