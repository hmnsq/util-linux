
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
#ضمن <stdio.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ضمن <fcntl.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/stat.h>
#ضمن <sys/mman.h>
#ضمن <inttypes.h>
#ضمن <dirent.h>

#ضمن "linux_version.h"
#ضمن "c.h"
#ضمن "timeutils.h"
#ضمن "sysfs.h"
#ضمن "pathnames.h"
#ضمن "loopdev.h"
#ضمن "canonicalize.h"
#ضمن "blkdev.h"
#ضمن "debug.h"
#ضمن "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ساكن فراغ loopdev_init_debug(فراغ)
{
	ܐܢ (loopdev_debug_mask)
		ܦܢܐ;
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
		صحيح _c = 0, _e;					\
		ܥܒܕ {						\
			errno = 0;				\
			_e = x;					\
			ܐܢ (_e == 0 || errno != EAGAIN)		\
				اكسر;				\
			ܐܢ (_c >= LOOPDEV_MAX_TRIES)		\
				اكسر;				\
			xusleep(250000);			\
			_c++;					\
		} ܟܡܐ (1);					\
		_e == 0 ? 0 : errno ? -errno : -1;		\
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
صحيح loopcxt_set_device(بنية loopdev_cxt *lc, ثابت حرف *device)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;

	ܐܢ (lc->fd >= 0) {
		close(lc->fd);
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
	memset(&lc->config, 0, حجم(lc->config));

	/* set new */
	ܐܢ (device) {
		ܐܢ (*device != '/') {
			ثابت حرف *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ܐܢ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ܐܢ (strlen(device) < 5)
					ܦܢܐ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, حجم(lc->device), "%s%s",
				dir, device);
		} ܐܚܪܢܐ
			xstrncpy(lc->device, device, حجم(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = عدم;
	ܦܢܐ 0;
}

صحيح loopcxt_has_device(بنية loopdev_cxt *lc)
{
	ܦܢܐ lc && *lc->device;
}

dev_t loopcxt_get_devno(بنية loopdev_cxt *lc)
{
	ܐܢ (!lc || !loopcxt_has_device(lc))
		ܦܢܐ 0;
	ܐܢ (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ܦܢܐ lc->devno;
}

صحيح loopcxt_is_lost(بنية loopdev_cxt *lc)
{
	ܐܢ (!lc || !loopcxt_has_device(lc))
		ܦܢܐ 0;
	ܐܢ (lc->is_lost)
		ܦܢܐ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ܦܢܐ lc->is_lost;
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
صحيح loopcxt_init(بنية loopdev_cxt *lc, صحيح flags)
{
	صحيح rc;
	بنية stat st;
	بنية loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	ܐܢ (!lc)
		ܦܢܐ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, حجم(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, عدم);
	ܐܢ (rc)
		ܦܢܐ rc;

	ܐܢ (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ܐܢ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ܐܢ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ܦܢܐ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
فراغ loopcxt_deinit(بنية loopdev_cxt *lc)
{
	صحيح errsv = errno;

	ܐܢ (!lc)
		ܦܢܐ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	حرر(lc->filename);
	lc->filename = عدم;

	ignore_result( loopcxt_set_device(lc, عدم) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
حرف *loopcxt_strdup_device(بنية loopdev_cxt *lc)
{
	ܐܢ (!lc || !*lc->device)
		ܦܢܐ عدم;
	ܦܢܐ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ثابت حرف *loopcxt_get_device(بنية loopdev_cxt *lc)
{
	ܦܢܐ lc && *lc->device ? lc->device : عدم;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ساكن بنية path_cxt *loopcxt_get_sysfs(بنية loopdev_cxt *lc)
{
	ܐܢ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ܦܢܐ عدم;

	ܐܢ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ܐܢ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ܦܢܐ عدم;
		}

		lc->sysfs = ul_new_sysfs_path(devno, عدم, عدم);
		ܐܢ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ܦܢܐ lc->sysfs;
}

ساكن صحيح __loopcxt_get_fd(بنية loopdev_cxt *lc, mode_t mode)
{
	صحيح old = -1;

	ܐܢ (!lc || !*lc->device)
		ܦܢܐ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ܐܢ (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ܐܢ (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		ܐܢ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ܐܢ (old >= 0)
		close(old);
	ܦܢܐ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
صحيح loopcxt_get_fd(بنية loopdev_cxt *lc)
{
	ܦܢܐ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_init_iterator(بنية loopdev_cxt *lc, صحيح flags)
{
	بنية loopdev_iter *iter;
	بنية stat st;

	ܐܢ (!lc)
		ܦܢܐ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, حجم(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ܐܢ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ܐܢ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ܦܢܐ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_deinit_iterator(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter;

	ܐܢ (!lc)
		ܦܢܐ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	حرر(iter->minors);
	ܐܢ (iter->proc)
		fclose(iter->proc);
	ܐܢ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, حجم(*iter));
	ܦܢܐ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ساكن صحيح loopiter_set_device(بنية loopdev_cxt *lc, ثابت حرف *device)
{
	صحيح rc = loopcxt_set_device(lc, device);
	صحيح used;

	ܐܢ (rc)
		ܦܢܐ rc;

	ܐܢ (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ܦܢܐ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, عدم) == 0;

	ܐܢ ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ܦܢܐ 0;

	ܐܢ ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ܦܢܐ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, عدم) );
	ܦܢܐ 1;
}

ساكن صحيح cmpnum(ثابت فراغ *p1, ثابت فراغ *p2)
{
	ܦܢܐ (((* (ثابت صحيح *) p1) > (* (ثابت صحيح *) p2)) -
			((* (ثابت صحيح *) p1) < (* (ثابت صحيح *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ساكن صحيح loop_scandir(ثابت حرف *dirname, صحيح **ary, صحيح hasprefix)
{
	DIR *dir;
	بنية dirent *d;
	unsigned صحيح n, count = 0, arylen = 0;

	ܐܢ (!dirname || !ary)
		ܦܢܐ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ܐܢ (!dir)
		ܦܢܐ 0;
	حرر(*ary);
	*ary = عدم;

	ܟܡܐ((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		ܐܢ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			استمر;
#endif
		ܐܢ (is_dotdir_dirent(d))
			استمر;

		ܐܢ (hasprefix) {
			/* /dev/loop<N> */
			ܐܢ (sscanf(d->d_name, "loop%u", &n) != 1)
				استمر;
		} ܐܚܪܢܐ {
			/* /dev/loop/<N> */
			حرف *end = عدم;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			ܐܢ (d->d_name == end || (end && *end) || errno)
				استمر;
		}
		ܐܢ (n < LOOPDEV_DEFAULT_NNODES)
			استمر;			/* ignore loop<0..7> */

		ܐܢ (count + 1 > arylen) {
			صحيح *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, حجم(صحيح));
			ܐܢ (!tmp) {
				حرر(*ary);
				*ary = عدم;
				closedir(dir);
				ܦܢܐ -1;
			}
			*ary = tmp;
		}
		ܐܢ (*ary)
			(*ary)[count++] = n;
	}
	ܐܢ (count && *ary)
		qsort(*ary, count, حجم(صحيح), cmpnum);

	closedir(dir);
	ܦܢܐ count;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ساكن صحيح loopcxt_next_from_proc(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter = &lc->iter;
	حرف buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ܐܢ (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ܐܢ (!iter->proc)
		ܦܢܐ 1;

	ܟܡܐ (fgets(buf, حجم(buf), iter->proc)) {
		unsigned صحيح m;
		حرف name[128 + 1];


		ܐܢ (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			استمر;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ܐܢ (loopiter_set_device(lc, name) == 0)
			ܦܢܐ 0;
	}

	ܦܢܐ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ساكن صحيح loopcxt_next_from_sysfs(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter = &lc->iter;
	بنية dirent *d;
	صحيح fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ܐܢ (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	ܐܢ (!iter->sysblock)
		ܦܢܐ 1;

	fd = dirfd(iter->sysblock);

	ܟܡܐ ((d = readdir(iter->sysblock))) {
		حرف name[NAME_MAX + 18 + 1];
		بنية stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ܐܢ (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			استمر;

		snprintf(name, حجم(name), "%s/loop/backing_file", d->d_name);
		ܐܢ (fstatat(fd, name, &st, 0) != 0)
			استمر;

		ܐܢ (loopiter_set_device(lc, d->d_name) == 0)
			ܦܢܐ 0;
	}

	ܦܢܐ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
صحيح loopcxt_next(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter;

	ܐܢ (!lc)
		ܦܢܐ -EINVAL;


	iter = &lc->iter;
	ܐܢ (iter->done)
		ܦܢܐ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ܐܢ (iter->flags & LOOPITER_FL_USED) {
		صحيح rc;

		ܐܢ (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		ܐܚܪܢܐ
			rc = loopcxt_next_from_proc(lc);
		ܐܢ (rc == 0)
			ܦܢܐ 0;
		اذهب done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ܐܢ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		لكل (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", iter->ncur);

			ܐܢ (loopiter_set_device(lc, name) == 0)
				ܦܢܐ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ܐܢ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	لكل (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		حرف name[16];
		snprintf(name, حجم(name), "loop%d", iter->minors[iter->ncur]);

		ܐܢ (loopiter_set_device(lc, name) == 0)
			ܦܢܐ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ܦܢܐ 1;
}

/*
 * @device: path to device
 */
صحيح is_loopdev(ثابت حرف *device)
{
	بنية stat st;
	صحيح rc = 0;

	ܐܢ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ܐܚܪܢܐ ܐܢ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ܐܚܪܢܐ ܐܢ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		حرف name[PATH_MAX], *cn, *p = عدم;

		snprintf(name, حجم(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		ܐܢ (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		حرر(cn);
	}

	ܐܢ (rc == 0)
		errno = ENODEV;
	ܦܢܐ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
بنية loop_info64 *loopcxt_get_info(بنية loopdev_cxt *lc)
{
	صحيح fd;

	ܐܢ (!lc || lc->info_failed) {
		errno = EINVAL;
		ܦܢܐ عدم;
	}
	errno = 0;
	ܐܢ (lc->has_info)
		ܦܢܐ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ܐܢ (fd < 0)
		ܦܢܐ عدم;

	ܐܢ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ܦܢܐ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ܦܢܐ عدم;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
حرف *loopcxt_get_backing_file(بنية loopdev_cxt *lc)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);
	حرف *res = عدم;

	ܐܢ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	ܐܢ (!res && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);

		ܐܢ (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((حرف *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ܦܢܐ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
حرف *loopcxt_get_refname(بنية loopdev_cxt *lc)
{
	حرف *res = عدم;
	بنية loop_info64 *lo = loopcxt_get_info(lc);

	ܐܢ (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((حرف *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ܦܢܐ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_offset(بنية loopdev_cxt *lc, uint64_t *offset)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	ܐܢ (sysfs)
		ܐܢ (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	ܐܢ (rc && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		ܐܢ (lo) {
			ܐܢ (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} ܐܚܪܢܐ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_blocksize(بنية loopdev_cxt *lc, uint64_t *blocksize)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	ܐܢ (sysfs)
		ܐܢ (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ܐܢ (rc) {
		صحيح fd = loopcxt_get_fd(lc);
		صحيح sz = 0;

		ܐܢ (fd < 0)
			ܦܢܐ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		ܐܢ (rc)
			ܦܢܐ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_sizelimit(بنية loopdev_cxt *lc, uint64_t *size)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	ܐܢ (sysfs)
		ܐܢ (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	ܐܢ (rc && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		ܐܢ (lo) {
			ܐܢ (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} ܐܚܪܢܐ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_encrypt_type(بنية loopdev_cxt *lc, uint32_t *type)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	/* not provided by sysfs */
	ܐܢ (lo) {
		ܐܢ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ܐܚܪܢܐ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ثابت حرف *loopcxt_get_crypt_name(بنية loopdev_cxt *lc)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);

	ܐܢ (lo)
		ܦܢܐ (حرف *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ܦܢܐ عدم;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_backing_devno(بنية loopdev_cxt *lc, dev_t *devno)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	ܐܢ (lo) {
		ܐܢ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ܐܚܪܢܐ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_backing_inode(بنية loopdev_cxt *lc, ino_t *ino)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	ܐܢ (lo) {
		ܐܢ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ܐܚܪܢܐ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ܦܢܐ rc;
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
صحيح loopmod_supports_partscan(فراغ)
{
	صحيح rc, ret = 0;
	FILE *f;

	ܐܢ (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ܦܢܐ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ܐܢ (!f)
		ܦܢܐ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ܦܢܐ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
صحيح loopcxt_is_partscan(بنية loopdev_cxt *lc)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ܐܢ (sysfs) {
		/* kernel >= 3.2 */
		صحيح fl;
		ܐܢ (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ܦܢܐ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ܦܢܐ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
صحيح loopcxt_is_autoclear(بنية loopdev_cxt *lc)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ܐܢ (sysfs) {
		صحيح fl;
		ܐܢ (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ܦܢܐ fl;
	}

	ܐܢ (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		ܐܢ (lo)
			ܦܢܐ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ܦܢܐ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
صحيح loopcxt_is_readonly(بنية loopdev_cxt *lc)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ܐܢ (sysfs) {
		صحيح fl;
		ܐܢ (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ܦܢܐ fl;
	}

	ܐܢ (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		ܐܢ (lo)
			ܦܢܐ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ܦܢܐ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
صحيح loopcxt_is_dio(بنية loopdev_cxt *lc)
{
	بنية path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ܐܢ (sysfs) {
		صحيح fl;
		ܐܢ (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ܦܢܐ fl;
	}
	ܐܢ (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		ܐܢ (lo)
			ܦܢܐ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ܦܢܐ 0;
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
صحيح loopcxt_is_used(بنية loopdev_cxt *lc,
		    بنية stat *st,
		    ثابت حرف *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    صحيح flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ܐܢ (!lc)
		ܦܢܐ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ܐܢ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ܐܢ (ino == st->st_ino && dev == st->st_dev)
			اذهب found;

		/* don't use filename if we have devno and inode */
		ܦܢܐ 0;
	}

	/* poor man's solution */
	ܐܢ (backing_file) {
		حرف *name = loopcxt_get_backing_file(lc);
		صحيح rc = name && strcmp(name, backing_file) == 0;

		حرر(name);
		ܐܢ (rc)
			اذهب found;
	}

	ܦܢܐ 0;
found:
	ܐܢ (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		صحيح rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		ܐܢ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ܦܢܐ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ܦܢܐ rc;
	}
	ܦܢܐ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_set_offset(بنية loopdev_cxt *lc, uint64_t offset)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ܦܢܐ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_set_sizelimit(بنية loopdev_cxt *lc, uint64_t sizelimit)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ܦܢܐ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_set_blocksize(بنية loopdev_cxt *lc, uint64_t blocksize)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ܦܢܐ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح loopcxt_set_flags(بنية loopdev_cxt *lc, uint32_t flags)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ܦܢܐ 0;
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
صحيح loopcxt_set_refname(بنية loopdev_cxt *lc, ثابت حرف *refname)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, حجم(lc->config.info.lo_file_name));
	ܐܢ (refname)
		xstrncpy((حرف *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (حرف *)lc->config.info.lo_file_name));
	ܦܢܐ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح loopcxt_set_backing_file(بنية loopdev_cxt *lc, ثابت حرف *filename)
{
	ܐܢ (!lc)
		ܦܢܐ -EINVAL;

	ܐܢ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ܐܚܪܢܐ
		lc->filename = ul_canonicalize_path(filename);
	ܐܢ (!lc->filename)
		ܦܢܐ -errno;

	ܐܢ (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ܦܢܐ 0;
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
ساكن صحيح loopcxt_check_size(بنية loopdev_cxt *lc, صحيح file_fd)
{
	uint64_t size, expected_size;
	صحيح dev_fd;
	بنية stat st;

	ܐܢ (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ܦܢܐ 0;

	ܐܢ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ܦܢܐ -errno;
	}
	ܐܢ (S_ISBLK(st.st_mode)) {
		ܐܢ (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ܦܢܐ -errno;
		}
	} ܐܚܪܢܐ
		expected_size = st.st_size;

	ܐܢ (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ܦܢܐ 0;	/* ignore this error */
	}

	ܐܢ (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	ܐܢ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ܐܢ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ܦܢܐ -errno;
	}

	ܐܢ (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ܦܢܐ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ܐܢ (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	ܐܢ (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		ܐܢ (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			ܐܢ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ܦܢܐ -errno;
		}

		ܐܢ (blkdev_get_size(dev_fd, &size))
			ܦܢܐ -errno;

		ܐܢ (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ܦܢܐ -errno;
		}
	}

	ܦܢܐ 0;
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
صحيح loopcxt_setup_device(بنية loopdev_cxt *lc)
{
	صحيح file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	صحيح rc = -1, cnt = 0;
	صحيح errsv = 0;
	صحيح fallback = 0;

	ܐܢ (!lc || !*lc->device || !lc->filename)
		ܦܢܐ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ܐܢ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	ܐܢ (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	ܐܢ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	ܐܚܪܢܐ
		file_fd = open(lc->filename, mode | flags);

	ܐܢ (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		ܐܢ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		ܐܚܪܢܐ
			file_fd = open(lc->filename, mode | flags);
	}
	ܐܢ (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ܦܢܐ -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	ܐܢ (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ܐܚܪܢܐ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	ܥܒܕ {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		ܐܢ (dev_fd >= 0 || lc->control_ok == 0)
			اكسر;
		ܐܢ (errno != EACCES && errno != ENOENT)
			اكسر;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ܟܡܐ (cnt++ < 16);

	ܐܢ (dev_fd < 0) {
		rc = -errno;
		اذهب err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ܐܢ (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ܐܢ (rc != 0) {
		errsv = errno;
		ܐܢ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			اذهب err;
		}
		fallback = 1;
	} ܐܚܪܢܐ {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ܐܢ (fallback) {
		ܐܢ (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			اذهب err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		ܐܢ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			اذهب err;
		}

		ܐܢ ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			اذهب err;
		}
	}

	ܐܢ ((rc = loopcxt_check_size(lc, file_fd)))
		اذهب err;

	close(file_fd);

	memset(&lc->config, 0, حجم(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ܦܢܐ 0;
err:
	ܐܢ (file_fd >= 0)
		close(file_fd);
	ܐܢ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ܐܢ (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ܦܢܐ rc;
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
صحيح loopcxt_ioctl_status(بنية loopdev_cxt *lc)
{
	صحيح dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ܐܢ (dev_fd < 0)
		ܦܢܐ -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	ܐܢ (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ܦܢܐ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ܦܢܐ 0;
}

صحيح loopcxt_ioctl_capacity(بنية loopdev_cxt *lc)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	ܐܢ (fd < 0)
		ܦܢܐ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	ܐܢ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ܦܢܐ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ܦܢܐ 0;
}

صحيح loopcxt_ioctl_dio(بنية loopdev_cxt *lc, unsigned long use_dio)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	ܐܢ (fd < 0)
		ܦܢܐ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	ܐܢ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ܦܢܐ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ܦܢܐ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
صحيح loopcxt_ioctl_blocksize(بنية loopdev_cxt *lc, uint64_t blocksize)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	ܐܢ (fd < 0)
		ܦܢܐ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	ܐܢ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ܦܢܐ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ܦܢܐ 0;
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
ساكن صحيح loopcxt_get_device_nr(بنية loopdev_cxt *lc, صحيح *nr)
{
	ثابت حرف *p, *dev;
	صحيح rc = -EINVAL;

	errno = 0;
	ܐܢ (!lc || !nr)
		ܦܢܐ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ܐܢ (!dev)
		اذهب done;

	p = strrchr(dev, '/');
	ܐܢ (!p)
		اذهب done;

	ܐܢ (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		اذهب done;

	ܐܢ (*nr < 0)
		اذهب done;
	rc = 0;
done:
	ܐܢ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ܦܢܐ rc;
}

صحيح loopcxt_detach_device(بنية loopdev_cxt *lc)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	ܐܢ (fd < 0)
		ܦܢܐ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ܐܢ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ܦܢܐ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ܦܢܐ 0;
}

صحيح loopcxt_remove_device(بنية loopdev_cxt *lc)
{
	صحيح rc = -EINVAL;
	صحيح ctl, nr = -1;

	ܐܢ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		اذهب done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ܐܢ (rc)
		اذهب done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ܐܢ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ܦܢܐ rc;
}

صحيح loopcxt_add_device(بنية loopdev_cxt *lc)
{
	صحيح rc = -EINVAL;
	صحيح ctl, nr = -1;

	ܐܢ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		اذهب done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ܐܢ (rc)
		اذهب done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ܐܢ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
صحيح loopcxt_find_unused(بنية loopdev_cxt *lc)
{
	صحيح rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ܐܢ (lc->flags & LOOPDEV_FL_CONTROL) {
		صحيح ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ܐܢ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ܐܚܪܢܐ
			rc = -errno;
		ܐܢ (rc >= 0) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ܐܢ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ܐܢ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ܐܢ (rc)
			ܦܢܐ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ܐܢ (rc)
			ܦܢܐ -ENOENT;
	}
	ܦܢܐ rc;
}



/*
 * Return: TRUE/FALSE
 */
صحيح loopdev_is_autoclear(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	صحيح rc;

	ܐܢ (!device)
		ܦܢܐ 0;

	rc = loopcxt_init(&lc, 0);
	ܐܢ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ܐܢ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ܦܢܐ rc;
}

حرف *loopdev_get_backing_file(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	حرف *res = عدم;

	ܐܢ (!device)
		ܦܢܐ عدم;
	ܐܢ (loopcxt_init(&lc, 0))
		ܦܢܐ عدم;
	ܐܢ (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ܦܢܐ res;
}

/*
 * Returns: TRUE/FALSE
 */
صحيح loopdev_has_backing_file(ثابت حرف *device)
{
	حرف *tmp = loopdev_get_backing_file(device);

	ܐܢ (tmp) {
		حرر(tmp);
		ܦܢܐ 1;
	}
	ܦܢܐ 0;
}

/*
 * Returns: TRUE/FALSE
 */
صحيح loopdev_is_used(ثابت حرف *device, ثابت حرف *filename,
		    uint64_t offset, uint64_t sizelimit, صحيح flags)
{
	بنية loopdev_cxt lc;
	بنية stat st;
	صحيح rc = 0;

	ܐܢ (!device || !filename)
		ܦܢܐ 0;

	rc = loopcxt_init(&lc, 0);
	ܐܢ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ܐܢ (rc)
		ܦܢܐ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : عدم, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ܦܢܐ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
صحيح loopdev_detach(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	صحيح rc;

	ܐܢ (!device)
		ܦܢܐ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ܐܢ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ܐܢ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ܦܢܐ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
صحيح loopcxt_find_by_backing_file(بنية loopdev_cxt *lc, ثابت حرف *filename,
				 uint64_t offset, uint64_t sizelimit, صحيح flags)
{
	صحيح rc, hasst;
	بنية stat st;

	ܐܢ (!filename)
		ܦܢܐ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ܐܢ (rc)
		ܦܢܐ rc;

	ܟܡܐ ((rc = loopcxt_next(lc)) == 0) {

		ܐܢ (loopcxt_is_used(lc, hasst ? &st : عدم,
				    filename, offset, sizelimit, flags))
			اكسر;
	}

	loopcxt_deinit_iterator(lc);
	ܦܢܐ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
صحيح loopcxt_find_overlap(بنية loopdev_cxt *lc, ثابت حرف *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	صحيح rc, hasst;
	بنية stat st;

	ܐܢ (!filename)
		ܦܢܐ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ܐܢ (rc)
		ܦܢܐ rc;

	ܟܡܐ ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : عدم,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ܐܢ (rc <= 0)
			استمر;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		ܐܢ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			استمر;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ܐܢ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			استمر;
		}

		/* full match */
		ܐܢ (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			اذهب found;
		}

		/* overlap */
		ܐܢ (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			استمر;
		ܐܢ (sizelimit != 0 && offset + sizelimit <= lc_offset)
			استمر;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			اذهب found;
	}

	ܐܢ (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ܦܢܐ rc;
}

/*
 * Returns allocated string with device name
 */
حرف *loopdev_find_by_backing_file(ثابت حرف *filename, uint64_t offset, uint64_t sizelimit, صحيح flags)
{
	بنية loopdev_cxt lc;
	حرف *res = عدم;

	ܐܢ (!filename)
		ܦܢܐ عدم;

	ܐܢ (loopcxt_init(&lc, 0))
		ܦܢܐ عدم;
	ܐܢ (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ܦܢܐ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
صحيح loopdev_count_by_backing_file(ثابت حرف *filename, حرف **loopdev)
{
	بنية loopdev_cxt lc;
	صحيح count = 0, rc;

	ܐܢ (!filename)
		ܦܢܐ -1;

	rc = loopcxt_init(&lc, 0);
	ܐܢ (rc)
		ܦܢܐ rc;
	ܐܢ (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ܦܢܐ -1;

	ܟܡܐ(loopcxt_next(&lc) == 0) {
		حرف *backing = loopcxt_get_backing_file(&lc);

		ܐܢ (!backing || strcmp(backing, filename) != 0) {
			حرر(backing);
			استمر;
		}

		حرر(backing);
		ܐܢ (loopdev && count == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		count++;
	}

	loopcxt_deinit(&lc);

	ܐܢ (loopdev && count > 1) {
		حرر(*loopdev);
		*loopdev = عدم;
	}
	ܦܢܐ count;
}

#ifdef TEST_PROGRAM_LOOPDEV
صحيح main(صحيح argc, حرف *argv[])
{
	ܐܢ (argc < 2)
		اذهب usage;

	ܐܢ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ܐܚܪܢܐ
		اذهب usage;

	ܦܢܐ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ܦܢܐ EXIT_FAILURE;
}
#endif

