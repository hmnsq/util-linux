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

ರಚನೆ passwd *xgetpwuid(uid_t uid, ಅಕ್ಷರ **pwdbuf)
{
	ರಚನೆ passwd *pwd = ಖಾಲಿ, *res = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;

	ಪ್ರತಿಜ್ಞಾ(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ಗಾತ್ರ(ರಚನೆ passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ಆದರೆ (rc != 0) {
		errno = rc;
		ಹೋಗು failed;
	}
	ಆದರೆ (!res) {
		errno = EINVAL;
		ಹೋಗು failed;
	}
	ಹಿಂತಿರುಗಿ pwd;
failed:
	ಬಿಡುಗಡೆ(pwd);
	ಬಿಡುಗಡೆ(*pwdbuf);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
ರಚನೆ passwd *xgetuserpw(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str ,ಅಕ್ಷರ **pwdbuf)
{
	ರಚನೆ passwd *pwd = ಖಾಲಿ, *res = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;
	uint64_t uid;

	ಪ್ರತಿಜ್ಞಾ(pwdbuf);
	ಪ್ರತಿಜ್ಞಾ(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, ಗಾತ್ರ(ರಚನೆ passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ಆದರೆ (rc == -ERANGE) {
		errno = ERANGE;
		ಹೋಗು failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ಆದರೆ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (uid > MAX_ದ_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			ಹೋಗು failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ಆದರೆ (rc != 0) {
		errno = rc;
		ಹೋಗು failed;
	}
	ಆದರೆ (!res) {
		errno = EINVAL;
		ಹೋಗು failed;
	}
	ಹಿಂತಿರುಗಿ pwd;
failed:
	ಬಿಡುಗಡೆ(pwd);
	ಬಿಡುಗಡೆ(*pwdbuf);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
ರಚನೆ group *xgetgroup(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಅಕ್ಷರ **grpbuf)
{
	ರಚನೆ group *grp = ಖಾಲಿ, *res = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;
	uint64_t gid;

	ಪ್ರತಿಜ್ಞಾ(grpbuf);
	ಪ್ರತಿಜ್ಞಾ(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, ಗಾತ್ರ(ರಚನೆ group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ಆದರೆ (rc == -ERANGE) {
		errno = ERANGE;
		ಹೋಗು failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ಆದರೆ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (gid > MAX_ದ_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			ಹೋಗು failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ಆದರೆ (rc != 0) {
		errno = rc;
		ಹೋಗು failed;
	}
	ಆದರೆ (!res) {
		errno = EINVAL;
		ಹೋಗು failed;
	}
	ಹಿಂತಿರುಗಿ grp;
failed:
	ಬಿಡುಗಡೆ(grp);
	ಬಿಡುಗಡೆ(*grpbuf);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಅಕ್ಷರ *xgetlogin(ಶೂನ್ಯ)
{
	ರಚನೆ passwd *pw = ಖಾಲಿ;
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

	ಆದರೆ (errno == 0)
		pw = getpwuid(ruid);
	ಆದರೆ (pw && pw->pw_name && *pw->pw_name)
		ಹಿಂತಿರುಗಿ xstrdup(pw->pw_name);

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ರಚನೆ group *ul_getgrp_str(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, gid_t *ಫಲಿತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ rc;
	uint64_t ಅಂಕೆ = 0;
	ರಚನೆ group *gr;

	ಆದರೆ (ಫಲಿತಾಂಶ)
		*ಫಲಿತಾಂಶ = (gid_t) -1;

	rc = ul_strtou64(str, &ಅಂಕೆ, 10);
	ಆದರೆ (rc == -ERANGE || ಅಂಕೆ > MAX_ದ_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* maybe an actual name */
	ಆದರೆ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ಆದರೆ (gr && ಫಲಿತಾಂಶ)
			*ಫಲಿತಾಂಶ = gr->gr_gid;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!gr && !errno)
			errno = EINVAL;
		ಹಿಂತಿರುಗಿ gr;
	}

	ಆದರೆ (ಫಲಿತಾಂಶ)
		*ಫಲಿತಾಂಶ = (gid_t) ಅಂಕೆ;

	errno = 0;
	gr = getgrgid((gid_t) ಅಂಕೆ);
	ಆದರೆ (!gr && !errno)
		errno = EINVAL;

	ಹಿಂತಿರುಗಿ gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
ರಚನೆ passwd *ul_getuserpw_str(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, uid_t *ಫಲಿತಾಂಶ)
{
	ಪೂರ್ಣಾಂಕ rc;
	uint64_t ಅಂಕೆ = 0;
	ರಚನೆ passwd *pw;

	ಆದರೆ (ಫಲಿತಾಂಶ)
		*ಫಲಿತಾಂಶ = (uid_t) -1;

	rc = ul_strtou64(str, &ಅಂಕೆ, 10);
	ಆದರೆ (rc == -ERANGE || ಅಂಕೆ > MAX_ದ_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	/* maybe an actual name */
	ಆದರೆ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ಆದರೆ (pw && ಫಲಿತಾಂಶ)
			*ಫಲಿತಾಂಶ = pw->pw_uid;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!pw && !errno)
			errno = EINVAL;
		ಹಿಂತಿರುಗಿ pw;
	}

	ಆದರೆ (ಫಲಿತಾಂಶ)
		*ಫಲಿತಾಂಶ = (uid_t) ಅಂಕೆ;

	errno = 0;
	pw = getpwuid((uid_t) ಅಂಕೆ);
	ಆದರೆ (!pw && !errno)
		errno = EINVAL;

	ಹಿಂತಿರುಗಿ pw;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಅಕ್ಷರ *buf = ಖಾಲಿ;
	ರಚನೆ passwd *pwd = ಖಾಲಿ;

	ಆದರೆ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ಆದರೆ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	ಬರೆ("Username: %s\n", pwd->pw_name);
	ಬರೆ("UID:      %u\n", pwd->pw_uid);
	ಬರೆ("HOME:     %s\n", pwd->pw_dir);
	ಬರೆ("GECO:     %s\n", pwd->pw_gecos);

	ಬಿಡುಗಡೆ(pwd);
	ಬಿಡುಗಡೆ(buf);

	ಬರೆ("Current:  %s\n", (buf = xgetlogin()));
	ಬಿಡುಗಡೆ(buf);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM */
