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
#ifdef HAVE_MNTENT_H
#ضمن <mntent.h>
#endif
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <ctype.h>
#ضمن <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#ضمن "pathnames.h"
#ضمن "strutils.h"
#ضمن "ismounted.h"
#ضمن "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
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
	ܐܢ ((f = setmntent (mtab_file, "r")) == عدم)
		ܦܢܐ errno;

	ܐܢ (stat(file, &st_buf) == 0) {
		ܐܢ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} ܐܚܪܢܐ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ܟܡܐ ((mnt = getmntent (f)) != عدم) {
		ܐܢ (mnt->mnt_fsname[0] != '/')
			استمر;
		ܐܢ (strcmp(file, mnt->mnt_fsname) == 0)
			اكسر;
		ܐܢ (stat(mnt->mnt_fsname, &st_buf) != 0)
			استمر;

		ܐܢ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ܐܢ (file_rdev && file_rdev == st_buf.st_rdev)
				اكسر;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ܐܢ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				اكسر;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} ܐܚܪܢܐ {
			ܐܢ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				اكسر;
		}
	}

	ܐܢ (mnt == عدم) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ܐܢ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ܐܢ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			اذهب is_root;
		}
#endif	/* __GNU__ */
		اذهب errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ܐܢ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ܐܢ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		اذهب errout;
	}
	ܐܢ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		اذهب errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	ܐܢ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ܐܢ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ܐܢ (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ܐܢ (fd < 0) {
			ܐܢ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ܐܚܪܢܐ
			close(fd);
		(فراغ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ܦܢܐ retval;
}

ساكن صحيح check_mntent(ثابت حرف *file, صحيح *mount_flags,
			      حرف *mtpt, صحيح mtlen)
{
	صحيح	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ܐܢ (retval == 0)
		ܦܢܐ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ܐܢ (retval == 0 && (*mount_flags != 0))
		ܦܢܐ 0;
	ܐܢ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ܦܢܐ retval;
	}
#endif /* __linux__ */
#ܐܢ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ܦܢܐ retval;
#ܐܚܪܢܐ
	*mount_flags = 0;
	ܦܢܐ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ܐܚܪܢܐ
#ܐܢ defined(HAVE_GETMNTINFO)

ساكن صحيح check_getmntinfo(ثابت حرف *file, صحيح *mount_flags,
				  حرف *mtpt, صحيح mtlen)
{
	بنية statfs *mp;
	صحيح    len, n;
	ثابت  حرف   *s1;
	حرف	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ܐܢ (n == 0)
		ܦܢܐ errno;

	len = حجم(_PATH_DEV) - 1;
	s1 = file;
	ܐܢ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ܟܡܐ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ܐܢ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ܐܢ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			اكسر;
		}
		++mp;
	}
	ܐܢ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ܦܢܐ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ساكن صحيح is_swap_device(ثابت حرف *file)
{
	FILE		*f;
	حرف		buf[1024], *cp;
	dev_t		file_dev;
	بنية stat	st_buf;
	صحيح		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ܐܢ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ܐܢ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ܦܢܐ 0;
	/* Skip the first line */
	ܐܢ (!fgets(buf, حجم(buf), f))
		اذهب leave;
	ܐܢ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		اذهب valid_first_line;

	ܟܡܐ (fgets(buf, حجم(buf), f)) {
valid_first_line:
		ܐܢ ((cp = strchr(buf, ' ')) != عدم)
			*cp = 0;
		ܐܢ ((cp = strchr(buf, '\t')) != عدم)
			*cp = 0;
		ܐܢ (strcmp(buf, file) == 0) {
			ret++;
			اكسر;
		}
#ifndef __GNU__
		ܐܢ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			اكسر;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ܦܢܐ ret;
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
صحيح check_mount_point(ثابت حرف *device, صحيح *mount_flags,
				  حرف *mtpt, صحيح mtlen)
{
	صحيح	retval = 0;

	ܐܢ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ܐܢ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ܐܚܪܢܐ {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ܐܚܪܢܐ
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ܐܚܪܢܐ
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ܐܢ (retval)
		ܦܢܐ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		بنية stat st_buf;
		صحيح fd;
		ܐܢ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ܦܢܐ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ܐܢ (fd < 0) {
			ܐܢ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ܐܚܪܢܐ
			close(fd);
	}
#endif

	ܦܢܐ 0;
}

صحيح is_mounted(ثابت حرف *file)
{
	صحيح	retval;
	صحيح	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, عدم, 0);
	ܐܢ (retval)
		ܦܢܐ 0;
	ܦܢܐ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
صحيح main(صحيح argc, حرف **argv)
{
	صحيح flags = 0;
	حرف devname[PATH_MAX];

	ܐܢ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ܦܢܐ EXIT_FAILURE;
	}

	ܐܢ (check_mount_point(argv[1], &flags, devname, حجم(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ܐܢ (flags & MF_SWAP)
			printf("used swap device\n");
		ܐܚܪܢܐ
			printf("mounted on %s\n", devname);
		ܦܢܐ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ܦܢܐ EXIT_FAILURE;
}
#endif /* DEBUG */
