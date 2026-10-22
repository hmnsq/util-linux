/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/vfs.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "pidfd-utils.h"
#അന്തര്ഭാവ "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
പൂർണ്ണസംഖ്യ pfd_is_pidfs(പൂർണ്ണസംഖ്യ pidfd)
{
	ഘടന statfs stfs;
	പൂർണ്ണസംഖ്യ rc;

	rc = fstatfs(pidfd, &stfs);
	എങ്കിൽ (rc < 0)
		തിരികെ 0;

	തിരികെ F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(പൂർണ്ണസംഖ്യ pidfd)
{
	ഘടന statx stx;
	പൂർണ്ണസംഖ്യ rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	എങ്കിൽ (rc < 0) {
		close(pidfd);
		err(EXIT_FAILURE, _("failed to statx() pidfd"));
	}
	തിരികെ stx.stx_ino;
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
പൂർണ്ണസംഖ്യ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	പൂർണ്ണസംഖ്യ pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	എങ്കിൽ (pfd < 0)
		err(EXIT_FAILURE, _("failed to obtain a valid file descriptor for PID %d"), pid);

	തിരികെ pfd;
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
പൂർണ്ണസംഖ്യ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	പൂർണ്ണസംഖ്യ pfd;

	pfd = pidfd_open(pid, 0);
	എങ്കിൽ (pfd < 0)
		തിരികെ -errno;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	എങ്കിൽ (!pfd_is_pidfs(pfd)) {
		close(pfd);
		തിരികെ -(errno = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	എങ്കിൽ (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		എങ്കിൽ (real_pidfd_ino != pidfd_ino) {
			close(pfd);
			തിരികെ -(errno = ESRCH);
		}
	}
#endif
	തിരികെ pfd;
}
