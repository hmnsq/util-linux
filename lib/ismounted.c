/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#నిర్వచితమైతే HAVE_MNTENT_H
#అన్తర్భావ <mntent.h>
#అయితే_ముగింపు
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <sys/param.h>

#నిర్వచితం_కాకపోతే __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#అయితే_ముగింపు

#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "ismounted.h"
#అన్తర్భావ "c.h"
#నిర్వచితమైతే __linux__
# include "loopdev.h"
#అయితే_ముగింపు



#నిర్వచితమైతే HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
స్థిర పూర్ణాంకం check_mntent_file(స్థిరరాశి అక్షరం *mtab_file, స్థిరరాశి అక్షరం *file,
				   పూర్ణాంకం *mount_flags, అక్షరం *mtpt, పూర్ణాంకం mtlen)
{
	నిర్మాణం mntent	*mnt;
	నిర్మాణం stat	st_buf;
	పూర్ణాంకం		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	FILE		*f;
	పూర్ణాంకం		fd;

	*mount_flags = 0;
	ఒకవేళ ((f = setmntent (mtab_file, "r")) == ఖాళీ)
		తిరిగి errno;

	ఒకవేళ (stat(file, &st_buf) == 0) {
		ఒకవేళ (S_ISBLK(st_buf.st_mode)) {
#నిర్వచితం_కాకపోతే __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#అయితే_ముగింపు	/* __GNU__ */
		} లేకపోతే {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	అయితే ((mnt = getmntent (f)) != ఖాళీ) {
		ఒకవేళ (mnt->mnt_fsname[0] != '/')
			కొనసాగు;
		ఒకవేళ (strcmp(file, mnt->mnt_fsname) == 0)
			ఆపు;
		ఒకవేళ (stat(mnt->mnt_fsname, &st_buf) != 0)
			కొనసాగు;

		ఒకవేళ (S_ISBLK(st_buf.st_mode)) {
#నిర్వచితం_కాకపోతే __GNU__
			ఒకవేళ (file_rdev && file_rdev == st_buf.st_rdev)
				ఆపు;
#నిర్వచితమైతే __linux__
			/* maybe the file is loopdev backing file */
			ఒకవేళ (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_వాడిన(mnt->mnt_fsname, file, 0, 0, 0))
				ఆపు;
#అయితే_ముగింపు /* __linux__ */
#అయితే_ముగింపు	/* __GNU__ */
		} లేకపోతే {
			ఒకవేళ (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				ఆపు;
		}
	}

	ఒకవేళ (mnt == ఖాళీ) {
#నిర్వచితం_కాకపోతే __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		ఒకవేళ (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			ఒకవేళ (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			వెళ్ళు is_మూలం;
		}
#అయితే_ముగింపు	/* __GNU__ */
		వెళ్ళు errout;
	}
#నిర్వచితం_కాకపోతే __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	ఒకవేళ (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = errno;
		ఒకవేళ (retval == ENOENT) {
#నిర్వచితమైతే DEBUG
			రాయి("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#అయితే_ముగింపు /* DEBUG */
			retval = 0;
		}
		వెళ్ళు errout;
	}
	ఒకవేళ (file_rdev && (st_buf.st_dev != file_rdev)) {
#నిర్వచితమైతే DEBUG
		రాయి("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#అయితే_ముగింపు /* DEBUG */
		వెళ్ళు errout;
	}
#అయితే_ముగింపు /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#నిర్వచితమైతే MNTOPT_RO
	/* Check to see if the ro option is set */
	ఒకవేళ (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#అయితే_ముగింపు

	ఒకవేళ (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	ఒకవేళ (!strcmp(mnt->mnt_dir, "/")) {
is_మూలం:
#నిర్వచించు TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = open(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		ఒకవేళ (fd < 0) {
			ఒకవేళ (errno == EROFS)
				*mount_flags |= MF_READONLY;
		} లేకపోతే
			close(fd);
		(శూన్య) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	తిరిగి retval;
}

స్థిర పూర్ణాంకం check_mntent(స్థిరరాశి అక్షరం *file, పూర్ణాంకం *mount_flags,
			      అక్షరం *mtpt, పూర్ణాంకం mtlen)
{
	పూర్ణాంకం	retval;

#నిర్వచితమైతే DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	ఒకవేళ (retval == 0)
		తిరిగి 0;
#అయితే_ముగింపు /* DEBUG */
#నిర్వచితమైతే __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	ఒకవేళ (retval == 0 && (*mount_flags != 0))
		తిరిగి 0;
	ఒకవేళ (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		తిరిగి retval;
	}
#అయితే_ముగింపు /* __linux__ */
#ఒకవేళ defined(MOUNTED) || defined(_PATH_MOUNTED)
#నిర్వచితం_కాకపోతే MOUNTED
#నిర్వచించు MOUNTED _PATH_MOUNTED
#అయితే_ముగింపు /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	తిరిగి retval;
#లేకపోతే
	*mount_flags = 0;
	తిరిగి 0;
#అయితే_ముగింపు /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#లేకపోతే
#ఒకవేళ defined(HAVE_GETMNTINFO)

స్థిర పూర్ణాంకం check_getmntinfo(స్థిరరాశి అక్షరం *file, పూర్ణాంకం *mount_flags,
				  అక్షరం *mtpt, పూర్ణాంకం mtlen)
{
	నిర్మాణం statfs *mp;
	పూర్ణాంకం    len, n;
	స్థిరరాశి  అక్షరం   *s1;
	అక్షరం	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	ఒకవేళ (n == 0)
		తిరిగి errno;

	len = పరిమాణం(_PATH_DEV) - 1;
	s1 = file;
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	అయితే (--n >= 0) {
		s2 = mp->f_mntfromname;
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		ఒకవేళ (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			ఆపు;
		}
		++mp;
	}
	ఒకవేళ (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	తిరిగి 0;
}
#అయితే_ముగింపు /* HAVE_GETMNTINFO */
#అయితే_ముగింపు /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
స్థిర పూర్ణాంకం is_మార్పిడి_device(స్థిరరాశి అక్షరం *file)
{
	FILE		*f;
	అక్షరం		buf[1024], *cp;
	dev_t		file_dev;
	నిర్మాణం stat	st_buf;
	పూర్ణాంకం		ret = 0;

	file_dev = 0;
#నిర్వచితం_కాకపోతే __GNU__ /* The GNU hurd is broken with respect to stat devices */
	ఒకవేళ ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#అయితే_ముగింపు	/* __GNU__ */

	ఒకవేళ (!(f = fopen("/proc/swaps", "r" UL_CLOEXECSTR)))
		తిరిగి 0;
	/* Skip the first line */
	ఒకవేళ (!fgets(buf, పరిమాణం(buf), f))
		వెళ్ళు leave;
	ఒకవేళ (*buf && అక్షరశ్రేణి_పోలిక(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		వెళ్ళు valid_మొదటి_పంక్తి;

	అయితే (fgets(buf, పరిమాణం(buf), f)) {
valid_మొదటి_పంక్తి:
		ఒకవేళ ((cp = strchr(buf, ' ')) != ఖాళీ)
			*cp = 0;
		ఒకవేళ ((cp = strchr(buf, '\t')) != ఖాళీ)
			*cp = 0;
		ఒకవేళ (strcmp(buf, file) == 0) {
			ret++;
			ఆపు;
		}
#నిర్వచితం_కాకపోతే __GNU__
		ఒకవేళ (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			ఆపు;
		}
#అయితే_ముగింపు	/* __GNU__ */
	}

leave:
	fclose(f);
	తిరిగి ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#నిర్వచితమైతే __TURBOC__
 #సూచన argsused
#అయితే_ముగింపు
పూర్ణాంకం check_mount_బిందువు(స్థిరరాశి అక్షరం *device, పూర్ణాంకం *mount_flags,
				  అక్షరం *mtpt, పూర్ణాంకం mtlen)
{
	పూర్ణాంకం	retval = 0;

	ఒకవేళ (is_మార్పిడి_device(device)) {
		*mount_flags = MF_MOUNTED | MF_మార్పిడి;
		ఒకవేళ (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} లేకపోతే {
#నిర్వచితమైతే HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#లేకపోతే
#నిర్వచితమైతే HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#లేకపోతే
#నిర్వచితమైతే __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#అయితే_ముగింపు
		*mount_flags = 0;
#అయితే_ముగింపు /* HAVE_GETMNTINFO */
#అయితే_ముగింపు /* HAVE_MNTENT_H */
	}
	ఒకవేళ (retval)
		తిరిగి retval;

#నిర్వచితమైతే __linux__ /* This only works on Linux 2.6+ systems */
	{
		నిర్మాణం stat st_buf;
		పూర్ణాంకం fd;
		ఒకవేళ ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			తిరిగి 0;
		fd = open(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		ఒకవేళ (fd < 0) {
			ఒకవేళ (errno == EBUSY)
				*mount_flags |= MF_BUSY;
		} లేకపోతే
			close(fd);
	}
#అయితే_ముగింపు

	తిరిగి 0;
}

పూర్ణాంకం is_mounted(స్థిరరాశి అక్షరం *file)
{
	పూర్ణాంకం	retval;
	పూర్ణాంకం	mount_flags = 0;

	retval = check_mount_బిందువు(file, &mount_flags, ఖాళీ, 0);
	ఒకవేళ (retval)
		తిరిగి 0;
	తిరిగి mount_flags & MF_MOUNTED;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_ISMOUNTED
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం flags = 0;
	అక్షరం devname[PATH_MAX];

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "Usage: %s device\n", argv[0]);
		తిరిగి EXIT_FAILURE;
	}

	ఒకవేళ (check_mount_బిందువు(argv[1], &flags, devname, పరిమాణం(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		ఒకవేళ (flags & MF_మార్పిడి)
			రాయి("used swap device\n");
		లేకపోతే
			రాయి("mounted on %s\n", devname);
		తిరిగి EXIT_SUCCESS;
	}

	రాయి("not mounted\n");
	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు /* DEBUG */
