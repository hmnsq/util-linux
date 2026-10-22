/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/vfs.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <errno.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "pidfd-utils.h"
#অন্তর্ভাৱ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
পূর্ণাঙ্ক pfd_is_pidfs(পূর্ণাঙ্ক pidfd)
{
	সংরচনা statfs stfs;
	পূর্ণাঙ্ক rc;

	rc = fstatfs(pidfd, &stfs);
	যদি (rc < 0)
		ঘূৰাই 0;

	ঘূৰাই F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(পূর্ণাঙ্ক pidfd)
{
	সংরচনা statx stx;
	পূর্ণাঙ্ক rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	যদি (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ঘূৰাই stx.stx_ino;
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
পূর্ণাঙ্ক ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#অন্যথা
পূর্ণাঙ্ক ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	পূর্ণাঙ্ক pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	যদি (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ঘূৰাই pfd;
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
পূর্ণাঙ্ক ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#অন্যথা
পূর্ণাঙ্ক ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	পূর্ণাঙ্ক pfd;

	pfd = pidfd_open(pid, 0);
	যদি (pfd < 0)
		ঘূৰাই -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	যদি (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ঘূৰাই -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	যদি (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		যদি (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ঘূৰাই -(errno = ESRCH);
		}
	}
#endif
	ঘূৰাই pfd;
}
