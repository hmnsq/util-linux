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
	ܐܢ (rc < 0)
		ܦܢܐ 0;

	ܦܢܐ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(صحيح pidfd)
{
	بنية statx stx;
	صحيح rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	ܐܢ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ܦܢܐ stx.stx_ino;
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
صحيح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#ܐܚܪܢܐ
صحيح ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	صحيح pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ܐܢ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ܦܢܐ pfd;
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
صحيح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#ܐܚܪܢܐ
صحيح ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	صحيح pfd;

	pfd = pidfd_open(pid, 0);
	ܐܢ (pfd < 0)
		ܦܢܐ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ܐܢ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ܦܢܐ -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ܐܢ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ܐܢ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ܦܢܐ -(errno = ESRCH);
		}
	}
#endif
	ܦܢܐ pfd;
}
