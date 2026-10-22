/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/vfs.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "pidfd-utils.h"
#अन्तर्भाव "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
पूर्णाङ्क pfd_is_pidfs(पूर्णाङ्क pidfd)
{
	संरचना statfs stfs;
	पूर्णाङ्क rc;

	rc = fstatfs(pidfd, &stfs);
	यदि (rc < 0)
		प्रत्यागम 0;

	प्रत्यागम F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#ifdef USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(पूर्णाङ्क pidfd)
{
	संरचना statx stx;
	पूर्णाङ्क rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	यदि (rc < 0) {
		बंद(pidfd);
		err(निकास_त्रुटि, _("failed to statx() pidfd"));
	}
	प्रत्यागम stx.stx_ino;
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
पूर्णाङ्क ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#अन्यथा
पूर्णाङ्क ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	पूर्णाङ्क pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	यदि (pfd < 0)
		err(निकास_त्रुटि, _("failed to obtain a valid file descriptor for PID %d"), pid);

	प्रत्यागम pfd;
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
पूर्णाङ्क ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#अन्यथा
पूर्णाङ्क ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#endif
{
	पूर्णाङ्क pfd;

	pfd = pidfd_open(pid, 0);
	यदि (pfd < 0)
		प्रत्यागम -त्रुटि_सं;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	यदि (!pfd_is_pidfs(pfd)) {
		बंद(pfd);
		प्रत्यागम -(त्रुटि_सं = ENOTSUP);
	}

#ifdef USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	यदि (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		यदि (real_pidfd_ino != pidfd_ino) {
			बंद(pfd);
			प्रत्यागम -(त्रुटि_सं = ESRCH);
		}
	}
#endif
	प्रत्यागम pfd;
}
