/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#ifdef HAVE_MNTENT_H
#અન્તર્ભાવ <mntent.h>
#endif
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "ismounted.h"
#અન્તર્ભાવ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
સ્થિર પૂર્ણાંક check_mntent_file(અચલ અક્ષર *mtab_file, અચલ અક્ષર *file,
				   પૂર્ણાંક *mount_flags, અક્ષર *mtpt, પૂર્ણાંક mtlen)
{
	સંરચના mntent	*mnt;
	સંરચના stat	st_buf;
	પૂર્ણાંક		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	પૂર્ણાંક		fd;

	*mount_flags = 0;
	જો ((f = setmntent (mtab_file, "r")) == ખાલી)
		પાછા errno;

	જો (stat(file, &st_buf) == 0) {
		જો (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} નહીંતર {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	જ્યારે ((mnt = getmntent (f)) != ખાલી) {
		જો (mnt->mnt_fsname[0] != '/')
			ચાલુ;
		જો (strcmp(file, mnt->mnt_fsname) == 0)
			તોડો;
		જો (stat(mnt->mnt_fsname, &st_buf) != 0)
			ચાલુ;

		જો (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			જો (file_rdev && file_rdev == st_buf.st_rdev)
				તોડો;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			જો (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				તોડો;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} નહીંતર {
			જો (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				તોડો;
		}
	}

	જો (mnt == ખાલી) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		જો (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			જો (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			જાઓ is_મૂળ;
		}
#endif	/* __GNU__ */
		જાઓ errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	જો (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		જો (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		જાઓ errout;
	}
	જો (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		જાઓ errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	જો (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	જો (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	જો (!strcmp(mnt->mnt_dir, "/")) {
is_મૂળ:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		જો (fd < 0) {
			જો (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} નહીંતર
			close(fd);
		(શૂન્ય) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	પાછા retval;
}

સ્થિર પૂર્ણાંક check_mntent(અચલ અક્ષર *file, પૂર્ણાંક *mount_flags,
			      અક્ષર *mtpt, પૂર્ણાંક mtlen)
{
	પૂર્ણાંક	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	જો (retval == 0)
		પાછા 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	જો (retval == 0 && (*mount_flags != 0))
		પાછા 0;
	જો (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		પાછા retval;
	}
#endif /* __linux__ */
#જો defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	પાછા retval;
#નહીંતર
	*mount_flags = 0;
	પાછા 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#નહીંતર
#જો defined(HAVE_GETMNTINFO)

સ્થિર પૂર્ણાંક check_getmntinfo(અચલ અક્ષર *file, પૂર્ણાંક *mount_flags,
				  અક્ષર *mtpt, પૂર્ણાંક mtlen)
{
	સંરચના statfs *mp;
	પૂર્ણાંક    len, n;
	અચલ  અક્ષર   *s1;
	અક્ષર	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	જો (n == 0)
		પાછા errno;

	len = કદ(_PATH_DEV) - 1;
	s1 = file;
	જો (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	જ્યારે (--n >= 0) {
		s2 = mp->f_mntfromname;
		જો (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		જો (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			તોડો;
		}
		++mp;
	}
	જો (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	પાછા 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
સ્થિર પૂર્ણાંક is_swap_device(અચલ અક્ષર *file)
{
	FILE		*f;
	અક્ષર		buf[1024], *cp;
	dev_t		file_dev;
	સંરચના stat	st_buf;
	પૂર્ણાંક		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	જો ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	જો (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		પાછા 0;
	/* Skip the first line */
	જો (!fgets(buf, કદ(buf), f))
		જાઓ leave;
	જો (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		જાઓ valid_first_લીટી;

	જ્યારે (fgets(buf, કદ(buf), f)) {
valid_first_લીટી:
		જો ((cp = strchr(buf, ' ')) != ખાલી)
			*cp = 0;
		જો ((cp = strchr(buf, '\t')) != ખાલી)
			*cp = 0;
		જો (strcmp(buf, file) == 0) {
			ret++;
			તોડો;
		}
#ifndef __GNU__
		જો (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			તોડો;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	પાછા ret;
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
પૂર્ણાંક check_mount_point(અચલ અક્ષર *device, પૂર્ણાંક *mount_flags,
				  અક્ષર *mtpt, પૂર્ણાંક mtlen)
{
	પૂર્ણાંક	retval = 0;

	જો (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		જો (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} નહીંતર {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#નહીંતર
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#નહીંતર
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	જો (retval)
		પાછા retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		સંરચના stat st_buf;
		પૂર્ણાંક fd;
		જો ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			પાછા 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		જો (fd < 0) {
			જો (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} નહીંતર
			close(fd);
	}
#endif

	પાછા 0;
}

પૂર્ણાંક is_mounted(અચલ અક્ષર *file)
{
	પૂર્ણાંક	retval;
	પૂર્ણાંક	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, ખાલી, 0);
	જો (retval)
		પાછા 0;
	પાછા mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક flags = 0;
	અક્ષર devname[PATH_MAX];

	જો (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		પાછા EXIT_FAILURE;
	}

	જો (check_mount_point(argv[1], &flags, devname, કદ(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		જો (flags & MF_SWAP)
			printf("used swap device\n");
		નહીંતર
			printf("mounted on %s\n", devname);
		પાછા EXIT_SUCCESS;
	}

	printf("not mounted\n");
	પાછા EXIT_FAILURE;
}
#endif /* DEBUG */
