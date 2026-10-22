/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "canonicalize.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "strutils.h"

পূর্ণসংখ্যা mkstemp_cloexec(অক্ষর *template)
{
#ifdef HAVE_MKOSTEMP
	ফেরত mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#অন্যথা
	পূর্ণসংখ্যা fd, old_flags, errno_save;

	fd = mkstemp(template);
	যদি (fd < 0)
		ফেরত fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	যদি (old_flags < 0)
		যাও unwind;
	যদি (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		যাও unwind;

	ফেরত fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	ফেরত -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
পূর্ণসংখ্যা xmkstemp(অক্ষর **tmpname, ধ্রুবক অক্ষর *dir, ধ্রুবক অক্ষর *prefix)
{
	অক্ষর *localtmp;
	ধ্রুবক অক্ষর *tmpenv;
	mode_t old_mode;
	পূর্ণসংখ্যা fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	যদি (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	যদি (rc < 0)
		ফেরত -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	যদি (fd == -1) {
		মুক্ত(localtmp);
		localtmp = ফাঁকা;
	}
	*tmpname = localtmp;
	ফেরত fd;
}

#ifdef F_DUPFD_CLOEXEC
পূর্ণসংখ্যা dup_fd_cloexec(পূর্ণসংখ্যা oldfd, পূর্ণসংখ্যা lowfd)
#অন্যথা
পূর্ণসংখ্যা dup_fd_cloexec(পূর্ণসংখ্যা oldfd, পূর্ণসংখ্যা lowfd  __attribute__((__unused__)))
#endif
{
	পূর্ণসংখ্যা fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	যদি (fd >= 0)
		ফেরত fd;
#endif

	fd = dup(oldfd);
	যদি (fd < 0)
		ফেরত fd;

	flags = fcntl(fd, F_GETFD);
	যদি (flags < 0)
		যাও unwind;
	যদি (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		যাও unwind;

	ফেরত fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ফেরত -1;
}

/*
 * portable getdtablesize()
 */
unsigned পূর্ণসংখ্যা get_fd_tabsize(শূন্য)
{
	পূর্ণসংখ্যা m;

#যদি defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#নতুবা_যদি defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	কাঠামো rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#নতুবা_যদি defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#অন্যথা
	m = OPEN_MAX;
#endif
	ফেরত m;
}

শূন্য ul_close_all_fds(unsigned পূর্ণসংখ্যা first, unsigned পূর্ণসংখ্যা last)
{
	কাঠামো dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	যদি (dir) {
		যখন ((d = xreaddir(dir))) {
			অক্ষর *end;
			unsigned পূর্ণসংখ্যা fd;
			পূর্ণসংখ্যা dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			যদি (errno || end == d->d_name || !end || *end)
				চালু;
			dfd = dirfd(dir);
			যদি (dfd < 0)
				চালু;
			যদি ((unsigned পূর্ণসংখ্যা)dfd == fd)
				চালু;
			যদি (fd < first || last < fd)
				চালু;
			close(fd);
		}
		closedir(dir);
	} অন্যথা {
		unsigned fd, tbsz = get_fd_tabsize();

		জন্য (fd = 0; fd < tbsz; fd++) {
			যদি (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
অক্ষর *ul_restricted_path_oper(ধ্রুবক অক্ষর *path,
		  পূর্ণসংখ্যা (*oper)(ধ্রুবক অক্ষর *path, অক্ষর **ফলাফল, শূন্য *তথ্য),
		  শূন্য *তথ্য)
{
	অক্ষর *ফলাফল = ফাঁকা;
	পূর্ণসংখ্যা errsv = 0;
	পূর্ণসংখ্যা pipes[2];
	ssize_t len;
	pid_t pid;

	যদি (!path || !*path)
		ফেরত ফাঁকা;

	যদি (pipe(pipes) != 0)
		ফেরত ফাঁকা;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	নির্বাচন (pid) {
	ক্ষেত্র -1:
		close(pipes[0]);
		close(pipes[1]);
		ফেরত ফাঁকা;			/* fork error */
	ক্ষেত্র 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		যদি (drop_permissions() != 0)
			ফলাফল = ফাঁকা;	/* failed */
		অন্যথা
			oper(path, &ফলাফল, তথ্য);

		len = ফলাফল ? (ssize_t) strlen(ফলাফল) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (অক্ষর *) &len, আকার(len));
		যদি (ফলাফল)
			ul_write_all(pipes[1], ফলাফল, len);
		_exit(0);
	পূর্বনির্ধারিত:
		বিরতি;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	যদি (ul_read_all(pipes[0], (অক্ষর *) &len, আকার(len)) != আকার(len))
		যাও done;
	যদি (len < 0) {
		errsv = -len;
		যাও done;
	}

	ফলাফল = malloc(len + 1);
	যদি (!ফলাফল) {
		errsv = ENOMEM;
		যাও done;
	}
	/* read path */
	যদি (ul_read_all(pipes[0], ফলাফল, len) != len) {
		errsv = errno;
		যাও done;
	}
	ফলাফল[len] = '\0';
done:
	যদি (errsv) {
		মুক্ত(ফলাফল);
		ফলাফল = ফাঁকা;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_ফলাফল( waitpid(pid, ফাঁকা, 0) );

	errno = errsv;
	ফেরত ফলাফল;

}

পূর্ণসংখ্যা ul_mkdir_p(ধ্রুবক অক্ষর *path, mode_t mode)
{
	অক্ষর *p, *dir;
	পূর্ণসংখ্যা rc = 0;

	যদি (!path || !*path)
		ফেরত -EINVAL;

	dir = p = strdup(path);
	যদি (!dir)
		ফেরত -ENOMEM;

	যদি (*p == '/')
		p++;

	যখন (p && *p) {
		অক্ষর *e = strchr(p, '/');
		যদি (e)
			*e = '\0';
		যদি (*p) {
			rc = mkdir(dir, mode);
			যদি (rc && errno != EEXIST)
				বিরতি;
			rc = 0;
		}
		যদি (!e)
			বিরতি;
		*e = '/';
		p = e + 1;
	}

	মুক্ত(dir);
	ফেরত rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
অক্ষর *stripoff_last_component(অক্ষর *path)
{
	অক্ষর *p = path ? strrchr(path, '/') : ফাঁকা;

	যদি (!p)
		ফেরত ফাঁকা;
	*p = '\0';
	ফেরত p + 1;
}

স্থির পূর্ণসংখ্যা copy_file_simple(পূর্ণসংখ্যা from, পূর্ণসংখ্যা to)
{
	ssize_t nr;
	অক্ষর buf[BUFSIZ];

	যখন ((nr = ul_read_all(from, buf, আকার(buf))) > 0)
		যদি (ul_write_all(to, buf, nr) == -1)
			ফেরত UL_COPY_WRITE_ERROR;
	যদি (nr < 0)
		ফেরত UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, আকার(buf));
#endif
	ফেরত 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
পূর্ণসংখ্যা ul_copy_file(পূর্ণসংখ্যা from, পূর্ণসংখ্যা to)
{
#ifdef HAVE_SENDFILE
	কাঠামো stat st;
	ssize_t nw;

	যদি (fstat(from, &st) == -1)
		ফেরত UL_COPY_READ_ERROR;
	যদি (!S_ISREG(st.st_mode))
		ফেরত copy_file_simple(from, to);
	যদি (ul_sendfile_all(to, from, ফাঁকা, st.st_size) < 0)
		ফেরত copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	যখন ((nw = ul_sendfile_all(to, from, ফাঁকা, 16*1024*1024)) != 0)
		যদি (nw < 0)
			ফেরত copy_file_simple(from, to);
	ফেরত 0;
#অন্যথা
	ফেরত copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
অক্ষর *ul_fd_mkpath(অক্ষর *buf, size_t bufsz, পূর্ণসংখ্যা fd)
{
	পূর্ণসংখ্যা len;

	যদি (fd < 0) {
		errno = EBADF;
		ফেরত ফাঁকা;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	যদি (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ফেরত ফাঁকা;
	}

	ফেরত buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
অক্ষর *ul_fd_get_path(পূর্ণসংখ্যা fd)
{
	ssize_t ssz;
	অক্ষর buf[PATH_MAX];
	অক্ষর fdpath[UL_FDPATH_BUFSIZ];

	যদি (!ul_fd_mkpath(fdpath, আকার(fdpath), fd))
		ফেরত ফাঁকা;

	ssz = readlink(fdpath, buf, আকার(buf));
	যদি (ssz < 0)
		ফেরত ফাঁকা;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	যদি ((size_t) ssz >= আকার(buf)) {
		errno = ENAMETOOLONG;
		ফেরত ফাঁকা;
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
	যদি (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		ফেরত ফাঁকা;
	}

	ফেরত strdup(buf);
}

পূর্ণসংখ্যা ul_reopen(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা flags)
{
	অক্ষর *path = ul_fd_get_path(fd);
	পূর্ণসংখ্যা ret;

	যদি (!path)
		ফেরত -errno;

	ret = open(path, flags);
	মুক্ত(path);

	ফেরত ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
অক্ষর *ul_basename(অক্ষর *path)
{
	অক্ষর *p;

	যদি (!path || !*path)
		ফেরত (অক্ষর *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	যদি (!p)
		ফেরত path;		/* no '/', return original */

	যদি (*(p + 1) != '\0')
		ফেরত p + 1;		/* begin of the name */

	যখন (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	যদি (p > path) {
		*p-- = '\0';
		যখন (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} অন্যথা যখন (*(p + 1) != '\0')
		++p;

	ফেরত p;
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
FILE *fopen_at_no_link(পূর্ণসংখ্যা dir, ধ্রুবক অক্ষর *filename,
                             পূর্ণসংখ্যা flags, mode_t perm, ধ্রুবক অক্ষর *mode)
{
	FILE *fp;
	পূর্ণসংখ্যা fd;
	কাঠামো stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	যদি (fd < 0)
		ফেরত ফাঁকা;

	যদি (fstat(fd, &st)) {
		close(fd);
		ফেরত ফাঁকা;
	}

	যদি (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ফেরত ফাঁকা;
	}

	যদি ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ফেরত ফাঁকা;
	}

	fp = fdopen(fd, mode);
	যদি (!fp)
		close(fd);
	ফেরত fp;
}
#endif /* HAVE_OPENAT */

#যদি defined(SYS_openat2)
পূর্ণসংখ্যা ul_openat_resolve(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা flags,
		      mode_t mode, unsigned long long resolve)
{
	কাঠামো open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	ফেরত syscall(SYS_openat2, dirfd, path, &how, আকার(how));
}
#অন্যথা
পূর্ণসংখ্যা ul_openat_resolve(
		পূর্ণসংখ্যা dirfd __attribute__((__unused__)),
		ধ্রুবক অক্ষর *path __attribute__((__unused__)),
		পূর্ণসংখ্যা flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ফেরত -1;
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
স্থির পূর্ণসংখ্যা open_no_symlinks_fallback(ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা flags, mode_t mode)
{
	অক্ষর *abspath = ফাঁকা, *kpath = ফাঁকা;
	কাঠামো stat st;
	পূর্ণসংখ্যা fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	যদি (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		ফেরত -1;
	}

	যদি (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		যদি (!abspath)
			ফেরত -1;
	}

	যদি (flags & O_PATH)
		fd = open(path, flags);
	অন্যথা
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	যদি (fd < 0)
		যাও fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	যদি (fstat(fd, &st) != 0)
		যাও fail;
	যদি (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		যাও fail;
	}

	kpath = ul_fd_get_path(fd);
	যদি (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		যাও fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	যদি (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		যাও fail;
	}

	যদি (!(flags & O_PATH)) {
		অক্ষর fdpath[UL_FDPATH_BUFSIZ];
		পূর্ণসংখ্যা nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		যদি (ul_fd_mkpath(fdpath, আকার(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		যদি (nfd < 0)
			যাও fail;
		close(fd);
		fd = nfd;
	}

	মুক্ত(abspath);
	মুক্ত(kpath);
	ফেরত fd;
fail:
	errsv = errno;
	মুক্ত(abspath);
	মুক্ত(kpath);
	যদি (fd >= 0)
		close(fd);
	errno = errsv;
	ফেরত -1;
}
#অন্যথা /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
স্থির পূর্ণসংখ্যা open_no_symlinks_fallback(
			ধ্রুবক অক্ষর *path __attribute__((__unused__)),
			পূর্ণসংখ্যা flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	ফেরত -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
পূর্ণসংখ্যা ul_open_no_symlinks(ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা flags, mode_t mode)
{
	পূর্ণসংখ্যা fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	যদি (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	ফেরত fd;
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
পূর্ণসংখ্যা ul_safe_statx(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা flags,
		  unsigned পূর্ণসংখ্যা mask, কাঠামো statx *stx)
{
	প্রতিজ্ঞা(stx);

	memset(stx, 0, আকার(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	যদি (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	যদি (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		ফেরত -errno;

	ফেরত 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
শূন্য ul_statx_to_stat(ধ্রুবক কাঠামো statx *stx, কাঠামো stat *st)
{
	memset(st, 0, আকার(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	যদি (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	যদি (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	যদি (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	যদি (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	যদি (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	যদি (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	যদি (stx->stx_mask & STATX_আকার)
		st->st_size = stx->stx_আকার;
	যদি (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	যদি (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	যদি (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	যদি (stx->stx_mask & STATX_CTIME) {
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
পূর্ণসংখ্যা ul_safe_stat(ধ্রুবক অক্ষর *target, কাঠামো stat *st,
		 পূর্ণসংখ্যা nofollow __attribute__((__unused__)),
		 unsigned পূর্ণসংখ্যা mask __attribute__((__unused__)),
		 unsigned পূর্ণসংখ্যা *retmask)
{
	প্রতিজ্ঞা(target);
	প্রতিজ্ঞা(st);

	memset(st, 0, আকার(কাঠামো stat));
	যদি (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		কাঠামো statx stx = { 0 };
		পূর্ণসংখ্যা rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		যদি (rc)
			ফেরত rc;

		ul_statx_to_stat(&stx, st);

		যদি (retmask)
			*retmask = stx.stx_mask;

		যদি ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ফেরত -EOPNOTSUPP;
		}

		ফেরত 0;
	}
#অন্যথা
	errno = ENOSYS;
	ফেরত -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	যদি (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	যদি (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		অক্ষর *tmpname = ফাঁকা;

		f = xfmkstemp(&tmpname, ফাঁকা, "test");
		unlink(tmpname);
		মুক্ত(tmpname);
		fclose(f);

	} অন্যথা যদি (strcmp(argv[1], "--close-fds") == 0) {
		ignore_ফলাফল( dup(STDIN_FILENO) );
		ignore_ফলাফল( dup(STDIN_FILENO) );
		ignore_ফলাফল( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		যদি (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} অন্যথা যদি (strcmp(argv[1], "--copy-file") == 0) {
		পূর্ণসংখ্যা ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		যদি (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		অন্যথা যদি (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} অন্যথা যদি (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		পূর্ণসংখ্যা flags = O_PATH | O_CLOEXEC;
#অন্যথা
		পূর্ণসংখ্যা flags = O_RDONLY | O_CLOEXEC;
#endif
		পূর্ণসংখ্যা fallback = 0;
		অক্ষর *name;
		পূর্ণসংখ্যা fd, i;

		যদি (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		জন্য (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			যদি (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			অন্যথা যদি (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		যদি (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		অন্যথা
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		যদি (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		মুক্ত(name);
		close(fd);

	} অন্যথা যদি (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned পূর্ণসংখ্যা mask = 0, retmask = 0;
		পূর্ণসংখ্যা nofollow = 0;
		কাঠামো stat st, ref;
		পূর্ণসংখ্যা rc, i;

		যদি (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		জন্য (i = 3; i < argc; i++) {
			যদি (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			অন্যথা যদি (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		যদি (rc)
			ফেরত EXIT_FAILURE;

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
		যদি (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned পূর্ণসংখ্যা emask = mask | UL_STATX_ESSENTIAL;

# define ফলাফল_2(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			যদি (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", ফলাফল_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			যদি (emask & STATX_MODE)
				printf(" st_mode: %s\n", ফলাফল_2(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			যদি (emask & STATX_INO)
				printf("  st_ino: %s\n",
					ফলাফল_2(STATX_INO, st.st_ino == ref.st_ino));
			যদি (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					ফলাফল_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			যদি (emask & STATX_UID)
				printf("  st_uid: %s\n",
					ফলাফল_2(STATX_UID, st.st_uid == ref.st_uid));
			যদি (emask & STATX_GID)
				printf("  st_gid: %s\n",
					ফলাফল_2(STATX_GID, st.st_gid == ref.st_gid));
			যদি (emask & STATX_আকার)
				printf(" st_size: %s\n",
					ফলাফল_2(STATX_আকার, st.st_size == ref.st_size));
# undef ফলাফল_2
		}
#endif
	}
	ফেরত EXIT_SUCCESS;
}
#endif
