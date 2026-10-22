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
static පූර්ණ_සංඛ්‍යා check_mntent_file(const var_na *mtab_file, const var_na *file,
				   පූර්ණ_සංඛ්‍යා *mount_flags, var_na *mtpt, පූර්ණ_සංඛ්‍යා mtlen)
{
	sa_nracanaa mntent	*mnt;
	sa_nracanaa stat	st_buf;
	පූර්ණ_සංඛ්‍යා		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	පූර්ණ_සංඛ්‍යා		fd;

	*mount_flags = 0;
	නම් ((f = setmntent (mtab_file, "r")) == NULL)
		ආපසු errno;

	නම් (stat(file, &st_buf) == 0) {
		නම් (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} නැතහොත් {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	අතරතුර ((mnt = getmntent (f)) != NULL) {
		නම් (mnt->mnt_fsname[0] != '/')
			continue;
		නම් (strcmp(file, mnt->mnt_fsname) == 0)
			bha_nga;
		නම් (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		නම් (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			නම් (file_rdev && file_rdev == st_buf.st_rdev)
				bha_nga;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			නම් (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				bha_nga;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} නැතහොත් {
			නම් (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				bha_nga;
		}
	}

	නම් (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		නම් (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			නම් (mtpt)
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
	නම් (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		නම් (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	නම් (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	නම් (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	නම් (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	නම් (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		නම් (fd < 0) {
			නම් (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} නැතහොත්
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ආපසු retval;
}

static පූර්ණ_සංඛ්‍යා check_mntent(const var_na *file, පූර්ණ_සංඛ්‍යා *mount_flags,
			      var_na *mtpt, පූර්ණ_සංඛ්‍යා mtlen)
{
	පූර්ණ_සංඛ්‍යා	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	නම් (retval == 0)
		ආපසු 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	නම් (retval == 0 && (*mount_flags != 0))
		ආපසු 0;
	නම් (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ආපසු retval;
	}
#endif /* __linux__ */
#නම් defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ආපසු retval;
#නැතහොත්
	*mount_flags = 0;
	ආපසු 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#නැතහොත්
#නම් defined(HAVE_GETMNTINFO)

static පූර්ණ_සංඛ්‍යා check_getmntinfo(const var_na *file, පූර්ණ_සංඛ්‍යා *mount_flags,
				  var_na *mtpt, පූර්ණ_සංඛ්‍යා mtlen)
{
	sa_nracanaa statfs *mp;
	පූර්ණ_සංඛ්‍යා    len, n;
	const  var_na   *s1;
	var_na	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	නම් (n == 0)
		ආපසු errno;

	len = parimaa_na(_PATH_DEV) - 1;
	s1 = file;
	නම් (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	අතරතුර (--n >= 0) {
		s2 = mp->f_mntfromname;
		නම් (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		නම් (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			bha_nga;
		}
		++mp;
	}
	නම් (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ආපසු 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static පූර්ණ_සංඛ්‍යා is_swap_device(const var_na *file)
{
	FILE		*f;
	var_na		buf[1024], *cp;
	dev_t		file_dev;
	sa_nracanaa stat	st_buf;
	පූර්ණ_සංඛ්‍යා		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	නම් ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	නම් (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ආපසු 0;
	/* Skip the first line */
	නම් (!fgets(buf, parimaa_na(buf), f))
		goto leave;
	නම් (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	අතරතුර (fgets(buf, parimaa_na(buf), f)) {
valid_first_line:
		නම් ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		නම් ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		නම් (strcmp(buf, file) == 0) {
			ret++;
			bha_nga;
		}
#ifndef __GNU__
		නම් (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			bha_nga;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ආපසු ret;
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
පූර්ණ_සංඛ්‍යා check_mount_point(const var_na *device, පූර්ණ_සංඛ්‍යා *mount_flags,
				  var_na *mtpt, පූර්ණ_සංඛ්‍යා mtlen)
{
	පූර්ණ_සංඛ්‍යා	retval = 0;

	නම් (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		නම් (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} නැතහොත් {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#නැතහොත්
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#නැතහොත්
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	නම් (retval)
		ආපසු retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		sa_nracanaa stat st_buf;
		පූර්ණ_සංඛ්‍යා fd;
		නම් ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ආපසු 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		නම් (fd < 0) {
			නම් (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} නැතහොත්
			close(fd);
	}
#endif

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා is_mounted(const var_na *file)
{
	පූර්ණ_සංඛ්‍යා	retval;
	පූර්ණ_සංඛ්‍යා	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	නම් (retval)
		ආපසු 0;
	ආපසු mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	පූර්ණ_සංඛ්‍යා flags = 0;
	var_na devname[PATH_MAX];

	නම් (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ආපසු EXIT_FAILURE;
	}

	නම් (check_mount_point(argv[1], &flags, devname, parimaa_na(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		නම් (flags & MF_SWAP)
			printf("used swap device\n");
		නැතහොත්
			printf("mounted on %s\n", devname);
		ආපසු EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ආපසු EXIT_FAILURE;
}
#endif /* DEBUG */
