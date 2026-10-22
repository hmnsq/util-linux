/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "pwdutils.h"
#ଅନ୍ତର୍ଭାଵ "xalloc.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"

ସଂରଚନା passwd *xgetpwuid(uid_t uid, ଅକ୍ଷର **pwdbuf)
{
	ସଂରଚନା passwd *pwd = ଖାଲି, *res = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ପ୍ରତିଜ୍ଞା(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ଆକାର(ସଂରଚନା passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ଯଦି (rc != 0) {
		errno = rc;
		ଯାଅ failed;
	}
	ଯଦି (!res) {
		errno = EINVAL;
		ଯାଅ failed;
	}
	ଫେରନ୍ତୁ pwd;
failed:
	ମୁକ୍ତ(pwd);
	ମୁକ୍ତ(*pwdbuf);
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ସଂରଚନା passwd *xgetuserpw(ଅଚଳ ଅକ୍ଷର *str ,ଅକ୍ଷର **pwdbuf)
{
	ସଂରଚନା passwd *pwd = ଖାଲି, *res = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	uint64_t uid;

	ପ୍ରତିଜ୍ଞା(pwdbuf);
	ପ୍ରତିଜ୍ଞା(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ଆକାର(ସଂରଚନା passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ଯଦି (rc == -ERANGE) {
		errno = ERANGE;
		ଯାଅ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ଯଦି (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} ଅନ୍ୟଥା {
		ଯଦି (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			ଯାଅ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ଯଦି (rc != 0) {
		errno = rc;
		ଯାଅ failed;
	}
	ଯଦି (!res) {
		errno = EINVAL;
		ଯାଅ failed;
	}
	ଫେରନ୍ତୁ pwd;
failed:
	ମୁକ୍ତ(pwd);
	ମୁକ୍ତ(*pwdbuf);
	ଫେରନ୍ତୁ ଖାଲି;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ସଂରଚନା group *xgetgroup(ଅଚଳ ଅକ୍ଷର *str, ଅକ୍ଷର **grpbuf)
{
	ସଂରଚନା group *grp = ଖାଲି, *res = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	uint64_t gid;

	ପ୍ରତିଜ୍ଞା(grpbuf);
	ପ୍ରତିଜ୍ଞା(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, ଆକାର(ସଂରଚନା group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ଯଦି (rc == -ERANGE) {
		errno = ERANGE;
		ଯାଅ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ଯଦି (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} ଅନ୍ୟଥା {
		ଯଦି (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			ଯାଅ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ଯଦି (rc != 0) {
		errno = rc;
		ଯାଅ failed;
	}
	ଯଦି (!res) {
		errno = EINVAL;
		ଯାଅ failed;
	}
	ଫେରନ୍ତୁ grp;
failed:
	ମୁକ୍ତ(grp);
	ମୁକ୍ତ(*grpbuf);
	ଫେରନ୍ତୁ ଖାଲି;
}

ଅକ୍ଷର *xgetlogin(ଶୂନ୍ୟ)
{
	ସଂରଚନା passwd *pw = ଖାଲି;
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

	ଯଦି (errno == 0)
		pw = getpwuid(ruid);
	ଯଦି (pw && pw->pw_name && *pw->pw_name)
		ଫେରନ୍ତୁ xstrdup(pw->pw_name);

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ସଂରଚନା group *ul_getgrp_str(ଅଚଳ ଅକ୍ଷର *str, gid_t *result)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	uint64_t ଅଙ୍କ = 0;
	ସଂରଚନା group *gr;

	ଯଦି (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &ଅଙ୍କ, 10);
	ଯଦି (rc == -ERANGE || ଅଙ୍କ > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* maybe an actual name */
	ଯଦି (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ଯଦି (gr && result)
			*result = gr->gr_gid;
		ଅନ୍ୟଥା ଯଦି (!gr && !errno)
			errno = EINVAL;
		ଫେରନ୍ତୁ gr;
	}

	ଯଦି (result)
		*result = (gid_t) ଅଙ୍କ;

	errno = 0;
	gr = getgrgid((gid_t) ଅଙ୍କ);
	ଯଦି (!gr && !errno)
		errno = EINVAL;

	ଫେରନ୍ତୁ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ସଂରଚନା passwd *ul_getuserpw_str(ଅଚଳ ଅକ୍ଷର *str, uid_t *result)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	uint64_t ଅଙ୍କ = 0;
	ସଂରଚନା passwd *pw;

	ଯଦି (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &ଅଙ୍କ, 10);
	ଯଦି (rc == -ERANGE || ଅଙ୍କ > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	/* maybe an actual name */
	ଯଦି (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ଯଦି (pw && result)
			*result = pw->pw_uid;
		ଅନ୍ୟଥା ଯଦି (!pw && !errno)
			errno = EINVAL;
		ଫେରନ୍ତୁ pw;
	}

	ଯଦି (result)
		*result = (uid_t) ଅଙ୍କ;

	errno = 0;
	pw = getpwuid((uid_t) ଅଙ୍କ);
	ଯଦି (!pw && !errno)
		errno = EINVAL;

	ଫେରନ୍ତୁ pw;
}

#ifdef TEST_PROGRAM
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅକ୍ଷର *buf = ଖାଲି;
	ସଂରଚନା passwd *pwd = ଖାଲି;

	ଯଦି (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ଯଦି (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	ମୁକ୍ତ(pwd);
	ମୁକ୍ତ(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	ମୁକ୍ତ(buf);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
