/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "pwdutils.h"
#அந்தர்பாவ "xalloc.h"
#அந்தர்பாவ "strutils.h"

கட்டமைப்பு passwd *xgetpwuid(uid_t uid, எழுத்து **pwdbuf)
{
	கட்டமைப்பு passwd *pwd = வெற்று, *res = வெற்று;
	முழு rc;

	ப்ரதிஜ்ஞா(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, அளவு(கட்டமைப்பு passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	எனில் (rc != 0) {
		errno = rc;
		செல் failed;
	}
	எனில் (!res) {
		errno = EINVAL;
		செல் failed;
	}
	திரும்பு pwd;
failed:
	விடுவி(pwd);
	விடுவி(*pwdbuf);
	திரும்பு வெற்று;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
கட்டமைப்பு passwd *xgetuserpw(மாறிலி எழுத்து *str ,எழுத்து **pwdbuf)
{
	கட்டமைப்பு passwd *pwd = வெற்று, *res = வெற்று;
	முழு rc;
	uint64_t uid;

	ப்ரதிஜ்ஞா(pwdbuf);
	ப்ரதிஜ்ஞா(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, அளவு(கட்டமைப்பு passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	எனில் (rc == -ERANGE) {
		errno = ERANGE;
		செல் failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	எனில் (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} இல்லையெனில் {
		எனில் (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			செல் failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	எனில் (rc != 0) {
		errno = rc;
		செல் failed;
	}
	எனில் (!res) {
		errno = EINVAL;
		செல் failed;
	}
	திரும்பு pwd;
failed:
	விடுவி(pwd);
	விடுவி(*pwdbuf);
	திரும்பு வெற்று;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
கட்டமைப்பு group *xgetgroup(மாறிலி எழுத்து *str, எழுத்து **grpbuf)
{
	கட்டமைப்பு group *grp = வெற்று, *res = வெற்று;
	முழு rc;
	uint64_t gid;

	ப்ரதிஜ்ஞா(grpbuf);
	ப்ரதிஜ்ஞா(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, அளவு(கட்டமைப்பு group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	எனில் (rc == -ERANGE) {
		errno = ERANGE;
		செல் failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	எனில் (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} இல்லையெனில் {
		எனில் (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			செல் failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	எனில் (rc != 0) {
		errno = rc;
		செல் failed;
	}
	எனில் (!res) {
		errno = EINVAL;
		செல் failed;
	}
	திரும்பு grp;
failed:
	விடுவி(grp);
	விடுவி(*grpbuf);
	திரும்பு வெற்று;
}

எழுத்து *xgetlogin(வெறுமை)
{
	கட்டமைப்பு passwd *pw = வெற்று;
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

	எனில் (errno == 0)
		pw = getpwuid(ruid);
	எனில் (pw && pw->pw_name && *pw->pw_name)
		திரும்பு xstrdup(pw->pw_name);

	திரும்பு வெற்று;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
கட்டமைப்பு group *ul_getgrp_str(மாறிலி எழுத்து *str, gid_t *result)
{
	முழு rc;
	uint64_t இலக்கம் = 0;
	கட்டமைப்பு group *gr;

	எனில் (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &இலக்கம், 10);
	எனில் (rc == -ERANGE || இலக்கம் > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		திரும்பு வெற்று;
	}

	/* maybe an actual name */
	எனில் (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		எனில் (gr && result)
			*result = gr->gr_gid;
		இல்லையெனில் எனில் (!gr && !errno)
			errno = EINVAL;
		திரும்பு gr;
	}

	எனில் (result)
		*result = (gid_t) இலக்கம்;

	errno = 0;
	gr = getgrgid((gid_t) இலக்கம்);
	எனில் (!gr && !errno)
		errno = EINVAL;

	திரும்பு gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
கட்டமைப்பு passwd *ul_getuserpw_str(மாறிலி எழுத்து *str, uid_t *result)
{
	முழு rc;
	uint64_t இலக்கம் = 0;
	கட்டமைப்பு passwd *pw;

	எனில் (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &இலக்கம், 10);
	எனில் (rc == -ERANGE || இலக்கம் > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		திரும்பு வெற்று;
	}

	/* maybe an actual name */
	எனில் (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		எனில் (pw && result)
			*result = pw->pw_uid;
		இல்லையெனில் எனில் (!pw && !errno)
			errno = EINVAL;
		திரும்பு pw;
	}

	எனில் (result)
		*result = (uid_t) இலக்கம்;

	errno = 0;
	pw = getpwuid((uid_t) இலக்கம்);
	எனில் (!pw && !errno)
		errno = EINVAL;

	திரும்பு pw;
}

#ifdef TEST_PROGRAM
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	எழுத்து *buf = வெற்று;
	கட்டமைப்பு passwd *pwd = வெற்று;

	எனில் (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		திரும்பு EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	எனில் (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	விடுவி(pwd);
	விடுவி(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	விடுவி(buf);

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
