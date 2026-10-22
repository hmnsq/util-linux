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
цяло pfd_is_pidfs(цяло pidfd)
{
	struct statfs stfs;
	цяло rc;

	rc = fstatfs(pidfd, &stfs);
	ако (rc < 0)
		върни 0;

	върни F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(цяло pidfd)
{
	struct statx stx;
	цяло rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	ако (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	върни stx.stx_ino;
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
цяло ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#иначе
цяло ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	цяло pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ако (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	върни pfd;
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
цяло ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#иначе
цяло ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	цяло pfd;

	pfd = pidfd_open(pid, 0);
	ако (pfd < 0)
		върни -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ако (!pfd_is_pidfs(pfd)) {
		close(pfd);
		върни -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ако (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ако (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			върни -(errno = ESRCH);
		}
	}
#endif
	върни pfd;
}
