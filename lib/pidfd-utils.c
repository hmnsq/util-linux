/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/vfs.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "pidfd-utils.h"
#అన్తర్భావ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
పూర్ణాంకం pfd_is_pidfs(పూర్ణాంకం pidfd)
{
	నిర్మాణం statfs stfs;
	పూర్ణాంకం rc;

	rc = fstatfs(pidfd, &stfs);
	ఒకవేళ (rc < 0)
		తిరిగి 0;

	తిరిగి F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#నిర్వచితమైతే USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(పూర్ణాంకం pidfd)
{
	నిర్మాణం statx stx;
	పూర్ణాంకం rc;

	rc = statx(pidfd, "", AT_ఖాళీలు_మార్గం, STATX_INO, &stx);
	ఒకవేళ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	తిరిగి stx.stx_ino;
}
#అయితే_ముగింపు

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
#నిర్వచితమైతే USE_PIDFD_INO_SUPPORT
పూర్ణాంకం ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#లేకపోతే
పూర్ణాంకం ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#అయితే_ముగింపు
{
	పూర్ణాంకం pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	ఒకవేళ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	తిరిగి pfd;
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
#నిర్వచితమైతే USE_PIDFD_INO_SUPPORT
పూర్ణాంకం ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#లేకపోతే
పూర్ణాంకం ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#అయితే_ముగింపు
{
	పూర్ణాంకం pfd;

	pfd = pidfd_open(pid, 0);
	ఒకవేళ (pfd < 0)
		తిరిగి -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	ఒకవేళ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		తిరిగి -(errno = ENOTSUP);
	}

#నిర్వచితమైతే USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	ఒకవేళ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		ఒకవేళ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			తిరిగి -(errno = ESRCH);
		}
	}
#అయితే_ముగింపు
	తిరిగి pfd;
}
