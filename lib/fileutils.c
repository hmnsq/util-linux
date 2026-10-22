/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "canonicalize.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "strutils.h"

पूर्णाङ्क mkstemp_cloexec(वर्ण *template)
{
#ifdef HAVE_MKOSTEMP
	पटिनिवत्त mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#अञ्ञथा
	पूर्णाङ्क fd, old_flags, errno_save;

	fd = mkstemp(template);
	सचे (fd < 0)
		पटिनिवत्त fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	सचे (old_flags < 0)
		गच्छ unwind;
	सचे (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		गच्छ unwind;

	पटिनिवत्त fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	पटिनिवत्त -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
पूर्णाङ्क xmkstemp(वर्ण **tmpname, ध्रुव वर्ण *dir, ध्रुव वर्ण *prefix)
{
	वर्ण *localtmp;
	ध्रुव वर्ण *tmpenv;
	mode_t old_mode;
	पूर्णाङ्क fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	सचे (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	सचे (rc < 0)
		पटिनिवत्त -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	सचे (fd == -1) {
		मुक्त(localtmp);
		localtmp = रिक्त;
	}
	*tmpname = localtmp;
	पटिनिवत्त fd;
}

#ifdef F_DUPFD_CLOEXEC
पूर्णाङ्क dup_fd_cloexec(पूर्णाङ्क oldfd, पूर्णाङ्क lowfd)
#अञ्ञथा
पूर्णाङ्क dup_fd_cloexec(पूर्णाङ्क oldfd, पूर्णाङ्क lowfd  __attribute__((__unused__)))
#endif
{
	पूर्णाङ्क fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	सचे (fd >= 0)
		पटिनिवत्त fd;
#endif

	fd = dup(oldfd);
	सचे (fd < 0)
		पटिनिवत्त fd;

	flags = fcntl(fd, F_GETFD);
	सचे (flags < 0)
		गच्छ unwind;
	सचे (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		गच्छ unwind;

	पटिनिवत्त fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	पटिनिवत्त -1;
}

/*
 * portable getdtablesize()
 */
unsigned पूर्णाङ्क get_fd_tabsize(शून्य)
{
	पूर्णाङ्क m;

#सचे defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#नोचेद्यदि defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	संरचना rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#नोचेद्यदि defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#अञ्ञथा
	m = OPEN_MAX;
#endif
	पटिनिवत्त m;
}

शून्य ul_close_all_fds(unsigned पूर्णाङ्क first, unsigned पूर्णाङ्क last)
{
	संरचना dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	सचे (dir) {
		याव ((d = xreaddir(dir))) {
			वर्ण *end;
			unsigned पूर्णाङ्क fd;
			पूर्णाङ्क dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			सचे (errno || end == d->d_name || !end || *end)
				अनुवर्त;
			dfd = dirfd(dir);
			सचे (dfd < 0)
				अनुवर्त;
			सचे ((unsigned पूर्णाङ्क)dfd == fd)
				अनुवर्त;
			सचे (fd < first || last < fd)
				अनुवर्त;
			close(fd);
		}
		closedir(dir);
	} अञ्ञथा {
		unsigned fd, tbsz = get_fd_tabsize();

		अत्थाय (fd = 0; fd < tbsz; fd++) {
			सचे (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
वर्ण *ul_restricted_path_oper(ध्रुव वर्ण *path,
		  पूर्णाङ्क (*oper)(ध्रुव वर्ण *path, वर्ण **result, शून्य *data),
		  शून्य *data)
{
	वर्ण *result = रिक्त;
	पूर्णाङ्क errsv = 0;
	पूर्णाङ्क pipes[2];
	ssize_t len;
	pid_t pid;

	सचे (!path || !*path)
		पटिनिवत्त रिक्त;

	सचे (pipe(pipes) != 0)
		पटिनिवत्त रिक्त;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	विकल्प (pid) {
	पक्ष -1:
		close(pipes[0]);
		close(pipes[1]);
		पटिनिवत्त रिक्त;			/* fork error */
	पक्ष 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		सचे (drop_permissions() != 0)
			result = रिक्त;	/* failed */
		अञ्ञथा
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (वर्ण *) &len, परिमाण(len));
		सचे (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	स्वभाव:
		भङ्ग;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	सचे (ul_read_all(pipes[0], (वर्ण *) &len, परिमाण(len)) != परिमाण(len))
		गच्छ done;
	सचे (len < 0) {
		errsv = -len;
		गच्छ done;
	}

	result = malloc(len + 1);
	सचे (!result) {
		errsv = ENOMEM;
		गच्छ done;
	}
	/* read path */
	सचे (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		गच्छ done;
	}
	result[len] = '\0';
done:
	सचे (errsv) {
		मुक्त(result);
		result = रिक्त;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, रिक्त, 0) );

	errno = errsv;
	पटिनिवत्त result;

}

पूर्णाङ्क ul_mkdir_p(ध्रुव वर्ण *path, mode_t mode)
{
	वर्ण *p, *dir;
	पूर्णाङ्क rc = 0;

	सचे (!path || !*path)
		पटिनिवत्त -EINVAL;

	dir = p = strdup(path);
	सचे (!dir)
		पटिनिवत्त -ENOMEM;

	सचे (*p == '/')
		p++;

	याव (p && *p) {
		वर्ण *e = strchr(p, '/');
		सचे (e)
			*e = '\0';
		सचे (*p) {
			rc = mkdir(dir, mode);
			सचे (rc && errno != EEXIST)
				भङ्ग;
			rc = 0;
		}
		सचे (!e)
			भङ्ग;
		*e = '/';
		p = e + 1;
	}

	मुक्त(dir);
	पटिनिवत्त rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
वर्ण *stripoff_last_component(वर्ण *path)
{
	वर्ण *p = path ? strrchr(path, '/') : रिक्त;

	सचे (!p)
		पटिनिवत्त रिक्त;
	*p = '\0';
	पटिनिवत्त p + 1;
}

स्थिर पूर्णाङ्क copy_file_simple(पूर्णाङ्क from, पूर्णाङ्क to)
{
	ssize_t nr;
	वर्ण buf[BUFSIZ];

	याव ((nr = ul_read_all(from, buf, परिमाण(buf))) > 0)
		सचे (ul_write_all(to, buf, nr) == -1)
			पटिनिवत्त UL_COPY_WRITE_ERROR;
	सचे (nr < 0)
		पटिनिवत्त UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, परिमाण(buf));
#endif
	पटिनिवत्त 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
पूर्णाङ्क ul_copy_file(पूर्णाङ्क from, पूर्णाङ्क to)
{
#ifdef HAVE_SENDFILE
	संरचना stat st;
	ssize_t nw;

	सचे (fstat(from, &st) == -1)
		पटिनिवत्त UL_COPY_READ_ERROR;
	सचे (!S_ISREG(st.st_mode))
		पटिनिवत्त copy_file_simple(from, to);
	सचे (ul_sendfile_all(to, from, रिक्त, st.st_size) < 0)
		पटिनिवत्त copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	याव ((nw = ul_sendfile_all(to, from, रिक्त, 16*1024*1024)) != 0)
		सचे (nw < 0)
			पटिनिवत्त copy_file_simple(from, to);
	पटिनिवत्त 0;
#अञ्ञथा
	पटिनिवत्त copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
वर्ण *ul_fd_mkpath(वर्ण *buf, size_t bufsz, पूर्णाङ्क fd)
{
	पूर्णाङ्क len;

	सचे (fd < 0) {
		errno = EBADF;
		पटिनिवत्त रिक्त;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	सचे (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
वर्ण *ul_fd_get_path(पूर्णाङ्क fd)
{
	ssize_t ssz;
	वर्ण buf[PATH_MAX];
	वर्ण fdpath[UL_FDPATH_BUFSIZ];

	सचे (!ul_fd_mkpath(fdpath, परिमाण(fdpath), fd))
		पटिनिवत्त रिक्त;

	ssz = readlink(fdpath, buf, परिमाण(buf));
	सचे (ssz < 0)
		पटिनिवत्त रिक्त;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	सचे ((size_t) ssz >= परिमाण(buf)) {
		errno = ENAMETOOLONG;
		पटिनिवत्त रिक्त;
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
	सचे (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त strdup(buf);
}

पूर्णाङ्क ul_reopen(पूर्णाङ्क fd, पूर्णाङ्क flags)
{
	वर्ण *path = ul_fd_get_path(fd);
	पूर्णाङ्क ret;

	सचे (!path)
		पटिनिवत्त -errno;

	ret = open(path, flags);
	मुक्त(path);

	पटिनिवत्त ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
वर्ण *ul_basename(वर्ण *path)
{
	वर्ण *p;

	सचे (!path || !*path)
		पटिनिवत्त (वर्ण *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	सचे (!p)
		पटिनिवत्त path;		/* no '/', return original */

	सचे (*(p + 1) != '\0')
		पटिनिवत्त p + 1;		/* begin of the name */

	याव (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	सचे (p > path) {
		*p-- = '\0';
		याव (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} अञ्ञथा याव (*(p + 1) != '\0')
		++p;

	पटिनिवत्त p;
}

#ifdef HAVE_OPENAT
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
FILE *fopen_at_no_link(पूर्णाङ्क dir, ध्रुव वर्ण *filename,
                             पूर्णाङ्क flags, mode_t perm, ध्रुव वर्ण *mode)
{
	FILE *fp;
	पूर्णाङ्क fd;
	संरचना stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	सचे (fd < 0)
		पटिनिवत्त रिक्त;

	सचे (fstat(fd, &st)) {
		close(fd);
		पटिनिवत्त रिक्त;
	}

	सचे (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		पटिनिवत्त रिक्त;
	}

	सचे ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		पटिनिवत्त रिक्त;
	}

	fp = fdopen(fd, mode);
	सचे (!fp)
		close(fd);
	पटिनिवत्त fp;
}
#endif /* HAVE_OPENAT */

#सचे defined(SYS_openat2)
पूर्णाङ्क ul_openat_resolve(पूर्णाङ्क dirfd, ध्रुव वर्ण *path, पूर्णाङ्क flags,
		      mode_t mode, unsigned long long resolve)
{
	संरचना open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	पटिनिवत्त syscall(SYS_openat2, dirfd, path, &how, परिमाण(how));
}
#अञ्ञथा
पूर्णाङ्क ul_openat_resolve(
		पूर्णाङ्क dirfd __attribute__((__unused__)),
		ध्रुव वर्ण *path __attribute__((__unused__)),
		पूर्णाङ्क flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	पटिनिवत्त -1;
}
#endif

#ifdef __linux__
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
स्थिर पूर्णाङ्क open_no_symlinks_fallback(ध्रुव वर्ण *path, पूर्णाङ्क flags, mode_t mode)
{
	वर्ण *abspath = रिक्त, *kpath = रिक्त;
	संरचना stat st;
	पूर्णाङ्क fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	सचे (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		पटिनिवत्त -1;
	}

	सचे (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		सचे (!abspath)
			पटिनिवत्त -1;
	}

	सचे (flags & O_PATH)
		fd = open(path, flags);
	अञ्ञथा
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	सचे (fd < 0)
		गच्छ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	सचे (fstat(fd, &st) != 0)
		गच्छ fail;
	सचे (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		गच्छ fail;
	}

	kpath = ul_fd_get_path(fd);
	सचे (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		गच्छ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	सचे (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		गच्छ fail;
	}

	सचे (!(flags & O_PATH)) {
		वर्ण fdpath[UL_FDPATH_BUFSIZ];
		पूर्णाङ्क nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		सचे (ul_fd_mkpath(fdpath, परिमाण(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		सचे (nfd < 0)
			गच्छ fail;
		close(fd);
		fd = nfd;
	}

	मुक्त(abspath);
	मुक्त(kpath);
	पटिनिवत्त fd;
fail:
	errsv = errno;
	मुक्त(abspath);
	मुक्त(kpath);
	सचे (fd >= 0)
		close(fd);
	errno = errsv;
	पटिनिवत्त -1;
}
#अञ्ञथा /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
स्थिर पूर्णाङ्क open_no_symlinks_fallback(
			ध्रुव वर्ण *path __attribute__((__unused__)),
			पूर्णाङ्क flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	पटिनिवत्त -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
पूर्णाङ्क ul_open_no_symlinks(ध्रुव वर्ण *path, पूर्णाङ्क flags, mode_t mode)
{
	पूर्णाङ्क fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	सचे (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	पटिनिवत्त fd;
}

#ifdef HAVE_UL_SAFE_STATX
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
पूर्णाङ्क ul_safe_statx(पूर्णाङ्क dirfd, ध्रुव वर्ण *path, पूर्णाङ्क flags,
		  unsigned पूर्णाङ्क mask, संरचना statx *stx)
{
	प्रतिज्ञा(stx);

	memset(stx, 0, परिमाण(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	सचे (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	सचे (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		पटिनिवत्त -errno;

	पटिनिवत्त 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
शून्य ul_statx_to_stat(ध्रुव संरचना statx *stx, संरचना stat *st)
{
	memset(st, 0, परिमाण(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	सचे (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	सचे (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	सचे (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	सचे (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	सचे (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	सचे (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	सचे (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	सचे (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	सचे (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	सचे (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	सचे (stx->stx_mask & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#endif

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
पूर्णाङ्क ul_safe_stat(ध्रुव वर्ण *target, संरचना stat *st,
		 पूर्णाङ्क nofollow __attribute__((__unused__)),
		 unsigned पूर्णाङ्क mask __attribute__((__unused__)),
		 unsigned पूर्णाङ्क *retmask)
{
	प्रतिज्ञा(target);
	प्रतिज्ञा(st);

	memset(st, 0, परिमाण(संरचना stat));
	सचे (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		संरचना statx stx = { 0 };
		पूर्णाङ्क rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		सचे (rc)
			पटिनिवत्त rc;

		ul_statx_to_stat(&stx, st);

		सचे (retmask)
			*retmask = stx.stx_mask;

		सचे ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			पटिनिवत्त -EOPNOTSUPP;
		}

		पटिनिवत्त 0;
	}
#अञ्ञथा
	errno = ENOSYS;
	पटिनिवत्त -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	सचे (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	सचे (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		वर्ण *tmpname = रिक्त;

		f = xfmkstemp(&tmpname, रिक्त, "test");
		unlink(tmpname);
		मुक्त(tmpname);
		fclose(f);

	} अञ्ञथा सचे (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		सचे (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} अञ्ञथा सचे (strcmp(argv[1], "--copy-file") == 0) {
		पूर्णाङ्क ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		सचे (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		अञ्ञथा सचे (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} अञ्ञथा सचे (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		पूर्णाङ्क flags = O_PATH | O_CLOEXEC;
#अञ्ञथा
		पूर्णाङ्क flags = O_RDONLY | O_CLOEXEC;
#endif
		पूर्णाङ्क fallback = 0;
		वर्ण *name;
		पूर्णाङ्क fd, i;

		सचे (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		अत्थाय (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			सचे (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			अञ्ञथा सचे (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		सचे (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		अञ्ञथा
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		सचे (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		मुक्त(name);
		close(fd);

	} अञ्ञथा सचे (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned पूर्णाङ्क mask = 0, retmask = 0;
		पूर्णाङ्क nofollow = 0;
		संरचना stat st, ref;
		पूर्णाङ्क rc, i;

		सचे (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		अत्थाय (i = 3; i < argc; i++) {
			सचे (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			अञ्ञथा सचे (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		सचे (rc)
			पटिनिवत्त EXIT_FAILURE;

		printf("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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
		सचे (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(EXIT_FAILURE, "%s", argv[2]);

		printf("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		printf(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#ifdef HAVE_UL_SAFE_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			unsigned पूर्णाङ्क emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			सचे (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			सचे (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			सचे (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			सचे (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			सचे (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			सचे (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			सचे (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	पटिनिवत्त EXIT_SUCCESS;
}
#endif
