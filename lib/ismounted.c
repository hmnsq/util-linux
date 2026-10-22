/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#include "pathnames.h"
#include "strutils.h"
#include "ismounted.h"
#include "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
static ціле check_mntent_file(const char *mtab_file, const char *file,
				   ціле *mount_flags, char *mtpt, ціле mtlen)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	ціле		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ціле		fd;

	*mount_flags = 0;
	якщо ((f = setmntent (mtab_file, "r")) == NULL)
		повернути errno;

	якщо (stat(file, &st_buf) == 0) {
		якщо (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} інакше {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	поки ((mnt = getmntent (f)) != NULL) {
		якщо (mnt->mnt_fsname[0] != '/')
			continue;
		якщо (strcmp(file, mnt->mnt_fsname) == 0)
			break;
		якщо (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		якщо (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			якщо (file_rdev && file_rdev == st_buf.st_rdev)
				break;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			якщо (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				break;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} інакше {
			якщо (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				break;
		}
	}

	якщо (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		якщо (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			якщо (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			goto is_root;
		}
#endif	/* __GNU__ */
		goto errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	якщо (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		якщо (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	якщо (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		goto errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	якщо (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	якщо (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	якщо (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		якщо (fd < 0) {
			якщо (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} інакше
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	повернути retval;
}

static ціле check_mntent(const char *file, ціле *mount_flags,
			      char *mtpt, ціле mtlen)
{
	ціле	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	якщо (retval == 0)
		повернути 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	якщо (retval == 0 && (*mount_flags != 0))
		повернути 0;
	якщо (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		повернути retval;
	}
#endif /* __linux__ */
#якщо defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	повернути retval;
#інакше
	*mount_flags = 0;
	повернути 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#інакше
#якщо defined(HAVE_GETMNTINFO)

static ціле check_getmntinfo(const char *file, ціле *mount_flags,
				  char *mtpt, ціле mtlen)
{
	struct statfs *mp;
	ціле    len, n;
	const  char   *s1;
	char	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	якщо (n == 0)
		повернути errno;

	len = sizeof(_PATH_DEV) - 1;
	s1 = file;
	якщо (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	поки (--n >= 0) {
		s2 = mp->f_mntfromname;
		якщо (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		якщо (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			break;
		}
		++mp;
	}
	якщо (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	повернути 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static ціле is_swap_device(const char *file)
{
	FILE		*f;
	char		buf[1024], *cp;
	dev_t		file_dev;
	struct stat	st_buf;
	ціле		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	якщо ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	якщо (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		повернути 0;
	/* Skip the first line */
	якщо (!fgets(buf, sizeof(buf), f))
		goto leave;
	якщо (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	поки (fgets(buf, sizeof(buf), f)) {
valid_first_line:
		якщо ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		якщо ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		якщо (strcmp(buf, file) == 0) {
			ret++;
			break;
		}
#ifndef __GNU__
		якщо (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			break;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	повернути ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#ifdef __TURBOC__
 #pragma argsused
#endif
ціле check_mount_point(const char *device, ціле *mount_flags,
				  char *mtpt, ціле mtlen)
{
	ціле	retval = 0;

	якщо (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		якщо (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} інакше {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#інакше
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#інакше
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	якщо (retval)
		повернути retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		struct stat st_buf;
		ціле fd;
		якщо ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			повернути 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		якщо (fd < 0) {
			якщо (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} інакше
			close(fd);
	}
#endif

	повернути 0;
}

ціле is_mounted(const char *file)
{
	ціле	retval;
	ціле	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	якщо (retval)
		повернути 0;
	повернути mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
ціле main(ціле argc, char **argv)
{
	ціле flags = 0;
	char devname[PATH_MAX];

	якщо (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		повернути EXIT_FAILURE;
	}

	якщо (check_mount_point(argv[1], &flags, devname, sizeof(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		якщо (flags & MF_SWAP)
			printf("used swap device\n");
		інакше
			printf("mounted on %s\n", devname);
		повернути EXIT_SUCCESS;
	}

	printf("not mounted\n");
	повернути EXIT_FAILURE;
}
#endif /* DEBUG */
