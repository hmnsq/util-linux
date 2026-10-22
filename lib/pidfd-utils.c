/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/vfs.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "pidfd-utils.h"
#ଅନ୍ତର୍ଭାଵ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା pfd_is_pidfs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା pidfd)
{
	ସଂରଚନା statfs stfs;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = fstatfs(pidfd, &stfs);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(ପୂର୍ଣ୍ଣସଂଖ୍ୟା pidfd)
{
	ସଂରଚନା statx stx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	ଯଦି (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ଫେରନ୍ତୁ stx.stx_ino;
}
#endif

/*
 * ul_get_valid_pidfd_or_err() - Return a valid file descriptor for a PID
 *                               or exit the process with an error message.
 *
 * @pid:     PID number for which to get a file descriptor
 * @pfd_ino: A pidfd inode number that is expected to be the
 *           same as for the new file descriptor.
 *
 * Pass @pfd_ino as 0, if the pidfd should not be validated.
 *
 * Return: On success, a file descriptor is returned.
 *         On failure, err() is called with an error message
 *         and the processes is terminated.
 *
 */
#ifdef USE_PIDFD_INO_SUPPORT
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ଯଦି (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ଫେରନ୍ତୁ pfd;
}

/*
 * ul_get_valid_pidfd() - Return a valid file descriptor for a PID
 *
 * @pid:     PID number for which to get a file descriptor
 * @pfd_ino: A pidfd inode number that is expected to be the
 *           same as for the new file descriptor.
 *
 * Pass @pfd_ino as 0, if the pidfd should not be validated.
 *
 * Return: On success, a file descriptor is returned.
 *         On failure, a negative errno is returned and errno
 *         is set accordingly.
 *
 */
#ifdef USE_PIDFD_INO_SUPPORT
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pfd;

	pfd = pidfd_open(pid, 0);
	ଯଦି (pfd < 0)
		ଫେରନ୍ତୁ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ଯଦି (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ଫେରନ୍ତୁ -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ଯଦି (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ଯଦି (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ଫେରନ୍ତୁ -(errno = ESRCH);
		}
	}
#endif
	ଫେରନ୍ତୁ pfd;
}
