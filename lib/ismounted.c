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
#ifdef HAVE_MNTENT_H
#شامل <mntent.h>
#endif
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <ctype.h>
#شامل <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#شامل "pathnames.h"
#شامل "strutils.h"
#شامل "ismounted.h"
#شامل "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ایستا صحیح check_mntent_file(ثابت نویسه *mtab_file, ثابت نویسه *file,
				   صحیح *mount_flags, نویسه *mtpt, صحیح mtlen)
{
	ساختار mntent	*mnt;
	ساختار stat	st_buf;
	صحیح		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	صحیح		fd;

	*mount_flags = 0;
	اگر ((f = setmntent (mtab_file, "r")) == پوچ)
		بازگشت errno;

	اگر (stat(file, &st_buf) == 0) {
		اگر (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} وگرنه {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	وقتی ((mnt = getmntent (f)) != پوچ) {
		اگر (mnt->mnt_fsname[0] != '/')
			ادامه;
		اگر (strcmp(file, mnt->mnt_fsname) == 0)
			بشکن;
		اگر (stat(mnt->mnt_fsname, &st_buf) != 0)
			ادامه;

		اگر (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			اگر (file_rdev && file_rdev == st_buf.st_rdev)
				بشکن;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			اگر (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				بشکن;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} وگرنه {
			اگر (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				بشکن;
		}
	}

	اگر (mnt == پوچ) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
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
			برو is_ریشه;
		}
#endif	/* __GNU__ */
		برو errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	اگر (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		اگر (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		برو errout;
	}
	اگر (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		برو errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	اگر (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	اگر (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	اگر (!strcmp(mnt->mnt_dir, "/")) {
is_ریشه:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		اگر (fd < 0) {
			اگر (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} وگرنه
			close(fd);
		(تهی) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	بازگشت retval;
}

ایستا صحیح check_mntent(ثابت نویسه *file, صحیح *mount_flags,
			      نویسه *mtpt, صحیح mtlen)
{
	صحیح	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	اگر (retval == 0)
		بازگشت 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	اگر (retval == 0 && (*mount_flags != 0))
		بازگشت 0;
	اگر (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		بازگشت retval;
	}
#endif /* __linux__ */
#اگر defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	بازگشت retval;
#وگرنه
	*mount_flags = 0;
	بازگشت 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#وگرنه
#اگر defined(HAVE_GETMNTINFO)

ایستا صحیح check_getmntinfo(ثابت نویسه *file, صحیح *mount_flags,
				  نویسه *mtpt, صحیح mtlen)
{
	ساختار statfs *mp;
	صحیح    len, n;
	ثابت  نویسه   *s1;
	نویسه	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	اگر (n == 0)
		بازگشت errno;

	len = اندازه(_PATH_DEV) - 1;
	s1 = file;
	اگر (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	وقتی (--n >= 0) {
		s2 = mp->f_mntfromname;
		اگر (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		اگر (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			بشکن;
		}
		++mp;
	}
	اگر (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	بازگشت 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ایستا صحیح is_swap_device(ثابت نویسه *file)
{
	FILE		*f;
	نویسه		buf[1024], *cp;
	dev_t		file_dev;
	ساختار stat	st_buf;
	صحیح		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	اگر ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	اگر (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		بازگشت 0;
	/* Skip the first line */
	اگر (!fgets(buf, اندازه(buf), f))
		برو leave;
	اگر (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		برو valid_first_سطر;

	وقتی (fgets(buf, اندازه(buf), f)) {
valid_first_سطر:
		اگر ((cp = strchr(buf, ' ')) != پوچ)
			*cp = 0;
		اگر ((cp = strchr(buf, '\t')) != پوچ)
			*cp = 0;
		اگر (strcmp(buf, file) == 0) {
			ret++;
			بشکن;
		}
#ifndef __GNU__
		اگر (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			بشکن;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	بازگشت ret;
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
صحیح check_mount_point(ثابت نویسه *device, صحیح *mount_flags,
				  نویسه *mtpt, صحیح mtlen)
{
	صحیح	retval = 0;

	اگر (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		اگر (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} وگرنه {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#وگرنه
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#وگرنه
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	اگر (retval)
		بازگشت retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		ساختار stat st_buf;
		صحیح fd;
		اگر ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			بازگشت 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		اگر (fd < 0) {
			اگر (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} وگرنه
			close(fd);
	}
#endif

	بازگشت 0;
}

صحیح is_mounted(ثابت نویسه *file)
{
	صحیح	retval;
	صحیح	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, پوچ, 0);
	اگر (retval)
		بازگشت 0;
	بازگشت mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح flags = 0;
	نویسه devname[PATH_MAX];

	اگر (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		بازگشت EXIT_FAILURE;
	}

	اگر (check_mount_point(argv[1], &flags, devname, اندازه(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		اگر (flags & MF_SWAP)
			printf("used swap device\n");
		وگرنه
			printf("mounted on %s\n", devname);
		بازگشت EXIT_SUCCESS;
	}

	printf("not mounted\n");
	بازگشت EXIT_FAILURE;
}
#endif /* DEBUG */
