/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <errno.h>

#include "c.h"
#include "nls.h"
#include "strutils.h"
#include "pidfd-utils.h"
#include "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
정수 pfd_is_pidfs(정수 pidfd)
{
	struct statfs stfs;
	정수 rc;

	rc = fstatfs(pidfd, &stfs);
	만약 (rc < 0)
		반환 0;

	반환 F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(정수 pidfd)
{
	struct statx stx;
	정수 rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	만약 (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	반환 stx.stx_ino;
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
정수 ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#아니면
정수 ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	정수 pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	만약 (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	반환 pfd;
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
정수 ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#아니면
정수 ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	정수 pfd;

	pfd = pidfd_open(pid, 0);
	만약 (pfd < 0)
		반환 -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	만약 (!pfd_is_pidfs(pfd)) {
		close(pfd);
		반환 -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	만약 (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		만약 (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			반환 -(errno = ESRCH);
		}
	}
#endif
	반환 pfd;
}
