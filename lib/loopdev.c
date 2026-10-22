
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
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>

#ಅನ್ತರ್ಭಾವ "linux_version.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "sysfs.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "loopdev.h"
#ಅನ್ತರ್ಭಾವ "canonicalize.h"
#ಅನ್ತರ್ಭಾವ "blkdev.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ loopdev_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (loopdev_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
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
		ಪೂರ್ಣಾಙ್ಕ _c = 0, _e;					\
		ಕುರು {						\
			errno = 0;				\
			_e = x;					\
			ಯದಿ (_e == 0 || errno != EAGAIN)		\
				ಭಙ್ಗ;				\
			ಯದಿ (_c >= LOOPDEV_MAX_TRIES)		\
				ಭಙ್ಗ;				\
			xusleep(250000);			\
			_c++;					\
		} ಯಾವತ್ (1);					\
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_device(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *device)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (lc->fd >= 0) {
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
	memset(&lc->config, 0, ಪರಿಮಾಣ(lc->config));

	/* set new */
	ಯದಿ (device) {
		ಯದಿ (*device != '/') {
			ಧ್ರುವ ವರ್ಣ *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ಯದಿ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ಯದಿ (strlen(device) < 5)
					ಪ್ರತ್ಯಾಗಮ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, ಪರಿಮಾಣ(lc->device), "%s%s",
				dir, device);
		} ಅನ್ಯಥಾ
			xstrncpy(lc->device, device, ಪರಿಮಾಣ(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_has_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪ್ರತ್ಯಾಗಮ lc && *lc->device;
}

dev_t loopcxt_get_devno(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಯದಿ (!lc || !loopcxt_has_device(lc))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ಪ್ರತ್ಯಾಗಮ lc->devno;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_is_lost(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಯದಿ (!lc || !loopcxt_has_device(lc))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (lc->is_lost)
		ಪ್ರತ್ಯಾಗಮ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ಪ್ರತ್ಯಾಗಮ lc->is_lost;
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_init(ಸಂರಚನಾ loopdev_cxt *lc, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಸಂರಚನಾ stat st;
	ಸಂರಚನಾ loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, ಪರಿಮಾಣ(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, ರಿಕ್ತ);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯದಿ (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ಯದಿ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ಯದಿ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
ಶೂನ್ಯ loopcxt_deinit(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ errsv = errno;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	ಮುಕ್ತ(lc->filename);
	lc->filename = ರಿಕ್ತ;

	ignore_result( loopcxt_set_device(lc, ರಿಕ್ತ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
ವರ್ಣ *loopcxt_strdup_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಯದಿ (!lc || !*lc->device)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ಧ್ರುವ ವರ್ಣ *loopcxt_get_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪ್ರತ್ಯಾಗಮ lc && *lc->device ? lc->device : ರಿಕ್ತ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ಸ್ಥಿರ ಸಂರಚನಾ path_cxt *loopcxt_get_sysfs(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಯದಿ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ಯದಿ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}

		lc->sysfs = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
		ಯದಿ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ಪ್ರತ್ಯಾಗಮ lc->sysfs;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __loopcxt_get_fd(ಸಂರಚನಾ loopdev_cxt *lc, mode_t mode)
{
	ಪೂರ್ಣಾಙ್ಕ old = -1;

	ಯದಿ (!lc || !*lc->device)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ಯದಿ (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ಯದಿ (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		ಯದಿ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ಯದಿ (old >= 0)
		close(old);
	ಪ್ರತ್ಯಾಗಮ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_fd(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪ್ರತ್ಯಾಗಮ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_init_iterator(ಸಂರಚನಾ loopdev_cxt *lc, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಸಂರಚನಾ loopdev_iter *iter;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, ಪರಿಮಾಣ(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ಯದಿ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ಯದಿ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_deinit_iterator(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ loopdev_iter *iter;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	ಮುಕ್ತ(iter->minors);
	ಯದಿ (iter->proc)
		fclose(iter->proc);
	ಯದಿ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, ಪರಿಮಾಣ(*iter));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loopiter_set_device(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *device)
{
	ಪೂರ್ಣಾಙ್ಕ rc = loopcxt_set_device(lc, device);
	ಪೂರ್ಣಾಙ್ಕ used;

	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯದಿ (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ಪ್ರತ್ಯಾಗಮ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, ರಿಕ್ತ) == 0;

	ಯದಿ ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ಪ್ರತ್ಯಾಗಮ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, ರಿಕ್ತ) );
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmpnum(ಧ್ರುವ ಶೂನ್ಯ *p1, ಧ್ರುವ ಶೂನ್ಯ *p2)
{
	ಪ್ರತ್ಯಾಗಮ (((* (ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *) p1) > (* (ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *) p2)) -
			((* (ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *) p1) < (* (ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loop_scandir(ಧ್ರುವ ವರ್ಣ *dirname, ಪೂರ್ಣಾಙ್ಕ **ary, ಪೂರ್ಣಾಙ್ಕ hasprefix)
{
	DIR *dir;
	ಸಂರಚನಾ dirent *d;
	unsigned ಪೂರ್ಣಾಙ್ಕ n, ಎಣಿಕೆ = 0, arylen = 0;

	ಯದಿ (!dirname || !ary)
		ಪ್ರತ್ಯಾಗಮ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಮುಕ್ತ(*ary);
	*ary = ರಿಕ್ತ;

	ಯಾವತ್((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		ಯದಿ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ಅನುವರ್ತ;
#endif
		ಯದಿ (is_dotdir_dirent(d))
			ಅನುವರ್ತ;

		ಯದಿ (hasprefix) {
			/* /dev/loop<N> */
			ಯದಿ (sscanf(d->d_name, "loop%u", &n) != 1)
				ಅನುವರ್ತ;
		} ಅನ್ಯಥಾ {
			/* /dev/loop/<N> */
			ವರ್ಣ *end = ರಿಕ್ತ;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			ಯದಿ (d->d_name == end || (end && *end) || errno)
				ಅನುವರ್ತ;
		}
		ಯದಿ (n < LOOPDEV_DEFAULT_NNODES)
			ಅನುವರ್ತ;			/* ignore loop<0..7> */

		ಯದಿ (ಎಣಿಕೆ + 1 > arylen) {
			ಪೂರ್ಣಾಙ್ಕ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
			ಯದಿ (!tmp) {
				ಮುಕ್ತ(*ary);
				*ary = ರಿಕ್ತ;
				closedir(dir);
				ಪ್ರತ್ಯಾಗಮ -1;
			}
			*ary = tmp;
		}
		ಯದಿ (*ary)
			(*ary)[ಎಣಿಕೆ++] = n;
	}
	ಯದಿ (ಎಣಿಕೆ && *ary)
		qsort(*ary, ಎಣಿಕೆ, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ), cmpnum);

	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loopcxt_next_from_proc(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ loopdev_iter *iter = &lc->iter;
	ವರ್ಣ buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ಯದಿ (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ಯದಿ (!iter->proc)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಯಾವತ್ (fgets(buf, ಪರಿಮಾಣ(buf), iter->proc)) {
		unsigned ಪೂರ್ಣಾಙ್ಕ m;
		ವರ್ಣ name[128 + 1];


		ಯದಿ (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ಅನುವರ್ತ;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ಯದಿ (loopiter_set_device(lc, name) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loopcxt_next_from_sysfs(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ loopdev_iter *iter = &lc->iter;
	ಸಂರಚನಾ dirent *d;
	ಪೂರ್ಣಾಙ್ಕ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ಯದಿ (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	ಯದಿ (!iter->sysblock)
		ಪ್ರತ್ಯಾಗಮ 1;

	fd = dirfd(iter->sysblock);

	ಯಾವತ್ ((d = readdir(iter->sysblock))) {
		ವರ್ಣ name[NAME_MAX + 18 + 1];
		ಸಂರಚನಾ stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ಯದಿ (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			ಅನುವರ್ತ;

		snprintf(name, ಪರಿಮಾಣ(name), "%s/loop/backing_file", d->d_name);
		ಯದಿ (fstatat(fd, name, &st, 0) != 0)
			ಅನುವರ್ತ;

		ಯದಿ (loopiter_set_device(lc, d->d_name) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_next(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ loopdev_iter *iter;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;


	iter = &lc->iter;
	ಯದಿ (iter->done)
		ಪ್ರತ್ಯಾಗಮ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ಯದಿ (iter->flags & LOOPITER_FL_USED) {
		ಪೂರ್ಣಾಙ್ಕ rc;

		ಯದಿ (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		ಅನ್ಯಥಾ
			rc = loopcxt_next_from_proc(lc);
		ಯದಿ (rc == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಗಚ್ಛ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ಯದಿ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		ಪ್ರತಿ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			ವರ್ಣ name[16];
			snprintf(name, ಪರಿಮಾಣ(name), "loop%d", iter->ncur);

			ಯದಿ (loopiter_set_device(lc, name) == 0)
				ಪ್ರತ್ಯಾಗಮ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ಯದಿ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	ಪ್ರತಿ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		ವರ್ಣ name[16];
		snprintf(name, ಪರಿಮಾಣ(name), "loop%d", iter->minors[iter->ncur]);

		ಯದಿ (loopiter_set_device(lc, name) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * @device: path to device
 */
ಪೂರ್ಣಾಙ್ಕ is_loopdev(ಧ್ರುವ ವರ್ಣ *device)
{
	ಸಂರಚನಾ stat st;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ಅನ್ಯಥಾ ಯದಿ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ಅನ್ಯಥಾ ಯದಿ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		ವರ್ಣ name[PATH_MAX], *cn, *p = ರಿಕ್ತ;

		snprintf(name, ಪರಿಮಾಣ(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		ಯದಿ (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		ಮುಕ್ತ(cn);
	}

	ಯದಿ (rc == 0)
		errno = ENODEV;
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ಸಂರಚನಾ loop_info64 *loopcxt_get_info(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ fd;

	ಯದಿ (!lc || lc->info_failed) {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	errno = 0;
	ಯದಿ (lc->has_info)
		ಪ್ರತ್ಯಾಗಮ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ಪ್ರತ್ಯಾಗಮ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
ವರ್ಣ *loopcxt_get_backing_file(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	ಯದಿ (!res && loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);

		ಯದಿ (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((ವರ್ಣ *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
ವರ್ಣ *loopcxt_get_refname(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ವರ್ಣ *res = ರಿಕ್ತ;
	ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);

	ಯದಿ (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((ವರ್ಣ *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_offset(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t *offset)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;

	ಯದಿ (sysfs)
		ಯದಿ (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	ಯದಿ (rc && loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
		ಯದಿ (lo) {
			ಯದಿ (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} ಅನ್ಯಥಾ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_blocksize(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t *blocksize)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;

	ಯದಿ (sysfs)
		ಯದಿ (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ಯದಿ (rc) {
		ಪೂರ್ಣಾಙ್ಕ fd = loopcxt_get_fd(lc);
		ಪೂರ್ಣಾಙ್ಕ sz = 0;

		ಯದಿ (fd < 0)
			ಪ್ರತ್ಯಾಗಮ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_sizelimit(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t *size)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;

	ಯದಿ (sysfs)
		ಯದಿ (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	ಯದಿ (rc && loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
		ಯದಿ (lo) {
			ಯದಿ (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} ಅನ್ಯಥಾ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_encrypt_type(ಸಂರಚನಾ loopdev_cxt *lc, uint32_t *type)
{
	ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಙ್ಕ rc;

	/* not provided by sysfs */
	ಯದಿ (lo) {
		ಯದಿ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ಅನ್ಯಥಾ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ಧ್ರುವ ವರ್ಣ *loopcxt_get_crypt_name(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);

	ಯದಿ (lo)
		ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_backing_devno(ಸಂರಚನಾ loopdev_cxt *lc, dev_t *devno)
{
	ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (lo) {
		ಯದಿ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ಅನ್ಯಥಾ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_get_backing_inode(ಸಂರಚನಾ loopdev_cxt *lc, ino_t *ino)
{
	ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (lo) {
		ಯದಿ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ಅನ್ಯಥಾ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ loopmod_supports_partscan(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ rc, ret = 0;
	FILE *f;

	ಯದಿ (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ಪ್ರತ್ಯಾಗಮ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ಪ್ರತ್ಯಾಗಮ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_is_partscan(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಯದಿ (sysfs) {
		/* kernel >= 3.2 */
		ಪೂರ್ಣಾಙ್ಕ fl;
		ಯದಿ (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ಪ್ರತ್ಯಾಗಮ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ಪ್ರತ್ಯಾಗಮ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_is_autoclear(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಯದಿ (sysfs) {
		ಪೂರ್ಣಾಙ್ಕ fl;
		ಯದಿ (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ಪ್ರತ್ಯಾಗಮ fl;
	}

	ಯದಿ (loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
		ಯದಿ (lo)
			ಪ್ರತ್ಯಾಗಮ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_is_readonly(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಯದಿ (sysfs) {
		ಪೂರ್ಣಾಙ್ಕ fl;
		ಯದಿ (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ಪ್ರತ್ಯಾಗಮ fl;
	}

	ಯದಿ (loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
		ಯದಿ (lo)
			ಪ್ರತ್ಯಾಗಮ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_is_dio(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಸಂರಚನಾ path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಯದಿ (sysfs) {
		ಪೂರ್ಣಾಙ್ಕ fl;
		ಯದಿ (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ಪ್ರತ್ಯಾಗಮ fl;
	}
	ಯದಿ (loopcxt_ioctl_enabled(lc)) {
		ಸಂರಚನಾ loop_info64 *lo = loopcxt_get_info(lc);
		ಯದಿ (lo)
			ಪ್ರತ್ಯಾಗಮ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_is_used(ಸಂರಚನಾ loopdev_cxt *lc,
		    ಸಂರಚನಾ stat *st,
		    ಧ್ರುವ ವರ್ಣ *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    ಪೂರ್ಣಾಙ್ಕ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ಯದಿ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ಯದಿ (ino == st->st_ino && dev == st->st_dev)
			ಗಚ್ಛ found;

		/* don't use filename if we have devno and inode */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* poor man's solution */
	ಯದಿ (backing_file) {
		ವರ್ಣ *name = loopcxt_get_backing_file(lc);
		ಪೂರ್ಣಾಙ್ಕ rc = name && strcmp(name, backing_file) == 0;

		ಮುಕ್ತ(name);
		ಯದಿ (rc)
			ಗಚ್ಛ found;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
found:
	ಯದಿ (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		ಪೂರ್ಣಾಙ್ಕ rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		ಯದಿ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ಪ್ರತ್ಯಾಗಮ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ಪ್ರತ್ಯಾಗಮ rc;
	}
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_offset(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t offset)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_sizelimit(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t sizelimit)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_blocksize(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t blocksize)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_flags(ಸಂರಚನಾ loopdev_cxt *lc, uint32_t flags)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_refname(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *refname)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, ಪರಿಮಾಣ(lc->config.info.lo_file_name));
	ಯದಿ (refname)
		xstrncpy((ವರ್ಣ *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (ವರ್ಣ *)lc->config.info.lo_file_name));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_set_backing_file(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *filename)
{
	ಯದಿ (!lc)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ಅನ್ಯಥಾ
		lc->filename = ul_canonicalize_path(filename);
	ಯದಿ (!lc->filename)
		ಪ್ರತ್ಯಾಗಮ -errno;

	ಯದಿ (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loopcxt_check_size(ಸಂರಚನಾ loopdev_cxt *lc, ಪೂರ್ಣಾಙ್ಕ file_fd)
{
	uint64_t size, expected_size;
	ಪೂರ್ಣಾಙ್ಕ dev_fd;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ಪ್ರತ್ಯಾಗಮ -errno;
	}
	ಯದಿ (S_ISBLK(st.st_mode)) {
		ಯದಿ (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ಪ್ರತ್ಯಾಗಮ -errno;
		}
	} ಅನ್ಯಥಾ
		expected_size = st.st_size;

	ಯದಿ (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ಪ್ರತ್ಯಾಗಮ 0;	/* ignore this error */
	}

	ಯದಿ (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	ಯದಿ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ಯದಿ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ಯದಿ (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ಪ್ರತ್ಯಾಗಮ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ಯದಿ (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	ಯದಿ (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		ಯದಿ (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			ಯದಿ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ಪ್ರತ್ಯಾಗಮ -errno;
		}

		ಯದಿ (blkdev_get_size(dev_fd, &size))
			ಪ್ರತ್ಯಾಗಮ -errno;

		ಯದಿ (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ಪ್ರತ್ಯಾಗಮ -errno;
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_setup_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	ಪೂರ್ಣಾಙ್ಕ rc = -1, cnt = 0;
	ಪೂರ್ಣಾಙ್ಕ errsv = 0;
	ಪೂರ್ಣಾಙ್ಕ fallback = 0;

	ಯದಿ (!lc || !*lc->device || !lc->filename)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ಯದಿ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	ಯದಿ (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	ಯದಿ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	ಅನ್ಯಥಾ
		file_fd = open(lc->filename, mode | flags);

	ಯದಿ (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		ಯದಿ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		ಅನ್ಯಥಾ
			file_fd = open(lc->filename, mode | flags);
	}
	ಯದಿ (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ಪ್ರತ್ಯಾಗಮ -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	ಯದಿ (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ಅನ್ಯಥಾ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	ಕುರು {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		ಯದಿ (dev_fd >= 0 || lc->control_ok == 0)
			ಭಙ್ಗ;
		ಯದಿ (errno != EACCES && errno != ENOENT)
			ಭಙ್ಗ;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ಯಾವತ್ (cnt++ < 16);

	ಯದಿ (dev_fd < 0) {
		rc = -errno;
		ಗಚ್ಛ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ಯದಿ (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ಯದಿ (rc != 0) {
		errsv = errno;
		ಯದಿ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			ಗಚ್ಛ err;
		}
		fallback = 1;
	} ಅನ್ಯಥಾ {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ಯದಿ (fallback) {
		ಯದಿ (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			ಗಚ್ಛ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		ಯದಿ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			ಗಚ್ಛ err;
		}

		ಯದಿ ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			ಗಚ್ಛ err;
		}
	}

	ಯದಿ ((rc = loopcxt_check_size(lc, file_fd)))
		ಗಚ್ಛ err;

	close(file_fd);

	memset(&lc->config, 0, ಪರಿಮಾಣ(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ಪ್ರತ್ಯಾಗಮ 0;
err:
	ಯದಿ (file_fd >= 0)
		close(file_fd);
	ಯದಿ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ಯದಿ (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ loopcxt_ioctl_status(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ಯದಿ (dev_fd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	ಯದಿ (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ಪ್ರತ್ಯಾಗಮ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_ioctl_capacity(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ rc, fd = loopcxt_get_fd(lc);

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	ಯದಿ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ಪ್ರತ್ಯಾಗಮ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_ioctl_dio(ಸಂರಚನಾ loopdev_cxt *lc, unsigned long use_dio)
{
	ಪೂರ್ಣಾಙ್ಕ rc, fd = loopcxt_get_fd(lc);

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	ಯದಿ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ಪ್ರತ್ಯಾಗಮ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_ioctl_blocksize(ಸಂರಚನಾ loopdev_cxt *lc, uint64_t blocksize)
{
	ಪೂರ್ಣಾಙ್ಕ rc, fd = loopcxt_get_fd(lc);

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	ಯದಿ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ಪ್ರತ್ಯಾಗಮ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loopcxt_get_device_nr(ಸಂರಚನಾ loopdev_cxt *lc, ಪೂರ್ಣಾಙ್ಕ *nr)
{
	ಧ್ರುವ ವರ್ಣ *p, *dev;
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;

	errno = 0;
	ಯದಿ (!lc || !nr)
		ಪ್ರತ್ಯಾಗಮ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ಯದಿ (!dev)
		ಗಚ್ಛ done;

	p = strrchr(dev, '/');
	ಯದಿ (!p)
		ಗಚ್ಛ done;

	ಯದಿ (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		ಗಚ್ಛ done;

	ಯದಿ (*nr < 0)
		ಗಚ್ಛ done;
	rc = 0;
done:
	ಯದಿ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_detach_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ rc, fd = loopcxt_get_fd(lc);

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ಯದಿ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ಪ್ರತ್ಯಾಗಮ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_remove_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;
	ಪೂರ್ಣಾಙ್ಕ ctl, nr = -1;

	ಯದಿ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ಗಚ್ಛ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ಯದಿ (rc)
		ಗಚ್ಛ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ಯದಿ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ loopcxt_add_device(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;
	ಪೂರ್ಣಾಙ್ಕ ctl, nr = -1;

	ಯದಿ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ಗಚ್ಛ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ಯದಿ (rc)
		ಗಚ್ಛ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ಯದಿ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_find_unused(ಸಂರಚನಾ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ಯದಿ (lc->flags & LOOPDEV_FL_CONTROL) {
		ಪೂರ್ಣಾಙ್ಕ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ಯದಿ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ಅನ್ಯಥಾ
			rc = -errno;
		ಯದಿ (rc >= 0) {
			ವರ್ಣ name[16];
			snprintf(name, ಪರಿಮಾಣ(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ಯದಿ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ಯದಿ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ಯದಿ (rc)
			ಪ್ರತ್ಯಾಗಮ -ENOENT;
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}



/*
 * Return: TRUE/FALSE
 */
ಪೂರ್ಣಾಙ್ಕ loopdev_is_autoclear(ಧ್ರುವ ವರ್ಣ *device)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!device)
		ಪ್ರತ್ಯಾಗಮ 0;

	rc = loopcxt_init(&lc, 0);
	ಯದಿ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ಯದಿ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ಪ್ರತ್ಯಾಗಮ rc;
}

ವರ್ಣ *loopdev_get_backing_file(ಧ್ರುವ ವರ್ಣ *device)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (!device)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (loopcxt_init(&lc, 0))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * Returns: TRUE/FALSE
 */
ಪೂರ್ಣಾಙ್ಕ loopdev_has_backing_file(ಧ್ರುವ ವರ್ಣ *device)
{
	ವರ್ಣ *tmp = loopdev_get_backing_file(device);

	ಯದಿ (tmp) {
		ಮುಕ್ತ(tmp);
		ಪ್ರತ್ಯಾಗಮ 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Returns: TRUE/FALSE
 */
ಪೂರ್ಣಾಙ್ಕ loopdev_is_used(ಧ್ರುವ ವರ್ಣ *device, ಧ್ರುವ ವರ್ಣ *filename,
		    uint64_t offset, uint64_t sizelimit, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ಸಂರಚನಾ stat st;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (!device || !filename)
		ಪ್ರತ್ಯಾಗಮ 0;

	rc = loopcxt_init(&lc, 0);
	ಯದಿ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : ರಿಕ್ತ, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ಪೂರ್ಣಾಙ್ಕ loopdev_detach(ಧ್ರುವ ವರ್ಣ *device)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!device)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ಯದಿ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ಯದಿ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_find_by_backing_file(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *filename,
				 uint64_t offset, uint64_t sizelimit, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪೂರ್ಣಾಙ್ಕ rc, hasst;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!filename)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯಾವತ್ ((rc = loopcxt_next(lc)) == 0) {

		ಯದಿ (loopcxt_is_used(lc, hasst ? &st : ರಿಕ್ತ,
				    filename, offset, sizelimit, flags))
			ಭಙ್ಗ;
	}

	loopcxt_deinit_iterator(lc);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ಪೂರ್ಣಾಙ್ಕ loopcxt_find_overlap(ಸಂರಚನಾ loopdev_cxt *lc, ಧ್ರುವ ವರ್ಣ *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	ಪೂರ್ಣಾಙ್ಕ rc, hasst;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!filename)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಯಾವತ್ ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : ರಿಕ್ತ,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ಯದಿ (rc <= 0)
			ಅನುವರ್ತ;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		ಯದಿ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ಅನುವರ್ತ;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ಯದಿ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ಅನುವರ್ತ;
		}

		/* full match */
		ಯದಿ (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			ಗಚ್ಛ found;
		}

		/* overlap */
		ಯದಿ (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			ಅನುವರ್ತ;
		ಯದಿ (sizelimit != 0 && offset + sizelimit <= lc_offset)
			ಅನುವರ್ತ;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			ಗಚ್ಛ found;
	}

	ಯದಿ (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns allocated string with device name
 */
ವರ್ಣ *loopdev_find_by_backing_file(ಧ್ರುವ ವರ್ಣ *filename, uint64_t offset, uint64_t sizelimit, ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (!filename)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (loopcxt_init(&lc, 0))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ಪೂರ್ಣಾಙ್ಕ loopdev_ಎಣಿಕೆ_by_backing_file(ಧ್ರುವ ವರ್ಣ *filename, ವರ್ಣ **loopdev)
{
	ಸಂರಚನಾ loopdev_cxt lc;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0, rc;

	ಯದಿ (!filename)
		ಪ್ರತ್ಯಾಗಮ -1;

	rc = loopcxt_init(&lc, 0);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;
	ಯದಿ (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯಾವತ್(loopcxt_next(&lc) == 0) {
		ವರ್ಣ *backing = loopcxt_get_backing_file(&lc);

		ಯದಿ (!backing || strcmp(backing, filename) != 0) {
			ಮುಕ್ತ(backing);
			ಅನುವರ್ತ;
		}

		ಮುಕ್ತ(backing);
		ಯದಿ (loopdev && ಎಣಿಕೆ == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		ಎಣಿಕೆ++;
	}

	loopcxt_deinit(&lc);

	ಯದಿ (loopdev && ಎಣಿಕೆ > 1) {
		ಮುಕ್ತ(*loopdev);
		*loopdev = ರಿಕ್ತ;
	}
	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}

#ifdef TEST_PROGRAM_LOOPDEV
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಯದಿ (argc < 2)
		ಗಚ್ಛ usage;

	ಯದಿ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ಅನ್ಯಥಾ
		ಗಚ್ಛ usage;

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif

