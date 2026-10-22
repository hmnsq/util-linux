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

পূর্ণাঙ্ক mkstemp_cloexec(বর্ণ *template)
{
#ifdef HAVE_MKOSTEMP
	প্রত্যাগম mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#অন্যথা
	পূর্ণাঙ্ক fd, old_flags, errno_save;

	fd = mkstemp(template);
	যদি (fd < 0)
		প্রত্যাগম fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	যদি (old_flags < 0)
		গচ্ছ unwind;
	যদি (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		গচ্ছ unwind;

	প্রত্যাগম fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	প্রত্যাগম -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
পূর্ণাঙ্ক xmkstemp(বর্ণ **tmpname, ধ্রুব বর্ণ *dir, ধ্রুব বর্ণ *prefix)
{
	বর্ণ *localtmp;
	ধ্রুব বর্ণ *tmpenv;
	mode_t old_mode;
	পূর্ণাঙ্ক fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	যদি (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	যদি (rc < 0)
		প্রত্যাগম -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	যদি (fd == -1) {
		মুক্ত(localtmp);
		localtmp = রিক্ত;
	}
	*tmpname = localtmp;
	প্রত্যাগম fd;
}

#ifdef F_DUPFD_CLOEXEC
পূর্ণাঙ্ক dup_fd_cloexec(পূর্ণাঙ্ক oldfd, পূর্ণাঙ্ক lowfd)
#অন্যথা
পূর্ণাঙ্ক dup_fd_cloexec(পূর্ণাঙ্ক oldfd, পূর্ণাঙ্ক lowfd  __attribute__((__unused__)))
#endif
{
	পূর্ণাঙ্ক fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	যদি (fd >= 0)
		প্রত্যাগম fd;
#endif

	fd = dup(oldfd);
	যদি (fd < 0)
		প্রত্যাগম fd;

	flags = fcntl(fd, F_GETFD);
	যদি (flags < 0)
		গচ্ছ unwind;
	যদি (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		গচ্ছ unwind;

	প্রত্যাগম fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	প্রত্যাগম -1;
}

/*
 * portable getdtablesize()
 */
unsigned পূর্ণাঙ্ক get_fd_tabsize(শূন্য)
{
	পূর্ণাঙ্ক m;

#যদি defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#নোচেদ্যদি defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	সংরচনা rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#নোচেদ্যদি defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#অন্যথা
	m = OPEN_MAX;
#endif
	প্রত্যাগম m;
}

শূন্য ul_close_all_fds(unsigned পূর্ণাঙ্ক first, unsigned পূর্ণাঙ্ক last)
{
	সংরচনা dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	যদি (dir) {
		যাবত্ ((d = xreaddir(dir))) {
			বর্ণ *end;
			unsigned পূর্ণাঙ্ক fd;
			পূর্ণাঙ্ক dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			যদি (errno || end == d->d_name || !end || *end)
				অনুবর্ত;
			dfd = dirfd(dir);
			যদি (dfd < 0)
				অনুবর্ত;
			যদি ((unsigned পূর্ণাঙ্ক)dfd == fd)
				অনুবর্ত;
			যদি (fd < first || last < fd)
				অনুবর্ত;
			close(fd);
		}
		closedir(dir);
	} অন্যথা {
		unsigned fd, tbsz = get_fd_tabsize();

		প্রতি (fd = 0; fd < tbsz; fd++) {
			যদি (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
বর্ণ *ul_restricted_path_oper(ধ্রুব বর্ণ *path,
		  পূর্ণাঙ্ক (*oper)(ধ্রুব বর্ণ *path, বর্ণ **result, শূন্য *data),
		  শূন্য *data)
{
	বর্ণ *result = রিক্ত;
	পূর্ণাঙ্ক errsv = 0;
	পূর্ণাঙ্ক pipes[2];
	ssize_t len;
	pid_t pid;

	যদি (!path || !*path)
		প্রত্যাগম রিক্ত;

	যদি (pipe(pipes) != 0)
		প্রত্যাগম রিক্ত;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	বিকল্প (pid) {
	পক্ষ -1:
		close(pipes[0]);
		close(pipes[1]);
		প্রত্যাগম রিক্ত;			/* fork error */
	পক্ষ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		যদি (drop_permissions() != 0)
			result = রিক্ত;	/* failed */
		অন্যথা
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (বর্ণ *) &len, পরিমাণ(len));
		যদি (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	স্বভাব:
		ভঙ্গ;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	যদি (ul_read_all(pipes[0], (বর্ণ *) &len, পরিমাণ(len)) != পরিমাণ(len))
		গচ্ছ done;
	যদি (len < 0) {
		errsv = -len;
		গচ্ছ done;
	}

	result = malloc(len + 1);
	যদি (!result) {
		errsv = ENOMEM;
		গচ্ছ done;
	}
	/* read path */
	যদি (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		গচ্ছ done;
	}
	result[len] = '\0';
done:
	যদি (errsv) {
		মুক্ত(result);
		result = রিক্ত;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, রিক্ত, 0) );

	errno = errsv;
	প্রত্যাগম result;

}

পূর্ণাঙ্ক ul_mkdir_p(ধ্রুব বর্ণ *path, mode_t mode)
{
	বর্ণ *p, *dir;
	পূর্ণাঙ্ক rc = 0;

	যদি (!path || !*path)
		প্রত্যাগম -EINVAL;

	dir = p = strdup(path);
	যদি (!dir)
		প্রত্যাগম -ENOMEM;

	যদি (*p == '/')
		p++;

	যাবত্ (p && *p) {
		বর্ণ *e = strchr(p, '/');
		যদি (e)
			*e = '\0';
		যদি (*p) {
			rc = mkdir(dir, mode);
			যদি (rc && errno != EEXIST)
				ভঙ্গ;
			rc = 0;
		}
		যদি (!e)
			ভঙ্গ;
		*e = '/';
		p = e + 1;
	}

	মুক্ত(dir);
	প্রত্যাগম rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
বর্ণ *stripoff_last_component(বর্ণ *path)
{
	বর্ণ *p = path ? strrchr(path, '/') : রিক্ত;

	যদি (!p)
		প্রত্যাগম রিক্ত;
	*p = '\0';
	প্রত্যাগম p + 1;
}

স্থির পূর্ণাঙ্ক copy_file_simple(পূর্ণাঙ্ক from, পূর্ণাঙ্ক to)
{
	ssize_t nr;
	বর্ণ buf[BUFSIZ];

	যাবত্ ((nr = ul_read_all(from, buf, পরিমাণ(buf))) > 0)
		যদি (ul_write_all(to, buf, nr) == -1)
			প্রত্যাগম UL_COPY_WRITE_ERROR;
	যদি (nr < 0)
		প্রত্যাগম UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, পরিমাণ(buf));
#endif
	প্রত্যাগম 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
পূর্ণাঙ্ক ul_copy_file(পূর্ণাঙ্ক from, পূর্ণাঙ্ক to)
{
#ifdef HAVE_SENDFILE
	সংরচনা stat st;
	ssize_t nw;

	যদি (fstat(from, &st) == -1)
		প্রত্যাগম UL_COPY_READ_ERROR;
	যদি (!S_ISREG(st.st_mode))
		প্রত্যাগম copy_file_simple(from, to);
	যদি (ul_sendfile_all(to, from, রিক্ত, st.st_size) < 0)
		প্রত্যাগম copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	যাবত্ ((nw = ul_sendfile_all(to, from, রিক্ত, 16*1024*1024)) != 0)
		যদি (nw < 0)
			প্রত্যাগম copy_file_simple(from, to);
	প্রত্যাগম 0;
#অন্যথা
	প্রত্যাগম copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
বর্ণ *ul_fd_mkpath(বর্ণ *buf, size_t bufsz, পূর্ণাঙ্ক fd)
{
	পূর্ণাঙ্ক len;

	যদি (fd < 0) {
		errno = EBADF;
		প্রত্যাগম রিক্ত;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	যদি (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
বর্ণ *ul_fd_get_path(পূর্ণাঙ্ক fd)
{
	ssize_t ssz;
	বর্ণ buf[PATH_MAX];
	বর্ণ fdpath[UL_FDPATH_BUFSIZ];

	যদি (!ul_fd_mkpath(fdpath, পরিমাণ(fdpath), fd))
		প্রত্যাগম রিক্ত;

	ssz = readlink(fdpath, buf, পরিমাণ(buf));
	যদি (ssz < 0)
		প্রত্যাগম রিক্ত;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	যদি ((size_t) ssz >= পরিমাণ(buf)) {
		errno = ENAMETOOLONG;
		প্রত্যাগম রিক্ত;
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
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম strdup(buf);
}

পূর্ণাঙ্ক ul_reopen(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক flags)
{
	বর্ণ *path = ul_fd_get_path(fd);
	পূর্ণাঙ্ক ret;

	যদি (!path)
		প্রত্যাগম -errno;

	ret = open(path, flags);
	মুক্ত(path);

	প্রত্যাগম ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
বর্ণ *ul_basename(বর্ণ *path)
{
	বর্ণ *p;

	যদি (!path || !*path)
		প্রত্যাগম (বর্ণ *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	যদি (!p)
		প্রত্যাগম path;		/* no '/', return original */

	যদি (*(p + 1) != '\0')
		প্রত্যাগম p + 1;		/* begin of the name */

	যাবত্ (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	যদি (p > path) {
		*p-- = '\0';
		যাবত্ (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} অন্যথা যাবত্ (*(p + 1) != '\0')
		++p;

	প্রত্যাগম p;
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
FILE *fopen_at_no_link(পূর্ণাঙ্ক dir, ধ্রুব বর্ণ *filename,
                             পূর্ণাঙ্ক flags, mode_t perm, ধ্রুব বর্ণ *mode)
{
	FILE *fp;
	পূর্ণাঙ্ক fd;
	সংরচনা stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	যদি (fd < 0)
		প্রত্যাগম রিক্ত;

	যদি (fstat(fd, &st)) {
		close(fd);
		প্রত্যাগম রিক্ত;
	}

	যদি (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		প্রত্যাগম রিক্ত;
	}

	যদি ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		প্রত্যাগম রিক্ত;
	}

	fp = fdopen(fd, mode);
	যদি (!fp)
		close(fd);
	প্রত্যাগম fp;
}
#endif /* HAVE_OPENAT */

#যদি defined(SYS_openat2)
পূর্ণাঙ্ক ul_openat_resolve(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *path, পূর্ণাঙ্ক flags,
		      mode_t mode, unsigned long long resolve)
{
	সংরচনা open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	প্রত্যাগম syscall(SYS_openat2, dirfd, path, &how, পরিমাণ(how));
}
#অন্যথা
পূর্ণাঙ্ক ul_openat_resolve(
		পূর্ণাঙ্ক dirfd __attribute__((__unused__)),
		ধ্রুব বর্ণ *path __attribute__((__unused__)),
		পূর্ণাঙ্ক flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	প্রত্যাগম -1;
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
স্থির পূর্ণাঙ্ক open_no_symlinks_fallback(ধ্রুব বর্ণ *path, পূর্ণাঙ্ক flags, mode_t mode)
{
	বর্ণ *abspath = রিক্ত, *kpath = রিক্ত;
	সংরচনা stat st;
	পূর্ণাঙ্ক fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	যদি (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		প্রত্যাগম -1;
	}

	যদি (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		যদি (!abspath)
			প্রত্যাগম -1;
	}

	যদি (flags & O_PATH)
		fd = open(path, flags);
	অন্যথা
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	যদি (fd < 0)
		গচ্ছ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	যদি (fstat(fd, &st) != 0)
		গচ্ছ fail;
	যদি (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		গচ্ছ fail;
	}

	kpath = ul_fd_get_path(fd);
	যদি (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		গচ্ছ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	যদি (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		গচ্ছ fail;
	}

	যদি (!(flags & O_PATH)) {
		বর্ণ fdpath[UL_FDPATH_BUFSIZ];
		পূর্ণাঙ্ক nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		যদি (ul_fd_mkpath(fdpath, পরিমাণ(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		যদি (nfd < 0)
			গচ্ছ fail;
		close(fd);
		fd = nfd;
	}

	মুক্ত(abspath);
	মুক্ত(kpath);
	প্রত্যাগম fd;
fail:
	errsv = errno;
	মুক্ত(abspath);
	মুক্ত(kpath);
	যদি (fd >= 0)
		close(fd);
	errno = errsv;
	প্রত্যাগম -1;
}
#অন্যথা /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
স্থির পূর্ণাঙ্ক open_no_symlinks_fallback(
			ধ্রুব বর্ণ *path __attribute__((__unused__)),
			পূর্ণাঙ্ক flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	প্রত্যাগম -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
পূর্ণাঙ্ক ul_open_no_symlinks(ধ্রুব বর্ণ *path, পূর্ণাঙ্ক flags, mode_t mode)
{
	পূর্ণাঙ্ক fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	যদি (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	প্রত্যাগম fd;
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
পূর্ণাঙ্ক ul_safe_statx(পূর্ণাঙ্ক dirfd, ধ্রুব বর্ণ *path, পূর্ণাঙ্ক flags,
		  unsigned পূর্ণাঙ্ক mask, সংরচনা statx *stx)
{
	প্রতিজ্ঞা(stx);

	memset(stx, 0, পরিমাণ(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	যদি (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	যদি (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		প্রত্যাগম -errno;

	প্রত্যাগম 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
শূন্য ul_statx_to_stat(ধ্রুব সংরচনা statx *stx, সংরচনা stat *st)
{
	memset(st, 0, পরিমাণ(*st));

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
	যদি (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
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
পূর্ণাঙ্ক ul_safe_stat(ধ্রুব বর্ণ *target, সংরচনা stat *st,
		 পূর্ণাঙ্ক nofollow __attribute__((__unused__)),
		 unsigned পূর্ণাঙ্ক mask __attribute__((__unused__)),
		 unsigned পূর্ণাঙ্ক *retmask)
{
	প্রতিজ্ঞা(target);
	প্রতিজ্ঞা(st);

	memset(st, 0, পরিমাণ(সংরচনা stat));
	যদি (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		সংরচনা statx stx = { 0 };
		পূর্ণাঙ্ক rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		যদি (rc)
			প্রত্যাগম rc;

		ul_statx_to_stat(&stx, st);

		যদি (retmask)
			*retmask = stx.stx_mask;

		যদি ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			প্রত্যাগম -EOPNOTSUPP;
		}

		প্রত্যাগম 0;
	}
#অন্যথা
	errno = ENOSYS;
	প্রত্যাগম -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	যদি (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	যদি (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		বর্ণ *tmpname = রিক্ত;

		f = xfmkstemp(&tmpname, রিক্ত, "test");
		unlink(tmpname);
		মুক্ত(tmpname);
		fclose(f);

	} অন্যথা যদি (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		যদি (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} অন্যথা যদি (strcmp(argv[1], "--copy-file") == 0) {
		পূর্ণাঙ্ক ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		যদি (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		অন্যথা যদি (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} অন্যথা যদি (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		পূর্ণাঙ্ক flags = O_PATH | O_CLOEXEC;
#অন্যথা
		পূর্ণাঙ্ক flags = O_RDONLY | O_CLOEXEC;
#endif
		পূর্ণাঙ্ক fallback = 0;
		বর্ণ *name;
		পূর্ণাঙ্ক fd, i;

		যদি (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		প্রতি (i = 3; i < argc; i++) {
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
		unsigned পূর্ণাঙ্ক mask = 0, retmask = 0;
		পূর্ণাঙ্ক nofollow = 0;
		সংরচনা stat st, ref;
		পূর্ণাঙ্ক rc, i;

		যদি (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		প্রতি (i = 3; i < argc; i++) {
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
			প্রত্যাগম EXIT_FAILURE;

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
			unsigned পূর্ণাঙ্ক emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			যদি (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			যদি (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			যদি (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			যদি (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			যদি (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			যদি (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			যদি (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	প্রত্যাগম EXIT_SUCCESS;
}
#endif
