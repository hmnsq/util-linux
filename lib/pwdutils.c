/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "pwdutils.h"
#અન્તર્ભાવ "xalloc.h"
#અન્તર્ભાવ "strutils.h"

સંરચના passwd *xgetpwuid(uid_t uid, અક્ષર **pwdbuf)
{
	સંરચના passwd *pwd = ખાલી, *res = ખાલી;
	પૂર્ણાંક rc;

	પ્રતિજ્ઞા(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, કદ(સંરચના passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	જો (rc != 0) {
		errno = rc;
		જાઓ failed;
	}
	જો (!res) {
		errno = EINVAL;
		જાઓ failed;
	}
	પાછા pwd;
failed:
	મુક્ત(pwd);
	મુક્ત(*pwdbuf);
	પાછા ખાલી;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
સંરચના passwd *xgetuserpw(અચલ અક્ષર *str ,અક્ષર **pwdbuf)
{
	સંરચના passwd *pwd = ખાલી, *res = ખાલી;
	પૂર્ણાંક rc;
	uint64_t uid;

	પ્રતિજ્ઞા(pwdbuf);
	પ્રતિજ્ઞા(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, કદ(સંરચના passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	જો (rc == -ERANGE) {
		errno = ERANGE;
		જાઓ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	જો (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} નહીંતર {
		જો (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			જાઓ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	જો (rc != 0) {
		errno = rc;
		જાઓ failed;
	}
	જો (!res) {
		errno = EINVAL;
		જાઓ failed;
	}
	પાછા pwd;
failed:
	મુક્ત(pwd);
	મુક્ત(*pwdbuf);
	પાછા ખાલી;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
સંરચના group *xgetgroup(અચલ અક્ષર *str, અક્ષર **grpbuf)
{
	સંરચના group *grp = ખાલી, *res = ખાલી;
	પૂર્ણાંક rc;
	uint64_t gid;

	પ્રતિજ્ઞા(grpbuf);
	પ્રતિજ્ઞા(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, કદ(સંરચના group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	જો (rc == -ERANGE) {
		errno = ERANGE;
		જાઓ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	જો (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} નહીંતર {
		જો (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			જાઓ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	જો (rc != 0) {
		errno = rc;
		જાઓ failed;
	}
	જો (!res) {
		errno = EINVAL;
		જાઓ failed;
	}
	પાછા grp;
failed:
	મુક્ત(grp);
	મુક્ત(*grpbuf);
	પાછા ખાલી;
}

અક્ષર *xgetlogin(શૂન્ય)
{
	સંરચના passwd *pw = ખાલી;
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

	જો (errno == 0)
		pw = getpwuid(ruid);
	જો (pw && pw->pw_name && *pw->pw_name)
		પાછા xstrdup(pw->pw_name);

	પાછા ખાલી;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
સંરચના group *ul_getgrp_str(અચલ અક્ષર *str, gid_t *result)
{
	પૂર્ણાંક rc;
	uint64_t અંક = 0;
	સંરચના group *gr;

	જો (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &અંક, 10);
	જો (rc == -ERANGE || અંક > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		પાછા ખાલી;
	}

	/* maybe an actual name */
	જો (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		જો (gr && result)
			*result = gr->gr_gid;
		નહીંતર જો (!gr && !errno)
			errno = EINVAL;
		પાછા gr;
	}

	જો (result)
		*result = (gid_t) અંક;

	errno = 0;
	gr = getgrgid((gid_t) અંક);
	જો (!gr && !errno)
		errno = EINVAL;

	પાછા gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
સંરચના passwd *ul_getuserpw_str(અચલ અક્ષર *str, uid_t *result)
{
	પૂર્ણાંક rc;
	uint64_t અંક = 0;
	સંરચના passwd *pw;

	જો (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &અંક, 10);
	જો (rc == -ERANGE || અંક > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		પાછા ખાલી;
	}

	/* maybe an actual name */
	જો (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		જો (pw && result)
			*result = pw->pw_uid;
		નહીંતર જો (!pw && !errno)
			errno = EINVAL;
		પાછા pw;
	}

	જો (result)
		*result = (uid_t) અંક;

	errno = 0;
	pw = getpwuid((uid_t) અંક);
	જો (!pw && !errno)
		errno = EINVAL;

	પાછા pw;
}

#ifdef TEST_PROGRAM
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અક્ષર *buf = ખાલી;
	સંરચના passwd *pwd = ખાલી;

	જો (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		પાછા EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	જો (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	મુક્ત(pwd);
	મુક્ત(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	મુક્ત(buf);

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
