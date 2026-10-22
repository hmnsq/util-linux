/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ifdef HAVE_MNTENT_H
#ಅನ್ತರ್ಭಾವ <mntent.h>
#endif
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "ismounted.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_mntent_file(ಧ್ರುವ ವರ್ಣ *mtab_file, ಧ್ರುವ ವರ್ಣ *file,
				   ಪೂರ್ಣಾಙ್ಕ *mount_flags, ವರ್ಣ *mtpt, ಪೂರ್ಣಾಙ್ಕ mtlen)
{
	ಸಂರಚನಾ mntent	*mnt;
	ಸಂರಚನಾ stat	st_buf;
	ಪೂರ್ಣಾಙ್ಕ		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ಪೂರ್ಣಾಙ್ಕ		fd;

	*mount_flags = 0;
	ಯದಿ ((f = setmntent (mtab_file, "r")) == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ errno;

	ಯದಿ (stat(file, &st_buf) == 0) {
		ಯದಿ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} ಅನ್ಯಥಾ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ಯಾವತ್ ((mnt = getmntent (f)) != ರಿಕ್ತ) {
		ಯದಿ (mnt->mnt_fsname[0] != '/')
			ಅನುವರ್ತ;
		ಯದಿ (strcmp(file, mnt->mnt_fsname) == 0)
			ಭಙ್ಗ;
		ಯದಿ (stat(mnt->mnt_fsname, &st_buf) != 0)
			ಅನುವರ್ತ;

		ಯದಿ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ಯದಿ (file_rdev && file_rdev == st_buf.st_rdev)
				ಭಙ್ಗ;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ಯದಿ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				ಭಙ್ಗ;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} ಅನ್ಯಥಾ {
			ಯದಿ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ಭಙ್ಗ;
		}
	}

	ಯದಿ (mnt == ರಿಕ್ತ) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ಯದಿ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ಯದಿ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			ಗಚ್ಛ is_ಬೇರು;
		}
#endif	/* __GNU__ */
		ಗಚ್ಛ errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ಯದಿ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ಯದಿ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		ಗಚ್ಛ errout;
	}
	ಯದಿ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		ಗಚ್ಛ errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	ಯದಿ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ಯದಿ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ಯದಿ (!strcmp(mnt->mnt_dir, "/")) {
is_ಬೇರು:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ಯದಿ (fd < 0) {
			ಯದಿ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ಅನ್ಯಥಾ
			close(fd);
		(ಶೂನ್ಯ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_mntent(ಧ್ರುವ ವರ್ಣ *file, ಪೂರ್ಣಾಙ್ಕ *mount_flags,
			      ವರ್ಣ *mtpt, ಪೂರ್ಣಾಙ್ಕ mtlen)
{
	ಪೂರ್ಣಾಙ್ಕ	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ಯದಿ (retval == 0)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ಯದಿ (retval == 0 && (*mount_flags != 0))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ಪ್ರತ್ಯಾಗಮ retval;
	}
#endif /* __linux__ */
#ಯದಿ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ಪ್ರತ್ಯಾಗಮ retval;
#ಅನ್ಯಥಾ
	*mount_flags = 0;
	ಪ್ರತ್ಯಾಗಮ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ಅನ್ಯಥಾ
#ಯದಿ defined(HAVE_GETMNTINFO)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_getmntinfo(ಧ್ರುವ ವರ್ಣ *file, ಪೂರ್ಣಾಙ್ಕ *mount_flags,
				  ವರ್ಣ *mtpt, ಪೂರ್ಣಾಙ್ಕ mtlen)
{
	ಸಂರಚನಾ statfs *mp;
	ಪೂರ್ಣಾಙ್ಕ    len, n;
	ಧ್ರುವ  ವರ್ಣ   *s1;
	ವರ್ಣ	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ಯದಿ (n == 0)
		ಪ್ರತ್ಯಾಗಮ errno;

	len = ಪರಿಮಾಣ(_PATH_DEV) - 1;
	s1 = file;
	ಯದಿ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ಯಾವತ್ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ಯದಿ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ಯದಿ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ಭಙ್ಗ;
		}
		++mp;
	}
	ಯದಿ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_swap_device(ಧ್ರುವ ವರ್ಣ *file)
{
	FILE		*f;
	ವರ್ಣ		buf[1024], *cp;
	dev_t		file_dev;
	ಸಂರಚನಾ stat	st_buf;
	ಪೂರ್ಣಾಙ್ಕ		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ಯದಿ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ಯದಿ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ಪ್ರತ್ಯಾಗಮ 0;
	/* Skip the first line */
	ಯದಿ (!fgets(buf, ಪರಿಮಾಣ(buf), f))
		ಗಚ್ಛ leave;
	ಯದಿ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		ಗಚ್ಛ valid_first_ಸಾಲು;

	ಯಾವತ್ (fgets(buf, ಪರಿಮಾಣ(buf), f)) {
valid_first_ಸಾಲು:
		ಯದಿ ((cp = strchr(buf, ' ')) != ರಿಕ್ತ)
			*cp = 0;
		ಯದಿ ((cp = strchr(buf, '\t')) != ರಿಕ್ತ)
			*cp = 0;
		ಯದಿ (strcmp(buf, file) == 0) {
			ret++;
			ಭಙ್ಗ;
		}
#ifndef __GNU__
		ಯದಿ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ಭಙ್ಗ;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ಪ್ರತ್ಯಾಗಮ ret;
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
ಪೂರ್ಣಾಙ್ಕ check_mount_point(ಧ್ರುವ ವರ್ಣ *device, ಪೂರ್ಣಾಙ್ಕ *mount_flags,
				  ವರ್ಣ *mtpt, ಪೂರ್ಣಾಙ್ಕ mtlen)
{
	ಪೂರ್ಣಾಙ್ಕ	retval = 0;

	ಯದಿ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ಯದಿ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ಅನ್ಯಥಾ {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ಅನ್ಯಥಾ
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ಅನ್ಯಥಾ
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ಯದಿ (retval)
		ಪ್ರತ್ಯಾಗಮ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		ಸಂರಚನಾ stat st_buf;
		ಪೂರ್ಣಾಙ್ಕ fd;
		ಯದಿ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ಪ್ರತ್ಯಾಗಮ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ಯದಿ (fd < 0) {
			ಯದಿ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ಅನ್ಯಥಾ
			close(fd);
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ is_mounted(ಧ್ರುವ ವರ್ಣ *file)
{
	ಪೂರ್ಣಾಙ್ಕ	retval;
	ಪೂರ್ಣಾಙ್ಕ	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, ರಿಕ್ತ, 0);
	ಯದಿ (retval)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ flags = 0;
	ವರ್ಣ devname[PATH_MAX];

	ಯದಿ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	ಯದಿ (check_mount_point(argv[1], &flags, devname, ಪರಿಮಾಣ(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ಯದಿ (flags & MF_SWAP)
			printf("used swap device\n");
		ಅನ್ಯಥಾ
			printf("mounted on %s\n", devname);
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif /* DEBUG */
