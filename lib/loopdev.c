
/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 *
 * -- based on mount/losetup.c
 *
 * Simple library for work with loop devices.
 *
 *  - requires kernel 2.6.x
 *  - reads info from /sys/block/loop<N>/loop/<attr> (new kernels)
 *  - reads info by ioctl
 *  - supports *unlimited* number of loop devices
 *  - supports /dev/loop<N> as well as /dev/loop/<N>
 *  - minimize overhead (fd, loopinfo, ... are shared for all operations)
 *  - setup (associate device and backing file)
 *  - delete (dis-associate file)
 *  - old LOOP_{SET,GET}_STATUS (32bit) ioctls are unsupported
 *  - extendible
 */
#समावेश <stdio.h>
#समावेश <stdint.h>
#समावेश <string.h>
#समावेश <ctype.h>
#समावेश <fcntl.h>
#समावेश <stdlib.h>
#समावेश <unistd.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/stat.h>
#समावेश <sys/mman.h>
#समावेश <inttypes.h>
#समावेश <dirent.h>

#समावेश "linux_version.h"
#समावेश "c.h"
#समावेश "timeutils.h"
#समावेश "sysfs.h"
#समावेश "pathnames.h"
#समावेश "loopdev.h"
#समावेश "canonicalize.h"
#समावेश "blkdev.h"
#समावेश "debug.h"
#समावेश "fileutils.h"

#घोषणा LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
अटल UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा LOOPDEV_DEBUG_INIT	(1 << 1)
#घोषणा LOOPDEV_DEBUG_CXT	(1 << 2)
#घोषणा LOOPDEV_DEBUG_ITER	(1 << 3)
#घोषणा LOOPDEV_DEBUG_SETUP	(1 << 4)

#घोषणा DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

अटल व्योम loopdev_init_debug(व्योम)
{
	अगर (loopdev_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#घोषणा loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#घोषणा loopcxt_sysfs_available(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#घोषणा repeat_on_eagain(x) __extension__ ({			\
		पूर्णांक _c = 0, _e;					\
		करो {						\
			त्रुटि_सं = 0;				\
			_e = x;					\
			अगर (_e == 0 || त्रुटि_सं != EAGAIN)		\
				अवरोध;				\
			अगर (_c >= LOOPDEV_MAX_TRIES)		\
				अवरोध;				\
			xusleep(250000);			\
			_c++;					\
		} जबतक (1);					\
		_e == 0 ? 0 : त्रुटि_सं ? -त्रुटि_सं : -1;		\
	})

/*
 * @lc: context
 * @device: device name, absolute device path or NULL to reset the current setting
 *
 * Sets device, absolute paths (e.g. "/dev/loop<N>") are unchanged, device
 * names ("loop<N>") are converted to the path (/dev/loop<N> or to
 * /dev/loop/<N>)
 *
 * This sets the device name, but does not check if the device exists!
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_set_device(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *device)
{
	अगर (!lc)
		वापस -EINVAL;

	अगर (lc->fd >= 0) {
		बंद(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_lost = 0;
	lc->devno = 0;
	lc->mode = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	स_रखो(&lc->config, 0, माप(lc->config));

	/* set new */
	अगर (device) {
		अगर (*device != '/') {
			स्थिर अक्षर *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			अगर (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				अगर (म_माप(device) < 5)
					वापस -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, माप(lc->device), "%s%s",
				dir, device);
		} अन्यथा
			xstrncpy(lc->device, device, माप(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = शून्य;
	वापस 0;
}

पूर्णांक loopcxt_has_device(काष्ठा loopdev_cxt *lc)
{
	वापस lc && *lc->device;
}

dev_t loopcxt_get_devno(काष्ठा loopdev_cxt *lc)
{
	अगर (!lc || !loopcxt_has_device(lc))
		वापस 0;
	अगर (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	वापस lc->devno;
}

पूर्णांक loopcxt_is_lost(काष्ठा loopdev_cxt *lc)
{
	अगर (!lc || !loopcxt_has_device(lc))
		वापस 0;
	अगर (lc->is_lost)
		वापस 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	वापस lc->is_lost;
}

/*
 * @lc: context
 * @flags: LOOPDEV_FL_* flags
 *
 * Initialize loop handler.
 *
 * We have two sets of the flags:
 *
 *	* LOOPDEV_FL_* flags control loopcxt_* API behavior
 *
 *	* LO_FLAGS_* are kernel flags used for LOOP_{SET,GET}_STAT64 ioctls
 *
 * Returns: <0 on error, 0 on success.
 */
पूर्णांक loopcxt_init(काष्ठा loopdev_cxt *lc, पूर्णांक flags)
{
	पूर्णांक rc;
	काष्ठा stat st;
	काष्ठा loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	अगर (!lc)
		वापस -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	स_नकल(lc, &dummy, माप(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, शून्य);
	अगर (rc)
		वापस rc;

	अगर (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	अगर (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	अगर (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	वापस 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
व्योम loopcxt_deinit(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक errsv = त्रुटि_सं;

	अगर (!lc)
		वापस;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	मुक्त(lc->filename);
	lc->filename = शून्य;

	ignore_परिणाम( loopcxt_set_device(lc, शून्य) );
	loopcxt_deinit_iterator(lc);

	त्रुटि_सं = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
अक्षर *loopcxt_strdup_device(काष्ठा loopdev_cxt *lc)
{
	अगर (!lc || !*lc->device)
		वापस शून्य;
	वापस strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
स्थिर अक्षर *loopcxt_get_device(काष्ठा loopdev_cxt *lc)
{
	वापस lc && *lc->device ? lc->device : शून्य;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
अटल काष्ठा path_cxt *loopcxt_get_sysfs(काष्ठा loopdev_cxt *lc)
{
	अगर (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		वापस शून्य;

	अगर (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		अगर (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			वापस शून्य;
		}

		lc->sysfs = ul_new_sysfs_path(devno, शून्य, शून्य);
		अगर (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	वापस lc->sysfs;
}

अटल पूर्णांक __loopcxt_get_fd(काष्ठा loopdev_cxt *lc, mode_t mode)
{
	पूर्णांक old = -1;

	अगर (!lc || !*lc->device)
		वापस -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	अगर (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	अगर (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = खोलो(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		अगर (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	अगर (old >= 0)
		बंद(old);
	वापस lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
पूर्णांक loopcxt_get_fd(काष्ठा loopdev_cxt *lc)
{
	वापस __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_init_iterator(काष्ठा loopdev_cxt *lc, पूर्णांक flags)
{
	काष्ठा loopdev_iter *iter;
	काष्ठा stat st;

	अगर (!lc)
		वापस -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	स_रखो(iter, 0, माप(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	अगर (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		अगर (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	वापस 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_deinit_iterator(काष्ठा loopdev_cxt *lc)
{
	काष्ठा loopdev_iter *iter;

	अगर (!lc)
		वापस -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	मुक्त(iter->minors);
	अगर (iter->proc)
		ख_बंद(iter->proc);
	अगर (iter->sysblock)
		बंद_सूची(iter->sysblock);

	स_रखो(iter, 0, माप(*iter));
	वापस 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
अटल पूर्णांक loopiter_set_device(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *device)
{
	पूर्णांक rc = loopcxt_set_device(lc, device);
	पूर्णांक used;

	अगर (rc)
		वापस rc;

	अगर (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		वापस 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, शून्य) == 0;

	अगर ((lc->iter.flags & LOOPITER_FL_USED) && used)
		वापस 0;

	अगर ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		वापस 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_परिणाम( loopcxt_set_device(lc, शून्य) );
	वापस 1;
}

अटल पूर्णांक cmpnum(स्थिर व्योम *p1, स्थिर व्योम *p2)
{
	वापस (((* (स्थिर पूर्णांक *) p1) > (* (स्थिर पूर्णांक *) p2)) -
			((* (स्थिर पूर्णांक *) p1) < (* (स्थिर पूर्णांक *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
अटल पूर्णांक loop_scandir(स्थिर अक्षर *स_नाम, पूर्णांक **ary, पूर्णांक hasprefix)
{
	सूची *dir;
	काष्ठा dirent *d;
	अचिन्हित पूर्णांक n, गिनती = 0, arylen = 0;

	अगर (!स_नाम || !ary)
		वापस 0;

	DBG(ITER, ul_debug("scan dir: %s", स_नाम));

	dir = सूची_खोलो(स_नाम);
	अगर (!dir)
		वापस 0;
	मुक्त(*ary);
	*ary = शून्य;

	जबतक((d = सूची_पढ़ो(dir))) {
#अगर_घोषित _DIRENT_HAVE_D_TYPE
		अगर (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			जारी;
#पूर्ण_अगर
		अगर (is_dotdir_dirent(d))
			जारी;

		अगर (hasprefix) {
			/* /dev/loop<N> */
			अगर (माला_पूछो(d->d_name, "loop%u", &n) != 1)
				जारी;
		} अन्यथा {
			/* /dev/loop/<N> */
			अक्षर *end = शून्य;

			त्रुटि_सं = 0;
			n = म_से_दीर्घ(d->d_name, &end, 10);
			अगर (d->d_name == end || (end && *end) || त्रुटि_सं)
				जारी;
		}
		अगर (n < LOOPDEV_DEFAULT_NNODES)
			जारी;			/* ignore loop<0..7> */

		अगर (गिनती + 1 > arylen) {
			पूर्णांक *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, माप(पूर्णांक));
			अगर (!tmp) {
				मुक्त(*ary);
				*ary = शून्य;
				बंद_सूची(dir);
				वापस -1;
			}
			*ary = tmp;
		}
		अगर (*ary)
			(*ary)[गिनती++] = n;
	}
	अगर (गिनती && *ary)
		क्विक(*ary, गिनती, माप(पूर्णांक), cmpnum);

	बंद_सूची(dir);
	वापस गिनती;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
अटल पूर्णांक loopcxt_अग्र_from_proc(काष्ठा loopdev_cxt *lc)
{
	काष्ठा loopdev_iter *iter = &lc->iter;
	अक्षर buf[बफ_मान];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	अगर (!iter->proc)
		iter->proc = ख_खोलो(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	अगर (!iter->proc)
		वापस 1;

	जबतक (ख_माला_लो(buf, माप(buf), iter->proc)) {
		अचिन्हित पूर्णांक m;
		अक्षर name[128 + 1];


		अगर (माला_पूछो(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			जारी;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		अगर (loopiter_set_device(lc, name) == 0)
			वापस 0;
	}

	वापस 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
अटल पूर्णांक loopcxt_अग्र_from_sysfs(काष्ठा loopdev_cxt *lc)
{
	काष्ठा loopdev_iter *iter = &lc->iter;
	काष्ठा dirent *d;
	पूर्णांक fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	अगर (!iter->sysblock)
		iter->sysblock = सूची_खोलो(_PATH_SYS_BLOCK);

	अगर (!iter->sysblock)
		वापस 1;

	fd = dirfd(iter->sysblock);

	जबतक ((d = सूची_पढ़ो(iter->sysblock))) {
		अक्षर name[NAME_MAX + 18 + 1];
		काष्ठा stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		अगर (is_dotdir_dirent(d) || म_भेदन(d->d_name, "loop", 4) != 0)
			जारी;

		snprintf(name, माप(name), "%s/loop/backing_file", d->d_name);
		अगर (fstatat(fd, name, &st, 0) != 0)
			जारी;

		अगर (loopiter_set_device(lc, d->d_name) == 0)
			वापस 0;
	}

	वापस 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
पूर्णांक loopcxt_अग्र(काष्ठा loopdev_cxt *lc)
{
	काष्ठा loopdev_iter *iter;

	अगर (!lc)
		वापस -EINVAL;


	iter = &lc->iter;
	अगर (iter->done)
		वापस 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	अगर (iter->flags & LOOPITER_FL_USED) {
		पूर्णांक rc;

		अगर (loopcxt_sysfs_available(lc))
			rc = loopcxt_अग्र_from_sysfs(lc);
		अन्यथा
			rc = loopcxt_अग्र_from_proc(lc);
		अगर (rc == 0)
			वापस 0;
		जाओ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	अगर (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		क्रम (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			अक्षर name[16];
			snprintf(name, माप(name), "loop%d", iter->ncur);

			अगर (loopiter_set_device(lc, name) == 0)
				वापस 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	अगर (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	क्रम (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		अक्षर name[16];
		snprintf(name, माप(name), "loop%d", iter->minors[iter->ncur]);

		अगर (loopiter_set_device(lc, name) == 0)
			वापस 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	वापस 1;
}

/*
 * @device: path to device
 */
पूर्णांक is_loopdev(स्थिर अक्षर *device)
{
	काष्ठा stat st;
	पूर्णांक rc = 0;

	अगर (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	अन्यथा अगर (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	अन्यथा अगर (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		अक्षर name[PATH_MAX], *cn, *p = शून्य;

		snprintf(name, माप(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		अगर (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		मुक्त(cn);
	}

	अगर (rc == 0)
		त्रुटि_सं = ENODEV;
	वापस rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
काष्ठा loop_info64 *loopcxt_get_info(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक fd;

	अगर (!lc || lc->info_failed) {
		त्रुटि_सं = EINVAL;
		वापस शून्य;
	}
	त्रुटि_सं = 0;
	अगर (lc->has_info)
		वापस &lc->config.info;

	fd = loopcxt_get_fd(lc);
	अगर (fd < 0)
		वापस शून्य;

	अगर (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		वापस &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	वापस शून्य;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
अक्षर *loopcxt_get_backing_file(काष्ठा loopdev_cxt *lc)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);
	अक्षर *res = शून्य;

	अगर (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	अगर (!res && loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);

		अगर (lo) {
			lo->lo_file_name[LO_NAME_आकार - 2] = '*';
			lo->lo_file_name[LO_NAME_आकार - 1] = '\0';
			res = strdup((अक्षर *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	वापस res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
अक्षर *loopcxt_get_refname(काष्ठा loopdev_cxt *lc)
{
	अक्षर *res = शून्य;
	काष्ठा loop_info64 *lo = loopcxt_get_info(lc);

	अगर (lo) {
		lo->lo_file_name[LO_NAME_आकार - 1] = '\0';
		res = strdup((अक्षर *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	वापस res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_offset(काष्ठा loopdev_cxt *lc, uint64_t *offset)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णांक rc = -EINVAL;

	अगर (sysfs)
		अगर (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	अगर (rc && loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
		अगर (lo) {
			अगर (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} अन्यथा
			rc = -त्रुटि_सं;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	वापस rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_blocksize(काष्ठा loopdev_cxt *lc, uint64_t *blocksize)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णांक rc = -EINVAL;

	अगर (sysfs)
		अगर (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	अगर (rc) {
		पूर्णांक fd = loopcxt_get_fd(lc);
		पूर्णांक sz = 0;

		अगर (fd < 0)
			वापस -EINVAL;
		rc = blkdev_get_sector_आकार(fd, &sz);
		अगर (rc)
			वापस rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	वापस rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_sizelimit(काष्ठा loopdev_cxt *lc, uint64_t *आकार)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णांक rc = -EINVAL;

	अगर (sysfs)
		अगर (ul_path_read_u64(sysfs, आकार, "loop/sizelimit") == 0)
			rc = 0;

	अगर (rc && loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
		अगर (lo) {
			अगर (आकार)
				*आकार = lo->lo_sizelimit;
			rc = 0;
		} अन्यथा
			rc = -त्रुटि_सं;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	वापस rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_encrypt_type(काष्ठा loopdev_cxt *lc, uint32_t *type)
{
	काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णांक rc;

	/* not provided by sysfs */
	अगर (lo) {
		अगर (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	वापस rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
स्थिर अक्षर *loopcxt_get_crypt_name(काष्ठा loopdev_cxt *lc)
{
	काष्ठा loop_info64 *lo = loopcxt_get_info(lc);

	अगर (lo)
		वापस (अक्षर *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	वापस शून्य;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_backing_devno(काष्ठा loopdev_cxt *lc, dev_t *devno)
{
	काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णांक rc;

	अगर (lo) {
		अगर (devno)
			*devno = lo->lo_device;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	वापस rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णांक loopcxt_get_backing_inode(काष्ठा loopdev_cxt *lc, ino_t *ino)
{
	काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णांक rc;

	अगर (lo) {
		अगर (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	वापस rc;
}

/*
 * Check if the kernel supports partitioned loop devices.
 *
 * Notes:
 *   - kernels < 3.2 support partitioned loop devices and PT scanning
 *     only if max_part= module parameter is non-zero
 *
 *   - kernels >= 3.2 always support partitioned loop devices
 *
 *   - kernels >= 3.2 always support BLKPG_{ADD,DEL}_PARTITION ioctls
 *
 *   - kernels >= 3.2 enable PT scanner only if max_part= is non-zero or if the
 *     LO_FLAGS_PARTSCAN flag is set for the device. The PT scanner is disabled
 *     by default.
 *
 *  See kernel commit e03c8dd14915fabc101aa495828d58598dc5af98.
 */
पूर्णांक loopmod_supports_partscan(व्योम)
{
	पूर्णांक rc, ret = 0;
	खाता *f;

	अगर (get_linux_version() >= KERNEL_VERSION(3,2,0))
		वापस 1;

	f = ख_खोलो("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	अगर (!f)
		वापस 0;
	rc = ख_पूछो(f, "%d", &ret);
	ख_बंद(f);
	वापस rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
पूर्णांक loopcxt_is_partscan(काष्ठा loopdev_cxt *lc)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);

	अगर (sysfs) {
		/* kernel >= 3.2 */
		पूर्णांक fl;
		अगर (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			वापस fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	वापस loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
पूर्णांक loopcxt_is_autoclear(काष्ठा loopdev_cxt *lc)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);

	अगर (sysfs) {
		पूर्णांक fl;
		अगर (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			वापस fl;
	}

	अगर (loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
		अगर (lo)
			वापस lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	वापस 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
पूर्णांक loopcxt_is_readonly(काष्ठा loopdev_cxt *lc)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);

	अगर (sysfs) {
		पूर्णांक fl;
		अगर (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			वापस fl;
	}

	अगर (loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
		अगर (lo)
			वापस lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	वापस 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
पूर्णांक loopcxt_is_dio(काष्ठा loopdev_cxt *lc)
{
	काष्ठा path_cxt *sysfs = loopcxt_get_sysfs(lc);

	अगर (sysfs) {
		पूर्णांक fl;
		अगर (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			वापस fl;
	}
	अगर (loopcxt_ioctl_enabled(lc)) {
		काष्ठा loop_info64 *lo = loopcxt_get_info(lc);
		अगर (lo)
			वापस lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	वापस 0;
}

/*
 * @lc: context
 * @st: backing file stat or NULL
 * @backing_file: filename
 * @offset: offset (use LOOPDEV_FL_OFFSET if specified)
 * @sizelimit: size limit (use LOOPDEV_FL_SIZELIMIT if specified)
 * @flags: LOOPDEV_FL_{OFFSET,SIZELIMIT}
 *
 * Returns 1 if the current @lc loopdev is associated with the given backing
 * file. Note that the preferred way is to use devno and inode number rather
 * than filename. The @backing_file filename is poor solution usable in case
 * that you don't have rights to call stat().
 *
 * LOOPDEV_FL_SIZELIMIT requires LOOPDEV_FL_OFFSET being set as well.
 *
 * Don't forget that old kernels provide very restricted (in size) backing
 * filename by LOOP_GET_STAT64 ioctl only.
 */
पूर्णांक loopcxt_is_used(काष्ठा loopdev_cxt *lc,
		    काष्ठा stat *st,
		    स्थिर अक्षर *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    पूर्णांक flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	अगर (!lc)
		वापस 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	अगर (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		अगर (ino == st->st_ino && dev == st->st_dev)
			जाओ found;

		/* don't use filename if we have devno and inode */
		वापस 0;
	}

	/* poor man's solution */
	अगर (backing_file) {
		अक्षर *name = loopcxt_get_backing_file(lc);
		पूर्णांक rc = name && म_भेद(name, backing_file) == 0;

		मुक्त(name);
		अगर (rc)
			जाओ found;
	}

	वापस 0;
found:
	अगर (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		पूर्णांक rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		अगर (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			वापस loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		वापस rc;
	}
	वापस 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णांक loopcxt_set_offset(काष्ठा loopdev_cxt *lc, uint64_t offset)
{
	अगर (!lc)
		वापस -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	वापस 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णांक loopcxt_set_sizelimit(काष्ठा loopdev_cxt *lc, uint64_t sizelimit)
{
	अगर (!lc)
		वापस -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	वापस 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णांक loopcxt_set_blocksize(काष्ठा loopdev_cxt *lc, uint64_t blocksize)
{
	अगर (!lc)
		वापस -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	वापस 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक loopcxt_set_flags(काष्ठा loopdev_cxt *lc, uint32_t flags)
{
	अगर (!lc)
		वापस -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (अचिन्हित) flags));
	वापस 0;
}

/*
 * @lc: context
 * @refname: reference name (used to overwrite lo_file_name where is backing
 *           file by default)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक loopcxt_set_refname(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *refname)
{
	अगर (!lc)
		वापस -EINVAL;

	स_रखो(lc->config.info.lo_file_name, 0, माप(lc->config.info.lo_file_name));
	अगर (refname)
		xstrncpy((अक्षर *)lc->config.info.lo_file_name, refname, LO_NAME_आकार);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (अक्षर *)lc->config.info.lo_file_name));
	वापस 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णांक loopcxt_set_backing_file(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *filename)
{
	अगर (!lc)
		वापस -EINVAL;

	अगर (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	अन्यथा
		lc->filename = ul_canonicalize_path(filename);
	अगर (!lc->filename)
		वापस -त्रुटि_सं;

	अगर (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	वापस 0;
}

/*
 * In kernels prior to v3.9, if the offset or sizelimit options
 * are used, the block device's size won't be synced automatically.
 * blockdev --getsize64 and filesystems will use the backing
 * file size until the block device has been re-opened or the
 * LOOP_SET_CAPACITY ioctl is called to sync the sizes.
 *
 * Since mount -oloop uses the LO_FLAGS_AUTOCLEAR option and passes
 * the open file descriptor to the mount system call, we need to use
 * the ioctl. Calling losetup directly doesn't have this problem since
 * it closes the device when it exits and whatever consumes the device
 * next will re-open it, causing the resync.
 */
अटल पूर्णांक loopcxt_check_आकार(काष्ठा loopdev_cxt *lc, पूर्णांक file_fd)
{
	uint64_t आकार, expected_आकार;
	पूर्णांक dev_fd;
	काष्ठा stat st;

	अगर (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		वापस 0;

	अगर (ख_स्थिति(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		वापस -त्रुटि_सं;
	}
	अगर (S_ISBLK(st.st_mode)) {
		अगर (blkdev_get_आकार(file_fd, &expected_आकार)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			वापस -त्रुटि_सं;
		}
	} अन्यथा
		expected_आकार = st.st_size;

	अगर (expected_आकार == 0 || expected_आकार <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		वापस 0;	/* ignore this error */
	}

	अगर (lc->config.info.lo_offset > 0)
		expected_आकार -= lc->config.info.lo_offset;

	अगर (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_आकार)
		expected_आकार = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	अगर (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		वापस -त्रुटि_सं;
	}

	अगर (blkdev_get_आकार(dev_fd, &आकार)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		वापस -त्रुटि_सं;
	}

	/* It's block device, so, align to 512-byte sectors */
	अगर (expected_आकार % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_आकार = (expected_आकार >> 9) << 9;
	}

	अगर (expected_आकार != आकार) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      आकार, expected_आकार));

		अगर (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			अगर (त्रुटि_सं == ENOTTY || त्रुटि_सं == EINVAL)
				त्रुटि_सं = दुस्फल;
			वापस -त्रुटि_सं;
		}

		अगर (blkdev_get_आकार(dev_fd, &आकार))
			वापस -त्रुटि_सं;

		अगर (expected_आकार != आकार) {
			त्रुटि_सं = दुस्फल;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					आकार, expected_आकार));
			वापस -त्रुटि_सं;
		}
	}

	वापस 0;
}


/*
 * @lc: context
 *
 * Associate the current device (see loopcxt_{set,get}_device()) with
 * a file (see loopcxt_set_backing_file()).
 *
 * The device is initialized read-write by default. If you want read-only
 * device then set LO_FLAGS_READ_ONLY by loopcxt_set_flags(). The LOOPDEV_FL_*
 * flags are ignored and modified according to LO_FLAGS_*.
 *
 * If the device is already open by loopcxt_get_fd() then this setup device
 * function will re-open the device to fix read/write mode.
 *
 * The device is also initialized read-only if the backing file is not
 * possible to open read-write (e.g. read-only FS).
 *
 * Returns: <0 on error, 0 on success.
 */
पूर्णांक loopcxt_setup_device(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	पूर्णांक rc = -1, cnt = 0;
	पूर्णांक errsv = 0;
	पूर्णांक fallback = 0;

	अगर (!lc || !*lc->device || !lc->filename)
		वापस -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	अगर (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	अगर (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	अगर (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	अन्यथा
		file_fd = खोलो(lc->filename, mode | flags);

	अगर (file_fd < 0 && mode != O_RDONLY
	    && (त्रुटि_सं == EROFS || त्रुटि_सं == EACCES)) {
		mode = O_RDONLY;
		अगर (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		अन्यथा
			file_fd = खोलो(lc->filename, mode | flags);
	}
	अगर (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		वापस -त्रुटि_सं;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	अगर (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	अन्यथा
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	करो {
		त्रुटि_सं = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		अगर (dev_fd >= 0 || lc->control_ok == 0)
			अवरोध;
		अगर (त्रुटि_सं != EACCES && त्रुटि_सं != ENOENT)
			अवरोध;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} जबतक (cnt++ < 16);

	अगर (dev_fd < 0) {
		rc = -त्रुटि_सं;
		जाओ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	अगर (lc->blocksize > 0)
		lc->config.block_आकार = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	अगर (rc != 0) {
		errsv = त्रुटि_सं;
		अगर (त्रुटि_सं != EINVAL && त्रुटि_सं != ENOTTY && त्रुटि_सं != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			जाओ err;
		}
		fallback = 1;
	} अन्यथा {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	अगर (fallback) {
		अगर (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -त्रुटि_सं;
			errsv = त्रुटि_सं;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			जाओ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		अगर (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			जाओ err;
		}

		अगर ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			जाओ err;
		}
	}

	अगर ((rc = loopcxt_check_आकार(lc, file_fd)))
		जाओ err;

	बंद(file_fd);

	स_रखो(&lc->config, 0, माप(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	वापस 0;
err:
	अगर (file_fd >= 0)
		बंद(file_fd);
	अगर (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	अगर (errsv)
		त्रुटि_सं = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	वापस rc;
}


/*
 * @lc: context
 *
 * Update status of the current device (see loopcxt_{set,get}_device()).
 *
 * Note that once initialized, kernel accepts only selected changes:
 * LO_FLAGS_AUTOCLEAR and LO_FLAGS_PARTSCAN
 * For more see linux/drivers/block/loop.c:loop_set_status()
 *
 * Returns: <0 on error, 0 on success.
 */
पूर्णांक loopcxt_ioctl_status(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक dev_fd, rc;

	त्रुटि_सं = 0;
	dev_fd = loopcxt_get_fd(lc);

	अगर (dev_fd < 0)
		वापस -त्रुटि_सं;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	अगर (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		वापस rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	वापस 0;
}

पूर्णांक loopcxt_ioctl_capacity(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक rc, fd = loopcxt_get_fd(lc);

	अगर (fd < 0)
		वापस -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	अगर (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		वापस rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	वापस 0;
}

पूर्णांक loopcxt_ioctl_dio(काष्ठा loopdev_cxt *lc, अचिन्हित दीर्घ use_dio)
{
	पूर्णांक rc, fd = loopcxt_get_fd(lc);

	अगर (fd < 0)
		वापस -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	अगर (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		वापस rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	वापस 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
पूर्णांक loopcxt_ioctl_blocksize(काष्ठा loopdev_cxt *lc, uint64_t blocksize)
{
	पूर्णांक rc, fd = loopcxt_get_fd(lc);

	अगर (fd < 0)
		वापस -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_आकार, (अचिन्हित दीर्घ) blocksize) );
	अगर (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		वापस rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	वापस 0;
}

/*
 * @lc: context
 * @nr: returns loop device number
 *
 * Extracts the loop device number from the device path.
 * Supports both /dev/loop<N> and /dev/loop/<N> formats.
 *
 * Returns: 0 on success, <0 on error
 */
अटल पूर्णांक loopcxt_get_device_nr(काष्ठा loopdev_cxt *lc, पूर्णांक *nr)
{
	स्थिर अक्षर *p, *dev;
	पूर्णांक rc = -EINVAL;

	त्रुटि_सं = 0;
	अगर (!lc || !nr)
		वापस rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	अगर (!dev)
		जाओ done;

	p = म_खोजप(dev, '/');
	अगर (!p)
		जाओ done;

	अगर (माला_पूछो(p, "/loop%d", nr) != 1 && माला_पूछो(p, "/%d", nr) != 1)
		जाओ done;

	अगर (*nr < 0)
		जाओ done;
	rc = 0;
done:
	अगर (rc && !त्रुटि_सं)
		त्रुटि_सं = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	वापस rc;
}

पूर्णांक loopcxt_detach_device(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक rc, fd = loopcxt_get_fd(lc);

	अगर (fd < 0)
		वापस -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	अगर (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		वापस rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	वापस 0;
}

पूर्णांक loopcxt_remove_device(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक rc = -EINVAL;
	पूर्णांक ctl, nr = -1;

	अगर (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		जाओ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	अगर (rc)
		जाओ done;

	ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	अगर (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		बंद(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	वापस rc;
}

पूर्णांक loopcxt_add_device(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक rc = -EINVAL;
	पूर्णांक ctl, nr = -1;

	अगर (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		जाओ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	अगर (rc)
		जाओ done;

	ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	अगर (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		बंद(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	वापस rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
पूर्णांक loopcxt_find_unused(काष्ठा loopdev_cxt *lc)
{
	पूर्णांक rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	अगर (lc->flags & LOOPDEV_FL_CONTROL) {
		पूर्णांक ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		अगर (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		अन्यथा
			rc = -त्रुटि_सं;
		अगर (rc >= 0) {
			अक्षर name[16];
			snprintf(name, माप(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		अगर (ctl >= 0)
			बंद(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	अगर (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		अगर (rc)
			वापस rc;

		rc = loopcxt_अग्र(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		अगर (rc)
			वापस -ENOENT;
	}
	वापस rc;
}



/*
 * Return: TRUE/FALSE
 */
पूर्णांक loopdev_is_autoclear(स्थिर अक्षर *device)
{
	काष्ठा loopdev_cxt lc;
	पूर्णांक rc;

	अगर (!device)
		वापस 0;

	rc = loopcxt_init(&lc, 0);
	अगर (!rc)
		rc = loopcxt_set_device(&lc, device);
	अगर (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	वापस rc;
}

अक्षर *loopdev_get_backing_file(स्थिर अक्षर *device)
{
	काष्ठा loopdev_cxt lc;
	अक्षर *res = शून्य;

	अगर (!device)
		वापस शून्य;
	अगर (loopcxt_init(&lc, 0))
		वापस शून्य;
	अगर (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	वापस res;
}

/*
 * Returns: TRUE/FALSE
 */
पूर्णांक loopdev_has_backing_file(स्थिर अक्षर *device)
{
	अक्षर *tmp = loopdev_get_backing_file(device);

	अगर (tmp) {
		मुक्त(tmp);
		वापस 1;
	}
	वापस 0;
}

/*
 * Returns: TRUE/FALSE
 */
पूर्णांक loopdev_is_used(स्थिर अक्षर *device, स्थिर अक्षर *filename,
		    uint64_t offset, uint64_t sizelimit, पूर्णांक flags)
{
	काष्ठा loopdev_cxt lc;
	काष्ठा stat st;
	पूर्णांक rc = 0;

	अगर (!device || !filename)
		वापस 0;

	rc = loopcxt_init(&lc, 0);
	अगर (!rc)
		rc = loopcxt_set_device(&lc, device);
	अगर (rc)
		वापस rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : शून्य, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	वापस rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
पूर्णांक loopdev_detach(स्थिर अक्षर *device)
{
	काष्ठा loopdev_cxt lc;
	पूर्णांक rc;

	अगर (!device)
		वापस -EINVAL;

	rc = loopcxt_init(&lc, 0);
	अगर (!rc)
		rc = loopcxt_set_device(&lc, device);
	अगर (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	वापस rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
पूर्णांक loopcxt_find_by_backing_file(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *filename,
				 uint64_t offset, uint64_t sizelimit, पूर्णांक flags)
{
	पूर्णांक rc, hasst;
	काष्ठा stat st;

	अगर (!filename)
		वापस -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	अगर (rc)
		वापस rc;

	जबतक ((rc = loopcxt_अग्र(lc)) == 0) {

		अगर (loopcxt_is_used(lc, hasst ? &st : शून्य,
				    filename, offset, sizelimit, flags))
			अवरोध;
	}

	loopcxt_deinit_iterator(lc);
	वापस rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
पूर्णांक loopcxt_find_overlap(काष्ठा loopdev_cxt *lc, स्थिर अक्षर *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	पूर्णांक rc, hasst;
	काष्ठा stat st;

	अगर (!filename)
		वापस -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	अगर (rc)
		वापस rc;

	जबतक ((rc = loopcxt_अग्र(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : शून्य,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		अगर (rc <= 0)
			जारी;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		अगर (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			जारी;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		अगर (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			जारी;
		}

		/* full match */
		अगर (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			जाओ found;
		}

		/* overlap */
		अगर (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			जारी;
		अगर (sizelimit != 0 && offset + sizelimit <= lc_offset)
			जारी;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			जाओ found;
	}

	अगर (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	वापस rc;
}

/*
 * Returns allocated string with device name
 */
अक्षर *loopdev_find_by_backing_file(स्थिर अक्षर *filename, uint64_t offset, uint64_t sizelimit, पूर्णांक flags)
{
	काष्ठा loopdev_cxt lc;
	अक्षर *res = शून्य;

	अगर (!filename)
		वापस शून्य;

	अगर (loopcxt_init(&lc, 0))
		वापस शून्य;
	अगर (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	वापस res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
पूर्णांक loopdev_गिनती_by_backing_file(स्थिर अक्षर *filename, अक्षर **loopdev)
{
	काष्ठा loopdev_cxt lc;
	पूर्णांक गिनती = 0, rc;

	अगर (!filename)
		वापस -1;

	rc = loopcxt_init(&lc, 0);
	अगर (rc)
		वापस rc;
	अगर (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		वापस -1;

	जबतक(loopcxt_अग्र(&lc) == 0) {
		अक्षर *backing = loopcxt_get_backing_file(&lc);

		अगर (!backing || म_भेद(backing, filename) != 0) {
			मुक्त(backing);
			जारी;
		}

		मुक्त(backing);
		अगर (loopdev && गिनती == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		गिनती++;
	}

	loopcxt_deinit(&lc);

	अगर (loopdev && गिनती > 1) {
		मुक्त(*loopdev);
		*loopdev = शून्य;
	}
	वापस गिनती;
}

#अगर_घोषित TEST_PROGRAM_LOOPDEV
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अगर (argc < 2)
		जाओ usage;

	अगर (म_भेद(argv[1], "--is-loopdev") == 0 && argc == 3)
		म_लिखो("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	अन्यथा
		जाओ usage;

	वापस निकास_सफल;
usage:
	ख_लिखो(मानक_त्रुटि, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	वापस निकास_त्रुटि;
}
#पूर्ण_अगर

