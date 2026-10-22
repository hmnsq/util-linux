/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#شامل <stdio.h>
#شامل <unistd.h>
#شامل <stdlib.h>
#شامل <errno.h>
#شامل <fcntl.h>
#اگر_تعریف_شدہ HAVE_MNTENT_H
#شامل <mntent.h>
#اگر_ختم
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <ctype.h>
#شامل <sys/param.h>

#اگر_غیر_تعریف_شدہ __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#اگر_ختم

#شامل "pathnames.h"
#شامل "strutils.h"
#شامل "ismounted.h"
#شامل "c.h"
#اگر_تعریف_شدہ __linux__
# include "loopdev.h"
#اگر_ختم



#اگر_تعریف_شدہ HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ساکن صحیح check_mntent_file(ثابت حرف *mtab_file, ثابت حرف *file,
				   صحیح *mount_flags, حرف *mtpt, صحیح mtlen)
{
	ساخت mntent	*mnt;
	ساخت stat	st_buf;
	صحیح		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	صحیح		fd;

	*mount_flags = 0;
	اگر ((f = setmntent (mtab_file, "r")) == تہی)
		واپس errno;

	اگر (stat(file, &st_buf) == 0) {
		اگر (S_ISBLK(st_buf.st_mode)) {
#اگر_غیر_تعریف_شدہ __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#اگر_ختم	/* __GNU__ */
		} ورنہ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	جب ((mnt = getmntent (f)) != تہی) {
		اگر (mnt->mnt_fsname[0] != '/')
			جاری;
		اگر (strcmp(file, mnt->mnt_fsname) == 0)
			توڑو;
		اگر (stat(mnt->mnt_fsname, &st_buf) != 0)
			جاری;

		اگر (S_ISBLK(st_buf.st_mode)) {
#اگر_غیر_تعریف_شدہ __GNU__
			اگر (file_rdev && file_rdev == st_buf.st_rdev)
				توڑو;
#اگر_تعریف_شدہ __linux__
			/* maybe the file is loopdev backing file */
			اگر (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_مستعمل(mnt->mnt_fsname, file, 0, 0, 0))
				توڑو;
#اگر_ختم /* __linux__ */
#اگر_ختم	/* __GNU__ */
		} ورنہ {
			اگر (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				توڑو;
		}
	}

	اگر (mnt == تہی) {
#اگر_غیر_تعریف_شدہ __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		اگر (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			اگر (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			جاؤ is_جڑ;
		}
#اگر_ختم	/* __GNU__ */
		جاؤ errout;
	}
#اگر_غیر_تعریف_شدہ __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	اگر (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		اگر (retval == ENOENT) {
#اگر_تعریف_شدہ DEBUG
			لکھو("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#اگر_ختم /* DEBUG */
			retval = 0;
		}
		جاؤ errout;
	}
	اگر (file_rdev && (st_buf.st_dev != file_rdev)) {
#اگر_تعریف_شدہ DEBUG
		لکھو("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#اگر_ختم /* DEBUG */
		جاؤ errout;
	}
#اگر_ختم /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#اگر_تعریف_شدہ MNTOPT_RO
	/* Check to see if the ro option is set */
	اگر (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#اگر_ختم

	اگر (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	اگر (!strcmp(mnt->mnt_dir, "/")) {
is_جڑ:
#تعریف_کرو TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		اگر (fd < 0) {
			اگر (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ورنہ
			close(fd);
		(خالی) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	واپس retval;
}

ساکن صحیح check_mntent(ثابت حرف *file, صحیح *mount_flags,
			      حرف *mtpt, صحیح mtlen)
{
	صحیح	retval;

#اگر_تعریف_شدہ DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	اگر (retval == 0)
		واپس 0;
#اگر_ختم /* DEBUG */
#اگر_تعریف_شدہ __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	اگر (retval == 0 && (*mount_flags != 0))
		واپس 0;
	اگر (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		واپس retval;
	}
#اگر_ختم /* __linux__ */
#اگر defined(MOUNTED) || defined(_PATH_MOUNTED)
#اگر_غیر_تعریف_شدہ MOUNTED
#تعریف_کرو MOUNTED _PATH_MOUNTED
#اگر_ختم /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	واپس retval;
#ورنہ
	*mount_flags = 0;
	واپس 0;
#اگر_ختم /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ورنہ
#اگر defined(HAVE_GETMNTINFO)

ساکن صحیح check_getmntinfo(ثابت حرف *file, صحیح *mount_flags,
				  حرف *mtpt, صحیح mtlen)
{
	ساخت statfs *mp;
	صحیح    len, n;
	ثابت  حرف   *s1;
	حرف	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	اگر (n == 0)
		واپس errno;

	len = حجم(_PATH_DEV) - 1;
	s1 = file;
	اگر (لڑی_موازنہ(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	جب (--n >= 0) {
		s2 = mp->f_mntfromname;
		اگر (لڑی_موازنہ(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		اگر (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			توڑو;
		}
		++mp;
	}
	اگر (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	واپس 0;
}
#اگر_ختم /* HAVE_GETMNTINFO */
#اگر_ختم /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ساکن صحیح is_تبادلہ_device(ثابت حرف *file)
{
	FILE		*f;
	حرف		buf[1024], *cp;
	dev_t		file_dev;
	ساخت stat	st_buf;
	صحیح		ret = 0;

	file_dev = 0;
#اگر_غیر_تعریف_شدہ __GNU__ /* The GNU hurd is broken with respect to stat devices */
	اگر ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#اگر_ختم	/* __GNU__ */

	اگر (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		واپس 0;
	/* Skip the first line */
	اگر (!fgets(buf, حجم(buf), f))
		جاؤ leave;
	اگر (*buf && لڑی_موازنہ(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		جاؤ valid_پہلا_سطر;

	جب (fgets(buf, حجم(buf), f)) {
valid_پہلا_سطر:
		اگر ((cp = strchr(buf, ' ')) != تہی)
			*cp = 0;
		اگر ((cp = strchr(buf, '\t')) != تہی)
			*cp = 0;
		اگر (strcmp(buf, file) == 0) {
			ret++;
			توڑو;
		}
#اگر_غیر_تعریف_شدہ __GNU__
		اگر (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			توڑو;
		}
#اگر_ختم	/* __GNU__ */
	}

leave:
	fclose(f);
	واپس ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#اگر_تعریف_شدہ __TURBOC__
 #ہدایت argsused
#اگر_ختم
صحیح check_mount_نقطہ(ثابت حرف *device, صحیح *mount_flags,
				  حرف *mtpt, صحیح mtlen)
{
	صحیح	retval = 0;

	اگر (is_تبادلہ_device(device)) {
		*mount_flags = MF_MOUNTED | MF_تبادلہ;
		اگر (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ورنہ {
#اگر_تعریف_شدہ HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ورنہ
#اگر_تعریف_شدہ HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ورنہ
#اگر_تعریف_شدہ __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#اگر_ختم
		*mount_flags = 0;
#اگر_ختم /* HAVE_GETMNTINFO */
#اگر_ختم /* HAVE_MNTENT_H */
	}
	اگر (retval)
		واپس retval;

#اگر_تعریف_شدہ __linux__ /* This only works on Linux 2.6+ systems */
	{
		ساخت stat st_buf;
		صحیح fd;
		اگر ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			واپس 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		اگر (fd < 0) {
			اگر (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ورنہ
			close(fd);
	}
#اگر_ختم

	واپس 0;
}

صحیح is_mounted(ثابت حرف *file)
{
	صحیح	retval;
	صحیح	mount_flags = 0;

	retval = check_mount_نقطہ(file, &mount_flags, تہی, 0);
	اگر (retval)
		واپس 0;
	واپس mount_flags & MF_MOUNTED;
}

#اگر_تعریف_شدہ TEST_پروگرام_ISMOUNTED
صحیح مین(صحیح argc, حرف **argv)
{
	صحیح flags = 0;
	حرف devname[PATH_MAX];

	اگر (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		واپس EXIT_FAILURE;
	}

	اگر (check_mount_نقطہ(argv[1], &flags, devname, حجم(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		اگر (flags & MF_تبادلہ)
			لکھو("used swap device\n");
		ورنہ
			لکھو("mounted on %s\n", devname);
		واپس EXIT_SUCCESS;
	}

	لکھو("not mounted\n");
	واپس EXIT_FAILURE;
}
#اگر_ختم /* DEBUG */
