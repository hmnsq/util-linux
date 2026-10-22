/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/vfs.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "pidfd-utils.h"
#ಅನ್ತರ್ಭಾವ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
ಪೂರ್ಣಾಙ್ಕ pfd_is_pidfs(ಪೂರ್ಣಾಙ್ಕ pidfd)
{
	ಸಂರಚನಾ statfs stfs;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = fstatfs(pidfd, &stfs);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(ಪೂರ್ಣಾಙ್ಕ pidfd)
{
	ಸಂರಚನಾ statx stx;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	ಯದಿ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ಪ್ರತ್ಯಾಗಮ stx.stx_ino;
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
ಪೂರ್ಣಾಙ್ಕ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	ಪೂರ್ಣಾಙ್ಕ pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ಯದಿ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ಪ್ರತ್ಯಾಗಮ pfd;
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
ಪೂರ್ಣಾಙ್ಕ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	ಪೂರ್ಣಾಙ್ಕ pfd;

	pfd = pidfd_open(pid, 0);
	ಯದಿ (pfd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ಯದಿ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ಪ್ರತ್ಯಾಗಮ -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ಯದಿ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ಯದಿ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ಪ್ರತ್ಯಾಗಮ -(errno = ESRCH);
		}
	}
#endif
	ಪ್ರತ್ಯಾಗಮ pfd;
}
