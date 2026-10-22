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
#اگر_تعریف_شدہ HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#اگر_ختم
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

صحیح mkstemp_cloexec(حرف *سانچہ)
{
#اگر_تعریف_شدہ HAVE_MKOSTEMP
	واپس mkostemp(سانچہ, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#ورنہ
	صحیح fd, old_flags, errno_save;

	fd = mkstemp(سانچہ);
	اگر (fd < 0)
		واپس fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	اگر (old_flags < 0)
		جاؤ unwind;
	اگر (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		جاؤ unwind;

	واپس fd;

unwind:
	errno_save = errno;
	unlink(سانچہ);
	close(fd);
	errno = errno_save;

	واپس -1;
#اگر_ختم
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
صحیح xmkstemp(حرف **tmpname, ثابت حرف *dir, ثابت حرف *سابقہ)
{
	حرف *localtmp;
	ثابت حرف *tmpenv;
	mode_t old_منوال;
	صحیح fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	اگر (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, سابقہ);
	اگر (rc < 0)
		واپس -1;

	old_منوال = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_منوال);
	اگر (fd == -1) {
		آزاد(localtmp);
		localtmp = تہی;
	}
	*tmpname = localtmp;
	واپس fd;
}

#اگر_تعریف_شدہ F_DUPFD_CLOEXEC
صحیح dup_fd_cloexec(صحیح oldfd, صحیح lowfd)
#ورنہ
صحیح dup_fd_cloexec(صحیح oldfd, صحیح lowfd  __attribute__((__unused__)))
#اگر_ختم
{
	صحیح fd, flags, errno_save;

#اگر_تعریف_شدہ F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	اگر (fd >= 0)
		واپس fd;
#اگر_ختم

	fd = dup(oldfd);
	اگر (fd < 0)
		واپس fd;

	flags = fcntl(fd, F_GETFD);
	اگر (flags < 0)
		جاؤ unwind;
	اگر (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		جاؤ unwind;

	واپس fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	واپس -1;
}

/*
 * portable getdtablesize()
 */
غیرعلامتی صحیح get_fd_tabsize(خالی)
{
	صحیح m;

#اگر defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#وگرنہ_اگر defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ساخت rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#وگرنہ_اگر defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#ورنہ
	m = OPEN_MAX;
#اگر_ختم
	واپس m;
}

خالی ul_close_all_fds(غیرعلامتی صحیح پہلا, غیرعلامتی صحیح آخری)
{
	ساخت dirent *d;
	DIR *dir;

	dir = opendir(راستہ_PROC_FDDIR);
	اگر (dir) {
		جب ((d = xreaddir(dir))) {
			حرف *انتہا;
			غیرعلامتی صحیح fd;
			صحیح dfd;

			errno = 0;
			fd = strtoul(d->d_name, &انتہا, 10);

			اگر (errno || انتہا == d->d_name || !انتہا || *انتہا)
				جاری;
			dfd = dirfd(dir);
			اگر (dfd < 0)
				جاری;
			اگر ((غیرعلامتی صحیح)dfd == fd)
				جاری;
			اگر (fd < پہلا || آخری < fd)
				جاری;
			close(fd);
		}
		closedir(dir);
	} ورنہ {
		غیرعلامتی fd, tbsz = get_fd_tabsize();

		برائے (fd = 0; fd < tbsz; fd++) {
			اگر (پہلا <= fd && fd <= آخری)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
حرف *ul_restricted_راستہ_oper(ثابت حرف *راستہ,
		  صحیح (*oper)(ثابت حرف *راستہ, حرف **نتیجہ, خالی *ڈیٹا),
		  خالی *ڈیٹا)
{
	حرف *نتیجہ = تہی;
	صحیح errsv = 0;
	صحیح pipes[2];
	ssize_t len;
	pid_t pid;

	اگر (!راستہ || !*راستہ)
		واپس تہی;

	اگر (pipe(pipes) != 0)
		واپس تہی;
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
		واپس تہی;			/* fork error */
	حالت 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		اگر (drop_permissions() != 0)
			نتیجہ = تہی;	/* failed */
		ورنہ
			oper(راستہ, &نتیجہ, ڈیٹا);

		len = نتیجہ ? (ssize_t) لڑی_لمبائی(نتیجہ) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (حرف *) &len, حجم(len));
		اگر (نتیجہ)
			ul_write_all(pipes[1], نتیجہ, len);
		_exit(0);
	طےشدہ:
		توڑو;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	اگر (ul_read_all(pipes[0], (حرف *) &len, حجم(len)) != حجم(len))
		جاؤ مکمل;
	اگر (len < 0) {
		errsv = -len;
		جاؤ مکمل;
	}

	نتیجہ = حافظہ_دو(len + 1);
	اگر (!نتیجہ) {
		errsv = ENOMEM;
		جاؤ مکمل;
	}
	/* read path */
	اگر (ul_read_all(pipes[0], نتیجہ, len) != len) {
		errsv = errno;
		جاؤ مکمل;
	}
	نتیجہ[len] = '\0';
مکمل:
	اگر (errsv) {
		آزاد(نتیجہ);
		نتیجہ = تہی;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_نتیجہ( waitpid(pid, تہی, 0) );

	errno = errsv;
	واپس نتیجہ;

}

صحیح ul_mkdir_p(ثابت حرف *راستہ, mode_t منوال)
{
	حرف *p, *dir;
	صحیح rc = 0;

	اگر (!راستہ || !*راستہ)
		واپس -EINVAL;

	dir = p = strdup(راستہ);
	اگر (!dir)
		واپس -ENOMEM;

	اگر (*p == '/')
		p++;

	جب (p && *p) {
		حرف *e = strchr(p, '/');
		اگر (e)
			*e = '\0';
		اگر (*p) {
			rc = mkdir(dir, منوال);
			اگر (rc && errno != EEXIST)
				توڑو;
			rc = 0;
		}
		اگر (!e)
			توڑو;
		*e = '/';
		p = e + 1;
	}

	آزاد(dir);
	واپس rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
حرف *stripoff_آخری_component(حرف *راستہ)
{
	حرف *p = راستہ ? strrchr(راستہ, '/') : تہی;

	اگر (!p)
		واپس تہی;
	*p = '\0';
	واپس p + 1;
}

ساکن صحیح copy_file_simple(صحیح from, صحیح to)
{
	ssize_t nr;
	حرف buf[BUFSIZ];

	جب ((nr = ul_read_all(from, buf, حجم(buf))) > 0)
		اگر (ul_write_all(to, buf, nr) == -1)
			واپس UL_COPY_WRITE_ERROR;
	اگر (nr < 0)
		واپس UL_COPY_READ_ERROR;
#اگر_تعریف_شدہ HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, حجم(buf));
#اگر_ختم
	واپس 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
صحیح ul_copy_file(صحیح from, صحیح to)
{
#اگر_تعریف_شدہ HAVE_SENDFILE
	ساخت stat st;
	ssize_t nw;

	اگر (fstat(from, &st) == -1)
		واپس UL_COPY_READ_ERROR;
	اگر (!S_ISREG(st.st_mode))
		واپس copy_file_simple(from, to);
	اگر (ul_sendfile_all(to, from, تہی, st.st_size) < 0)
		واپس copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	جب ((nw = ul_sendfile_all(to, from, تہی, 16*1024*1024)) != 0)
		اگر (nw < 0)
			واپس copy_file_simple(from, to);
	واپس 0;
#ورنہ
	واپس copy_file_simple(from, to);
#اگر_ختم
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
حرف *ul_fd_mkpath(حرف *buf, size_t bufsz, صحیح fd)
{
	صحیح len;

	اگر (fd < 0) {
		errno = EBADF;
		واپس تہی;
	}

	len = snprintf(buf, bufsz, راستہ_PROC_FDDIR "/%d", fd);
	اگر (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		واپس تہی;
	}

	واپس buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
حرف *ul_fd_get_راستہ(صحیح fd)
{
	ssize_t ssz;
	حرف buf[PATH_MAX];
	حرف fdpath[UL_FDPATH_BUFSIZ];

	اگر (!ul_fd_mkpath(fdpath, حجم(fdpath), fd))
		واپس تہی;

	ssz = readlink(fdpath, buf, حجم(buf));
	اگر (ssz < 0)
		واپس تہی;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	اگر ((size_t) ssz >= حجم(buf)) {
		errno = ENAMETOOLONG;
		واپس تہی;
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
	اگر (*buf != '/' || ul_endswith(buf, راستہ_DELETED_SUFFIX)) {
		errno = ENOENT;
		واپس تہی;
	}

	واپس strdup(buf);
}

صحیح ul_reopen(صحیح fd, صحیح flags)
{
	حرف *راستہ = ul_fd_get_راستہ(fd);
	صحیح ret;

	اگر (!راستہ)
		واپس -errno;

	ret = open(راستہ, flags);
	آزاد(راستہ);

	واپس ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
حرف *ul_basename(حرف *راستہ)
{
	حرف *p;

	اگر (!راستہ || !*راستہ)
		واپس (حرف *) ".";	/* ugly, static string */

	p = strrchr(راستہ, '/');
	اگر (!p)
		واپس راستہ;		/* no '/', return original */

	اگر (*(p + 1) != '\0')
		واپس p + 1;		/* begin of the name */

	جب (p > راستہ && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	اگر (p > راستہ) {
		*p-- = '\0';
		جب (p > راستہ && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} ورنہ جب (*(p + 1) != '\0')
		++p;

	واپس p;
}

#اگر_تعریف_شدہ HAVE_OPENAT
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
FILE *fopen_at_no_link(صحیح dir, ثابت حرف *filename,
                             صحیح flags, mode_t perm, ثابت حرف *منوال)
{
	FILE *fp;
	صحیح fd;
	ساخت stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	اگر (fd < 0)
		واپس تہی;

	اگر (fstat(fd, &st)) {
		close(fd);
		واپس تہی;
	}

	اگر (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		واپس تہی;
	}

	اگر ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		واپس تہی;
	}

	fp = fdopen(fd, منوال);
	اگر (!fp)
		close(fd);
	واپس fp;
}
#اگر_ختم /* HAVE_OPENAT */

#اگر defined(SYS_openat2)
صحیح ul_openat_resolve(صحیح dirfd, ثابت حرف *راستہ, صحیح flags,
		      mode_t منوال, غیرعلامتی طویل طویل resolve)
{
	ساخت open_how how = {
		.flags = (__u64) flags,
		.منوال = (__u64) منوال,
		.resolve = resolve,
	};

	واپس syscall(SYS_openat2, dirfd, راستہ, &how, حجم(how));
}
#ورنہ
صحیح ul_openat_resolve(
		صحیح dirfd __attribute__((__unused__)),
		ثابت حرف *راستہ __attribute__((__unused__)),
		صحیح flags __attribute__((__unused__)),
		mode_t منوال __attribute__((__unused__)),
		غیرعلامتی طویل طویل resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	واپس -1;
}
#اگر_ختم

#اگر_تعریف_شدہ __linux__
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
ساکن صحیح open_no_symlinks_fallback(ثابت حرف *راستہ, صحیح flags, mode_t منوال)
{
	حرف *abspath = تہی, *kpath = تہی;
	ساخت stat st;
	صحیح fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	اگر (!راستہ || (flags & O_CREAT)) {
		errno = ENOSYS;
		واپس -1;
	}

	اگر (ul_is_relative_راستہ(راستہ)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_راستہ(راستہ);
		اگر (!abspath)
			واپس -1;
	}

	اگر (flags & O_راستہ)
		fd = open(راستہ, flags);
	ورنہ
		fd = open(راستہ, O_راستہ | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	اگر (fd < 0)
		جاؤ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	اگر (fstat(fd, &st) != 0)
		جاؤ fail;
	اگر (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		جاؤ fail;
	}

	kpath = ul_fd_get_راستہ(fd);
	اگر (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		جاؤ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	اگر (streq_paths(abspath ? abspath : راستہ, kpath) != 1) {
		errno = ELOOP;
		جاؤ fail;
	}

	اگر (!(flags & O_راستہ)) {
		حرف fdpath[UL_FDPATH_BUFSIZ];
		صحیح nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		اگر (ul_fd_mkpath(fdpath, حجم(fdpath), fd))
			nfd = open(fdpath, flags, منوال);
		اگر (nfd < 0)
			جاؤ fail;
		close(fd);
		fd = nfd;
	}

	آزاد(abspath);
	آزاد(kpath);
	واپس fd;
fail:
	errsv = errno;
	آزاد(abspath);
	آزاد(kpath);
	اگر (fd >= 0)
		close(fd);
	errno = errsv;
	واپس -1;
}
#ورنہ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ساکن صحیح open_no_symlinks_fallback(
			ثابت حرف *راستہ __attribute__((__unused__)),
			صحیح flags __attribute__((__unused__)),
			mode_t منوال __attribute__((__unused__)))
{
	errno = ENOSYS;
	واپس -1;
}
#اگر_ختم /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
صحیح ul_open_no_symlinks(ثابت حرف *راستہ, صحیح flags, mode_t منوال)
{
	صحیح fd = ul_openat_resolve(AT_FDCWD, راستہ, flags, منوال,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	اگر (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(راستہ, flags, منوال);

	واپس fd;
}

#اگر_تعریف_شدہ HAVE_UL_محفوظ_STATX
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
صحیح ul_محفوظ_statx(صحیح dirfd, ثابت حرف *راستہ, صحیح flags,
		  غیرعلامتی صحیح نقاب, ساخت statx *stx)
{
	أكد(stx);

	memset(stx, 0, حجم(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#اگر_تعریف_شدہ AT_خالی_خانے_راستہ
	اگر (!راستہ || !*راستہ)
		flags |= AT_خالی_خانے_راستہ;
#اگر_ختم
	اگر (statx(dirfd, راستہ ? راستہ : "", flags, نقاب, stx) != 0)
		واپس -errno;

	واپس 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
خالی ul_statx_مقصد_stat(ثابت ساخت statx *stx, ساخت stat *st)
{
	memset(st, 0, حجم(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	اگر (stx->stx_نقاب & STATX_TYPE)
		st->st_mode |= stx->stx_منوال & S_IFMT;
	اگر (stx->stx_نقاب & STATX_منوال)
		st->st_mode |= stx->stx_منوال & ~S_IFMT;

	اگر (stx->stx_نقاب & STATX_INO)
		st->st_ino = stx->stx_ino;
	اگر (stx->stx_نقاب & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	اگر (stx->stx_نقاب & STATX_UID)
		st->st_uid = stx->stx_uid;
	اگر (stx->stx_نقاب & STATX_GID)
		st->st_gid = stx->stx_gid;
	اگر (stx->stx_نقاب & STATX_سائز)
		st->st_size = stx->stx_سائز;
	اگر (stx->stx_نقاب & STATX_بلاکیں)
		st->st_blocks = stx->stx_بلاکیں;

	اگر (stx->stx_نقاب & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	اگر (stx->stx_نقاب & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	اگر (stx->stx_نقاب & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#اگر_ختم

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
صحیح ul_محفوظ_stat(ثابت حرف *ہدف, ساخت stat *st,
		 صحیح nofollow __attribute__((__unused__)),
		 غیرعلامتی صحیح نقاب __attribute__((__unused__)),
		 غیرعلامتی صحیح *retmask)
{
	أكد(ہدف);
	أكد(st);

	memset(st, 0, حجم(ساخت stat));
	اگر (retmask)
		*retmask = 0;

#اگر_تعریف_شدہ HAVE_UL_محفوظ_STAT
	{
		ساخت statx stx = { 0 };
		صحیح rc;

		نقاب |= UL_STATX_ESSENTIAL;

		rc = ul_محفوظ_statx(AT_FDCWD, ہدف,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, نقاب, &stx);
		اگر (rc)
			واپس rc;

		ul_statx_مقصد_stat(&stx, st);

		اگر (retmask)
			*retmask = stx.stx_نقاب;

		اگر ((stx.stx_نقاب & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			واپس -EOPNOTSUPP;
		}

		واپس 0;
	}
#ورنہ
	errno = ENOSYS;
	واپس -ENOSYS;
#اگر_ختم
}


#اگر_تعریف_شدہ TEST_پروگرام_FILEUTILS
صحیح مین(صحیح argc, حرف *argv[])
{
	اگر (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	اگر (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		حرف *tmpname = تہی;

		f = xfmkstemp(&tmpname, تہی, "test");
		unlink(tmpname);
		آزاد(tmpname);
		fclose(f);

	} ورنہ اگر (strcmp(argv[1], "--close-fds") == 0) {
		ignore_نتیجہ( dup(STDIN_FILENO) );
		ignore_نتیجہ( dup(STDIN_FILENO) );
		ignore_نتیجہ( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		اگر (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} ورنہ اگر (strcmp(argv[1], "--copy-file") == 0) {
		صحیح ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		اگر (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		ورنہ اگر (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} ورنہ اگر (strcmp(argv[1], "--open-no-symlinks") == 0) {
#اگر_تعریف_شدہ __linux__
		صحیح flags = O_راستہ | O_CLOEXEC;
#ورنہ
		صحیح flags = O_RDONLY | O_CLOEXEC;
#اگر_ختم
		صحیح fallback = 0;
		حرف *name;
		صحیح fd, i;

		اگر (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		برائے (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			اگر (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			ورنہ اگر (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		اگر (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		ورنہ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		اگر (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_راستہ(fd);
		لکھو("%s\n", name);
		آزاد(name);
		close(fd);

	} ورنہ اگر (strcmp(argv[1], "--safe-stat") == 0) {
		غیرعلامتی صحیح نقاب = 0, retmask = 0;
		صحیح nofollow = 0;
		ساخت stat st, ref;
		صحیح rc, i;

		اگر (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		برائے (i = 3; i < argc; i++) {
			اگر (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			ورنہ اگر (strcmp(argv[i], "--basic") == 0)
				نقاب |= UL_STATX_BASIC;
		}

		لکھو("%s:\n", argv[2]);

		rc = ul_محفوظ_stat(argv[2], &st, nofollow, نقاب, &retmask);

		/* the function returns -errno and it sets errno too */
		لکھو("      rc: %d\n", rc);
		لکھو("   errno: %d\n", rc ? errno : 0);
		اگر (rc)
			واپس EXIT_FAILURE;

		لکھو("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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

		لکھو("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		لکھو(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#اگر_تعریف_شدہ HAVE_UL_محفوظ_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			غیرعلامتی صحیح emask = نقاب | UL_STATX_ESSENTIAL;

# define نتیجہ_2(ثنائی_ہندسہ, x)	(!(retmask & (ثنائی_ہندسہ)) || (x) ? "OK" : "FAILED")

			اگر (emask & STATX_TYPE)
				لکھو("  S_IFMT: %s\n", نتیجہ_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			اگر (emask & STATX_منوال)
				لکھو(" st_mode: %s\n", نتیجہ_2(STATX_منوال,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			اگر (emask & STATX_INO)
				لکھو("  st_ino: %s\n",
					نتیجہ_2(STATX_INO, st.st_ino == ref.st_ino));
			اگر (emask & STATX_NLINK)
				لکھو("st_nlink: %s\n",
					نتیجہ_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			اگر (emask & STATX_UID)
				لکھو("  st_uid: %s\n",
					نتیجہ_2(STATX_UID, st.st_uid == ref.st_uid));
			اگر (emask & STATX_GID)
				لکھو("  st_gid: %s\n",
					نتیجہ_2(STATX_GID, st.st_gid == ref.st_gid));
			اگر (emask & STATX_سائز)
				لکھو(" st_size: %s\n",
					نتیجہ_2(STATX_سائز, st.st_size == ref.st_size));
# undef نتیجہ_2
		}
#اگر_ختم
	}
	واپس EXIT_SUCCESS;
}
#اگر_ختم
