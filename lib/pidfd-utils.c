/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/vfs.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "pidfd-utils.h"
#அந்தர்பாவ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
முழு pfd_is_pidfs(முழு pidfd)
{
	கட்டமைப்பு statfs stfs;
	முழு rc;

	rc = fstatfs(pidfd, &stfs);
	எனில் (rc < 0)
		திரும்பு 0;

	திரும்பு F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(முழு pidfd)
{
	கட்டமைப்பு statx stx;
	முழு rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	எனில் (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	திரும்பு stx.stx_ino;
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
முழு ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#இல்லையெனில்
முழு ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	முழு pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	எனில் (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	திரும்பு pfd;
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
முழு ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#இல்லையெனில்
முழு ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	முழு pfd;

	pfd = pidfd_open(pid, 0);
	எனில் (pfd < 0)
		திரும்பு -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	எனில் (!pfd_is_pidfs(pfd)) {
		close(pfd);
		திரும்பு -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	எனில் (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		எனில் (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			திரும்பு -(errno = ESRCH);
		}
	}
#endif
	திரும்பு pfd;
}
