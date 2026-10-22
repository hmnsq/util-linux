/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT_H
#ਅਨ੍ਤਰ੍ਭਾਵ <mntent.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ismounted.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
# include "loopdev.h"
#ਜੇ_ਅੰਤ



#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ਸਥਿਰ ਪੂਰਨ check_mntent_file(ਅਚਲ ਅੱਖਰ *mtab_file, ਅਚਲ ਅੱਖਰ *file,
				   ਪੂਰਨ *mount_flags, ਅੱਖਰ *mtpt, ਪੂਰਨ mtlen)
{
	ਬਣਤਰ mntent	*mnt;
	ਬਣਤਰ stat	st_buf;
	ਪੂਰਨ		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ਪੂਰਨ		fd;

	*mount_flags = 0;
	ਜੇ ((f = setmntent (mtab_file, "r")) == ਨਿਰਾ)
		ਵਾਪਸ errno;

	ਜੇ (stat(file, &st_buf) == 0) {
		ਜੇ (S_ISBLK(st_buf.st_mode)) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#ਜੇ_ਅੰਤ	/* __GNU__ */
		} ਨਹੀਂਤਾਂ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ਜਦੋਂ ((mnt = getmntent (f)) != ਨਿਰਾ) {
		ਜੇ (mnt->mnt_fsname[0] != '/')
			ਜਾਰੀ;
		ਜੇ (strcmp(file, mnt->mnt_fsname) == 0)
			ਰੋਕੋ;
		ਜੇ (stat(mnt->mnt_fsname, &st_buf) != 0)
			ਜਾਰੀ;

		ਜੇ (S_ISBLK(st_buf.st_mode)) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__
			ਜੇ (file_rdev && file_rdev == st_buf.st_rdev)
				ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			/* maybe the file is loopdev backing file */
			ਜੇ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_ਵਰਤਿਆ(mnt->mnt_fsname, file, 0, 0, 0))
				ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* __linux__ */
#ਜੇ_ਅੰਤ	/* __GNU__ */
		} ਨਹੀਂਤਾਂ {
			ਜੇ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ਰੋਕੋ;
		}
	}

	ਜੇ (mnt == ਨਿਰਾ) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ਜੇ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ਜੇ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			ਜਾਓ is_ਜੜ੍ਹ;
		}
#ਜੇ_ਅੰਤ	/* __GNU__ */
		ਜਾਓ errout;
	}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ਜੇ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ਜੇ (retval == ENOENT) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
			ਲਿਖੋ("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#ਜੇ_ਅੰਤ /* DEBUG */
			retval = 0;
		}
		ਜਾਓ errout;
	}
	ਜੇ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
		ਲਿਖੋ("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#ਜੇ_ਅੰਤ /* DEBUG */
		ਜਾਓ errout;
	}
#ਜੇ_ਅੰਤ /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MNTOPT_RO
	/* Check to see if the ro option is set */
	ਜੇ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#ਜੇ_ਅੰਤ

	ਜੇ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ਜੇ (!strcmp(mnt->mnt_dir, "/")) {
is_ਜੜ੍ਹ:
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ਜੇ (fd < 0) {
			ਜੇ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ਨਹੀਂਤਾਂ
			close(fd);
		(ਖਾਲੀ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ਵਾਪਸ retval;
}

ਸਥਿਰ ਪੂਰਨ check_mntent(ਅਚਲ ਅੱਖਰ *file, ਪੂਰਨ *mount_flags,
			      ਅੱਖਰ *mtpt, ਪੂਰਨ mtlen)
{
	ਪੂਰਨ	retval;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ਜੇ (retval == 0)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ /* DEBUG */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ਜੇ (retval == 0 && (*mount_flags != 0))
		ਵਾਪਸ 0;
	ਜੇ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ਵਾਪਸ retval;
	}
#ਜੇ_ਅੰਤ /* __linux__ */
#ਜੇ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MOUNTED
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MOUNTED _PATH_MOUNTED
#ਜੇ_ਅੰਤ /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ਵਾਪਸ retval;
#ਨਹੀਂਤਾਂ
	*mount_flags = 0;
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ਨਹੀਂਤਾਂ
#ਜੇ defined(HAVE_GETMNTINFO)

ਸਥਿਰ ਪੂਰਨ check_getmntinfo(ਅਚਲ ਅੱਖਰ *file, ਪੂਰਨ *mount_flags,
				  ਅੱਖਰ *mtpt, ਪੂਰਨ mtlen)
{
	ਬਣਤਰ statfs *mp;
	ਪੂਰਨ    len, n;
	ਅਚਲ  ਅੱਖਰ   *s1;
	ਅੱਖਰ	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ਜੇ (n == 0)
		ਵਾਪਸ errno;

	len = ਆਕਾਰ(_PATH_DEV) - 1;
	s1 = file;
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ਜਦੋਂ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ਜੇ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ਰੋਕੋ;
		}
		++mp;
	}
	ਜੇ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* HAVE_GETMNTINFO */
#ਜੇ_ਅੰਤ /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ਸਥਿਰ ਪੂਰਨ is_ਅਦਲਾ_ਬਦਲੀ_device(ਅਚਲ ਅੱਖਰ *file)
{
	FILE		*f;
	ਅੱਖਰ		buf[1024], *cp;
	dev_t		file_dev;
	ਬਣਤਰ stat	st_buf;
	ਪੂਰਨ		ret = 0;

	file_dev = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ਜੇ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#ਜੇ_ਅੰਤ	/* __GNU__ */

	ਜੇ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ਵਾਪਸ 0;
	/* Skip the first line */
	ਜੇ (!fgets(buf, ਆਕਾਰ(buf), f))
		ਜਾਓ leave;
	ਜੇ (*buf && ਲੜੀ_ਤੁਲਨਾ(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		ਜਾਓ valid_ਪਹਿਲਾ_ਸਤਰ;

	ਜਦੋਂ (fgets(buf, ਆਕਾਰ(buf), f)) {
valid_ਪਹਿਲਾ_ਸਤਰ:
		ਜੇ ((cp = strchr(buf, ' ')) != ਨਿਰਾ)
			*cp = 0;
		ਜੇ ((cp = strchr(buf, '\t')) != ਨਿਰਾ)
			*cp = 0;
		ਜੇ (strcmp(buf, file) == 0) {
			ret++;
			ਰੋਕੋ;
		}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __GNU__
		ਜੇ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ	/* __GNU__ */
	}

leave:
	fclose(f);
	ਵਾਪਸ ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __TURBOC__
 #ਹਦਾਇਤ argsused
#ਜੇ_ਅੰਤ
ਪੂਰਨ check_mount_ਬਿੰਦੂ(ਅਚਲ ਅੱਖਰ *device, ਪੂਰਨ *mount_flags,
				  ਅੱਖਰ *mtpt, ਪੂਰਨ mtlen)
{
	ਪੂਰਨ	retval = 0;

	ਜੇ (is_ਅਦਲਾ_ਬਦਲੀ_device(device)) {
		*mount_flags = MF_MOUNTED | MF_ਅਦਲਾ_ਬਦਲੀ;
		ਜੇ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#ਜੇ_ਅੰਤ
		*mount_flags = 0;
#ਜੇ_ਅੰਤ /* HAVE_GETMNTINFO */
#ਜੇ_ਅੰਤ /* HAVE_MNTENT_H */
	}
	ਜੇ (retval)
		ਵਾਪਸ retval;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__ /* This only works on Linux 2.6+ systems */
	{
		ਬਣਤਰ stat st_buf;
		ਪੂਰਨ fd;
		ਜੇ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ਵਾਪਸ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ਜੇ (fd < 0) {
			ਜੇ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ਨਹੀਂਤਾਂ
			close(fd);
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਪੂਰਨ is_mounted(ਅਚਲ ਅੱਖਰ *file)
{
	ਪੂਰਨ	retval;
	ਪੂਰਨ	mount_flags = 0;

	retval = check_mount_ਬਿੰਦੂ(file, &mount_flags, ਨਿਰਾ, 0);
	ਜੇ (retval)
		ਵਾਪਸ 0;
	ਵਾਪਸ mount_flags & MF_MOUNTED;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_ISMOUNTED
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ flags = 0;
	ਅੱਖਰ devname[PATH_MAX];

	ਜੇ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਜੇ (check_mount_ਬਿੰਦੂ(argv[1], &flags, devname, ਆਕਾਰ(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ਜੇ (flags & MF_ਅਦਲਾ_ਬਦਲੀ)
			ਲਿਖੋ("used swap device\n");
		ਨਹੀਂਤਾਂ
			ਲਿਖੋ("mounted on %s\n", devname);
		ਵਾਪਸ EXIT_SUCCESS;
	}

	ਲਿਖੋ("not mounted\n");
	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ /* DEBUG */
