/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#ifdef HAVE_MNTENT_H
#অন্তর্ভাৱ <mntent.h>
#endif
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#অন্তর্ভাৱ "pathnames.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "ismounted.h"
#অন্তর্ভাৱ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
স্থির পূর্ণাঙ্ক check_mntent_file(ধ্রুৱ ৱর্ণ *mtab_file, ধ্রুৱ ৱর্ণ *file,
				   পূর্ণাঙ্ক *mount_flags, ৱর্ণ *mtpt, পূর্ণাঙ্ক mtlen)
{
	সংরচনা mntent	*mnt;
	সংরচনা stat	st_buf;
	পূর্ণাঙ্ক		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	পূর্ণাঙ্ক		fd;

	*mount_flags = 0;
	যদি ((f = setmntent (mtab_file, "r")) == রিক্ত)
		ঘূৰাই errno;

	যদি (stat(file, &st_buf) == 0) {
		যদি (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} অন্যথা {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	যেতিয়ালৈ ((mnt = getmntent (f)) != রিক্ত) {
		যদি (mnt->mnt_fsname[0] != '/')
			অনুৱর্ত;
		যদি (strcmp(file, mnt->mnt_fsname) == 0)
			ভঙ্গ;
		যদি (stat(mnt->mnt_fsname, &st_buf) != 0)
			অনুৱর্ত;

		যদি (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			যদি (file_rdev && file_rdev == st_buf.st_rdev)
				ভঙ্গ;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			যদি (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				ভঙ্গ;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} অন্যথা {
			যদি (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ভঙ্গ;
		}
	}

	যদি (mnt == রিক্ত) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		যদি (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			যদি (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			গচ্ছ is_মূল;
		}
#endif	/* __GNU__ */
		গচ্ছ errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	যদি (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		যদি (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		গচ্ছ errout;
	}
	যদি (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		গচ্ছ errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	যদি (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	যদি (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	যদি (!strcmp(mnt->mnt_dir, "/")) {
is_মূল:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		যদি (fd < 0) {
			যদি (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} অন্যথা
			close(fd);
		(শূন্য) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ঘূৰাই retval;
}

স্থির পূর্ণাঙ্ক check_mntent(ধ্রুৱ ৱর্ণ *file, পূর্ণাঙ্ক *mount_flags,
			      ৱর্ণ *mtpt, পূর্ণাঙ্ক mtlen)
{
	পূর্ণাঙ্ক	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	যদি (retval == 0)
		ঘূৰাই 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	যদি (retval == 0 && (*mount_flags != 0))
		ঘূৰাই 0;
	যদি (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ঘূৰাই retval;
	}
#endif /* __linux__ */
#যদি defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ঘূৰাই retval;
#অন্যথা
	*mount_flags = 0;
	ঘূৰাই 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#অন্যথা
#যদি defined(HAVE_GETMNTINFO)

স্থির পূর্ণাঙ্ক check_getmntinfo(ধ্রুৱ ৱর্ণ *file, পূর্ণাঙ্ক *mount_flags,
				  ৱর্ণ *mtpt, পূর্ণাঙ্ক mtlen)
{
	সংরচনা statfs *mp;
	পূর্ণাঙ্ক    len, n;
	ধ্রুৱ  ৱর্ণ   *s1;
	ৱর্ণ	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	যদি (n == 0)
		ঘূৰাই errno;

	len = পরিমাণ(_PATH_DEV) - 1;
	s1 = file;
	যদি (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	যেতিয়ালৈ (--n >= 0) {
		s2 = mp->f_mntfromname;
		যদি (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		যদি (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ভঙ্গ;
		}
		++mp;
	}
	যদি (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ঘূৰাই 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
স্থির পূর্ণাঙ্ক is_swap_device(ধ্রুৱ ৱর্ণ *file)
{
	FILE		*f;
	ৱর্ণ		buf[1024], *cp;
	dev_t		file_dev;
	সংরচনা stat	st_buf;
	পূর্ণাঙ্ক		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	যদি ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	যদি (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ঘূৰাই 0;
	/* Skip the first line */
	যদি (!fgets(buf, পরিমাণ(buf), f))
		গচ্ছ leave;
	যদি (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		গচ্ছ valid_first_সাৰি;

	যেতিয়ালৈ (fgets(buf, পরিমাণ(buf), f)) {
valid_first_সাৰি:
		যদি ((cp = strchr(buf, ' ')) != রিক্ত)
			*cp = 0;
		যদি ((cp = strchr(buf, '\t')) != রিক্ত)
			*cp = 0;
		যদি (strcmp(buf, file) == 0) {
			ret++;
			ভঙ্গ;
		}
#ifndef __GNU__
		যদি (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ভঙ্গ;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ঘূৰাই ret;
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
পূর্ণাঙ্ক check_mount_point(ধ্রুৱ ৱর্ণ *device, পূর্ণাঙ্ক *mount_flags,
				  ৱর্ণ *mtpt, পূর্ণাঙ্ক mtlen)
{
	পূর্ণাঙ্ক	retval = 0;

	যদি (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		যদি (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} অন্যথা {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#অন্যথা
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#অন্যথা
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	যদি (retval)
		ঘূৰাই retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		সংরচনা stat st_buf;
		পূর্ণাঙ্ক fd;
		যদি ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ঘূৰাই 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		যদি (fd < 0) {
			যদি (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} অন্যথা
			close(fd);
	}
#endif

	ঘূৰাই 0;
}

পূর্ণাঙ্ক is_mounted(ধ্রুৱ ৱর্ণ *file)
{
	পূর্ণাঙ্ক	retval;
	পূর্ণাঙ্ক	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, রিক্ত, 0);
	যদি (retval)
		ঘূৰাই 0;
	ঘূৰাই mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক flags = 0;
	ৱর্ণ devname[PATH_MAX];

	যদি (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ঘূৰাই EXIT_FAILURE;
	}

	যদি (check_mount_point(argv[1], &flags, devname, পরিমাণ(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		যদি (flags & MF_SWAP)
			printf("used swap device\n");
		অন্যথা
			printf("mounted on %s\n", devname);
		ঘূৰাই EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ঘূৰাই EXIT_FAILURE;
}
#endif /* DEBUG */
