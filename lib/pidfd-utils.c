/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/vfs.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "pidfd-utils.h"
#અન્તર્ભાવ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
પૂર્ણાંક pfd_is_pidfs(પૂર્ણાંક pidfd)
{
	સંરચના statfs stfs;
	પૂર્ણાંક rc;

	rc = fstatfs(pidfd, &stfs);
	જો (rc < 0)
		પાછા 0;

	પાછા F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(પૂર્ણાંક pidfd)
{
	સંરચના statx stx;
	પૂર્ણાંક rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	જો (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	પાછા stx.stx_ino;
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
પૂર્ણાંક ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#નહીંતર
પૂર્ણાંક ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	પૂર્ણાંક pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	જો (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	પાછા pfd;
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
પૂર્ણાંક ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#નહીંતર
પૂર્ણાંક ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	પૂર્ણાંક pfd;

	pfd = pidfd_open(pid, 0);
	જો (pfd < 0)
		પાછા -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	જો (!pfd_is_pidfs(pfd)) {
		close(pfd);
		પાછા -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	જો (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		જો (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			પાછા -(errno = ESRCH);
		}
	}
#endif
	પાછા pfd;
}
