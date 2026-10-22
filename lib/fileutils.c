/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "canonicalize.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

ಪೂರ್ಣಾಙ್ಕ mkstemp_cloexec(ವರ್ಣ *template)
{
#ifdef HAVE_MKOSTEMP
	ಪ್ರತ್ಯಾಗಮ mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ fd, old_flags, errno_save;

	fd = mkstemp(template);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ಯದಿ (old_flags < 0)
		ಗಚ್ಛ unwind;
	ಯದಿ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		ಗಚ್ಛ unwind;

	ಪ್ರತ್ಯಾಗಮ fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	ಪ್ರತ್ಯಾಗಮ -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
ಪೂರ್ಣಾಙ್ಕ xmkstemp(ವರ್ಣ **tmpname, ಧ್ರುವ ವರ್ಣ *dir, ಧ್ರುವ ವರ್ಣ *prefix)
{
	ವರ್ಣ *localtmp;
	ಧ್ರುವ ವರ್ಣ *tmpenv;
	mode_t old_mode;
	ಪೂರ್ಣಾಙ್ಕ fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ಯದಿ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	ಯದಿ (fd == -1) {
		ಮುಕ್ತ(localtmp);
		localtmp = ರಿಕ್ತ;
	}
	*tmpname = localtmp;
	ಪ್ರತ್ಯಾಗಮ fd;
}

#ifdef F_DUPFD_CLOEXEC
ಪೂರ್ಣಾಙ್ಕ dup_fd_cloexec(ಪೂರ್ಣಾಙ್ಕ oldfd, ಪೂರ್ಣಾಙ್ಕ lowfd)
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ dup_fd_cloexec(ಪೂರ್ಣಾಙ್ಕ oldfd, ಪೂರ್ಣಾಙ್ಕ lowfd  __attribute__((__unused__)))
#endif
{
	ಪೂರ್ಣಾಙ್ಕ fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ಯದಿ (fd >= 0)
		ಪ್ರತ್ಯಾಗಮ fd;
#endif

	fd = dup(oldfd);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ fd;

	flags = fcntl(fd, F_GETFD);
	ಯದಿ (flags < 0)
		ಗಚ್ಛ unwind;
	ಯದಿ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		ಗಚ್ಛ unwind;

	ಪ್ರತ್ಯಾಗಮ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ಪ್ರತ್ಯಾಗಮ -1;
}

/*
 * portable getdtablesize()
 */
unsigned ಪೂರ್ಣಾಙ್ಕ get_fd_tabsize(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ m;

#ಯದಿ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#ನೋಚೇದ್ಯದಿ defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ಸಂರಚನಾ rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#ನೋಚೇದ್ಯದಿ defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#ಅನ್ಯಥಾ
	m = OPEN_MAX;
#endif
	ಪ್ರತ್ಯಾಗಮ m;
}

ಶೂನ್ಯ ul_close_all_fds(unsigned ಪೂರ್ಣಾಙ್ಕ first, unsigned ಪೂರ್ಣಾಙ್ಕ last)
{
	ಸಂರಚನಾ dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	ಯದಿ (dir) {
		ಯಾವತ್ ((d = xreaddir(dir))) {
			ವರ್ಣ *end;
			unsigned ಪೂರ್ಣಾಙ್ಕ fd;
			ಪೂರ್ಣಾಙ್ಕ dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			ಯದಿ (errno || end == d->d_name || !end || *end)
				ಅನುವರ್ತ;
			dfd = dirfd(dir);
			ಯದಿ (dfd < 0)
				ಅನುವರ್ತ;
			ಯದಿ ((unsigned ಪೂರ್ಣಾಙ್ಕ)dfd == fd)
				ಅನುವರ್ತ;
			ಯದಿ (fd < first || last < fd)
				ಅನುವರ್ತ;
			close(fd);
		}
		closedir(dir);
	} ಅನ್ಯಥಾ {
		unsigned fd, tbsz = get_fd_tabsize();

		ಪ್ರತಿ (fd = 0; fd < tbsz; fd++) {
			ಯದಿ (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
ವರ್ಣ *ul_restricted_path_oper(ಧ್ರುವ ವರ್ಣ *path,
		  ಪೂರ್ಣಾಙ್ಕ (*oper)(ಧ್ರುವ ವರ್ಣ *path, ವರ್ಣ **result, ಶೂನ್ಯ *data),
		  ಶೂನ್ಯ *data)
{
	ವರ್ಣ *result = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ errsv = 0;
	ಪೂರ್ಣಾಙ್ಕ pipes[2];
	ssize_t len;
	pid_t pid;

	ಯದಿ (!path || !*path)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (pipe(pipes) != 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	ವಿಕಲ್ಪ (pid) {
	ಪಕ್ಷ -1:
		close(pipes[0]);
		close(pipes[1]);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;			/* fork error */
	ಪಕ್ಷ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ಯದಿ (drop_permissions() != 0)
			result = ರಿಕ್ತ;	/* failed */
		ಅನ್ಯಥಾ
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (ವರ್ಣ *) &len, ಪರಿಮಾಣ(len));
		ಯದಿ (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ಯದಿ (ul_read_all(pipes[0], (ವರ್ಣ *) &len, ಪರಿಮಾಣ(len)) != ಪರಿಮಾಣ(len))
		ಗಚ್ಛ done;
	ಯದಿ (len < 0) {
		errsv = -len;
		ಗಚ್ಛ done;
	}

	result = malloc(len + 1);
	ಯದಿ (!result) {
		errsv = ENOMEM;
		ಗಚ್ಛ done;
	}
	/* read path */
	ಯದಿ (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		ಗಚ್ಛ done;
	}
	result[len] = '\0';
done:
	ಯದಿ (errsv) {
		ಮುಕ್ತ(result);
		result = ರಿಕ್ತ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, ರಿಕ್ತ, 0) );

	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ result;

}

ಪೂರ್ಣಾಙ್ಕ ul_mkdir_p(ಧ್ರುವ ವರ್ಣ *path, mode_t mode)
{
	ವರ್ಣ *p, *dir;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (!path || !*path)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	dir = p = strdup(path);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	ಯದಿ (*p == '/')
		p++;

	ಯಾವತ್ (p && *p) {
		ವರ್ಣ *e = strchr(p, '/');
		ಯದಿ (e)
			*e = '\0';
		ಯದಿ (*p) {
			rc = mkdir(dir, mode);
			ಯದಿ (rc && errno != EEXIST)
				ಭಙ್ಗ;
			rc = 0;
		}
		ಯದಿ (!e)
			ಭಙ್ಗ;
		*e = '/';
		p = e + 1;
	}

	ಮುಕ್ತ(dir);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
ವರ್ಣ *stripoff_last_component(ವರ್ಣ *path)
{
	ವರ್ಣ *p = path ? strrchr(path, '/') : ರಿಕ್ತ;

	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	*p = '\0';
	ಪ್ರತ್ಯಾಗಮ p + 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ copy_file_simple(ಪೂರ್ಣಾಙ್ಕ from, ಪೂರ್ಣಾಙ್ಕ to)
{
	ssize_t nr;
	ವರ್ಣ buf[BUFSIZ];

	ಯಾವತ್ ((nr = ul_read_all(from, buf, ಪರಿಮಾಣ(buf))) > 0)
		ಯದಿ (ul_write_all(to, buf, nr) == -1)
			ಪ್ರತ್ಯಾಗಮ UL_COPY_WRITE_ERROR;
	ಯದಿ (nr < 0)
		ಪ್ರತ್ಯಾಗಮ UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, ಪರಿಮಾಣ(buf));
#endif
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
ಪೂರ್ಣಾಙ್ಕ ul_copy_file(ಪೂರ್ಣಾಙ್ಕ from, ಪೂರ್ಣಾಙ್ಕ to)
{
#ifdef HAVE_SENDFILE
	ಸಂರಚನಾ stat st;
	ssize_t nw;

	ಯದಿ (fstat(from, &st) == -1)
		ಪ್ರತ್ಯಾಗಮ UL_COPY_READ_ERROR;
	ಯದಿ (!S_ISREG(st.st_mode))
		ಪ್ರತ್ಯಾಗಮ copy_file_simple(from, to);
	ಯದಿ (ul_sendfile_all(to, from, ರಿಕ್ತ, st.st_size) < 0)
		ಪ್ರತ್ಯಾಗಮ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ಯಾವತ್ ((nw = ul_sendfile_all(to, from, ರಿಕ್ತ, 16*1024*1024)) != 0)
		ಯದಿ (nw < 0)
			ಪ್ರತ್ಯಾಗಮ copy_file_simple(from, to);
	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
ವರ್ಣ *ul_fd_mkpath(ವರ್ಣ *buf, size_t bufsz, ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಪೂರ್ಣಾಙ್ಕ len;

	ಯದಿ (fd < 0) {
		errno = EBADF;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	ಯದಿ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
ವರ್ಣ *ul_fd_get_path(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ssize_t ssz;
	ವರ್ಣ buf[PATH_MAX];
	ವರ್ಣ fdpath[UL_FDPATH_BUFSIZ];

	ಯದಿ (!ul_fd_mkpath(fdpath, ಪರಿಮಾಣ(fdpath), fd))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ssz = readlink(fdpath, buf, ಪರಿಮಾಣ(buf));
	ಯದಿ (ssz < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ಯದಿ ((size_t) ssz >= ಪರಿಮಾಣ(buf)) {
		errno = ENAMETOOLONG;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
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
	ಯದಿ (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ strdup(buf);
}

ಪೂರ್ಣಾಙ್ಕ ul_reopen(ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ವರ್ಣ *path = ul_fd_get_path(fd);
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ -errno;

	ret = open(path, flags);
	ಮುಕ್ತ(path);

	ಪ್ರತ್ಯಾಗಮ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
ವರ್ಣ *ul_basename(ವರ್ಣ *path)
{
	ವರ್ಣ *p;

	ಯದಿ (!path || !*path)
		ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ path;		/* no '/', return original */

	ಯದಿ (*(p + 1) != '\0')
		ಪ್ರತ್ಯಾಗಮ p + 1;		/* begin of the name */

	ಯಾವತ್ (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ಯದಿ (p > path) {
		*p-- = '\0';
		ಯಾವತ್ (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} ಅನ್ಯಥಾ ಯಾವತ್ (*(p + 1) != '\0')
		++p;

	ಪ್ರತ್ಯಾಗಮ p;
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
FILE *fopen_at_no_link(ಪೂರ್ಣಾಙ್ಕ dir, ಧ್ರುವ ವರ್ಣ *filename,
                             ಪೂರ್ಣಾಙ್ಕ flags, mode_t perm, ಧ್ರುವ ವರ್ಣ *mode)
{
	FILE *fp;
	ಪೂರ್ಣಾಙ್ಕ fd;
	ಸಂರಚನಾ stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (fstat(fd, &st)) {
		close(fd);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	fp = fdopen(fd, mode);
	ಯದಿ (!fp)
		close(fd);
	ಪ್ರತ್ಯಾಗಮ fp;
}
#endif /* HAVE_OPENAT */

#ಯದಿ defined(SYS_openat2)
ಪೂರ್ಣಾಙ್ಕ ul_openat_resolve(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ flags,
		      mode_t mode, unsigned long long resolve)
{
	ಸಂರಚನಾ open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	ಪ್ರತ್ಯಾಗಮ syscall(SYS_openat2, dirfd, path, &how, ಪರಿಮಾಣ(how));
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ ul_openat_resolve(
		ಪೂರ್ಣಾಙ್ಕ dirfd __attribute__((__unused__)),
		ಧ್ರುವ ವರ್ಣ *path __attribute__((__unused__)),
		ಪೂರ್ಣಾಙ್ಕ flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಪ್ರತ್ಯಾಗಮ -1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ open_no_symlinks_fallback(ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ flags, mode_t mode)
{
	ವರ್ಣ *abspath = ರಿಕ್ತ, *kpath = ರಿಕ್ತ;
	ಸಂರಚನಾ stat st;
	ಪೂರ್ಣಾಙ್ಕ fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ಯದಿ (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		ಯದಿ (!abspath)
			ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (flags & O_PATH)
		fd = open(path, flags);
	ಅನ್ಯಥಾ
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ಯದಿ (fd < 0)
		ಗಚ್ಛ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ಯದಿ (fstat(fd, &st) != 0)
		ಗಚ್ಛ fail;
	ಯದಿ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		ಗಚ್ಛ fail;
	}

	kpath = ul_fd_get_path(fd);
	ಯದಿ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		ಗಚ್ಛ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ಯದಿ (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		ಗಚ್ಛ fail;
	}

	ಯದಿ (!(flags & O_PATH)) {
		ವರ್ಣ fdpath[UL_FDPATH_BUFSIZ];
		ಪೂರ್ಣಾಙ್ಕ nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ಯದಿ (ul_fd_mkpath(fdpath, ಪರಿಮಾಣ(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		ಯದಿ (nfd < 0)
			ಗಚ್ಛ fail;
		close(fd);
		fd = nfd;
	}

	ಮುಕ್ತ(abspath);
	ಮುಕ್ತ(kpath);
	ಪ್ರತ್ಯಾಗಮ fd;
fail:
	errsv = errno;
	ಮುಕ್ತ(abspath);
	ಮುಕ್ತ(kpath);
	ಯದಿ (fd >= 0)
		close(fd);
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ -1;
}
#ಅನ್ಯಥಾ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ open_no_symlinks_fallback(
			ಧ್ರುವ ವರ್ಣ *path __attribute__((__unused__)),
			ಪೂರ್ಣಾಙ್ಕ flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಪ್ರತ್ಯಾಗಮ -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
ಪೂರ್ಣಾಙ್ಕ ul_open_no_symlinks(ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ flags, mode_t mode)
{
	ಪೂರ್ಣಾಙ್ಕ fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ಯದಿ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	ಪ್ರತ್ಯಾಗಮ fd;
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
ಪೂರ್ಣಾಙ್ಕ ul_safe_statx(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ flags,
		  unsigned ಪೂರ್ಣಾಙ್ಕ mask, ಸಂರಚನಾ statx *stx)
{
	ಪ್ರತಿಜ್ಞಾ(stx);

	memset(stx, 0, ಪರಿಮಾಣ(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	ಯದಿ (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	ಯದಿ (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
ಶೂನ್ಯ ul_statx_to_stat(ಧ್ರುವ ಸಂರಚನಾ statx *stx, ಸಂರಚನಾ stat *st)
{
	memset(st, 0, ಪರಿಮಾಣ(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ಯದಿ (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	ಯದಿ (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	ಯದಿ (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	ಯದಿ (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ಯದಿ (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	ಯದಿ (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	ಯದಿ (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	ಯದಿ (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	ಯದಿ (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ಯದಿ (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ಯದಿ (stx->stx_mask & STATX_CTIME) {
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
ಪೂರ್ಣಾಙ್ಕ ul_safe_stat(ಧ್ರುವ ವರ್ಣ *target, ಸಂರಚನಾ stat *st,
		 ಪೂರ್ಣಾಙ್ಕ nofollow __attribute__((__unused__)),
		 unsigned ಪೂರ್ಣಾಙ್ಕ mask __attribute__((__unused__)),
		 unsigned ಪೂರ್ಣಾಙ್ಕ *retmask)
{
	ಪ್ರತಿಜ್ಞಾ(target);
	ಪ್ರತಿಜ್ಞಾ(st);

	memset(st, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ stat));
	ಯದಿ (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		ಸಂರಚನಾ statx stx = { 0 };
		ಪೂರ್ಣಾಙ್ಕ rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ rc;

		ul_statx_to_stat(&stx, st);

		ಯದಿ (retmask)
			*retmask = stx.stx_mask;

		ಯದಿ ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ಪ್ರತ್ಯಾಗಮ -EOPNOTSUPP;
		}

		ಪ್ರತ್ಯಾಗಮ 0;
	}
#ಅನ್ಯಥಾ
	errno = ENOSYS;
	ಪ್ರತ್ಯಾಗಮ -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಯದಿ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ಯದಿ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		ವರ್ಣ *tmpname = ರಿಕ್ತ;

		f = xfmkstemp(&tmpname, ರಿಕ್ತ, "test");
		unlink(tmpname);
		ಮುಕ್ತ(tmpname);
		fclose(f);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ಯದಿ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--copy-file") == 0) {
		ಪೂರ್ಣಾಙ್ಕ ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ಯದಿ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		ಅನ್ಯಥಾ ಯದಿ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		ಪೂರ್ಣಾಙ್ಕ flags = O_PATH | O_CLOEXEC;
#ಅನ್ಯಥಾ
		ಪೂರ್ಣಾಙ್ಕ flags = O_RDONLY | O_CLOEXEC;
#endif
		ಪೂರ್ಣಾಙ್ಕ fallback = 0;
		ವರ್ಣ *name;
		ಪೂರ್ಣಾಙ್ಕ fd, i;

		ಯದಿ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ಪ್ರತಿ (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ಯದಿ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ಯದಿ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		ಅನ್ಯಥಾ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ಯದಿ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		ಮುಕ್ತ(name);
		close(fd);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned ಪೂರ್ಣಾಙ್ಕ mask = 0, retmask = 0;
		ಪೂರ್ಣಾಙ್ಕ nofollow = 0;
		ಸಂರಚನಾ stat st, ref;
		ಪೂರ್ಣಾಙ್ಕ rc, i;

		ಯದಿ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ಪ್ರತಿ (i = 3; i < argc; i++) {
			ಯದಿ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

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
		ಯದಿ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned ಪೂರ್ಣಾಙ್ಕ emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			ಯದಿ (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ಯದಿ (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ಯದಿ (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			ಯದಿ (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ಯದಿ (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			ಯದಿ (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			ಯದಿ (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif
