/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "canonicalize.h"
#അന്തര്ഭാവ "fileutils.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "strutils.h"

പൂർണ്ണസംഖ്യ mkstemp_cloexec(അക്ഷരം *template)
{
#ifdef HAVE_MKOSTEMP
	തിരികെ mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ fd, old_flags, errno_save;

	fd = mkstemp(template);
	എങ്കിൽ (fd < 0)
		തിരികെ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	എങ്കിൽ (old_flags < 0)
		പോകുക unwind;
	എങ്കിൽ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		പോകുക unwind;

	തിരികെ fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	തിരികെ -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
പൂർണ്ണസംഖ്യ xmkstemp(അക്ഷരം **tmpname, സ്ഥിരം അക്ഷരം *dir, സ്ഥിരം അക്ഷരം *prefix)
{
	അക്ഷരം *localtmp;
	സ്ഥിരം അക്ഷരം *tmpenv;
	mode_t old_mode;
	പൂർണ്ണസംഖ്യ fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	എങ്കിൽ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	എങ്കിൽ (rc < 0)
		തിരികെ -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	എങ്കിൽ (fd == -1) {
		സ്വതന്ത്രം(localtmp);
		localtmp = ശൂന്യ;
	}
	*tmpname = localtmp;
	തിരികെ fd;
}

#ifdef F_DUPFD_CLOEXEC
പൂർണ്ണസംഖ്യ dup_fd_cloexec(പൂർണ്ണസംഖ്യ oldfd, പൂർണ്ണസംഖ്യ lowfd)
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ dup_fd_cloexec(പൂർണ്ണസംഖ്യ oldfd, പൂർണ്ണസംഖ്യ lowfd  __attribute__((__unused__)))
#endif
{
	പൂർണ്ണസംഖ്യ fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	എങ്കിൽ (fd >= 0)
		തിരികെ fd;
#endif

	fd = dup(oldfd);
	എങ്കിൽ (fd < 0)
		തിരികെ fd;

	flags = fcntl(fd, F_GETFD);
	എങ്കിൽ (flags < 0)
		പോകുക unwind;
	എങ്കിൽ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		പോകുക unwind;

	തിരികെ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	തിരികെ -1;
}

/*
 * portable getdtablesize()
 */
unsigned പൂർണ്ണസംഖ്യ get_fd_tabsize(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ m;

#എങ്കിൽ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#അല്ലെങ്കിലെങ്കിൽ defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ഘടന rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#അല്ലെങ്കിലെങ്കിൽ defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#അല്ലെങ്കിൽ
	m = OPEN_MAX;
#endif
	തിരികെ m;
}

ശൂന്യം ul_close_all_fds(unsigned പൂർണ്ണസംഖ്യ first, unsigned പൂർണ്ണസംഖ്യ last)
{
	ഘടന dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	എങ്കിൽ (dir) {
		ആകുമ്പോൾ ((d = xreaddir(dir))) {
			അക്ഷരം *end;
			unsigned പൂർണ്ണസംഖ്യ fd;
			പൂർണ്ണസംഖ്യ dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			എങ്കിൽ (errno || end == d->d_name || !end || *end)
				തുടരുക;
			dfd = dirfd(dir);
			എങ്കിൽ (dfd < 0)
				തുടരുക;
			എങ്കിൽ ((unsigned പൂർണ്ണസംഖ്യ)dfd == fd)
				തുടരുക;
			എങ്കിൽ (fd < first || last < fd)
				തുടരുക;
			close(fd);
		}
		closedir(dir);
	} അല്ലെങ്കിൽ {
		unsigned fd, tbsz = get_fd_tabsize();

		വേണ്ടി (fd = 0; fd < tbsz; fd++) {
			എങ്കിൽ (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
അക്ഷരം *ul_restricted_path_oper(സ്ഥിരം അക്ഷരം *path,
		  പൂർണ്ണസംഖ്യ (*oper)(സ്ഥിരം അക്ഷരം *path, അക്ഷരം **result, ശൂന്യം *data),
		  ശൂന്യം *data)
{
	അക്ഷരം *result = ശൂന്യ;
	പൂർണ്ണസംഖ്യ errsv = 0;
	പൂർണ്ണസംഖ്യ pipes[2];
	ssize_t len;
	pid_t pid;

	എങ്കിൽ (!path || !*path)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (pipe(pipes) != 0)
		തിരികെ ശൂന്യ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	തിരഞ്ഞെടുപ്പ് (pid) {
	അവസ്ഥ -1:
		close(pipes[0]);
		close(pipes[1]);
		തിരികെ ശൂന്യ;			/* fork error */
	അവസ്ഥ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		എങ്കിൽ (drop_permissions() != 0)
			result = ശൂന്യ;	/* failed */
		അല്ലെങ്കിൽ
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (അക്ഷരം *) &len, വലുപ്പം(len));
		എങ്കിൽ (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	എങ്കിൽ (ul_read_all(pipes[0], (അക്ഷരം *) &len, വലുപ്പം(len)) != വലുപ്പം(len))
		പോകുക done;
	എങ്കിൽ (len < 0) {
		errsv = -len;
		പോകുക done;
	}

	result = malloc(len + 1);
	എങ്കിൽ (!result) {
		errsv = ENOMEM;
		പോകുക done;
	}
	/* read path */
	എങ്കിൽ (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		പോകുക done;
	}
	result[len] = '\0';
done:
	എങ്കിൽ (errsv) {
		സ്വതന്ത്രം(result);
		result = ശൂന്യ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, ശൂന്യ, 0) );

	errno = errsv;
	തിരികെ result;

}

പൂർണ്ണസംഖ്യ ul_mkdir_p(സ്ഥിരം അക്ഷരം *path, mode_t mode)
{
	അക്ഷരം *p, *dir;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (!path || !*path)
		തിരികെ -EINVAL;

	dir = p = strdup(path);
	എങ്കിൽ (!dir)
		തിരികെ -ENOMEM;

	എങ്കിൽ (*p == '/')
		p++;

	ആകുമ്പോൾ (p && *p) {
		അക്ഷരം *e = strchr(p, '/');
		എങ്കിൽ (e)
			*e = '\0';
		എങ്കിൽ (*p) {
			rc = mkdir(dir, mode);
			എങ്കിൽ (rc && errno != EEXIST)
				നിർത്തുക;
			rc = 0;
		}
		എങ്കിൽ (!e)
			നിർത്തുക;
		*e = '/';
		p = e + 1;
	}

	സ്വതന്ത്രം(dir);
	തിരികെ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
അക്ഷരം *stripoff_last_component(അക്ഷരം *path)
{
	അക്ഷരം *p = path ? strrchr(path, '/') : ശൂന്യ;

	എങ്കിൽ (!p)
		തിരികെ ശൂന്യ;
	*p = '\0';
	തിരികെ p + 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ copy_file_simple(പൂർണ്ണസംഖ്യ from, പൂർണ്ണസംഖ്യ to)
{
	ssize_t nr;
	അക്ഷരം buf[BUFSIZ];

	ആകുമ്പോൾ ((nr = ul_read_all(from, buf, വലുപ്പം(buf))) > 0)
		എങ്കിൽ (ul_write_all(to, buf, nr) == -1)
			തിരികെ UL_COPY_WRITE_ERROR;
	എങ്കിൽ (nr < 0)
		തിരികെ UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, വലുപ്പം(buf));
#endif
	തിരികെ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
പൂർണ്ണസംഖ്യ ul_copy_file(പൂർണ്ണസംഖ്യ from, പൂർണ്ണസംഖ്യ to)
{
#ifdef HAVE_SENDFILE
	ഘടന stat st;
	ssize_t nw;

	എങ്കിൽ (fstat(from, &st) == -1)
		തിരികെ UL_COPY_READ_ERROR;
	എങ്കിൽ (!S_ISREG(st.st_mode))
		തിരികെ copy_file_simple(from, to);
	എങ്കിൽ (ul_sendfile_all(to, from, ശൂന്യ, st.st_size) < 0)
		തിരികെ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ആകുമ്പോൾ ((nw = ul_sendfile_all(to, from, ശൂന്യ, 16*1024*1024)) != 0)
		എങ്കിൽ (nw < 0)
			തിരികെ copy_file_simple(from, to);
	തിരികെ 0;
#അല്ലെങ്കിൽ
	തിരികെ copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
അക്ഷരം *ul_fd_mkpath(അക്ഷരം *buf, size_t bufsz, പൂർണ്ണസംഖ്യ fd)
{
	പൂർണ്ണസംഖ്യ len;

	എങ്കിൽ (fd < 0) {
		errno = EBADF;
		തിരികെ ശൂന്യ;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	എങ്കിൽ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		തിരികെ ശൂന്യ;
	}

	തിരികെ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
അക്ഷരം *ul_fd_get_path(പൂർണ്ണസംഖ്യ fd)
{
	ssize_t ssz;
	അക്ഷരം buf[PATH_MAX];
	അക്ഷരം fdpath[UL_FDPATH_BUFSIZ];

	എങ്കിൽ (!ul_fd_mkpath(fdpath, വലുപ്പം(fdpath), fd))
		തിരികെ ശൂന്യ;

	ssz = readlink(fdpath, buf, വലുപ്പം(buf));
	എങ്കിൽ (ssz < 0)
		തിരികെ ശൂന്യ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	എങ്കിൽ ((size_t) ssz >= വലുപ്പം(buf)) {
		errno = ENAMETOOLONG;
		തിരികെ ശൂന്യ;
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
	എങ്കിൽ (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		തിരികെ ശൂന്യ;
	}

	തിരികെ strdup(buf);
}

പൂർണ്ണസംഖ്യ ul_reopen(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ flags)
{
	അക്ഷരം *path = ul_fd_get_path(fd);
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (!path)
		തിരികെ -errno;

	ret = open(path, flags);
	സ്വതന്ത്രം(path);

	തിരികെ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
അക്ഷരം *ul_basename(അക്ഷരം *path)
{
	അക്ഷരം *p;

	എങ്കിൽ (!path || !*path)
		തിരികെ (അക്ഷരം *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	എങ്കിൽ (!p)
		തിരികെ path;		/* no '/', return original */

	എങ്കിൽ (*(p + 1) != '\0')
		തിരികെ p + 1;		/* begin of the name */

	ആകുമ്പോൾ (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	എങ്കിൽ (p > path) {
		*p-- = '\0';
		ആകുമ്പോൾ (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} അല്ലെങ്കിൽ ആകുമ്പോൾ (*(p + 1) != '\0')
		++p;

	തിരികെ p;
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
FILE *fopen_at_no_link(പൂർണ്ണസംഖ്യ dir, സ്ഥിരം അക്ഷരം *filename,
                             പൂർണ്ണസംഖ്യ flags, mode_t perm, സ്ഥിരം അക്ഷരം *mode)
{
	FILE *fp;
	പൂർണ്ണസംഖ്യ fd;
	ഘടന stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	എങ്കിൽ (fd < 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (fstat(fd, &st)) {
		close(fd);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		തിരികെ ശൂന്യ;
	}

	fp = fdopen(fd, mode);
	എങ്കിൽ (!fp)
		close(fd);
	തിരികെ fp;
}
#endif /* HAVE_OPENAT */

#എങ്കിൽ defined(SYS_openat2)
പൂർണ്ണസംഖ്യ ul_openat_resolve(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ flags,
		      mode_t mode, unsigned long long resolve)
{
	ഘടന open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	തിരികെ syscall(SYS_openat2, dirfd, path, &how, വലുപ്പം(how));
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ ul_openat_resolve(
		പൂർണ്ണസംഖ്യ dirfd __attribute__((__unused__)),
		സ്ഥിരം അക്ഷരം *path __attribute__((__unused__)),
		പൂർണ്ണസംഖ്യ flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	തിരികെ -1;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ open_no_symlinks_fallback(സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ flags, mode_t mode)
{
	അക്ഷരം *abspath = ശൂന്യ, *kpath = ശൂന്യ;
	ഘടന stat st;
	പൂർണ്ണസംഖ്യ fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	എങ്കിൽ (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		തിരികെ -1;
	}

	എങ്കിൽ (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		എങ്കിൽ (!abspath)
			തിരികെ -1;
	}

	എങ്കിൽ (flags & O_PATH)
		fd = open(path, flags);
	അല്ലെങ്കിൽ
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	എങ്കിൽ (fd < 0)
		പോകുക fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	എങ്കിൽ (fstat(fd, &st) != 0)
		പോകുക fail;
	എങ്കിൽ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		പോകുക fail;
	}

	kpath = ul_fd_get_path(fd);
	എങ്കിൽ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		പോകുക fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	എങ്കിൽ (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		പോകുക fail;
	}

	എങ്കിൽ (!(flags & O_PATH)) {
		അക്ഷരം fdpath[UL_FDPATH_BUFSIZ];
		പൂർണ്ണസംഖ്യ nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		എങ്കിൽ (ul_fd_mkpath(fdpath, വലുപ്പം(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		എങ്കിൽ (nfd < 0)
			പോകുക fail;
		close(fd);
		fd = nfd;
	}

	സ്വതന്ത്രം(abspath);
	സ്വതന്ത്രം(kpath);
	തിരികെ fd;
fail:
	errsv = errno;
	സ്വതന്ത്രം(abspath);
	സ്വതന്ത്രം(kpath);
	എങ്കിൽ (fd >= 0)
		close(fd);
	errno = errsv;
	തിരികെ -1;
}
#അല്ലെങ്കിൽ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
സ്ഥിതം പൂർണ്ണസംഖ്യ open_no_symlinks_fallback(
			സ്ഥിരം അക്ഷരം *path __attribute__((__unused__)),
			പൂർണ്ണസംഖ്യ flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	തിരികെ -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
പൂർണ്ണസംഖ്യ ul_open_no_symlinks(സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ flags, mode_t mode)
{
	പൂർണ്ണസംഖ്യ fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	എങ്കിൽ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	തിരികെ fd;
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
പൂർണ്ണസംഖ്യ ul_safe_statx(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ flags,
		  unsigned പൂർണ്ണസംഖ്യ mask, ഘടന statx *stx)
{
	പ്രതിജ്ഞാ(stx);

	memset(stx, 0, വലുപ്പം(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	എങ്കിൽ (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	എങ്കിൽ (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		തിരികെ -errno;

	തിരികെ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
ശൂന്യം ul_statx_to_stat(സ്ഥിരം ഘടന statx *stx, ഘടന stat *st)
{
	memset(st, 0, വലുപ്പം(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	എങ്കിൽ (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	എങ്കിൽ (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	എങ്കിൽ (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	എങ്കിൽ (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	എങ്കിൽ (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	എങ്കിൽ (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	എങ്കിൽ (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	എങ്കിൽ (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	എങ്കിൽ (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	എങ്കിൽ (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	എങ്കിൽ (stx->stx_mask & STATX_CTIME) {
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
പൂർണ്ണസംഖ്യ ul_safe_stat(സ്ഥിരം അക്ഷരം *target, ഘടന stat *st,
		 പൂർണ്ണസംഖ്യ nofollow __attribute__((__unused__)),
		 unsigned പൂർണ്ണസംഖ്യ mask __attribute__((__unused__)),
		 unsigned പൂർണ്ണസംഖ്യ *retmask)
{
	പ്രതിജ്ഞാ(target);
	പ്രതിജ്ഞാ(st);

	memset(st, 0, വലുപ്പം(ഘടന stat));
	എങ്കിൽ (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		ഘടന statx stx = { 0 };
		പൂർണ്ണസംഖ്യ rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		എങ്കിൽ (rc)
			തിരികെ rc;

		ul_statx_to_stat(&stx, st);

		എങ്കിൽ (retmask)
			*retmask = stx.stx_mask;

		എങ്കിൽ ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			തിരികെ -EOPNOTSUPP;
		}

		തിരികെ 0;
	}
#അല്ലെങ്കിൽ
	errno = ENOSYS;
	തിരികെ -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	എങ്കിൽ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	എങ്കിൽ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		അക്ഷരം *tmpname = ശൂന്യ;

		f = xfmkstemp(&tmpname, ശൂന്യ, "test");
		unlink(tmpname);
		സ്വതന്ത്രം(tmpname);
		fclose(f);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		എങ്കിൽ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--copy-file") == 0) {
		പൂർണ്ണസംഖ്യ ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		എങ്കിൽ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		അല്ലെങ്കിൽ എങ്കിൽ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		പൂർണ്ണസംഖ്യ flags = O_PATH | O_CLOEXEC;
#അല്ലെങ്കിൽ
		പൂർണ്ണസംഖ്യ flags = O_RDONLY | O_CLOEXEC;
#endif
		പൂർണ്ണസംഖ്യ fallback = 0;
		അക്ഷരം *name;
		പൂർണ്ണസംഖ്യ fd, i;

		എങ്കിൽ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		വേണ്ടി (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			എങ്കിൽ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		എങ്കിൽ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		അല്ലെങ്കിൽ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		എങ്കിൽ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		സ്വതന്ത്രം(name);
		close(fd);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned പൂർണ്ണസംഖ്യ mask = 0, retmask = 0;
		പൂർണ്ണസംഖ്യ nofollow = 0;
		ഘടന stat st, ref;
		പൂർണ്ണസംഖ്യ rc, i;

		എങ്കിൽ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		വേണ്ടി (i = 3; i < argc; i++) {
			എങ്കിൽ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		എങ്കിൽ (rc)
			തിരികെ EXIT_FAILURE;

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
		എങ്കിൽ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned പൂർണ്ണസംഖ്യ emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			എങ്കിൽ (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			എങ്കിൽ (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			എങ്കിൽ (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			എങ്കിൽ (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			എങ്കിൽ (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			എങ്കിൽ (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			എങ്കിൽ (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	തിരികെ EXIT_SUCCESS;
}
#endif
