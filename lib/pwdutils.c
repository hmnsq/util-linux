/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "pwdutils.h"
#അന്തര്ഭാവ "xalloc.h"
#അന്തര്ഭാവ "strutils.h"

ഘടന passwd *xgetpwuid(uid_t uid, അക്ഷരം **pwdbuf)
{
	ഘടന passwd *pwd = ശൂന്യ, *res = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;

	പ്രതിജ്ഞാ(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, വലുപ്പം(ഘടന passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	എങ്കിൽ (rc != 0) {
		errno = rc;
		പോകുക failed;
	}
	എങ്കിൽ (!res) {
		errno = EINVAL;
		പോകുക failed;
	}
	തിരികെ pwd;
failed:
	സ്വതന്ത്രം(pwd);
	സ്വതന്ത്രം(*pwdbuf);
	തിരികെ ശൂന്യ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ഘടന passwd *xgetuserpw(സ്ഥിരം അക്ഷരം *str ,അക്ഷരം **pwdbuf)
{
	ഘടന passwd *pwd = ശൂന്യ, *res = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;
	uint64_t uid;

	പ്രതിജ്ഞാ(pwdbuf);
	പ്രതിജ്ഞാ(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, വലുപ്പം(ഘടന passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	എങ്കിൽ (rc == -ERANGE) {
		errno = ERANGE;
		പോകുക failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	എങ്കിൽ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (uid > MAX_OF_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			പോകുക failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	എങ്കിൽ (rc != 0) {
		errno = rc;
		പോകുക failed;
	}
	എങ്കിൽ (!res) {
		errno = EINVAL;
		പോകുക failed;
	}
	തിരികെ pwd;
failed:
	സ്വതന്ത്രം(pwd);
	സ്വതന്ത്രം(*pwdbuf);
	തിരികെ ശൂന്യ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ഘടന group *xgetgroup(സ്ഥിരം അക്ഷരം *str, അക്ഷരം **grpbuf)
{
	ഘടന group *grp = ശൂന്യ, *res = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;
	uint64_t gid;

	പ്രതിജ്ഞാ(grpbuf);
	പ്രതിജ്ഞാ(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, വലുപ്പം(ഘടന group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	എങ്കിൽ (rc == -ERANGE) {
		errno = ERANGE;
		പോകുക failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	എങ്കിൽ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (gid > MAX_OF_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			പോകുക failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	എങ്കിൽ (rc != 0) {
		errno = rc;
		പോകുക failed;
	}
	എങ്കിൽ (!res) {
		errno = EINVAL;
		പോകുക failed;
	}
	തിരികെ grp;
failed:
	സ്വതന്ത്രം(grp);
	സ്വതന്ത്രം(*grpbuf);
	തിരികെ ശൂന്യ;
}

അക്ഷരം *xgetlogin(ശൂന്യം)
{
	ഘടന passwd *pw = ശൂന്യ;
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

	എങ്കിൽ (errno == 0)
		pw = getpwuid(ruid);
	എങ്കിൽ (pw && pw->pw_name && *pw->pw_name)
		തിരികെ xstrdup(pw->pw_name);

	തിരികെ ശൂന്യ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ഘടന group *ul_getgrp_str(സ്ഥിരം അക്ഷരം *str, gid_t *result)
{
	പൂർണ്ണസംഖ്യ rc;
	uint64_t അക്കം = 0;
	ഘടന group *gr;

	എങ്കിൽ (result)
		*result = (gid_t) -1;

	rc = ul_strtou64(str, &അക്കം, 10);
	എങ്കിൽ (rc == -ERANGE || അക്കം > MAX_OF_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		തിരികെ ശൂന്യ;
	}

	/* maybe an actual name */
	എങ്കിൽ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		എങ്കിൽ (gr && result)
			*result = gr->gr_gid;
		അല്ലെങ്കിൽ എങ്കിൽ (!gr && !errno)
			errno = EINVAL;
		തിരികെ gr;
	}

	എങ്കിൽ (result)
		*result = (gid_t) അക്കം;

	errno = 0;
	gr = getgrgid((gid_t) അക്കം);
	എങ്കിൽ (!gr && !errno)
		errno = EINVAL;

	തിരികെ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ഘടന passwd *ul_getuserpw_str(സ്ഥിരം അക്ഷരം *str, uid_t *result)
{
	പൂർണ്ണസംഖ്യ rc;
	uint64_t അക്കം = 0;
	ഘടന passwd *pw;

	എങ്കിൽ (result)
		*result = (uid_t) -1;

	rc = ul_strtou64(str, &അക്കം, 10);
	എങ്കിൽ (rc == -ERANGE || അക്കം > MAX_OF_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		തിരികെ ശൂന്യ;
	}

	/* maybe an actual name */
	എങ്കിൽ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		എങ്കിൽ (pw && result)
			*result = pw->pw_uid;
		അല്ലെങ്കിൽ എങ്കിൽ (!pw && !errno)
			errno = EINVAL;
		തിരികെ pw;
	}

	എങ്കിൽ (result)
		*result = (uid_t) അക്കം;

	errno = 0;
	pw = getpwuid((uid_t) അക്കം);
	എങ്കിൽ (!pw && !errno)
		errno = EINVAL;

	തിരികെ pw;
}

#ifdef TEST_PROGRAM
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	അക്ഷരം *buf = ശൂന്യ;
	ഘടന passwd *pwd = ശൂന്യ;

	എങ്കിൽ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		തിരികെ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	എങ്കിൽ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	printf("Username: %s\n", pwd->pw_name);
	printf("UID:      %u\n", pwd->pw_uid);
	printf("HOME:     %s\n", pwd->pw_dir);
	printf("GECO:     %s\n", pwd->pw_gecos);

	സ്വതന്ത്രം(pwd);
	സ്വതന്ത്രം(buf);

	printf("Current:  %s\n", (buf = xgetlogin()));
	സ്വതന്ത്രം(buf);

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM */
