/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <unistd.h>
#شامل <sys/time.h>
#شامل <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#شامل <string.h>
#شامل <sys/wait.h>
#شامل <fcntl.h>
#شامل <errno.h>

#شامل "c.h"
#شامل "all-io.h"
#شامل "canonicalize.h"
#شامل "fileutils.h"
#شامل "pathnames.h"
#شامل "strutils.h"

صحیح mkstemp_cloexec(نویسه *template)
{
#ifdef HAVE_MKOSTEMP
	بازگشت mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#وگرنه
	صحیح fd, old_flags, errno_save;

	fd = mkstemp(template);
	اگر (fd < 0)
		بازگشت fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	اگر (old_flags < 0)
		برو unwind;
	اگر (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		برو unwind;

	بازگشت fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	بازگشت -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
صحیح xmkstemp(نویسه **tmpname, ثابت نویسه *dir, ثابت نویسه *prefix)
{
	نویسه *localtmp;
	ثابت نویسه *tmpenv;
	mode_t old_mode;
	صحیح fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	اگر (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	اگر (rc < 0)
		بازگشت -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	اگر (fd == -1) {
		آزاد(localtmp);
		localtmp = پوچ;
	}
	*tmpname = localtmp;
	بازگشت fd;
}

#ifdef F_DUPFD_CLOEXEC
صحیح dup_fd_cloexec(صحیح oldfd, صحیح lowfd)
#وگرنه
صحیح dup_fd_cloexec(صحیح oldfd, صحیح lowfd  __attribute__((__unused__)))
#endif
{
	صحیح fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	اگر (fd >= 0)
		بازگشت fd;
#endif

	fd = dup(oldfd);
	اگر (fd < 0)
		بازگشت fd;

	flags = fcntl(fd, F_GETFD);
	اگر (flags < 0)
		برو unwind;
	اگر (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		برو unwind;

	بازگشت fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	بازگشت -1;
}

/*
 * portable getdtablesize()
 */
unsigned صحیح get_fd_tabsize(تهی)
{
	صحیح m;

#اگر defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#وگرنهاگر defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ساختار rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#وگرنهاگر defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#وگرنه
	m = OPEN_MAX;
#endif
	بازگشت m;
}

تهی ul_close_all_fds(unsigned صحیح first, unsigned صحیح last)
{
	ساختار dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	اگر (dir) {
		وقتی ((d = xreaddir(dir))) {
			نویسه *end;
			unsigned صحیح fd;
			صحیح dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			اگر (errno || end == d->d_name || !end || *end)
				ادامه;
			dfd = dirfd(dir);
			اگر (dfd < 0)
				ادامه;
			اگر ((unsigned صحیح)dfd == fd)
				ادامه;
			اگر (fd < first || last < fd)
				ادامه;
			close(fd);
		}
		closedir(dir);
	} وگرنه {
		unsigned fd, tbsz = get_fd_tabsize();

		برای (fd = 0; fd < tbsz; fd++) {
			اگر (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
نویسه *ul_restricted_path_oper(ثابت نویسه *path,
		  صحیح (*oper)(ثابت نویسه *path, نویسه **result, تهی *data),
		  تهی *data)
{
	نویسه *result = پوچ;
	صحیح errsv = 0;
	صحیح pipes[2];
	ssize_t len;
	pid_t pid;

	اگر (!path || !*path)
		بازگشت پوچ;

	اگر (pipe(pipes) != 0)
		بازگشت پوچ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	انتخاب (pid) {
	حالت -1:
		close(pipes[0]);
		close(pipes[1]);
		بازگشت پوچ;			/* fork error */
	حالت 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		اگر (drop_permissions() != 0)
			result = پوچ;	/* failed */
		وگرنه
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (نویسه *) &len, اندازه(len));
		اگر (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	پیشفرض:
		بشکن;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	اگر (ul_read_all(pipes[0], (نویسه *) &len, اندازه(len)) != اندازه(len))
		برو done;
	اگر (len < 0) {
		errsv = -len;
		برو done;
	}

	result = malloc(len + 1);
	اگر (!result) {
		errsv = ENOMEM;
		برو done;
	}
	/* read path */
	اگر (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		برو done;
	}
	result[len] = '\0';
done:
	اگر (errsv) {
		آزاد(result);
		result = پوچ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, پوچ, 0) );

	errno = errsv;
	بازگشت result;

}

صحیح ul_mkdir_p(ثابت نویسه *path, mode_t mode)
{
	نویسه *p, *dir;
	صحیح rc = 0;

	اگر (!path || !*path)
		بازگشت -EINVAL;

	dir = p = strdup(path);
	اگر (!dir)
		بازگشت -ENOMEM;

	اگر (*p == '/')
		p++;

	وقتی (p && *p) {
		نویسه *e = strchr(p, '/');
		اگر (e)
			*e = '\0';
		اگر (*p) {
			rc = mkdir(dir, mode);
			اگر (rc && errno != EEXIST)
				بشکن;
			rc = 0;
		}
		اگر (!e)
			بشکن;
		*e = '/';
		p = e + 1;
	}

	آزاد(dir);
	بازگشت rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
نویسه *stripoff_last_component(نویسه *path)
{
	نویسه *p = path ? strrchr(path, '/') : پوچ;

	اگر (!p)
		بازگشت پوچ;
	*p = '\0';
	بازگشت p + 1;
}

ایستا صحیح copy_file_simple(صحیح from, صحیح to)
{
	ssize_t nr;
	نویسه buf[BUFSIZ];

	وقتی ((nr = ul_read_all(from, buf, اندازه(buf))) > 0)
		اگر (ul_write_all(to, buf, nr) == -1)
			بازگشت UL_COPY_WRITE_ERROR;
	اگر (nr < 0)
		بازگشت UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, اندازه(buf));
#endif
	بازگشت 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
صحیح ul_copy_file(صحیح from, صحیح to)
{
#ifdef HAVE_SENDFILE
	ساختار stat st;
	ssize_t nw;

	اگر (fstat(from, &st) == -1)
		بازگشت UL_COPY_READ_ERROR;
	اگر (!S_ISREG(st.st_mode))
		بازگشت copy_file_simple(from, to);
	اگر (ul_sendfile_all(to, from, پوچ, st.st_size) < 0)
		بازگشت copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	وقتی ((nw = ul_sendfile_all(to, from, پوچ, 16*1024*1024)) != 0)
		اگر (nw < 0)
			بازگشت copy_file_simple(from, to);
	بازگشت 0;
#وگرنه
	بازگشت copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
نویسه *ul_fd_mkpath(نویسه *buf, size_t bufsz, صحیح fd)
{
	صحیح len;

	اگر (fd < 0) {
		errno = EBADF;
		بازگشت پوچ;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	اگر (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		بازگشت پوچ;
	}

	بازگشت buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
نویسه *ul_fd_get_path(صحیح fd)
{
	ssize_t ssz;
	نویسه buf[PATH_MAX];
	نویسه fdpath[UL_FDPATH_BUFSIZ];

	اگر (!ul_fd_mkpath(fdpath, اندازه(fdpath), fd))
		بازگشت پوچ;

	ssz = readlink(fdpath, buf, اندازه(buf));
	اگر (ssz < 0)
		بازگشت پوچ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	اگر ((size_t) ssz >= اندازه(buf)) {
		errno = ENAMETOOLONG;
		بازگشت پوچ;
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
	اگر (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		بازگشت پوچ;
	}

	بازگشت strdup(buf);
}

صحیح ul_reopen(صحیح fd, صحیح flags)
{
	نویسه *path = ul_fd_get_path(fd);
	صحیح ret;

	اگر (!path)
		بازگشت -errno;

	ret = open(path, flags);
	آزاد(path);

	بازگشت ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
نویسه *ul_basename(نویسه *path)
{
	نویسه *p;

	اگر (!path || !*path)
		بازگشت (نویسه *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	اگر (!p)
		بازگشت path;		/* no '/', return original */

	اگر (*(p + 1) != '\0')
		بازگشت p + 1;		/* begin of the name */

	وقتی (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	اگر (p > path) {
		*p-- = '\0';
		وقتی (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} وگرنه وقتی (*(p + 1) != '\0')
		++p;

	بازگشت p;
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
FILE *fopen_at_no_link(صحیح dir, ثابت نویسه *filename,
                             صحیح flags, mode_t perm, ثابت نویسه *mode)
{
	FILE *fp;
	صحیح fd;
	ساختار stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	اگر (fd < 0)
		بازگشت پوچ;

	اگر (fstat(fd, &st)) {
		close(fd);
		بازگشت پوچ;
	}

	اگر (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		بازگشت پوچ;
	}

	اگر ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		بازگشت پوچ;
	}

	fp = fdopen(fd, mode);
	اگر (!fp)
		close(fd);
	بازگشت fp;
}
#endif /* HAVE_OPENAT */

#اگر defined(SYS_openat2)
صحیح ul_openat_resolve(صحیح dirfd, ثابت نویسه *path, صحیح flags,
		      mode_t mode, unsigned long long resolve)
{
	ساختار open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	بازگشت syscall(SYS_openat2, dirfd, path, &how, اندازه(how));
}
#وگرنه
صحیح ul_openat_resolve(
		صحیح dirfd __attribute__((__unused__)),
		ثابت نویسه *path __attribute__((__unused__)),
		صحیح flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	بازگشت -1;
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
ایستا صحیح open_no_symlinks_fallback(ثابت نویسه *path, صحیح flags, mode_t mode)
{
	نویسه *abspath = پوچ, *kpath = پوچ;
	ساختار stat st;
	صحیح fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	اگر (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		بازگشت -1;
	}

	اگر (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		اگر (!abspath)
			بازگشت -1;
	}

	اگر (flags & O_PATH)
		fd = open(path, flags);
	وگرنه
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	اگر (fd < 0)
		برو fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	اگر (fstat(fd, &st) != 0)
		برو fail;
	اگر (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		برو fail;
	}

	kpath = ul_fd_get_path(fd);
	اگر (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		برو fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	اگر (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		برو fail;
	}

	اگر (!(flags & O_PATH)) {
		نویسه fdpath[UL_FDPATH_BUFSIZ];
		صحیح nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		اگر (ul_fd_mkpath(fdpath, اندازه(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		اگر (nfd < 0)
			برو fail;
		close(fd);
		fd = nfd;
	}

	آزاد(abspath);
	آزاد(kpath);
	بازگشت fd;
fail:
	errsv = errno;
	آزاد(abspath);
	آزاد(kpath);
	اگر (fd >= 0)
		close(fd);
	errno = errsv;
	بازگشت -1;
}
#وگرنه /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ایستا صحیح open_no_symlinks_fallback(
			ثابت نویسه *path __attribute__((__unused__)),
			صحیح flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	بازگشت -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
صحیح ul_open_no_symlinks(ثابت نویسه *path, صحیح flags, mode_t mode)
{
	صحیح fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	اگر (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	بازگشت fd;
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
صحیح ul_safe_statx(صحیح dirfd, ثابت نویسه *path, صحیح flags,
		  unsigned صحیح mask, ساختار statx *stx)
{
	أكد(stx);

	memset(stx, 0, اندازه(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	اگر (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	اگر (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		بازگشت -errno;

	بازگشت 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
تهی ul_statx_to_stat(ثابت ساختار statx *stx, ساختار stat *st)
{
	memset(st, 0, اندازه(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	اگر (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	اگر (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	اگر (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	اگر (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	اگر (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	اگر (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	اگر (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	اگر (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	اگر (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	اگر (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	اگر (stx->stx_mask & STATX_CTIME) {
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
صحیح ul_safe_stat(ثابت نویسه *target, ساختار stat *st,
		 صحیح nofollow __attribute__((__unused__)),
		 unsigned صحیح mask __attribute__((__unused__)),
		 unsigned صحیح *retmask)
{
	أكد(target);
	أكد(st);

	memset(st, 0, اندازه(ساختار stat));
	اگر (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		ساختار statx stx = { 0 };
		صحیح rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		اگر (rc)
			بازگشت rc;

		ul_statx_to_stat(&stx, st);

		اگر (retmask)
			*retmask = stx.stx_mask;

		اگر ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			بازگشت -EOPNOTSUPP;
		}

		بازگشت 0;
	}
#وگرنه
	errno = ENOSYS;
	بازگشت -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	اگر (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	اگر (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		نویسه *tmpname = پوچ;

		f = xfmkstemp(&tmpname, پوچ, "test");
		unlink(tmpname);
		آزاد(tmpname);
		fclose(f);

	} وگرنه اگر (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		اگر (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} وگرنه اگر (strcmp(argv[1], "--copy-file") == 0) {
		صحیح ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		اگر (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		وگرنه اگر (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} وگرنه اگر (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		صحیح flags = O_PATH | O_CLOEXEC;
#وگرنه
		صحیح flags = O_RDONLY | O_CLOEXEC;
#endif
		صحیح fallback = 0;
		نویسه *name;
		صحیح fd, i;

		اگر (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		برای (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			اگر (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			وگرنه اگر (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		اگر (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		وگرنه
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		اگر (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		آزاد(name);
		close(fd);

	} وگرنه اگر (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned صحیح mask = 0, retmask = 0;
		صحیح nofollow = 0;
		ساختار stat st, ref;
		صحیح rc, i;

		اگر (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		برای (i = 3; i < argc; i++) {
			اگر (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			وگرنه اگر (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		اگر (rc)
			بازگشت EXIT_FAILURE;

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
		اگر (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned صحیح emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			اگر (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			اگر (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			اگر (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			اگر (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			اگر (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			اگر (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			اگر (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	بازگشت EXIT_SUCCESS;
}
#endif
