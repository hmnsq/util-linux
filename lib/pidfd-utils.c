/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#شامل <stdlib.h>
#شامل <sys/stat.h>
#شامل <sys/vfs.h>
#شامل <sys/types.h>
#شامل <errno.h>

#شامل "c.h"
#شامل "nls.h"
#شامل "strutils.h"
#شامل "pidfd-utils.h"
#شامل "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
صحیح pfd_is_pidfs(صحیح pidfd)
{
	ساختار statfs stfs;
	صحیح rc;

	rc = fstatfs(pidfd, &stfs);
	اگر (rc < 0)
		بازگشت 0;

	بازگشت F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(صحیح pidfd)
{
	ساختار statx stx;
	صحیح rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	اگر (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	بازگشت stx.stx_ino;
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
صحیح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#وگرنه
صحیح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	صحیح pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	اگر (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	بازگشت pfd;
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
صحیح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#وگرنه
صحیح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	صحیح pfd;

	pfd = pidfd_open(pid, 0);
	اگر (pfd < 0)
		بازگشت -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	اگر (!pfd_is_pidfs(pfd)) {
		close(pfd);
		بازگشت -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	اگر (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		اگر (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			بازگشت -(errno = ESRCH);
		}
	}
#endif
	بازگشت pfd;
}
