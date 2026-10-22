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
static số_nguyên check_mntent_file(const char *mtab_file, const char *file,
				   số_nguyên *mount_flags, char *mtpt, số_nguyên mtlen)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	số_nguyên		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	số_nguyên		fd;

	*mount_flags = 0;
	nếu ((f = setmntent (mtab_file, "r")) == NULL)
		trả_về errno;

	nếu (stat(file, &st_buf) == 0) {
		nếu (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} không_thì {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	trong_khi ((mnt = getmntent (f)) != NULL) {
		nếu (mnt->mnt_fsname[0] != '/')
			continue;
		nếu (strcmp(file, mnt->mnt_fsname) == 0)
			break;
		nếu (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		nếu (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			nếu (file_rdev && file_rdev == st_buf.st_rdev)
				break;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			nếu (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				break;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} không_thì {
			nếu (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				break;
		}
	}

	nếu (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		nếu (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			nếu (mtpt)
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
	nếu (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		nếu (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	nếu (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	nếu (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	nếu (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	nếu (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		nếu (fd < 0) {
			nếu (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} không_thì
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	trả_về retval;
}

static số_nguyên check_mntent(const char *file, số_nguyên *mount_flags,
			      char *mtpt, số_nguyên mtlen)
{
	số_nguyên	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	nếu (retval == 0)
		trả_về 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	nếu (retval == 0 && (*mount_flags != 0))
		trả_về 0;
	nếu (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		trả_về retval;
	}
#endif /* __linux__ */
#nếu defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	trả_về retval;
#không_thì
	*mount_flags = 0;
	trả_về 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#không_thì
#nếu defined(HAVE_GETMNTINFO)

static số_nguyên check_getmntinfo(const char *file, số_nguyên *mount_flags,
				  char *mtpt, số_nguyên mtlen)
{
	struct statfs *mp;
	số_nguyên    len, n;
	const  char   *s1;
	char	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	nếu (n == 0)
		trả_về errno;

	len = sizeof(_PATH_DEV) - 1;
	s1 = file;
	nếu (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	trong_khi (--n >= 0) {
		s2 = mp->f_mntfromname;
		nếu (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		nếu (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			break;
		}
		++mp;
	}
	nếu (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	trả_về 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static số_nguyên is_swap_device(const char *file)
{
	FILE		*f;
	char		buf[1024], *cp;
	dev_t		file_dev;
	struct stat	st_buf;
	số_nguyên		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	nếu ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	nếu (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		trả_về 0;
	/* Skip the first line */
	nếu (!fgets(buf, sizeof(buf), f))
		goto leave;
	nếu (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	trong_khi (fgets(buf, sizeof(buf), f)) {
valid_first_line:
		nếu ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		nếu ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		nếu (strcmp(buf, file) == 0) {
			ret++;
			break;
		}
#ifndef __GNU__
		nếu (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			break;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	trả_về ret;
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
số_nguyên check_mount_point(const char *device, số_nguyên *mount_flags,
				  char *mtpt, số_nguyên mtlen)
{
	số_nguyên	retval = 0;

	nếu (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		nếu (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} không_thì {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#không_thì
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#không_thì
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	nếu (retval)
		trả_về retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		struct stat st_buf;
		số_nguyên fd;
		nếu ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			trả_về 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		nếu (fd < 0) {
			nếu (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} không_thì
			close(fd);
	}
#endif

	trả_về 0;
}

số_nguyên is_mounted(const char *file)
{
	số_nguyên	retval;
	số_nguyên	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	nếu (retval)
		trả_về 0;
	trả_về mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
số_nguyên main(số_nguyên argc, char **argv)
{
	số_nguyên flags = 0;
	char devname[PATH_MAX];

	nếu (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		trả_về EXIT_FAILURE;
	}

	nếu (check_mount_point(argv[1], &flags, devname, sizeof(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		nếu (flags & MF_SWAP)
			printf("used swap device\n");
		không_thì
			printf("mounted on %s\n", devname);
		trả_về EXIT_SUCCESS;
	}

	printf("not mounted\n");
	trả_về EXIT_FAILURE;
}
#endif /* DEBUG */
