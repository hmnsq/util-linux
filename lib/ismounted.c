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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT_H
#ಅನ್ತರ್ಭಾವ <mntent.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <sys/param.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "ismounted.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
# include "loopdev.h"
#ಆದರೆ_ಅಂತ್ಯ



#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_mntent_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mtab_file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file,
				   ಪೂರ್ಣಾಂಕ *mount_flags, ಅಕ್ಷರ *mtpt, ಪೂರ್ಣಾಂಕ mtlen)
{
	ರಚನೆ mntent	*mnt;
	ರಚನೆ stat	st_buf;
	ಪೂರ್ಣಾಂಕ		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ಪೂರ್ಣಾಂಕ		fd;

	*mount_flags = 0;
	ಆದರೆ ((f = setmntent (mtab_file, "r")) == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ errno;

	ಆದರೆ (stat(file, &st_buf) == 0) {
		ಆದರೆ (S_ISBLK(st_buf.st_mode)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#ಆದರೆ_ಅಂತ್ಯ	/* __GNU__ */
		} ಇಲ್ಲದಿದ್ದರೆ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ಆಗ ((mnt = getmntent (f)) != ಖಾಲಿ) {
		ಆದರೆ (mnt->mnt_fsname[0] != '/')
			ಮುಂದುವರಿ;
		ಆದರೆ (strcmp(file, mnt->mnt_fsname) == 0)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (stat(mnt->mnt_fsname, &st_buf) != 0)
			ಮುಂದುವರಿ;

		ಆದರೆ (S_ISBLK(st_buf.st_mode)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__
			ಆದರೆ (file_rdev && file_rdev == st_buf.st_rdev)
				ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			/* maybe the file is loopdev backing file */
			ಆದರೆ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_ಬಳಸಿದ(mnt->mnt_fsname, file, 0, 0, 0))
				ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* __linux__ */
#ಆದರೆ_ಅಂತ್ಯ	/* __GNU__ */
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (mnt == ಖಾಲಿ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ಆದರೆ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ಆದರೆ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			ಹೋಗು is_ಬೇರು;
		}
#ಆದರೆ_ಅಂತ್ಯ	/* __GNU__ */
		ಹೋಗು errout;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ಆದರೆ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ಆದರೆ (retval == ENOENT) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
			ಬರೆ("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#ಆದರೆ_ಅಂತ್ಯ /* DEBUG */
			retval = 0;
		}
		ಹೋಗು errout;
	}
	ಆದರೆ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
		ಬರೆ("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#ಆದರೆ_ಅಂತ್ಯ /* DEBUG */
		ಹೋಗು errout;
	}
#ಆದರೆ_ಅಂತ್ಯ /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MNTOPT_RO
	/* Check to see if the ro option is set */
	ಆದರೆ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ಆದರೆ (!strcmp(mnt->mnt_dir, "/")) {
is_ಬೇರು:
#ವ್ಯಾಖ್ಯಾನಿಸು TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ಆದರೆ (fd < 0) {
			ಆದರೆ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ಇಲ್ಲದಿದ್ದರೆ
			close(fd);
		(ಶೂನ್ಯ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_mntent(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ *mount_flags,
			      ಅಕ್ಷರ *mtpt, ಪೂರ್ಣಾಂಕ mtlen)
{
	ಪೂರ್ಣಾಂಕ	retval;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ಆದರೆ (retval == 0)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ /* DEBUG */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ಆದರೆ (retval == 0 && (*mount_flags != 0))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ಹಿಂತಿರುಗಿ retval;
	}
#ಆದರೆ_ಅಂತ್ಯ /* __linux__ */
#ಆದರೆ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MOUNTED
#ವ್ಯಾಖ್ಯಾನಿಸು MOUNTED _PATH_MOUNTED
#ಆದರೆ_ಅಂತ್ಯ /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ಹಿಂತಿರುಗಿ retval;
#ಇಲ್ಲದಿದ್ದರೆ
	*mount_flags = 0;
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ defined(HAVE_GETMNTINFO)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_getmntinfo(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ *mount_flags,
				  ಅಕ್ಷರ *mtpt, ಪೂರ್ಣಾಂಕ mtlen)
{
	ರಚನೆ statfs *mp;
	ಪೂರ್ಣಾಂಕ    len, n;
	ಸ್ಥಿರಾಂಕ  ಅಕ್ಷರ   *s1;
	ಅಕ್ಷರ	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ಆದರೆ (n == 0)
		ಹಿಂತಿರುಗಿ errno;

	len = ಗಾತ್ರ(_PATH_DEV) - 1;
	s1 = file;
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ಆಗ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ಆದರೆ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ನಿಲ್ಲಿಸು;
		}
		++mp;
	}
	ಆದರೆ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_GETMNTINFO */
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_ಅದಲುಬದಲು_device(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	FILE		*f;
	ಅಕ್ಷರ		buf[1024], *cp;
	dev_t		file_dev;
	ರಚನೆ stat	st_buf;
	ಪೂರ್ಣಾಂಕ		ret = 0;

	file_dev = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ಆದರೆ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#ಆದರೆ_ಅಂತ್ಯ	/* __GNU__ */

	ಆದರೆ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ಹಿಂತಿರುಗಿ 0;
	/* Skip the first line */
	ಆದರೆ (!fgets(buf, ಗಾತ್ರ(buf), f))
		ಹೋಗು leave;
	ಆದರೆ (*buf && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		ಹೋಗು valid_ಮೊದಲ_ಸಾಲು;

	ಆಗ (fgets(buf, ಗಾತ್ರ(buf), f)) {
valid_ಮೊದಲ_ಸಾಲು:
		ಆದರೆ ((cp = strchr(buf, ' ')) != ಖಾಲಿ)
			*cp = 0;
		ಆದರೆ ((cp = strchr(buf, '\t')) != ಖಾಲಿ)
			*cp = 0;
		ಆದರೆ (strcmp(buf, file) == 0) {
			ret++;
			ನಿಲ್ಲಿಸು;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __GNU__
		ಆದರೆ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ	/* __GNU__ */
	}

leave:
	fclose(f);
	ಹಿಂತಿರುಗಿ ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __TURBOC__
 #ಸೂಚನೆ argsused
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ check_mount_ಬಿಂದು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device, ಪೂರ್ಣಾಂಕ *mount_flags,
				  ಅಕ್ಷರ *mtpt, ಪೂರ್ಣಾಂಕ mtlen)
{
	ಪೂರ್ಣಾಂಕ	retval = 0;

	ಆದರೆ (is_ಅದಲುಬದಲು_device(device)) {
		*mount_flags = MF_MOUNTED | MF_ಅದಲುಬದಲು;
		ಆದರೆ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#ಆದರೆ_ಅಂತ್ಯ
		*mount_flags = 0;
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_GETMNTINFO */
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MNTENT_H */
	}
	ಆದರೆ (retval)
		ಹಿಂತಿರುಗಿ retval;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__ /* This only works on Linux 2.6+ systems */
	{
		ರಚನೆ stat st_buf;
		ಪೂರ್ಣಾಂಕ fd;
		ಆದರೆ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ಹಿಂತಿರುಗಿ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ಆದರೆ (fd < 0) {
			ಆದರೆ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ಇಲ್ಲದಿದ್ದರೆ
			close(fd);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ is_mounted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file)
{
	ಪೂರ್ಣಾಂಕ	retval;
	ಪೂರ್ಣಾಂಕ	mount_flags = 0;

	retval = check_mount_ಬಿಂದು(file, &mount_flags, ಖಾಲಿ, 0);
	ಆದರೆ (retval)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ mount_flags & MF_MOUNTED;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_ISMOUNTED
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ flags = 0;
	ಅಕ್ಷರ devname[PATH_MAX];

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಆದರೆ (check_mount_ಬಿಂದು(argv[1], &flags, devname, ಗಾತ್ರ(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ಆದರೆ (flags & MF_ಅದಲುಬದಲು)
			ಬರೆ("used swap device\n");
		ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ("mounted on %s\n", devname);
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	ಬರೆ("not mounted\n");
	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ /* DEBUG */
