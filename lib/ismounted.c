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
static puur_naa_nka check_mntent_file(const var_na *mtab_file, const var_na *file,
				   puur_naa_nka *mount_flags, var_na *mtpt, puur_naa_nka mtlen)
{
	sa_nracanaa mntent	*mnt;
	sa_nracanaa stat	st_buf;
	puur_naa_nka		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	puur_naa_nka		fd;

	*mount_flags = 0;
	ᬬᬾᬦ᭄ ((f = setmntent (mtab_file, "r")) == NULL)
		ᬯᬮᬶ errno;

	ᬬᬾᬦ᭄ (stat(file, &st_buf) == 0) {
		ᬬᬾᬦ᭄ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} ᬮᬶᬬ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ᬲᭂᬤᭂᬮ ((mnt = getmntent (f)) != NULL) {
		ᬬᬾᬦ᭄ (mnt->mnt_fsname[0] != '/')
			continue;
		ᬬᬾᬦ᭄ (strcmp(file, mnt->mnt_fsname) == 0)
			bha_nga;
		ᬬᬾᬦ᭄ (stat(mnt->mnt_fsname, &st_buf) != 0)
			continue;

		ᬬᬾᬦ᭄ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ᬬᬾᬦ᭄ (file_rdev && file_rdev == st_buf.st_rdev)
				bha_nga;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ᬬᬾᬦ᭄ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				bha_nga;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} ᬮᬶᬬ {
			ᬬᬾᬦ᭄ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				bha_nga;
		}
	}

	ᬬᬾᬦ᭄ (mnt == NULL) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ᬬᬾᬦ᭄ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ᬬᬾᬦ᭄ (mtpt)
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
	ᬬᬾᬦ᭄ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ᬬᬾᬦ᭄ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		goto errout;
	}
	ᬬᬾᬦ᭄ (file_rdev && (st_buf.st_dev != file_rdev)) {
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
	ᬬᬾᬦ᭄ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ᬬᬾᬦ᭄ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ᬬᬾᬦ᭄ (!strcmp(mnt->mnt_dir, "/")) {
is_root:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ᬬᬾᬦ᭄ (fd < 0) {
			ᬬᬾᬦ᭄ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ᬮᬶᬬ
			close(fd);
		(void) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ᬯᬮᬶ retval;
}

static puur_naa_nka check_mntent(const var_na *file, puur_naa_nka *mount_flags,
			      var_na *mtpt, puur_naa_nka mtlen)
{
	puur_naa_nka	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ᬬᬾᬦ᭄ (retval == 0)
		ᬯᬮᬶ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ᬬᬾᬦ᭄ (retval == 0 && (*mount_flags != 0))
		ᬯᬮᬶ 0;
	ᬬᬾᬦ᭄ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ᬯᬮᬶ retval;
	}
#endif /* __linux__ */
#ᬬᬾᬦ᭄ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ᬯᬮᬶ retval;
#ᬮᬶᬬ
	*mount_flags = 0;
	ᬯᬮᬶ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ᬮᬶᬬ
#ᬬᬾᬦ᭄ defined(HAVE_GETMNTINFO)

static puur_naa_nka check_getmntinfo(const var_na *file, puur_naa_nka *mount_flags,
				  var_na *mtpt, puur_naa_nka mtlen)
{
	sa_nracanaa statfs *mp;
	puur_naa_nka    len, n;
	const  var_na   *s1;
	var_na	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ᬬᬾᬦ᭄ (n == 0)
		ᬯᬮᬶ errno;

	len = parimaa_na(_PATH_DEV) - 1;
	s1 = file;
	ᬬᬾᬦ᭄ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ᬲᭂᬤᭂᬮ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ᬬᬾᬦ᭄ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ᬬᬾᬦ᭄ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			bha_nga;
		}
		++mp;
	}
	ᬬᬾᬦ᭄ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ᬯᬮᬶ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
static puur_naa_nka is_swap_device(const var_na *file)
{
	FILE		*f;
	var_na		buf[1024], *cp;
	dev_t		file_dev;
	sa_nracanaa stat	st_buf;
	puur_naa_nka		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ᬬᬾᬦ᭄ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ᬬᬾᬦ᭄ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ᬯᬮᬶ 0;
	/* Skip the first line */
	ᬬᬾᬦ᭄ (!fgets(buf, parimaa_na(buf), f))
		goto leave;
	ᬬᬾᬦ᭄ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		goto valid_first_line;

	ᬲᭂᬤᭂᬮ (fgets(buf, parimaa_na(buf), f)) {
valid_first_line:
		ᬬᬾᬦ᭄ ((cp = strchr(buf, ' ')) != NULL)
			*cp = 0;
		ᬬᬾᬦ᭄ ((cp = strchr(buf, '\t')) != NULL)
			*cp = 0;
		ᬬᬾᬦ᭄ (strcmp(buf, file) == 0) {
			ret++;
			bha_nga;
		}
#ifndef __GNU__
		ᬬᬾᬦ᭄ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			bha_nga;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ᬯᬮᬶ ret;
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
puur_naa_nka check_mount_point(const var_na *device, puur_naa_nka *mount_flags,
				  var_na *mtpt, puur_naa_nka mtlen)
{
	puur_naa_nka	retval = 0;

	ᬬᬾᬦ᭄ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ᬬᬾᬦ᭄ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ᬮᬶᬬ {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ᬮᬶᬬ
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ᬮᬶᬬ
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ᬬᬾᬦ᭄ (retval)
		ᬯᬮᬶ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		sa_nracanaa stat st_buf;
		puur_naa_nka fd;
		ᬬᬾᬦ᭄ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ᬯᬮᬶ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ᬬᬾᬦ᭄ (fd < 0) {
			ᬬᬾᬦ᭄ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ᬮᬶᬬ
			close(fd);
	}
#endif

	ᬯᬮᬶ 0;
}

puur_naa_nka is_mounted(const var_na *file)
{
	puur_naa_nka	retval;
	puur_naa_nka	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, NULL, 0);
	ᬬᬾᬦ᭄ (retval)
		ᬯᬮᬶ 0;
	ᬯᬮᬶ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka flags = 0;
	var_na devname[PATH_MAX];

	ᬬᬾᬦ᭄ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ᬯᬮᬶ EXIT_FAILURE;
	}

	ᬬᬾᬦ᭄ (check_mount_point(argv[1], &flags, devname, parimaa_na(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ᬬᬾᬦ᭄ (flags & MF_SWAP)
			printf("used swap device\n");
		ᬮᬶᬬ
			printf("mounted on %s\n", devname);
		ᬯᬮᬶ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ᬯᬮᬶ EXIT_FAILURE;
}
#endif /* DEBUG */
