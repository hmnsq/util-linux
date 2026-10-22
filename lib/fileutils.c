/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/resource.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "canonicalize.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"

ਪੂਰਨ mkstemp_cloexec(ਅੱਖਰ *ਸਾਂਚਾ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MKOSTEMP
	ਵਾਪਸ mkostemp(ਸਾਂਚਾ, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ fd, old_flags, errno_save;

	fd = mkstemp(ਸਾਂਚਾ);
	ਜੇ (fd < 0)
		ਵਾਪਸ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ਜੇ (old_flags < 0)
		ਜਾਓ unwind;
	ਜੇ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		ਜਾਓ unwind;

	ਵਾਪਸ fd;

unwind:
	errno_save = errno;
	unlink(ਸਾਂਚਾ);
	close(fd);
	errno = errno_save;

	ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
ਪੂਰਨ xmkstemp(ਅੱਖਰ **tmpname, ਅਚਲ ਅੱਖਰ *dir, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਅੱਖਰ *localtmp;
	ਅਚਲ ਅੱਖਰ *tmpenv;
	mode_t old_ਬਹੁਲਕ;
	ਪੂਰਨ fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ਜੇ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, ਅਗੇਤਰ);
	ਜੇ (rc < 0)
		ਵਾਪਸ -1;

	old_ਬਹੁਲਕ = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_ਬਹੁਲਕ);
	ਜੇ (fd == -1) {
		ਮੁਕਤ(localtmp);
		localtmp = ਨਿਰਾ;
	}
	*tmpname = localtmp;
	ਵਾਪਸ fd;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ F_DUPFD_CLOEXEC
ਪੂਰਨ dup_fd_cloexec(ਪੂਰਨ oldfd, ਪੂਰਨ lowfd)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ dup_fd_cloexec(ਪੂਰਨ oldfd, ਪੂਰਨ lowfd  __attribute__((__unused__)))
#ਜੇ_ਅੰਤ
{
	ਪੂਰਨ fd, flags, errno_save;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ਜੇ (fd >= 0)
		ਵਾਪਸ fd;
#ਜੇ_ਅੰਤ

	fd = dup(oldfd);
	ਜੇ (fd < 0)
		ਵਾਪਸ fd;

	flags = fcntl(fd, F_GETFD);
	ਜੇ (flags < 0)
		ਜਾਓ unwind;
	ਜੇ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		ਜਾਓ unwind;

	ਵਾਪਸ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ਵਾਪਸ -1;
}

/*
 * portable getdtablesize()
 */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ get_fd_tabsize(ਖਾਲੀ)
{
	ਪੂਰਨ m;

#ਜੇ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#ਜਾਂਜੇ defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ਬਣਤਰ rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#ਜਾਂਜੇ defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#ਨਹੀਂਤਾਂ
	m = OPEN_MAX;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ m;
}

ਖਾਲੀ ul_close_all_fds(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪਹਿਲਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਖਰੀ)
{
	ਬਣਤਰ dirent *d;
	DIR *dir;

	dir = opendir(ਰਸਤਾ_PROC_FDDIR);
	ਜੇ (dir) {
		ਜਦੋਂ ((d = xreaddir(dir))) {
			ਅੱਖਰ *ਸਿਰਾ;
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ fd;
			ਪੂਰਨ dfd;

			errno = 0;
			fd = strtoul(d->d_name, &ਸਿਰਾ, 10);

			ਜੇ (errno || ਸਿਰਾ == d->d_name || !ਸਿਰਾ || *ਸਿਰਾ)
				ਜਾਰੀ;
			dfd = dirfd(dir);
			ਜੇ (dfd < 0)
				ਜਾਰੀ;
			ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)dfd == fd)
				ਜਾਰੀ;
			ਜੇ (fd < ਪਹਿਲਾ || ਆਖਰੀ < fd)
				ਜਾਰੀ;
			close(fd);
		}
		closedir(dir);
	} ਨਹੀਂਤਾਂ {
		ਬਿਨਾਨਿਸ਼ਾਨੀ fd, tbsz = get_fd_tabsize();

		ਲਈ (fd = 0; fd < tbsz; fd++) {
			ਜੇ (ਪਹਿਲਾ <= fd && fd <= ਆਖਰੀ)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
ਅੱਖਰ *ul_restricted_ਰਸਤਾ_oper(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
		  ਪੂਰਨ (*oper)(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅੱਖਰ **ਨਤੀਜਾ, ਖਾਲੀ *ਅੰਕੜੇ),
		  ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਅੱਖਰ *ਨਤੀਜਾ = ਨਿਰਾ;
	ਪੂਰਨ errsv = 0;
	ਪੂਰਨ pipes[2];
	ssize_t len;
	pid_t pid;

	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (pipe(pipes) != 0)
		ਵਾਪਸ ਨਿਰਾ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	ਚੋਣ (pid) {
	ਹਾਲਤ -1:
		close(pipes[0]);
		close(pipes[1]);
		ਵਾਪਸ ਨਿਰਾ;			/* fork error */
	ਹਾਲਤ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ਜੇ (drop_permissions() != 0)
			ਨਤੀਜਾ = ਨਿਰਾ;	/* failed */
		ਨਹੀਂਤਾਂ
			oper(ਰਸਤਾ, &ਨਤੀਜਾ, ਅੰਕੜੇ);

		len = ਨਤੀਜਾ ? (ssize_t) ਲੜੀ_ਲੰਬਾਈ(ਨਤੀਜਾ) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (ਅੱਖਰ *) &len, ਆਕਾਰ(len));
		ਜੇ (ਨਤੀਜਾ)
			ul_write_all(pipes[1], ਨਤੀਜਾ, len);
		_exit(0);
	ਮੂਲ:
		ਰੋਕੋ;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ਜੇ (ul_read_all(pipes[0], (ਅੱਖਰ *) &len, ਆਕਾਰ(len)) != ਆਕਾਰ(len))
		ਜਾਓ ਪੂਰਾ;
	ਜੇ (len < 0) {
		errsv = -len;
		ਜਾਓ ਪੂਰਾ;
	}

	ਨਤੀਜਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
	ਜੇ (!ਨਤੀਜਾ) {
		errsv = ENOMEM;
		ਜਾਓ ਪੂਰਾ;
	}
	/* read path */
	ਜੇ (ul_read_all(pipes[0], ਨਤੀਜਾ, len) != len) {
		errsv = errno;
		ਜਾਓ ਪੂਰਾ;
	}
	ਨਤੀਜਾ[len] = '\0';
ਪੂਰਾ:
	ਜੇ (errsv) {
		ਮੁਕਤ(ਨਤੀਜਾ);
		ਨਤੀਜਾ = ਨਿਰਾ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_ਨਤੀਜਾ( waitpid(pid, ਨਿਰਾ, 0) );

	errno = errsv;
	ਵਾਪਸ ਨਤੀਜਾ;

}

ਪੂਰਨ ul_mkdir_p(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, mode_t ਬਹੁਲਕ)
{
	ਅੱਖਰ *p, *dir;
	ਪੂਰਨ rc = 0;

	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ)
		ਵਾਪਸ -EINVAL;

	dir = p = strdup(ਰਸਤਾ);
	ਜੇ (!dir)
		ਵਾਪਸ -ENOMEM;

	ਜੇ (*p == '/')
		p++;

	ਜਦੋਂ (p && *p) {
		ਅੱਖਰ *e = strchr(p, '/');
		ਜੇ (e)
			*e = '\0';
		ਜੇ (*p) {
			rc = mkdir(dir, ਬਹੁਲਕ);
			ਜੇ (rc && errno != EEXIST)
				ਰੋਕੋ;
			rc = 0;
		}
		ਜੇ (!e)
			ਰੋਕੋ;
		*e = '/';
		p = e + 1;
	}

	ਮੁਕਤ(dir);
	ਵਾਪਸ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
ਅੱਖਰ *stripoff_ਆਖਰੀ_component(ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *p = ਰਸਤਾ ? strrchr(ਰਸਤਾ, '/') : ਨਿਰਾ;

	ਜੇ (!p)
		ਵਾਪਸ ਨਿਰਾ;
	*p = '\0';
	ਵਾਪਸ p + 1;
}

ਸਥਿਰ ਪੂਰਨ copy_file_simple(ਪੂਰਨ from, ਪੂਰਨ to)
{
	ssize_t nr;
	ਅੱਖਰ buf[BUFSIZ];

	ਜਦੋਂ ((nr = ul_read_all(from, buf, ਆਕਾਰ(buf))) > 0)
		ਜੇ (ul_write_all(to, buf, nr) == -1)
			ਵਾਪਸ UL_COPY_WRITE_ERROR;
	ਜੇ (nr < 0)
		ਵਾਪਸ UL_COPY_READ_ERROR;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, ਆਕਾਰ(buf));
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
ਪੂਰਨ ul_copy_file(ਪੂਰਨ from, ਪੂਰਨ to)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SENDFILE
	ਬਣਤਰ stat st;
	ssize_t nw;

	ਜੇ (fstat(from, &st) == -1)
		ਵਾਪਸ UL_COPY_READ_ERROR;
	ਜੇ (!S_ISREG(st.st_mode))
		ਵਾਪਸ copy_file_simple(from, to);
	ਜੇ (ul_sendfile_all(to, from, ਨਿਰਾ, st.st_size) < 0)
		ਵਾਪਸ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ਜਦੋਂ ((nw = ul_sendfile_all(to, from, ਨਿਰਾ, 16*1024*1024)) != 0)
		ਜੇ (nw < 0)
			ਵਾਪਸ copy_file_simple(from, to);
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ copy_file_simple(from, to);
#ਜੇ_ਅੰਤ
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
ਅੱਖਰ *ul_fd_mkpath(ਅੱਖਰ *buf, size_t bufsz, ਪੂਰਨ fd)
{
	ਪੂਰਨ len;

	ਜੇ (fd < 0) {
		errno = EBADF;
		ਵਾਪਸ ਨਿਰਾ;
	}

	len = snprintf(buf, bufsz, ਰਸਤਾ_PROC_FDDIR "/%d", fd);
	ਜੇ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
ਅੱਖਰ *ul_fd_get_ਰਸਤਾ(ਪੂਰਨ fd)
{
	ssize_t ssz;
	ਅੱਖਰ buf[PATH_MAX];
	ਅੱਖਰ fdpath[UL_FDPATH_BUFSIZ];

	ਜੇ (!ul_fd_mkpath(fdpath, ਆਕਾਰ(fdpath), fd))
		ਵਾਪਸ ਨਿਰਾ;

	ssz = readlink(fdpath, buf, ਆਕਾਰ(buf));
	ਜੇ (ssz < 0)
		ਵਾਪਸ ਨਿਰਾ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ਜੇ ((size_t) ssz >= ਆਕਾਰ(buf)) {
		errno = ENAMETOOLONG;
		ਵਾਪਸ ਨਿਰਾ;
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
	ਜੇ (*buf != '/' || ul_endswith(buf, ਰਸਤਾ_DELETED_SUFFIX)) {
		errno = ENOENT;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ strdup(buf);
}

ਪੂਰਨ ul_reopen(ਪੂਰਨ fd, ਪੂਰਨ flags)
{
	ਅੱਖਰ *ਰਸਤਾ = ul_fd_get_ਰਸਤਾ(fd);
	ਪੂਰਨ ret;

	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ -errno;

	ret = open(ਰਸਤਾ, flags);
	ਮੁਕਤ(ਰਸਤਾ);

	ਵਾਪਸ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
ਅੱਖਰ *ul_basename(ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *p;

	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ)
		ਵਾਪਸ (ਅੱਖਰ *) ".";	/* ugly, static string */

	p = strrchr(ਰਸਤਾ, '/');
	ਜੇ (!p)
		ਵਾਪਸ ਰਸਤਾ;		/* no '/', return original */

	ਜੇ (*(p + 1) != '\0')
		ਵਾਪਸ p + 1;		/* begin of the name */

	ਜਦੋਂ (p > ਰਸਤਾ && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ਜੇ (p > ਰਸਤਾ) {
		*p-- = '\0';
		ਜਦੋਂ (p > ਰਸਤਾ && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} ਨਹੀਂਤਾਂ ਜਦੋਂ (*(p + 1) != '\0')
		++p;

	ਵਾਪਸ p;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_OPENAT
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
FILE *fopen_at_no_link(ਪੂਰਨ dir, ਅਚਲ ਅੱਖਰ *filename,
                             ਪੂਰਨ flags, mode_t perm, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ)
{
	FILE *fp;
	ਪੂਰਨ fd;
	ਬਣਤਰ stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ਜੇ (fd < 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (fstat(fd, &st)) {
		close(fd);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ਵਾਪਸ ਨਿਰਾ;
	}

	fp = fdopen(fd, ਬਹੁਲਕ);
	ਜੇ (!fp)
		close(fd);
	ਵਾਪਸ fp;
}
#ਜੇ_ਅੰਤ /* HAVE_OPENAT */

#ਜੇ defined(SYS_openat2)
ਪੂਰਨ ul_openat_resolve(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ flags,
		      mode_t ਬਹੁਲਕ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ resolve)
{
	ਬਣਤਰ open_how how = {
		.flags = (__u64) flags,
		.ਬਹੁਲਕ = (__u64) ਬਹੁਲਕ,
		.resolve = resolve,
	};

	ਵਾਪਸ syscall(SYS_openat2, dirfd, ਰਸਤਾ, &how, ਆਕਾਰ(how));
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ ul_openat_resolve(
		ਪੂਰਨ dirfd __attribute__((__unused__)),
		ਅਚਲ ਅੱਖਰ *ਰਸਤਾ __attribute__((__unused__)),
		ਪੂਰਨ flags __attribute__((__unused__)),
		mode_t ਬਹੁਲਕ __attribute__((__unused__)),
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ਵਾਪਸ -1;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
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
ਸਥਿਰ ਪੂਰਨ open_no_symlinks_fallback(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ flags, mode_t ਬਹੁਲਕ)
{
	ਅੱਖਰ *abspath = ਨਿਰਾ, *kpath = ਨਿਰਾ;
	ਬਣਤਰ stat st;
	ਪੂਰਨ fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ਜੇ (!ਰਸਤਾ || (flags & O_CREAT)) {
		errno = ENOSYS;
		ਵਾਪਸ -1;
	}

	ਜੇ (ul_is_relative_ਰਸਤਾ(ਰਸਤਾ)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_ਰਸਤਾ(ਰਸਤਾ);
		ਜੇ (!abspath)
			ਵਾਪਸ -1;
	}

	ਜੇ (flags & O_ਰਸਤਾ)
		fd = open(ਰਸਤਾ, flags);
	ਨਹੀਂਤਾਂ
		fd = open(ਰਸਤਾ, O_ਰਸਤਾ | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ਜੇ (fd < 0)
		ਜਾਓ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ਜੇ (fstat(fd, &st) != 0)
		ਜਾਓ fail;
	ਜੇ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		ਜਾਓ fail;
	}

	kpath = ul_fd_get_ਰਸਤਾ(fd);
	ਜੇ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		ਜਾਓ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ਜੇ (streq_paths(abspath ? abspath : ਰਸਤਾ, kpath) != 1) {
		errno = ELOOP;
		ਜਾਓ fail;
	}

	ਜੇ (!(flags & O_ਰਸਤਾ)) {
		ਅੱਖਰ fdpath[UL_FDPATH_BUFSIZ];
		ਪੂਰਨ nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ਜੇ (ul_fd_mkpath(fdpath, ਆਕਾਰ(fdpath), fd))
			nfd = open(fdpath, flags, ਬਹੁਲਕ);
		ਜੇ (nfd < 0)
			ਜਾਓ fail;
		close(fd);
		fd = nfd;
	}

	ਮੁਕਤ(abspath);
	ਮੁਕਤ(kpath);
	ਵਾਪਸ fd;
fail:
	errsv = errno;
	ਮੁਕਤ(abspath);
	ਮੁਕਤ(kpath);
	ਜੇ (fd >= 0)
		close(fd);
	errno = errsv;
	ਵਾਪਸ -1;
}
#ਨਹੀਂਤਾਂ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ਸਥਿਰ ਪੂਰਨ open_no_symlinks_fallback(
			ਅਚਲ ਅੱਖਰ *ਰਸਤਾ __attribute__((__unused__)),
			ਪੂਰਨ flags __attribute__((__unused__)),
			mode_t ਬਹੁਲਕ __attribute__((__unused__)))
{
	errno = ENOSYS;
	ਵਾਪਸ -1;
}
#ਜੇ_ਅੰਤ /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
ਪੂਰਨ ul_open_no_symlinks(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ flags, mode_t ਬਹੁਲਕ)
{
	ਪੂਰਨ fd = ul_openat_resolve(AT_FDCWD, ਰਸਤਾ, flags, ਬਹੁਲਕ,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ਜੇ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(ਰਸਤਾ, flags, ਬਹੁਲਕ);

	ਵਾਪਸ fd;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UL_ਸੁਰੱਖਿਅਤ_STATX
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
ਪੂਰਨ ul_ਸੁਰੱਖਿਅਤ_statx(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ flags,
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ, ਬਣਤਰ statx *stx)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(stx);

	memset(stx, 0, ਆਕਾਰ(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ AT_ਖਾਲੀ_ਥਾਵਾਂ_ਰਸਤਾ
	ਜੇ (!ਰਸਤਾ || !*ਰਸਤਾ)
		flags |= AT_ਖਾਲੀ_ਥਾਵਾਂ_ਰਸਤਾ;
#ਜੇ_ਅੰਤ
	ਜੇ (statx(dirfd, ਰਸਤਾ ? ਰਸਤਾ : "", flags, ਨਕਾਬ, stx) != 0)
		ਵਾਪਸ -errno;

	ਵਾਪਸ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
ਖਾਲੀ ul_statx_ਮੰਜ਼ਿਲ_stat(ਅਚਲ ਬਣਤਰ statx *stx, ਬਣਤਰ stat *st)
{
	memset(st, 0, ਆਕਾਰ(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_TYPE)
		st->st_mode |= stx->stx_ਬਹੁਲਕ & S_IFMT;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_ਬਹੁਲਕ)
		st->st_mode |= stx->stx_ਬਹੁਲਕ & ~S_IFMT;

	ਜੇ (stx->stx_ਨਕਾਬ & STATX_INO)
		st->st_ino = stx->stx_ino;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_UID)
		st->st_uid = stx->stx_uid;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_GID)
		st->st_gid = stx->stx_gid;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_ਮਾਪ)
		st->st_size = stx->stx_ਮਾਪ;
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_ਖੰਡਾਂ)
		st->st_blocks = stx->stx_ਖੰਡਾਂ;

	ਜੇ (stx->stx_ਨਕਾਬ & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ਜੇ (stx->stx_ਨਕਾਬ & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#ਜੇ_ਅੰਤ

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
ਪੂਰਨ ul_ਸੁਰੱਖਿਅਤ_stat(ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, ਬਣਤਰ stat *st,
		 ਪੂਰਨ nofollow __attribute__((__unused__)),
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ __attribute__((__unused__)),
		 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *retmask)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(ਨਿਸ਼ਾਨਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(st);

	memset(st, 0, ਆਕਾਰ(ਬਣਤਰ stat));
	ਜੇ (retmask)
		*retmask = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UL_ਸੁਰੱਖਿਅਤ_STAT
	{
		ਬਣਤਰ statx stx = { 0 };
		ਪੂਰਨ rc;

		ਨਕਾਬ |= UL_STATX_ESSENTIAL;

		rc = ul_ਸੁਰੱਖਿਅਤ_statx(AT_FDCWD, ਨਿਸ਼ਾਨਾ,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, ਨਕਾਬ, &stx);
		ਜੇ (rc)
			ਵਾਪਸ rc;

		ul_statx_ਮੰਜ਼ਿਲ_stat(&stx, st);

		ਜੇ (retmask)
			*retmask = stx.stx_ਨਕਾਬ;

		ਜੇ ((stx.stx_ਨਕਾਬ & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ਵਾਪਸ -EOPNOTSUPP;
		}

		ਵਾਪਸ 0;
	}
#ਨਹੀਂਤਾਂ
	errno = ENOSYS;
	ਵਾਪਸ -ENOSYS;
#ਜੇ_ਅੰਤ
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_FILEUTILS
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਜੇ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ਜੇ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		ਅੱਖਰ *tmpname = ਨਿਰਾ;

		f = xfmkstemp(&tmpname, ਨਿਰਾ, "test");
		unlink(tmpname);
		ਮੁਕਤ(tmpname);
		fclose(f);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_ਨਤੀਜਾ( dup(STDIN_FILENO) );
		ignore_ਨਤੀਜਾ( dup(STDIN_FILENO) );
		ignore_ਨਤੀਜਾ( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ਜੇ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--copy-file") == 0) {
		ਪੂਰਨ ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ਜੇ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		ਨਹੀਂਤਾਂ ਜੇ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		ਪੂਰਨ flags = O_ਰਸਤਾ | O_CLOEXEC;
#ਨਹੀਂਤਾਂ
		ਪੂਰਨ flags = O_RDONLY | O_CLOEXEC;
#ਜੇ_ਅੰਤ
		ਪੂਰਨ fallback = 0;
		ਅੱਖਰ *name;
		ਪੂਰਨ fd, i;

		ਜੇ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ਲਈ (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ਜੇ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ਜੇ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		ਨਹੀਂਤਾਂ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ਜੇ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_ਰਸਤਾ(fd);
		ਲਿਖੋ("%s\n", name);
		ਮੁਕਤ(name);
		close(fd);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--safe-stat") == 0) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ = 0, retmask = 0;
		ਪੂਰਨ nofollow = 0;
		ਬਣਤਰ stat st, ref;
		ਪੂਰਨ rc, i;

		ਜੇ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ਲਈ (i = 3; i < argc; i++) {
			ਜੇ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[i], "--basic") == 0)
				ਨਕਾਬ |= UL_STATX_BASIC;
		}

		ਲਿਖੋ("%s:\n", argv[2]);

		rc = ul_ਸੁਰੱਖਿਅਤ_stat(argv[2], &st, nofollow, ਨਕਾਬ, &retmask);

		/* the function returns -errno and it sets errno too */
		ਲਿਖੋ("      rc: %d\n", rc);
		ਲਿਖੋ("   errno: %d\n", rc ? errno : 0);
		ਜੇ (rc)
			ਵਾਪਸ EXIT_FAILURE;

		ਲਿਖੋ("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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
		ਜੇ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(EXIT_FAILURE, "%s", argv[2]);

		ਲਿਖੋ("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		ਲਿਖੋ(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UL_ਸੁਰੱਖਿਅਤ_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ emask = ਨਕਾਬ | UL_STATX_ESSENTIAL;

# define ਨਤੀਜਾ_2(ਦੋਆਧਾਰੀ_ਅੰਕ, x)	(!(retmask & (ਦੋਆਧਾਰੀ_ਅੰਕ)) || (x) ? "OK" : "FAILED")

			ਜੇ (emask & STATX_TYPE)
				ਲਿਖੋ("  S_IFMT: %s\n", ਨਤੀਜਾ_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ਜੇ (emask & STATX_ਬਹੁਲਕ)
				ਲਿਖੋ(" st_mode: %s\n", ਨਤੀਜਾ_2(STATX_ਬਹੁਲਕ,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ਜੇ (emask & STATX_INO)
				ਲਿਖੋ("  st_ino: %s\n",
					ਨਤੀਜਾ_2(STATX_INO, st.st_ino == ref.st_ino));
			ਜੇ (emask & STATX_NLINK)
				ਲਿਖੋ("st_nlink: %s\n",
					ਨਤੀਜਾ_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ਜੇ (emask & STATX_UID)
				ਲਿਖੋ("  st_uid: %s\n",
					ਨਤੀਜਾ_2(STATX_UID, st.st_uid == ref.st_uid));
			ਜੇ (emask & STATX_GID)
				ਲਿਖੋ("  st_gid: %s\n",
					ਨਤੀਜਾ_2(STATX_GID, st.st_gid == ref.st_gid));
			ਜੇ (emask & STATX_ਮਾਪ)
				ਲਿਖੋ(" st_size: %s\n",
					ਨਤੀਜਾ_2(STATX_ਮਾਪ, st.st_size == ref.st_size));
# undef ਨਤੀਜਾ_2
		}
#ਜੇ_ਅੰਤ
	}
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ
