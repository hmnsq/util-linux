/*
 * No copyright is claimed. This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <stdlib.h>
#समावेश <assert.h>
#समावेश <unistd.h>

#समावेश "c.h"
#समावेश "pwdutils.h"
#समावेश "xalloc.h"
#समावेश "strutils.h"

काष्ठा passwd *xgetpwuid(uid_t uid, अक्षर **pwdbuf)
{
	काष्ठा passwd *pwd = शून्य, *res = शून्य;
	पूर्णांक rc;

	निश्चित(pwdbuf);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, माप(काष्ठा passwd));

	त्रुटि_सं = 0;
	rc = getpwuid_r(uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	अगर (rc != 0) {
		त्रुटि_सं = rc;
		जाओ failed;
	}
	अगर (!res) {
		त्रुटि_सं = EINVAL;
		जाओ failed;
	}
	वापस pwd;
failed:
	मुक्त(pwd);
	मुक्त(*pwdbuf);
	वापस शून्य;
}

/* Returns allocated passwd and allocated pwdbuf for the username or UID passed
 * as @str. In case of error returns NULL and sets errno, for unknown user it
 * sets errno to EINVAL.
 */
काष्ठा passwd *xgetuserpw(स्थिर अक्षर *str ,अक्षर **pwdbuf)
{
	काष्ठा passwd *pwd = शून्य, *res = शून्य;
	पूर्णांक rc;
	uint64_t uid;

	निश्चित(pwdbuf);
	निश्चित(str);

	*pwdbuf = xmalloc(UL_GETPW_BUFSIZ);
	pwd = xcalloc(1, माप(काष्ठा passwd));

	/* is @str a UID ? */
	rc = ul_strtou64(str, &uid, 10);
	अगर (rc == -दुस्फल) {
		त्रुटि_सं = दुस्फल;
		जाओ failed;
	}
	/* @str is an invalid number, let's assume it is the username */
	अगर (rc == -EINVAL) {
		rc = getpwnam_r(str, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	} अन्यथा {
		अगर (uid > MAX_OF_UINT_TYPE(uid_t)) {
			त्रुटि_सं = दुस्फल;
			जाओ failed;
		}
		rc = getpwuid_r((uid_t)uid, pwd, *pwdbuf, UL_GETPW_BUFSIZ, &res);
	}

	अगर (rc != 0) {
		त्रुटि_सं = rc;
		जाओ failed;
	}
	अगर (!res) {
		त्रुटि_सं = EINVAL;
		जाओ failed;
	}
	वापस pwd;
failed:
	मुक्त(pwd);
	मुक्त(*pwdbuf);
	वापस शून्य;
}

/* Returns allocated group and allocated grpbuf for the group name or GID passed
 * as @str. In case of error returns NULL and sets errno, for unknown group it
 * sets errno to EINVAL.
 */
काष्ठा group *xgetgroup(स्थिर अक्षर *str, अक्षर **grpbuf)
{
	काष्ठा group *grp = शून्य, *res = शून्य;
	पूर्णांक rc;
	uint64_t gid;

	निश्चित(grpbuf);
	निश्चित(str);

	*grpbuf = xmalloc(UL_GETPW_BUFSIZ);
	grp = xcalloc(1, माप(काष्ठा group));

	/* is @str a GID ? */
	rc = ul_strtou64(str, &gid, 10);
	अगर (rc == -दुस्फल) {
		त्रुटि_सं = दुस्फल;
		जाओ failed;
	}
	/* @str is an invalid number, let's assume it is the group name */
	अगर (rc == -EINVAL) {
		rc = getgrnam_r(str, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	} अन्यथा {
		अगर (gid > MAX_OF_UINT_TYPE(gid_t)) {
			त्रुटि_सं = दुस्फल;
			जाओ failed;
		}
		rc = getgrgid_r((gid_t)gid, grp, *grpbuf, UL_GETPW_BUFSIZ, &res);
	}

	अगर (rc != 0) {
		त्रुटि_सं = rc;
		जाओ failed;
	}
	अगर (!res) {
		त्रुटि_सं = EINVAL;
		जाओ failed;
	}
	वापस grp;
failed:
	मुक्त(grp);
	मुक्त(*grpbuf);
	वापस शून्य;
}

अक्षर *xgetlogin(व्योम)
{
	काष्ठा passwd *pw = शून्य;
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
	त्रुटि_सं = 0;
	ruid = getuid();

	अगर (त्रुटि_सं == 0)
		pw = getpwuid(ruid);
	अगर (pw && pw->pw_name && *pw->pw_name)
		वापस xstrdup(pw->pw_name);

	वापस शून्य;
}

/*
 * Return a pointer to a `struct group` for a matching group name or GID.
 *
 * If @result is not NULL, it will always be set to the parsed GID on success
 * or (gid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
काष्ठा group *ul_getgrp_str(स्थिर अक्षर *str, gid_t *परिणाम)
{
	पूर्णांक rc;
	uint64_t अंक = 0;
	काष्ठा group *gr;

	अगर (परिणाम)
		*परिणाम = (gid_t) -1;

	rc = ul_strtou64(str, &अंक, 10);
	अगर (rc == -दुस्फल || अंक > MAX_OF_UINT_TYPE(gid_t)) {
		त्रुटि_सं = दुस्फल;
		वापस शून्य;
	}

	/* maybe an actual name */
	अगर (rc == -EINVAL) {
		त्रुटि_सं = 0;
		gr = getgrnam(str);
		अगर (gr && परिणाम)
			*परिणाम = gr->gr_gid;
		अन्यथा अगर (!gr && !त्रुटि_सं)
			त्रुटि_सं = EINVAL;
		वापस gr;
	}

	अगर (परिणाम)
		*परिणाम = (gid_t) अंक;

	त्रुटि_सं = 0;
	gr = getgrgid((gid_t) अंक);
	अगर (!gr && !त्रुटि_सं)
		त्रुटि_सं = EINVAL;

	वापस gr;
}

/*
 * Return a pointer to a `struct passwd` for a matching username or UID.
 *
 * If @result is not NULL, it will always be set to the parsed UID on success
 * or (uid_t) -1 on failure.
 *
 * On failure NULL is returned and errno is set to indicate the error.
 */
काष्ठा passwd *ul_getuserpw_str(स्थिर अक्षर *str, uid_t *परिणाम)
{
	पूर्णांक rc;
	uint64_t अंक = 0;
	काष्ठा passwd *pw;

	अगर (परिणाम)
		*परिणाम = (uid_t) -1;

	rc = ul_strtou64(str, &अंक, 10);
	अगर (rc == -दुस्फल || अंक > MAX_OF_UINT_TYPE(uid_t)) {
		त्रुटि_सं = दुस्फल;
		वापस शून्य;
	}

	/* maybe an actual name */
	अगर (rc == -EINVAL) {
		त्रुटि_सं = 0;
		pw = getpwnam(str);
		अगर (pw && परिणाम)
			*परिणाम = pw->pw_uid;
		अन्यथा अगर (!pw && !त्रुटि_सं)
			त्रुटि_सं = EINVAL;
		वापस pw;
	}

	अगर (परिणाम)
		*परिणाम = (uid_t) अंक;

	त्रुटि_सं = 0;
	pw = getpwuid((uid_t) अंक);
	अगर (!pw && !त्रुटि_सं)
		त्रुटि_सं = EINVAL;

	वापस pw;
}

#अगर_घोषित TEST_PROGRAM
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *buf = शून्य;
	काष्ठा passwd *pwd = शून्य;

	अगर (argc != 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s <username>\n", argv[0]);
		वापस निकास_त्रुटि;
	}

	pwd = xgetuserpw(argv[1], &buf);
	अगर (!pwd)
		err(निकास_त्रुटि, "failed to get %s pwd entry", argv[1]);

	म_लिखो("Username: %s\n", pwd->pw_name);
	म_लिखो("UID:      %u\n", pwd->pw_uid);
	म_लिखो("HOME:     %s\n", pwd->pw_dir);
	म_लिखो("GECO:     %s\n", pwd->pw_gecos);

	मुक्त(pwd);
	मुक्त(buf);

	म_लिखो("Current:  %s\n", (buf = xgetlogin()));
	मुक्त(buf);

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM */
