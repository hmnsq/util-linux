/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>
#ضمن <sys/time.h>
#ضمن <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#ضمن <string.h>
#ضمن <sys/wait.h>
#ضمن <fcntl.h>
#ضمن <errno.h>

#ضمن "c.h"
#ضمن "all-io.h"
#ضمن "canonicalize.h"
#ضمن "fileutils.h"
#ضمن "pathnames.h"
#ضمن "strutils.h"

صحيح mkstemp_cloexec(حرف *template)
{
#ifdef HAVE_MKOSTEMP
	أرجع mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#وإلا
	صحيح fd, old_flags, errno_save;

	fd = mkstemp(template);
	𐡄𐡍 (fd < 0)
		أرجع fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	𐡄𐡍 (old_flags < 0)
		اذهب unwind;
	𐡄𐡍 (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		اذهب unwind;

	أرجع fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	أرجع -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
صحيح xmkstemp(حرف **tmpname, ثابت حرف *dir, ثابت حرف *prefix)
{
	حرف *localtmp;
	ثابت حرف *tmpenv;
	mode_t old_mode;
	صحيح fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	𐡄𐡍 (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	𐡄𐡍 (rc < 0)
		أرجع -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	𐡄𐡍 (fd == -1) {
		حرر(localtmp);
		localtmp = عدم;
	}
	*tmpname = localtmp;
	أرجع fd;
}

#ifdef F_DUPFD_CLOEXEC
صحيح dup_fd_cloexec(صحيح oldfd, صحيح lowfd)
#وإلا
صحيح dup_fd_cloexec(صحيح oldfd, صحيح lowfd  __attribute__((__unused__)))
#endif
{
	صحيح fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	𐡄𐡍 (fd >= 0)
		أرجع fd;
#endif

	fd = dup(oldfd);
	𐡄𐡍 (fd < 0)
		أرجع fd;

	flags = fcntl(fd, F_GETFD);
	𐡄𐡍 (flags < 0)
		اذهب unwind;
	𐡄𐡍 (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		اذهب unwind;

	أرجع fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	أرجع -1;
}

/*
 * portable getdtablesize()
 */
unsigned صحيح get_fd_tabsize(فراغ)
{
	صحيح m;

#𐡄𐡍 defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#وإلاإذا defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	بنية rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#وإلاإذا defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#وإلا
	m = OPEN_MAX;
#endif
	أرجع m;
}

فراغ ul_close_all_fds(unsigned صحيح first, unsigned صحيح last)
{
	بنية dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	𐡄𐡍 (dir) {
		طالما ((d = xreaddir(dir))) {
			حرف *end;
			unsigned صحيح fd;
			صحيح dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			𐡄𐡍 (errno || end == d->d_name || !end || *end)
				استمر;
			dfd = dirfd(dir);
			𐡄𐡍 (dfd < 0)
				استمر;
			𐡄𐡍 ((unsigned صحيح)dfd == fd)
				استمر;
			𐡄𐡍 (fd < first || last < fd)
				استمر;
			close(fd);
		}
		closedir(dir);
	} وإلا {
		unsigned fd, tbsz = get_fd_tabsize();

		لكل (fd = 0; fd < tbsz; fd++) {
			𐡄𐡍 (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
حرف *ul_restricted_path_oper(ثابت حرف *path,
		  صحيح (*oper)(ثابت حرف *path, حرف **result, فراغ *data),
		  فراغ *data)
{
	حرف *result = عدم;
	صحيح errsv = 0;
	صحيح pipes[2];
	ssize_t len;
	pid_t pid;

	𐡄𐡍 (!path || !*path)
		أرجع عدم;

	𐡄𐡍 (pipe(pipes) != 0)
		أرجع عدم;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	بدل (pid) {
	حالة -1:
		close(pipes[0]);
		close(pipes[1]);
		أرجع عدم;			/* fork error */
	حالة 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		𐡄𐡍 (drop_permissions() != 0)
			result = عدم;	/* failed */
		وإلا
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (حرف *) &len, حجم(len));
		𐡄𐡍 (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	افتراضي:
		اكسر;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	𐡄𐡍 (ul_read_all(pipes[0], (حرف *) &len, حجم(len)) != حجم(len))
		اذهب done;
	𐡄𐡍 (len < 0) {
		errsv = -len;
		اذهب done;
	}

	result = malloc(len + 1);
	𐡄𐡍 (!result) {
		errsv = ENOMEM;
		اذهب done;
	}
	/* read path */
	𐡄𐡍 (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		اذهب done;
	}
	result[len] = '\0';
done:
	𐡄𐡍 (errsv) {
		حرر(result);
		result = عدم;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, عدم, 0) );

	errno = errsv;
	أرجع result;

}

صحيح ul_mkdir_p(ثابت حرف *path, mode_t mode)
{
	حرف *p, *dir;
	صحيح rc = 0;

	𐡄𐡍 (!path || !*path)
		أرجع -EINVAL;

	dir = p = strdup(path);
	𐡄𐡍 (!dir)
		أرجع -ENOMEM;

	𐡄𐡍 (*p == '/')
		p++;

	طالما (p && *p) {
		حرف *e = strchr(p, '/');
		𐡄𐡍 (e)
			*e = '\0';
		𐡄𐡍 (*p) {
			rc = mkdir(dir, mode);
			𐡄𐡍 (rc && errno != EEXIST)
				اكسر;
			rc = 0;
		}
		𐡄𐡍 (!e)
			اكسر;
		*e = '/';
		p = e + 1;
	}

	حرر(dir);
	أرجع rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
حرف *stripoff_last_component(حرف *path)
{
	حرف *p = path ? strrchr(path, '/') : عدم;

	𐡄𐡍 (!p)
		أرجع عدم;
	*p = '\0';
	أرجع p + 1;
}

ساكن صحيح copy_file_simple(صحيح from, صحيح to)
{
	ssize_t nr;
	حرف buf[BUFSIZ];

	طالما ((nr = ul_read_all(from, buf, حجم(buf))) > 0)
		𐡄𐡍 (ul_write_all(to, buf, nr) == -1)
			أرجع UL_COPY_WRITE_ERROR;
	𐡄𐡍 (nr < 0)
		أرجع UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, حجم(buf));
#endif
	أرجع 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
صحيح ul_copy_file(صحيح from, صحيح to)
{
#ifdef HAVE_SENDFILE
	بنية stat st;
	ssize_t nw;

	𐡄𐡍 (fstat(from, &st) == -1)
		أرجع UL_COPY_READ_ERROR;
	𐡄𐡍 (!S_ISREG(st.st_mode))
		أرجع copy_file_simple(from, to);
	𐡄𐡍 (ul_sendfile_all(to, from, عدم, st.st_size) < 0)
		أرجع copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	طالما ((nw = ul_sendfile_all(to, from, عدم, 16*1024*1024)) != 0)
		𐡄𐡍 (nw < 0)
			أرجع copy_file_simple(from, to);
	أرجع 0;
#وإلا
	أرجع copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
حرف *ul_fd_mkpath(حرف *buf, size_t bufsz, صحيح fd)
{
	صحيح len;

	𐡄𐡍 (fd < 0) {
		errno = EBADF;
		أرجع عدم;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	𐡄𐡍 (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
حرف *ul_fd_get_path(صحيح fd)
{
	ssize_t ssz;
	حرف buf[PATH_MAX];
	حرف fdpath[UL_FDPATH_BUFSIZ];

	𐡄𐡍 (!ul_fd_mkpath(fdpath, حجم(fdpath), fd))
		أرجع عدم;

	ssz = readlink(fdpath, buf, حجم(buf));
	𐡄𐡍 (ssz < 0)
		أرجع عدم;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	𐡄𐡍 ((size_t) ssz >= حجم(buf)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
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
	𐡄𐡍 (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		أرجع عدم;
	}

	أرجع strdup(buf);
}

صحيح ul_reopen(صحيح fd, صحيح flags)
{
	حرف *path = ul_fd_get_path(fd);
	صحيح ret;

	𐡄𐡍 (!path)
		أرجع -errno;

	ret = open(path, flags);
	حرر(path);

	أرجع ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
حرف *ul_basename(حرف *path)
{
	حرف *p;

	𐡄𐡍 (!path || !*path)
		أرجع (حرف *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	𐡄𐡍 (!p)
		أرجع path;		/* no '/', return original */

	𐡄𐡍 (*(p + 1) != '\0')
		أرجع p + 1;		/* begin of the name */

	طالما (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	𐡄𐡍 (p > path) {
		*p-- = '\0';
		طالما (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} وإلا طالما (*(p + 1) != '\0')
		++p;

	أرجع p;
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
FILE *fopen_at_no_link(صحيح dir, ثابت حرف *filename,
                             صحيح flags, mode_t perm, ثابت حرف *mode)
{
	FILE *fp;
	صحيح fd;
	بنية stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	𐡄𐡍 (fd < 0)
		أرجع عدم;

	𐡄𐡍 (fstat(fd, &st)) {
		close(fd);
		أرجع عدم;
	}

	𐡄𐡍 (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		أرجع عدم;
	}

	𐡄𐡍 ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		أرجع عدم;
	}

	fp = fdopen(fd, mode);
	𐡄𐡍 (!fp)
		close(fd);
	أرجع fp;
}
#endif /* HAVE_OPENAT */

#𐡄𐡍 defined(SYS_openat2)
صحيح ul_openat_resolve(صحيح dirfd, ثابت حرف *path, صحيح flags,
		      mode_t mode, unsigned long long resolve)
{
	بنية open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	أرجع syscall(SYS_openat2, dirfd, path, &how, حجم(how));
}
#وإلا
صحيح ul_openat_resolve(
		صحيح dirfd __attribute__((__unused__)),
		ثابت حرف *path __attribute__((__unused__)),
		صحيح flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
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
ساكن صحيح open_no_symlinks_fallback(ثابت حرف *path, صحيح flags, mode_t mode)
{
	حرف *abspath = عدم, *kpath = عدم;
	بنية stat st;
	صحيح fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	𐡄𐡍 (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		أرجع -1;
	}

	𐡄𐡍 (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		𐡄𐡍 (!abspath)
			أرجع -1;
	}

	𐡄𐡍 (flags & O_PATH)
		fd = open(path, flags);
	وإلا
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	𐡄𐡍 (fd < 0)
		اذهب fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	𐡄𐡍 (fstat(fd, &st) != 0)
		اذهب fail;
	𐡄𐡍 (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		اذهب fail;
	}

	kpath = ul_fd_get_path(fd);
	𐡄𐡍 (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		اذهب fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	𐡄𐡍 (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		اذهب fail;
	}

	𐡄𐡍 (!(flags & O_PATH)) {
		حرف fdpath[UL_FDPATH_BUFSIZ];
		صحيح nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		𐡄𐡍 (ul_fd_mkpath(fdpath, حجم(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		𐡄𐡍 (nfd < 0)
			اذهب fail;
		close(fd);
		fd = nfd;
	}

	حرر(abspath);
	حرر(kpath);
	أرجع fd;
fail:
	errsv = errno;
	حرر(abspath);
	حرر(kpath);
	𐡄𐡍 (fd >= 0)
		close(fd);
	errno = errsv;
	أرجع -1;
}
#وإلا /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ساكن صحيح open_no_symlinks_fallback(
			ثابت حرف *path __attribute__((__unused__)),
			صحيح flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
صحيح ul_open_no_symlinks(ثابت حرف *path, صحيح flags, mode_t mode)
{
	صحيح fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	𐡄𐡍 (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	أرجع fd;
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
صحيح ul_safe_statx(صحيح dirfd, ثابت حرف *path, صحيح flags,
		  unsigned صحيح mask, بنية statx *stx)
{
	أكد(stx);

	memset(stx, 0, حجم(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	𐡄𐡍 (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	𐡄𐡍 (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		أرجع -errno;

	أرجع 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
فراغ ul_statx_to_stat(ثابت بنية statx *stx, بنية stat *st)
{
	memset(st, 0, حجم(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	𐡄𐡍 (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	𐡄𐡍 (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	𐡄𐡍 (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	𐡄𐡍 (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	𐡄𐡍 (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	𐡄𐡍 (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	𐡄𐡍 (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	𐡄𐡍 (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	𐡄𐡍 (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	𐡄𐡍 (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	𐡄𐡍 (stx->stx_mask & STATX_CTIME) {
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
صحيح ul_safe_stat(ثابت حرف *target, بنية stat *st,
		 صحيح nofollow __attribute__((__unused__)),
		 unsigned صحيح mask __attribute__((__unused__)),
		 unsigned صحيح *retmask)
{
	أكد(target);
	أكد(st);

	memset(st, 0, حجم(بنية stat));
	𐡄𐡍 (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		بنية statx stx = { 0 };
		صحيح rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		𐡄𐡍 (rc)
			أرجع rc;

		ul_statx_to_stat(&stx, st);

		𐡄𐡍 (retmask)
			*retmask = stx.stx_mask;

		𐡄𐡍 ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			أرجع -EOPNOTSUPP;
		}

		أرجع 0;
	}
#وإلا
	errno = ENOSYS;
	أرجع -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
صحيح main(صحيح argc, حرف *argv[])
{
	𐡄𐡍 (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	𐡄𐡍 (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		حرف *tmpname = عدم;

		f = xfmkstemp(&tmpname, عدم, "test");
		unlink(tmpname);
		حرر(tmpname);
		fclose(f);

	} وإلا 𐡄𐡍 (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		𐡄𐡍 (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} وإلا 𐡄𐡍 (strcmp(argv[1], "--copy-file") == 0) {
		صحيح ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		𐡄𐡍 (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		وإلا 𐡄𐡍 (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} وإلا 𐡄𐡍 (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		صحيح flags = O_PATH | O_CLOEXEC;
#وإلا
		صحيح flags = O_RDONLY | O_CLOEXEC;
#endif
		صحيح fallback = 0;
		حرف *name;
		صحيح fd, i;

		𐡄𐡍 (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		لكل (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			𐡄𐡍 (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			وإلا 𐡄𐡍 (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		𐡄𐡍 (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		وإلا
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		𐡄𐡍 (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		حرر(name);
		close(fd);

	} وإلا 𐡄𐡍 (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned صحيح mask = 0, retmask = 0;
		صحيح nofollow = 0;
		بنية stat st, ref;
		صحيح rc, i;

		𐡄𐡍 (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		لكل (i = 3; i < argc; i++) {
			𐡄𐡍 (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			وإلا 𐡄𐡍 (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		𐡄𐡍 (rc)
			أرجع EXIT_FAILURE;

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
		𐡄𐡍 (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned صحيح emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			𐡄𐡍 (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			𐡄𐡍 (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			𐡄𐡍 (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			𐡄𐡍 (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			𐡄𐡍 (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			𐡄𐡍 (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			𐡄𐡍 (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	أرجع EXIT_SUCCESS;
}
#endif
