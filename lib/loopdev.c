
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
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/mman.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <dirent.h>

#अन्तर्भाव "linux_version.h"
#अन्तर्भाव "c.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "sysfs.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "loopdev.h"
#अन्तर्भाव "canonicalize.h"
#अन्तर्भाव "blkdev.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

स्थिर शून्य loopdev_init_debug(शून्य)
{
	यदि (loopdev_debug_mask)
		फर्काऊ;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#define loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#define loopcxt_sysfs_available(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#define repeat_on_eagain(x) __extension__ ({			\
		पूर्णाङ्क _c = 0, _e;					\
		गर {						\
			त्रुटि_सं = 0;				\
			_e = x;					\
			यदि (_e == 0 || त्रुटि_सं != EAGAIN)		\
				रोक;				\
			यदि (_c >= LOOPDEV_MAX_TRIES)		\
				रोक;				\
			xusleep(250000);			\
			_c++;					\
		} जबसम्म (1);					\
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
पूर्णाङ्क loopcxt_set_device(संरचना loopdev_cxt *lc, अचर वर्ण *device)
{
	यदि (!lc)
		फर्काऊ -EINVAL;

	यदि (lc->fd >= 0) {
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
	स_रखो(&lc->config, 0, परिमाण(lc->config));

	/* set new */
	यदि (device) {
		यदि (*device != '/') {
			अचर वर्ण *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			यदि (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				यदि (म_माप(device) < 5)
					फर्काऊ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, परिमाण(lc->device), "%s%s",
				dir, device);
		} अन्यथा
			xstrncpy(lc->device, device, परिमाण(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = रिक्त;
	फर्काऊ 0;
}

पूर्णाङ्क loopcxt_has_device(संरचना loopdev_cxt *lc)
{
	फर्काऊ lc && *lc->device;
}

dev_t loopcxt_get_devno(संरचना loopdev_cxt *lc)
{
	यदि (!lc || !loopcxt_has_device(lc))
		फर्काऊ 0;
	यदि (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	फर्काऊ lc->devno;
}

पूर्णाङ्क loopcxt_is_lost(संरचना loopdev_cxt *lc)
{
	यदि (!lc || !loopcxt_has_device(lc))
		फर्काऊ 0;
	यदि (lc->is_lost)
		फर्काऊ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	फर्काऊ lc->is_lost;
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
पूर्णाङ्क loopcxt_init(संरचना loopdev_cxt *lc, पूर्णाङ्क flags)
{
	पूर्णाङ्क rc;
	संरचना stat st;
	संरचना loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	यदि (!lc)
		फर्काऊ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	स_नकल(lc, &dummy, परिमाण(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, रिक्त);
	यदि (rc)
		फर्काऊ rc;

	यदि (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	यदि (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	यदि (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	फर्काऊ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
शून्य loopcxt_deinit(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क errsv = त्रुटि_सं;

	यदि (!lc)
		फर्काऊ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	मुक्त(lc->filename);
	lc->filename = रिक्त;

	ignore_result( loopcxt_set_device(lc, रिक्त) );
	loopcxt_deinit_iterator(lc);

	त्रुटि_सं = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
वर्ण *loopcxt_strdup_device(संरचना loopdev_cxt *lc)
{
	यदि (!lc || !*lc->device)
		फर्काऊ रिक्त;
	फर्काऊ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
अचर वर्ण *loopcxt_get_device(संरचना loopdev_cxt *lc)
{
	फर्काऊ lc && *lc->device ? lc->device : रिक्त;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
स्थिर संरचना path_cxt *loopcxt_get_sysfs(संरचना loopdev_cxt *lc)
{
	यदि (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		फर्काऊ रिक्त;

	यदि (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		यदि (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			फर्काऊ रिक्त;
		}

		lc->sysfs = ul_new_sysfs_path(devno, रिक्त, रिक्त);
		यदि (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	फर्काऊ lc->sysfs;
}

स्थिर पूर्णाङ्क __loopcxt_get_fd(संरचना loopdev_cxt *lc, mode_t mode)
{
	पूर्णाङ्क old = -1;

	यदि (!lc || !*lc->device)
		फर्काऊ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	यदि (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	यदि (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = खोलो(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		यदि (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	यदि (old >= 0)
		बंद(old);
	फर्काऊ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
पूर्णाङ्क loopcxt_get_fd(संरचना loopdev_cxt *lc)
{
	फर्काऊ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_init_iterator(संरचना loopdev_cxt *lc, पूर्णाङ्क flags)
{
	संरचना loopdev_iter *iter;
	संरचना stat st;

	यदि (!lc)
		फर्काऊ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	स_रखो(iter, 0, परिमाण(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	यदि (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		यदि (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	फर्काऊ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_deinit_iterator(संरचना loopdev_cxt *lc)
{
	संरचना loopdev_iter *iter;

	यदि (!lc)
		फर्काऊ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	मुक्त(iter->minors);
	यदि (iter->proc)
		ख_बंद(iter->proc);
	यदि (iter->sysblock)
		बंद_सूची(iter->sysblock);

	स_रखो(iter, 0, परिमाण(*iter));
	फर्काऊ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
स्थिर पूर्णाङ्क loopiter_set_device(संरचना loopdev_cxt *lc, अचर वर्ण *device)
{
	पूर्णाङ्क rc = loopcxt_set_device(lc, device);
	पूर्णाङ्क used;

	यदि (rc)
		फर्काऊ rc;

	यदि (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		फर्काऊ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, रिक्त) == 0;

	यदि ((lc->iter.flags & LOOPITER_FL_USED) && used)
		फर्काऊ 0;

	यदि ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		फर्काऊ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, रिक्त) );
	फर्काऊ 1;
}

स्थिर पूर्णाङ्क cmpnum(अचर शून्य *p1, अचर शून्य *p2)
{
	फर्काऊ (((* (अचर पूर्णाङ्क *) p1) > (* (अचर पूर्णाङ्क *) p2)) -
			((* (अचर पूर्णाङ्क *) p1) < (* (अचर पूर्णाङ्क *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
स्थिर पूर्णाङ्क loop_scandir(अचर वर्ण *स_नाम, पूर्णाङ्क **ary, पूर्णाङ्क hasprefix)
{
	सूची *dir;
	संरचना dirent *d;
	अचिन्हित पूर्णाङ्क n, गन्ती = 0, arylen = 0;

	यदि (!स_नाम || !ary)
		फर्काऊ 0;

	DBG(ITER, ul_debug("scan dir: %s", स_नाम));

	dir = सूची_खोलो(स_नाम);
	यदि (!dir)
		फर्काऊ 0;
	मुक्त(*ary);
	*ary = रिक्त;

	जबसम्म((d = सूची_पढ़ो(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		यदि (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			जारी;
#endif
		यदि (is_dotdir_dirent(d))
			जारी;

		यदि (hasprefix) {
			/* /dev/loop<N> */
			यदि (माला_पूछो(d->d_name, "loop%u", &n) != 1)
				जारी;
		} अन्यथा {
			/* /dev/loop/<N> */
			वर्ण *end = रिक्त;

			त्रुटि_सं = 0;
			n = म_से_दीर्घ(d->d_name, &end, 10);
			यदि (d->d_name == end || (end && *end) || त्रुटि_सं)
				जारी;
		}
		यदि (n < LOOPDEV_DEFAULT_NNODES)
			जारी;			/* ignore loop<0..7> */

		यदि (गन्ती + 1 > arylen) {
			पूर्णाङ्क *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, परिमाण(पूर्णाङ्क));
			यदि (!tmp) {
				मुक्त(*ary);
				*ary = रिक्त;
				बंद_सूची(dir);
				फर्काऊ -1;
			}
			*ary = tmp;
		}
		यदि (*ary)
			(*ary)[गन्ती++] = n;
	}
	यदि (गन्ती && *ary)
		क्विक(*ary, गन्ती, परिमाण(पूर्णाङ्क), cmpnum);

	बंद_सूची(dir);
	फर्काऊ गन्ती;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
स्थिर पूर्णाङ्क loopcxt_next_from_proc(संरचना loopdev_cxt *lc)
{
	संरचना loopdev_iter *iter = &lc->iter;
	वर्ण buf[बफ_मान];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	यदि (!iter->proc)
		iter->proc = ख_खोलो(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	यदि (!iter->proc)
		फर्काऊ 1;

	जबसम्म (ख_माला_लो(buf, परिमाण(buf), iter->proc)) {
		अचिन्हित पूर्णाङ्क m;
		वर्ण name[128 + 1];


		यदि (माला_पूछो(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			जारी;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		यदि (loopiter_set_device(lc, name) == 0)
			फर्काऊ 0;
	}

	फर्काऊ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
स्थिर पूर्णाङ्क loopcxt_next_from_sysfs(संरचना loopdev_cxt *lc)
{
	संरचना loopdev_iter *iter = &lc->iter;
	संरचना dirent *d;
	पूर्णाङ्क fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	यदि (!iter->sysblock)
		iter->sysblock = सूची_खोलो(_PATH_SYS_BLOCK);

	यदि (!iter->sysblock)
		फर्काऊ 1;

	fd = dirfd(iter->sysblock);

	जबसम्म ((d = सूची_पढ़ो(iter->sysblock))) {
		वर्ण name[NAME_MAX + 18 + 1];
		संरचना stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		यदि (is_dotdir_dirent(d) || म_भेदन(d->d_name, "loop", 4) != 0)
			जारी;

		snprintf(name, परिमाण(name), "%s/loop/backing_file", d->d_name);
		यदि (fstatat(fd, name, &st, 0) != 0)
			जारी;

		यदि (loopiter_set_device(lc, d->d_name) == 0)
			फर्काऊ 0;
	}

	फर्काऊ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
पूर्णाङ्क loopcxt_next(संरचना loopdev_cxt *lc)
{
	संरचना loopdev_iter *iter;

	यदि (!lc)
		फर्काऊ -EINVAL;


	iter = &lc->iter;
	यदि (iter->done)
		फर्काऊ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	यदि (iter->flags & LOOPITER_FL_USED) {
		पूर्णाङ्क rc;

		यदि (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		अन्यथा
			rc = loopcxt_next_from_proc(lc);
		यदि (rc == 0)
			फर्काऊ 0;
		गच्छ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	यदि (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		लागि (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			वर्ण name[16];
			snprintf(name, परिमाण(name), "loop%d", iter->ncur);

			यदि (loopiter_set_device(lc, name) == 0)
				फर्काऊ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	यदि (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	लागि (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		वर्ण name[16];
		snprintf(name, परिमाण(name), "loop%d", iter->minors[iter->ncur]);

		यदि (loopiter_set_device(lc, name) == 0)
			फर्काऊ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	फर्काऊ 1;
}

/*
 * @device: path to device
 */
पूर्णाङ्क is_loopdev(अचर वर्ण *device)
{
	संरचना stat st;
	पूर्णाङ्क rc = 0;

	यदि (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	अन्यथा यदि (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	अन्यथा यदि (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		वर्ण name[PATH_MAX], *cn, *p = रिक्त;

		snprintf(name, परिमाण(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		यदि (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		मुक्त(cn);
	}

	यदि (rc == 0)
		त्रुटि_सं = ENODEV;
	फर्काऊ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
संरचना loop_info64 *loopcxt_get_info(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क fd;

	यदि (!lc || lc->info_failed) {
		त्रुटि_सं = EINVAL;
		फर्काऊ रिक्त;
	}
	त्रुटि_सं = 0;
	यदि (lc->has_info)
		फर्काऊ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	यदि (fd < 0)
		फर्काऊ रिक्त;

	यदि (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		फर्काऊ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	फर्काऊ रिक्त;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
वर्ण *loopcxt_get_backing_file(संरचना loopdev_cxt *lc)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);
	वर्ण *res = रिक्त;

	यदि (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	यदि (!res && loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);

		यदि (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((वर्ण *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	फर्काऊ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
वर्ण *loopcxt_get_refname(संरचना loopdev_cxt *lc)
{
	वर्ण *res = रिक्त;
	संरचना loop_info64 *lo = loopcxt_get_info(lc);

	यदि (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((वर्ण *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	फर्काऊ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_offset(संरचना loopdev_cxt *lc, uint64_t *offset)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णाङ्क rc = -EINVAL;

	यदि (sysfs)
		यदि (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	यदि (rc && loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);
		यदि (lo) {
			यदि (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} अन्यथा
			rc = -त्रुटि_सं;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_blocksize(संरचना loopdev_cxt *lc, uint64_t *blocksize)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णाङ्क rc = -EINVAL;

	यदि (sysfs)
		यदि (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	यदि (rc) {
		पूर्णाङ्क fd = loopcxt_get_fd(lc);
		पूर्णाङ्क sz = 0;

		यदि (fd < 0)
			फर्काऊ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		यदि (rc)
			फर्काऊ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_sizelimit(संरचना loopdev_cxt *lc, uint64_t *size)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);
	पूर्णाङ्क rc = -EINVAL;

	यदि (sysfs)
		यदि (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	यदि (rc && loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);
		यदि (lo) {
			यदि (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} अन्यथा
			rc = -त्रुटि_सं;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_encrypt_type(संरचना loopdev_cxt *lc, uint32_t *type)
{
	संरचना loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णाङ्क rc;

	/* not provided by sysfs */
	यदि (lo) {
		यदि (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
अचर वर्ण *loopcxt_get_crypt_name(संरचना loopdev_cxt *lc)
{
	संरचना loop_info64 *lo = loopcxt_get_info(lc);

	यदि (lo)
		फर्काऊ (वर्ण *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	फर्काऊ रिक्त;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_backing_devno(संरचना loopdev_cxt *lc, dev_t *devno)
{
	संरचना loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णाङ्क rc;

	यदि (lo) {
		यदि (devno)
			*devno = lo->lo_device;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
पूर्णाङ्क loopcxt_get_backing_inode(संरचना loopdev_cxt *lc, ino_t *ino)
{
	संरचना loop_info64 *lo = loopcxt_get_info(lc);
	पूर्णाङ्क rc;

	यदि (lo) {
		यदि (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} अन्यथा
		rc = -त्रुटि_सं;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	फर्काऊ rc;
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
पूर्णाङ्क loopmod_supports_partscan(शून्य)
{
	पूर्णाङ्क rc, ret = 0;
	खाता *f;

	यदि (get_linux_version() >= KERNEL_VERSION(3,2,0))
		फर्काऊ 1;

	f = ख_खोलो("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	यदि (!f)
		फर्काऊ 0;
	rc = ख_पूछो(f, "%d", &ret);
	ख_बंद(f);
	फर्काऊ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
पूर्णाङ्क loopcxt_is_partscan(संरचना loopdev_cxt *lc)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);

	यदि (sysfs) {
		/* kernel >= 3.2 */
		पूर्णाङ्क fl;
		यदि (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			फर्काऊ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	फर्काऊ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
पूर्णाङ्क loopcxt_is_autoclear(संरचना loopdev_cxt *lc)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);

	यदि (sysfs) {
		पूर्णाङ्क fl;
		यदि (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			फर्काऊ fl;
	}

	यदि (loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);
		यदि (lo)
			फर्काऊ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	फर्काऊ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
पूर्णाङ्क loopcxt_is_readonly(संरचना loopdev_cxt *lc)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);

	यदि (sysfs) {
		पूर्णाङ्क fl;
		यदि (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			फर्काऊ fl;
	}

	यदि (loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);
		यदि (lo)
			फर्काऊ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	फर्काऊ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
पूर्णाङ्क loopcxt_is_dio(संरचना loopdev_cxt *lc)
{
	संरचना path_cxt *sysfs = loopcxt_get_sysfs(lc);

	यदि (sysfs) {
		पूर्णाङ्क fl;
		यदि (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			फर्काऊ fl;
	}
	यदि (loopcxt_ioctl_enabled(lc)) {
		संरचना loop_info64 *lo = loopcxt_get_info(lc);
		यदि (lo)
			फर्काऊ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	फर्काऊ 0;
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
पूर्णाङ्क loopcxt_is_used(संरचना loopdev_cxt *lc,
		    संरचना stat *st,
		    अचर वर्ण *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    पूर्णाङ्क flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	यदि (!lc)
		फर्काऊ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	यदि (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		यदि (ino == st->st_ino && dev == st->st_dev)
			गच्छ found;

		/* don't use filename if we have devno and inode */
		फर्काऊ 0;
	}

	/* poor man's solution */
	यदि (backing_file) {
		वर्ण *name = loopcxt_get_backing_file(lc);
		पूर्णाङ्क rc = name && म_भेद(name, backing_file) == 0;

		मुक्त(name);
		यदि (rc)
			गच्छ found;
	}

	फर्काऊ 0;
found:
	यदि (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		पूर्णाङ्क rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		यदि (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			फर्काऊ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		फर्काऊ rc;
	}
	फर्काऊ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णाङ्क loopcxt_set_offset(संरचना loopdev_cxt *lc, uint64_t offset)
{
	यदि (!lc)
		फर्काऊ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	फर्काऊ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णाङ्क loopcxt_set_sizelimit(संरचना loopdev_cxt *lc, uint64_t sizelimit)
{
	यदि (!lc)
		फर्काऊ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	फर्काऊ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
पूर्णाङ्क loopcxt_set_blocksize(संरचना loopdev_cxt *lc, uint64_t blocksize)
{
	यदि (!lc)
		फर्काऊ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	फर्काऊ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क loopcxt_set_flags(संरचना loopdev_cxt *lc, uint32_t flags)
{
	यदि (!lc)
		फर्काऊ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (अचिन्हित) flags));
	फर्काऊ 0;
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
पूर्णाङ्क loopcxt_set_refname(संरचना loopdev_cxt *lc, अचर वर्ण *refname)
{
	यदि (!lc)
		फर्काऊ -EINVAL;

	स_रखो(lc->config.info.lo_file_name, 0, परिमाण(lc->config.info.lo_file_name));
	यदि (refname)
		xstrncpy((वर्ण *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (वर्ण *)lc->config.info.lo_file_name));
	फर्काऊ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
पूर्णाङ्क loopcxt_set_backing_file(संरचना loopdev_cxt *lc, अचर वर्ण *filename)
{
	यदि (!lc)
		फर्काऊ -EINVAL;

	यदि (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	अन्यथा
		lc->filename = ul_canonicalize_path(filename);
	यदि (!lc->filename)
		फर्काऊ -त्रुटि_सं;

	यदि (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	फर्काऊ 0;
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
स्थिर पूर्णाङ्क loopcxt_check_size(संरचना loopdev_cxt *lc, पूर्णाङ्क file_fd)
{
	uint64_t size, expected_size;
	पूर्णाङ्क dev_fd;
	संरचना stat st;

	यदि (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		फर्काऊ 0;

	यदि (ख_स्थिति(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		फर्काऊ -त्रुटि_सं;
	}
	यदि (S_ISBLK(st.st_mode)) {
		यदि (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			फर्काऊ -त्रुटि_सं;
		}
	} अन्यथा
		expected_size = st.st_size;

	यदि (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		फर्काऊ 0;	/* ignore this error */
	}

	यदि (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	यदि (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	यदि (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		फर्काऊ -त्रुटि_सं;
	}

	यदि (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		फर्काऊ -त्रुटि_सं;
	}

	/* It's block device, so, align to 512-byte sectors */
	यदि (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	यदि (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		यदि (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			यदि (त्रुटि_सं == ENOTTY || त्रुटि_सं == EINVAL)
				त्रुटि_सं = दुस्फल;
			फर्काऊ -त्रुटि_सं;
		}

		यदि (blkdev_get_size(dev_fd, &size))
			फर्काऊ -त्रुटि_सं;

		यदि (expected_size != size) {
			त्रुटि_सं = दुस्फल;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			फर्काऊ -त्रुटि_सं;
		}
	}

	फर्काऊ 0;
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
पूर्णाङ्क loopcxt_setup_device(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	पूर्णाङ्क rc = -1, cnt = 0;
	पूर्णाङ्क errsv = 0;
	पूर्णाङ्क fallback = 0;

	यदि (!lc || !*lc->device || !lc->filename)
		फर्काऊ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	यदि (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	यदि (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	यदि (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	अन्यथा
		file_fd = खोलो(lc->filename, mode | flags);

	यदि (file_fd < 0 && mode != O_RDONLY
	    && (त्रुटि_सं == EROFS || त्रुटि_सं == EACCES)) {
		mode = O_RDONLY;
		यदि (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		अन्यथा
			file_fd = खोलो(lc->filename, mode | flags);
	}
	यदि (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		फर्काऊ -त्रुटि_सं;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	यदि (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	अन्यथा
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	गर {
		त्रुटि_सं = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		यदि (dev_fd >= 0 || lc->control_ok == 0)
			रोक;
		यदि (त्रुटि_सं != EACCES && त्रुटि_सं != ENOENT)
			रोक;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} जबसम्म (cnt++ < 16);

	यदि (dev_fd < 0) {
		rc = -त्रुटि_सं;
		गच्छ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	यदि (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	यदि (rc != 0) {
		errsv = त्रुटि_सं;
		यदि (त्रुटि_सं != EINVAL && त्रुटि_सं != ENOTTY && त्रुटि_सं != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			गच्छ err;
		}
		fallback = 1;
	} अन्यथा {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	यदि (fallback) {
		यदि (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -त्रुटि_सं;
			errsv = त्रुटि_सं;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			गच्छ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		यदि (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			गच्छ err;
		}

		यदि ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			गच्छ err;
		}
	}

	यदि ((rc = loopcxt_check_size(lc, file_fd)))
		गच्छ err;

	बंद(file_fd);

	स_रखो(&lc->config, 0, परिमाण(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	फर्काऊ 0;
err:
	यदि (file_fd >= 0)
		बंद(file_fd);
	यदि (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	यदि (errsv)
		त्रुटि_सं = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	फर्काऊ rc;
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
पूर्णाङ्क loopcxt_ioctl_status(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क dev_fd, rc;

	त्रुटि_सं = 0;
	dev_fd = loopcxt_get_fd(lc);

	यदि (dev_fd < 0)
		फर्काऊ -त्रुटि_सं;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	यदि (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		फर्काऊ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	फर्काऊ 0;
}

पूर्णाङ्क loopcxt_ioctl_capacity(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क rc, fd = loopcxt_get_fd(lc);

	यदि (fd < 0)
		फर्काऊ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	यदि (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		फर्काऊ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	फर्काऊ 0;
}

पूर्णाङ्क loopcxt_ioctl_dio(संरचना loopdev_cxt *lc, अचिन्हित दीर्घ use_dio)
{
	पूर्णाङ्क rc, fd = loopcxt_get_fd(lc);

	यदि (fd < 0)
		फर्काऊ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	यदि (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		फर्काऊ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	फर्काऊ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
पूर्णाङ्क loopcxt_ioctl_blocksize(संरचना loopdev_cxt *lc, uint64_t blocksize)
{
	पूर्णाङ्क rc, fd = loopcxt_get_fd(lc);

	यदि (fd < 0)
		फर्काऊ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (अचिन्हित दीर्घ) blocksize) );
	यदि (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		फर्काऊ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	फर्काऊ 0;
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
स्थिर पूर्णाङ्क loopcxt_get_device_nr(संरचना loopdev_cxt *lc, पूर्णाङ्क *nr)
{
	अचर वर्ण *p, *dev;
	पूर्णाङ्क rc = -EINVAL;

	त्रुटि_सं = 0;
	यदि (!lc || !nr)
		फर्काऊ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	यदि (!dev)
		गच्छ done;

	p = म_खोजप(dev, '/');
	यदि (!p)
		गच्छ done;

	यदि (माला_पूछो(p, "/loop%d", nr) != 1 && माला_पूछो(p, "/%d", nr) != 1)
		गच्छ done;

	यदि (*nr < 0)
		गच्छ done;
	rc = 0;
done:
	यदि (rc && !त्रुटि_सं)
		त्रुटि_सं = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	फर्काऊ rc;
}

पूर्णाङ्क loopcxt_detach_device(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क rc, fd = loopcxt_get_fd(lc);

	यदि (fd < 0)
		फर्काऊ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	यदि (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		फर्काऊ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	फर्काऊ 0;
}

पूर्णाङ्क loopcxt_remove_device(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क rc = -EINVAL;
	पूर्णाङ्क ctl, nr = -1;

	यदि (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		गच्छ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	यदि (rc)
		गच्छ done;

	ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	यदि (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		बंद(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	फर्काऊ rc;
}

पूर्णाङ्क loopcxt_add_device(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क rc = -EINVAL;
	पूर्णाङ्क ctl, nr = -1;

	यदि (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		गच्छ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	यदि (rc)
		गच्छ done;

	ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	यदि (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		बंद(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
पूर्णाङ्क loopcxt_find_unused(संरचना loopdev_cxt *lc)
{
	पूर्णाङ्क rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	यदि (lc->flags & LOOPDEV_FL_CONTROL) {
		पूर्णाङ्क ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = खोलो(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		यदि (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		अन्यथा
			rc = -त्रुटि_सं;
		यदि (rc >= 0) {
			वर्ण name[16];
			snprintf(name, परिमाण(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		यदि (ctl >= 0)
			बंद(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	यदि (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		यदि (rc)
			फर्काऊ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		यदि (rc)
			फर्काऊ -ENOENT;
	}
	फर्काऊ rc;
}



/*
 * Return: TRUE/FALSE
 */
पूर्णाङ्क loopdev_is_autoclear(अचर वर्ण *device)
{
	संरचना loopdev_cxt lc;
	पूर्णाङ्क rc;

	यदि (!device)
		फर्काऊ 0;

	rc = loopcxt_init(&lc, 0);
	यदि (!rc)
		rc = loopcxt_set_device(&lc, device);
	यदि (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	फर्काऊ rc;
}

वर्ण *loopdev_get_backing_file(अचर वर्ण *device)
{
	संरचना loopdev_cxt lc;
	वर्ण *res = रिक्त;

	यदि (!device)
		फर्काऊ रिक्त;
	यदि (loopcxt_init(&lc, 0))
		फर्काऊ रिक्त;
	यदि (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	फर्काऊ res;
}

/*
 * Returns: TRUE/FALSE
 */
पूर्णाङ्क loopdev_has_backing_file(अचर वर्ण *device)
{
	वर्ण *tmp = loopdev_get_backing_file(device);

	यदि (tmp) {
		मुक्त(tmp);
		फर्काऊ 1;
	}
	फर्काऊ 0;
}

/*
 * Returns: TRUE/FALSE
 */
पूर्णाङ्क loopdev_is_used(अचर वर्ण *device, अचर वर्ण *filename,
		    uint64_t offset, uint64_t sizelimit, पूर्णाङ्क flags)
{
	संरचना loopdev_cxt lc;
	संरचना stat st;
	पूर्णाङ्क rc = 0;

	यदि (!device || !filename)
		फर्काऊ 0;

	rc = loopcxt_init(&lc, 0);
	यदि (!rc)
		rc = loopcxt_set_device(&lc, device);
	यदि (rc)
		फर्काऊ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : रिक्त, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	फर्काऊ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
पूर्णाङ्क loopdev_detach(अचर वर्ण *device)
{
	संरचना loopdev_cxt lc;
	पूर्णाङ्क rc;

	यदि (!device)
		फर्काऊ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	यदि (!rc)
		rc = loopcxt_set_device(&lc, device);
	यदि (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	फर्काऊ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
पूर्णाङ्क loopcxt_find_by_backing_file(संरचना loopdev_cxt *lc, अचर वर्ण *filename,
				 uint64_t offset, uint64_t sizelimit, पूर्णाङ्क flags)
{
	पूर्णाङ्क rc, hasst;
	संरचना stat st;

	यदि (!filename)
		फर्काऊ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	यदि (rc)
		फर्काऊ rc;

	जबसम्म ((rc = loopcxt_next(lc)) == 0) {

		यदि (loopcxt_is_used(lc, hasst ? &st : रिक्त,
				    filename, offset, sizelimit, flags))
			रोक;
	}

	loopcxt_deinit_iterator(lc);
	फर्काऊ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
पूर्णाङ्क loopcxt_find_overlap(संरचना loopdev_cxt *lc, अचर वर्ण *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	पूर्णाङ्क rc, hasst;
	संरचना stat st;

	यदि (!filename)
		फर्काऊ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	यदि (rc)
		फर्काऊ rc;

	जबसम्म ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : रिक्त,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		यदि (rc <= 0)
			जारी;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		यदि (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			जारी;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		यदि (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			जारी;
		}

		/* full match */
		यदि (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			गच्छ found;
		}

		/* overlap */
		यदि (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			जारी;
		यदि (sizelimit != 0 && offset + sizelimit <= lc_offset)
			जारी;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			गच्छ found;
	}

	यदि (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	फर्काऊ rc;
}

/*
 * Returns allocated string with device name
 */
वर्ण *loopdev_find_by_backing_file(अचर वर्ण *filename, uint64_t offset, uint64_t sizelimit, पूर्णाङ्क flags)
{
	संरचना loopdev_cxt lc;
	वर्ण *res = रिक्त;

	यदि (!filename)
		फर्काऊ रिक्त;

	यदि (loopcxt_init(&lc, 0))
		फर्काऊ रिक्त;
	यदि (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	फर्काऊ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
पूर्णाङ्क loopdev_गन्ती_by_backing_file(अचर वर्ण *filename, वर्ण **loopdev)
{
	संरचना loopdev_cxt lc;
	पूर्णाङ्क गन्ती = 0, rc;

	यदि (!filename)
		फर्काऊ -1;

	rc = loopcxt_init(&lc, 0);
	यदि (rc)
		फर्काऊ rc;
	यदि (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		फर्काऊ -1;

	जबसम्म(loopcxt_next(&lc) == 0) {
		वर्ण *backing = loopcxt_get_backing_file(&lc);

		यदि (!backing || म_भेद(backing, filename) != 0) {
			मुक्त(backing);
			जारी;
		}

		मुक्त(backing);
		यदि (loopdev && गन्ती == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		गन्ती++;
	}

	loopcxt_deinit(&lc);

	यदि (loopdev && गन्ती > 1) {
		मुक्त(*loopdev);
		*loopdev = रिक्त;
	}
	फर्काऊ गन्ती;
}

#ifdef TEST_PROGRAM_LOOPDEV
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	यदि (argc < 2)
		गच्छ usage;

	यदि (म_भेद(argv[1], "--is-loopdev") == 0 && argc == 3)
		म_लिखो("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	अन्यथा
		गच्छ usage;

	फर्काऊ निकास_सफल;
usage:
	ख_लिखो(मानक_त्रुटि, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	फर्काऊ निकास_त्रुटि;
}
#endif

