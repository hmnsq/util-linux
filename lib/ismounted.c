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
static 정수 check_mntent_file(const char *mtab_file, const char *file,
				   정수 *mount_flags, char *mtpt, 정수 mtlen)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	정수		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	정수		fd;

	*mount_flags = 0;
	만약 ((f = setmntent (mtab_file, "r")) == NULL)
		반환 errno;

	만약 (stat(file, &st_buf) == 0) {
		만약 (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} 아니면 {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	동안 ((mnt = getmntent (f)) != NULL) {
		만약 (mnt->mnt_fsname[0] != '/')
			continue;
		만약 (strcmp(file, mnt->mnt_fsname) == 0)
			break;
		만약 (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		만약 (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			만약 (file_rdev && file_rdev == st_buf.st_rdev)
				break;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			만약 (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				break;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} 아니면 {
			만약 (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				break;
		}
	}

	만약 (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		만약 (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			만약 (mtpt)
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
	만약 (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		만약 (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	만약 (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	만약 (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	만약 (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	만약 (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		만약 (fd < 0) {
			만약 (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} 아니면
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	반환 retval;
}

static 정수 check_mntent(const char *file, 정수 *mount_flags,
			      char *mtpt, 정수 mtlen)
{
	정수	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	만약 (retval == 0)
		반환 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	만약 (retval == 0 && (*mount_flags != 0))
		반환 0;
	만약 (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		반환 retval;
	}
#endif /* __linux__ */
#만약 defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	반환 retval;
#아니면
	*mount_flags = 0;
	반환 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#아니면
#만약 defined(HAVE_GETMNTINFO)

static 정수 check_getmntinfo(const char *file, 정수 *mount_flags,
				  char *mtpt, 정수 mtlen)
{
	struct statfs *mp;
	정수    len, n;
	const  char   *s1;
	char	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	만약 (n == 0)
		반환 errno;

	len = sizeof(_PATH_DEV) - 1;
	s1 = file;
	만약 (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	동안 (--n >= 0) {
		s2 = mp->f_mntfromname;
		만약 (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		만약 (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			break;
		}
		++mp;
	}
	만약 (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	반환 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static 정수 is_swap_device(const char *file)
{
	FILE		*f;
	char		buf[1024], *cp;
	dev_t		file_dev;
	struct stat	st_buf;
	정수		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	만약 ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	만약 (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		반환 0;
	/* Skip the first line */
	만약 (!fgets(buf, sizeof(buf), f))
		goto leave;
	만약 (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	동안 (fgets(buf, sizeof(buf), f)) {
valid_first_line:
		만약 ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		만약 ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		만약 (strcmp(buf, file) == 0) {
			ret++;
			break;
		}
#ifndef __GNU__
		만약 (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			break;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	반환 ret;
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
정수 check_mount_point(const char *device, 정수 *mount_flags,
				  char *mtpt, 정수 mtlen)
{
	정수	retval = 0;

	만약 (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		만약 (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} 아니면 {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#아니면
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#아니면
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	만약 (retval)
		반환 retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		struct stat st_buf;
		정수 fd;
		만약 ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			반환 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		만약 (fd < 0) {
			만약 (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} 아니면
			close(fd);
	}
#endif

	반환 0;
}

정수 is_mounted(const char *file)
{
	정수	retval;
	정수	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	만약 (retval)
		반환 0;
	반환 mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
정수 main(정수 argc, char **argv)
{
	정수 flags = 0;
	char devname[PATH_MAX];

	만약 (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		반환 EXIT_FAILURE;
	}

	만약 (check_mount_point(argv[1], &flags, devname, sizeof(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		만약 (flags & MF_SWAP)
			printf("used swap device\n");
		아니면
			printf("mounted on %s\n", devname);
		반환 EXIT_SUCCESS;
	}

	printf("not mounted\n");
	반환 EXIT_FAILURE;
}
#endif /* DEBUG */
