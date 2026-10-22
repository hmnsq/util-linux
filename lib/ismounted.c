/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#ifdef HAVE_MNTENT_H
#अन्तर्भाव <mntent.h>
#endif
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <sys/param.h>

#ifndef __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#endif

#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "ismounted.h"
#अन्तर्भाव "c.h"
#ifdef __linux__
# include "loopdev.h"
#endif



#ifdef HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
स्थिर पूर्णाङ्क check_mntent_file(ध्रुव वर्ण *mtab_file, ध्रुव वर्ण *file,
				   पूर्णाङ्क *mount_flags, वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	संरचना mntent	*mnt;
	संरचना stat	st_buf;
	पूर्णाङ्क		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	पूर्णाङ्क		fd;

	*mount_flags = 0;
	जइ ((f = setmntent (mtab_file, "r")) == रिक्त)
		प्रत्यागम errno;

	जइ (stat(file, &st_buf) == 0) {
		जइ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} अण्णहा {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	जाव ((mnt = getmntent (f)) != रिक्त) {
		जइ (mnt->mnt_fsname[0] != '/')
			अनुवर्त;
		जइ (strcmp(file, mnt->mnt_fsname) == 0)
			भङ्ग;
		जइ (stat(mnt->mnt_fsname, &st_buf) != 0)
			अनुवर्त;

		जइ (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			जइ (file_rdev && file_rdev == st_buf.st_rdev)
				भङ्ग;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			जइ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				भङ्ग;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} अण्णहा {
			जइ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				भङ्ग;
		}
	}

	जइ (mnt == रिक्त) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		जइ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			जइ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			गच्छ is_मूलम्;
		}
#endif	/* __GNU__ */
		गच्छ errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	जइ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		जइ (retval == ENOENT) {
#ifdef DEBUG
			printf("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		गच्छ errout;
	}
	जइ (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		printf("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		गच्छ errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	जइ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	जइ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	जइ (!strcmp(mnt->mnt_dir, "/")) {
is_मूलम्:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		जइ (fd < 0) {
			जइ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} अण्णहा
			close(fd);
		(शून्य) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	प्रत्यागम retval;
}

स्थिर पूर्णाङ्क check_mntent(ध्रुव वर्ण *file, पूर्णाङ्क *mount_flags,
			      वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	पूर्णाङ्क	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	जइ (retval == 0)
		प्रत्यागम 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	जइ (retval == 0 && (*mount_flags != 0))
		प्रत्यागम 0;
	जइ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		प्रत्यागम retval;
	}
#endif /* __linux__ */
#जइ defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	प्रत्यागम retval;
#अण्णहा
	*mount_flags = 0;
	प्रत्यागम 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#अण्णहा
#जइ defined(HAVE_GETMNTINFO)

स्थिर पूर्णाङ्क check_getmntinfo(ध्रुव वर्ण *file, पूर्णाङ्क *mount_flags,
				  वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	संरचना statfs *mp;
	पूर्णाङ्क    len, n;
	ध्रुव  वर्ण   *s1;
	वर्ण	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	जइ (n == 0)
		प्रत्यागम errno;

	len = परिमाण(_PATH_DEV) - 1;
	s1 = file;
	जइ (strncmp(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	जाव (--n >= 0) {
		s2 = mp->f_mntfromname;
		जइ (strncmp(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		जइ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			भङ्ग;
		}
		++mp;
	}
	जइ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	प्रत्यागम 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
स्थिर पूर्णाङ्क is_swap_device(ध्रुव वर्ण *file)
{
	FILE		*f;
	वर्ण		buf[1024], *cp;
	dev_t		file_dev;
	संरचना stat	st_buf;
	पूर्णाङ्क		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	जइ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	जइ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		प्रत्यागम 0;
	/* Skip the first line */
	जइ (!fgets(buf, परिमाण(buf), f))
		गच्छ leave;
	जइ (*buf && strncmp(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		गच्छ valid_first_पङ्क्तिः;

	जाव (fgets(buf, परिमाण(buf), f)) {
valid_first_पङ्क्तिः:
		जइ ((cp = strchr(buf, ' ')) != रिक्त)
			*cp = 0;
		जइ ((cp = strchr(buf, '\t')) != रिक्त)
			*cp = 0;
		जइ (strcmp(buf, file) == 0) {
			ret++;
			भङ्ग;
		}
#ifndef __GNU__
		जइ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			भङ्ग;
		}
#endif	/* __GNU__ */
	}

leave:
	fclose(f);
	प्रत्यागम ret;
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
पूर्णाङ्क check_mount_point(ध्रुव वर्ण *device, पूर्णाङ्क *mount_flags,
				  वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	पूर्णाङ्क	retval = 0;

	जइ (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		जइ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} अण्णहा {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#अण्णहा
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#अण्णहा
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	जइ (retval)
		प्रत्यागम retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		संरचना stat st_buf;
		पूर्णाङ्क fd;
		जइ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			प्रत्यागम 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		जइ (fd < 0) {
			जइ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} अण्णहा
			close(fd);
	}
#endif

	प्रत्यागम 0;
}

पूर्णाङ्क is_mounted(ध्रुव वर्ण *file)
{
	पूर्णाङ्क	retval;
	पूर्णाङ्क	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, रिक्त, 0);
	जइ (retval)
		प्रत्यागम 0;
	प्रत्यागम mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क flags = 0;
	वर्ण devname[PATH_MAX];

	जइ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		प्रत्यागम EXIT_FAILURE;
	}

	जइ (check_mount_point(argv[1], &flags, devname, परिमाण(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		जइ (flags & MF_SWAP)
			printf("used swap device\n");
		अण्णहा
			printf("mounted on %s\n", devname);
		प्रत्यागम EXIT_SUCCESS;
	}

	printf("not mounted\n");
	प्रत्यागम EXIT_FAILURE;
}
#endif /* DEBUG */
