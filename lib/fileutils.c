/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>
#समावेश <sys/time.h>
#समावेश <sys/resource.h>
#अगर_घोषित HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#पूर्ण_अगर
#समावेश <string.h>
#समावेश <sys/wait.h>
#समावेश <fcntl.h>
#समावेश <errno.h>

#समावेश "c.h"
#समावेश "all-io.h"
#समावेश "canonicalize.h"
#समावेश "fileutils.h"
#समावेश "pathnames.h"
#समावेश "strutils.h"

पूर्णांक mkstemp_cloexec(अक्षर *ढाँचा)
{
#अगर_घोषित HAVE_MKOSTEMP
	वापस mkostemp(ढाँचा, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#अन्यथा
	पूर्णांक fd, old_flags, errno_save;

	fd = mkstemp(ढाँचा);
	अगर (fd < 0)
		वापस fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	अगर (old_flags < 0)
		जाओ unwind;
	अगर (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		जाओ unwind;

	वापस fd;

unwind:
	errno_save = त्रुटि_सं;
	unlink(ढाँचा);
	बंद(fd);
	त्रुटि_सं = errno_save;

	वापस -1;
#पूर्ण_अगर
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
पूर्णांक xmkstemp(अक्षर **tmpname, स्थिर अक्षर *dir, स्थिर अक्षर *prefix)
{
	अक्षर *localtmp;
	स्थिर अक्षर *tmpenv;
	mode_t old_mode;
	पूर्णांक fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : दो_पर्या("TMPDIR");
	अगर (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	अगर (rc < 0)
		वापस -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	अगर (fd == -1) {
		मुक्त(localtmp);
		localtmp = शून्य;
	}
	*tmpname = localtmp;
	वापस fd;
}

#अगर_घोषित F_DUPFD_CLOEXEC
पूर्णांक dup_fd_cloexec(पूर्णांक oldfd, पूर्णांक lowfd)
#अन्यथा
पूर्णांक dup_fd_cloexec(पूर्णांक oldfd, पूर्णांक lowfd  __attribute__((__unused__)))
#पूर्ण_अगर
{
	पूर्णांक fd, flags, errno_save;

#अगर_घोषित F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	अगर (fd >= 0)
		वापस fd;
#पूर्ण_अगर

	fd = dup(oldfd);
	अगर (fd < 0)
		वापस fd;

	flags = fcntl(fd, F_GETFD);
	अगर (flags < 0)
		जाओ unwind;
	अगर (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		जाओ unwind;

	वापस fd;

unwind:
	errno_save = त्रुटि_सं;
	बंद(fd);
	त्रुटि_सं = errno_save;

	वापस -1;
}

/*
 * portable getdtablesize()
 */
अचिन्हित पूर्णांक get_fd_tabsize(व्योम)
{
	पूर्णांक m;

#अगर defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#या_अगर defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	काष्ठा rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#या_अगर defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#अन्यथा
	m = OPEN_MAX;
#पूर्ण_अगर
	वापस m;
}

व्योम ul_close_all_fds(अचिन्हित पूर्णांक first, अचिन्हित पूर्णांक last)
{
	काष्ठा dirent *d;
	सूची *dir;

	dir = सूची_खोलो(_PATH_PROC_FDDIR);
	अगर (dir) {
		जबतक ((d = xreaddir(dir))) {
			अक्षर *end;
			अचिन्हित पूर्णांक fd;
			पूर्णांक dfd;

			त्रुटि_सं = 0;
			fd = म_से_अदीर्घ(d->d_name, &end, 10);

			अगर (त्रुटि_सं || end == d->d_name || !end || *end)
				जारी;
			dfd = dirfd(dir);
			अगर (dfd < 0)
				जारी;
			अगर ((अचिन्हित पूर्णांक)dfd == fd)
				जारी;
			अगर (fd < first || last < fd)
				जारी;
			बंद(fd);
		}
		बंद_सूची(dir);
	} अन्यथा {
		अचिन्हित fd, tbsz = get_fd_tabsize();

		क्रम (fd = 0; fd < tbsz; fd++) {
			अगर (first <= fd && fd <= last)
				बंद(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
अक्षर *ul_restricted_path_oper(स्थिर अक्षर *path,
		  पूर्णांक (*oper)(स्थिर अक्षर *path, अक्षर **परिणाम, व्योम *तथ्य),
		  व्योम *तथ्य)
{
	अक्षर *परिणाम = शून्य;
	पूर्णांक errsv = 0;
	पूर्णांक pipes[2];
	ssize_t len;
	pid_t pid;

	अगर (!path || !*path)
		वापस शून्य;

	अगर (pipe(pipes) != 0)
		वापस शून्य;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = विभाजन();
	चयन (pid) {
	हाल -1:
		बंद(pipes[0]);
		बंद(pipes[1]);
		वापस शून्य;			/* fork error */
	हाल 0:
		बंद(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		त्रुटि_सं = 0;

		अगर (drop_permissions() != 0)
			परिणाम = शून्य;	/* failed */
		अन्यथा
			oper(path, &परिणाम, तथ्य);

		len = परिणाम ? (ssize_t) म_माप(परिणाम) :
		          त्रुटि_सं ? -त्रुटि_सं : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (अक्षर *) &len, माप(len));
		अगर (परिणाम)
			ul_write_all(pipes[1], परिणाम, len);
		_निकास(0);
	शेष:
		अवरोध;
	}

	बंद(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	अगर (ul_read_all(pipes[0], (अक्षर *) &len, माप(len)) != माप(len))
		जाओ done;
	अगर (len < 0) {
		errsv = -len;
		जाओ done;
	}

	परिणाम = दो_स्मृति(len + 1);
	अगर (!परिणाम) {
		errsv = ENOMEM;
		जाओ done;
	}
	/* read path */
	अगर (ul_read_all(pipes[0], परिणाम, len) != len) {
		errsv = त्रुटि_सं;
		जाओ done;
	}
	परिणाम[len] = '\0';
done:
	अगर (errsv) {
		मुक्त(परिणाम);
		परिणाम = शून्य;
	}
	बंद(pipes[0]);

	/* We make a best effort to reap child */
	ignore_परिणाम( waitpid(pid, शून्य, 0) );

	त्रुटि_सं = errsv;
	वापस परिणाम;

}

पूर्णांक ul_mkdir_p(स्थिर अक्षर *path, mode_t mode)
{
	अक्षर *p, *dir;
	पूर्णांक rc = 0;

	अगर (!path || !*path)
		वापस -EINVAL;

	dir = p = strdup(path);
	अगर (!dir)
		वापस -ENOMEM;

	अगर (*p == '/')
		p++;

	जबतक (p && *p) {
		अक्षर *e = म_अक्षर(p, '/');
		अगर (e)
			*e = '\0';
		अगर (*p) {
			rc = सूची_गढ़ो(dir, mode);
			अगर (rc && त्रुटि_सं != EEXIST)
				अवरोध;
			rc = 0;
		}
		अगर (!e)
			अवरोध;
		*e = '/';
		p = e + 1;
	}

	मुक्त(dir);
	वापस rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
अक्षर *stripoff_last_component(अक्षर *path)
{
	अक्षर *p = path ? म_खोजप(path, '/') : शून्य;

	अगर (!p)
		वापस शून्य;
	*p = '\0';
	वापस p + 1;
}

अटल पूर्णांक copy_file_simple(पूर्णांक from, पूर्णांक to)
{
	ssize_t nr;
	अक्षर buf[बफ_मान];

	जबतक ((nr = ul_read_all(from, buf, माप(buf))) > 0)
		अगर (ul_write_all(to, buf, nr) == -1)
			वापस UL_COPY_WRITE_ERROR;
	अगर (nr < 0)
		वापस UL_COPY_READ_ERROR;
#अगर_घोषित HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, माप(buf));
#पूर्ण_अगर
	वापस 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
पूर्णांक ul_copy_file(पूर्णांक from, पूर्णांक to)
{
#अगर_घोषित HAVE_SENDFILE
	काष्ठा stat st;
	ssize_t nw;

	अगर (ख_स्थिति(from, &st) == -1)
		वापस UL_COPY_READ_ERROR;
	अगर (!S_ISREG(st.st_mode))
		वापस copy_file_simple(from, to);
	अगर (ul_sendfile_all(to, from, शून्य, st.st_size) < 0)
		वापस copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	जबतक ((nw = ul_sendfile_all(to, from, शून्य, 16*1024*1024)) != 0)
		अगर (nw < 0)
			वापस copy_file_simple(from, to);
	वापस 0;
#अन्यथा
	वापस copy_file_simple(from, to);
#पूर्ण_अगर
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
अक्षर *ul_fd_mkpath(अक्षर *buf, माप_प्रकार bufsz, पूर्णांक fd)
{
	पूर्णांक len;

	अगर (fd < 0) {
		त्रुटि_सं = EBADF;
		वापस शून्य;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	अगर (len < 0 || (माप_प्रकार) len >= bufsz) {
		त्रुटि_सं = ENAMETOOLONG;
		वापस शून्य;
	}

	वापस buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
अक्षर *ul_fd_get_path(पूर्णांक fd)
{
	ssize_t ssz;
	अक्षर buf[PATH_MAX];
	अक्षर fdpath[UL_FDPATH_BUFSIZ];

	अगर (!ul_fd_mkpath(fdpath, माप(fdpath), fd))
		वापस शून्य;

	ssz = readlink(fdpath, buf, माप(buf));
	अगर (ssz < 0)
		वापस शून्य;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	अगर ((माप_प्रकार) ssz >= माप(buf)) {
		त्रुटि_सं = ENAMETOOLONG;
		वापस शून्य;
	}

	buf[ssz] = '\0';

	/* readlink() also succeeds for things without a pathname (pipes,
	 * sockets, ...) and it returns "<path> (deleted)" for unlinked files;
	 * refuse all of it rather than return a bogus path.
	 *
	 * Note that this also refuses a real file named "foo (deleted)". To
	 * tell it apart we would have to stat() the name, and such a path
	 * lookup may trigger an automount or block on an unreachable NFS
	 * server. */
	अगर (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		त्रुटि_सं = ENOENT;
		वापस शून्य;
	}

	वापस strdup(buf);
}

पूर्णांक ul_reopen(पूर्णांक fd, पूर्णांक flags)
{
	अक्षर *path = ul_fd_get_path(fd);
	पूर्णांक ret;

	अगर (!path)
		वापस -त्रुटि_सं;

	ret = खोलो(path, flags);
	मुक्त(path);

	वापस ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
अक्षर *ul_basename(अक्षर *path)
{
	अक्षर *p;

	अगर (!path || !*path)
		वापस (अक्षर *) ".";	/* ugly, static string */

	p = म_खोजप(path, '/');
	अगर (!p)
		वापस path;		/* no '/', return original */

	अगर (*(p + 1) != '\0')
		वापस p + 1;		/* begin of the name */

	जबतक (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	अगर (p > path) {
		*p-- = '\0';
		जबतक (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} अन्यथा जबतक (*(p + 1) != '\0')
		++p;

	वापस p;
}

#अगर_घोषित HAVE_OPENAT
/*
 * fopen_at_no_link() - Open a file stream that is not a symbolic/hard link.
 *
 * This function wraps around openat(2), fstat(2), ftruncate(2) and fdopen(3)
 * to create a file stream that is not a symbolic or hard link in a race-free
 * manner.
 *
 * @dir:	dirfd as passed to openat(2), e.g. AT_FDCWD for the calling process
 *		current working directory
 * @filename:	name of the target file
 * @flags:	open(2) file creation/status flags, O_NOFOLLOW is implicitly set
 * @perm:	open(2) file mode, can be bitwise ORed, these are only relevant
 *		when O_CREAT is set in @flags, otherwise pass as 0.
 * @mode:	fopen(3) mode
 *
 * Return: On success, a valid pointer to a file stream is returned.
 *         On failure, NULL is returned and errno is set to indicate the issue.
 */
खाता *fopen_at_no_link(पूर्णांक dir, स्थिर अक्षर *filename,
                             पूर्णांक flags, mode_t perm, स्थिर अक्षर *mode)
{
	खाता *fp;
	पूर्णांक fd;
	काष्ठा stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	अगर (fd < 0)
		वापस शून्य;

	अगर (ख_स्थिति(fd, &st)) {
		बंद(fd);
		वापस शून्य;
	}

	अगर (st.st_nlink > 1) {
		बंद(fd);
		त्रुटि_सं = EMLINK;
		वापस शून्य;
	}

	अगर ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		बंद(fd);
		वापस शून्य;
	}

	fp = fdopen(fd, mode);
	अगर (!fp)
		बंद(fd);
	वापस fp;
}
#पूर्ण_अगर /* HAVE_OPENAT */

#अगर defined(SYS_openat2)
पूर्णांक ul_openat_resolve(पूर्णांक dirfd, स्थिर अक्षर *path, पूर्णांक flags,
		      mode_t mode, अचिन्हित दीर्घ दीर्घ resolve)
{
	काष्ठा open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	वापस syscall(SYS_openat2, dirfd, path, &how, माप(how));
}
#अन्यथा
पूर्णांक ul_openat_resolve(
		पूर्णांक dirfd __attribute__((__unused__)),
		स्थिर अक्षर *path __attribute__((__unused__)),
		पूर्णांक flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		अचिन्हित दीर्घ दीर्घ resolve __attribute__((__unused__)))
{
	त्रुटि_सं = ENOSYS;
	वापस -1;
}
#पूर्ण_अगर

#अगर_घोषित __linux__
/*
 * Fallback for kernels without openat2() (Linux < 5.6).
 *
 * Open the path and then ask the kernel for the name of the result. If any
 * component of the path is a symbolic link then the name reported by the
 * kernel differs from the requested path and we refuse the file descriptor.
 * A concurrent rename is detected the same way. The name always belongs to
 * the file descriptor we return, so there is no time-of-check-to-time-of-use
 * window between the check and the use.
 *
 * Note that the symlink is detected after it has been followed rather than
 * refused during the path resolution. The path is opened with O_PATH to keep
 * this free of side effects (no device open, no blocking on a FIFO, ...) and
 * the caller's flags are applied by re-opening the verified file descriptor.
 *
 * Returns a file descriptor, or -1 and sets errno to ELOOP when a symlink has
 * been detected, or to ENOSYS when the check is not possible.
 */
अटल पूर्णांक open_no_symlinks_fallback(स्थिर अक्षर *path, पूर्णांक flags, mode_t mode)
{
	अक्षर *abspath = शून्य, *kpath = शून्य;
	काष्ठा stat st;
	पूर्णांक fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	अगर (!path || (flags & O_CREAT)) {
		त्रुटि_सं = ENOSYS;
		वापस -1;
	}

	अगर (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		अगर (!abspath)
			वापस -1;
	}

	अगर (flags & O_PATH)
		fd = खोलो(path, flags);
	अन्यथा
		fd = खोलो(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	अगर (fd < 0)
		जाओ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	अगर (ख_स्थिति(fd, &st) != 0)
		जाओ fail;
	अगर (S_ISLNK(st.st_mode)) {
		त्रुटि_सं = ELOOP;
		जाओ fail;
	}

	kpath = ul_fd_get_path(fd);
	अगर (!kpath) {
		त्रुटि_सं = ENOSYS;		/* no /proc, no verification */
		जाओ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	अगर (streq_paths(abspath ? abspath : path, kpath) != 1) {
		त्रुटि_सं = ELOOP;
		जाओ fail;
	}

	अगर (!(flags & O_PATH)) {
		अक्षर fdpath[UL_FDPATH_BUFSIZ];
		पूर्णांक nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		अगर (ul_fd_mkpath(fdpath, माप(fdpath), fd))
			nfd = खोलो(fdpath, flags, mode);
		अगर (nfd < 0)
			जाओ fail;
		बंद(fd);
		fd = nfd;
	}

	मुक्त(abspath);
	मुक्त(kpath);
	वापस fd;
fail:
	errsv = त्रुटि_सं;
	मुक्त(abspath);
	मुक्त(kpath);
	अगर (fd >= 0)
		बंद(fd);
	त्रुटि_सं = errsv;
	वापस -1;
}
#अन्यथा /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
अटल पूर्णांक open_no_symlinks_fallback(
			स्थिर अक्षर *path __attribute__((__unused__)),
			पूर्णांक flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	त्रुटि_सं = ENOSYS;
	वापस -1;
}
#पूर्ण_अगर /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
पूर्णांक ul_open_no_symlinks(स्थिर अक्षर *path, पूर्णांक flags, mode_t mode)
{
	पूर्णांक fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	अगर (fd < 0 && त्रुटि_सं == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	वापस fd;
}

#अगर_घोषित HAVE_UL_SAFE_STATX
/* statx() that never asks the filesystem and never triggers an automount, so
 * it cannot block on a hung NFS server or FUSE daemon. AT_EMPTY_PATH is added
 * to @flags when @path is NULL or empty.
 *
 * Note that statx(2) is not obliged to return all the attributes the caller
 * asked for; the caller has to check stx_mask.
 *
 * The @stx is always zeroized, so it never contains stale data on error.
 *
 * Returns 0 on success, otherwise negative errno (and errno is set too).
 */
पूर्णांक ul_safe_statx(पूर्णांक dirfd, स्थिर अक्षर *path, पूर्णांक flags,
		  अचिन्हित पूर्णांक mask, काष्ठा statx *stx)
{
	निश्चित(stx);

	स_रखो(stx, 0, माप(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#अगर_घोषित AT_EMPTY_PATH
	अगर (!path || !*path)
		flags |= AT_EMPTY_PATH;
#पूर्ण_अगर
	अगर (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		वापस -त्रुटि_सं;

	वापस 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
व्योम ul_statx_to_stat(स्थिर काष्ठा statx *stx, काष्ठा stat *st)
{
	स_रखो(st, 0, माप(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	अगर (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	अगर (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	अगर (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	अगर (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	अगर (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	अगर (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	अगर (stx->stx_mask & STATX_आकार)
		st->st_size = stx->stx_आकार;
	अगर (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	अगर (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	अगर (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	अगर (stx->stx_mask & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#पूर्ण_अगर

/* This very simplified stat() alternative uses cached VFS data and does not
 * directly ask the filesystem for details. It requires a kernel that supports
 * statx() with AT_STATX_DONT_SYNC.
 *
 * @mask is a statx(2) attribute mask, see UL_STATX_*. UL_STATX_ESSENTIAL is
 * always added to it, so zero is a valid request for the minimum. Attributes
 * the kernel did not return are zero in @st.
 *
 * The optional @retmask returns the mask of the attributes the kernel really
 * provided. Note that it may be less than requested (that's the whole point
 * of AT_STATX_DONT_SYNC), but also more, because the kernel returns whatever
 * it has cheaply at hand.
 *
 * Returns 0 on success, otherwise negative errno (and errno is set too).
 * -EOPNOTSUPP means the kernel did not provide the essential attributes and
 * the caller should fall back to stat().
 */
पूर्णांक ul_safe_stat(स्थिर अक्षर *target, काष्ठा stat *st,
		 पूर्णांक nofollow __attribute__((__unused__)),
		 अचिन्हित पूर्णांक mask __attribute__((__unused__)),
		 अचिन्हित पूर्णांक *retmask)
{
	निश्चित(target);
	निश्चित(st);

	स_रखो(st, 0, माप(काष्ठा stat));
	अगर (retmask)
		*retmask = 0;

#अगर_घोषित HAVE_UL_SAFE_STAT
	{
		काष्ठा statx stx = { 0 };
		पूर्णांक rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		अगर (rc)
			वापस rc;

		ul_statx_to_stat(&stx, st);

		अगर (retmask)
			*retmask = stx.stx_mask;

		अगर ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			त्रुटि_सं = EOPNOTSUPP;
			वापस -EOPNOTSUPP;
		}

		वापस 0;
	}
#अन्यथा
	त्रुटि_सं = ENOSYS;
	वापस -ENOSYS;
#पूर्ण_अगर
}


#अगर_घोषित TEST_PROGRAM_FILEUTILS
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अगर (argc < 2)
		errx(निकास_त्रुटि, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	अगर (म_भेद(argv[1], "--mkstemp") == 0) {
		खाता *f;
		अक्षर *tmpname = शून्य;

		f = xfmkstemp(&tmpname, शून्य, "test");
		unlink(tmpname);
		मुक्त(tmpname);
		ख_बंद(f);

	} अन्यथा अगर (म_भेद(argv[1], "--close-fds") == 0) {
		ignore_परिणाम( dup(STDIN_FILENO) );
		ignore_परिणाम( dup(STDIN_FILENO) );
		ignore_परिणाम( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		अगर (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} अन्यथा अगर (म_भेद(argv[1], "--copy-file") == 0) {
		पूर्णांक ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		अगर (ret == UL_COPY_READ_ERROR)
			err(निकास_त्रुटि, "read");
		अन्यथा अगर (ret == UL_COPY_WRITE_ERROR)
			err(निकास_त्रुटि, "write");

	} अन्यथा अगर (म_भेद(argv[1], "--open-no-symlinks") == 0) {
#अगर_घोषित __linux__
		पूर्णांक flags = O_PATH | O_CLOEXEC;
#अन्यथा
		पूर्णांक flags = O_RDONLY | O_CLOEXEC;
#पूर्ण_अगर
		पूर्णांक fallback = 0;
		अक्षर *name;
		पूर्णांक fd, i;

		अगर (argc < 3)
			errx(निकास_त्रुटि, "no path specified");

		क्रम (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			अगर (म_भेद(argv[i], "--fallback") == 0)
				fallback = 1;
			अन्यथा अगर (म_भेद(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		अगर (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		अन्यथा
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		अगर (fd < 0)
			err(निकास_त्रुटि, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		म_लिखो("%s\n", name);
		मुक्त(name);
		बंद(fd);

	} अन्यथा अगर (म_भेद(argv[1], "--safe-stat") == 0) {
		अचिन्हित पूर्णांक mask = 0, retmask = 0;
		पूर्णांक nofollow = 0;
		काष्ठा stat st, ref;
		पूर्णांक rc, i;

		अगर (argc < 3)
			errx(निकास_त्रुटि, "no path specified");

		क्रम (i = 3; i < argc; i++) {
			अगर (म_भेद(argv[i], "--nofollow") == 0)
				nofollow = 1;
			अन्यथा अगर (म_भेद(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		म_लिखो("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		म_लिखो("      rc: %d\n", rc);
		म_लिखो("   errno: %d\n", rc ? त्रुटि_सं : 0);
		अगर (rc)
			वापस निकास_त्रुटि;

		म_लिखो("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
					 S_ISREG(st.st_mode)  ? "reg" :
					 S_ISLNK(st.st_mode)  ? "lnk" :
					 S_ISBLK(st.st_mode)  ? "blk" :
					 S_ISCHR(st.st_mode)  ? "chr" :
					 S_ISFIFO(st.st_mode) ? "fifo" :
					 S_ISSOCK(st.st_mode) ? "sock" : "unknown");

		/* The attribute values depend on the system, so compare them
		 * with the classic stat() rather than print them. Only the
		 * attributes advertised by @retmask are comparable, the rest
		 * is zero in @st.
		 */
		अगर (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(निकास_त्रुटि, "%s", argv[2]);

		म_लिखो("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		म_लिखो(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#अगर_घोषित HAVE_UL_SAFE_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			अचिन्हित पूर्णांक emask = mask | UL_STATX_ESSENTIAL;

# define परिणाम_2(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			अगर (emask & STATX_TYPE)
				म_लिखो("  S_IFMT: %s\n", परिणाम_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			अगर (emask & STATX_MODE)
				म_लिखो(" st_mode: %s\n", परिणाम_2(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			अगर (emask & STATX_INO)
				म_लिखो("  st_ino: %s\n",
					परिणाम_2(STATX_INO, st.st_ino == ref.st_ino));
			अगर (emask & STATX_NLINK)
				म_लिखो("st_nlink: %s\n",
					परिणाम_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			अगर (emask & STATX_UID)
				म_लिखो("  st_uid: %s\n",
					परिणाम_2(STATX_UID, st.st_uid == ref.st_uid));
			अगर (emask & STATX_GID)
				म_लिखो("  st_gid: %s\n",
					परिणाम_2(STATX_GID, st.st_gid == ref.st_gid));
			अगर (emask & STATX_आकार)
				म_लिखो(" st_size: %s\n",
					परिणाम_2(STATX_आकार, st.st_size == ref.st_size));
# undef परिणाम_2
		}
#पूर्ण_अगर
	}
	वापस निकास_सफल;
}
#पूर्ण_अगर
