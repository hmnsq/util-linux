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
static целое check_mntent_file(const char *mtab_file, const char *file,
				   целое *mount_flags, char *mtpt, целое mtlen)
{
	struct mntent	*mnt;
	struct stat	st_buf;
	целое		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	целое		fd;

	*mount_flags = 0;
	если ((f = setmntent (mtab_file, "r")) == NULL)
		вернуть errno;

	если (stat(file, &st_buf) == 0) {
		если (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} иначе {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	пока ((mnt = getmntent (f)) != NULL) {
		если (mnt->mnt_fsname[0] != '/')
			continue;
		если (strcmp(file, mnt->mnt_fsname) == 0)
			break;
		если (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		если (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			если (file_rdev && file_rdev == st_buf.st_rdev)
				break;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			если (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				break;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} иначе {
			если (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				break;
		}
	}

	если (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		если (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			если (mtpt)
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
	если (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		если (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	если (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	если (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	если (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	если (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		если (fd < 0) {
			если (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} иначе
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	вернуть retval;
}

static целое check_mntent(const char *file, целое *mount_flags,
			      char *mtpt, целое mtlen)
{
	целое	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	если (retval == 0)
		вернуть 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	если (retval == 0 && (*mount_flags != 0))
		вернуть 0;
	если (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		вернуть retval;
	}
#endif /* __linux__ */
#если defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	вернуть retval;
#иначе
	*mount_flags = 0;
	вернуть 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#иначе
#если defined(HAVE_GETMNTINFO)

static целое check_getmntinfo(const char *file, целое *mount_flags,
				  char *mtpt, целое mtlen)
{
	struct statfs *mp;
	целое    len, n;
	const  char   *s1;
	char	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	если (n == 0)
		вернуть errno;

	len = sizeof(_PATH_DEV) - 1;
	s1 = file;
	если (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	пока (--n >= 0) {
		s2 = mp->f_mntfromname;
		если (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		если (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			break;
		}
		++mp;
	}
	если (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	вернуть 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static целое is_swap_device(const char *file)
{
	FILE		*f;
	char		buf[1024], *cp;
	dev_t		file_dev;
	struct stat	st_buf;
	целое		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	если ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	если (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		вернуть 0;
	/* Skip the first line */
	если (!fgets(buf, sizeof(buf), f))
		goto leave;
	если (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	пока (fgets(buf, sizeof(buf), f)) {
valid_first_line:
		если ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		если ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		если (strcmp(buf, file) == 0) {
			ret++;
			break;
		}
#ifndef __GNU__
		если (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			break;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	вернуть ret;
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
целое check_mount_point(const char *device, целое *mount_flags,
				  char *mtpt, целое mtlen)
{
	целое	retval = 0;

	если (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		если (mtpt && mtlen)
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
	если (retval)
		вернуть retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		struct stat st_buf;
		целое fd;
		если ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			вернуть 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		если (fd < 0) {
			если (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} иначе
			close(fd);
	}
#endif

	вернуть 0;
}

целое is_mounted(const char *file)
{
	целое	retval;
	целое	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	если (retval)
		вернуть 0;
	вернуть mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
целое основной(целое argc, char **argv)
{
	целое flags = 0;
	char devname[PATH_MAX];

	если (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		вернуть EXIT_FAILURE;
	}

	если (check_mount_point(argv[1], &flags, devname, sizeof(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		если (flags & MF_SWAP)
			printf("used swap device\n");
		иначе
			printf("mounted on %s\n", devname);
		вернуть EXIT_SUCCESS;
	}

	printf("not mounted\n");
	вернуть EXIT_FAILURE;
}
#endif /* DEBUG */
