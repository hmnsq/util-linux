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
#यदि_परिभाषितम् HAVE_MNTENT_H
#अन्तर्भाव <mntent.h>
#यदि_अन्तः
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <sys/param.h>

#यदि_अपरिभाषितम् __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#यदि_अन्तः

#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "ismounted.h"
#अन्तर्भाव "c.h"
#यदि_परिभाषितम् __linux__
# include "loopdev.h"
#यदि_अन्तः



#यदि_परिभाषितम् HAVE_MNTENT_H
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
	यदि ((f = setmntent (mtab_file, "r")) == रिक्त)
		प्रत्यागम errno;

	यदि (stat(file, &st_buf) == 0) {
		यदि (S_ISBLK(st_buf.st_mode)) {
#यदि_अपरिभाषितम् __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#यदि_अन्तः	/* __GNU__ */
		} अन्यथा {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	यावत् ((mnt = getmntent (f)) != रिक्त) {
		यदि (mnt->mnt_fsname[0] != '/')
			अनुवर्त;
		यदि (वाक्यं_तुलय(file, mnt->mnt_fsname) == 0)
			भङ्ग;
		यदि (stat(mnt->mnt_fsname, &st_buf) != 0)
			अनुवर्त;

		यदि (S_ISBLK(st_buf.st_mode)) {
#यदि_अपरिभाषितम् __GNU__
			यदि (file_rdev && file_rdev == st_buf.st_rdev)
				भङ्ग;
#यदि_परिभाषितम् __linux__
			/* maybe the file is loopdev backing file */
			यदि (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_प्रयुक्तम्(mnt->mnt_fsname, file, 0, 0, 0))
				भङ्ग;
#यदि_अन्तः /* __linux__ */
#यदि_अन्तः	/* __GNU__ */
		} अन्यथा {
			यदि (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				भङ्ग;
		}
	}

	यदि (mnt == रिक्त) {
#यदि_अपरिभाषितम् __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		यदि (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			यदि (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			गच्छ is_मूलम्;
		}
#यदि_अन्तः	/* __GNU__ */
		गच्छ errout;
	}
#यदि_अपरिभाषितम् __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	यदि (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		यदि (retval == ENOENT) {
#यदि_परिभाषितम् DEBUG
			प्रारूप_लिख("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#यदि_अन्तः /* DEBUG */
			retval = 0;
		}
		गच्छ errout;
	}
	यदि (file_rdev && (st_buf.st_dev != file_rdev)) {
#यदि_परिभाषितम् DEBUG
		प्रारूप_लिख("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#यदि_अन्तः /* DEBUG */
		गच्छ errout;
	}
#यदि_अन्तः /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#यदि_परिभाषितम् MNTOPT_RO
	/* Check to see if the ro option is set */
	यदि (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#यदि_अन्तः

	यदि (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	यदि (!वाक्यं_तुलय(mnt->mnt_dir, "/")) {
is_मूलम्:
#परिभाषय TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = उद्घाटय(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		यदि (fd < 0) {
			यदि (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} अन्यथा
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

#यदि_परिभाषितम् DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	यदि (retval == 0)
		प्रत्यागम 0;
#यदि_अन्तः /* DEBUG */
#यदि_परिभाषितम् __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	यदि (retval == 0 && (*mount_flags != 0))
		प्रत्यागम 0;
	यदि (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		प्रत्यागम retval;
	}
#यदि_अन्तः /* __linux__ */
#यदि defined(MOUNTED) || defined(_PATH_MOUNTED)
#यदि_अपरिभाषितम् MOUNTED
#परिभाषय MOUNTED _PATH_MOUNTED
#यदि_अन्तः /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	प्रत्यागम retval;
#अन्यथा
	*mount_flags = 0;
	प्रत्यागम 0;
#यदि_अन्तः /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#अन्यथा
#यदि defined(HAVE_GETMNTINFO)

स्थिर पूर्णाङ्क check_getmntinfo(ध्रुव वर्ण *file, पूर्णाङ्क *mount_flags,
				  वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	संरचना statfs *mp;
	पूर्णाङ्क    len, n;
	ध्रुव  वर्ण   *s1;
	वर्ण	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	यदि (n == 0)
		प्रत्यागम errno;

	len = परिमाण(_PATH_DEV) - 1;
	s1 = file;
	यदि (वाक्ये_तुलय(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	यावत् (--n >= 0) {
		s2 = mp->f_mntfromname;
		यदि (वाक्ये_तुलय(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		यदि (वाक्यं_तुलय(s1, s2) == 0 || वाक्यं_तुलय(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			भङ्ग;
		}
		++mp;
	}
	यदि (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	प्रत्यागम 0;
}
#यदि_अन्तः /* HAVE_GETMNTINFO */
#यदि_अन्तः /* HAVE_MNTENT_H */

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
#यदि_अपरिभाषितम् __GNU__ /* The GNU hurd is broken with respect to stat devices */
	यदि ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#यदि_अन्तः	/* __GNU__ */

	यदि (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		प्रत्यागम 0;
	/* Skip the first line */
	यदि (!fgets(buf, परिमाण(buf), f))
		गच्छ leave;
	यदि (*buf && वाक्ये_तुलय(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		गच्छ valid_first_पङ्क्तिः;

	यावत् (fgets(buf, परिमाण(buf), f)) {
valid_first_पङ्क्तिः:
		यदि ((cp = strchr(buf, ' ')) != रिक्त)
			*cp = 0;
		यदि ((cp = strchr(buf, '\t')) != रिक्त)
			*cp = 0;
		यदि (वाक्यं_तुलय(buf, file) == 0) {
			ret++;
			भङ्ग;
		}
#यदि_अपरिभाषितम् __GNU__
		यदि (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			भङ्ग;
		}
#यदि_अन्तः	/* __GNU__ */
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
#यदि_परिभाषितम् __TURBOC__
 #निर्देशः argsused
#यदि_अन्तः
पूर्णाङ्क check_mount_point(ध्रुव वर्ण *device, पूर्णाङ्क *mount_flags,
				  वर्ण *mtpt, पूर्णाङ्क mtlen)
{
	पूर्णाङ्क	retval = 0;

	यदि (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		यदि (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} अन्यथा {
#यदि_परिभाषितम् HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#अन्यथा
#यदि_परिभाषितम् HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#अन्यथा
#यदि_परिभाषितम् __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#यदि_अन्तः
		*mount_flags = 0;
#यदि_अन्तः /* HAVE_GETMNTINFO */
#यदि_अन्तः /* HAVE_MNTENT_H */
	}
	यदि (retval)
		प्रत्यागम retval;

#यदि_परिभाषितम् __linux__ /* This only works on Linux 2.6+ systems */
	{
		संरचना stat st_buf;
		पूर्णाङ्क fd;
		यदि ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			प्रत्यागम 0;
		fd = उद्घाटय(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		यदि (fd < 0) {
			यदि (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} अन्यथा
			close(fd);
	}
#यदि_अन्तः

	प्रत्यागम 0;
}

पूर्णाङ्क is_mounted(ध्रुव वर्ण *file)
{
	पूर्णाङ्क	retval;
	पूर्णाङ्क	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, रिक्त, 0);
	यदि (retval)
		प्रत्यागम 0;
	प्रत्यागम mount_flags & MF_MOUNTED;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_ISMOUNTED
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क flags = 0;
	वर्ण devname[PATH_MAX];

	यदि (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		प्रत्यागम EXIT_FAILURE;
	}

	यदि (check_mount_point(argv[1], &flags, devname, परिमाण(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		यदि (flags & MF_SWAP)
			प्रारूप_लिख("used swap device\n");
		अन्यथा
			प्रारूप_लिख("mounted on %s\n", devname);
		प्रत्यागम EXIT_SUCCESS;
	}

	प्रारूप_लिख("not mounted\n");
	प्रत्यागम EXIT_FAILURE;
}
#यदि_अन्तः /* DEBUG */
