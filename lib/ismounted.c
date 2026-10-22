/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#ضمن <stdio.h>
#ضمن <unistd.h>
#ضمن <stdlib.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#که_تعریف_شوی HAVE_MNTENT_H
#ضمن <mntent.h>
#د_که_پای
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <ctype.h>
#ضمن <sys/param.h>

#که_نه_تعریف_شوی __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#د_که_پای

#ضمن "pathnames.h"
#ضمن "strutils.h"
#ضمن "ismounted.h"
#ضمن "c.h"
#که_تعریف_شوی __linux__
# include "loopdev.h"
#د_که_پای



#که_تعریف_شوی HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ساكن صحيح check_mntent_file(ثابت حرف *mtab_file, ثابت حرف *file,
				   صحيح *mount_flags, حرف *mtpt, صحيح mtlen)
{
	بنية mntent	*mnt;
	بنية stat	st_buf;
	صحيح		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	صحيح		fd;

	*mount_flags = 0;
	إذا ((f = setmntent (mtab_file, "r")) == عدم)
		أرجع errno;

	إذا (stat(file, &st_buf) == 0) {
		إذا (S_ISBLK(st_buf.st_mode)) {
#که_نه_تعریف_شوی __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#د_که_پای	/* __GNU__ */
		} وإلا {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	طالما ((mnt = getmntent (f)) != عدم) {
		إذا (mnt->mnt_fsname[0] != '/')
			استمر;
		إذا (strcmp(file, mnt->mnt_fsname) == 0)
			اكسر;
		إذا (stat(mnt->mnt_fsname, &st_buf) != 0)
			استمر;

		إذا (S_ISBLK(st_buf.st_mode)) {
#که_نه_تعریف_شوی __GNU__
			إذا (file_rdev && file_rdev == st_buf.st_rdev)
				اكسر;
#که_تعریف_شوی __linux__
			/* maybe the file is loopdev backing file */
			إذا (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_کارول_شوی(mnt->mnt_fsname, file, 0, 0, 0))
				اكسر;
#د_که_پای /* __linux__ */
#د_که_پای	/* __GNU__ */
		} وإلا {
			إذا (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				اكسر;
		}
	}

	إذا (mnt == عدم) {
#که_نه_تعریف_شوی __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		إذا (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			إذا (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			اذهب is_ریښه;
		}
#د_که_پای	/* __GNU__ */
		اذهب errout;
	}
#که_نه_تعریف_شوی __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	إذا (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		إذا (retval == ENOENT) {
#که_تعریف_شوی DEBUG
			چاپ_کړه("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#د_که_پای /* DEBUG */
			retval = 0;
		}
		اذهب errout;
	}
	إذا (file_rdev && (st_buf.st_dev != file_rdev)) {
#که_تعریف_شوی DEBUG
		چاپ_کړه("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#د_که_پای /* DEBUG */
		اذهب errout;
	}
#د_که_پای /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#که_تعریف_شوی MNTOPT_RO
	/* Check to see if the ro option is set */
	إذا (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#د_که_پای

	إذا (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	إذا (!strcmp(mnt->mnt_dir, "/")) {
is_ریښه:
#تعریف_کړه TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		إذا (fd < 0) {
			إذا (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} وإلا
			close(fd);
		(فراغ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	أرجع retval;
}

ساكن صحيح check_mntent(ثابت حرف *file, صحيح *mount_flags,
			      حرف *mtpt, صحيح mtlen)
{
	صحيح	retval;

#که_تعریف_شوی DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	إذا (retval == 0)
		أرجع 0;
#د_که_پای /* DEBUG */
#که_تعریف_شوی __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	إذا (retval == 0 && (*mount_flags != 0))
		أرجع 0;
	إذا (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		أرجع retval;
	}
#د_که_پای /* __linux__ */
#إذا defined(MOUNTED) || defined(_PATH_MOUNTED)
#که_نه_تعریف_شوی MOUNTED
#تعریف_کړه MOUNTED _PATH_MOUNTED
#د_که_پای /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	أرجع retval;
#وإلا
	*mount_flags = 0;
	أرجع 0;
#د_که_پای /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#وإلا
#إذا defined(HAVE_GETMNTINFO)

ساكن صحيح check_getmntinfo(ثابت حرف *file, صحيح *mount_flags,
				  حرف *mtpt, صحيح mtlen)
{
	بنية statfs *mp;
	صحيح    len, n;
	ثابت  حرف   *s1;
	حرف	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	إذا (n == 0)
		أرجع errno;

	len = حجم(_PATH_DEV) - 1;
	s1 = file;
	إذا (مزي_پرتله(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	طالما (--n >= 0) {
		s2 = mp->f_mntfromname;
		إذا (مزي_پرتله(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		إذا (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			اكسر;
		}
		++mp;
	}
	إذا (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	أرجع 0;
}
#د_که_پای /* HAVE_GETMNTINFO */
#د_که_پای /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ساكن صحيح is_بدلون_device(ثابت حرف *file)
{
	FILE		*f;
	حرف		buf[1024], *cp;
	dev_t		file_dev;
	بنية stat	st_buf;
	صحيح		ret = 0;

	file_dev = 0;
#که_نه_تعریف_شوی __GNU__ /* The GNU hurd is broken with respect to stat devices */
	إذا ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#د_که_پای	/* __GNU__ */

	إذا (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		أرجع 0;
	/* Skip the first line */
	إذا (!fgets(buf, حجم(buf), f))
		اذهب leave;
	إذا (*buf && مزي_پرتله(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		اذهب valid_لومړی_کرښه;

	طالما (fgets(buf, حجم(buf), f)) {
valid_لومړی_کرښه:
		إذا ((cp = strchr(buf, ' ')) != عدم)
			*cp = 0;
		إذا ((cp = strchr(buf, '\t')) != عدم)
			*cp = 0;
		إذا (strcmp(buf, file) == 0) {
			ret++;
			اكسر;
		}
#که_نه_تعریف_شوی __GNU__
		إذا (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			اكسر;
		}
#د_که_پای	/* __GNU__ */
	}

leave:
	fclose(f);
	أرجع ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#که_تعریف_شوی __TURBOC__
 #لارښوونه argsused
#د_که_پای
صحيح check_mount_ټکی(ثابت حرف *device, صحيح *mount_flags,
				  حرف *mtpt, صحيح mtlen)
{
	صحيح	retval = 0;

	إذا (is_بدلون_device(device)) {
		*mount_flags = MF_MOUNTED | MF_بدلون;
		إذا (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} وإلا {
#که_تعریف_شوی HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#وإلا
#که_تعریف_شوی HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#وإلا
#که_تعریف_شوی __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#د_که_پای
		*mount_flags = 0;
#د_که_پای /* HAVE_GETMNTINFO */
#د_که_پای /* HAVE_MNTENT_H */
	}
	إذا (retval)
		أرجع retval;

#که_تعریف_شوی __linux__ /* This only works on Linux 2.6+ systems */
	{
		بنية stat st_buf;
		صحيح fd;
		إذا ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			أرجع 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		إذا (fd < 0) {
			إذا (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} وإلا
			close(fd);
	}
#د_که_پای

	أرجع 0;
}

صحيح is_mounted(ثابت حرف *file)
{
	صحيح	retval;
	صحيح	mount_flags = 0;

	retval = check_mount_ټکی(file, &mount_flags, عدم, 0);
	إذا (retval)
		أرجع 0;
	أرجع mount_flags & MF_MOUNTED;
}

#که_تعریف_شوی TEST_پروګرام_ISMOUNTED
صحيح اصلي(صحيح argc, حرف **argv)
{
	صحيح flags = 0;
	حرف devname[PATH_MAX];

	إذا (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		أرجع EXIT_FAILURE;
	}

	إذا (check_mount_ټکی(argv[1], &flags, devname, حجم(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		إذا (flags & MF_بدلون)
			چاپ_کړه("used swap device\n");
		وإلا
			چاپ_کړه("mounted on %s\n", devname);
		أرجع EXIT_SUCCESS;
	}

	چاپ_کړه("not mounted\n");
	أرجع EXIT_FAILURE;
}
#د_که_پای /* DEBUG */
