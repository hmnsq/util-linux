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
स्थिर पूर्णांक check_mntent_file(स्थिरांक अक्षर *mtab_file, स्थिरांक अक्षर *file,
				   पूर्णांक *mount_flags, अक्षर *mtpt, पूर्णांक mtlen)
{
	संरचना mntent	*mnt;
	संरचना stat	st_buf;
	पूर्णांक		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	खाता		*f;
	पूर्णांक		fd;

	*mount_flags = 0;
	जर ((f = setmntent (mtab_file, "r")) == रिक्त)
		परत त्रुटि_सं;

	जर (stat(file, &st_buf) == 0) {
		जर (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#endif	/* __GNU__ */
		} अन्यथा {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	पर्यंत ((mnt = getmntent (f)) != रिक्त) {
		जर (mnt->mnt_fsname[0] != '/')
			सुरूठेवा;
		जर (म_भेद(file, mnt->mnt_fsname) == 0)
			थांबा;
		जर (stat(mnt->mnt_fsname, &st_buf) != 0)
			सुरूठेवा;

		जर (S_ISBLK(st_buf.st_mode)) {
#ifndef __GNU__
			जर (file_rdev && file_rdev == st_buf.st_rdev)
				थांबा;
#ifdef __linux__
			/* maybe the file is loopdev backing file */
			जर (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				थांबा;
#endif /* __linux__ */
#endif	/* __GNU__ */
		} अन्यथा {
			जर (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				थांबा;
		}
	}

	जर (mnt == रिक्त) {
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		जर (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			जर (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			जा is_मूळ;
		}
#endif	/* __GNU__ */
		जा errout;
	}
#ifndef __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	जर (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = त्रुटि_सं;
		जर (retval == ENOENT) {
#ifdef DEBUG
			म_लिखो("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#endif /* DEBUG */
			retval = 0;
		}
		जा errout;
	}
	जर (file_rdev && (st_buf.st_dev != file_rdev)) {
#ifdef DEBUG
		म_लिखो("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#endif /* DEBUG */
		जा errout;
	}
#endif /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#ifdef MNTOPT_RO
	/* Check to see if the ro option is set */
	जर (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#endif

	जर (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	जर (!म_भेद(mnt->mnt_dir, "/")) {
is_मूळ:
#define TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = खोलो(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		जर (fd < 0) {
			जर (त्रुटि_सं == EROFS)
				*mount_flags |= MF_READONLY;
		} अन्यथा
			बंद(fd);
		(शून्य) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	परत retval;
}

स्थिर पूर्णांक check_mntent(स्थिरांक अक्षर *file, पूर्णांक *mount_flags,
			      अक्षर *mtpt, पूर्णांक mtlen)
{
	पूर्णांक	retval;

#ifdef DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	जर (retval == 0)
		परत 0;
#endif /* DEBUG */
#ifdef __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	जर (retval == 0 && (*mount_flags != 0))
		परत 0;
	जर (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		परत retval;
	}
#endif /* __linux__ */
#जर defined(MOUNTED) || defined(_PATH_MOUNTED)
#ifndef MOUNTED
#define MOUNTED _PATH_MOUNTED
#endif /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	परत retval;
#अन्यथा
	*mount_flags = 0;
	परत 0;
#endif /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#अन्यथा
#जर defined(HAVE_GETMNTINFO)

स्थिर पूर्णांक check_getmntinfo(स्थिरांक अक्षर *file, पूर्णांक *mount_flags,
				  अक्षर *mtpt, पूर्णांक mtlen)
{
	संरचना statfs *mp;
	पूर्णांक    len, n;
	स्थिरांक  अक्षर   *s1;
	अक्षर	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	जर (n == 0)
		परत त्रुटि_सं;

	len = आकार(_PATH_DEV) - 1;
	s1 = file;
	जर (म_भेदन(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	पर्यंत (--n >= 0) {
		s2 = mp->f_mntfromname;
		जर (म_भेदन(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		जर (म_भेद(s1, s2) == 0 || म_भेद(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			थांबा;
		}
		++mp;
	}
	जर (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	परत 0;
}
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
स्थिर पूर्णांक is_swap_device(स्थिरांक अक्षर *file)
{
	खाता		*f;
	अक्षर		buf[1024], *cp;
	dev_t		file_dev;
	संरचना stat	st_buf;
	पूर्णांक		ret = 0;

	file_dev = 0;
#ifndef __GNU__ /* The GNU hurd is broken with respect to stat devices */
	जर ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#endif	/* __GNU__ */

	जर (!(f = ख_खोलो("/proc/swaps", "r" UL_CLOEXECSTR)))
		परत 0;
	/* Skip the first line */
	जर (!ख_माला_लो(buf, आकार(buf), f))
		जा leave;
	जर (*buf && म_भेदन(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		जा valid_first_ओळ;

	पर्यंत (ख_माला_लो(buf, आकार(buf), f)) {
valid_first_ओळ:
		जर ((cp = म_अक्षर(buf, ' ')) != रिक्त)
			*cp = 0;
		जर ((cp = म_अक्षर(buf, '\t')) != रिक्त)
			*cp = 0;
		जर (म_भेद(buf, file) == 0) {
			ret++;
			थांबा;
		}
#ifndef __GNU__
		जर (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			थांबा;
		}
#endif	/* __GNU__ */
	}

leave:
	ख_बंद(f);
	परत ret;
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
पूर्णांक check_mount_point(स्थिरांक अक्षर *device, पूर्णांक *mount_flags,
				  अक्षर *mtpt, पूर्णांक mtlen)
{
	पूर्णांक	retval = 0;

	जर (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		जर (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} अन्यथा {
#ifdef HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#अन्यथा
#ifdef HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#अन्यथा
#ifdef __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#endif
		*mount_flags = 0;
#endif /* HAVE_GETMNTINFO */
#endif /* HAVE_MNTENT_H */
	}
	जर (retval)
		परत retval;

#ifdef __linux__ /* This only works on Linux 2.6+ systems */
	{
		संरचना stat st_buf;
		पूर्णांक fd;
		जर ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			परत 0;
		fd = खोलो(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		जर (fd < 0) {
			जर (त्रुटि_सं == EBUSY)
				*mount_flags |= MF_BUSY;
		} अन्यथा
			बंद(fd);
	}
#endif

	परत 0;
}

पूर्णांक is_mounted(स्थिरांक अक्षर *file)
{
	पूर्णांक	retval;
	पूर्णांक	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, रिक्त, 0);
	जर (retval)
		परत 0;
	परत mount_flags & MF_MOUNTED;
}

#ifdef TEST_PROGRAM_ISMOUNTED
पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक flags = 0;
	अक्षर devname[PATH_MAX];

	जर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "Usage: %s device\n", argv[0]);
		परत निकास_त्रुटि;
	}

	जर (check_mount_point(argv[1], &flags, devname, आकार(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		जर (flags & MF_SWAP)
			म_लिखो("used swap device\n");
		अन्यथा
			म_लिखो("mounted on %s\n", devname);
		परत निकास_सफल;
	}

	म_लिखो("not mounted\n");
	परत निकास_त्रुटि;
}
#endif /* DEBUG */
