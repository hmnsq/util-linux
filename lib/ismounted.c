/*
 * ismounted.c --- Check to see if the filesystem was mounted
 *
 * Copyright (C) 1995,1996,1997,1998,1999,2000,2008 Theodore Ts'o.
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 */
#समावेश <stdio.h>
#समावेश <unistd.h>
#समावेश <stdlib.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#अगर_घोषित HAVE_MNTENT_H
#समावेश <mntent.h>
#पूर्ण_अगर
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <ctype.h>
#समावेश <sys/param.h>

#अगर_अघोषित __linux__
# ifdef HAVE_SYS_UCRED_H
#  include <sys/ucred.h>
# endif
# ifdef HAVE_SYS_MOUNT_H
#  include <sys/mount.h>
# endif
#पूर्ण_अगर

#समावेश "pathnames.h"
#समावेश "strutils.h"
#समावेश "ismounted.h"
#समावेश "c.h"
#अगर_घोषित __linux__
# include "loopdev.h"
#पूर्ण_अगर



#अगर_घोषित HAVE_MNTENT_H
/*
 * Helper function which checks a file in /etc/mtab format to see if a
 * filesystem is mounted.  Returns an error if the file doesn't exist
 * or can't be opened.
 */
अटल पूर्णांक check_mntent_file(स्थिर अक्षर *mtab_file, स्थिर अक्षर *file,
				   पूर्णांक *mount_flags, अक्षर *mtpt, पूर्णांक mtlen)
{
	काष्ठा mntent	*mnt;
	काष्ठा stat	st_buf;
	पूर्णांक		retval = 0;
	dev_t		file_dev=0, file_rdev=0;
	ino_t		file_ino=0;
	खाता		*f;
	पूर्णांक		fd;

	*mount_flags = 0;
	अगर ((f = setmntent (mtab_file, "r")) == शून्य)
		वापस त्रुटि_सं;

	अगर (stat(file, &st_buf) == 0) {
		अगर (S_ISBLK(st_buf.st_mode)) {
#अगर_अघोषित __GNU__ /* The GNU hurd is broken with respect to stat devices */
			file_rdev = st_buf.st_rdev;
#पूर्ण_अगर	/* __GNU__ */
		} अन्यथा {
			file_dev = st_buf.st_dev;
			file_ino = st_buf.st_ino;
		}
	}

	जबतक ((mnt = getmntent (f)) != शून्य) {
		अगर (mnt->mnt_fsname[0] != '/')
			जारी;
		अगर (म_भेद(file, mnt->mnt_fsname) == 0)
			अवरोध;
		अगर (stat(mnt->mnt_fsname, &st_buf) != 0)
			जारी;

		अगर (S_ISBLK(st_buf.st_mode)) {
#अगर_अघोषित __GNU__
			अगर (file_rdev && file_rdev == st_buf.st_rdev)
				अवरोध;
#अगर_घोषित __linux__
			/* maybe the file is loopdev backing file */
			अगर (file_dev
			    && major(st_buf.st_rdev) == LOOPDEV_MAJOR
			    && loopdev_is_used(mnt->mnt_fsname, file, 0, 0, 0))
				अवरोध;
#पूर्ण_अगर /* __linux__ */
#पूर्ण_अगर	/* __GNU__ */
		} अन्यथा {
			अगर (file_dev && ((file_dev == st_buf.st_dev) &&
					 (file_ino == st_buf.st_ino)))
				अवरोध;
		}
	}

	अगर (mnt == शून्य) {
#अगर_अघोषित __GNU__ /* The GNU hurd is broken with respect to stat devices */
		/*
		 * Do an extra check to see if this is the root device.  We
		 * can't trust /etc/mtab, and /proc/mounts will only list
		 * /dev/root for the root filesystem.  Argh.  Instead we
		 * check if the given device has the same major/minor number
		 * as the device that the root directory is on.
		 */
		अगर (file_rdev && stat("/", &st_buf) == 0 &&
		    st_buf.st_dev == file_rdev) {
			*mount_flags = MF_MOUNTED;
			अगर (mtpt)
				xstrncpy(mtpt, "/", mtlen);
			जाओ is_मूल;
		}
#पूर्ण_अगर	/* __GNU__ */
		जाओ errout;
	}
#अगर_अघोषित __GNU__ /* The GNU hurd is deficient; what else is new? */
	/* Validate the entry in case /etc/mtab is out of date */
	/*
	 * We need to be paranoid, because some broken distributions
	 * (read: Slackware) don't initialize /etc/mtab before checking
	 * all of the non-root filesystems on the disk.
	 */
	अगर (stat(mnt->mnt_dir, &st_buf) < 0) {
		retval = त्रुटि_सं;
		अगर (retval == ENOENT) {
#अगर_घोषित DEBUG
			म_लिखो("Bogus entry in %s!  (%s does not exist)\n",
			       mtab_file, mnt->mnt_dir);
#पूर्ण_अगर /* DEBUG */
			retval = 0;
		}
		जाओ errout;
	}
	अगर (file_rdev && (st_buf.st_dev != file_rdev)) {
#अगर_घोषित DEBUG
		म_लिखो("Bogus entry in %s!  (%s not mounted on %s)\n",
		       mtab_file, file, mnt->mnt_dir);
#पूर्ण_अगर /* DEBUG */
		जाओ errout;
	}
#पूर्ण_अगर /* __GNU__ */
	*mount_flags = MF_MOUNTED;

#अगर_घोषित MNTOPT_RO
	/* Check to see if the ro option is set */
	अगर (hasmntopt(mnt, MNTOPT_RO))
		*mount_flags |= MF_READONLY;
#पूर्ण_अगर

	अगर (mtpt)
		xstrncpy(mtpt, mnt->mnt_dir, mtlen);
	/*
	 * Check to see if we're referring to the root filesystem.
	 * If so, do a manual check to see if we can open /etc/mtab
	 * read/write, since if the root is mounted read/only, the
	 * contents of /etc/mtab may not be accurate.
	 */
	अगर (!म_भेद(mnt->mnt_dir, "/")) {
is_मूल:
#घोषणा TEST_FILE "/.ismount-test-file"
		*mount_flags |= MF_ISROOT;
		fd = खोलो(TEST_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0600);
		अगर (fd < 0) {
			अगर (त्रुटि_सं == EROFS)
				*mount_flags |= MF_READONLY;
		} अन्यथा
			बंद(fd);
		(व्योम) unlink(TEST_FILE);
	}
	retval = 0;
errout:
	endmntent (f);
	वापस retval;
}

अटल पूर्णांक check_mntent(स्थिर अक्षर *file, पूर्णांक *mount_flags,
			      अक्षर *mtpt, पूर्णांक mtlen)
{
	पूर्णांक	retval;

#अगर_घोषित DEBUG
	retval = check_mntent_file("/tmp/mtab", file, mount_flags,
				   mtpt, mtlen);
	अगर (retval == 0)
		वापस 0;
#पूर्ण_अगर /* DEBUG */
#अगर_घोषित __linux__
	retval = check_mntent_file("/proc/mounts", file, mount_flags,
				   mtpt, mtlen);
	अगर (retval == 0 && (*mount_flags != 0))
		वापस 0;
	अगर (access("/proc/mounts", R_OK) == 0) {
		*mount_flags = 0;
		वापस retval;
	}
#पूर्ण_अगर /* __linux__ */
#अगर defined(MOUNTED) || defined(_PATH_MOUNTED)
#अगर_अघोषित MOUNTED
#घोषणा MOUNTED _PATH_MOUNTED
#पूर्ण_अगर /* MOUNTED */
	retval = check_mntent_file(MOUNTED, file, mount_flags, mtpt, mtlen);
	वापस retval;
#अन्यथा
	*mount_flags = 0;
	वापस 0;
#पूर्ण_अगर /* defined(MOUNTED) || defined(_PATH_MOUNTED) */
}

#अन्यथा
#अगर defined(HAVE_GETMNTINFO)

अटल पूर्णांक check_getmntinfo(स्थिर अक्षर *file, पूर्णांक *mount_flags,
				  अक्षर *mtpt, पूर्णांक mtlen)
{
	काष्ठा statfs *mp;
	पूर्णांक    len, n;
	स्थिर  अक्षर   *s1;
	अक्षर	*s2;

	n = getmntinfo(&mp, MNT_NOWAIT);
	अगर (n == 0)
		वापस त्रुटि_सं;

	len = माप(_PATH_DEV) - 1;
	s1 = file;
	अगर (म_भेदन(_PATH_DEV, s1, len) == 0)
		s1 += len;

	*mount_flags = 0;
	जबतक (--n >= 0) {
		s2 = mp->f_mntfromname;
		अगर (म_भेदन(_PATH_DEV, s2, len) == 0) {
			s2 += len - 1;
			*s2 = 'r';
		}
		अगर (म_भेद(s1, s2) == 0 || म_भेद(s1, &s2[1]) == 0) {
			*mount_flags = MF_MOUNTED;
			अवरोध;
		}
		++mp;
	}
	अगर (mtpt && n >= 0)
		xstrncpy(mtpt, mp->f_mntonname, mtlen);
	वापस 0;
}
#पूर्ण_अगर /* HAVE_GETMNTINFO */
#पूर्ण_अगर /* HAVE_MNTENT_H */

/*
 * Check to see if we're dealing with the swap device.
 */
अटल पूर्णांक is_swap_device(स्थिर अक्षर *file)
{
	खाता		*f;
	अक्षर		buf[1024], *cp;
	dev_t		file_dev;
	काष्ठा stat	st_buf;
	पूर्णांक		ret = 0;

	file_dev = 0;
#अगर_अघोषित __GNU__ /* The GNU hurd is broken with respect to stat devices */
	अगर ((stat(file, &st_buf) == 0) &&
	    S_ISBLK(st_buf.st_mode))
		file_dev = st_buf.st_rdev;
#पूर्ण_अगर	/* __GNU__ */

	अगर (!(f = ख_खोलो("/proc/swaps", "r" UL_CLOEXECSTR)))
		वापस 0;
	/* Skip the first line */
	अगर (!ख_माला_लो(buf, माप(buf), f))
		जाओ leave;
	अगर (*buf && म_भेदन(buf, "Filename\t", 9) != 0)
		/* Linux <=2.6.19 contained a bug in the /proc/swaps
		 * code where the header would not be displayed
		 */
		जाओ valid_first_पंक्ति;

	जबतक (ख_माला_लो(buf, माप(buf), f)) {
valid_first_पंक्ति:
		अगर ((cp = म_अक्षर(buf, ' ')) != शून्य)
			*cp = 0;
		अगर ((cp = म_अक्षर(buf, '\t')) != शून्य)
			*cp = 0;
		अगर (म_भेद(buf, file) == 0) {
			ret++;
			अवरोध;
		}
#अगर_अघोषित __GNU__
		अगर (file_dev && (stat(buf, &st_buf) == 0) &&
		    S_ISBLK(st_buf.st_mode) &&
		    file_dev == st_buf.st_rdev) {
			ret++;
			अवरोध;
		}
#पूर्ण_अगर	/* __GNU__ */
	}

leave:
	ख_बंद(f);
	वापस ret;
}


/*
 * check_mount_point() determines if the device is mounted or otherwise
 * busy, and fills in mount_flags with one or more of the following flags:
 * MF_MOUNTED, MF_ISROOT, MF_READONLY, MF_SWAP, and MF_BUSY.  If mtpt is
 * non-NULL, the directory where the device is mounted is copied to where mtpt
 * is pointing, up to mtlen characters.
 */
#अगर_घोषित __TURBOC__
 #आशय argsused
#पूर्ण_अगर
पूर्णांक check_mount_point(स्थिर अक्षर *device, पूर्णांक *mount_flags,
				  अक्षर *mtpt, पूर्णांक mtlen)
{
	पूर्णांक	retval = 0;

	अगर (is_swap_device(device)) {
		*mount_flags = MF_MOUNTED | MF_SWAP;
		अगर (mtpt && mtlen)
			xstrncpy(mtpt, "[SWAP]", mtlen);
	} अन्यथा {
#अगर_घोषित HAVE_MNTENT_H
		retval = check_mntent(device, mount_flags, mtpt, mtlen);
#अन्यथा
#अगर_घोषित HAVE_GETMNTINFO
		retval = check_getmntinfo(device, mount_flags, mtpt, mtlen);
#अन्यथा
#अगर_घोषित __GNUC__
 #warning "Can't use getmntent or getmntinfo to check for mounted filesystems!"
#पूर्ण_अगर
		*mount_flags = 0;
#पूर्ण_अगर /* HAVE_GETMNTINFO */
#पूर्ण_अगर /* HAVE_MNTENT_H */
	}
	अगर (retval)
		वापस retval;

#अगर_घोषित __linux__ /* This only works on Linux 2.6+ systems */
	{
		काष्ठा stat st_buf;
		पूर्णांक fd;
		अगर ((stat(device, &st_buf) != 0) ||
		    !S_ISBLK(st_buf.st_mode))
			वापस 0;
		fd = खोलो(device, O_RDONLY|O_EXCL|O_CLOEXEC|O_NONBLOCK);
		अगर (fd < 0) {
			अगर (त्रुटि_सं == EBUSY)
				*mount_flags |= MF_BUSY;
		} अन्यथा
			बंद(fd);
	}
#पूर्ण_अगर

	वापस 0;
}

पूर्णांक is_mounted(स्थिर अक्षर *file)
{
	पूर्णांक	retval;
	पूर्णांक	mount_flags = 0;

	retval = check_mount_point(file, &mount_flags, शून्य, 0);
	अगर (retval)
		वापस 0;
	वापस mount_flags & MF_MOUNTED;
}

#अगर_घोषित TEST_PROGRAM_ISMOUNTED
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक flags = 0;
	अक्षर devname[PATH_MAX];

	अगर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "Usage: %s device\n", argv[0]);
		वापस निकास_त्रुटि;
	}

	अगर (check_mount_point(argv[1], &flags, devname, माप(devname)) == 0 &&
	    (flags & MF_MOUNTED)) {
		अगर (flags & MF_SWAP)
			म_लिखो("used swap device\n");
		अन्यथा
			म_लिखो("mounted on %s\n", devname);
		वापस निकास_सफल;
	}

	म_लिखो("not mounted\n");
	वापस निकास_त्रुटि;
}
#पूर्ण_अगर /* DEBUG */
