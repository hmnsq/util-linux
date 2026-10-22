/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#ifdef HAVE_MNTENT_H
#அந்தர்பாவ <mntent.h>
#endif
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "ismounted.h"
#அந்தர்பாவ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
நிலையான முழு check_mntent_file(மாறிலி எழுத்து *mtab_file, மாறிலி எழுத்து *file,
				   முழு *mount_flags, எழுத்து *mtpt, முழு mtlen)
{
	கட்டமைப்பு mntent	*mnt;
	கட்டமைப்பு stat	st_buf;
	முழு		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	முழு		fd;

	*mount_flags = 0;
	எனில் ((f = setmntent (mtab_file, "r")) == வெற்று)
		திரும்பு errno;

	எனில் (stat(file, &st_buf) == 0) {
		எனில் (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} இல்லையெனில் {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	போது ((mnt = getmntent (f)) != வெற்று) {
		எனில் (mnt->mnt_fsname[0] != '/')
			தொடர்;
		எனில் (strcmp(file, mnt->mnt_fsname) == 0)
			நிறுத்து;
		எனில் (stat(mnt->mnt_fsname, &st_buf) != 0)
			தொடர்;

		எனில் (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			எனில் (file_rdev && file_rdev == st_buf.st_rdev)
				நிறுத்து;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			எனில் (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				நிறுத்து;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} இல்லையெனில் {
			எனில் (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				நிறுத்து;
		}
	}

	எனில் (mnt == வெற்று) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		எனில் (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			எனில் (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			செல் is_வேர்;
		}
#endif	/* __GNU__ */
		செல் errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	எனில் (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		எனில் (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		செல் errout;
	}
	எனில் (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		செல் errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	எனில் (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	எனில் (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	எனில் (!strcmp(mnt->mnt_dir, "/")) {
is_வேர்:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		எனில் (fd < 0) {
			எனில் (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} இல்லையெனில்
			close(fd);
		(வெறுமை) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	திரும்பு retval;
}

நிலையான முழு check_mntent(மாறிலி எழுத்து *file, முழு *mount_flags,
			      எழுத்து *mtpt, முழு mtlen)
{
	முழு	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	எனில் (retval == 0)
		திரும்பு 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	எனில் (retval == 0 && (*mount_flags != 0))
		திரும்பு 0;
	எனில் (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		திரும்பு retval;
	}
#endif /* __linux__ */
#எனில் defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	திரும்பு retval;
#இல்லையெனில்
	*mount_flags = 0;
	திரும்பு 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#இல்லையெனில்
#எனில் defined(HAVE_GETMNTINFO)

நிலையான முழு check_getmntinfo(மாறிலி எழுத்து *file, முழு *mount_flags,
				  எழுத்து *mtpt, முழு mtlen)
{
	கட்டமைப்பு statfs *mp;
	முழு    len, n;
	மாறிலி  எழுத்து   *s1;
	எழுத்து	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	எனில் (n == 0)
		திரும்பு errno;

	len = அளவு(_PATH_DEV) - 1;
	s1 = file;
	எனில் (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	போது (--n >= 0) {
		s2 = mp->f_mntfromname;
		எனில் (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		எனில் (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			நிறுத்து;
		}
		++mp;
	}
	எனில் (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	திரும்பு 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
நிலையான முழு is_swap_device(மாறிலி எழுத்து *file)
{
	FILE		*f;
	எழுத்து		buf[1024], *cp;
	dev_t		file_dev;
	கட்டமைப்பு stat	st_buf;
	முழு		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	எனில் ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	எனில் (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		திரும்பு 0;
	/* Skip the first line */
	எனில் (!fgets(buf, அளவு(buf), f))
		செல் leave;
	எனில் (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		செல் valid_first_வரி;

	போது (fgets(buf, அளவு(buf), f)) {
valid_first_வரி:
		எனில் ((cp = strchr(buf, ' ')) != வெற்று)
			*cp = 0;
		எனில் ((cp = strchr(buf, '\t')) != வெற்று)
			*cp = 0;
		எனில் (strcmp(buf, file) == 0) {
			ret++;
			நிறுத்து;
		}
#ifndef __GNU__
		எனில் (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			நிறுத்து;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	திரும்பு ret;
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
முழு check_mount_point(மாறிலி எழுத்து *device, முழு *mount_flags,
				  எழுத்து *mtpt, முழு mtlen)
{
	முழு	retval = 0;

	எனில் (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		எனில் (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} இல்லையெனில் {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#இல்லையெனில்
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#இல்லையெனில்
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	எனில் (retval)
		திரும்பு retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		கட்டமைப்பு stat st_buf;
		முழு fd;
		எனில் ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			திரும்பு 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		எனில் (fd < 0) {
			எனில் (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} இல்லையெனில்
			close(fd);
	}
#endif

	திரும்பு 0;
}

முழு is_mounted(மாறிலி எழுத்து *file)
{
	முழு	retval;
	முழு	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, வெற்று, 0);
	எனில் (retval)
		திரும்பு 0;
	திரும்பு mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு flags = 0;
	எழுத்து devname[PATH_MAX];

	எனில் (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		திரும்பு EXIT_FAILURE;
	}

	எனில் (check_mount_point(argv[1], &flags, devname, அளவு(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		எனில் (flags & MF_SWAP)
			printf("used swap device\n");
		இல்லையெனில்
			printf("mounted on %s\n", devname);
		திரும்பு EXIT_SUCCESS;
	}

	printf("not mounted\n");
	திரும்பு EXIT_FAILURE;
}
#endif /* DEBUG */
