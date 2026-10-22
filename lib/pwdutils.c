/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "pwdutils.h"
#అన్తర్భావ "xalloc.h"
#అన్తర్భావ "strutils.h"

నిర్మాణం passwd *xgetpwuid(uid_t uid, అక్షరం **pwdbuf)
{
	నిర్మాణం passwd *pwd = ఖాళీ, *res = ఖాళీ;
	పూర్ణాంకం rc;

	ప్రతిజ్ఞా(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, పరిమాణం(నిర్మాణం passwd));

	errno = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	ఒకవేళ (rc != 0) {
		errno = rc;
		వెళ్ళు failed;
	}
	ఒకవేళ (!res) {
		errno = EINVAL;
		వెళ్ళు failed;
	}
	తిరిగి pwd;
failed:
	విడుదల(pwd);
	విడుదల(*pwdbuf);
	తిరిగి ఖాళీ;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
నిర్మాణం passwd *xgetuserpw(స్థిరరాశి అక్షరం *str ,అక్షరం **pwdbuf)
{
	నిర్మాణం passwd *pwd = ఖాళీ, *res = ఖాళీ;
	పూర్ణాంకం rc;
	uint64_t uid;

	ప్రతిజ్ఞా(pwdbuf);
	ప్రతిజ్ఞా(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, పరిమాణం(నిర్మాణం passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	ఒకవేళ (rc == -ERANGE) {
		errno = ERANGE;
		వెళ్ళు failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	ఒకవేళ (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} లేకపోతే {
		ఒకవేళ (uid > MAX_యొక్క_UINT_TYPE(uid_t)) {
			errno = ERANGE;
			వెళ్ళు failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	ఒకవేళ (rc != 0) {
		errno = rc;
		వెళ్ళు failed;
	}
	ఒకవేళ (!res) {
		errno = EINVAL;
		వెళ్ళు failed;
	}
	తిరిగి pwd;
failed:
	విడుదల(pwd);
	విడుదల(*pwdbuf);
	తిరిగి ఖాళీ;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
నిర్మాణం group *xgetgroup(స్థిరరాశి అక్షరం *str, అక్షరం **grpbuf)
{
	నిర్మాణం group *grp = ఖాళీ, *res = ఖాళీ;
	పూర్ణాంకం rc;
	uint64_t gid;

	ప్రతిజ్ఞా(grpbuf);
	ప్రతిజ్ఞా(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, పరిమాణం(నిర్మాణం group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	ఒకవేళ (rc == -ERANGE) {
		errno = ERANGE;
		వెళ్ళు failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	ఒకవేళ (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} లేకపోతే {
		ఒకవేళ (gid > MAX_యొక్క_UINT_TYPE(gid_t)) {
			errno = ERANGE;
			వెళ్ళు failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	ఒకవేళ (rc != 0) {
		errno = rc;
		వెళ్ళు failed;
	}
	ఒకవేళ (!res) {
		errno = EINVAL;
		వెళ్ళు failed;
	}
	తిరిగి grp;
failed:
	విడుదల(grp);
	విడుదల(*grpbuf);
	తిరిగి ఖాళీ;
}

అక్షరం *xgetlogin(శూన్య)
{
	నిర్మాణం passwd *pw = ఖాళీ;
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

	ఒకవేళ (errno == 0)
		pw = getpwuid(ruid);
	ఒకవేళ (pw && pw->pw_name && *pw->pw_name)
		తిరిగి xstrdup(pw->pw_name);

	తిరిగి ఖాళీ;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
నిర్మాణం group *ul_getgrp_str(స్థిరరాశి అక్షరం *str, gid_t *ఫలితం)
{
	పూర్ణాంకం rc;
	uint64_t అంకె = 0;
	నిర్మాణం group *gr;

	ఒకవేళ (ఫలితం)
		*ఫలితం = (gid_t) -1;

	rc = ul_strtou64(str, &అంకె, 10);
	ఒకవేళ (rc == -ERANGE || అంకె > MAX_యొక్క_UINT_TYPE(gid_t)) {
		errno = ERANGE;
		తిరిగి ఖాళీ;
	}

	/* maybe an actual name */
	ఒకవేళ (rc == -EINVAL) {
		errno = 0;
		gr = getgrnam(str);
		ఒకవేళ (gr && ఫలితం)
			*ఫలితం = gr->gr_gid;
		లేకపోతే ఒకవేళ (!gr && !errno)
			errno = EINVAL;
		తిరిగి gr;
	}

	ఒకవేళ (ఫలితం)
		*ఫలితం = (gid_t) అంకె;

	errno = 0;
	gr = getgrgid((gid_t) అంకె);
	ఒకవేళ (!gr && !errno)
		errno = EINVAL;

	తిరిగి gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
నిర్మాణం passwd *ul_getuserpw_str(స్థిరరాశి అక్షరం *str, uid_t *ఫలితం)
{
	పూర్ణాంకం rc;
	uint64_t అంకె = 0;
	నిర్మాణం passwd *pw;

	ఒకవేళ (ఫలితం)
		*ఫలితం = (uid_t) -1;

	rc = ul_strtou64(str, &అంకె, 10);
	ఒకవేళ (rc == -ERANGE || అంకె > MAX_యొక్క_UINT_TYPE(uid_t)) {
		errno = ERANGE;
		తిరిగి ఖాళీ;
	}

	/* maybe an actual name */
	ఒకవేళ (rc == -EINVAL) {
		errno = 0;
		pw = getpwnam(str);
		ఒకవేళ (pw && ఫలితం)
			*ఫలితం = pw->pw_uid;
		లేకపోతే ఒకవేళ (!pw && !errno)
			errno = EINVAL;
		తిరిగి pw;
	}

	ఒకవేళ (ఫలితం)
		*ఫలితం = (uid_t) అంకె;

	errno = 0;
	pw = getpwuid((uid_t) అంకె);
	ఒకవేళ (!pw && !errno)
		errno = EINVAL;

	తిరిగి pw;
}

#నిర్వచితమైతే TEST_కార్యక్రమం
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	అక్షరం *buf = ఖాళీ;
	నిర్మాణం passwd *pwd = ఖాళీ;

	ఒకవేళ (argc != 2) {
		fprintf(stderr, "usage: %s <username>\n", argv[0]);
		తిరిగి EXIT_FAILURE;
	}

	pwd = xgetuserpw(argv[1], &buf);
	ఒకవేళ (!pwd)
		err(EXIT_FAILURE, "failed to get %s pwd entry", argv[1]);

	రాయి("Username: %s\n", pwd->pw_name);
	రాయి("UID:      %u\n", pwd->pw_uid);
	రాయి("HOME:     %s\n", pwd->pw_dir);
	రాయి("GECO:     %s\n", pwd->pw_gecos);

	విడుదల(pwd);
	విడుదల(buf);

	రాయి("Current:  %s\n", (buf = xgetlogin()));
	విడుదల(buf);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM */
