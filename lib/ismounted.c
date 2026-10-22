/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#ifdef HAVE_MNTENT_H
#অন্তর্ভাব <mntent.h>
#endif
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "ismounted.h"
#অন্তর্ভাব "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
স্থির পূর্ণসংখ্যা check_mntent_file(ধ্রুবক অক্ষর *mtab_file, ধ্রুবক অক্ষর *file,
				   পূর্ণসংখ্যা *mount_flags, অক্ষর *mtpt, পূর্ণসংখ্যা mtlen)
{
	কাঠামো mntent	*mnt;
	কাঠামো stat	st_buf;
	পূর্ণসংখ্যা		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	পূর্ণসংখ্যা		fd;

	*mount_flags = 0;
	যদি ((f = setmntent (mtab_file, "r")) == ফাঁকা)
		ফেরত errno;

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

	যখন ((mnt = getmntent (f)) != ফাঁকা) {
		যদি (mnt->mnt_fsname[0] != '/')
			চালু;
		যদি (strcmp(file, mnt->mnt_fsname) == 0)
			বিরতি;
		যদি (stat(mnt->mnt_fsname, &st_buf) != 0)
			চালু;

		যদি (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			যদি (file_rdev && file_rdev == st_buf.st_rdev)
				বিরতি;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			যদি (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				বিরতি;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} অন্যথা {
			যদি (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				বিরতি;
		}
	}

	যদি (mnt == ফাঁকা) {
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
			যাও is_মূল;
		}
#endif	/* __GNU__ */
		যাও errout;
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
		যাও errout;
	}
	যদি (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		যাও errout;
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
	ফেরত retval;
}

স্থির পূর্ণসংখ্যা check_mntent(ধ্রুবক অক্ষর *file, পূর্ণসংখ্যা *mount_flags,
			      অক্ষর *mtpt, পূর্ণসংখ্যা mtlen)
{
	পূর্ণসংখ্যা	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	যদি (retval == 0)
		ফেরত 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	যদি (retval == 0 && (*mount_flags != 0))
		ফেরত 0;
	যদি (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ফেরত retval;
	}
#endif /* __linux__ */
#যদি defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ফেরত retval;
#অন্যথা
	*mount_flags = 0;
	ফেরত 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#অন্যথা
#যদি defined(HAVE_GETMNTINFO)

স্থির পূর্ণসংখ্যা check_getmntinfo(ধ্রুবক অক্ষর *file, পূর্ণসংখ্যা *mount_flags,
				  অক্ষর *mtpt, পূর্ণসংখ্যা mtlen)
{
	কাঠামো statfs *mp;
	পূর্ণসংখ্যা    len, n;
	ধ্রুবক  অক্ষর   *s1;
	অক্ষর	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	যদি (n == 0)
		ফেরত errno;

	len = আকার(_PATH_DEV) - 1;
	s1 = file;
	যদি (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	যখন (--n >= 0) {
		s2 = mp->f_mntfromname;
		যদি (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		যদি (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			বিরতি;
		}
		++mp;
	}
	যদি (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ফেরত 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
স্থির পূর্ণসংখ্যা is_swap_device(ধ্রুবক অক্ষর *file)
{
	FILE		*f;
	অক্ষর		buf[1024], *cp;
	dev_t		file_dev;
	কাঠামো stat	st_buf;
	পূর্ণসংখ্যা		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	যদি ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	যদি (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ফেরত 0;
	/* Skip the first line */
	যদি (!fgets(buf, আকার(buf), f))
		যাও leave;
	যদি (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		যাও valid_first_সারি;

	যখন (fgets(buf, আকার(buf), f)) {
valid_first_সারি:
		যদি ((cp = strchr(buf, ' ')) != ফাঁকা)
			*cp = 0;
		যদি ((cp = strchr(buf, '\t')) != ফাঁকা)
			*cp = 0;
		যদি (strcmp(buf, file) == 0) {
			ret++;
			বিরতি;
		}
#ifndef __GNU__
		যদি (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			বিরতি;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ফেরত ret;
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
পূর্ণসংখ্যা check_mount_point(ধ্রুবক অক্ষর *device, পূর্ণসংখ্যা *mount_flags,
				  অক্ষর *mtpt, পূর্ণসংখ্যা mtlen)
{
	পূর্ণসংখ্যা	retval = 0;

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
		ফেরত retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		কাঠামো stat st_buf;
		পূর্ণসংখ্যা fd;
		যদি ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ফেরত 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		যদি (fd < 0) {
			যদি (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} অন্যথা
			close(fd);
	}
#endif

	ফেরত 0;
}

পূর্ণসংখ্যা is_mounted(ধ্রুবক অক্ষর *file)
{
	পূর্ণসংখ্যা	retval;
	পূর্ণসংখ্যা	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, ফাঁকা, 0);
	যদি (retval)
		ফেরত 0;
	ফেরত mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা flags = 0;
	অক্ষর devname[PATH_MAX];

	যদি (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ফেরত EXIT_FAILURE;
	}

	যদি (check_mount_point(argv[1], &flags, devname, আকার(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		যদি (flags & MF_SWAP)
			printf("used swap device\n");
		অন্যথা
			printf("mounted on %s\n", devname);
		ফেরত EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ফেরত EXIT_FAILURE;
}
#endif /* DEBUG */
