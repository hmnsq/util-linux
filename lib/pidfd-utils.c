/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/vfs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pidfd-utils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
ਪੂਰਨ pfd_is_pidfs(ਪੂਰਨ pidfd)
{
	ਬਣਤਰ statfs stfs;
	ਪੂਰਨ rc;

	rc = fstatfs(pidfd, &stfs);
	ਜੇ (rc < 0)
		ਵਾਪਸ 0;

	ਵਾਪਸ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(ਪੂਰਨ pidfd)
{
	ਬਣਤਰ statx stx;
	ਪੂਰਨ rc;

	rc = statx(pidfd, "", AT_ਖਾਲੀ_ਥਾਵਾਂ_ਰਸਤਾ, STATX_INO, &stx);
	ਜੇ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ਵਾਪਸ stx.stx_ino;
}
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_PIDFD_INO_SUPPORT
ਪੂਰਨ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#ਜੇ_ਅੰਤ
{
	ਪੂਰਨ pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ਜੇ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ਵਾਪਸ pfd;
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_PIDFD_INO_SUPPORT
ਪੂਰਨ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#ਜੇ_ਅੰਤ
{
	ਪੂਰਨ pfd;

	pfd = pidfd_open(pid, 0);
	ਜੇ (pfd < 0)
		ਵਾਪਸ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ਜੇ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ਵਾਪਸ -(errno = ENOTSUP);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ਜੇ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ਜੇ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ਵਾਪਸ -(errno = ESRCH);
		}
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ pfd;
}
