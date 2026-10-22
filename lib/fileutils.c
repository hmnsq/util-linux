/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "canonicalize.h"
#அந்தர்பாவ "fileutils.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "strutils.h"

முழு mkstemp_cloexec(எழுத்து *template)
{
#ifdef HAVE_MKOSTEMP
	திரும்பு mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#இல்லையெனில்
	முழு fd, old_flags, errno_save;

	fd = mkstemp(template);
	எனில் (fd < 0)
		திரும்பு fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	எனில் (old_flags < 0)
		செல் unwind;
	எனில் (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		செல் unwind;

	திரும்பு fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	திரும்பு -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
முழு xmkstemp(எழுத்து **tmpname, மாறிலி எழுத்து *dir, மாறிலி எழுத்து *prefix)
{
	எழுத்து *localtmp;
	மாறிலி எழுத்து *tmpenv;
	mode_t old_mode;
	முழு fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	எனில் (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	எனில் (rc < 0)
		திரும்பு -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	எனில் (fd == -1) {
		விடுவி(localtmp);
		localtmp = வெற்று;
	}
	*tmpname = localtmp;
	திரும்பு fd;
}

#ifdef F_DUPFD_CLOEXEC
முழு dup_fd_cloexec(முழு oldfd, முழு lowfd)
#இல்லையெனில்
முழு dup_fd_cloexec(முழு oldfd, முழு lowfd  __attribute__((__unused__)))
#endif
{
	முழு fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	எனில் (fd >= 0)
		திரும்பு fd;
#endif

	fd = dup(oldfd);
	எனில் (fd < 0)
		திரும்பு fd;

	flags = fcntl(fd, F_GETFD);
	எனில் (flags < 0)
		செல் unwind;
	எனில் (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		செல் unwind;

	திரும்பு fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	திரும்பு -1;
}

/*
 * portable getdtablesize()
 */
unsigned முழு get_fd_tabsize(வெறுமை)
{
	முழு m;

#எனில் defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#அல்லதெனில் defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	கட்டமைப்பு rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#அல்லதெனில் defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#இல்லையெனில்
	m = OPEN_MAX;
#endif
	திரும்பு m;
}

வெறுமை ul_close_all_fds(unsigned முழு first, unsigned முழு last)
{
	கட்டமைப்பு dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	எனில் (dir) {
		போது ((d = xreaddir(dir))) {
			எழுத்து *end;
			unsigned முழு fd;
			முழு dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			எனில் (errno || end == d->d_name || !end || *end)
				தொடர்;
			dfd = dirfd(dir);
			எனில் (dfd < 0)
				தொடர்;
			எனில் ((unsigned முழு)dfd == fd)
				தொடர்;
			எனில் (fd < first || last < fd)
				தொடர்;
			close(fd);
		}
		closedir(dir);
	} இல்லையெனில் {
		unsigned fd, tbsz = get_fd_tabsize();

		க்கு (fd = 0; fd < tbsz; fd++) {
			எனில் (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
எழுத்து *ul_restricted_path_oper(மாறிலி எழுத்து *path,
		  முழு (*oper)(மாறிலி எழுத்து *path, எழுத்து **result, வெறுமை *data),
		  வெறுமை *data)
{
	எழுத்து *result = வெற்று;
	முழு errsv = 0;
	முழு pipes[2];
	ssize_t len;
	pid_t pid;

	எனில் (!path || !*path)
		திரும்பு வெற்று;

	எனில் (pipe(pipes) != 0)
		திரும்பு வெற்று;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	தேர்வு (pid) {
	நிலை -1:
		close(pipes[0]);
		close(pipes[1]);
		திரும்பு வெற்று;			/* fork error */
	நிலை 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		எனில் (drop_permissions() != 0)
			result = வெற்று;	/* failed */
		இல்லையெனில்
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (எழுத்து *) &len, அளவு(len));
		எனில் (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	இயல்பு:
		நிறுத்து;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	எனில் (ul_read_all(pipes[0], (எழுத்து *) &len, அளவு(len)) != அளவு(len))
		செல் done;
	எனில் (len < 0) {
		errsv = -len;
		செல் done;
	}

	result = malloc(len + 1);
	எனில் (!result) {
		errsv = ENOMEM;
		செல் done;
	}
	/* read path */
	எனில் (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		செல் done;
	}
	result[len] = '\0';
done:
	எனில் (errsv) {
		விடுவி(result);
		result = வெற்று;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, வெற்று, 0) );

	errno = errsv;
	திரும்பு result;

}

முழு ul_mkdir_p(மாறிலி எழுத்து *path, mode_t mode)
{
	எழுத்து *p, *dir;
	முழு rc = 0;

	எனில் (!path || !*path)
		திரும்பு -EINVAL;

	dir = p = strdup(path);
	எனில் (!dir)
		திரும்பு -ENOMEM;

	எனில் (*p == '/')
		p++;

	போது (p && *p) {
		எழுத்து *e = strchr(p, '/');
		எனில் (e)
			*e = '\0';
		எனில் (*p) {
			rc = mkdir(dir, mode);
			எனில் (rc && errno != EEXIST)
				நிறுத்து;
			rc = 0;
		}
		எனில் (!e)
			நிறுத்து;
		*e = '/';
		p = e + 1;
	}

	விடுவி(dir);
	திரும்பு rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
எழுத்து *stripoff_last_component(எழுத்து *path)
{
	எழுத்து *p = path ? strrchr(path, '/') : வெற்று;

	எனில் (!p)
		திரும்பு வெற்று;
	*p = '\0';
	திரும்பு p + 1;
}

நிலையான முழு copy_file_simple(முழு from, முழு to)
{
	ssize_t nr;
	எழுத்து buf[BUFSIZ];

	போது ((nr = ul_read_all(from, buf, அளவு(buf))) > 0)
		எனில் (ul_write_all(to, buf, nr) == -1)
			திரும்பு UL_COPY_WRITE_ERROR;
	எனில் (nr < 0)
		திரும்பு UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, அளவு(buf));
#endif
	திரும்பு 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
முழு ul_copy_file(முழு from, முழு to)
{
#ifdef HAVE_SENDFILE
	கட்டமைப்பு stat st;
	ssize_t nw;

	எனில் (fstat(from, &st) == -1)
		திரும்பு UL_COPY_READ_ERROR;
	எனில் (!S_ISREG(st.st_mode))
		திரும்பு copy_file_simple(from, to);
	எனில் (ul_sendfile_all(to, from, வெற்று, st.st_size) < 0)
		திரும்பு copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	போது ((nw = ul_sendfile_all(to, from, வெற்று, 16*1024*1024)) != 0)
		எனில் (nw < 0)
			திரும்பு copy_file_simple(from, to);
	திரும்பு 0;
#இல்லையெனில்
	திரும்பு copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
எழுத்து *ul_fd_mkpath(எழுத்து *buf, size_t bufsz, முழு fd)
{
	முழு len;

	எனில் (fd < 0) {
		errno = EBADF;
		திரும்பு வெற்று;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	எனில் (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		திரும்பு வெற்று;
	}

	திரும்பு buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
எழுத்து *ul_fd_get_path(முழு fd)
{
	ssize_t ssz;
	எழுத்து buf[PATH_MAX];
	எழுத்து fdpath[UL_FDPATH_BUFSIZ];

	எனில் (!ul_fd_mkpath(fdpath, அளவு(fdpath), fd))
		திரும்பு வெற்று;

	ssz = readlink(fdpath, buf, அளவு(buf));
	எனில் (ssz < 0)
		திரும்பு வெற்று;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	எனில் ((size_t) ssz >= அளவு(buf)) {
		errno = ENAMETOOLONG;
		திரும்பு வெற்று;
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
	எனில் (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		திரும்பு வெற்று;
	}

	திரும்பு strdup(buf);
}

முழு ul_reopen(முழு fd, முழு flags)
{
	எழுத்து *path = ul_fd_get_path(fd);
	முழு ret;

	எனில் (!path)
		திரும்பு -errno;

	ret = open(path, flags);
	விடுவி(path);

	திரும்பு ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
எழுத்து *ul_basename(எழுத்து *path)
{
	எழுத்து *p;

	எனில் (!path || !*path)
		திரும்பு (எழுத்து *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	எனில் (!p)
		திரும்பு path;		/* no '/', return original */

	எனில் (*(p + 1) != '\0')
		திரும்பு p + 1;		/* begin of the name */

	போது (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	எனில் (p > path) {
		*p-- = '\0';
		போது (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} இல்லையெனில் போது (*(p + 1) != '\0')
		++p;

	திரும்பு p;
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
FILE *fopen_at_no_link(முழு dir, மாறிலி எழுத்து *filename,
                             முழு flags, mode_t perm, மாறிலி எழுத்து *mode)
{
	FILE *fp;
	முழு fd;
	கட்டமைப்பு stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	எனில் (fd < 0)
		திரும்பு வெற்று;

	எனில் (fstat(fd, &st)) {
		close(fd);
		திரும்பு வெற்று;
	}

	எனில் (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		திரும்பு வெற்று;
	}

	எனில் ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		திரும்பு வெற்று;
	}

	fp = fdopen(fd, mode);
	எனில் (!fp)
		close(fd);
	திரும்பு fp;
}
#endif /* HAVE_OPENAT */

#எனில் defined(SYS_openat2)
முழு ul_openat_resolve(முழு dirfd, மாறிலி எழுத்து *path, முழு flags,
		      mode_t mode, unsigned long long resolve)
{
	கட்டமைப்பு open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	திரும்பு syscall(SYS_openat2, dirfd, path, &how, அளவு(how));
}
#இல்லையெனில்
முழு ul_openat_resolve(
		முழு dirfd __attribute__((__unused__)),
		மாறிலி எழுத்து *path __attribute__((__unused__)),
		முழு flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	திரும்பு -1;
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
நிலையான முழு open_no_symlinks_fallback(மாறிலி எழுத்து *path, முழு flags, mode_t mode)
{
	எழுத்து *abspath = வெற்று, *kpath = வெற்று;
	கட்டமைப்பு stat st;
	முழு fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	எனில் (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		திரும்பு -1;
	}

	எனில் (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		எனில் (!abspath)
			திரும்பு -1;
	}

	எனில் (flags & O_PATH)
		fd = open(path, flags);
	இல்லையெனில்
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	எனில் (fd < 0)
		செல் fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	எனில் (fstat(fd, &st) != 0)
		செல் fail;
	எனில் (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		செல் fail;
	}

	kpath = ul_fd_get_path(fd);
	எனில் (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		செல் fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	எனில் (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		செல் fail;
	}

	எனில் (!(flags & O_PATH)) {
		எழுத்து fdpath[UL_FDPATH_BUFSIZ];
		முழு nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		எனில் (ul_fd_mkpath(fdpath, அளவு(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		எனில் (nfd < 0)
			செல் fail;
		close(fd);
		fd = nfd;
	}

	விடுவி(abspath);
	விடுவி(kpath);
	திரும்பு fd;
fail:
	errsv = errno;
	விடுவி(abspath);
	விடுவி(kpath);
	எனில் (fd >= 0)
		close(fd);
	errno = errsv;
	திரும்பு -1;
}
#இல்லையெனில் /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
நிலையான முழு open_no_symlinks_fallback(
			மாறிலி எழுத்து *path __attribute__((__unused__)),
			முழு flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	திரும்பு -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
முழு ul_open_no_symlinks(மாறிலி எழுத்து *path, முழு flags, mode_t mode)
{
	முழு fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	எனில் (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	திரும்பு fd;
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
முழு ul_safe_statx(முழு dirfd, மாறிலி எழுத்து *path, முழு flags,
		  unsigned முழு mask, கட்டமைப்பு statx *stx)
{
	ப்ரதிஜ்ஞா(stx);

	memset(stx, 0, அளவு(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	எனில் (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	எனில் (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		திரும்பு -errno;

	திரும்பு 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
வெறுமை ul_statx_to_stat(மாறிலி கட்டமைப்பு statx *stx, கட்டமைப்பு stat *st)
{
	memset(st, 0, அளவு(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	எனில் (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	எனில் (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	எனில் (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	எனில் (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	எனில் (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	எனில் (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	எனில் (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	எனில் (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	எனில் (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	எனில் (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	எனில் (stx->stx_mask & STATX_CTIME) {
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
முழு ul_safe_stat(மாறிலி எழுத்து *target, கட்டமைப்பு stat *st,
		 முழு nofollow __attribute__((__unused__)),
		 unsigned முழு mask __attribute__((__unused__)),
		 unsigned முழு *retmask)
{
	ப்ரதிஜ்ஞா(target);
	ப்ரதிஜ்ஞா(st);

	memset(st, 0, அளவு(கட்டமைப்பு stat));
	எனில் (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		கட்டமைப்பு statx stx = { 0 };
		முழு rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		எனில் (rc)
			திரும்பு rc;

		ul_statx_to_stat(&stx, st);

		எனில் (retmask)
			*retmask = stx.stx_mask;

		எனில் ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			திரும்பு -EOPNOTSUPP;
		}

		திரும்பு 0;
	}
#இல்லையெனில்
	errno = ENOSYS;
	திரும்பு -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	எனில் (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	எனில் (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		எழுத்து *tmpname = வெற்று;

		f = xfmkstemp(&tmpname, வெற்று, "test");
		unlink(tmpname);
		விடுவி(tmpname);
		fclose(f);

	} இல்லையெனில் எனில் (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		எனில் (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} இல்லையெனில் எனில் (strcmp(argv[1], "--copy-file") == 0) {
		முழு ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		எனில் (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		இல்லையெனில் எனில் (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} இல்லையெனில் எனில் (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		முழு flags = O_PATH | O_CLOEXEC;
#இல்லையெனில்
		முழு flags = O_RDONLY | O_CLOEXEC;
#endif
		முழு fallback = 0;
		எழுத்து *name;
		முழு fd, i;

		எனில் (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		க்கு (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			எனில் (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			இல்லையெனில் எனில் (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		எனில் (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		இல்லையெனில்
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		எனில் (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		விடுவி(name);
		close(fd);

	} இல்லையெனில் எனில் (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned முழு mask = 0, retmask = 0;
		முழு nofollow = 0;
		கட்டமைப்பு stat st, ref;
		முழு rc, i;

		எனில் (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		க்கு (i = 3; i < argc; i++) {
			எனில் (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			இல்லையெனில் எனில் (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		எனில் (rc)
			திரும்பு EXIT_FAILURE;

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
		எனில் (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned முழு emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			எனில் (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			எனில் (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			எனில் (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			எனில் (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			எனில் (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			எனில் (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			எனில் (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	திரும்பு EXIT_SUCCESS;
}
#endif
