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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_SYSCALL_H
# include <sys/syscall.h>
#ಆದರೆ_ಅಂತ್ಯ
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

ಪೂರ್ಣಾಂಕ mkstemp_cloexec(ಅಕ್ಷರ *ಮಾದರಿ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MKOSTEMP
	ಹಿಂತಿರುಗಿ mkostemp(ಮಾದರಿ, O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC);
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ fd, old_flags, errno_save;

	fd = mkstemp(ಮಾದರಿ);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ fd;

	old_flags = fcntl(fd, F_GETFD, 0);
	ಆದರೆ (old_flags < 0)
		ಹೋಗು unwind;
	ಆದರೆ (fcntl(fd, F_SETFD, old_flags | O_CLOEXEC) < 0)
		ಹೋಗು unwind;

	ಹಿಂತಿರುಗಿ fd;

unwind:
	errno_save = errno;
	unlink(ಮಾದರಿ);
	close(fd);
	errno = errno_save;

	ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
}

/* Create open temporary file in safe way.  Please notice that the
 * file permissions are -rw------- by default. */
ಪೂರ್ಣಾಂಕ xmkstemp(ಅಕ್ಷರ **tmpname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ಅಕ್ಷರ *localtmp;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tmpenv;
	mode_t old_ಬಹುಲಕ;
	ಪೂರ್ಣಾಂಕ fd, rc;

	/* Some use cases must be capable of being moved atomically
	 * with rename(2), which is the reason why dir is here.  */
	tmpenv = dir ? dir : getenv("TMPDIR");
	ಆದರೆ (!tmpenv)
		tmpenv = _PATH_TMP;

	rc = asprintf(&localtmp, "%s/%s.XXXXXX", tmpenv, ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ -1;

	old_ಬಹುಲಕ = umask(077);
	fd = mkstemp_cloexec(localtmp);
	umask(old_ಬಹುಲಕ);
	ಆದರೆ (fd == -1) {
		ಬಿಡುಗಡೆ(localtmp);
		localtmp = ಖಾಲಿ;
	}
	*tmpname = localtmp;
	ಹಿಂತಿರುಗಿ fd;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ F_DUPFD_CLOEXEC
ಪೂರ್ಣಾಂಕ dup_fd_cloexec(ಪೂರ್ಣಾಂಕ oldfd, ಪೂರ್ಣಾಂಕ lowfd)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ dup_fd_cloexec(ಪೂರ್ಣಾಂಕ oldfd, ಪೂರ್ಣಾಂಕ lowfd  __attribute__((__unused__)))
#ಆದರೆ_ಅಂತ್ಯ
{
	ಪೂರ್ಣಾಂಕ fd, flags, errno_save;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ F_DUPFD_CLOEXEC
	fd = fcntl(oldfd, F_DUPFD_CLOEXEC, lowfd);
	ಆದರೆ (fd >= 0)
		ಹಿಂತಿರುಗಿ fd;
#ಆದರೆ_ಅಂತ್ಯ

	fd = dup(oldfd);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ fd;

	flags = fcntl(fd, F_GETFD);
	ಆದರೆ (flags < 0)
		ಹೋಗು unwind;
	ಆದರೆ (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		ಹೋಗು unwind;

	ಹಿಂತಿರುಗಿ fd;

unwind:
	errno_save = errno;
	close(fd);
	errno = errno_save;

	ಹಿಂತಿರುಗಿ -1;
}

/*
 * portable getdtablesize()
 */
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ get_fd_tabsize(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ m;

#ಆದರೆ defined(HAVE_GETDTABLESIZE)
	m = getdtablesize();
#ಅಥವಾಆದರೆ defined(HAVE_GETRLIMIT) && defined(RLIMIT_NOFILE)
	ರಚನೆ rlimit rl;

	getrlimit(RLIMIT_NOFILE, &rl);
	m = rl.rlim_cur;
#ಅಥವಾಆದರೆ defined(HAVE_SYSCONF) && defined(_SC_OPEN_MAX)
	m = sysconf(_SC_OPEN_MAX);
#ಇಲ್ಲದಿದ್ದರೆ
	m = OPEN_MAX;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ m;
}

ಶೂನ್ಯ ul_close_all_fds(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮೊದಲ, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೊನೆಯ)
{
	ರಚನೆ dirent *d;
	DIR *dir;

	dir = opendir(ಮಾರ್ಗ_PROC_FDDIR);
	ಆದರೆ (dir) {
		ಆಗ ((d = xreaddir(dir))) {
			ಅಕ್ಷರ *ಕೊನೆ;
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ fd;
			ಪೂರ್ಣಾಂಕ dfd;

			errno = 0;
			fd = strtoul(d->d_name, &ಕೊನೆ, 10);

			ಆದರೆ (errno || ಕೊನೆ == d->d_name || !ಕೊನೆ || *ಕೊನೆ)
				ಮುಂದುವರಿ;
			dfd = dirfd(dir);
			ಆದರೆ (dfd < 0)
				ಮುಂದುವರಿ;
			ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)dfd == fd)
				ಮುಂದುವರಿ;
			ಆದರೆ (fd < ಮೊದಲ || ಕೊನೆಯ < fd)
				ಮುಂದುವರಿ;
			close(fd);
		}
		closedir(dir);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಚಿಹ್ನೆರಹಿತ fd, tbsz = get_fd_tabsize();

		ಗಾಗಿ (fd = 0; fd < tbsz; fd++) {
			ಆದರೆ (ಮೊದಲ <= fd && fd <= ಕೊನೆಯ)
				close(fd);
		}
	}
}

/*
 * Fork, drop permissions, and call oper() and return result.
 */
ಅಕ್ಷರ *ul_restricted_ಮಾರ್ಗ_oper(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
		  ಪೂರ್ಣಾಂಕ (*oper)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಅಕ್ಷರ **ಫಲಿತಾಂಶ, ಶೂನ್ಯ *ದತ್ತಾಂಶ),
		  ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ಅಕ್ಷರ *ಫಲಿತಾಂಶ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ errsv = 0;
	ಪೂರ್ಣಾಂಕ pipes[2];
	ssize_t len;
	pid_t pid;

	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (pipe(pipes) != 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	/*
	 * To accurately assume identity of getuid() we must use setuid()
	 * but if we do that, we lose ability to reassume euid of 0, so
	 * we fork to do the check to keep euid intact.
	 */
	pid = fork();
	ಆಯ್ಕೆ (pid) {
	ಸ್ಥಿತಿ -1:
		close(pipes[0]);
		close(pipes[1]);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;			/* fork error */
	ಸ್ಥಿತಿ 0:
		close(pipes[0]);		/* close unused end */
		pipes[0] = -1;
		errno = 0;

		ಆದರೆ (drop_permissions() != 0)
			ಫಲಿತಾಂಶ = ಖಾಲಿ;	/* failed */
		ಇಲ್ಲದಿದ್ದರೆ
			oper(ಮಾರ್ಗ, &ಫಲಿತಾಂಶ, ದತ್ತಾಂಶ);

		len = ಫಲಿತಾಂಶ ? (ssize_t) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಫಲಿತಾಂಶ) :
		          errno ? -errno : -EINVAL;

		/* send length or errno */
		ul_write_all(pipes[1], (ಅಕ್ಷರ *) &len, ಗಾತ್ರ(len));
		ಆದರೆ (ಫಲಿತಾಂಶ)
			ul_write_all(pipes[1], ಫಲಿತಾಂಶ, len);
		_exit(0);
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}

	close(pipes[1]);		/* close unused end */
	pipes[1] = -1;

	/* read size or -errno */
	ಆದರೆ (ul_read_all(pipes[0], (ಅಕ್ಷರ *) &len, ಗಾತ್ರ(len)) != ಗಾತ್ರ(len))
		ಹೋಗು ಮುಗಿದ;
	ಆದರೆ (len < 0) {
		errsv = -len;
		ಹೋಗು ಮುಗಿದ;
	}

	ಫಲಿತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(len + 1);
	ಆದರೆ (!ಫಲಿತಾಂಶ) {
		errsv = ENOMEM;
		ಹೋಗು ಮುಗಿದ;
	}
	/* read path */
	ಆದರೆ (ul_read_all(pipes[0], ಫಲಿತಾಂಶ, len) != len) {
		errsv = errno;
		ಹೋಗು ಮುಗಿದ;
	}
	ಫಲಿತಾಂಶ[len] = '\0';
ಮುಗಿದ:
	ಆದರೆ (errsv) {
		ಬಿಡುಗಡೆ(ಫಲಿತಾಂಶ);
		ಫಲಿತಾಂಶ = ಖಾಲಿ;
	}
	close(pipes[0]);

	/* We make a best effort to reap child */
	ignore_ಫಲಿತಾಂಶ( waitpid(pid, ಖಾಲಿ, 0) );

	errno = errsv;
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;

}

ಪೂರ್ಣಾಂಕ ul_mkdir_p(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, mode_t ಬಹುಲಕ)
{
	ಅಕ್ಷರ *p, *dir;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ -EINVAL;

	dir = p = strdup(ಮಾರ್ಗ);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	ಆದರೆ (*p == '/')
		p++;

	ಆಗ (p && *p) {
		ಅಕ್ಷರ *e = strchr(p, '/');
		ಆದರೆ (e)
			*e = '\0';
		ಆದರೆ (*p) {
			rc = mkdir(dir, ಬಹುಲಕ);
			ಆದರೆ (rc && errno != EEXIST)
				ನಿಲ್ಲಿಸು;
			rc = 0;
		}
		ಆದರೆ (!e)
			ನಿಲ್ಲಿಸು;
		*e = '/';
		p = e + 1;
	}

	ಬಿಡುಗಡೆ(dir);
	ಹಿಂತಿರುಗಿ rc;
}

/* returns basename and keeps dirname in the @path, if @path is "/" (root)
 * then returns empty string */
ಅಕ್ಷರ *stripoff_ಕೊನೆಯ_component(ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *p = ಮಾರ್ಗ ? strrchr(ಮಾರ್ಗ, '/') : ಖಾಲಿ;

	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	*p = '\0';
	ಹಿಂತಿರುಗಿ p + 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ copy_file_simple(ಪೂರ್ಣಾಂಕ from, ಪೂರ್ಣಾಂಕ to)
{
	ssize_t nr;
	ಅಕ್ಷರ buf[BUFSIZ];

	ಆಗ ((nr = ul_read_all(from, buf, ಗಾತ್ರ(buf))) > 0)
		ಆದರೆ (ul_write_all(to, buf, nr) == -1)
			ಹಿಂತಿರುಗಿ UL_COPY_WRITE_ERROR;
	ಆದರೆ (nr < 0)
		ಹಿಂತಿರುಗಿ UL_COPY_READ_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_EXPLICIT_BZERO
	explicit_bzero(buf, ಗಾತ್ರ(buf));
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}

/* Copies the contents of a file. Returns -1 on read error, -2 on write error. */
ಪೂರ್ಣಾಂಕ ul_copy_file(ಪೂರ್ಣಾಂಕ from, ಪೂರ್ಣಾಂಕ to)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SENDFILE
	ರಚನೆ stat st;
	ssize_t nw;

	ಆದರೆ (fstat(from, &st) == -1)
		ಹಿಂತಿರುಗಿ UL_COPY_READ_ERROR;
	ಆದರೆ (!S_ISREG(st.st_mode))
		ಹಿಂತಿರುಗಿ copy_file_simple(from, to);
	ಆದರೆ (ul_sendfile_all(to, from, ಖಾಲಿ, st.st_size) < 0)
		ಹಿಂತಿರುಗಿ copy_file_simple(from, to);
	/* ensure we either get an EOF or an error */
	ಆಗ ((nw = ul_sendfile_all(to, from, ಖಾಲಿ, 16*1024*1024)) != 0)
		ಆದರೆ (nw < 0)
			ಹಿಂತಿರುಗಿ copy_file_simple(from, to);
	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ copy_file_simple(from, to);
#ಆದರೆ_ಅಂತ್ಯ
}

/* Composes the /proc/self/fd/<fd> pathname for @fd. The @bufsz has to be at
 * least UL_FDPATH_BUFSIZ bytes.
 *
 * This is the only place where the /proc/self/fd/ pathnames are generated.
 *
 * Returns @buf, or NULL on error.
 */
ಅಕ್ಷರ *ul_fd_mkpath(ಅಕ್ಷರ *buf, size_t bufsz, ಪೂರ್ಣಾಂಕ fd)
{
	ಪೂರ್ಣಾಂಕ len;

	ಆದರೆ (fd < 0) {
		errno = EBADF;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	len = snprintf(buf, bufsz, ಮಾರ್ಗ_PROC_FDDIR "/%d", fd);
	ಆದರೆ (len < 0 || (size_t) len >= bufsz) {
		errno = ENAMETOOLONG;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ buf;
}

/* Returns the pathname the @fd refers to as used by the kernel, or NULL on
 * error. The result has to be deallocated by free().
 */
ಅಕ್ಷರ *ul_fd_get_ಮಾರ್ಗ(ಪೂರ್ಣಾಂಕ fd)
{
	ssize_t ssz;
	ಅಕ್ಷರ buf[PATH_MAX];
	ಅಕ್ಷರ fdpath[UL_FDPATH_BUFSIZ];

	ಆದರೆ (!ul_fd_mkpath(fdpath, ಗಾತ್ರ(fdpath), fd))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ssz = readlink(fdpath, buf, ಗಾತ್ರ(buf));
	ಆದರೆ (ssz < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* readlink() does not terminate the result and it does not report
	 * truncation; a name we cannot read completely is unusable */
	ಆದರೆ ((size_t) ssz >= ಗಾತ್ರ(buf)) {
		errno = ENAMETOOLONG;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
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
	ಆದರೆ (*buf != '/' || ul_endswith(buf, ಮಾರ್ಗ_DELETED_SUFFIX)) {
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ strdup(buf);
}

ಪೂರ್ಣಾಂಕ ul_reopen(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ flags)
{
	ಅಕ್ಷರ *ಮಾರ್ಗ = ul_fd_get_ಮಾರ್ಗ(fd);
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ -errno;

	ret = open(ಮಾರ್ಗ, flags);
	ಬಿಡುಗಡೆ(ಮಾರ್ಗ);

	ಹಿಂತಿರುಗಿ ret;
}


/* This is a libc-independent version of basename(), which is necessary to
 * maintain functionality across different libc implementations. It was
 * inspired by the behavior and implementation of glibc.
 */
ಅಕ್ಷರ *ul_basename(ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *p;

	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *) ".";	/* ugly, static string */

	p = strrchr(ಮಾರ್ಗ, '/');
	ಆದರೆ (!p)
		ಹಿಂತಿರುಗಿ ಮಾರ್ಗ;		/* no '/', return original */

	ಆದರೆ (*(p + 1) != '\0')
		ಹಿಂತಿರುಗಿ p + 1;		/* begin of the name */

	ಆಗ (p > ಮಾರ್ಗ && *(p - 1) == '/')
		--p;			/* remove trailing '/' */

	ಆದರೆ (p > ಮಾರ್ಗ) {
		*p-- = '\0';
		ಆಗ (p > ಮಾರ್ಗ && *(p - 1) != '/')
			--p;		/* move to the beginning of the name */
	} ಇಲ್ಲದಿದ್ದರೆ ಆಗ (*(p + 1) != '\0')
		++p;

	ಹಿಂತಿರುಗಿ p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_OPENAT
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
FILE *fopen_at_no_link(ಪೂರ್ಣಾಂಕ dir, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
                             ಪೂರ್ಣಾಂಕ flags, mode_t perm, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಹುಲಕ)
{
	FILE *fp;
	ಪೂರ್ಣಾಂಕ fd;
	ರಚನೆ stat st;

	/* We temporarily clear the O_TRUNC bit because we do not want
	 * to accidentally truncate the target file if it is a hard link
	 * instead of a symbolic one, where the latter is what we are
	 * guarding against here. The test for the hard link is done below
	 * with fstat()...
	 */
	fd = openat(dir, filename, ((flags & ~O_TRUNC) | O_NOFOLLOW | O_CLOEXEC), perm);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (fstat(fd, &st)) {
		close(fd);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (st.st_nlink > 1) {
		close(fd);
		errno = EMLINK;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ ((flags & O_TRUNC) && ftruncate(fd, 0)) {
		close(fd);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	fp = fdopen(fd, ಬಹುಲಕ);
	ಆದರೆ (!fp)
		close(fd);
	ಹಿಂತಿರುಗಿ fp;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_OPENAT */

#ಆದರೆ defined(SYS_openat2)
ಪೂರ್ಣಾಂಕ ul_openat_resolve(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ flags,
		      mode_t ಬಹುಲಕ, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ resolve)
{
	ರಚನೆ open_how how = {
		.flags = (__u64) flags,
		.ಬಹುಲಕ = (__u64) ಬಹುಲಕ,
		.resolve = resolve,
	};

	ಹಿಂತಿರುಗಿ syscall(SYS_openat2, dirfd, ಮಾರ್ಗ, &how, ಗಾತ್ರ(how));
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ ul_openat_resolve(
		ಪೂರ್ಣಾಂಕ dirfd __attribute__((__unused__)),
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ __attribute__((__unused__)),
		ಪೂರ್ಣಾಂಕ flags __attribute__((__unused__)),
		mode_t ಬಹುಲಕ __attribute__((__unused__)),
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ resolve __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಹಿಂತಿರುಗಿ -1;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ open_no_symlinks_fallback(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ flags, mode_t ಬಹುಲಕ)
{
	ಅಕ್ಷರ *abspath = ಖಾಲಿ, *kpath = ಖಾಲಿ;
	ರಚನೆ stat st;
	ಪೂರ್ಣಾಂಕ fd = -1, errsv;

	/* we cannot verify a file we have to create first */
	ಆದರೆ (!ಮಾರ್ಗ || (flags & O_CREAT)) {
		errno = ENOSYS;
		ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (ul_is_relative_ಮಾರ್ಗ(ಮಾರ್ಗ)) {
		/* the kernel reports an absolute pathname; note that
		 * ul_absolute_path() only prepends the CWD as returned by
		 * getcwd(), it resolves nothing */
		abspath = ul_absolute_ಮಾರ್ಗ(ಮಾರ್ಗ);
		ಆದರೆ (!abspath)
			ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (flags & O_ಮಾರ್ಗ)
		fd = open(ಮಾರ್ಗ, flags);
	ಇಲ್ಲದಿದ್ದರೆ
		fd = open(ಮಾರ್ಗ, O_ಮಾರ್ಗ | O_CLOEXEC |
				(flags & (O_NOFOLLOW | O_DIRECTORY)));
	ಆದರೆ (fd < 0)
		ಹೋಗು fail;

	/* O_PATH|O_NOFOLLOW returns a FD to the symlink itself */
	ಆದರೆ (fstat(fd, &st) != 0)
		ಹೋಗು fail;
	ಆದರೆ (S_ISLNK(st.st_mode)) {
		errno = ELOOP;
		ಹೋಗು fail;
	}

	kpath = ul_fd_get_ಮಾರ್ಗ(fd);
	ಆದರೆ (!kpath) {
		errno = ENOSYS;		/* no /proc, no verification */
		ಹೋಗು fail;
	}

	/* streq_paths() ignores duplicate and trailing slashes, but "." and
	 * ".." in the requested path are refused as a symlink */
	ಆದರೆ (streq_paths(abspath ? abspath : ಮಾರ್ಗ, kpath) != 1) {
		errno = ELOOP;
		ಹೋಗು fail;
	}

	ಆದರೆ (!(flags & O_ಮಾರ್ಗ)) {
		ಅಕ್ಷರ fdpath[UL_FDPATH_BUFSIZ];
		ಪೂರ್ಣಾಂಕ nfd = -1;

		/* apply the caller's flags; the /proc link refers to the
		 * verified file, the path is not resolved for the second time */
		ಆದರೆ (ul_fd_mkpath(fdpath, ಗಾತ್ರ(fdpath), fd))
			nfd = open(fdpath, flags, ಬಹುಲಕ);
		ಆದರೆ (nfd < 0)
			ಹೋಗು fail;
		close(fd);
		fd = nfd;
	}

	ಬಿಡುಗಡೆ(abspath);
	ಬಿಡುಗಡೆ(kpath);
	ಹಿಂತಿರುಗಿ fd;
fail:
	errsv = errno;
	ಬಿಡುಗಡೆ(abspath);
	ಬಿಡುಗಡೆ(kpath);
	ಆದರೆ (fd >= 0)
		close(fd);
	errno = errsv;
	ಹಿಂತಿರುಗಿ -1;
}
#ಇಲ್ಲದಿದ್ದರೆ /* !__linux__ */
/* O_PATH and the /proc/self/fd/ names are Linux specific */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ open_no_symlinks_fallback(
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ __attribute__((__unused__)),
			ಪೂರ್ಣಾಂಕ flags __attribute__((__unused__)),
			mode_t ಬಹುಲಕ __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಹಿಂತಿರುಗಿ -1;
}
#ಆದರೆ_ಅಂತ್ಯ /* __linux__ */

/* Opens @path with the guarantee that no component of the path is a symbolic
 * link, otherwise it fails with ELOOP.
 */
ಪೂರ್ಣಾಂಕ ul_open_no_symlinks(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ flags, mode_t ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ fd = ul_openat_resolve(AT_FDCWD, ಮಾರ್ಗ, flags, ಬಹುಲಕ,
				   RESOLVE_NO_SYMLINKS);

	/* openat2() is Linux 5.6+ */
	ಆದರೆ (fd < 0 && errno == ENOSYS)
		fd = open_no_symlinks_fallback(ಮಾರ್ಗ, flags, ಬಹುಲಕ);

	ಹಿಂತಿರುಗಿ fd;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_UL_ಸುರಕ್ಷಿತ_STATX
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
ಪೂರ್ಣಾಂಕ ul_ಸುರಕ್ಷಿತ_statx(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ flags,
		  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ, ರಚನೆ statx *stx)
{
	ಪ್ರತಿಜ್ಞಾ(stx);

	memset(stx, 0, ಗಾತ್ರ(*stx));

	flags |= AT_STATX_DONT_SYNC | AT_NO_AUTOMOUNT;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ AT_ಖಾಲಿಗಳು_ಮಾರ್ಗ
	ಆದರೆ (!ಮಾರ್ಗ || !*ಮಾರ್ಗ)
		flags |= AT_ಖಾಲಿಗಳು_ಮಾರ್ಗ;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (statx(dirfd, ಮಾರ್ಗ ? ಮಾರ್ಗ : "", flags, ಮುಖವಾಡ, stx) != 0)
		ಹಿಂತಿರುಗಿ -errno;

	ಹಿಂತಿರುಗಿ 0;
}

/* Converts @stx to @st. statx(2) is not obliged to return everything the
 * caller asked for -- especially with AT_STATX_DONT_SYNC -- so copy only the
 * attributes advertised by stx_mask and leave the rest zeroed. Callers that
 * care have to check stx_mask themselves.
 */
ಶೂನ್ಯ ul_statx_ಗಮ್ಯ_stat(ಸ್ಥಿರಾಂಕ ರಚನೆ statx *stx, ರಚನೆ stat *st)
{
	memset(st, 0, ಗಾತ್ರ(*st));

	/* no bit in stx_mask, always returned by the kernel */
	st->st_dev     = makedev(stx->stx_dev_major, stx->stx_dev_minor);
	st->st_rdev    = makedev(stx->stx_rdev_major, stx->stx_rdev_minor);
	st->st_blksize = stx->stx_blksize;

	/* st_mode mixes two independently reported things */
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_TYPE)
		st->st_mode |= stx->stx_ಬಹುಲಕ & S_IFMT;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_ಬಹುಲಕ)
		st->st_mode |= stx->stx_ಬಹುಲಕ & ~S_IFMT;

	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_INO)
		st->st_ino = stx->stx_ino;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_NLINK)
		st->st_nlink = stx->stx_nlink;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_UID)
		st->st_uid = stx->stx_uid;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_GID)
		st->st_gid = stx->stx_gid;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_ಗಾತ್ರ)
		st->st_size = stx->stx_ಗಾತ್ರ;
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_ಖಂಡಗಳು)
		st->st_blocks = stx->stx_ಖಂಡಗಳು;

	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_ATIME) {
		st->st_atim.tv_sec  = stx->stx_atime.tv_sec;
		st->st_atim.tv_nsec = stx->stx_atime.tv_nsec;
	}
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_MTIME) {
		st->st_mtim.tv_sec  = stx->stx_mtime.tv_sec;
		st->st_mtim.tv_nsec = stx->stx_mtime.tv_nsec;
	}
	ಆದರೆ (stx->stx_ಮುಖವಾಡ & STATX_CTIME) {
		st->st_ctim.tv_sec  = stx->stx_ctime.tv_sec;
		st->st_ctim.tv_nsec = stx->stx_ctime.tv_nsec;
	}
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಪೂರ್ಣಾಂಕ ul_ಸುರಕ್ಷಿತ_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, ರಚನೆ stat *st,
		 ಪೂರ್ಣಾಂಕ nofollow __attribute__((__unused__)),
		 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ __attribute__((__unused__)),
		 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *retmask)
{
	ಪ್ರತಿಜ್ಞಾ(ಗುರಿ);
	ಪ್ರತಿಜ್ಞಾ(st);

	memset(st, 0, ಗಾತ್ರ(ರಚನೆ stat));
	ಆದರೆ (retmask)
		*retmask = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_UL_ಸುರಕ್ಷಿತ_STAT
	{
		ರಚನೆ statx stx = { 0 };
		ಪೂರ್ಣಾಂಕ rc;

		ಮುಖವಾಡ |= UL_STATX_ESSENTIAL;

		rc = ul_ಸುರಕ್ಷಿತ_statx(AT_FDCWD, ಗುರಿ,
				nofollow ? AT_SYMLINK_NOFOLLOW : 0, ಮುಖವಾಡ, &stx);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;

		ul_statx_ಗಮ್ಯ_stat(&stx, st);

		ಆದರೆ (retmask)
			*retmask = stx.stx_ಮುಖವಾಡ;

		ಆದರೆ ((stx.stx_ಮುಖವಾಡ & UL_STATX_ESSENTIAL) != UL_STATX_ESSENTIAL) {
			errno = EOPNOTSUPP;
			ಹಿಂತಿರುಗಿ -EOPNOTSUPP;
		}

		ಹಿಂತಿರುಗಿ 0;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	errno = ENOSYS;
	ಹಿಂತಿರುಗಿ -ENOSYS;
#ಆದರೆ_ಅಂತ್ಯ
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_FILEUTILS
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಆದರೆ (argc < 2)
		errx(EXIT_FAILURE, "Usage %s --{mkstemp,close-fds,copy-file,open-no-symlinks,safe-stat}",
				argv[0]);

	ಆದರೆ (strcmp(argv[1], "--mkstemp") == 0) {
		FILE *f;
		ಅಕ್ಷರ *tmpname = ಖಾಲಿ;

		f = xfmkstemp(&tmpname, ಖಾಲಿ, "test");
		unlink(tmpname);
		ಬಿಡುಗಡೆ(tmpname);
		fclose(f);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--close-fds") == 0) {
		ignore_ಫಲಿತಾಂಶ( dup(STDIN_FILENO) );
		ignore_ಫಲಿತಾಂಶ( dup(STDIN_FILENO) );
		ignore_ಫಲಿತಾಂಶ( dup(STDIN_FILENO) );

# ifdef HAVE_CLOSE_RANGE
		ಆದರೆ (close_range(STDERR_FILENO + 1, ~0U, 0) < 0)
# endif
			ul_close_all_fds(STDERR_FILENO + 1, ~0U);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--copy-file") == 0) {
		ಪೂರ್ಣಾಂಕ ret = ul_copy_file(STDIN_FILENO, STDOUT_FILENO);
		ಆದರೆ (ret == UL_COPY_READ_ERROR)
			err(EXIT_FAILURE, "read");
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret == UL_COPY_WRITE_ERROR)
			err(EXIT_FAILURE, "write");

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--open-no-symlinks") == 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		ಪೂರ್ಣಾಂಕ flags = O_ಮಾರ್ಗ | O_CLOEXEC;
#ಇಲ್ಲದಿದ್ದರೆ
		ಪೂರ್ಣಾಂಕ flags = O_RDONLY | O_CLOEXEC;
#ಆದರೆ_ಅಂತ್ಯ
		ಪೂರ್ಣಾಂಕ fallback = 0;
		ಅಕ್ಷರ *name;
		ಪೂರ್ಣಾಂಕ fd, i;

		ಆದರೆ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ಗಾಗಿ (i = 3; i < argc; i++) {
			/* the fallback is used on kernels without openat2()
			 * only, "--fallback" makes it testable anywhere */
			ಆದರೆ (strcmp(argv[i], "--fallback") == 0)
				fallback = 1;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[i], "--rdonly") == 0)
				flags = O_RDONLY | O_CLOEXEC;
		}

		ಆದರೆ (fallback)
			fd = open_no_symlinks_fallback(argv[2], flags, 0);
		ಇಲ್ಲದಿದ್ದರೆ
			fd = ul_open_no_symlinks(argv[2], flags, 0);
		ಆದರೆ (fd < 0)
			err(EXIT_FAILURE, "%s", argv[2]);

		name = ul_fd_get_ಮಾರ್ಗ(fd);
		ಬರೆ("%s\n", name);
		ಬಿಡುಗಡೆ(name);
		close(fd);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--safe-stat") == 0) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = 0, retmask = 0;
		ಪೂರ್ಣಾಂಕ nofollow = 0;
		ರಚನೆ stat st, ref;
		ಪೂರ್ಣಾಂಕ rc, i;

		ಆದರೆ (argc < 3)
			errx(EXIT_FAILURE, "no path specified");

		ಗಾಗಿ (i = 3; i < argc; i++) {
			ಆದರೆ (strcmp(argv[i], "--nofollow") == 0)
				nofollow = 1;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[i], "--basic") == 0)
				ಮುಖವಾಡ |= UL_STATX_BASIC;
		}

		ಬರೆ("%s:\n", argv[2]);

		rc = ul_ಸುರಕ್ಷಿತ_stat(argv[2], &st, nofollow, ಮುಖವಾಡ, &retmask);

		/* the function returns -errno and it sets errno too */
		ಬರೆ("      rc: %d\n", rc);
		ಬರೆ("   errno: %d\n", rc ? errno : 0);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;

		ಬರೆ("    type: %s\n", S_ISDIR(st.st_mode)  ? "dir" :
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
		ಆದರೆ (nofollow ? lstat(argv[2], &ref) : stat(argv[2], &ref))
			err(EXIT_FAILURE, "%s", argv[2]);

		ಬರೆ("  st_dev: %s\n", st.st_dev == ref.st_dev ? "OK" : "FAILED");
		ಬರೆ(" st_rdev: %s\n", st.st_rdev == ref.st_rdev ? "OK" : "FAILED");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_UL_ಸುರಕ್ಷಿತ_STAT
		{
			/* Report all the attributes we asked for, so that
			 * the output does not depend on how generous the
			 * kernel is; it may return more than requested.
			 *
			 * An attribute missing in @retmask is zero in @st by
			 * design, there is nothing to compare and it's not a
			 * failure.
			 */
			ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ emask = ಮುಖವಾಡ | UL_STATX_ESSENTIAL;

# define ಫಲಿತಾಂಶ_2(ಬಿಟ್, x)	(!(retmask & (ಬಿಟ್)) || (x) ? "OK" : "FAILED")

			ಆದರೆ (emask & STATX_TYPE)
				ಬರೆ("  S_IFMT: %s\n", ಫಲಿತಾಂಶ_2(STATX_TYPE,
					(st.st_mode & S_IFMT) == (ref.st_mode & S_IFMT)));
			ಆದರೆ (emask & STATX_ಬಹುಲಕ)
				ಬರೆ(" st_mode: %s\n", ಫಲಿತಾಂಶ_2(STATX_ಬಹುಲಕ,
					(st.st_mode & ~S_IFMT) == (ref.st_mode & ~S_IFMT)));
			ಆದರೆ (emask & STATX_INO)
				ಬರೆ("  st_ino: %s\n",
					ಫಲಿತಾಂಶ_2(STATX_INO, st.st_ino == ref.st_ino));
			ಆದರೆ (emask & STATX_NLINK)
				ಬರೆ("st_nlink: %s\n",
					ಫಲಿತಾಂಶ_2(STATX_NLINK, st.st_nlink == ref.st_nlink));
			ಆದರೆ (emask & STATX_UID)
				ಬರೆ("  st_uid: %s\n",
					ಫಲಿತಾಂಶ_2(STATX_UID, st.st_uid == ref.st_uid));
			ಆದರೆ (emask & STATX_GID)
				ಬರೆ("  st_gid: %s\n",
					ಫಲಿತಾಂಶ_2(STATX_GID, st.st_gid == ref.st_gid));
			ಆದರೆ (emask & STATX_ಗಾತ್ರ)
				ಬರೆ(" st_size: %s\n",
					ಫಲಿತಾಂಶ_2(STATX_ಗಾತ್ರ, st.st_size == ref.st_size));
# undef ಫಲಿತಾಂಶ_2
		}
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ
