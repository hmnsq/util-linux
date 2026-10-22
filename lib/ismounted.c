/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ifdef HAVE_MNTENT_H
#ଅନ୍ତର୍ଭାଵ <mntent.h>
#endif
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "ismounted.h"
#ଅନ୍ତର୍ଭାଵ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_mntent_file(ଅଚଳ ଅକ୍ଷର *mtab_file, ଅଚଳ ଅକ୍ଷର *file,
				   ପୂର୍ଣ୍ଣସଂଖ୍ୟା *mount_flags, ଅକ୍ଷର *mtpt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mtlen)
{
	ସଂରଚନା mntent	*mnt;
	ସଂରଚନା stat	st_buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		fd;

	*mount_flags = 0;
	ଯଦି ((f = setmntent (mtab_file, "r")) == ଖାଲି)
		ଫେରନ୍ତୁ errno;

	ଯଦି (stat(file, &st_buf) == 0) {
		ଯଦି (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} ଅନ୍ୟଥା {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ଯେତେବେଳେ ((mnt = getmntent (f)) != ଖାଲି) {
		ଯଦି (mnt->mnt_fsname[0] != '/')
			ଜାରି;
		ଯଦି (strcmp(file, mnt->mnt_fsname) == 0)
			ଭାଙ୍ଗ;
		ଯଦି (stat(mnt->mnt_fsname, &st_buf) != 0)
			ଜାରି;

		ଯଦି (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			ଯଦି (file_rdev && file_rdev == st_buf.st_rdev)
				ଭାଙ୍ଗ;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			ଯଦି (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				ଭାଙ୍ଗ;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} ଅନ୍ୟଥା {
			ଯଦି (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ଭାଙ୍ଗ;
		}
	}

	ଯଦି (mnt == ଖାଲି) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ଯଦି (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ଯଦି (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			ଯାଅ is_ମୂଳ;
		}
#endif	/* __GNU__ */
		ଯାଅ errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ଯଦି (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ଯଦି (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		ଯାଅ errout;
	}
	ଯଦି (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		ଯାଅ errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	ଯଦି (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	ଯଦି (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ଯଦି (!strcmp(mnt->mnt_dir, "/")) {
is_ମୂଳ:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ଯଦି (fd < 0) {
			ଯଦି (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} ଅନ୍ୟଥା
			close(fd);
		(ଶୂନ୍ୟ) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_mntent(ଅଚଳ ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *mount_flags,
			      ଅକ୍ଷର *mtpt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mtlen)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ଯଦି (retval == 0)
		ଫେରନ୍ତୁ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ଯଦି (retval == 0 && (*mount_flags != 0))
		ଫେରନ୍ତୁ 0;
	ଯଦି (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		ଫେରନ୍ତୁ retval;
	}
#endif /* __linux__ */
#ଯଦି defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	ଫେରନ୍ତୁ retval;
#ଅନ୍ୟଥା
	*mount_flags = 0;
	ଫେରନ୍ତୁ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#ଅନ୍ୟଥା
#ଯଦି defined(HAVE_GETMNTINFO)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_getmntinfo(ଅଚଳ ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *mount_flags,
				  ଅକ୍ଷର *mtpt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mtlen)
{
	ସଂରଚନା statfs *mp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା    len, n;
	ଅଚଳ  ଅକ୍ଷର   *s1;
	ଅକ୍ଷର	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ଯଦି (n == 0)
		ଫେରନ୍ତୁ errno;

	len = ଆକାର(_PATH_DEV) - 1;
	s1 = file;
	ଯଦି (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ଯେତେବେଳେ (--n >= 0) {
		s2 = mp->f_mntfromname;
		ଯଦି (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ଯଦି (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ଭାଙ୍ଗ;
		}
		++mp;
	}
	ଯଦି (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	ଫେରନ୍ତୁ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_swap_device(ଅଚଳ ଅକ୍ଷର *file)
{
	FILE		*f;
	ଅକ୍ଷର		buf[1024], *cp;
	dev_t		file_dev;
	ସଂରଚନା stat	st_buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ଯଦି ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	ଯଦି (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		ଫେରନ୍ତୁ 0;
	/* Skip the first line */
	ଯଦି (!fgets(buf, ଆକାର(buf), f))
		ଯାଅ leave;
	ଯଦି (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		ଯାଅ valid_first_ଧାଡ଼ି;

	ଯେତେବେଳେ (fgets(buf, ଆକାର(buf), f)) {
valid_first_ଧାଡ଼ି:
		ଯଦି ((cp = strchr(buf, ' ')) != ଖାଲି)
			*cp = 0;
		ଯଦି ((cp = strchr(buf, '\t')) != ଖାଲି)
			*cp = 0;
		ଯଦି (strcmp(buf, file) == 0) {
			ret++;
			ଭାଙ୍ଗ;
		}
#ifndef __GNU__
		ଯଦି (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ଭାଙ୍ଗ;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	ଫେରନ୍ତୁ ret;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_mount_point(ଅଚଳ ଅକ୍ଷର *device, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *mount_flags,
				  ଅକ୍ଷର *mtpt, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mtlen)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	retval = 0;

	ଯଦି (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		ଯଦି (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} ଅନ୍ୟଥା {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#ଅନ୍ୟଥା
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#ଅନ୍ୟଥା
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	ଯଦି (retval)
		ଫେରନ୍ତୁ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		ସଂରଚନା stat st_buf;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
		ଯଦି ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			ଫେରନ୍ତୁ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ଯଦି (fd < 0) {
			ଯଦି (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} ଅନ୍ୟଥା
			close(fd);
	}
#endif

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_mounted(ଅଚଳ ଅକ୍ଷର *file)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	retval;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, ଖାଲି, 0);
	ଯଦି (retval)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;
	ଅକ୍ଷର devname[PATH_MAX];

	ଯଦି (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଯଦି (check_mount_point(argv[1], &flags, devname, ଆକାର(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ଯଦି (flags & MF_SWAP)
			printf("used swap device\n");
		ଅନ୍ୟଥା
			printf("mounted on %s\n", devname);
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif /* DEBUG */
