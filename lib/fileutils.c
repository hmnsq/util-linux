/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "canonicalize.h"
#અન્તર્ભાવ "fileutils.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "strutils.h"

પૂર્ણાંક mkstemp_cloexec(અક્ષર *template)
{
#ifdef HAVE_MKOSTEMP
	પાછા mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#નહીંતર
	પૂર્ણાંક fd, old_flags, errno_save;

	fd = mkstemp(template);
	જો (fd < 0)
		પાછા fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	જો (old_flags < 0)
		જાઓ unwind;
	જો (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		જાઓ unwind;

	પાછા fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	પાછા -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
પૂર્ણાંક xmkstemp(અક્ષર **tmpname, અચલ અક્ષર *dir, અચલ અક્ષર *prefix)
{
	અક્ષર *localtmp;
	અચલ અક્ષર *tmpenv;
	mode_t old_mode;
	પૂર્ણાંક fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	જો (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	જો (rc < 0)
		પાછા -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	જો (fd == -1) {
		મુક્ત(localtmp);
		localtmp = ખાલી;
	}
	*tmpname = localtmp;
	પાછા fd;
}

#ifdef F_DUPFD_CLOEXEC
પૂર્ણાંક dup_fd_cloexec(પૂર્ણાંક oldfd, પૂર્ણાંક lowfd)
#નહીંતર
પૂર્ણાંક dup_fd_cloexec(પૂર્ણાંક oldfd, પૂર્ણાંક lowfd  __attribute__((__unused__)))
#endif
{
	પૂર્ણાંક fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	જો (fd >= 0)
		પાછા fd;
#endif

	fd = dup(oldfd);
	જો (fd < 0)
		પાછા fd;

	flags = fcntl(fd, F_GETFD);
	જો (flags < 0)
		જાઓ unwind;
	જો (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		જાઓ unwind;

	પાછા fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	પાછા -1;
}

/*
 * portable getdtablesize()
 */
unsigned પૂર્ણાંક get_fd_tabsize(શૂન્ય)
{
	પૂર્ણાંક m;

#જો defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#અથવાજો defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	સંરચના rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#અથવાજો defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#નહીંતર
	m = OPEN_MAX;
#endif
	પાછા m;
}

શૂન્ય ul_close_all_fds(unsigned પૂર્ણાંક first, unsigned પૂર્ણાંક last)
{
	સંરચના dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	જો (dir) {
		જ્યારે ((d = xreaddir(dir))) {
			અક્ષર *end;
			unsigned પૂર્ણાંક fd;
			પૂર્ણાંક dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			જો (errno || end == d->d_name || !end || *end)
				ચાલુ;
			dfd = dirfd(dir);
			જો (dfd < 0)
				ચાલુ;
			જો ((unsigned પૂર્ણાંક)dfd == fd)
				ચાલુ;
			જો (fd < first || last < fd)
				ચાલુ;
			close(fd);
		}
		closedir(dir);
	} નહીંતર {
		unsigned fd, tbsz = get_fd_tabsize();

		માટે (fd = 0; fd < tbsz; fd++) {
			જો (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
અક્ષર *ul_restricted_path_oper(અચલ અક્ષર *path,
		  પૂર્ણાંક (*oper)(અચલ અક્ષર *path, અક્ષર **result, શૂન્ય *data),
		  શૂન્ય *data)
{
	અક્ષર *result = ખાલી;
	પૂર્ણાંક errsv = 0;
	પૂર્ણાંક pipes[2];
	ssize_t len;
	pid_t pid;

	જો (!path || !*path)
		પાછા ખાલી;

	જો (pipe(pipes) != 0)
		પાછા ખાલી;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	પસંદગી (pid) {
	સ્થિતિ -1:
		close(pipes[0]);
		close(pipes[1]);
		પાછા ખાલી;			/* fork error */
	સ્થિતિ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		જો (drop_permissions() != 0)
			result = ખાલી;	/* failed */
		નહીંતર
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (અક્ષર *) &len, કદ(len));
		જો (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	મૂળભૂત:
		તોડો;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	જો (ul_read_all(pipes[0], (અક્ષર *) &len, કદ(len)) != કદ(len))
		જાઓ done;
	જો (len < 0) {
		errsv = -len;
		જાઓ done;
	}

	result = malloc(len + 1);
	જો (!result) {
		errsv = ENOMEM;
		જાઓ done;
	}
	/* read path */
	જો (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		જાઓ done;
	}
	result[len] = '\0';
done:
	જો (errsv) {
		મુક્ત(result);
		result = ખાલી;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, ખાલી, 0) );

	errno = errsv;
	પાછા result;

}

પૂર્ણાંક ul_mkdir_p(અચલ અક્ષર *path, mode_t mode)
{
	અક્ષર *p, *dir;
	પૂર્ણાંક rc = 0;

	જો (!path || !*path)
		પાછા -EINVAL;

	dir = p = strdup(path);
	જો (!dir)
		પાછા -ENOMEM;

	જો (*p == '/')
		p++;

	જ્યારે (p && *p) {
		અક્ષર *e = strchr(p, '/');
		જો (e)
			*e = '\0';
		જો (*p) {
			rc = mkdir(dir, mode);
			જો (rc && errno != EEXIST)
				તોડો;
			rc = 0;
		}
		જો (!e)
			તોડો;
		*e = '/';
		p = e + 1;
	}

	મુક્ત(dir);
	પાછા rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
અક્ષર *stripoff_last_component(અક્ષર *path)
{
	અક્ષર *p = path ? strrchr(path, '/') : ખાલી;

	જો (!p)
		પાછા ખાલી;
	*p = '\0';
	પાછા p + 1;
}

સ્થિર પૂર્ણાંક copy_file_simple(પૂર્ણાંક from, પૂર્ણાંક to)
{
	ssize_t nr;
	અક્ષર buf[BUFSIZ];

	જ્યારે ((nr = ul_read_all(from, buf, કદ(buf))) > 0)
		જો (ul_write_all(to, buf, nr) == -1)
			પાછા UL_COPY_WRITE_ERROR;
	જો (nr < 0)
		પાછા UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, કદ(buf));
#endif
	પાછા 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
પૂર્ણાંક ul_copy_file(પૂર્ણાંક from, પૂર્ણાંક to)
{
#ifdef HAVE_SENDFILE
	સંરચના stat st;
	ssize_t nw;

	જો (fstat(from, &st) == -1)
		પાછા UL_COPY_READ_ERROR;
	જો (!S_ISREG(st.st_mode))
		પાછા copy_file_simple(from, to);
	જો (ul_sendfile_all(to, from, ખાલી, st.st_size) < 0)
		પાછા copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	જ્યારે ((nw = ul_sendfile_all(to, from, ખાલી, 16*1024*1024)) != 0)
		જો (nw < 0)
			પાછા copy_file_simple(from, to);
	પાછા 0;
#નહીંતર
	પાછા copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
અક્ષર *ul_fd_mkpath(અક્ષર *buf, size_t bufsz, પૂર્ણાંક fd)
{
	પૂર્ણાંક len;

	જો (fd < 0) {
		errno = EBADF;
		પાછા ખાલી;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	જો (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		પાછા ખાલી;
	}

	પાછા buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
અક્ષર *ul_fd_get_path(પૂર્ણાંક fd)
{
	ssize_t ssz;
	અક્ષર buf[PATH_MAX];
	અક્ષર fdpath[UL_FDPATH_BUFSIZ];

	જો (!ul_fd_mkpath(fdpath, કદ(fdpath), fd))
		પાછા ખાલી;

	ssz = readlink(fdpath, buf, કદ(buf));
	જો (ssz < 0)
		પાછા ખાલી;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	જો ((size_t) ssz >= કદ(buf)) {
		errno = ENAMETOOLONG;
		પાછા ખાલી;
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
	જો (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		પાછા ખાલી;
	}

	પાછા strdup(buf);
}

પૂર્ણાંક ul_reopen(પૂર્ણાંક fd, પૂર્ણાંક flags)
{
	અક્ષર *path = ul_fd_get_path(fd);
	પૂર્ણાંક ret;

	જો (!path)
		પાછા -errno;

	ret = open(path, flags);
	મુક્ત(path);

	પાછા ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
અક્ષર *ul_basename(અક્ષર *path)
{
	અક્ષર *p;

	જો (!path || !*path)
		પાછા (અક્ષર *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	જો (!p)
		પાછા path;		/* no '/', return original */

	જો (*(p + 1) != '\0')
		પાછા p + 1;		/* begin of the name */

	જ્યારે (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	જો (p > path) {
		*p-- = '\0';
		જ્યારે (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} નહીંતર જ્યારે (*(p + 1) != '\0')
		++p;

	પાછા p;
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
FILE *fopen_at_no_link(પૂર્ણાંક dir, અચલ અક્ષર *filename,
                             પૂર્ણાંક flags, mode_t perm, અચલ અક્ષર *mode)
{
	FILE *fp;
	પૂર્ણાંક fd;
	સંરચના stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	જો (fd < 0)
		પાછા ખાલી;

	જો (fstat(fd, &st)) {
		close(fd);
		પાછા ખાલી;
	}

	જો (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		પાછા ખાલી;
	}

	જો ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		પાછા ખાલી;
	}

	fp = fdopen(fd, mode);
	જો (!fp)
		close(fd);
	પાછા fp;
}
#endif /* HAVE_OPENAT */

#જો defined(SYS_openat2)
પૂર્ણાંક ul_openat_resolve(પૂર્ણાંક dirfd, અચલ અક્ષર *path, પૂર્ણાંક flags,
		      mode_t mode, unsigned long long resolve)
{
	સંરચના open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	પાછા syscall(SYS_openat2, dirfd, path, &how, કદ(how));
}
#નહીંતર
પૂર્ણાંક ul_openat_resolve(
		પૂર્ણાંક dirfd __attribute__((__unused__)),
		અચલ અક્ષર *path __attribute__((__unused__)),
		પૂર્ણાંક flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	પાછા -1;
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
સ્થિર પૂર્ણાંક open_no_symlinks_fallback(અચલ અક્ષર *path, પૂર્ણાંક flags, mode_t mode)
{
	અક્ષર *abspath = ખાલી, *kpath = ખાલી;
	સંરચના stat st;
	પૂર્ણાંક fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	જો (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		પાછા -1;
	}

	જો (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		જો (!abspath)
			પાછા -1;
	}

	જો (flags & O_PATH)
		fd = open(path, flags);
	નહીંતર
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	જો (fd < 0)
		જાઓ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	જો (fstat(fd, &st) != 0)
		જાઓ fail;
	જો (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		જાઓ fail;
	}

	kpath = ul_fd_get_path(fd);
	જો (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		જાઓ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	જો (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		જાઓ fail;
	}

	જો (!(flags & O_PATH)) {
		અક્ષર fdpath[UL_FDPATH_BUFSIZ];
		પૂર્ણાંક nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		જો (ul_fd_mkpath(fdpath, કદ(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		જો (nfd < 0)
			જાઓ fail;
		close(fd);
		fd = nfd;
	}

	મુક્ત(abspath);
	મુક્ત(kpath);
	પાછા fd;
fail:
	errsv = errno;
	મુક્ત(abspath);
	મુક્ત(kpath);
	જો (fd >= 0)
		close(fd);
	errno = errsv;
	પાછા -1;
}
#નહીંતર /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
સ્થિર પૂર્ણાંક open_no_symlinks_fallback(
			અચલ અક્ષર *path __attribute__((__unused__)),
			પૂર્ણાંક flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	પાછા -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
પૂર્ણાંક ul_open_no_symlinks(અચલ અક્ષર *path, પૂર્ણાંક flags, mode_t mode)
{
	પૂર્ણાંક fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	જો (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	પાછા fd;
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
પૂર્ણાંક ul_safe_statx(પૂર્ણાંક dirfd, અચલ અક્ષર *path, પૂર્ણાંક flags,
		  unsigned પૂર્ણાંક mask, સંરચના statx *stx)
{
	પ્રતિજ્ઞા(stx);

	memset(stx, 0, કદ(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	જો (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	જો (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		પાછા -errno;

	પાછા 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
શૂન્ય ul_statx_to_stat(અચલ સંરચના statx *stx, સંરચના stat *st)
{
	memset(st, 0, કદ(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	જો (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	જો (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	જો (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	જો (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	જો (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	જો (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	જો (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	જો (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	જો (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	જો (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	જો (stx->stx_mask & STATX_CTIME) {
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
પૂર્ણાંક ul_safe_stat(અચલ અક્ષર *target, સંરચના stat *st,
		 પૂર્ણાંક nofollow __attribute__((__unused__)),
		 unsigned પૂર્ણાંક mask __attribute__((__unused__)),
		 unsigned પૂર્ણાંક *retmask)
{
	પ્રતિજ્ઞા(target);
	પ્રતિજ્ઞા(st);

	memset(st, 0, કદ(સંરચના stat));
	જો (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		સંરચના statx stx = { 0 };
		પૂર્ણાંક rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		જો (rc)
			પાછા rc;

		ul_statx_to_stat(&stx, st);

		જો (retmask)
			*retmask = stx.stx_mask;

		જો ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			પાછા -EOPNOTSUPP;
		}

		પાછા 0;
	}
#નહીંતર
	errno = ENOSYS;
	પાછા -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	જો (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	જો (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		અક્ષર *tmpname = ખાલી;

		f = xfmkstemp(&tmpname, ખાલી, "test");
		unlink(tmpname);
		મુક્ત(tmpname);
		fclose(f);

	} નહીંતર જો (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		જો (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} નહીંતર જો (strcmp(argv[1], "--copy-file") == 0) {
		પૂર્ણાંક ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		જો (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		નહીંતર જો (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} નહીંતર જો (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		પૂર્ણાંક flags = O_PATH | O_CLOEXEC;
#નહીંતર
		પૂર્ણાંક flags = O_RDONLY | O_CLOEXEC;
#endif
		પૂર્ણાંક fallback = 0;
		અક્ષર *name;
		પૂર્ણાંક fd, i;

		જો (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		માટે (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			જો (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			નહીંતર જો (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		જો (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		નહીંતર
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		જો (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		મુક્ત(name);
		close(fd);

	} નહીંતર જો (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned પૂર્ણાંક mask = 0, retmask = 0;
		પૂર્ણાંક nofollow = 0;
		સંરચના stat st, ref;
		પૂર્ણાંક rc, i;

		જો (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		માટે (i = 3; i < argc; i++) {
			જો (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			નહીંતર જો (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		જો (rc)
			પાછા EXIT_FAILURE;

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
		જો (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned પૂર્ણાંક emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			જો (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			જો (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			જો (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			જો (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			જો (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			જો (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			જો (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	પાછા EXIT_SUCCESS;
}
#endif
