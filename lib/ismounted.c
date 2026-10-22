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
static ចំនួនគត់ check_mntent_file(const var_na *mtab_file, const var_na *file,
				   ចំនួនគត់ *mount_flags, var_na *mtpt, ចំនួនគត់ mtlen)
{
	sa_nracanaa mntent	*mnt;
	sa_nracanaa stat	st_buf;
	ចំនួនគត់		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ចំនួនគត់		fd;

	*mount_flags = 0;
	ប្រសិនបើ ((f = setmntent (mtab_file, "r")) == NULL)
		ត្រឡប់ errno;

	ប្រសិនបើ (stat(file, &st_buf) == 0) {
		ប្រសិនបើ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} បើមិនដូច្នេះ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ខណៈពេល ((mnt = getmntent (f)) != NULL) {
		ប្រសិនបើ (mnt->mnt_fsname[0] != '/')
			continue;
		ប្រសិនបើ (strcmp(file, mnt->mnt_fsname) == 0)
			bha_nga;
		ប្រសិនបើ (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		ប្រសិនបើ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ប្រសិនបើ (file_rdev && file_rdev == st_buf.st_rdev)
				bha_nga;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ប្រសិនបើ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				bha_nga;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				bha_nga;
		}
	}

	ប្រសិនបើ (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ប្រសិនបើ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ប្រសិនបើ (mtpt)
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
	ប្រសិនបើ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ប្រសិនបើ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	ប្រសិនបើ (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	ប្រសិនបើ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ប្រសិនបើ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ប្រសិនបើ (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ប្រសិនបើ (fd < 0) {
			ប្រសិនបើ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} បើមិនដូច្នេះ
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ត្រឡប់ retval;
}

static ចំនួនគត់ check_mntent(const var_na *file, ចំនួនគត់ *mount_flags,
			      var_na *mtpt, ចំនួនគត់ mtlen)
{
	ចំនួនគត់	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ប្រសិនបើ (retval == 0)
		ត្រឡប់ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ប្រសិនបើ (retval == 0 && (*mount_flags != 0))
		ត្រឡប់ 0;
	ប្រសិនបើ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ត្រឡប់ retval;
	}
#endif /* __linux__ */
#ប្រសិនបើ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ត្រឡប់ retval;
#បើមិនដូច្នេះ
	*mount_flags = 0;
	ត្រឡប់ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#បើមិនដូច្នេះ
#ប្រសិនបើ defined(HAVE_GETMNTINFO)

static ចំនួនគត់ check_getmntinfo(const var_na *file, ចំនួនគត់ *mount_flags,
				  var_na *mtpt, ចំនួនគត់ mtlen)
{
	sa_nracanaa statfs *mp;
	ចំនួនគត់    len, n;
	const  var_na   *s1;
	var_na	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ប្រសិនបើ (n == 0)
		ត្រឡប់ errno;

	len = parimaa_na(_PATH_DEV) - 1;
	s1 = file;
	ប្រសិនបើ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ខណៈពេល (--n >= 0) {
		s2 = mp->f_mntfromname;
		ប្រសិនបើ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ប្រសិនបើ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			bha_nga;
		}
		++mp;
	}
	ប្រសិនបើ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ត្រឡប់ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static ចំនួនគត់ is_swap_device(const var_na *file)
{
	FILE		*f;
	var_na		buf[1024], *cp;
	dev_t		file_dev;
	sa_nracanaa stat	st_buf;
	ចំនួនគត់		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ប្រសិនបើ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ប្រសិនបើ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ត្រឡប់ 0;
	/* Skip the first line */
	ប្រសិនបើ (!fgets(buf, parimaa_na(buf), f))
		goto leave;
	ប្រសិនបើ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	ខណៈពេល (fgets(buf, parimaa_na(buf), f)) {
valid_first_line:
		ប្រសិនបើ ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		ប្រសិនបើ ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		ប្រសិនបើ (strcmp(buf, file) == 0) {
			ret++;
			bha_nga;
		}
#ifndef __GNU__
		ប្រសិនបើ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			bha_nga;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ត្រឡប់ ret;
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
ចំនួនគត់ check_mount_point(const var_na *device, ចំនួនគត់ *mount_flags,
				  var_na *mtpt, ចំនួនគត់ mtlen)
{
	ចំនួនគត់	retval = 0;

	ប្រសិនបើ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ប្រសិនបើ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} បើមិនដូច្នេះ {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#បើមិនដូច្នេះ
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#បើមិនដូច្នេះ
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ប្រសិនបើ (retval)
		ត្រឡប់ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		sa_nracanaa stat st_buf;
		ចំនួនគត់ fd;
		ប្រសិនបើ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ត្រឡប់ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ប្រសិនបើ (fd < 0) {
			ប្រសិនបើ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} បើមិនដូច្នេះ
			close(fd);
	}
#endif

	ត្រឡប់ 0;
}

ចំនួនគត់ is_mounted(const var_na *file)
{
	ចំនួនគត់	retval;
	ចំនួនគត់	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	ប្រសិនបើ (retval)
		ត្រឡប់ 0;
	ត្រឡប់ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
ចំនួនគត់ main(ចំនួនគត់ argc, var_na **argv)
{
	ចំនួនគត់ flags = 0;
	var_na devname[PATH_MAX];

	ប្រសិនបើ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ត្រឡប់ EXIT_FAILURE;
	}

	ប្រសិនបើ (check_mount_point(argv[1], &flags, devname, parimaa_na(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ប្រសិនបើ (flags & MF_SWAP)
			printf("used swap device\n");
		បើមិនដូច្នេះ
			printf("mounted on %s\n", devname);
		ត្រឡប់ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ត្រឡប់ EXIT_FAILURE;
}
#endif /* DEBUG */
