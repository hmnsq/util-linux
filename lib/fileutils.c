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

ចំនួនគត់ mkstemp_cloexec(var_na *template)
{
#ifdef HAVE_MKOSTEMP
	ត្រឡប់ mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#បើមិនដូច្នេះ
	ចំនួនគត់ fd, old_flags, errno_save;

	fd = mkstemp(template);
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ប្រសិនបើ (old_flags < 0)
		goto unwind;
	ប្រសិនបើ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		goto unwind;

	ត្រឡប់ fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	ត្រឡប់ -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
ចំនួនគត់ xmkstemp(var_na **tmpname, const var_na *dir, const var_na *prefix)
{
	var_na *localtmp;
	const var_na *tmpenv;
	mode_t old_mode;
	ចំនួនគត់ fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ប្រសិនបើ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	ប្រសិនបើ (rc < 0)
		ត្រឡប់ -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	ប្រសិនបើ (fd == -1) {
		free(localtmp);
		localtmp = NULL;
	}
	*tmpname = localtmp;
	ត្រឡប់ fd;
}

#ifdef F_DUPFD_CLOEXEC
ចំនួនគត់ dup_fd_cloexec(ចំនួនគត់ oldfd, ចំនួនគត់ lowfd)
#បើមិនដូច្នេះ
ចំនួនគត់ dup_fd_cloexec(ចំនួនគត់ oldfd, ចំនួនគត់ lowfd  __attribute__((__unused__)))
#endif
{
	ចំនួនគត់ fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ប្រសិនបើ (fd >= 0)
		ត្រឡប់ fd;
#endif

	fd = dup(oldfd);
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ fd;

	flags = fcntl(fd, F_GETFD);
	ប្រសិនបើ (flags < 0)
		goto unwind;
	ប្រសិនបើ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		goto unwind;

	ត្រឡប់ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ត្រឡប់ -1;
}

/*
 * portable getdtablesize()
 */
unsigned ចំនួនគត់ get_fd_tabsize(void)
{
	ចំនួនគត់ m;

#ប្រសិនបើ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#elif defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	sa_nracanaa rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#elif defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#បើមិនដូច្នេះ
	m = OPEN_MAX;
#endif
	ត្រឡប់ m;
}

void ul_close_all_fds(unsigned ចំនួនគត់ first, unsigned ចំនួនគត់ last)
{
	sa_nracanaa dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	ប្រសិនបើ (dir) {
		ខណៈពេល ((d = xreaddir(dir))) {
			var_na *end;
			unsigned ចំនួនគត់ fd;
			ចំនួនគត់ dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			ប្រសិនបើ (errno || end == d->d_name || !end || *end)
				continue;
			dfd = dirfd(dir);
			ប្រសិនបើ (dfd < 0)
				continue;
			ប្រសិនបើ ((unsigned ចំនួនគត់)dfd == fd)
				continue;
			ប្រសិនបើ (fd < first || last < fd)
				continue;
			close(fd);
		}
		closedir(dir);
	} បើមិនដូច្នេះ {
		unsigned fd, tbsz = get_fd_tabsize();

		សម្រាប់ (fd = 0; fd < tbsz; fd++) {
			ប្រសិនបើ (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
var_na *ul_restricted_path_oper(const var_na *path,
		  ចំនួនគត់ (*oper)(const var_na *path, var_na **result, void *data),
		  void *data)
{
	var_na *result = NULL;
	ចំនួនគត់ errsv = 0;
	ចំនួនគត់ pipes[2];
	ssize_t len;
	pid_t pid;

	ប្រសិនបើ (!path || !*path)
		ត្រឡប់ NULL;

	ប្រសិនបើ (pipe(pipes) != 0)
		ត្រឡប់ NULL;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	switch (pid) {
	pak_sa -1:
		close(pipes[0]);
		close(pipes[1]);
		ត្រឡប់ NULL;			/* fork error */
	pak_sa 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ប្រសិនបើ (drop_permissions() != 0)
			result = NULL;	/* failed */
		បើមិនដូច្នេះ
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (var_na *) &len, parimaa_na(len));
		ប្រសិនបើ (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	default:
		bha_nga;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ប្រសិនបើ (ul_read_all(pipes[0], (var_na *) &len, parimaa_na(len)) != parimaa_na(len))
		goto done;
	ប្រសិនបើ (len < 0) {
		errsv = -len;
		goto done;
	}

	result = malloc(len + 1);
	ប្រសិនបើ (!result) {
		errsv = ENOMEM;
		goto done;
	}
	/* read path */
	ប្រសិនបើ (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		goto done;
	}
	result[len] = '\0';
done:
	ប្រសិនបើ (errsv) {
		free(result);
		result = NULL;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, NULL, 0) );

	errno = errsv;
	ត្រឡប់ result;

}

ចំនួនគត់ ul_mkdir_p(const var_na *path, mode_t mode)
{
	var_na *p, *dir;
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (!path || !*path)
		ត្រឡប់ -EINVAL;

	dir = p = strdup(path);
	ប្រសិនបើ (!dir)
		ត្រឡប់ -ENOMEM;

	ប្រសិនបើ (*p == '/')
		p++;

	ខណៈពេល (p && *p) {
		var_na *e = strchr(p, '/');
		ប្រសិនបើ (e)
			*e = '\0';
		ប្រសិនបើ (*p) {
			rc = mkdir(dir, mode);
			ប្រសិនបើ (rc && errno != EEXIST)
				bha_nga;
			rc = 0;
		}
		ប្រសិនបើ (!e)
			bha_nga;
		*e = '/';
		p = e + 1;
	}

	free(dir);
	ត្រឡប់ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
var_na *stripoff_last_component(var_na *path)
{
	var_na *p = path ? strrchr(path, '/') : NULL;

	ប្រសិនបើ (!p)
		ត្រឡប់ NULL;
	*p = '\0';
	ត្រឡប់ p + 1;
}

static ចំនួនគត់ copy_file_simple(ចំនួនគត់ from, ចំនួនគត់ to)
{
	ssize_t nr;
	var_na buf[BUFSIZ];

	ខណៈពេល ((nr = ul_read_all(from, buf, parimaa_na(buf))) > 0)
		ប្រសិនបើ (ul_write_all(to, buf, nr) == -1)
			ត្រឡប់ UL_COPY_WRITE_ERROR;
	ប្រសិនបើ (nr < 0)
		ត្រឡប់ UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, parimaa_na(buf));
#endif
	ត្រឡប់ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
ចំនួនគត់ ul_copy_file(ចំនួនគត់ from, ចំនួនគត់ to)
{
#ifdef HAVE_SENDFILE
	sa_nracanaa stat st;
	ssize_t nw;

	ប្រសិនបើ (fstat(from, &st) == -1)
		ត្រឡប់ UL_COPY_READ_ERROR;
	ប្រសិនបើ (!S_ISREG(st.st_mode))
		ត្រឡប់ copy_file_simple(from, to);
	ប្រសិនបើ (ul_sendfile_all(to, from, NULL, st.st_size) < 0)
		ត្រឡប់ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ខណៈពេល ((nw = ul_sendfile_all(to, from, NULL, 16*1024*1024)) != 0)
		ប្រសិនបើ (nw < 0)
			ត្រឡប់ copy_file_simple(from, to);
	ត្រឡប់ 0;
#បើមិនដូច្នេះ
	ត្រឡប់ copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
var_na *ul_fd_mkpath(var_na *buf, size_t bufsz, ចំនួនគត់ fd)
{
	ចំនួនគត់ len;

	ប្រសិនបើ (fd < 0) {
		errno = EBADF;
		ត្រឡប់ NULL;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	ប្រសិនបើ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ត្រឡប់ NULL;
	}

	ត្រឡប់ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
var_na *ul_fd_get_path(ចំនួនគត់ fd)
{
	ssize_t ssz;
	var_na buf[PATH_MAX];
	var_na fdpath[UL_FDPATH_BUFSIZ];

	ប្រសិនបើ (!ul_fd_mkpath(fdpath, parimaa_na(fdpath), fd))
		ត្រឡប់ NULL;

	ssz = readlink(fdpath, buf, parimaa_na(buf));
	ប្រសិនបើ (ssz < 0)
		ត្រឡប់ NULL;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ប្រសិនបើ ((size_t) ssz >= parimaa_na(buf)) {
		errno = ENAMETOOLONG;
		ត្រឡប់ NULL;
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
	ប្រសិនបើ (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		ត្រឡប់ NULL;
	}

	ត្រឡប់ strdup(buf);
}

ចំនួនគត់ ul_reopen(ចំនួនគត់ fd, ចំនួនគត់ flags)
{
	var_na *path = ul_fd_get_path(fd);
	ចំនួនគត់ ret;

	ប្រសិនបើ (!path)
		ត្រឡប់ -errno;

	ret = open(path, flags);
	free(path);

	ត្រឡប់ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
var_na *ul_basename(var_na *path)
{
	var_na *p;

	ប្រសិនបើ (!path || !*path)
		ត្រឡប់ (var_na *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	ប្រសិនបើ (!p)
		ត្រឡប់ path;		/* no '/', return original */

	ប្រសិនបើ (*(p + 1) != '\0')
		ត្រឡប់ p + 1;		/* begin of the name */

	ខណៈពេល (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ប្រសិនបើ (p > path) {
		*p-- = '\0';
		ខណៈពេល (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} បើមិនដូច្នេះ ខណៈពេល (*(p + 1) != '\0')
		++p;

	ត្រឡប់ p;
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
FILE *fopen_at_no_link(ចំនួនគត់ dir, const var_na *filename,
                             ចំនួនគត់ flags, mode_t perm, const var_na *mode)
{
	FILE *fp;
	ចំនួនគត់ fd;
	sa_nracanaa stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ NULL;

	ប្រសិនបើ (fstat(fd, &st)) {
		close(fd);
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ត្រឡប់ NULL;
	}

	fp = fdopen(fd, mode);
	ប្រសិនបើ (!fp)
		close(fd);
	ត្រឡប់ fp;
}
#endif /* HAVE_OPENAT */

#ប្រសិនបើ defined(SYS_openat2)
ចំនួនគត់ ul_openat_resolve(ចំនួនគត់ dirfd, const var_na *path, ចំនួនគត់ flags,
		      mode_t mode, unsigned long long resolve)
{
	sa_nracanaa open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	ត្រឡប់ syscall(SYS_openat2, dirfd, path, &how, parimaa_na(how));
}
#បើមិនដូច្នេះ
ចំនួនគត់ ul_openat_resolve(
		ចំនួនគត់ dirfd __attribute__((__unused__)),
		const var_na *path __attribute__((__unused__)),
		ចំនួនគត់ flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ត្រឡប់ -1;
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
static ចំនួនគត់ open_no_symlinks_fallback(const var_na *path, ចំនួនគត់ flags, mode_t mode)
{
	var_na *abspath = NULL, *kpath = NULL;
	sa_nracanaa stat st;
	ចំនួនគត់ fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ប្រសិនបើ (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		ត្រឡប់ -1;
	}

	ប្រសិនបើ (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		ប្រសិនបើ (!abspath)
			ត្រឡប់ -1;
	}

	ប្រសិនបើ (flags & O_PATH)
		fd = open(path, flags);
	បើមិនដូច្នេះ
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ប្រសិនបើ (fd < 0)
		goto fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ប្រសិនបើ (fstat(fd, &st) != 0)
		goto fail;
	ប្រសិនបើ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		goto fail;
	}

	kpath = ul_fd_get_path(fd);
	ប្រសិនបើ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		goto fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ប្រសិនបើ (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		goto fail;
	}

	ប្រសិនបើ (!(flags & O_PATH)) {
		var_na fdpath[UL_FDPATH_BUFSIZ];
		ចំនួនគត់ nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ប្រសិនបើ (ul_fd_mkpath(fdpath, parimaa_na(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		ប្រសិនបើ (nfd < 0)
			goto fail;
		close(fd);
		fd = nfd;
	}

	free(abspath);
	free(kpath);
	ត្រឡប់ fd;
fail:
	errsv = errno;
	free(abspath);
	free(kpath);
	ប្រសិនបើ (fd >= 0)
		close(fd);
	errno = errsv;
	ត្រឡប់ -1;
}
#បើមិនដូច្នេះ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
static ចំនួនគត់ open_no_symlinks_fallback(
			const var_na *path __attribute__((__unused__)),
			ចំនួនគត់ flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	ត្រឡប់ -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
ចំនួនគត់ ul_open_no_symlinks(const var_na *path, ចំនួនគត់ flags, mode_t mode)
{
	ចំនួនគត់ fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ប្រសិនបើ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	ត្រឡប់ fd;
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
ចំនួនគត់ ul_safe_statx(ចំនួនគត់ dirfd, const var_na *path, ចំនួនគត់ flags,
		  unsigned ចំនួនគត់ mask, sa_nracanaa statx *stx)
{
	assert(stx);

	memset(stx, 0, parimaa_na(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	ប្រសិនបើ (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	ប្រសិនបើ (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		ត្រឡប់ -errno;

	ត្រឡប់ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
void ul_statx_to_stat(const sa_nracanaa statx *stx, sa_nracanaa stat *st)
{
	memset(st, 0, parimaa_na(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ប្រសិនបើ (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	ប្រសិនបើ (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	ប្រសិនបើ (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	ប្រសិនបើ (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ប្រសិនបើ (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	ប្រសិនបើ (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	ប្រសិនបើ (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	ប្រសិនបើ (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	ប្រសិនបើ (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ប្រសិនបើ (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ប្រសិនបើ (stx->stx_mask & STATX_CTIME) {
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
ចំនួនគត់ ul_safe_stat(const var_na *target, sa_nracanaa stat *st,
		 ចំនួនគត់ nofollow __attribute__((__unused__)),
		 unsigned ចំនួនគត់ mask __attribute__((__unused__)),
		 unsigned ចំនួនគត់ *retmask)
{
	assert(target);
	assert(st);

	memset(st, 0, parimaa_na(sa_nracanaa stat));
	ប្រសិនបើ (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		sa_nracanaa statx stx = { 0 };
		ចំនួនគត់ rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		ប្រសិនបើ (rc)
			ត្រឡប់ rc;

		ul_statx_to_stat(&stx, st);

		ប្រសិនបើ (retmask)
			*retmask = stx.stx_mask;

		ប្រសិនបើ ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ត្រឡប់ -EOPNOTSUPP;
		}

		ត្រឡប់ 0;
	}
#បើមិនដូច្នេះ
	errno = ENOSYS;
	ត្រឡប់ -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	ប្រសិនបើ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ប្រសិនបើ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		var_na *tmpname = NULL;

		f = xfmkstemp(&tmpname, NULL, "test");
		unlink(tmpname);
		free(tmpname);
		fclose(f);

	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ប្រសិនបើ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--copy-file") == 0) {
		ចំនួនគត់ ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ប្រសិនបើ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		បើមិនដូច្នេះ ប្រសិនបើ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		ចំនួនគត់ flags = O_PATH | O_CLOEXEC;
#បើមិនដូច្នេះ
		ចំនួនគត់ flags = O_RDONLY | O_CLOEXEC;
#endif
		ចំនួនគត់ fallback = 0;
		var_na *name;
		ចំនួនគត់ fd, i;

		ប្រសិនបើ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		សម្រាប់ (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ប្រសិនបើ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ប្រសិនបើ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		បើមិនដូច្នេះ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ប្រសិនបើ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		free(name);
		close(fd);

	} បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned ចំនួនគត់ mask = 0, retmask = 0;
		ចំនួនគត់ nofollow = 0;
		sa_nracanaa stat st, ref;
		ចំនួនគត់ rc, i;

		ប្រសិនបើ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		សម្រាប់ (i = 3; i < argc; i++) {
			ប្រសិនបើ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		ប្រសិនបើ (rc)
			ត្រឡប់ EXIT_FAILURE;

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
		ប្រសិនបើ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned ចំនួនគត់ emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			ប្រសិនបើ (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ប្រសិនបើ (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ប្រសិនបើ (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			ប្រសិនបើ (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ប្រសិនបើ (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			ប្រសិនបើ (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			ប្រសិនបើ (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	ត្រឡប់ EXIT_SUCCESS;
}
#endif
