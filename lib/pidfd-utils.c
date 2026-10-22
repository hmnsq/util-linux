/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ضمن <stdlib.h>
#ضمن <sys/stat.h>
#ضمن <sys/vfs.h>
#ضمن <sys/types.h>
#ضمن <errno.h>

#ضمن "c.h"
#ضمن "nls.h"
#ضمن "strutils.h"
#ضمن "pidfd-utils.h"
#ضمن "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
صحيح pfd_is_pidfs(صحيح pidfd)
{
	بنية statfs stfs;
	صحيح rc;

	rc = fstatfs(pidfd, &stfs);
	إذا (rc < 0)
		أرجع 0;

	أرجع F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#اگر_تعریف_شده USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(صحيح pidfd)
{
	بنية statx stx;
	صحيح rc;

	rc = statx(pidfd, "", AT_خالی_مسیر, STATX_INO, &stx);
	إذا (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	أرجع stx.stx_ino;
}
#پایان_اگر

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
#اگر_تعریف_شده USE_PIDFD_INO_SUPPORT
صحيح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#وإلا
صحيح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#پایان_اگر
{
	صحيح pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	إذا (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	أرجع pfd;
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
#اگر_تعریف_شده USE_PIDFD_INO_SUPPORT
صحيح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#وإلا
صحيح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#پایان_اگر
{
	صحيح pfd;

	pfd = pidfd_open(pid, 0);
	إذا (pfd < 0)
		أرجع -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	إذا (!pfd_is_pidfs(pfd)) {
		close(pfd);
		أرجع -(errno = ENOTSUP);
	}

#اگر_تعریف_شده USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	إذا (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		إذا (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			أرجع -(errno = ESRCH);
		}
	}
#پایان_اگر
	أرجع pfd;
}
