/*
 * This code is in the public domain; do with it what you wish.
 *
 * Copyright (C) 2012 Sami Kerola <kerolasa@iki.fi>
 * Copyright (C) 2012-2024 Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <sys/resource.h>
#నిర్వచితమైతే HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#అయితే_ముగింపు
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "canonicalize.h"
#అన్తర్భావ "fileutils.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "strutils.h"

పూర్ణాంకం mkstemp_cloexec(అక్షరం *మూస)
{
#నిర్వచితమైతే HAVE_MKOSTEMP
	తిరిగి mkostemp(మూస, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#లేకపోతే
	పూర్ణాంకం fd, old_flags, errno_save;

	fd = mkstemp(మూస);
	ఒకవేళ (fd < 0)
		తిరిగి fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ఒకవేళ (old_flags < 0)
		వెళ్ళు unwind;
	ఒకవేళ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		వెళ్ళు unwind;

	తిరిగి fd;

unwind:
	errno_save = errno;
	unlink(మూస);
	close(fd);
	errno = errno_save;

	తిరిగి -1;
#అయితే_ముగింపు
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
పూర్ణాంకం xmkstemp(అక్షరం **tmpname, స్థిరరాశి అక్షరం *dir, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	అక్షరం *localtmp;
	స్థిరరాశి అక్షరం *tmpenv;
	mode_t old_బహుళకం;
	పూర్ణాంకం fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ఒకవేళ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, ఉపసర్గ);
	ఒకవేళ (rc < 0)
		తిరిగి -1;

	old_బహుళకం = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_బహుళకం);
	ఒకవేళ (fd == -1) {
		విడుదల(localtmp);
		localtmp = ఖాళీ;
	}
	*tmpname = localtmp;
	తిరిగి fd;
}

#నిర్వచితమైతే F_DUPFD_CLOEXEC
పూర్ణాంకం dup_fd_cloexec(పూర్ణాంకం oldfd, పూర్ణాంకం lowfd)
#లేకపోతే
పూర్ణాంకం dup_fd_cloexec(పూర్ణాంకం oldfd, పూర్ణాంకం lowfd  __attribute__((__unused__)))
#అయితే_ముగింపు
{
	పూర్ణాంకం fd, flags, errno_save;

#నిర్వచితమైతే F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ఒకవేళ (fd >= 0)
		తిరిగి fd;
#అయితే_ముగింపు

	fd = dup(oldfd);
	ఒకవేళ (fd < 0)
		తిరిగి fd;

	flags = fcntl(fd, F_GETFD);
	ఒకవేళ (flags < 0)
		వెళ్ళు unwind;
	ఒకవేళ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		వెళ్ళు unwind;

	తిరిగి fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	తిరిగి -1;
}

/*
 * portable getdtablesize()
 */
గుర్తులేని పూర్ణాంకం get_fd_tabsize(శూన్య)
{
	పూర్ణాంకం m;

#ఒకవేళ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#లేదావేళ defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	నిర్మాణం rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#లేదావేళ defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#లేకపోతే
	m = OPEN_MAX;
#అయితే_ముగింపు
	తిరిగి m;
}

శూన్య ul_close_all_fds(గుర్తులేని పూర్ణాంకం మొదటి, గుర్తులేని పూర్ణాంకం చివరి)
{
	నిర్మాణం dirent *d;
	DIR *dir;

	dir = opendir(మార్గం_PROC_FDDIR);
	ఒకవేళ (dir) {
		అయితే ((d = xreaddir(dir))) {
			అక్షరం *చివర;
			గుర్తులేని పూర్ణాంకం fd;
			పూర్ణాంకం dfd;

			errno = 0;
			fd = strtoul(d->d_name, &చివర, 10);

			ఒకవేళ (errno || చివర == d->d_name || !చివర || *చివర)
				కొనసాగు;
			dfd = dirfd(dir);
			ఒకవేళ (dfd < 0)
				కొనసాగు;
			ఒకవేళ ((గుర్తులేని పూర్ణాంకం)dfd == fd)
				కొనసాగు;
			ఒకవేళ (fd < మొదటి || చివరి < fd)
				కొనసాగు;
			close(fd);
		}
		closedir(dir);
	} లేకపోతే {
		గుర్తులేని fd, tbsz = get_fd_tabsize();

		కొరకు (fd = 0; fd < tbsz; fd++) {
			ఒకవేళ (మొదటి <= fd && fd <= చివరి)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
అక్షరం *ul_restricted_మార్గం_oper(స్థిరరాశి అక్షరం *మార్గం,
		  పూర్ణాంకం (*oper)(స్థిరరాశి అక్షరం *మార్గం, అక్షరం **ఫలితం, శూన్య *దత్తాంశం),
		  శూన్య *దత్తాంశం)
{
	అక్షరం *ఫలితం = ఖాళీ;
	పూర్ణాంకం errsv = 0;
	పూర్ణాంకం pipes[2];
	ssize_t len;
	pid_t pid;

	ఒకవేళ (!మార్గం || !*మార్గం)
		తిరిగి ఖాళీ;

	ఒకవేళ (pipe(pipes) != 0)
		తిరిగి ఖాళీ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	ఎంపిక (pid) {
	స్థితి -1:
		close(pipes[0]);
		close(pipes[1]);
		తిరిగి ఖాళీ;			/* fork error */
	స్థితి 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ఒకవేళ (drop_permissions() != 0)
			ఫలితం = ఖాళీ;	/* failed */
		లేకపోతే
			oper(మార్గం, &ఫలితం, దత్తాంశం);

		len = ఫలితం ? (ssize_t) అక్షరశ్రేణి_పొడవు(ఫలితం) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (అక్షరం *) &len, పరిమాణం(len));
		ఒకవేళ (ఫలితం)
			ul_write_all(pipes[1], ఫలితం, len);
		_exit(0);
	అప్రమేయ:
		ఆపు;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ఒకవేళ (ul_read_all(pipes[0], (అక్షరం *) &len, పరిమాణం(len)) != పరిమాణం(len))
		వెళ్ళు పూర్తి;
	ఒకవేళ (len < 0) {
		errsv = -len;
		వెళ్ళు పూర్తి;
	}

	ఫలితం = జ్ఞాపకం_ఇవ్వు(len + 1);
	ఒకవేళ (!ఫలితం) {
		errsv = ENOMEM;
		వెళ్ళు పూర్తి;
	}
	/* read path */
	ఒకవేళ (ul_read_all(pipes[0], ఫలితం, len) != len) {
		errsv = errno;
		వెళ్ళు పూర్తి;
	}
	ఫలితం[len] = '\0';
పూర్తి:
	ఒకవేళ (errsv) {
		విడుదల(ఫలితం);
		ఫలితం = ఖాళీ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_ఫలితం( waitpid(pid, ఖాళీ, 0) );

	errno = errsv;
	తిరిగి ఫలితం;

}

పూర్ణాంకం ul_mkdir_p(స్థిరరాశి అక్షరం *మార్గం, mode_t బహుళకం)
{
	అక్షరం *p, *dir;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (!మార్గం || !*మార్గం)
		తిరిగి -EINVAL;

	dir = p = strdup(మార్గం);
	ఒకవేళ (!dir)
		తిరిగి -ENOMEM;

	ఒకవేళ (*p == '/')
		p++;

	అయితే (p && *p) {
		అక్షరం *e = strchr(p, '/');
		ఒకవేళ (e)
			*e = '\0';
		ఒకవేళ (*p) {
			rc = mkdir(dir, బహుళకం);
			ఒకవేళ (rc && errno != EEXIST)
				ఆపు;
			rc = 0;
		}
		ఒకవేళ (!e)
			ఆపు;
		*e = '/';
		p = e + 1;
	}

	విడుదల(dir);
	తిరిగి rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
అక్షరం *stripoff_చివరి_component(అక్షరం *మార్గం)
{
	అక్షరం *p = మార్గం ? strrchr(మార్గం, '/') : ఖాళీ;

	ఒకవేళ (!p)
		తిరిగి ఖాళీ;
	*p = '\0';
	తిరిగి p + 1;
}

స్థిర పూర్ణాంకం copy_file_simple(పూర్ణాంకం from, పూర్ణాంకం to)
{
	ssize_t nr;
	అక్షరం buf[BUFSIZ];

	అయితే ((nr = ul_read_all(from, buf, పరిమాణం(buf))) > 0)
		ఒకవేళ (ul_write_all(to, buf, nr) == -1)
			తిరిగి UL_COPY_WRITE_ERROR;
	ఒకవేళ (nr < 0)
		తిరిగి UL_COPY_READ_ERROR;
#నిర్వచితమైతే HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, పరిమాణం(buf));
#అయితే_ముగింపు
	తిరిగి 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
పూర్ణాంకం ul_copy_file(పూర్ణాంకం from, పూర్ణాంకం to)
{
#నిర్వచితమైతే HAVE_SENDFILE
	నిర్మాణం stat st;
	ssize_t nw;

	ఒకవేళ (fstat(from, &st) == -1)
		తిరిగి UL_COPY_READ_ERROR;
	ఒకవేళ (!S_ISREG(st.st_mode))
		తిరిగి copy_file_simple(from, to);
	ఒకవేళ (ul_sendfile_all(to, from, ఖాళీ, st.st_size) < 0)
		తిరిగి copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	అయితే ((nw = ul_sendfile_all(to, from, ఖాళీ, 16*1024*1024)) != 0)
		ఒకవేళ (nw < 0)
			తిరిగి copy_file_simple(from, to);
	తిరిగి 0;
#లేకపోతే
	తిరిగి copy_file_simple(from, to);
#అయితే_ముగింపు
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
అక్షరం *ul_fd_mkpath(అక్షరం *buf, size_t bufsz, పూర్ణాంకం fd)
{
	పూర్ణాంకం len;

	ఒకవేళ (fd < 0) {
		errno = EBADF;
		తిరిగి ఖాళీ;
	}

	len = snprintf(buf, bufsz, మార్గం_PROC_FDDIR "/%d", fd);
	ఒకవేళ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		తిరిగి ఖాళీ;
	}

	తిరిగి buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
అక్షరం *ul_fd_get_మార్గం(పూర్ణాంకం fd)
{
	ssize_t ssz;
	అక్షరం buf[PATH_MAX];
	అక్షరం fdpath[UL_FDPATH_BUFSIZ];

	ఒకవేళ (!ul_fd_mkpath(fdpath, పరిమాణం(fdpath), fd))
		తిరిగి ఖాళీ;

	ssz = readlink(fdpath, buf, పరిమాణం(buf));
	ఒకవేళ (ssz < 0)
		తిరిగి ఖాళీ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ఒకవేళ ((size_t) ssz >= పరిమాణం(buf)) {
		errno = ENAMETOOLONG;
		తిరిగి ఖాళీ;
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
	ఒకవేళ (*buf != '/' || ul_endswith(buf, మార్గం_DELETED_SUFFIX)) {
		errno = ENOENT;
		తిరిగి ఖాళీ;
	}

	తిరిగి strdup(buf);
}

పూర్ణాంకం ul_reopen(పూర్ణాంకం fd, పూర్ణాంకం flags)
{
	అక్షరం *మార్గం = ul_fd_get_మార్గం(fd);
	పూర్ణాంకం ret;

	ఒకవేళ (!మార్గం)
		తిరిగి -errno;

	ret = open(మార్గం, flags);
	విడుదల(మార్గం);

	తిరిగి ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
అక్షరం *ul_basename(అక్షరం *మార్గం)
{
	అక్షరం *p;

	ఒకవేళ (!మార్గం || !*మార్గం)
		తిరిగి (అక్షరం *) ".";	/* ugly, static string */

	p = strrchr(మార్గం, '/');
	ఒకవేళ (!p)
		తిరిగి మార్గం;		/* no '/', return original */

	ఒకవేళ (*(p + 1) != '\0')
		తిరిగి p + 1;		/* begin of the name */

	అయితే (p > మార్గం && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ఒకవేళ (p > మార్గం) {
		*p-- = '\0';
		అయితే (p > మార్గం && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} లేకపోతే అయితే (*(p + 1) != '\0')
		++p;

	తిరిగి p;
}

#నిర్వచితమైతే HAVE_OPENAT
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
FILE *fopen_at_no_link(పూర్ణాంకం dir, స్థిరరాశి అక్షరం *filename,
                             పూర్ణాంకం flags, mode_t perm, స్థిరరాశి అక్షరం *బహుళకం)
{
	FILE *fp;
	పూర్ణాంకం fd;
	నిర్మాణం stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ఒకవేళ (fd < 0)
		తిరిగి ఖాళీ;

	ఒకవేళ (fstat(fd, &st)) {
		close(fd);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		తిరిగి ఖాళీ;
	}

	ఒకవేళ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		తిరిగి ఖాళీ;
	}

	fp = fdopen(fd, బహుళకం);
	ఒకవేళ (!fp)
		close(fd);
	తిరిగి fp;
}
#అయితే_ముగింపు /* HAVE_OPENAT */

#ఒకవేళ defined(SYS_openat2)
పూర్ణాంకం ul_openat_resolve(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం flags,
		      mode_t బహుళకం, గుర్తులేని దీర్ఘ దీర్ఘ resolve)
{
	నిర్మాణం open_how how = {
		.flags = (__u64) flags,
		.బహుళకం = (__u64) బహుళకం,
		.resolve = resolve,
	};

	తిరిగి syscall(SYS_openat2, dirfd, మార్గం, &how, పరిమాణం(how));
}
#లేకపోతే
పూర్ణాంకం ul_openat_resolve(
		పూర్ణాంకం dirfd __attribute__((__unused__)),
		స్థిరరాశి అక్షరం *మార్గం __attribute__((__unused__)),
		పూర్ణాంకం flags __attribute__((__unused__)),
		mode_t బహుళకం __attribute__((__unused__)),
		గుర్తులేని దీర్ఘ దీర్ఘ resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	తిరిగి -1;
}
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
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
స్థిర పూర్ణాంకం open_no_symlinks_fallback(స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం flags, mode_t బహుళకం)
{
	అక్షరం *abspath = ఖాళీ, *kpath = ఖాళీ;
	నిర్మాణం stat st;
	పూర్ణాంకం fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ఒకవేళ (!మార్గం || (flags & O_CREAT)) {
		errno = ENOSYS;
		తిరిగి -1;
	}

	ఒకవేళ (ul_is_relative_మార్గం(మార్గం)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_మార్గం(మార్గం);
		ఒకవేళ (!abspath)
			తిరిగి -1;
	}

	ఒకవేళ (flags & O_మార్గం)
		fd = open(మార్గం, flags);
	లేకపోతే
		fd = open(మార్గం, O_మార్గం | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ఒకవేళ (fd < 0)
		వెళ్ళు fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ఒకవేళ (fstat(fd, &st) != 0)
		వెళ్ళు fail;
	ఒకవేళ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		వెళ్ళు fail;
	}

	kpath = ul_fd_get_మార్గం(fd);
	ఒకవేళ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		వెళ్ళు fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ఒకవేళ (streq_paths(abspath ? abspath : మార్గం, kpath) != 1) {
		errno = ELOOP;
		వెళ్ళు fail;
	}

	ఒకవేళ (!(flags & O_మార్గం)) {
		అక్షరం fdpath[UL_FDPATH_BUFSIZ];
		పూర్ణాంకం nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ఒకవేళ (ul_fd_mkpath(fdpath, పరిమాణం(fdpath), fd))
			nfd = open(fdpath, flags, బహుళకం);
		ఒకవేళ (nfd < 0)
			వెళ్ళు fail;
		close(fd);
		fd = nfd;
	}

	విడుదల(abspath);
	విడుదల(kpath);
	తిరిగి fd;
fail:
	errsv = errno;
	విడుదల(abspath);
	విడుదల(kpath);
	ఒకవేళ (fd >= 0)
		close(fd);
	errno = errsv;
	తిరిగి -1;
}
#లేకపోతే /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
స్థిర పూర్ణాంకం open_no_symlinks_fallback(
			స్థిరరాశి అక్షరం *మార్గం __attribute__((__unused__)),
			పూర్ణాంకం flags __attribute__((__unused__)),
			mode_t బహుళకం __attribute__((__unused__)))
{
	errno = ENOSYS;
	తిరిగి -1;
}
#అయితే_ముగింపు /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
పూర్ణాంకం ul_open_no_symlinks(స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం flags, mode_t బహుళకం)
{
	పూర్ణాంకం fd = ul_openat_resolve(AT_FDCWD, మార్గం, flags, బహుళకం,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ఒకవేళ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(మార్గం, flags, బహుళకం);

	తిరిగి fd;
}

#నిర్వచితమైతే HAVE_UL_సురక్షిత_STATX
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
పూర్ణాంకం ul_సురక్షిత_statx(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం flags,
		  గుర్తులేని పూర్ణాంకం ముసుగు, నిర్మాణం statx *stx)
{
	ప్రతిజ్ఞా(stx);

	memset(stx, 0, పరిమాణం(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#నిర్వచితమైతే AT_ఖాళీలు_మార్గం
	ఒకవేళ (!మార్గం || !*మార్గం)
		flags |= AT_ఖాళీలు_మార్గం;
#అయితే_ముగింపు
	ఒకవేళ (statx(dirfd, మార్గం ? మార్గం : "", flags, ముసుగు, stx) != 0)
		తిరిగి -errno;

	తిరిగి 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
శూన్య ul_statx_గమ్యం_stat(స్థిరరాశి నిర్మాణం statx *stx, నిర్మాణం stat *st)
{
	memset(st, 0, పరిమాణం(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ఒకవేళ (stx->stx_ముసుగు & STATX_TYPE)
		st->st_mode |= stx->stx_బహుళకం & S_IFMT;
	ఒకవేళ (stx->stx_ముసుగు & STATX_బహుళకం)
		st->st_mode |= stx->stx_బహుళకం & ~S_IFMT;

	ఒకవేళ (stx->stx_ముసుగు & STATX_INO)
		st->st_ino = stx->stx_ino;
	ఒకవేళ (stx->stx_ముసుగు & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ఒకవేళ (stx->stx_ముసుగు & STATX_UID)
		st->st_uid = stx->stx_uid;
	ఒకవేళ (stx->stx_ముసుగు & STATX_GID)
		st->st_gid = stx->stx_gid;
	ఒకవేళ (stx->stx_ముసుగు & STATX_కొలత)
		st->st_size = stx->stx_కొలత;
	ఒకవేళ (stx->stx_ముసుగు & STATX_ఖండాలు)
		st->st_blocks = stx->stx_ఖండాలు;

	ఒకవేళ (stx->stx_ముసుగు & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ఒకవేళ (stx->stx_ముసుగు & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ఒకవేళ (stx->stx_ముసుగు & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#అయితే_ముగింపు

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
పూర్ణాంకం ul_సురక్షిత_stat(స్థిరరాశి అక్షరం *లక్ష్యం, నిర్మాణం stat *st,
		 పూర్ణాంకం nofollow __attribute__((__unused__)),
		 గుర్తులేని పూర్ణాంకం ముసుగు __attribute__((__unused__)),
		 గుర్తులేని పూర్ణాంకం *retmask)
{
	ప్రతిజ్ఞా(లక్ష్యం);
	ప్రతిజ్ఞా(st);

	memset(st, 0, పరిమాణం(నిర్మాణం stat));
	ఒకవేళ (retmask)
		*retmask = 0;

#నిర్వచితమైతే HAVE_UL_సురక్షిత_STAT
	{
		నిర్మాణం statx stx = { 0 };
		పూర్ణాంకం rc;

		ముసుగు |= UL_STATX_ESSENTIAL;

		rc = ul_సురక్షిత_statx(AT_FDCWD, లక్ష్యం,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, ముసుగు, &stx);
		ఒకవేళ (rc)
			తిరిగి rc;

		ul_statx_గమ్యం_stat(&stx, st);

		ఒకవేళ (retmask)
			*retmask = stx.stx_ముసుగు;

		ఒకవేళ ((stx.stx_ముసుగు & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			తిరిగి -EOPNOTSUPP;
		}

		తిరిగి 0;
	}
#లేకపోతే
	errno = ENOSYS;
	తిరిగి -ENOSYS;
#అయితే_ముగింపు
}


#నిర్వచితమైతే TEST_కార్యక్రమం_FILEUTILS
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	ఒకవేళ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ఒకవేళ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		అక్షరం *tmpname = ఖాళీ;

		f = xfmkstemp(&tmpname, ఖాళీ, "test");
		unlink(tmpname);
		విడుదల(tmpname);
		fclose(f);

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_ఫలితం( dup(STDIN_FILENO) );
		ignore_ఫలితం( dup(STDIN_FILENO) );
		ignore_ఫలితం( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ఒకవేళ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--copy-file") == 0) {
		పూర్ణాంకం ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ఒకవేళ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		లేకపోతే ఒకవేళ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#నిర్వచితమైతే __linux__
		పూర్ణాంకం flags = O_మార్గం | O_CLOEXEC;
#లేకపోతే
		పూర్ణాంకం flags = O_RDONLY | O_CLOEXEC;
#అయితే_ముగింపు
		పూర్ణాంకం fallback = 0;
		అక్షరం *name;
		పూర్ణాంకం fd, i;

		ఒకవేళ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		కొరకు (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ఒకవేళ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			లేకపోతే ఒకవేళ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ఒకవేళ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		లేకపోతే
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ఒకవేళ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_మార్గం(fd);
		రాయి("%s\n", name);
		విడుదల(name);
		close(fd);

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--safe-stat") == 0) {
		గుర్తులేని పూర్ణాంకం ముసుగు = 0, retmask = 0;
		పూర్ణాంకం nofollow = 0;
		నిర్మాణం stat st, ref;
		పూర్ణాంకం rc, i;

		ఒకవేళ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		కొరకు (i = 3; i < argc; i++) {
			ఒకవేళ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			లేకపోతే ఒకవేళ (strcmp(argv[i], "--basic") == 0)
				ముసుగు |= UL_STATX_BASIC;
		}

		రాయి("%s:\n", argv[2]);

		rc = ul_సురక్షిత_stat(argv[2], &st, nofollow, ముసుగు, &retmask);

		/* the function returns -errno and it sets errno too */
		రాయి("      rc: %d\n", rc);
		రాయి("   errno: %d\n", rc ? errno : 0);
		ఒకవేళ (rc)
			తిరిగి EXIT_FAILURE;

		రాయి("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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
		ఒకవేళ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(EXIT_FAILURE, "%s", argv[2]);

		రాయి("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		రాయి(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#నిర్వచితమైతే HAVE_UL_సురక్షిత_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			గుర్తులేని పూర్ణాంకం emask = ముసుగు | UL_STATX_ESSENTIAL;

# define ఫలితం_2(బిట్టు, x)	(!(retmask & (బిట్టు)) || (x) ? "OK" : "FAILED")

			ఒకవేళ (emask & STATX_TYPE)
				రాయి("  S_IFMT: %s\n", ఫలితం_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ఒకవేళ (emask & STATX_బహుళకం)
				రాయి(" st_mode: %s\n", ఫలితం_2(STATX_బహుళకం,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ఒకవేళ (emask & STATX_INO)
				రాయి("  st_ino: %s\n",
					ఫలితం_2(STATX_INO, st.st_ino == ref.st_ino));
			ఒకవేళ (emask & STATX_NLINK)
				రాయి("st_nlink: %s\n",
					ఫలితం_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ఒకవేళ (emask & STATX_UID)
				రాయి("  st_uid: %s\n",
					ఫలితం_2(STATX_UID, st.st_uid == ref.st_uid));
			ఒకవేళ (emask & STATX_GID)
				రాయి("  st_gid: %s\n",
					ఫలితం_2(STATX_GID, st.st_gid == ref.st_gid));
			ఒకవేళ (emask & STATX_కొలత)
				రాయి(" st_size: %s\n",
					ఫలితం_2(STATX_కొలత, st.st_size == ref.st_size));
# undef ఫలితం_2
		}
#అయితే_ముగింపు
	}
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు
