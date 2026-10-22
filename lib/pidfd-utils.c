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
ಪೂರ್ಣಾಂಕ pfd_is_pidfs(ಪೂರ್ಣಾಂಕ pidfd)
{
	ರಚನೆ statfs stfs;
	ಪೂರ್ಣಾಂಕ rc;

	rc = fstatfs(pidfd, &stfs);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(ಪೂರ್ಣಾಂಕ pidfd)
{
	ರಚನೆ statx stx;
	ಪೂರ್ಣಾಂಕ rc;

	rc = statx(pidfd, "", AT_ಖಾಲಿಗಳು_ಮಾರ್ಗ, STATX_INO, &stx);
	ಆದರೆ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	ಹಿಂತಿರುಗಿ stx.stx_ino;
}
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_PIDFD_INO_SUPPORT
ಪೂರ್ಣಾಂಕ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#ಆದರೆ_ಅಂತ್ಯ
{
	ಪೂರ್ಣಾಂಕ pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ಆದರೆ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	ಹಿಂತಿರುಗಿ pfd;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_PIDFD_INO_SUPPORT
ಪೂರ್ಣಾಂಕ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#ಆದರೆ_ಅಂತ್ಯ
{
	ಪೂರ್ಣಾಂಕ pfd;

	pfd = pidfd_open(pid, 0);
	ಆದರೆ (pfd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ಆದರೆ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		ಹಿಂತಿರುಗಿ -(errno = ENOTSUP);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ಆದರೆ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ಆದರೆ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			ಹಿಂತಿರುಗಿ -(errno = ESRCH);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ pfd;
}
