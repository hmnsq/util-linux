/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#ifdef HAVE_MNTENT_H
#അന്തര്ഭാവ <mntent.h>
#endif
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "ismounted.h"
#അന്തര്ഭാവ "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ check_mntent_file(സ്ഥിരം അക്ഷരം *mtab_file, സ്ഥിരം അക്ഷരം *file,
				   പൂർണ്ണസംഖ്യ *mount_flags, അക്ഷരം *mtpt, പൂർണ്ണസംഖ്യ mtlen)
{
	ഘടന mntent	*mnt;
	ഘടന stat	st_buf;
	പൂർണ്ണസംഖ്യ		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	പൂർണ്ണസംഖ്യ		fd;

	*mount_flags = 0;
	എങ്കിൽ ((f = setmntent (mtab_file, "r")) == ശൂന്യ)
		തിരികെ errno;

	എങ്കിൽ (stat(file, &st_buf) == 0) {
		എങ്കിൽ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} അല്ലെങ്കിൽ {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	ആകുമ്പോൾ ((mnt = getmntent (f)) != ശൂന്യ) {
		എങ്കിൽ (mnt->mnt_fsname[0] != '/')
			തുടരുക;
		എങ്കിൽ (strcmp(file, mnt->mnt_fsname) == 0)
			നിർത്തുക;
		എങ്കിൽ (stat(mnt->mnt_fsname, &st_buf) != 0)
			തുടരുക;

		എങ്കിൽ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			എങ്കിൽ (file_rdev && file_rdev == st_buf.st_rdev)
				നിർത്തുക;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			എങ്കിൽ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				നിർത്തുക;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				നിർത്തുക;
		}
	}

	എങ്കിൽ (mnt == ശൂന്യ) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		എങ്കിൽ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			എങ്കിൽ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			പോകുക is_വേര്;
		}
#endif	/* __GNU__ */
		പോകുക errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	എങ്കിൽ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		എങ്കിൽ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		പോകുക errout;
	}
	എങ്കിൽ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		പോകുക errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	എങ്കിൽ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	എങ്കിൽ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	എങ്കിൽ (!strcmp(mnt->mnt_dir, "/")) {
is_വേര്:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		എങ്കിൽ (fd < 0) {
			എങ്കിൽ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} അല്ലെങ്കിൽ
			close(fd);
		(ശൂന്യം) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	തിരികെ retval;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ check_mntent(സ്ഥിരം അക്ഷരം *file, പൂർണ്ണസംഖ്യ *mount_flags,
			      അക്ഷരം *mtpt, പൂർണ്ണസംഖ്യ mtlen)
{
	പൂർണ്ണസംഖ്യ	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	എങ്കിൽ (retval == 0)
		തിരികെ 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	എങ്കിൽ (retval == 0 && (*mount_flags != 0))
		തിരികെ 0;
	എങ്കിൽ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		തിരികെ retval;
	}
#endif /* __linux__ */
#എങ്കിൽ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	തിരികെ retval;
#അല്ലെങ്കിൽ
	*mount_flags = 0;
	തിരികെ 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#അല്ലെങ്കിൽ
#എങ്കിൽ defined(HAVE_GETMNTINFO)

സ്ഥിതം പൂർണ്ണസംഖ്യ check_getmntinfo(സ്ഥിരം അക്ഷരം *file, പൂർണ്ണസംഖ്യ *mount_flags,
				  അക്ഷരം *mtpt, പൂർണ്ണസംഖ്യ mtlen)
{
	ഘടന statfs *mp;
	പൂർണ്ണസംഖ്യ    len, n;
	സ്ഥിരം  അക്ഷരം   *s1;
	അക്ഷരം	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	എങ്കിൽ (n == 0)
		തിരികെ errno;

	len = വലുപ്പം(_PATH_DEV) - 1;
	s1 = file;
	എങ്കിൽ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	ആകുമ്പോൾ (--n >= 0) {
		s2 = mp->f_mntfromname;
		എങ്കിൽ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		എങ്കിൽ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			നിർത്തുക;
		}
		++mp;
	}
	എങ്കിൽ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	തിരികെ 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ is_swap_device(സ്ഥിരം അക്ഷരം *file)
{
	FILE		*f;
	അക്ഷരം		buf[1024], *cp;
	dev_t		file_dev;
	ഘടന stat	st_buf;
	പൂർണ്ണസംഖ്യ		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	എങ്കിൽ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	എങ്കിൽ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		തിരികെ 0;
	/* Skip the first line */
	എങ്കിൽ (!fgets(buf, വലുപ്പം(buf), f))
		പോകുക leave;
	എങ്കിൽ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		പോകുക valid_first_വരി;

	ആകുമ്പോൾ (fgets(buf, വലുപ്പം(buf), f)) {
valid_first_വരി:
		എങ്കിൽ ((cp = strchr(buf, ' ')) != ശൂന്യ)
			*cp = 0;
		എങ്കിൽ ((cp = strchr(buf, '\t')) != ശൂന്യ)
			*cp = 0;
		എങ്കിൽ (strcmp(buf, file) == 0) {
			ret++;
			നിർത്തുക;
		}
#ifndef __GNU__
		എങ്കിൽ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			നിർത്തുക;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	തിരികെ ret;
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
പൂർണ്ണസംഖ്യ check_mount_point(സ്ഥിരം അക്ഷരം *device, പൂർണ്ണസംഖ്യ *mount_flags,
				  അക്ഷരം *mtpt, പൂർണ്ണസംഖ്യ mtlen)
{
	പൂർണ്ണസംഖ്യ	retval = 0;

	എങ്കിൽ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		എങ്കിൽ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} അല്ലെങ്കിൽ {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#അല്ലെങ്കിൽ
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#അല്ലെങ്കിൽ
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	എങ്കിൽ (retval)
		തിരികെ retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		ഘടന stat st_buf;
		പൂർണ്ണസംഖ്യ fd;
		എങ്കിൽ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			തിരികെ 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		എങ്കിൽ (fd < 0) {
			എങ്കിൽ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} അല്ലെങ്കിൽ
			close(fd);
	}
#endif

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ is_mounted(സ്ഥിരം അക്ഷരം *file)
{
	പൂർണ്ണസംഖ്യ	retval;
	പൂർണ്ണസംഖ്യ	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, ശൂന്യ, 0);
	എങ്കിൽ (retval)
		തിരികെ 0;
	തിരികെ mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ flags = 0;
	അക്ഷരം devname[PATH_MAX];

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		തിരികെ EXIT_FAILURE;
	}

	എങ്കിൽ (check_mount_point(argv[1], &flags, devname, വലുപ്പം(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		എങ്കിൽ (flags & MF_SWAP)
			printf("used swap device\n");
		അല്ലെങ്കിൽ
			printf("mounted on %s\n", devname);
		തിരികെ EXIT_SUCCESS;
	}

	printf("not mounted\n");
	തിരികെ EXIT_FAILURE;
}
#endif /* DEBUG */
