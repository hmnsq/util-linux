/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Authors: Christian Goeschel Ndjomouo <cgoesc2@wgu.edu> [2025]
 */
#समावेश <stdlib.h>
#समावेश <sys/stat.h>
#समावेश <sys/vfs.h>
#समावेश <sys/types.h>
#समावेश <errno.h>

#समावेश "c.h"
#समावेश "nls.h"
#समावेश "strutils.h"
#समावेश "pidfd-utils.h"
#समावेश "statfs_magic.h"

/*
 * Returns 1, if the pidfd has the pidfs file system type, otherwise 0.
 */
पूर्णांक pfd_is_pidfs(पूर्णांक pidfd)
{
	काष्ठा statfs stfs;
	पूर्णांक rc;

	rc = fstatfs(pidfd, &stfs);
	अगर (rc < 0)
		वापस 0;

	वापस F_TYPE_EQUAL(stfs.f_type, STATFS_PIDFS_MAGIC);
}

#अगर_घोषित USE_PIDFD_INO_SUPPORT
uint64_t pidfd_get_inode(पूर्णांक pidfd)
{
	काष्ठा statx stx;
	पूर्णांक rc;

	rc = statx(pidfd, "", AT_EMPTY_PATH, STATX_INO, &stx);
	अगर (rc < 0) {
		बंद(pidfd);
		err(निकास_त्रुटि, _("failed to statx() pidfd"));
	}
	वापस stx.stx_ino;
}
#पूर्ण_अगर

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
#अगर_घोषित USE_PIDFD_INO_SUPPORT
पूर्णांक ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino)
#अन्यथा
पूर्णांक ul_get_valid_pidfd_or_err(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#पूर्ण_अगर
{
	पूर्णांक pfd;

	pfd = ul_get_valid_pidfd(pid, pidfd_ino);
	अगर (pfd < 0)
		err(निकास_त्रुटि, _("failed to obtain a valid file descriptor for PID %d"), pid);

	वापस pfd;
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
#अगर_घोषित USE_PIDFD_INO_SUPPORT
पूर्णांक ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino)
#अन्यथा
पूर्णांक ul_get_valid_pidfd(pid_t pid, uint64_t pidfd_ino __attribute__((__unused__)))
#पूर्ण_अगर
{
	पूर्णांक pfd;

	pfd = pidfd_open(pid, 0);
	अगर (pfd < 0)
		वापस -त्रुटि_सं;

	/* the file descriptor has to have the pidfs file system type
	 * otherwise the inode assigned to it will not be useful.
	 */
	अगर (!pfd_is_pidfs(pfd)) {
		बंद(pfd);
		वापस -(त्रुटि_सं = ENOTSUP);
	}

#अगर_घोषित USE_PIDFD_INO_SUPPORT
	uint64_t real_pidfd_ino;
	अगर (pidfd_ino) {
		real_pidfd_ino = pidfd_get_inode(pfd);
		अगर (real_pidfd_ino != pidfd_ino) {
			बंद(pfd);
			वापस -(त्रुटि_सं = ESRCH);
		}
	}
#पूर्ण_अगर
	वापस pfd;
}
