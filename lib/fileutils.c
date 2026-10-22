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
	प्रत्यागम mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#अन्यथा
	पूर्णाङ्क fd, old_flags, errno_save;

	fd = mkstemp(template);
	यदि (fd < 0)
		प्रत्यागम fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	यदि (old_flags < 0)
		गच्छ unwind;
	यदि (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		गच्छ unwind;

	प्रत्यागम fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	प्रत्यागम -1;
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
	यदि (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	यदि (rc < 0)
		प्रत्यागम -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	यदि (fd == -1) {
		मुक्त(localtmp);
		localtmp = रिक्त;
	}
	*tmpname = localtmp;
	प्रत्यागम fd;
}

#ifdef F_DUPFD_CLOEXEC
पूर्णाङ्क dup_fd_cloexec(पूर्णाङ्क oldfd, पूर्णाङ्क lowfd)
#अन्यथा
पूर्णाङ्क dup_fd_cloexec(पूर्णाङ्क oldfd, पूर्णाङ्क lowfd  __attribute__((__unused__)))
#endif
{
	पूर्णाङ्क fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	यदि (fd >= 0)
		प्रत्यागम fd;
#endif

	fd = dup(oldfd);
	यदि (fd < 0)
		प्रत्यागम fd;

	flags = fcntl(fd, F_GETFD);
	यदि (flags < 0)
		गच्छ unwind;
	यदि (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		गच्छ unwind;

	प्रत्यागम fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	प्रत्यागम -1;
}

/*
 * portable getdtablesize()
 */
unsigned पूर्णाङ्क get_fd_tabsize(शून्य)
{
	पूर्णाङ्क m;

#यदि defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#नोचेद्यदि defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	संरचना rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#नोचेद्यदि defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#अन्यथा
	m = OPEN_MAX;
#endif
	प्रत्यागम m;
}

शून्य ul_close_all_fds(unsigned पूर्णाङ्क first, unsigned पूर्णाङ्क last)
{
	संरचना dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	यदि (dir) {
		यावत् ((d = xreaddir(dir))) {
			वर्ण *end;
			unsigned पूर्णाङ्क fd;
			पूर्णाङ्क dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			यदि (errno || end == d->d_name || !end || *end)
				अनुवर्त;
			dfd = dirfd(dir);
			यदि (dfd < 0)
				अनुवर्त;
			यदि ((unsigned पूर्णाङ्क)dfd == fd)
				अनुवर्त;
			यदि (fd < first || last < fd)
				अनुवर्त;
			close(fd);
		}
		closedir(dir);
	} अन्यथा {
		unsigned fd, tbsz = get_fd_tabsize();

		प्रति (fd = 0; fd < tbsz; fd++) {
			यदि (first <= fd && fd <= last)
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

	यदि (!path || !*path)
		प्रत्यागम रिक्त;

	यदि (pipe(pipes) != 0)
		प्रत्यागम रिक्त;
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
		प्रत्यागम रिक्त;			/* fork error */
	पक्ष 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		यदि (drop_permissions() != 0)
			result = रिक्त;	/* failed */
		अन्यथा
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (वर्ण *) &len, परिमाण(len));
		यदि (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	स्वभाव:
		भङ्ग;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	यदि (ul_read_all(pipes[0], (वर्ण *) &len, परिमाण(len)) != परिमाण(len))
		गच्छ done;
	यदि (len < 0) {
		errsv = -len;
		गच्छ done;
	}

	result = malloc(len + 1);
	यदि (!result) {
		errsv = ENOMEM;
		गच्छ done;
	}
	/* read path */
	यदि (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		गच्छ done;
	}
	result[len] = '\0';
done:
	यदि (errsv) {
		मुक्त(result);
		result = रिक्त;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, रिक्त, 0) );

	errno = errsv;
	प्रत्यागम result;

}

पूर्णाङ्क ul_mkdir_p(ध्रुव वर्ण *path, mode_t mode)
{
	वर्ण *p, *dir;
	पूर्णाङ्क rc = 0;

	यदि (!path || !*path)
		प्रत्यागम -EINVAL;

	dir = p = strdup(path);
	यदि (!dir)
		प्रत्यागम -ENOMEM;

	यदि (*p == '/')
		p++;

	यावत् (p && *p) {
		वर्ण *e = strchr(p, '/');
		यदि (e)
			*e = '\0';
		यदि (*p) {
			rc = mkdir(dir, mode);
			यदि (rc && errno != EEXIST)
				भङ्ग;
			rc = 0;
		}
		यदि (!e)
			भङ्ग;
		*e = '/';
		p = e + 1;
	}

	मुक्त(dir);
	प्रत्यागम rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
वर्ण *stripoff_last_component(वर्ण *path)
{
	वर्ण *p = path ? strrchr(path, '/') : रिक्त;

	यदि (!p)
		प्रत्यागम रिक्त;
	*p = '\0';
	प्रत्यागम p + 1;
}

स्थिर पूर्णाङ्क copy_file_simple(पूर्णाङ्क from, पूर्णाङ्क to)
{
	ssize_t nr;
	वर्ण buf[BUFSIZ];

	यावत् ((nr = ul_read_all(from, buf, परिमाण(buf))) > 0)
		यदि (ul_write_all(to, buf, nr) == -1)
			प्रत्यागम UL_COPY_WRITE_ERROR;
	यदि (nr < 0)
		प्रत्यागम UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, परिमाण(buf));
#endif
	प्रत्यागम 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
पूर्णाङ्क ul_copy_file(पूर्णाङ्क from, पूर्णाङ्क to)
{
#ifdef HAVE_SENDFILE
	संरचना stat st;
	ssize_t nw;

	यदि (fstat(from, &st) == -1)
		प्रत्यागम UL_COPY_READ_ERROR;
	यदि (!S_ISREG(st.st_mode))
		प्रत्यागम copy_file_simple(from, to);
	यदि (ul_sendfile_all(to, from, रिक्त, st.st_size) < 0)
		प्रत्यागम copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	यावत् ((nw = ul_sendfile_all(to, from, रिक्त, 16*1024*1024)) != 0)
		यदि (nw < 0)
			प्रत्यागम copy_file_simple(from, to);
	प्रत्यागम 0;
#अन्यथा
	प्रत्यागम copy_file_simple(from, to);
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

	यदि (fd < 0) {
		errno = EBADF;
		प्रत्यागम रिक्त;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	यदि (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
वर्ण *ul_fd_get_path(पूर्णाङ्क fd)
{
	ssize_t ssz;
	वर्ण buf[PATH_MAX];
	वर्ण fdpath[UL_FDPATH_BUFSIZ];

	यदि (!ul_fd_mkpath(fdpath, परिमाण(fdpath), fd))
		प्रत्यागम रिक्त;

	ssz = readlink(fdpath, buf, परिमाण(buf));
	यदि (ssz < 0)
		प्रत्यागम रिक्त;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	यदि ((size_t) ssz >= परिमाण(buf)) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
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
	यदि (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम strdup(buf);
}

पूर्णाङ्क ul_reopen(पूर्णाङ्क fd, पूर्णाङ्क flags)
{
	वर्ण *path = ul_fd_get_path(fd);
	पूर्णाङ्क ret;

	यदि (!path)
		प्रत्यागम -errno;

	ret = open(path, flags);
	मुक्त(path);

	प्रत्यागम ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
वर्ण *ul_basename(वर्ण *path)
{
	वर्ण *p;

	यदि (!path || !*path)
		प्रत्यागम (वर्ण *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	यदि (!p)
		प्रत्यागम path;		/* no '/', return original */

	यदि (*(p + 1) != '\0')
		प्रत्यागम p + 1;		/* begin of the name */

	यावत् (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	यदि (p > path) {
		*p-- = '\0';
		यावत् (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} अन्यथा यावत् (*(p + 1) != '\0')
		++p;

	प्रत्यागम p;
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
	यदि (fd < 0)
		प्रत्यागम रिक्त;

	यदि (fstat(fd, &st)) {
		close(fd);
		प्रत्यागम रिक्त;
	}

	यदि (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		प्रत्यागम रिक्त;
	}

	यदि ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		प्रत्यागम रिक्त;
	}

	fp = fdopen(fd, mode);
	यदि (!fp)
		close(fd);
	प्रत्यागम fp;
}
#endif /* HAVE_OPENAT */

#यदि defined(SYS_openat2)
पूर्णाङ्क ul_openat_resolve(पूर्णाङ्क dirfd, ध्रुव वर्ण *path, पूर्णाङ्क flags,
		      mode_t mode, unsigned long long resolve)
{
	संरचना open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	प्रत्यागम syscall(SYS_openat2, dirfd, path, &how, परिमाण(how));
}
#अन्यथा
पूर्णाङ्क ul_openat_resolve(
		पूर्णाङ्क dirfd __attribute__((__unused__)),
		ध्रुव वर्ण *path __attribute__((__unused__)),
		पूर्णाङ्क flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	प्रत्यागम -1;
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
	यदि (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		प्रत्यागम -1;
	}

	यदि (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		यदि (!abspath)
			प्रत्यागम -1;
	}

	यदि (flags & O_PATH)
		fd = open(path, flags);
	अन्यथा
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	यदि (fd < 0)
		गच्छ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	यदि (fstat(fd, &st) != 0)
		गच्छ fail;
	यदि (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		गच्छ fail;
	}

	kpath = ul_fd_get_path(fd);
	यदि (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		गच्छ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	यदि (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		गच्छ fail;
	}

	यदि (!(flags & O_PATH)) {
		वर्ण fdpath[UL_FDPATH_BUFSIZ];
		पूर्णाङ्क nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		यदि (ul_fd_mkpath(fdpath, परिमाण(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		यदि (nfd < 0)
			गच्छ fail;
		close(fd);
		fd = nfd;
	}

	मुक्त(abspath);
	मुक्त(kpath);
	प्रत्यागम fd;
fail:
	errsv = errno;
	मुक्त(abspath);
	मुक्त(kpath);
	यदि (fd >= 0)
		close(fd);
	errno = errsv;
	प्रत्यागम -1;
}
#अन्यथा /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
स्थिर पूर्णाङ्क open_no_symlinks_fallback(
			ध्रुव वर्ण *path __attribute__((__unused__)),
			पूर्णाङ्क flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	प्रत्यागम -1;
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
	यदि (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	प्रत्यागम fd;
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
	यदि (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	यदि (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		प्रत्यागम -errno;

	प्रत्यागम 0;
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
	यदि (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	यदि (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	यदि (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	यदि (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	यदि (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	यदि (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	यदि (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	यदि (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	यदि (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	यदि (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	यदि (stx->stx_mask & STATX_CTIME) {
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
	यदि (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		संरचना statx stx = { 0 };
		पूर्णाङ्क rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		यदि (rc)
			प्रत्यागम rc;

		ul_statx_to_stat(&stx, st);

		यदि (retmask)
			*retmask = stx.stx_mask;

		यदि ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			प्रत्यागम -EOPNOTSUPP;
		}

		प्रत्यागम 0;
	}
#अन्यथा
	errno = ENOSYS;
	प्रत्यागम -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	यदि (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	यदि (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		वर्ण *tmpname = रिक्त;

		f = xfmkstemp(&tmpname, रिक्त, "test");
		unlink(tmpname);
		मुक्त(tmpname);
		fclose(f);

	} अन्यथा यदि (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		यदि (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} अन्यथा यदि (strcmp(argv[1], "--copy-file") == 0) {
		पूर्णाङ्क ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		यदि (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		अन्यथा यदि (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} अन्यथा यदि (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		पूर्णाङ्क flags = O_PATH | O_CLOEXEC;
#अन्यथा
		पूर्णाङ्क flags = O_RDONLY | O_CLOEXEC;
#endif
		पूर्णाङ्क fallback = 0;
		वर्ण *name;
		पूर्णाङ्क fd, i;

		यदि (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		प्रति (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			यदि (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			अन्यथा यदि (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		यदि (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		अन्यथा
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		यदि (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		मुक्त(name);
		close(fd);

	} अन्यथा यदि (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned पूर्णाङ्क mask = 0, retmask = 0;
		पूर्णाङ्क nofollow = 0;
		संरचना stat st, ref;
		पूर्णाङ्क rc, i;

		यदि (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		प्रति (i = 3; i < argc; i++) {
			यदि (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			अन्यथा यदि (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		यदि (rc)
			प्रत्यागम EXIT_FAILURE;

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
		यदि (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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

			यदि (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			यदि (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			यदि (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			यदि (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			यदि (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			यदि (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			यदि (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	प्रत्यागम EXIT_SUCCESS;
}
#endif
