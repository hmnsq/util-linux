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

số_nguyên mkstemp_cloexec(char *template)
{
#ifdef HAVE_MKOSTEMP
	trả_về mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#không_thì
	số_nguyên fd, old_flags, errno_save;

	fd = mkstemp(template);
	nếu (fd < 0)
		trả_về fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	nếu (old_flags < 0)
		goto unwind;
	nếu (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		goto unwind;

	trả_về fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	trả_về -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
số_nguyên xmkstemp(char **tmpname, const char *dir, const char *prefix)
{
	char *localtmp;
	const char *tmpenv;
	mode_t old_mode;
	số_nguyên fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	nếu (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	nếu (rc < 0)
		trả_về -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	nếu (fd == -1) {
		free(localtmp);
		localtmp = NULL;
	}
	*tmpname = localtmp;
	trả_về fd;
}

#ifdef F_DUPFD_CLOEXEC
số_nguyên dup_fd_cloexec(số_nguyên oldfd, số_nguyên lowfd)
#không_thì
số_nguyên dup_fd_cloexec(số_nguyên oldfd, số_nguyên lowfd  __attribute__((__unused__)))
#endif
{
	số_nguyên fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	nếu (fd >= 0)
		trả_về fd;
#endif

	fd = dup(oldfd);
	nếu (fd < 0)
		trả_về fd;

	flags = fcntl(fd, F_GETFD);
	nếu (flags < 0)
		goto unwind;
	nếu (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		goto unwind;

	trả_về fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	trả_về -1;
}

/*
 * portable getdtablesize()
 */
unsigned số_nguyên get_fd_tabsize(void)
{
	số_nguyên m;

#nếu defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#elif defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	struct rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#elif defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#không_thì
	m = OPEN_MAX;
#endif
	trả_về m;
}

void ul_close_all_fds(unsigned số_nguyên first, unsigned số_nguyên last)
{
	struct dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	nếu (dir) {
		trong_khi ((d = xreaddir(dir))) {
			char *end;
			unsigned số_nguyên fd;
			số_nguyên dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			nếu (errno || end == d->d_name || !end || *end)
				continue;
			dfd = dirfd(dir);
			nếu (dfd < 0)
				continue;
			nếu ((unsigned số_nguyên)dfd == fd)
				continue;
			nếu (fd < first || last < fd)
				continue;
			close(fd);
		}
		closedir(dir);
	} không_thì {
		unsigned fd, tbsz = get_fd_tabsize();

		cho (fd = 0; fd < tbsz; fd++) {
			nếu (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
char *ul_restricted_path_oper(const char *path,
		  số_nguyên (*oper)(const char *path, char **result, void *data),
		  void *data)
{
	char *result = NULL;
	số_nguyên errsv = 0;
	số_nguyên pipes[2];
	ssize_t len;
	pid_t pid;

	nếu (!path || !*path)
		trả_về NULL;

	nếu (pipe(pipes) != 0)
		trả_về NULL;
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
		trả_về NULL;			/* fork error */
	case 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		nếu (drop_permissions() != 0)
			result = NULL;	/* failed */
		không_thì
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (char *) &len, sizeof(len));
		nếu (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	default:
		break;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	nếu (ul_read_all(pipes[0], (char *) &len, sizeof(len)) != sizeof(len))
		goto done;
	nếu (len < 0) {
		errsv = -len;
		goto done;
	}

	result = malloc(len + 1);
	nếu (!result) {
		errsv = ENOMEM;
		goto done;
	}
	/* read path */
	nếu (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		goto done;
	}
	result[len] = '\0';
done:
	nếu (errsv) {
		free(result);
		result = NULL;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, NULL, 0) );

	errno = errsv;
	trả_về result;

}

số_nguyên ul_mkdir_p(const char *path, mode_t mode)
{
	char *p, *dir;
	số_nguyên rc = 0;

	nếu (!path || !*path)
		trả_về -EINVAL;

	dir = p = strdup(path);
	nếu (!dir)
		trả_về -ENOMEM;

	nếu (*p == '/')
		p++;

	trong_khi (p && *p) {
		char *e = strchr(p, '/');
		nếu (e)
			*e = '\0';
		nếu (*p) {
			rc = mkdir(dir, mode);
			nếu (rc && errno != EEXIST)
				break;
			rc = 0;
		}
		nếu (!e)
			break;
		*e = '/';
		p = e + 1;
	}

	free(dir);
	trả_về rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
char *stripoff_last_component(char *path)
{
	char *p = path ? strrchr(path, '/') : NULL;

	nếu (!p)
		trả_về NULL;
	*p = '\0';
	trả_về p + 1;
}

static số_nguyên copy_file_simple(số_nguyên from, số_nguyên to)
{
	ssize_t nr;
	char buf[BUFSIZ];

	trong_khi ((nr = ul_read_all(from, buf, sizeof(buf))) > 0)
		nếu (ul_write_all(to, buf, nr) == -1)
			trả_về UL_COPY_WRITE_ERROR;
	nếu (nr < 0)
		trả_về UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, sizeof(buf));
#endif
	trả_về 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
số_nguyên ul_copy_file(số_nguyên from, số_nguyên to)
{
#ifdef HAVE_SENDFILE
	struct stat st;
	ssize_t nw;

	nếu (fstat(from, &st) == -1)
		trả_về UL_COPY_READ_ERROR;
	nếu (!S_ISREG(st.st_mode))
		trả_về copy_file_simple(from, to);
	nếu (ul_sendfile_all(to, from, NULL, st.st_size) < 0)
		trả_về copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	trong_khi ((nw = ul_sendfile_all(to, from, NULL, 16*1024*1024)) != 0)
		nếu (nw < 0)
			trả_về copy_file_simple(from, to);
	trả_về 0;
#không_thì
	trả_về copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
char *ul_fd_mkpath(char *buf, size_t bufsz, số_nguyên fd)
{
	số_nguyên len;

	nếu (fd < 0) {
		errno = EBADF;
		trả_về NULL;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	nếu (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		trả_về NULL;
	}

	trả_về buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
char *ul_fd_get_path(số_nguyên fd)
{
	ssize_t ssz;
	char buf[PATH_MAX];
	char fdpath[UL_FDPATH_BUFSIZ];

	nếu (!ul_fd_mkpath(fdpath, sizeof(fdpath), fd))
		trả_về NULL;

	ssz = readlink(fdpath, buf, sizeof(buf));
	nếu (ssz < 0)
		trả_về NULL;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	nếu ((size_t) ssz >= sizeof(buf)) {
		errno = ENAMETOOLONG;
		trả_về NULL;
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
	nếu (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		trả_về NULL;
	}

	trả_về strdup(buf);
}

số_nguyên ul_reopen(số_nguyên fd, số_nguyên flags)
{
	char *path = ul_fd_get_path(fd);
	số_nguyên ret;

	nếu (!path)
		trả_về -errno;

	ret = open(path, flags);
	free(path);

	trả_về ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
char *ul_basename(char *path)
{
	char *p;

	nếu (!path || !*path)
		trả_về (char *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	nếu (!p)
		trả_về path;		/* no '/', return original */

	nếu (*(p + 1) != '\0')
		trả_về p + 1;		/* begin of the name */

	trong_khi (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	nếu (p > path) {
		*p-- = '\0';
		trong_khi (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} không_thì trong_khi (*(p + 1) != '\0')
		++p;

	trả_về p;
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
FILE *fopen_at_no_link(số_nguyên dir, const char *filename,
                             số_nguyên flags, mode_t perm, const char *mode)
{
	FILE *fp;
	số_nguyên fd;
	struct stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	nếu (fd < 0)
		trả_về NULL;

	nếu (fstat(fd, &st)) {
		close(fd);
		trả_về NULL;
	}

	nếu (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		trả_về NULL;
	}

	nếu ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		trả_về NULL;
	}

	fp = fdopen(fd, mode);
	nếu (!fp)
		close(fd);
	trả_về fp;
}
#endif /* HAVE_OPENAT */

#nếu defined(SYS_openat2)
số_nguyên ul_openat_resolve(số_nguyên dirfd, const char *path, số_nguyên flags,
		      mode_t mode, unsigned long long resolve)
{
	struct open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	trả_về syscall(SYS_openat2, dirfd, path, &how, sizeof(how));
}
#không_thì
số_nguyên ul_openat_resolve(
		số_nguyên dirfd __attribute__((__unused__)),
		const char *path __attribute__((__unused__)),
		số_nguyên flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	trả_về -1;
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
static số_nguyên open_no_symlinks_fallback(const char *path, số_nguyên flags, mode_t mode)
{
	char *abspath = NULL, *kpath = NULL;
	struct stat st;
	số_nguyên fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	nếu (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		trả_về -1;
	}

	nếu (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		nếu (!abspath)
			trả_về -1;
	}

	nếu (flags & O_PATH)
		fd = open(path, flags);
	không_thì
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	nếu (fd < 0)
		goto fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	nếu (fstat(fd, &st) != 0)
		goto fail;
	nếu (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		goto fail;
	}

	kpath = ul_fd_get_path(fd);
	nếu (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		goto fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	nếu (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		goto fail;
	}

	nếu (!(flags & O_PATH)) {
		char fdpath[UL_FDPATH_BUFSIZ];
		số_nguyên nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		nếu (ul_fd_mkpath(fdpath, sizeof(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		nếu (nfd < 0)
			goto fail;
		close(fd);
		fd = nfd;
	}

	free(abspath);
	free(kpath);
	trả_về fd;
fail:
	errsv = errno;
	free(abspath);
	free(kpath);
	nếu (fd >= 0)
		close(fd);
	errno = errsv;
	trả_về -1;
}
#không_thì /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
static số_nguyên open_no_symlinks_fallback(
			const char *path __attribute__((__unused__)),
			số_nguyên flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	trả_về -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
số_nguyên ul_open_no_symlinks(const char *path, số_nguyên flags, mode_t mode)
{
	số_nguyên fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	nếu (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	trả_về fd;
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
số_nguyên ul_safe_statx(số_nguyên dirfd, const char *path, số_nguyên flags,
		  unsigned số_nguyên mask, struct statx *stx)
{
	assert(stx);

	memset(stx, 0, sizeof(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	nếu (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	nếu (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		trả_về -errno;

	trả_về 0;
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
	nếu (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	nếu (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	nếu (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	nếu (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	nếu (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	nếu (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	nếu (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	nếu (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	nếu (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	nếu (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	nếu (stx->stx_mask & STATX_CTIME) {
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
số_nguyên ul_safe_stat(const char *target, struct stat *st,
		 số_nguyên nofollow __attribute__((__unused__)),
		 unsigned số_nguyên mask __attribute__((__unused__)),
		 unsigned số_nguyên *retmask)
{
	assert(target);
	assert(st);

	memset(st, 0, sizeof(struct stat));
	nếu (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		struct statx stx = { 0 };
		số_nguyên rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		nếu (rc)
			trả_về rc;

		ul_statx_to_stat(&stx, st);

		nếu (retmask)
			*retmask = stx.stx_mask;

		nếu ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			trả_về -EOPNOTSUPP;
		}

		trả_về 0;
	}
#không_thì
	errno = ENOSYS;
	trả_về -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
số_nguyên main(số_nguyên argc, char *argv[])
{
	nếu (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	nếu (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		char *tmpname = NULL;

		f = xfmkstemp(&tmpname, NULL, "test");
		unlink(tmpname);
		free(tmpname);
		fclose(f);

	} không_thì nếu (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		nếu (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} không_thì nếu (strcmp(argv[1], "--copy-file") == 0) {
		số_nguyên ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		nếu (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		không_thì nếu (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} không_thì nếu (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		số_nguyên flags = O_PATH | O_CLOEXEC;
#không_thì
		số_nguyên flags = O_RDONLY | O_CLOEXEC;
#endif
		số_nguyên fallback = 0;
		char *name;
		số_nguyên fd, i;

		nếu (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		cho (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			nếu (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			không_thì nếu (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		nếu (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		không_thì
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		nếu (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		free(name);
		close(fd);

	} không_thì nếu (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned số_nguyên mask = 0, retmask = 0;
		số_nguyên nofollow = 0;
		struct stat st, ref;
		số_nguyên rc, i;

		nếu (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		cho (i = 3; i < argc; i++) {
			nếu (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			không_thì nếu (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		nếu (rc)
			trả_về EXIT_FAILURE;

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
		nếu (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned số_nguyên emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			nếu (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			nếu (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			nếu (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			nếu (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			nếu (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			nếu (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			nếu (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	trả_về EXIT_SUCCESS;
}
#endif
