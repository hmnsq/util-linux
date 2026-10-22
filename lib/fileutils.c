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
#اگر_تعریف_شده HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#پایان_اگر
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

صحيح mkstemp_cloexec(حرف *قالب)
{
#اگر_تعریف_شده HAVE_MKOSTEMP
	أرجع mkostemp(قالب, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#وإلا
	صحيح fd, old_flags, errno_save;

	fd = mkstemp(قالب);
	إذا (fd < 0)
		أرجع fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	إذا (old_flags < 0)
		اذهب unwind;
	إذا (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		اذهب unwind;

	أرجع fd;

unwind:
	errno_save = errno;
	unlink(قالب);
	close(fd);
	errno = errno_save;

	أرجع -1;
#پایان_اگر
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
صحيح xmkstemp(حرف **tmpname, ثابت حرف *dir, ثابت حرف *پیشوند)
{
	حرف *localtmp;
	ثابت حرف *tmpenv;
	mode_t old_نما;
	صحيح fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	إذا (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, پیشوند);
	إذا (rc < 0)
		أرجع -1;

	old_نما = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_نما);
	إذا (fd == -1) {
		حرر(localtmp);
		localtmp = عدم;
	}
	*tmpname = localtmp;
	أرجع fd;
}

#اگر_تعریف_شده F_DUPFD_CLOEXEC
صحيح dup_fd_cloexec(صحيح oldfd, صحيح lowfd)
#وإلا
صحيح dup_fd_cloexec(صحيح oldfd, صحيح lowfd  __attribute__((__unused__)))
#پایان_اگر
{
	صحيح fd, flags, errno_save;

#اگر_تعریف_شده F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	إذا (fd >= 0)
		أرجع fd;
#پایان_اگر

	fd = dup(oldfd);
	إذا (fd < 0)
		أرجع fd;

	flags = fcntl(fd, F_GETFD);
	إذا (flags < 0)
		اذهب unwind;
	إذا (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
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
بیعلامت صحيح get_fd_tabsize(فراغ)
{
	صحيح m;

#إذا defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#وإلاإذا defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	بنية rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#وإلاإذا defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#وإلا
	m = OPEN_MAX;
#پایان_اگر
	أرجع m;
}

فراغ ul_close_all_fds(بیعلامت صحيح اولی, بیعلامت صحيح آخرین)
{
	بنية dirent *d;
	DIR *dir;

	dir = opendir(مسیر_PROC_FDDIR);
	إذا (dir) {
		طالما ((d = xreaddir(dir))) {
			حرف *انتها;
			بیعلامت صحيح fd;
			صحيح dfd;

			errno = 0;
			fd = strtoul(d->d_name, &انتها, 10);

			إذا (errno || انتها == d->d_name || !انتها || *انتها)
				استمر;
			dfd = dirfd(dir);
			إذا (dfd < 0)
				استمر;
			إذا ((بیعلامت صحيح)dfd == fd)
				استمر;
			إذا (fd < اولی || آخرین < fd)
				استمر;
			close(fd);
		}
		closedir(dir);
	} وإلا {
		بیعلامت fd, tbsz = get_fd_tabsize();

		لكل (fd = 0; fd < tbsz; fd++) {
			إذا (اولی <= fd && fd <= آخرین)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
حرف *ul_restricted_مسیر_oper(ثابت حرف *مسیر,
		  صحيح (*oper)(ثابت حرف *مسیر, حرف **نتیجه, فراغ *داده),
		  فراغ *داده)
{
	حرف *نتیجه = عدم;
	صحيح errsv = 0;
	صحيح pipes[2];
	ssize_t len;
	pid_t pid;

	إذا (!مسیر || !*مسیر)
		أرجع عدم;

	إذا (pipe(pipes) != 0)
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

		إذا (drop_permissions() != 0)
			نتیجه = عدم;	/* failed */
		وإلا
			oper(مسیر, &نتیجه, داده);

		len = نتیجه ? (ssize_t) طول_رشته(نتیجه) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (حرف *) &len, حجم(len));
		إذا (نتیجه)
			ul_write_all(pipes[1], نتیجه, len);
		_exit(0);
	افتراضي:
		اكسر;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	إذا (ul_read_all(pipes[0], (حرف *) &len, حجم(len)) != حجم(len))
		اذهب انجامشده;
	إذا (len < 0) {
		errsv = -len;
		اذهب انجامشده;
	}

	نتیجه = حافظه_بده(len + 1);
	إذا (!نتیجه) {
		errsv = ENOMEM;
		اذهب انجامشده;
	}
	/* read path */
	إذا (ul_read_all(pipes[0], نتیجه, len) != len) {
		errsv = errno;
		اذهب انجامشده;
	}
	نتیجه[len] = '\0';
انجامشده:
	إذا (errsv) {
		حرر(نتیجه);
		نتیجه = عدم;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_نتیجه( waitpid(pid, عدم, 0) );

	errno = errsv;
	أرجع نتیجه;

}

صحيح ul_mkdir_p(ثابت حرف *مسیر, mode_t نما)
{
	حرف *p, *dir;
	صحيح rc = 0;

	إذا (!مسیر || !*مسیر)
		أرجع -EINVAL;

	dir = p = strdup(مسیر);
	إذا (!dir)
		أرجع -ENOMEM;

	إذا (*p == '/')
		p++;

	طالما (p && *p) {
		حرف *e = strchr(p, '/');
		إذا (e)
			*e = '\0';
		إذا (*p) {
			rc = mkdir(dir, نما);
			إذا (rc && errno != EEXIST)
				اكسر;
			rc = 0;
		}
		إذا (!e)
			اكسر;
		*e = '/';
		p = e + 1;
	}

	حرر(dir);
	أرجع rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
حرف *stripoff_آخرین_component(حرف *مسیر)
{
	حرف *p = مسیر ? strrchr(مسیر, '/') : عدم;

	إذا (!p)
		أرجع عدم;
	*p = '\0';
	أرجع p + 1;
}

ساكن صحيح copy_file_simple(صحيح from, صحيح to)
{
	ssize_t nr;
	حرف buf[BUFSIZ];

	طالما ((nr = ul_read_all(from, buf, حجم(buf))) > 0)
		إذا (ul_write_all(to, buf, nr) == -1)
			أرجع UL_COPY_WRITE_ERROR;
	إذا (nr < 0)
		أرجع UL_COPY_READ_ERROR;
#اگر_تعریف_شده HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, حجم(buf));
#پایان_اگر
	أرجع 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
صحيح ul_copy_file(صحيح from, صحيح to)
{
#اگر_تعریف_شده HAVE_SENDFILE
	بنية stat st;
	ssize_t nw;

	إذا (fstat(from, &st) == -1)
		أرجع UL_COPY_READ_ERROR;
	إذا (!S_ISREG(st.st_mode))
		أرجع copy_file_simple(from, to);
	إذا (ul_sendfile_all(to, from, عدم, st.st_size) < 0)
		أرجع copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	طالما ((nw = ul_sendfile_all(to, from, عدم, 16*1024*1024)) != 0)
		إذا (nw < 0)
			أرجع copy_file_simple(from, to);
	أرجع 0;
#وإلا
	أرجع copy_file_simple(from, to);
#پایان_اگر
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

	إذا (fd < 0) {
		errno = EBADF;
		أرجع عدم;
	}

	len = snprintf(buf, bufsz, مسیر_PROC_FDDIR "/%d", fd);
	إذا (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
حرف *ul_fd_get_مسیر(صحيح fd)
{
	ssize_t ssz;
	حرف buf[PATH_MAX];
	حرف fdpath[UL_FDPATH_BUFSIZ];

	إذا (!ul_fd_mkpath(fdpath, حجم(fdpath), fd))
		أرجع عدم;

	ssz = readlink(fdpath, buf, حجم(buf));
	إذا (ssz < 0)
		أرجع عدم;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	إذا ((size_t) ssz >= حجم(buf)) {
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
	إذا (*buf != '/' || ul_endswith(buf, مسیر_DELETED_SUFFIX)) {
		errno = ENOENT;
		أرجع عدم;
	}

	أرجع strdup(buf);
}

صحيح ul_reopen(صحيح fd, صحيح flags)
{
	حرف *مسیر = ul_fd_get_مسیر(fd);
	صحيح ret;

	إذا (!مسیر)
		أرجع -errno;

	ret = open(مسیر, flags);
	حرر(مسیر);

	أرجع ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
حرف *ul_basename(حرف *مسیر)
{
	حرف *p;

	إذا (!مسیر || !*مسیر)
		أرجع (حرف *) ".";	/* ugly, static string */

	p = strrchr(مسیر, '/');
	إذا (!p)
		أرجع مسیر;		/* no '/', return original */

	إذا (*(p + 1) != '\0')
		أرجع p + 1;		/* begin of the name */

	طالما (p > مسیر && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	إذا (p > مسیر) {
		*p-- = '\0';
		طالما (p > مسیر && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} وإلا طالما (*(p + 1) != '\0')
		++p;

	أرجع p;
}

#اگر_تعریف_شده HAVE_OPENAT
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
                             صحيح flags, mode_t perm, ثابت حرف *نما)
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
	إذا (fd < 0)
		أرجع عدم;

	إذا (fstat(fd, &st)) {
		close(fd);
		أرجع عدم;
	}

	إذا (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		أرجع عدم;
	}

	إذا ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		أرجع عدم;
	}

	fp = fdopen(fd, نما);
	إذا (!fp)
		close(fd);
	أرجع fp;
}
#پایان_اگر /* HAVE_OPENAT */

#إذا defined(SYS_openat2)
صحيح ul_openat_resolve(صحيح dirfd, ثابت حرف *مسیر, صحيح flags,
		      mode_t نما, بیعلامت دراز دراز resolve)
{
	بنية open_how how = {
		.flags = (__u64) flags,
		.نما = (__u64) نما,
		.resolve = resolve,
	};

	أرجع syscall(SYS_openat2, dirfd, مسیر, &how, حجم(how));
}
#وإلا
صحيح ul_openat_resolve(
		صحيح dirfd __attribute__((__unused__)),
		ثابت حرف *مسیر __attribute__((__unused__)),
		صحيح flags __attribute__((__unused__)),
		mode_t نما __attribute__((__unused__)),
		بیعلامت دراز دراز resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#پایان_اگر

#اگر_تعریف_شده __linux__
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
ساكن صحيح open_no_symlinks_fallback(ثابت حرف *مسیر, صحيح flags, mode_t نما)
{
	حرف *abspath = عدم, *kpath = عدم;
	بنية stat st;
	صحيح fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	إذا (!مسیر || (flags & O_CREAT)) {
		errno = ENOSYS;
		أرجع -1;
	}

	إذا (ul_is_relative_مسیر(مسیر)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_مسیر(مسیر);
		إذا (!abspath)
			أرجع -1;
	}

	إذا (flags & O_مسیر)
		fd = open(مسیر, flags);
	وإلا
		fd = open(مسیر, O_مسیر | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	إذا (fd < 0)
		اذهب fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	إذا (fstat(fd, &st) != 0)
		اذهب fail;
	إذا (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		اذهب fail;
	}

	kpath = ul_fd_get_مسیر(fd);
	إذا (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		اذهب fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	إذا (streq_paths(abspath ? abspath : مسیر, kpath) != 1) {
		errno = ELOOP;
		اذهب fail;
	}

	إذا (!(flags & O_مسیر)) {
		حرف fdpath[UL_FDPATH_BUFSIZ];
		صحيح nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		إذا (ul_fd_mkpath(fdpath, حجم(fdpath), fd))
			nfd = open(fdpath, flags, نما);
		إذا (nfd < 0)
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
	إذا (fd >= 0)
		close(fd);
	errno = errsv;
	أرجع -1;
}
#وإلا /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ساكن صحيح open_no_symlinks_fallback(
			ثابت حرف *مسیر __attribute__((__unused__)),
			صحيح flags __attribute__((__unused__)),
			mode_t نما __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#پایان_اگر /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
صحيح ul_open_no_symlinks(ثابت حرف *مسیر, صحيح flags, mode_t نما)
{
	صحيح fd = ul_openat_resolve(AT_FDCWD, مسیر, flags, نما,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	إذا (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(مسیر, flags, نما);

	أرجع fd;
}

#اگر_تعریف_شده HAVE_UL_امن_STATX
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
صحيح ul_امن_statx(صحيح dirfd, ثابت حرف *مسیر, صحيح flags,
		  بیعلامت صحيح ماسک, بنية statx *stx)
{
	أكد(stx);

	memset(stx, 0, حجم(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#اگر_تعریف_شده AT_خالی_مسیر
	إذا (!مسیر || !*مسیر)
		flags |= AT_خالی_مسیر;
#پایان_اگر
	إذا (statx(dirfd, مسیر ? مسیر : "", flags, ماسک, stx) != 0)
		أرجع -errno;

	أرجع 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
فراغ ul_statx_مقصد_stat(ثابت بنية statx *stx, بنية stat *st)
{
	memset(st, 0, حجم(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	إذا (stx->stx_ماسک & STATX_TYPE)
		st->st_mode |= stx->stx_نما & S_IFMT;
	إذا (stx->stx_ماسک & STATX_نما)
		st->st_mode |= stx->stx_نما & ~S_IFMT;

	إذا (stx->stx_ماسک & STATX_INO)
		st->st_ino = stx->stx_ino;
	إذا (stx->stx_ماسک & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	إذا (stx->stx_ماسک & STATX_UID)
		st->st_uid = stx->stx_uid;
	إذا (stx->stx_ماسک & STATX_GID)
		st->st_gid = stx->stx_gid;
	إذا (stx->stx_ماسک & STATX_اندازه)
		st->st_size = stx->stx_اندازه;
	إذا (stx->stx_ماسک & STATX_بلوکها)
		st->st_blocks = stx->stx_بلوکها;

	إذا (stx->stx_ماسک & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	إذا (stx->stx_ماسک & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	إذا (stx->stx_ماسک & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#پایان_اگر

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
صحيح ul_امن_stat(ثابت حرف *هدف, بنية stat *st,
		 صحيح nofollow __attribute__((__unused__)),
		 بیعلامت صحيح ماسک __attribute__((__unused__)),
		 بیعلامت صحيح *retmask)
{
	أكد(هدف);
	أكد(st);

	memset(st, 0, حجم(بنية stat));
	إذا (retmask)
		*retmask = 0;

#اگر_تعریف_شده HAVE_UL_امن_STAT
	{
		بنية statx stx = { 0 };
		صحيح rc;

		ماسک |= UL_STATX_ESSENTIAL;

		rc = ul_امن_statx(AT_FDCWD, هدف,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, ماسک, &stx);
		إذا (rc)
			أرجع rc;

		ul_statx_مقصد_stat(&stx, st);

		إذا (retmask)
			*retmask = stx.stx_ماسک;

		إذا ((stx.stx_ماسک & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			أرجع -EOPNOTSUPP;
		}

		أرجع 0;
	}
#وإلا
	errno = ENOSYS;
	أرجع -ENOSYS;
#پایان_اگر
}


#اگر_تعریف_شده TEST_برنامه_FILEUTILS
صحيح اصلی(صحيح argc, حرف *argv[])
{
	إذا (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	إذا (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		حرف *tmpname = عدم;

		f = xfmkstemp(&tmpname, عدم, "test");
		unlink(tmpname);
		حرر(tmpname);
		fclose(f);

	} وإلا إذا (strcmp(argv[1], "--close-fds") == 0) {
		ignore_نتیجه( dup(STDIN_FILENO) );
		ignore_نتیجه( dup(STDIN_FILENO) );
		ignore_نتیجه( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		إذا (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} وإلا إذا (strcmp(argv[1], "--copy-file") == 0) {
		صحيح ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		إذا (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		وإلا إذا (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} وإلا إذا (strcmp(argv[1], "--open-no-symlinks") == 0) {
#اگر_تعریف_شده __linux__
		صحيح flags = O_مسیر | O_CLOEXEC;
#وإلا
		صحيح flags = O_RDONLY | O_CLOEXEC;
#پایان_اگر
		صحيح fallback = 0;
		حرف *name;
		صحيح fd, i;

		إذا (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		لكل (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			إذا (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			وإلا إذا (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		إذا (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		وإلا
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		إذا (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_مسیر(fd);
		چاپ_کن("%s\n", name);
		حرر(name);
		close(fd);

	} وإلا إذا (strcmp(argv[1], "--safe-stat") == 0) {
		بیعلامت صحيح ماسک = 0, retmask = 0;
		صحيح nofollow = 0;
		بنية stat st, ref;
		صحيح rc, i;

		إذا (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		لكل (i = 3; i < argc; i++) {
			إذا (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			وإلا إذا (strcmp(argv[i], "--basic") == 0)
				ماسک |= UL_STATX_BASIC;
		}

		چاپ_کن("%s:\n", argv[2]);

		rc = ul_امن_stat(argv[2], &st, nofollow, ماسک, &retmask);

		/* the function returns -errno and it sets errno too */
		چاپ_کن("      rc: %d\n", rc);
		چاپ_کن("   errno: %d\n", rc ? errno : 0);
		إذا (rc)
			أرجع EXIT_FAILURE;

		چاپ_کن("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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
		إذا (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(EXIT_FAILURE, "%s", argv[2]);

		چاپ_کن("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		چاپ_کن(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#اگر_تعریف_شده HAVE_UL_امن_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			بیعلامت صحيح emask = ماسک | UL_STATX_ESSENTIAL;

# define نتیجه_2(بیت, x)	(!(retmask & (بیت)) || (x) ? "OK" : "FAILED")

			إذا (emask & STATX_TYPE)
				چاپ_کن("  S_IFMT: %s\n", نتیجه_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			إذا (emask & STATX_نما)
				چاپ_کن(" st_mode: %s\n", نتیجه_2(STATX_نما,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			إذا (emask & STATX_INO)
				چاپ_کن("  st_ino: %s\n",
					نتیجه_2(STATX_INO, st.st_ino == ref.st_ino));
			إذا (emask & STATX_NLINK)
				چاپ_کن("st_nlink: %s\n",
					نتیجه_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			إذا (emask & STATX_UID)
				چاپ_کن("  st_uid: %s\n",
					نتیجه_2(STATX_UID, st.st_uid == ref.st_uid));
			إذا (emask & STATX_GID)
				چاپ_کن("  st_gid: %s\n",
					نتیجه_2(STATX_GID, st.st_gid == ref.st_gid));
			إذا (emask & STATX_اندازه)
				چاپ_کن(" st_size: %s\n",
					نتیجه_2(STATX_اندازه, st.st_size == ref.st_size));
# undef نتیجه_2
		}
#پایان_اگر
	}
	أرجع EXIT_SUCCESS;
}
#پایان_اگر
