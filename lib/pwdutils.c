/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pwdutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "xalloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"

ਬਣਤਰ passwd *xgetpwuid(uid_t uid, ਅੱਖਰ **pwdbuf)
{
	ਬਣਤਰ passwd *pwd = ਨਿਰਾ, *res = ਨਿਰਾ;
	ਪੂਰਨ rc;

	ਪ੍ਰਤਿਜ੍ਞਾ(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ਆਕਾਰ(ਬਣਤਰ passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ਜੇ (rc != 0) {
		errno = rc;
		ਜਾਓ failed;
	}
	ਜੇ (!res) {
		errno = EINVAL;
		ਜਾਓ failed;
	}
	ਵਾਪਸ pwd;
failed:
	ਮੁਕਤ(pwd);
	ਮੁਕਤ(*pwdbuf);
	ਵਾਪਸ ਨਿਰਾ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ਬਣਤਰ passwd *xgetuserpw(ਅਚਲ ਅੱਖਰ *str ,ਅੱਖਰ **pwdbuf)
{
	ਬਣਤਰ passwd *pwd = ਨਿਰਾ, *res = ਨਿਰਾ;
	ਪੂਰਨ rc;
	uint64_t uid;

	ਪ੍ਰਤਿਜ੍ਞਾ(pwdbuf);
	ਪ੍ਰਤਿਜ੍ਞਾ(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ਆਕਾਰ(ਬਣਤਰ passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ਜੇ (rc == -ERANGE) {
		errno = ERANGE;
		ਜਾਓ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ਜੇ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (uid > MAX_ਦਾ_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			ਜਾਓ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ਜੇ (rc != 0) {
		errno = rc;
		ਜਾਓ failed;
	}
	ਜੇ (!res) {
		errno = EINVAL;
		ਜਾਓ failed;
	}
	ਵਾਪਸ pwd;
failed:
	ਮੁਕਤ(pwd);
	ਮੁਕਤ(*pwdbuf);
	ਵਾਪਸ ਨਿਰਾ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ਬਣਤਰ group *xgetgroup(ਅਚਲ ਅੱਖਰ *str, ਅੱਖਰ **grpbuf)
{
	ਬਣਤਰ group *grp = ਨਿਰਾ, *res = ਨਿਰਾ;
	ਪੂਰਨ rc;
	uint64_t gid;

	ਪ੍ਰਤਿਜ੍ਞਾ(grpbuf);
	ਪ੍ਰਤਿਜ੍ਞਾ(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, ਆਕਾਰ(ਬਣਤਰ group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ਜੇ (rc == -ERANGE) {
		errno = ERANGE;
		ਜਾਓ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ਜੇ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (gid > MAX_ਦਾ_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			ਜਾਓ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ਜੇ (rc != 0) {
		errno = rc;
		ਜਾਓ failed;
	}
	ਜੇ (!res) {
		errno = EINVAL;
		ਜਾਓ failed;
	}
	ਵਾਪਸ grp;
failed:
	ਮੁਕਤ(grp);
	ਮੁਕਤ(*grpbuf);
	ਵਾਪਸ ਨਿਰਾ;
}

ਅੱਖਰ *xgetlogin(ਖਾਲੀ)
{
	ਬਣਤਰ passwd *pw = ਨਿਰਾ;
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

	ਜੇ (errno == 0)
		pw = getpwuid(ruid);
	ਜੇ (pw && pw->pw_name && *pw->pw_name)
		ਵਾਪਸ xstrdup(pw->pw_name);

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ਬਣਤਰ group *ul_getgrp_str(ਅਚਲ ਅੱਖਰ *str, gid_t *ਨਤੀਜਾ)
{
	ਪੂਰਨ rc;
	uint64_t ਅੰਕ = 0;
	ਬਣਤਰ group *gr;

	ਜੇ (ਨਤੀਜਾ)
		*ਨਤੀਜਾ = (gid_t) -1;

	rc = ul_strtou64(str, &ਅੰਕ, 10);
	ਜੇ (rc == -ERANGE || ਅੰਕ > MAX_ਦਾ_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* maybe an actual name */
	ਜੇ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ਜੇ (gr && ਨਤੀਜਾ)
			*ਨਤੀਜਾ = gr->gr_gid;
		ਨਹੀਂਤਾਂ ਜੇ (!gr && !errno)
			errno = EINVAL;
		ਵਾਪਸ gr;
	}

	ਜੇ (ਨਤੀਜਾ)
		*ਨਤੀਜਾ = (gid_t) ਅੰਕ;

	errno = 0;
	gr = getgrgid((gid_t) ਅੰਕ);
	ਜੇ (!gr && !errno)
		errno = EINVAL;

	ਵਾਪਸ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ਬਣਤਰ passwd *ul_getuserpw_str(ਅਚਲ ਅੱਖਰ *str, uid_t *ਨਤੀਜਾ)
{
	ਪੂਰਨ rc;
	uint64_t ਅੰਕ = 0;
	ਬਣਤਰ passwd *pw;

	ਜੇ (ਨਤੀਜਾ)
		*ਨਤੀਜਾ = (uid_t) -1;

	rc = ul_strtou64(str, &ਅੰਕ, 10);
	ਜੇ (rc == -ERANGE || ਅੰਕ > MAX_ਦਾ_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ਵਾਪਸ ਨਿਰਾ;
	}

	/* maybe an actual name */
	ਜੇ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ਜੇ (pw && ਨਤੀਜਾ)
			*ਨਤੀਜਾ = pw->pw_uid;
		ਨਹੀਂਤਾਂ ਜੇ (!pw && !errno)
			errno = EINVAL;
		ਵਾਪਸ pw;
	}

	ਜੇ (ਨਤੀਜਾ)
		*ਨਤੀਜਾ = (uid_t) ਅੰਕ;

	errno = 0;
	pw = getpwuid((uid_t) ਅੰਕ);
	ਜੇ (!pw && !errno)
		errno = EINVAL;

	ਵਾਪਸ pw;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅੱਖਰ *buf = ਨਿਰਾ;
	ਬਣਤਰ passwd *pwd = ਨਿਰਾ;

	ਜੇ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ਵਾਪਸ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ਜੇ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	ਲਿਖੋ("Username: %s\n", pwd->pw_name);
	ਲਿਖੋ("UID:      %u\n", pwd->pw_uid);
	ਲਿਖੋ("HOME:     %s\n", pwd->pw_dir);
	ਲਿਖੋ("GECO:     %s\n", pwd->pw_gecos);

	ਮੁਕਤ(pwd);
	ਮੁਕਤ(buf);

	ਲਿਖੋ("Current:  %s\n", (buf = xgetlogin()));
	ਮੁਕਤ(buf);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM */
