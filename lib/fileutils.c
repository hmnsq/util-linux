/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "c.h"
#include "all-io.h"
#include "canonicalize.h"
#include "fileutils.h"
#include "pathnames.h"
#include "strutils.h"

整数 mkstemp_cloexec(char *template)
{
#ifdef HAVE_MKOSTEMP
	返回 mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#否则
	整数 fd, old_flags, errno_save;

	fd = mkstemp(template);
	如果 (fd < 0)
		返回 fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	如果 (old_flags < 0)
		goto unwind;
	如果 (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		goto unwind;

	返回 fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	返回 -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
整数 xmkstemp(char **tmpname, const char *dir, const char *prefix)
{
	char *localtmp;
	const char *tmpenv;
	mode_t old_mode;
	整数 fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	如果 (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	如果 (rc < 0)
		返回 -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	如果 (fd == -1) {
		free(localtmp);
		localtmp = NULL;
	}
	*tmpname = localtmp;
	返回 fd;
}

#ifdef F_DUPFD_CLOEXEC
整数 dup_fd_cloexec(整数 oldfd, 整数 lowfd)
#否则
整数 dup_fd_cloexec(整数 oldfd, 整数 lowfd  __attribute__((__unused__)))
#endif
{
	整数 fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	如果 (fd >= 0)
		返回 fd;
#endif

	fd = dup(oldfd);
	如果 (fd < 0)
		返回 fd;

	flags = fcntl(fd, F_GETFD);
	如果 (flags < 0)
		goto unwind;
	如果 (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		goto unwind;

	返回 fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	返回 -1;
}

/*
 * portable getdtablesize()
 */
unsigned 整数 get_fd_tabsize(void)
{
	整数 m;

#如果 defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#elif defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	struct rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#elif defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#否则
	m = OPEN_MAX;
#endif
	返回 m;
}

void ul_close_all_fds(unsigned 整数 first, unsigned 整数 last)
{
	struct dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	如果 (dir) {
		当 ((d = xreaddir(dir))) {
			char *end;
			unsigned 整数 fd;
			整数 dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			如果 (errno || end == d->d_name || !end || *end)
				continue;
			dfd = dirfd(dir);
			如果 (dfd < 0)
				continue;
			如果 ((unsigned 整数)dfd == fd)
				continue;
			如果 (fd < first || last < fd)
				continue;
			close(fd);
		}
		closedir(dir);
	} 否则 {
		unsigned fd, tbsz = get_fd_tabsize();

		为 (fd = 0; fd < tbsz; fd++) {
			如果 (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
char *ul_restricted_path_oper(const char *path,
		  整数 (*oper)(const char *path, char **result, void *data),
		  void *data)
{
	char *result = NULL;
	整数 errsv = 0;
	整数 pipes[2];
	ssize_t len;
	pid_t pid;

	如果 (!path || !*path)
		返回 NULL;

	如果 (pipe(pipes) != 0)
		返回 NULL;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	switch (pid) {
	case -1:
		close(pipes[0]);
		close(pipes[1]);
		返回 NULL;			/* fork error */
	case 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		如果 (drop_permissions() != 0)
			result = NULL;	/* failed */
		否则
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (char *) &len, sizeof(len));
		如果 (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	default:
		break;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	如果 (ul_read_all(pipes[0], (char *) &len, sizeof(len)) != sizeof(len))
		goto done;
	如果 (len < 0) {
		errsv = -len;
		goto done;
	}

	result = malloc(len + 1);
	如果 (!result) {
		errsv = ENOMEM;
		goto done;
	}
	/* read path */
	如果 (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		goto done;
	}
	result[len] = '\0';
done:
	如果 (errsv) {
		free(result);
		result = NULL;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, NULL, 0) );

	errno = errsv;
	返回 result;

}

整数 ul_mkdir_p(const char *path, mode_t mode)
{
	char *p, *dir;
	整数 rc = 0;

	如果 (!path || !*path)
		返回 -EINVAL;

	dir = p = strdup(path);
	如果 (!dir)
		返回 -ENOMEM;

	如果 (*p == '/')
		p++;

	当 (p && *p) {
		char *e = strchr(p, '/');
		如果 (e)
			*e = '\0';
		如果 (*p) {
			rc = mkdir(dir, mode);
			如果 (rc && errno != EEXIST)
				break;
			rc = 0;
		}
		如果 (!e)
			break;
		*e = '/';
		p = e + 1;
	}

	free(dir);
	返回 rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
char *stripoff_last_component(char *path)
{
	char *p = path ? strrchr(path, '/') : NULL;

	如果 (!p)
		返回 NULL;
	*p = '\0';
	返回 p + 1;
}

static 整数 copy_file_simple(整数 from, 整数 to)
{
	ssize_t nr;
	char buf[BUFSIZ];

	当 ((nr = ul_read_all(from, buf, sizeof(buf))) > 0)
		如果 (ul_write_all(to, buf, nr) == -1)
			返回 UL_COPY_WRITE_ERROR;
	如果 (nr < 0)
		返回 UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, sizeof(buf));
#endif
	返回 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
整数 ul_copy_file(整数 from, 整数 to)
{
#ifdef HAVE_SENDFILE
	struct stat st;
	ssize_t nw;

	如果 (fstat(from, &st) == -1)
		返回 UL_COPY_READ_ERROR;
	如果 (!S_ISREG(st.st_mode))
		返回 copy_file_simple(from, to);
	如果 (ul_sendfile_all(to, from, NULL, st.st_size) < 0)
		返回 copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	当 ((nw = ul_sendfile_all(to, from, NULL, 16*1024*1024)) != 0)
		如果 (nw < 0)
			返回 copy_file_simple(from, to);
	返回 0;
#否则
	返回 copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
char *ul_fd_mkpath(char *buf, size_t bufsz, 整数 fd)
{
	整数 len;

	如果 (fd < 0) {
		errno = EBADF;
		返回 NULL;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	如果 (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		返回 NULL;
	}

	返回 buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
char *ul_fd_get_path(整数 fd)
{
	ssize_t ssz;
	char buf[PATH_MAX];
	char fdpath[UL_FDPATH_BUFSIZ];

	如果 (!ul_fd_mkpath(fdpath, sizeof(fdpath), fd))
		返回 NULL;

	ssz = readlink(fdpath, buf, sizeof(buf));
	如果 (ssz < 0)
		返回 NULL;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	如果 ((size_t) ssz >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		返回 NULL;
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
	如果 (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		返回 NULL;
	}

	返回 strdup(buf);
}

整数 ul_reopen(整数 fd, 整数 flags)
{
	char *path = ul_fd_get_path(fd);
	整数 ret;

	如果 (!path)
		返回 -errno;

	ret = open(path, flags);
	free(path);

	返回 ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
char *ul_basename(char *path)
{
	char *p;

	如果 (!path || !*path)
		返回 (char *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	如果 (!p)
		返回 path;		/* no '/', return original */

	如果 (*(p + 1) != '\0')
		返回 p + 1;		/* begin of the name */

	当 (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	如果 (p > path) {
		*p-- = '\0';
		当 (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} 否则 当 (*(p + 1) != '\0')
		++p;

	返回 p;
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
FILE *fopen_at_no_link(整数 dir, const char *filename,
                             整数 flags, mode_t perm, const char *mode)
{
	FILE *fp;
	整数 fd;
	struct stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	如果 (fd < 0)
		返回 NULL;

	如果 (fstat(fd, &st)) {
		close(fd);
		返回 NULL;
	}

	如果 (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		返回 NULL;
	}

	如果 ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		返回 NULL;
	}

	fp = fdopen(fd, mode);
	如果 (!fp)
		close(fd);
	返回 fp;
}
#endif /* HAVE_OPENAT */

#如果 defined(SYS_openat2)
整数 ul_openat_resolve(整数 dirfd, const char *path, 整数 flags,
		      mode_t mode, unsigned long long resolve)
{
	struct open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	返回 syscall(SYS_openat2, dirfd, path, &how, sizeof(how));
}
#否则
整数 ul_openat_resolve(
		整数 dirfd __attribute__((__unused__)),
		const char *path __attribute__((__unused__)),
		整数 flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	返回 -1;
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
static 整数 open_no_symlinks_fallback(const char *path, 整数 flags, mode_t mode)
{
	char *abspath = NULL, *kpath = NULL;
	struct stat st;
	整数 fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	如果 (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		返回 -1;
	}

	如果 (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		如果 (!abspath)
			返回 -1;
	}

	如果 (flags & O_PATH)
		fd = open(path, flags);
	否则
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	如果 (fd < 0)
		goto fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	如果 (fstat(fd, &st) != 0)
		goto fail;
	如果 (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		goto fail;
	}

	kpath = ul_fd_get_path(fd);
	如果 (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		goto fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	如果 (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		goto fail;
	}

	如果 (!(flags & O_PATH)) {
		char fdpath[UL_FDPATH_BUFSIZ];
		整数 nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		如果 (ul_fd_mkpath(fdpath, sizeof(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		如果 (nfd < 0)
			goto fail;
		close(fd);
		fd = nfd;
	}

	free(abspath);
	free(kpath);
	返回 fd;
fail:
	errsv = errno;
	free(abspath);
	free(kpath);
	如果 (fd >= 0)
		close(fd);
	errno = errsv;
	返回 -1;
}
#否则 /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
static 整数 open_no_symlinks_fallback(
			const char *path __attribute__((__unused__)),
			整数 flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	返回 -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
整数 ul_open_no_symlinks(const char *path, 整数 flags, mode_t mode)
{
	整数 fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	如果 (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	返回 fd;
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
整数 ul_safe_statx(整数 dirfd, const char *path, 整数 flags,
		  unsigned 整数 mask, struct statx *stx)
{
	assert(stx);

	memset(stx, 0, sizeof(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	如果 (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	如果 (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		返回 -errno;

	返回 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
void ul_statx_to_stat(const struct statx *stx, struct stat *st)
{
	memset(st, 0, sizeof(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	如果 (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	如果 (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	如果 (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	如果 (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	如果 (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	如果 (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	如果 (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	如果 (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	如果 (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	如果 (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	如果 (stx->stx_mask & STATX_CTIME) {
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
整数 ul_safe_stat(const char *target, struct stat *st,
		 整数 nofollow __attribute__((__unused__)),
		 unsigned 整数 mask __attribute__((__unused__)),
		 unsigned 整数 *retmask)
{
	assert(target);
	assert(st);

	memset(st, 0, sizeof(struct stat));
	如果 (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		struct statx stx = { 0 };
		整数 rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		如果 (rc)
			返回 rc;

		ul_statx_to_stat(&stx, st);

		如果 (retmask)
			*retmask = stx.stx_mask;

		如果 ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			返回 -EOPNOTSUPP;
		}

		返回 0;
	}
#否则
	errno = ENOSYS;
	返回 -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
整数 主要(整数 argc, char *argv[])
{
	如果 (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	如果 (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		char *tmpname = NULL;

		f = xfmkstemp(&tmpname, NULL, "test");
		unlink(tmpname);
		free(tmpname);
		fclose(f);

	} 否则 如果 (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		如果 (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} 否则 如果 (strcmp(argv[1], "--copy-file") == 0) {
		整数 ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		如果 (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		否则 如果 (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} 否则 如果 (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		整数 flags = O_PATH | O_CLOEXEC;
#否则
		整数 flags = O_RDONLY | O_CLOEXEC;
#endif
		整数 fallback = 0;
		char *name;
		整数 fd, i;

		如果 (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		为 (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			如果 (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			否则 如果 (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		如果 (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		否则
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		如果 (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		free(name);
		close(fd);

	} 否则 如果 (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned 整数 mask = 0, retmask = 0;
		整数 nofollow = 0;
		struct stat st, ref;
		整数 rc, i;

		如果 (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		为 (i = 3; i < argc; i++) {
			如果 (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			否则 如果 (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		如果 (rc)
			返回 EXIT_FAILURE;

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
		如果 (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned 整数 emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			如果 (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			如果 (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			如果 (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			如果 (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			如果 (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			如果 (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			如果 (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	返回 EXIT_SUCCESS;
}
#endif
