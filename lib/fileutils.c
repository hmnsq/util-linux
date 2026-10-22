/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <sys/resource.h>
#ifdef HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#endif
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "canonicalize.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mkstemp_cloexec(ଅକ୍ଷର *template)
{
#ifdef HAVE_MKOSTEMP
	ଫେରନ୍ତୁ mkostemp(template, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, old_flags, errno_save;

	fd = mkstemp(template);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ଯଦି (old_flags < 0)
		ଯାଅ unwind;
	ଯଦି (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		ଯାଅ unwind;

	ଫେରନ୍ତୁ fd;

unwind:
	errno_save = errno;
	unlink(template);
	close(fd);
	errno = errno_save;

	ଫେରନ୍ତୁ -1;
#endif
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା xmkstemp(ଅକ୍ଷର **tmpname, ଅଚଳ ଅକ୍ଷର *dir, ଅଚଳ ଅକ୍ଷର *prefix)
{
	ଅକ୍ଷର *localtmp;
	ଅଚଳ ଅକ୍ଷର *tmpenv;
	mode_t old_mode;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ଯଦି (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, prefix);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ -1;

	old_mode = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_mode);
	ଯଦି (fd == -1) {
		ମୁକ୍ତ(localtmp);
		localtmp = ଖାଲି;
	}
	*tmpname = localtmp;
	ଫେରନ୍ତୁ fd;
}

#ifdef F_DUPFD_CLOEXEC
ପୂର୍ଣ୍ଣସଂଖ୍ୟା dup_fd_cloexec(ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldfd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lowfd)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା dup_fd_cloexec(ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldfd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lowfd  __attribute__((__unused__)))
#endif
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, flags, errno_save;

#ifdef F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ଯଦି (fd >= 0)
		ଫେରନ୍ତୁ fd;
#endif

	fd = dup(oldfd);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ fd;

	flags = fcntl(fd, F_GETFD);
	ଯଦି (flags < 0)
		ଯାଅ unwind;
	ଯଦି (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		ଯାଅ unwind;

	ଫେରନ୍ତୁ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ଫେରନ୍ତୁ -1;
}

/*
 * portable getdtablesize()
 */
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_fd_tabsize(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା m;

#ଯଦି defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#ନଚେତଯଦି defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ସଂରଚନା rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#ନଚେତଯଦି defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#ଅନ୍ୟଥା
	m = OPEN_MAX;
#endif
	ଫେରନ୍ତୁ m;
}

ଶୂନ୍ୟ ul_close_all_fds(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା first, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା last)
{
	ସଂରଚନା dirent *d;
	DIR *dir;

	dir = opendir(_PATH_PROC_FDDIR);
	ଯଦି (dir) {
		ଯେତେବେଳେ ((d = xreaddir(dir))) {
			ଅକ୍ଷର *end;
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା dfd;

			errno = 0;
			fd = strtoul(d->d_name, &end, 10);

			ଯଦି (errno || end == d->d_name || !end || *end)
				ଜାରି;
			dfd = dirfd(dir);
			ଯଦି (dfd < 0)
				ଜାରି;
			ଯଦି ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dfd == fd)
				ଜାରି;
			ଯଦି (fd < first || last < fd)
				ଜାରି;
			close(fd);
		}
		closedir(dir);
	} ଅନ୍ୟଥା {
		unsigned fd, tbsz = get_fd_tabsize();

		ପାଇଁ (fd = 0; fd < tbsz; fd++) {
			ଯଦି (first <= fd && fd <= last)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
ଅକ୍ଷର *ul_restricted_path_oper(ଅଚଳ ଅକ୍ଷର *path,
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*oper)(ଅଚଳ ଅକ୍ଷର *path, ଅକ୍ଷର **result, ଶୂନ୍ୟ *data),
		  ଶୂନ୍ୟ *data)
{
	ଅକ୍ଷର *result = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା errsv = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipes[2];
	ssize_t len;
	pid_t pid;

	ଯଦି (!path || !*path)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (pipe(pipes) != 0)
		ଫେରନ୍ତୁ ଖାଲି;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	ବାଛ (pid) {
	ସ୍ଥିତି -1:
		close(pipes[0]);
		close(pipes[1]);
		ଫେରନ୍ତୁ ଖାଲି;			/* fork error */
	ସ୍ଥିତି 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ଯଦି (drop_permissions() != 0)
			result = ଖାଲି;	/* failed */
		ଅନ୍ୟଥା
			oper(path, &result, data);

		len = result ? (ssize_t) strlen(result) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (ଅକ୍ଷର *) &len, ଆକାର(len));
		ଯଦି (result)
			ul_write_all(pipes[1], result, len);
		_exit(0);
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ଯଦି (ul_read_all(pipes[0], (ଅକ୍ଷର *) &len, ଆକାର(len)) != ଆକାର(len))
		ଯାଅ done;
	ଯଦି (len < 0) {
		errsv = -len;
		ଯାଅ done;
	}

	result = malloc(len + 1);
	ଯଦି (!result) {
		errsv = ENOMEM;
		ଯାଅ done;
	}
	/* read path */
	ଯଦି (ul_read_all(pipes[0], result, len) != len) {
		errsv = errno;
		ଯାଅ done;
	}
	result[len] = '\0';
done:
	ଯଦି (errsv) {
		ମୁକ୍ତ(result);
		result = ଖାଲି;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_result( waitpid(pid, ଖାଲି, 0) );

	errno = errsv;
	ଫେରନ୍ତୁ result;

}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_mkdir_p(ଅଚଳ ଅକ୍ଷର *path, mode_t mode)
{
	ଅକ୍ଷର *p, *dir;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (!path || !*path)
		ଫେରନ୍ତୁ -EINVAL;

	dir = p = strdup(path);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ -ENOMEM;

	ଯଦି (*p == '/')
		p++;

	ଯେତେବେଳେ (p && *p) {
		ଅକ୍ଷର *e = strchr(p, '/');
		ଯଦି (e)
			*e = '\0';
		ଯଦି (*p) {
			rc = mkdir(dir, mode);
			ଯଦି (rc && errno != EEXIST)
				ଭାଙ୍ଗ;
			rc = 0;
		}
		ଯଦି (!e)
			ଭାଙ୍ଗ;
		*e = '/';
		p = e + 1;
	}

	ମୁକ୍ତ(dir);
	ଫେରନ୍ତୁ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
ଅକ୍ଷର *stripoff_last_component(ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *p = path ? strrchr(path, '/') : ଖାଲି;

	ଯଦି (!p)
		ଫେରନ୍ତୁ ଖାଲି;
	*p = '\0';
	ଫେରନ୍ତୁ p + 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା copy_file_simple(ପୂର୍ଣ୍ଣସଂଖ୍ୟା from, ପୂର୍ଣ୍ଣସଂଖ୍ୟା to)
{
	ssize_t nr;
	ଅକ୍ଷର buf[BUFSIZ];

	ଯେତେବେଳେ ((nr = ul_read_all(from, buf, ଆକାର(buf))) > 0)
		ଯଦି (ul_write_all(to, buf, nr) == -1)
			ଫେରନ୍ତୁ UL_COPY_WRITE_ERROR;
	ଯଦି (nr < 0)
		ଫେରନ୍ତୁ UL_COPY_READ_ERROR;
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, ଆକାର(buf));
#endif
	ଫେରନ୍ତୁ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_copy_file(ପୂର୍ଣ୍ଣସଂଖ୍ୟା from, ପୂର୍ଣ୍ଣସଂଖ୍ୟା to)
{
#ifdef HAVE_SENDFILE
	ସଂରଚନା stat st;
	ssize_t nw;

	ଯଦି (fstat(from, &st) == -1)
		ଫେରନ୍ତୁ UL_COPY_READ_ERROR;
	ଯଦି (!S_ISREG(st.st_mode))
		ଫେରନ୍ତୁ copy_file_simple(from, to);
	ଯଦି (ul_sendfile_all(to, from, ଖାଲି, st.st_size) < 0)
		ଫେରନ୍ତୁ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ଯେତେବେଳେ ((nw = ul_sendfile_all(to, from, ଖାଲି, 16*1024*1024)) != 0)
		ଯଦି (nw < 0)
			ଫେରନ୍ତୁ copy_file_simple(from, to);
	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ copy_file_simple(from, to);
#endif
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
ଅକ୍ଷର *ul_fd_mkpath(ଅକ୍ଷର *buf, size_t bufsz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	ଯଦି (fd < 0) {
		errno = EBADF;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	len = snprintf(buf, bufsz, _PATH_PROC_FDDIR "/%d", fd);
	ଯଦି (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
ଅକ୍ଷର *ul_fd_get_path(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ssize_t ssz;
	ଅକ୍ଷର buf[PATH_MAX];
	ଅକ୍ଷର fdpath[UL_FDPATH_BUFSIZ];

	ଯଦି (!ul_fd_mkpath(fdpath, ଆକାର(fdpath), fd))
		ଫେରନ୍ତୁ ଖାଲି;

	ssz = readlink(fdpath, buf, ଆକାର(buf));
	ଯଦି (ssz < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ଯଦି ((size_t) ssz >= ଆକାର(buf)) {
		errno = ENAMETOOLONG;
		ଫେରନ୍ତୁ ଖାଲି;
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
	ଯଦି (*buf != '/' || ul_endswith(buf, PATH_DELETED_SUFFIX)) {
		errno = ENOENT;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ strdup(buf);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_reopen(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଅକ୍ଷର *path = ul_fd_get_path(fd);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (!path)
		ଫେରନ୍ତୁ -errno;

	ret = open(path, flags);
	ମୁକ୍ତ(path);

	ଫେରନ୍ତୁ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
ଅକ୍ଷର *ul_basename(ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *p;

	ଯଦି (!path || !*path)
		ଫେରନ୍ତୁ (ଅକ୍ଷର *) ".";	/* ugly, static string */

	p = strrchr(path, '/');
	ଯଦି (!p)
		ଫେରନ୍ତୁ path;		/* no '/', return original */

	ଯଦି (*(p + 1) != '\0')
		ଫେରନ୍ତୁ p + 1;		/* begin of the name */

	ଯେତେବେଳେ (p > path && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ଯଦି (p > path) {
		*p-- = '\0';
		ଯେତେବେଳେ (p > path && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} ଅନ୍ୟଥା ଯେତେବେଳେ (*(p + 1) != '\0')
		++p;

	ଫେରନ୍ତୁ p;
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
FILE *fopen_at_no_link(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dir, ଅଚଳ ଅକ୍ଷର *filename,
                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, mode_t perm, ଅଚଳ ଅକ୍ଷର *mode)
{
	FILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ସଂରଚନା stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (fstat(fd, &st)) {
		close(fd);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	fp = fdopen(fd, mode);
	ଯଦି (!fp)
		close(fd);
	ଫେରନ୍ତୁ fp;
}
#endif /* HAVE_OPENAT */

#ଯଦି defined(SYS_openat2)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_openat_resolve(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
		      mode_t mode, unsigned long long resolve)
{
	ସଂରଚନା open_how how = {
		.flags = (__u64) flags,
		.mode = (__u64) mode,
		.resolve = resolve,
	};

	ଫେରନ୍ତୁ syscall(SYS_openat2, dirfd, path, &how, ଆକାର(how));
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_openat_resolve(
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd __attribute__((__unused__)),
		ଅଚଳ ଅକ୍ଷର *path __attribute__((__unused__)),
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags __attribute__((__unused__)),
		mode_t mode __attribute__((__unused__)),
		unsigned long long resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ଫେରନ୍ତୁ -1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_no_symlinks_fallback(ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, mode_t mode)
{
	ଅକ୍ଷର *abspath = ଖାଲି, *kpath = ଖାଲି;
	ସଂରଚନା stat st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ଯଦି (!path || (flags & O_CREAT)) {
		errno = ENOSYS;
		ଫେରନ୍ତୁ -1;
	}

	ଯଦି (ul_is_relative_path(path)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_path(path);
		ଯଦି (!abspath)
			ଫେରନ୍ତୁ -1;
	}

	ଯଦି (flags & O_PATH)
		fd = open(path, flags);
	ଅନ୍ୟଥା
		fd = open(path, O_PATH | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ଯଦି (fd < 0)
		ଯାଅ fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ଯଦି (fstat(fd, &st) != 0)
		ଯାଅ fail;
	ଯଦି (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		ଯାଅ fail;
	}

	kpath = ul_fd_get_path(fd);
	ଯଦି (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		ଯାଅ fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ଯଦି (streq_paths(abspath ? abspath : path, kpath) != 1) {
		errno = ELOOP;
		ଯାଅ fail;
	}

	ଯଦି (!(flags & O_PATH)) {
		ଅକ୍ଷର fdpath[UL_FDPATH_BUFSIZ];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ଯଦି (ul_fd_mkpath(fdpath, ଆକାର(fdpath), fd))
			nfd = open(fdpath, flags, mode);
		ଯଦି (nfd < 0)
			ଯାଅ fail;
		close(fd);
		fd = nfd;
	}

	ମୁକ୍ତ(abspath);
	ମୁକ୍ତ(kpath);
	ଫେରନ୍ତୁ fd;
fail:
	errsv = errno;
	ମୁକ୍ତ(abspath);
	ମୁକ୍ତ(kpath);
	ଯଦି (fd >= 0)
		close(fd);
	errno = errsv;
	ଫେରନ୍ତୁ -1;
}
#ଅନ୍ୟଥା /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_no_symlinks_fallback(
			ଅଚଳ ଅକ୍ଷର *path __attribute__((__unused__)),
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags __attribute__((__unused__)),
			mode_t mode __attribute__((__unused__)))
{
	errno = ENOSYS;
	ଫେରନ୍ତୁ -1;
}
#endif /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_open_no_symlinks(ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, mode_t mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = ul_openat_resolve(AT_FDCWD, path, flags, mode,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ଯଦି (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(path, flags, mode);

	ଫେରନ୍ତୁ fd;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_safe_statx(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
		  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask, ସଂରଚନା statx *stx)
{
	ପ୍ରତିଜ୍ଞା(stx);

	memset(stx, 0, ଆକାର(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ifdef AT_EMPTY_PATH
	ଯଦି (!path || !*path)
		flags |= AT_EMPTY_PATH;
#endif
	ଯଦି (statx(dirfd, path ? path : "", flags, mask, stx) != 0)
		ଫେରନ୍ତୁ -errno;

	ଫେରନ୍ତୁ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
ଶୂନ୍ୟ ul_statx_to_stat(ଅଚଳ ସଂରଚନା statx *stx, ସଂରଚନା stat *st)
{
	memset(st, 0, ଆକାର(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ଯଦି (stx->stx_mask & STATX_TYPE)
		st->st_mode |= stx->stx_mode & S_IFMT;
	ଯଦି (stx->stx_mask & STATX_MODE)
		st->st_mode |= stx->stx_mode & ~S_IFMT;

	ଯଦି (stx->stx_mask & STATX_INO)
		st->st_ino = stx->stx_ino;
	ଯଦି (stx->stx_mask & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ଯଦି (stx->stx_mask & STATX_UID)
		st->st_uid = stx->stx_uid;
	ଯଦି (stx->stx_mask & STATX_GID)
		st->st_gid = stx->stx_gid;
	ଯଦି (stx->stx_mask & STATX_SIZE)
		st->st_size = stx->stx_size;
	ଯଦି (stx->stx_mask & STATX_BLOCKS)
		st->st_blocks = stx->stx_blocks;

	ଯଦି (stx->stx_mask & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ଯଦି (stx->stx_mask & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ଯଦି (stx->stx_mask & STATX_CTIME) {
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_safe_stat(ଅଚଳ ଅକ୍ଷର *target, ସଂରଚନା stat *st,
		 ପୂର୍ଣ୍ଣସଂଖ୍ୟା nofollow __attribute__((__unused__)),
		 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask __attribute__((__unused__)),
		 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *retmask)
{
	ପ୍ରତିଜ୍ଞା(target);
	ପ୍ରତିଜ୍ଞା(st);

	memset(st, 0, ଆକାର(ସଂରଚନା stat));
	ଯଦି (retmask)
		*retmask = 0;

#ifdef HAVE_UL_SAFE_STAT
	{
		ସଂରଚନା statx stx = { 0 };
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

		mask |= UL_STATX_ESSENTIAL;

		rc = ul_safe_statx(AT_FDCWD, target,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, mask, &stx);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;

		ul_statx_to_stat(&stx, st);

		ଯଦି (retmask)
			*retmask = stx.stx_mask;

		ଯଦି ((stx.stx_mask & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ଫେରନ୍ତୁ -EOPNOTSUPP;
		}

		ଫେରନ୍ତୁ 0;
	}
#ଅନ୍ୟଥା
	errno = ENOSYS;
	ଫେରନ୍ତୁ -ENOSYS;
#endif
}


#ifdef TEST_PROGRAM_FILEUTILS
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଯଦି (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ଯଦି (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		ଅକ୍ଷର *tmpname = ଖାଲି;

		f = xfmkstemp(&tmpname, ଖାଲି, "test");
		unlink(tmpname);
		ମୁକ୍ତ(tmpname);
		fclose(f);

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--close-fds") == 0) {
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );
		ignore_result( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ଯଦି (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--copy-file") == 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ଯଦି (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		ଅନ୍ୟଥା ଯଦି (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ifdef __linux__
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = O_PATH | O_CLOEXEC;
#ଅନ୍ୟଥା
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = O_RDONLY | O_CLOEXEC;
#endif
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fallback = 0;
		ଅକ୍ଷର *name;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, i;

		ଯଦି (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ପାଇଁ (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ଯଦି (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			ଅନ୍ୟଥା ଯଦି (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ଯଦି (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		ଅନ୍ୟଥା
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ଯଦି (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_path(fd);
		printf("%s\n", name);
		ମୁକ୍ତ(name);
		close(fd);

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--safe-stat") == 0) {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = 0, retmask = 0;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା nofollow = 0;
		ସଂରଚନା stat st, ref;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, i;

		ଯଦି (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ପାଇଁ (i = 3; i < argc; i++) {
			ଯଦି (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			ଅନ୍ୟଥା ଯଦି (strcmp(argv[i], "--basic") == 0)
				mask |= UL_STATX_BASIC;
		}

		printf("%s:\n", argv[2]);

		rc = ul_safe_stat(argv[2], &st, nofollow, mask, &retmask);

		/* the function returns -errno and it sets errno too */
		printf("      rc: %d\n", rc);
		printf("   errno: %d\n", rc ? errno : 0);
		ଯଦି (rc)
			ଫେରନ୍ତୁ EXIT_FAILURE;

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
		ଯଦି (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
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
			unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା emask = mask | UL_STATX_ESSENTIAL;

# define RESULT(bit, x)	(!(retmask & (bit)) || (x) ? "OK" : "FAILED")

			ଯଦି (emask & STATX_TYPE)
				printf("  S_IFMT: %s\n", RESULT(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ଯଦି (emask & STATX_MODE)
				printf(" st_mode: %s\n", RESULT(STATX_MODE,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ଯଦି (emask & STATX_INO)
				printf("  st_ino: %s\n",
					RESULT(STATX_INO, st.st_ino == ref.st_ino));
			ଯଦି (emask & STATX_NLINK)
				printf("st_nlink: %s\n",
					RESULT(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ଯଦି (emask & STATX_UID)
				printf("  st_uid: %s\n",
					RESULT(STATX_UID, st.st_uid == ref.st_uid));
			ଯଦି (emask & STATX_GID)
				printf("  st_gid: %s\n",
					RESULT(STATX_GID, st.st_gid == ref.st_gid));
			ଯଦି (emask & STATX_SIZE)
				printf(" st_size: %s\n",
					RESULT(STATX_SIZE, st.st_size == ref.st_size));
# undef RESULT
		}
#endif
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif
