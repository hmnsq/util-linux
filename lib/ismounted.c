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
static цяло check_mntent_file(const char *mtab_file, const char *file,
				   цяло *mount_flags, char *mtpt, цяло mtlen)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	цяло		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	цяло		fd;

	*mount_flags = 0;
	ако ((f = setmntent (mtab_file, "r")) == NULL)
		върни errno;

	ако (stat(file, &st_buf) == 0) {
		ако (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} иначе {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	докато ((mnt = getmntent (f)) != NULL) {
		ако (mnt->mnt_fsname[0] != '/')
			continue;
		ако (strcmp(file, mnt->mnt_fsname) == 0)
			break;
		ако (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		ако (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ако (file_rdev && file_rdev == st_buf.st_rdev)
				break;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ако (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				break;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} иначе {
			ако (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				break;
		}
	}

	ако (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ако (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ако (mtpt)
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
	ако (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ако (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	ако (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	ако (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ако (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ако (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ако (fd < 0) {
			ако (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} иначе
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	върни retval;
}

static цяло check_mntent(const char *file, цяло *mount_flags,
			      char *mtpt, цяло mtlen)
{
	цяло	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ако (retval == 0)
		върни 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ако (retval == 0 && (*mount_flags != 0))
		върни 0;
	ако (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		върни retval;
	}
#endif /* __linux__ */
#ако defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	върни retval;
#иначе
	*mount_flags = 0;
	върни 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#иначе
#ако defined(HAVE_GETMNTINFO)

static цяло check_getmntinfo(const char *file, цяло *mount_flags,
				  char *mtpt, цяло mtlen)
{
	struct statfs *mp;
	цяло    len, n;
	const  char   *s1;
	char	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ако (n == 0)
		върни errno;

	len = sizeof(_PATH_DEV) - 1;
	s1 = file;
	ако (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	докато (--n >= 0) {
		s2 = mp->f_mntfromname;
		ако (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ако (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			break;
		}
		++mp;
	}
	ако (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	върни 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static цяло is_swap_device(const char *file)
{
	FILE		*f;
	char		buf[1024], *cp;
	dev_t		file_dev;
	struct stat	st_buf;
	цяло		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ако ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ако (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		върни 0;
	/* Skip the first line */
	ако (!fgets(buf, sizeof(buf), f))
		goto leave;
	ако (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	докато (fgets(buf, sizeof(buf), f)) {
valid_first_line:
		ако ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		ако ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		ако (strcmp(buf, file) == 0) {
			ret++;
			break;
		}
#ifndef __GNU__
		ако (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			break;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	върни ret;
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
цяло check_mount_point(const char *device, цяло *mount_flags,
				  char *mtpt, цяло mtlen)
{
	цяло	retval = 0;

	ако (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ако (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} иначе {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#иначе
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#иначе
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ако (retval)
		върни retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		struct stat st_buf;
		цяло fd;
		ако ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			върни 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ако (fd < 0) {
			ако (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} иначе
			close(fd);
	}
#endif

	върни 0;
}

цяло is_mounted(const char *file)
{
	цяло	retval;
	цяло	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	ако (retval)
		върни 0;
	върни mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
цяло main(цяло argc, char **argv)
{
	цяло flags = 0;
	char devname[PATH_MAX];

	ако (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		върни EXIT_FAILURE;
	}

	ако (check_mount_point(argv[1], &flags, devname, sizeof(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ако (flags & MF_SWAP)
			printf("used swap device\n");
		иначе
			printf("mounted on %s\n", devname);
		върни EXIT_SUCCESS;
	}

	printf("not mounted\n");
	върни EXIT_FAILURE;
}
#endif /* DEBUG */
