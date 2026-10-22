
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
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "linux_version.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sysfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "loopdev.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "canonicalize.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "blkdev.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOPDEV_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOPDEV_DEBUG_CXT	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOPDEV_DEBUG_ITER	(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOOPDEV_DEBUG_ਤਿਆਰੀ	(1 << 4)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ਸਥਿਰ ਖਾਲੀ loopdev_init_debug(ਖਾਲੀ)
{
	ਜੇ (loopdev_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ loopcxt_sysfs_ਉਪਲਬਧ(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ repeat_on_eagain(x) __extension__ ({			\
		ਪੂਰਨ _c = 0, _e;					\
		ਕਰੋ {						\
			errno = 0;				\
			_e = x;					\
			ਜੇ (_e == 0 || errno != EAGAIN)		\
				ਰੋਕੋ;				\
			ਜੇ (_c >= LOOPDEV_MAX_TRIES)		\
				ਰੋਕੋ;				\
			xusleep(250000);			\
			_c++;					\
		} ਜਦੋਂ (1);					\
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
ਪੂਰਨ loopcxt_ਸੈੱਟ_device(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *device)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;

	ਜੇ (lc->fd >= 0) {
		close(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_ਗੁਆਚਾ = 0;
	lc->devno = 0;
	lc->ਬਹੁਲਕ = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	memset(&lc->config, 0, ਆਕਾਰ(lc->config));

	/* set new */
	ਜੇ (device) {
		ਜੇ (*device != '/') {
			ਅਚਲ ਅੱਖਰ *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ਜੇ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ਜੇ (ਲੜੀ_ਲੰਬਾਈ(device) < 5)
					ਵਾਪਸ -1;
				device += 4;
				dir = ਰਸਤਾ_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, ਆਕਾਰ(lc->device), "%s%s",
				dir, device);
		} ਨਹੀਂਤਾਂ
			xstrncpy(lc->device, device, ਆਕਾਰ(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_ਰਸਤਾ(lc->sysfs);
	lc->sysfs = ਨਿਰਾ;
	ਵਾਪਸ 0;
}

ਪੂਰਨ loopcxt_has_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਵਾਪਸ lc && *lc->device;
}

dev_t loopcxt_get_devno(ਬਣਤਰ loopdev_cxt *lc)
{
	ਜੇ (!lc || !loopcxt_has_device(lc))
		ਵਾਪਸ 0;
	ਜੇ (!lc->devno)
		lc->devno = sysfs_devname_ਮੰਜ਼ਿਲ_devno(lc->device);
	ਵਾਪਸ lc->devno;
}

ਪੂਰਨ loopcxt_is_ਗੁਆਚਾ(ਬਣਤਰ loopdev_cxt *lc)
{
	ਜੇ (!lc || !loopcxt_has_device(lc))
		ਵਾਪਸ 0;
	ਜੇ (lc->is_ਗੁਆਚਾ)
		ਵਾਪਸ 1;

	lc->is_ਗੁਆਚਾ = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ਵਾਪਸ lc->is_ਗੁਆਚਾ;
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
ਪੂਰਨ loopcxt_init(ਬਣਤਰ loopdev_cxt *lc, ਪੂਰਨ flags)
{
	ਪੂਰਨ rc;
	ਬਣਤਰ stat st;
	ਬਣਤਰ loopdev_cxt dummy = UL_LOOPDEVCXT_ਖਾਲੀ_ਥਾਵਾਂ;

	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(lc, &dummy, ਆਕਾਰ(dummy));
	lc->flags = flags;

	rc = loopcxt_ਸੈੱਟ_device(lc, ਨਿਰਾ);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	ਜੇ (stat(ਰਸਤਾ_SYS_ਖੰਡ, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ਜੇ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_ਸੰਸਕਰਣ() >= KERNEL_ਸੰਸਕਰਣ(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ਜੇ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(ਰਸਤਾ_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ਵਾਪਸ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
ਖਾਲੀ loopcxt_deinit(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ errsv = errno;

	ਜੇ (!lc)
		ਵਾਪਸ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	ਮੁਕਤ(lc->filename);
	lc->filename = ਨਿਰਾ;

	ignore_ਨਤੀਜਾ( loopcxt_ਸੈੱਟ_device(lc, ਨਿਰਾ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
ਅੱਖਰ *loopcxt_strdup_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਜੇ (!lc || !*lc->device)
		ਵਾਪਸ ਨਿਰਾ;
	ਵਾਪਸ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ਅਚਲ ਅੱਖਰ *loopcxt_get_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਵਾਪਸ lc && *lc->device ? lc->device : ਨਿਰਾ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ਸਥਿਰ ਬਣਤਰ ਰਸਤਾ_cxt *loopcxt_get_sysfs(ਬਣਤਰ loopdev_cxt *lc)
{
	ਜੇ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ਜੇ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ਵਾਪਸ ਨਿਰਾ;
		}

		lc->sysfs = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
		ਜੇ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ਵਾਪਸ lc->sysfs;
}

ਸਥਿਰ ਪੂਰਨ __loopcxt_get_fd(ਬਣਤਰ loopdev_cxt *lc, mode_t ਬਹੁਲਕ)
{
	ਪੂਰਨ old = -1;

	ਜੇ (!lc || !*lc->device)
		ਵਾਪਸ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ਜੇ (lc->fd >= 0 && ਬਹੁਲਕ == O_RDWR && lc->ਬਹੁਲਕ == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ਜੇ (lc->fd < 0) {
		lc->ਬਹੁਲਕ = ਬਹੁਲਕ;
		lc->fd = open(lc->device, lc->ਬਹੁਲਕ | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				ਬਹੁਲਕ == O_RDONLY ? "ro" :
				ਬਹੁਲਕ == O_RDWR ? "rw" : "??"));

		ਜੇ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ਜੇ (old >= 0)
		close(old);
	ਵਾਪਸ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ਪੂਰਨ loopcxt_get_fd(ਬਣਤਰ loopdev_cxt *lc)
{
	ਵਾਪਸ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_init_iterator(ਬਣਤਰ loopdev_cxt *lc, ਪੂਰਨ flags)
{
	ਬਣਤਰ loopdev_iter *iter;
	ਬਣਤਰ stat st;

	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, ਆਕਾਰ(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ਜੇ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ਜੇ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(ਰਸਤਾ_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ਵਾਪਸ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_deinit_iterator(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ loopdev_iter *iter;

	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	ਮੁਕਤ(iter->minors);
	ਜੇ (iter->proc)
		fclose(iter->proc);
	ਜੇ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, ਆਕਾਰ(*iter));
	ਵਾਪਸ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ਸਥਿਰ ਪੂਰਨ loopiter_ਸੈੱਟ_device(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *device)
{
	ਪੂਰਨ rc = loopcxt_ਸੈੱਟ_device(lc, device);
	ਪੂਰਨ ਵਰਤਿਆ;

	ਜੇ (rc)
		ਵਾਪਸ rc;

	ਜੇ (!(lc->iter.flags & LOOPITER_FL_ਵਰਤਿਆ) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ਵਾਪਸ 0;	/* caller does not care about device status */

	ਵਰਤਿਆ = loopcxt_get_ਵਿੱਥ(lc, ਨਿਰਾ) == 0;

	ਜੇ ((lc->iter.flags & LOOPITER_FL_ਵਰਤਿਆ) && ਵਰਤਿਆ)
		ਵਾਪਸ 0;

	ਜੇ ((lc->iter.flags & LOOPITER_FL_FREE) && !ਵਰਤਿਆ)
		ਵਾਪਸ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_ਨਤੀਜਾ( loopcxt_ਸੈੱਟ_device(lc, ਨਿਰਾ) );
	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ cmpnum(ਅਚਲ ਖਾਲੀ *p1, ਅਚਲ ਖਾਲੀ *p2)
{
	ਵਾਪਸ (((* (ਅਚਲ ਪੂਰਨ *) p1) > (* (ਅਚਲ ਪੂਰਨ *) p2)) -
			((* (ਅਚਲ ਪੂਰਨ *) p1) < (* (ਅਚਲ ਪੂਰਨ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ਸਥਿਰ ਪੂਰਨ loop_scandir(ਅਚਲ ਅੱਖਰ *dirname, ਪੂਰਨ **ary, ਪੂਰਨ hasprefix)
{
	DIR *dir;
	ਬਣਤਰ dirent *d;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n, ਗਿਣਤੀ_2 = 0, arylen = 0;

	ਜੇ (!dirname || !ary)
		ਵਾਪਸ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ਜੇ (!dir)
		ਵਾਪਸ 0;
	ਮੁਕਤ(*ary);
	*ary = ਨਿਰਾ;

	ਜਦੋਂ((d = readdir(dir))) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
		ਜੇ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ਜਾਰੀ;
#ਜੇ_ਅੰਤ
		ਜੇ (is_dotdir_dirent(d))
			ਜਾਰੀ;

		ਜੇ (hasprefix) {
			/* /dev/loop<N> */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(d->d_name, "loop%u", &n) != 1)
				ਜਾਰੀ;
		} ਨਹੀਂਤਾਂ {
			/* /dev/loop/<N> */
			ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

			errno = 0;
			n = strtol(d->d_name, &ਸਿਰਾ, 10);
			ਜੇ (d->d_name == ਸਿਰਾ || (ਸਿਰਾ && *ਸਿਰਾ) || errno)
				ਜਾਰੀ;
		}
		ਜੇ (n < LOOPDEV_DEFAULT_NNODES)
			ਜਾਰੀ;			/* ignore loop<0..7> */

		ਜੇ (ਗਿਣਤੀ_2 + 1 > arylen) {
			ਪੂਰਨ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, ਆਕਾਰ(ਪੂਰਨ));
			ਜੇ (!tmp) {
				ਮੁਕਤ(*ary);
				*ary = ਨਿਰਾ;
				closedir(dir);
				ਵਾਪਸ -1;
			}
			*ary = tmp;
		}
		ਜੇ (*ary)
			(*ary)[ਗਿਣਤੀ_2++] = n;
	}
	ਜੇ (ਗਿਣਤੀ_2 && *ary)
		qsort(*ary, ਗਿਣਤੀ_2, ਆਕਾਰ(ਪੂਰਨ), cmpnum);

	closedir(dir);
	ਵਾਪਸ ਗਿਣਤੀ_2;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ਸਥਿਰ ਪੂਰਨ loopcxt_ਅਗਲਾ_from_proc(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ loopdev_iter *iter = &lc->iter;
	ਅੱਖਰ buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ਜੇ (!iter->proc)
		iter->proc = fopen(ਰਸਤਾ_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ਜੇ (!iter->proc)
		ਵਾਪਸ 1;

	ਜਦੋਂ (fgets(buf, ਆਕਾਰ(buf), iter->proc)) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ m;
		ਅੱਖਰ name[128 + 1];


		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ਜਾਰੀ;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ਜੇ (loopiter_ਸੈੱਟ_device(lc, name) == 0)
			ਵਾਪਸ 0;
	}

	ਵਾਪਸ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ਸਥਿਰ ਪੂਰਨ loopcxt_ਅਗਲਾ_from_sysfs(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ loopdev_iter *iter = &lc->iter;
	ਬਣਤਰ dirent *d;
	ਪੂਰਨ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ਜੇ (!iter->sysblock)
		iter->sysblock = opendir(ਰਸਤਾ_SYS_ਖੰਡ);

	ਜੇ (!iter->sysblock)
		ਵਾਪਸ 1;

	fd = dirfd(iter->sysblock);

	ਜਦੋਂ ((d = readdir(iter->sysblock))) {
		ਅੱਖਰ name[NAME_MAX + 18 + 1];
		ਬਣਤਰ stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ਜੇ (is_dotdir_dirent(d) || ਲੜੀ_ਤੁਲਨਾ(d->d_name, "loop", 4) != 0)
			ਜਾਰੀ;

		snprintf(name, ਆਕਾਰ(name), "%s/loop/backing_file", d->d_name);
		ਜੇ (fstatat(fd, name, &st, 0) != 0)
			ਜਾਰੀ;

		ਜੇ (loopiter_ਸੈੱਟ_device(lc, d->d_name) == 0)
			ਵਾਪਸ 0;
	}

	ਵਾਪਸ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ਪੂਰਨ loopcxt_ਅਗਲਾ(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ loopdev_iter *iter;

	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;


	iter = &lc->iter;
	ਜੇ (iter->ਪੂਰਾ)
		ਵਾਪਸ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ਜੇ (iter->flags & LOOPITER_FL_ਵਰਤਿਆ) {
		ਪੂਰਨ rc;

		ਜੇ (loopcxt_sysfs_ਉਪਲਬਧ(lc))
			rc = loopcxt_ਅਗਲਾ_from_sysfs(lc);
		ਨਹੀਂਤਾਂ
			rc = loopcxt_ਅਗਲਾ_from_proc(lc);
		ਜੇ (rc == 0)
			ਵਾਪਸ 0;
		ਜਾਓ ਪੂਰਾ;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ਜੇ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		ਲਈ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			ਅੱਖਰ name[16];
			snprintf(name, ਆਕਾਰ(name), "loop%d", iter->ncur);

			ਜੇ (loopiter_ਸੈੱਟ_device(lc, name) == 0)
				ਵਾਪਸ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ਜੇ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(ਰਸਤਾ_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	ਲਈ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		ਅੱਖਰ name[16];
		snprintf(name, ਆਕਾਰ(name), "loop%d", iter->minors[iter->ncur]);

		ਜੇ (loopiter_ਸੈੱਟ_device(lc, name) == 0)
			ਵਾਪਸ 0;
	}
ਪੂਰਾ:
	loopcxt_deinit_iterator(lc);
	ਵਾਪਸ 1;
}

/*
 * @device: path to device
 */
ਪੂਰਨ is_loopdev(ਅਚਲ ਅੱਖਰ *device)
{
	ਬਣਤਰ stat st;
	ਪੂਰਨ rc = 0;

	ਜੇ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ਨਹੀਂਤਾਂ ਜੇ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ਨਹੀਂਤਾਂ ਜੇ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		ਅੱਖਰ name[PATH_MAX], *cn, *p = ਨਿਰਾ;

		snprintf(name, ਆਕਾਰ(name), ਰਸਤਾ_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_ਰਸਤਾ(name);
		ਜੇ (cn)
			p = stripoff_ਆਖਰੀ_component(cn);
		rc = p && ul_startswith(p, "loop");
		ਮੁਕਤ(cn);
	}

	ਜੇ (rc == 0)
		errno = ENODEV;
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ਬਣਤਰ loop_info64 *loopcxt_get_info(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ fd;

	ਜੇ (!lc || lc->info_failed) {
		errno = EINVAL;
		ਵਾਪਸ ਨਿਰਾ;
	}
	errno = 0;
	ਜੇ (lc->has_info)
		ਵਾਪਸ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ਜੇ (fd < 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ਵਾਪਸ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
ਅੱਖਰ *loopcxt_get_backing_file(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_ਰਸਤਾ_read_string(sysfs, &res, "loop/backing_file");

	ਜੇ (!res && loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);

		ਜੇ (lo) {
			lo->lo_file_name[LO_NAME_ਮਾਪ - 2] = '*';
			lo->lo_file_name[LO_NAME_ਮਾਪ - 1] = '\0';
			res = strdup((ਅੱਖਰ *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ਵਾਪਸ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
ਅੱਖਰ *loopcxt_get_refname(ਬਣਤਰ loopdev_cxt *lc)
{
	ਅੱਖਰ *res = ਨਿਰਾ;
	ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);

	ਜੇ (lo) {
		lo->lo_file_name[LO_NAME_ਮਾਪ - 1] = '\0';
		res = strdup((ਅੱਖਰ *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ਵਾਪਸ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_ਵਿੱਥ(ਬਣਤਰ loopdev_cxt *lc, uint64_t *ਵਿੱਥ)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ਪੂਰਨ rc = -EINVAL;

	ਜੇ (sysfs)
		ਜੇ (ul_ਰਸਤਾ_read_u64(sysfs, ਵਿੱਥ, "loop/offset") == 0)
			rc = 0;

	ਜੇ (rc && loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
		ਜੇ (lo) {
			ਜੇ (ਵਿੱਥ)
				*ਵਿੱਥ = lo->lo_ਵਿੱਥ;
			rc = 0;
		} ਨਹੀਂਤਾਂ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_blocksize(ਬਣਤਰ loopdev_cxt *lc, uint64_t *blocksize)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ਪੂਰਨ rc = -EINVAL;

	ਜੇ (sysfs)
		ਜੇ (ul_ਰਸਤਾ_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ਜੇ (rc) {
		ਪੂਰਨ fd = loopcxt_get_fd(lc);
		ਪੂਰਨ sz = 0;

		ਜੇ (fd < 0)
			ਵਾਪਸ -EINVAL;
		rc = blkdev_get_sector_ਮਾਪ(fd, &sz);
		ਜੇ (rc)
			ਵਾਪਸ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_sizelimit(ਬਣਤਰ loopdev_cxt *lc, uint64_t *ਮਾਪ)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ਪੂਰਨ rc = -EINVAL;

	ਜੇ (sysfs)
		ਜੇ (ul_ਰਸਤਾ_read_u64(sysfs, ਮਾਪ, "loop/sizelimit") == 0)
			rc = 0;

	ਜੇ (rc && loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
		ਜੇ (lo) {
			ਜੇ (ਮਾਪ)
				*ਮਾਪ = lo->lo_sizelimit;
			rc = 0;
		} ਨਹੀਂਤਾਂ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_encrypt_type(ਬਣਤਰ loopdev_cxt *lc, uint32_t *type)
{
	ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
	ਪੂਰਨ rc;

	/* not provided by sysfs */
	ਜੇ (lo) {
		ਜੇ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ਨਹੀਂਤਾਂ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ਅਚਲ ਅੱਖਰ *loopcxt_get_crypt_name(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);

	ਜੇ (lo)
		ਵਾਪਸ (ਅੱਖਰ *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_backing_devno(ਬਣਤਰ loopdev_cxt *lc, dev_t *devno)
{
	ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
	ਪੂਰਨ rc;

	ਜੇ (lo) {
		ਜੇ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ਨਹੀਂਤਾਂ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ਪੂਰਨ loopcxt_get_backing_inode(ਬਣਤਰ loopdev_cxt *lc, ino_t *ino)
{
	ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
	ਪੂਰਨ rc;

	ਜੇ (lo) {
		ਜੇ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ਨਹੀਂਤਾਂ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ਵਾਪਸ rc;
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
ਪੂਰਨ loopmod_supports_partscan(ਖਾਲੀ)
{
	ਪੂਰਨ rc, ret = 0;
	FILE *f;

	ਜੇ (get_linux_ਸੰਸਕਰਣ() >= KERNEL_ਸੰਸਕਰਣ(3,2,0))
		ਵਾਪਸ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ਜੇ (!f)
		ਵਾਪਸ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ਵਾਪਸ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ਪੂਰਨ loopcxt_is_partscan(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ਜੇ (sysfs) {
		/* kernel >= 3.2 */
		ਪੂਰਨ fl;
		ਜੇ (ul_ਰਸਤਾ_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ਵਾਪਸ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ਵਾਪਸ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ਪੂਰਨ loopcxt_is_autoclear(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ਜੇ (sysfs) {
		ਪੂਰਨ fl;
		ਜੇ (ul_ਰਸਤਾ_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ਵਾਪਸ fl;
	}

	ਜੇ (loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
		ਜੇ (lo)
			ਵਾਪਸ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ਵਾਪਸ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ਪੂਰਨ loopcxt_is_readonly(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ਜੇ (sysfs) {
		ਪੂਰਨ fl;
		ਜੇ (ul_ਰਸਤਾ_read_s32(sysfs, &fl, "ro") == 0)
			ਵਾਪਸ fl;
	}

	ਜੇ (loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
		ਜੇ (lo)
			ਵਾਪਸ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ਵਾਪਸ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ਪੂਰਨ loopcxt_is_dio(ਬਣਤਰ loopdev_cxt *lc)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ਜੇ (sysfs) {
		ਪੂਰਨ fl;
		ਜੇ (ul_ਰਸਤਾ_read_s32(sysfs, &fl, "loop/dio") == 0)
			ਵਾਪਸ fl;
	}
	ਜੇ (loopcxt_ioctl_enabled(lc)) {
		ਬਣਤਰ loop_info64 *lo = loopcxt_get_info(lc);
		ਜੇ (lo)
			ਵਾਪਸ lo->lo_flags & LO_FLAGS_ਸਿੱਧਾ_IO;
	}
	ਵਾਪਸ 0;
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
ਪੂਰਨ loopcxt_is_ਵਰਤਿਆ(ਬਣਤਰ loopdev_cxt *lc,
		    ਬਣਤਰ stat *st,
		    ਅਚਲ ਅੱਖਰ *backing_file,
		    uint64_t ਵਿੱਥ,
		    uint64_t sizelimit,
		    ਪੂਰਨ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ਜੇ (!lc)
		ਵਾਪਸ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ਜੇ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ਜੇ (ino == st->st_ino && dev == st->st_dev)
			ਜਾਓ ਮਿਲਿਆ;

		/* don't use filename if we have devno and inode */
		ਵਾਪਸ 0;
	}

	/* poor man's solution */
	ਜੇ (backing_file) {
		ਅੱਖਰ *name = loopcxt_get_backing_file(lc);
		ਪੂਰਨ rc = name && strcmp(name, backing_file) == 0;

		ਮੁਕਤ(name);
		ਜੇ (rc)
			ਜਾਓ ਮਿਲਿਆ;
	}

	ਵਾਪਸ 0;
ਮਿਲਿਆ:
	ਜੇ (flags & LOOPDEV_FL_ਵਿੱਥ) {
		uint64_t off = 0;

		ਪੂਰਨ rc = loopcxt_get_ਵਿੱਥ(lc, &off) == 0 && off == ਵਿੱਥ;

		ਜੇ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ਵਾਪਸ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ਵਾਪਸ rc;
	}
	ਵਾਪਸ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ਪੂਰਨ loopcxt_ਸੈੱਟ_ਵਿੱਥ(ਬਣਤਰ loopdev_cxt *lc, uint64_t ਵਿੱਥ)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;
	lc->config.info.lo_ਵਿੱਥ = ਵਿੱਥ;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, ਵਿੱਥ));
	ਵਾਪਸ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ਪੂਰਨ loopcxt_ਸੈੱਟ_sizelimit(ਬਣਤਰ loopdev_cxt *lc, uint64_t sizelimit)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ਵਾਪਸ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ਪੂਰਨ loopcxt_ਸੈੱਟ_blocksize(ਬਣਤਰ loopdev_cxt *lc, uint64_t blocksize)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ਵਾਪਸ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ਪੂਰਨ loopcxt_ਸੈੱਟ_flags(ਬਣਤਰ loopdev_cxt *lc, uint32_t flags)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (ਬਿਨਾਨਿਸ਼ਾਨੀ) flags));
	ਵਾਪਸ 0;
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
ਪੂਰਨ loopcxt_ਸੈੱਟ_refname(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *refname)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, ਆਕਾਰ(lc->config.info.lo_file_name));
	ਜੇ (refname)
		xstrncpy((ਅੱਖਰ *)lc->config.info.lo_file_name, refname, LO_NAME_ਮਾਪ);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (ਅੱਖਰ *)lc->config.info.lo_file_name));
	ਵਾਪਸ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ਪੂਰਨ loopcxt_ਸੈੱਟ_backing_file(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *filename)
{
	ਜੇ (!lc)
		ਵਾਪਸ -EINVAL;

	ਜੇ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ਨਹੀਂਤਾਂ
		lc->filename = ul_canonicalize_ਰਸਤਾ(filename);
	ਜੇ (!lc->filename)
		ਵਾਪਸ -errno;

	ਜੇ (!lc->config.info.lo_file_name[0])
		loopcxt_ਸੈੱਟ_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ loopcxt_check_ਮਾਪ(ਬਣਤਰ loopdev_cxt *lc, ਪੂਰਨ file_fd)
{
	uint64_t ਮਾਪ, expected_ਮਾਪ;
	ਪੂਰਨ dev_fd;
	ਬਣਤਰ stat st;

	ਜੇ (!lc->config.info.lo_ਵਿੱਥ && !lc->config.info.lo_sizelimit)
		ਵਾਪਸ 0;

	ਜੇ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ਵਾਪਸ -errno;
	}
	ਜੇ (S_ISBLK(st.st_mode)) {
		ਜੇ (blkdev_get_ਮਾਪ(file_fd, &expected_ਮਾਪ)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ਵਾਪਸ -errno;
		}
	} ਨਹੀਂਤਾਂ
		expected_ਮਾਪ = st.st_size;

	ਜੇ (expected_ਮਾਪ == 0 || expected_ਮਾਪ <= lc->config.info.lo_ਵਿੱਥ) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ਵਾਪਸ 0;	/* ignore this error */
	}

	ਜੇ (lc->config.info.lo_ਵਿੱਥ > 0)
		expected_ਮਾਪ -= lc->config.info.lo_ਵਿੱਥ;

	ਜੇ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_ਮਾਪ)
		expected_ਮਾਪ = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ਜੇ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ਵਾਪਸ -errno;
	}

	ਜੇ (blkdev_get_ਮਾਪ(dev_fd, &ਮਾਪ)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ਵਾਪਸ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ਜੇ (expected_ਮਾਪ % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_ਮਾਪ = (expected_ਮਾਪ >> 9) << 9;
	}

	ਜੇ (expected_ਮਾਪ != ਮਾਪ) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      ਮਾਪ, expected_ਮਾਪ));

		ਜੇ (loopcxt_ioctl_ਸਮਰੱਥਾ(lc)) {
			/* ioctl not available */
			ਜੇ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ਵਾਪਸ -errno;
		}

		ਜੇ (blkdev_get_ਮਾਪ(dev_fd, &ਮਾਪ))
			ਵਾਪਸ -errno;

		ਜੇ (expected_ਮਾਪ != ਮਾਪ) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					ਮਾਪ, expected_ਮਾਪ));
			ਵਾਪਸ -errno;
		}
	}

	ਵਾਪਸ 0;
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
ਪੂਰਨ loopcxt_ਤਿਆਰੀ_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, ਬਹੁਲਕ = O_RDWR;
	ਪੂਰਨ rc = -1, cnt = 0;
	ਪੂਰਨ errsv = 0;
	ਪੂਰਨ fallback = 0;

	ਜੇ (!lc || !*lc->device || !lc->filename)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ਜੇ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		ਬਹੁਲਕ = O_RDONLY;

	ਜੇ (lc->config.info.lo_flags & LO_FLAGS_ਸਿੱਧਾ_IO)
		flags |= O_ਸਿੱਧਾ;
	ਜੇ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, ਬਹੁਲਕ | flags, 0);
	ਨਹੀਂਤਾਂ
		file_fd = open(lc->filename, ਬਹੁਲਕ | flags);

	ਜੇ (file_fd < 0 && ਬਹੁਲਕ != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		ਬਹੁਲਕ = O_RDONLY;
		ਜੇ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, ਬਹੁਲਕ | flags, 0);
		ਨਹੀਂਤਾਂ
			file_fd = open(lc->filename, ਬਹੁਲਕ | flags);
	}
	ਜੇ (file_fd < 0) {
		DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("open backing file failed: %m"));
		ਵਾਪਸ -errno;
	}
	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("backing file open: OK"));

	ਜੇ (ਬਹੁਲਕ == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ਨਹੀਂਤਾਂ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	ਕਰੋ {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, ਬਹੁਲਕ);
		ਜੇ (dev_fd >= 0 || lc->control_ok == 0)
			ਰੋਕੋ;
		ਜੇ (errno != EACCES && errno != ENOENT)
			ਰੋਕੋ;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ਜਦੋਂ (cnt++ < 16);

	ਜੇ (dev_fd < 0) {
		rc = -errno;
		ਜਾਓ err;
	}

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ਜੇ (lc->blocksize > 0)
		lc->config.ਖੰਡ_ਮਾਪ = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ਜੇ (rc != 0) {
		errsv = errno;
		ਜੇ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			ਜਾਓ err;
		}
		fallback = 1;
	} ਨਹੀਂਤਾਂ {
		DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ਜੇ (fallback) {
		ਜੇ (ioctl(dev_fd, LOOP_ਸੈੱਟ_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_SET_FD failed: %m"));
			ਜਾਓ err;
		}

		DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_SET_FD: OK"));

		ਜੇ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			ਜਾਓ err;
		}

		ਜੇ ((rc = loopcxt_ioctl_ਸਥਿਤੀ(lc)) < 0) {
			errsv = -rc;
			ਜਾਓ err;
		}
	}

	ਜੇ ((rc = loopcxt_check_ਮਾਪ(lc, file_fd)))
		ਜਾਓ err;

	close(file_fd);

	memset(&lc->config, 0, ਆਕਾਰ(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("success [rc=0]"));
	ਵਾਪਸ 0;
err:
	ਜੇ (file_fd >= 0)
		close(file_fd);
	ਜੇ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ਜੇ (errsv)
		errno = errsv;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("failed [rc=%d]", rc));
	ਵਾਪਸ rc;
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
ਪੂਰਨ loopcxt_ioctl_ਸਥਿਤੀ(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ਜੇ (dev_fd < 0)
		ਵਾਪਸ -errno;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_ਸੈੱਟ_STATUS64, &lc->config.info) );
	ਜੇ (rc != 0) {
		DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ਵਾਪਸ rc;
	}

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ਵਾਪਸ 0;
}

ਪੂਰਨ loopcxt_ioctl_ਸਮਰੱਥਾ(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ rc, fd = loopcxt_get_fd(lc);

	ਜੇ (fd < 0)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_ਸੈੱਟ_ਸਮਰੱਥਾ, 0) );
	ਜੇ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ਵਾਪਸ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ਵਾਪਸ 0;
}

ਪੂਰਨ loopcxt_ioctl_dio(ਬਣਤਰ loopdev_cxt *lc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ use_dio)
{
	ਪੂਰਨ rc, fd = loopcxt_get_fd(lc);

	ਜੇ (fd < 0)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_ਸੈੱਟ_ਸਿੱਧਾ_IO, use_dio) );
	ਜੇ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ਵਾਪਸ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ਵਾਪਸ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ਪੂਰਨ loopcxt_ioctl_blocksize(ਬਣਤਰ loopdev_cxt *lc, uint64_t blocksize)
{
	ਪੂਰਨ rc, fd = loopcxt_get_fd(lc);

	ਜੇ (fd < 0)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_ਸੈੱਟ_ਖੰਡ_ਮਾਪ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) blocksize) );
	ਜੇ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ਵਾਪਸ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ loopcxt_get_device_nr(ਬਣਤਰ loopdev_cxt *lc, ਪੂਰਨ *nr)
{
	ਅਚਲ ਅੱਖਰ *p, *dev;
	ਪੂਰਨ rc = -EINVAL;

	errno = 0;
	ਜੇ (!lc || !nr)
		ਵਾਪਸ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ਜੇ (!dev)
		ਜਾਓ ਪੂਰਾ;

	p = strrchr(dev, '/');
	ਜੇ (!p)
		ਜਾਓ ਪੂਰਾ;

	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(p, "/loop%d", nr) != 1 && ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(p, "/%d", nr) != 1)
		ਜਾਓ ਪੂਰਾ;

	ਜੇ (*nr < 0)
		ਜਾਓ ਪੂਰਾ;
	rc = 0;
ਪੂਰਾ:
	ਜੇ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ਵਾਪਸ rc;
}

ਪੂਰਨ loopcxt_detach_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ rc, fd = loopcxt_get_fd(lc);

	ਜੇ (fd < 0)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(ਤਿਆਰੀ, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ਜੇ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ਵਾਪਸ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ਵਾਪਸ 0;
}

ਪੂਰਨ loopcxt_remove_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ rc = -EINVAL;
	ਪੂਰਨ ctl, nr = -1;

	ਜੇ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ਜਾਓ ਪੂਰਾ;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ਜੇ (rc)
		ਜਾਓ ਪੂਰਾ;

	ctl = open(ਰਸਤਾ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ਜੇ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
ਪੂਰਾ:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ਵਾਪਸ rc;
}

ਪੂਰਨ loopcxt_ਜੋੜੋ_device(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ rc = -EINVAL;
	ਪੂਰਨ ctl, nr = -1;

	ਜੇ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ਜਾਓ ਪੂਰਾ;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ਜੇ (rc)
		ਜਾਓ ਪੂਰਾ;

	ctl = open(ਰਸਤਾ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ਜੇ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ਜੋੜੋ, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
ਪੂਰਾ:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ਪੂਰਨ loopcxt_ਲੱਭੋ_ਅਣਵਰਤਿਆ(ਬਣਤਰ loopdev_cxt *lc)
{
	ਪੂਰਨ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ਜੇ (lc->flags & LOOPDEV_FL_CONTROL) {
		ਪੂਰਨ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(ਰਸਤਾ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ਜੇ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ਨਹੀਂਤਾਂ
			rc = -errno;
		ਜੇ (rc >= 0) {
			ਅੱਖਰ name[16];
			snprintf(name, ਆਕਾਰ(name), "loop%d", rc);

			rc = loopiter_ਸੈੱਟ_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ਜੇ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ਜੇ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ਜੇ (rc)
			ਵਾਪਸ rc;

		rc = loopcxt_ਅਗਲਾ(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ਜੇ (rc)
			ਵਾਪਸ -ENOENT;
	}
	ਵਾਪਸ rc;
}



/*
 * Return: TRUE/FALSE
 */
ਪੂਰਨ loopdev_is_autoclear(ਅਚਲ ਅੱਖਰ *device)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਪੂਰਨ rc;

	ਜੇ (!device)
		ਵਾਪਸ 0;

	rc = loopcxt_init(&lc, 0);
	ਜੇ (!rc)
		rc = loopcxt_ਸੈੱਟ_device(&lc, device);
	ਜੇ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ਵਾਪਸ rc;
}

ਅੱਖਰ *loopdev_get_backing_file(ਅਚਲ ਅੱਖਰ *device)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (!device)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (loopcxt_init(&lc, 0))
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (loopcxt_ਸੈੱਟ_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ਵਾਪਸ res;
}

/*
 * Returns: TRUE/FALSE
 */
ਪੂਰਨ loopdev_has_backing_file(ਅਚਲ ਅੱਖਰ *device)
{
	ਅੱਖਰ *tmp = loopdev_get_backing_file(device);

	ਜੇ (tmp) {
		ਮੁਕਤ(tmp);
		ਵਾਪਸ 1;
	}
	ਵਾਪਸ 0;
}

/*
 * Returns: TRUE/FALSE
 */
ਪੂਰਨ loopdev_is_ਵਰਤਿਆ(ਅਚਲ ਅੱਖਰ *device, ਅਚਲ ਅੱਖਰ *filename,
		    uint64_t ਵਿੱਥ, uint64_t sizelimit, ਪੂਰਨ flags)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਬਣਤਰ stat st;
	ਪੂਰਨ rc = 0;

	ਜੇ (!device || !filename)
		ਵਾਪਸ 0;

	rc = loopcxt_init(&lc, 0);
	ਜੇ (!rc)
		rc = loopcxt_ਸੈੱਟ_device(&lc, device);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_ਵਰਤਿਆ(&lc, rc ? &st : ਨਿਰਾ, filename, ਵਿੱਥ, sizelimit, flags);

	loopcxt_deinit(&lc);
	ਵਾਪਸ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ਪੂਰਨ loopdev_detach(ਅਚਲ ਅੱਖਰ *device)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਪੂਰਨ rc;

	ਜੇ (!device)
		ਵਾਪਸ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ਜੇ (!rc)
		rc = loopcxt_ਸੈੱਟ_device(&lc, device);
	ਜੇ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ਵਾਪਸ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ਪੂਰਨ loopcxt_ਲੱਭੋ_by_backing_file(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *filename,
				 uint64_t ਵਿੱਥ, uint64_t sizelimit, ਪੂਰਨ flags)
{
	ਪੂਰਨ rc, hasst;
	ਬਣਤਰ stat st;

	ਜੇ (!filename)
		ਵਾਪਸ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ਵਰਤਿਆ);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	ਜਦੋਂ ((rc = loopcxt_ਅਗਲਾ(lc)) == 0) {

		ਜੇ (loopcxt_is_ਵਰਤਿਆ(lc, hasst ? &st : ਨਿਰਾ,
				    filename, ਵਿੱਥ, sizelimit, flags))
			ਰੋਕੋ;
	}

	loopcxt_deinit_iterator(lc);
	ਵਾਪਸ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ਪੂਰਨ loopcxt_ਲੱਭੋ_overlap(ਬਣਤਰ loopdev_cxt *lc, ਅਚਲ ਅੱਖਰ *filename,
			   uint64_t ਵਿੱਥ, uint64_t sizelimit)
{
	ਪੂਰਨ rc, hasst;
	ਬਣਤਰ stat st;

	ਜੇ (!filename)
		ਵਾਪਸ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ਵਰਤਿਆ);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	ਜਦੋਂ ((rc = loopcxt_ਅਗਲਾ(lc)) == 0) {
		uint64_t lc_sizelimit, lc_ਵਿੱਥ;

		rc = loopcxt_is_ਵਰਤਿਆ(lc, hasst ? &st : ਨਿਰਾ,
				     filename, ਵਿੱਥ, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ਜੇ (rc <= 0)
			ਜਾਰੀ;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_ਵਿੱਥ(lc, &lc_ਵਿੱਥ);
		ਜੇ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ਜਾਰੀ;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ਜੇ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ਜਾਰੀ;
		}

		/* full match */
		ਜੇ (lc_sizelimit == sizelimit && lc_ਵਿੱਥ == ਵਿੱਥ) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			ਜਾਓ ਮਿਲਿਆ;
		}

		/* overlap */
		ਜੇ (lc_sizelimit != 0 && ਵਿੱਥ >= lc_ਵਿੱਥ + lc_sizelimit)
			ਜਾਰੀ;
		ਜੇ (sizelimit != 0 && ਵਿੱਥ + sizelimit <= lc_ਵਿੱਥ)
			ਜਾਰੀ;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			ਜਾਓ ਮਿਲਿਆ;
	}

	ਜੇ (rc == 1)
		rc = 0;	/* not found */
ਮਿਲਿਆ:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ਵਾਪਸ rc;
}

/*
 * Returns allocated string with device name
 */
ਅੱਖਰ *loopdev_ਲੱਭੋ_by_backing_file(ਅਚਲ ਅੱਖਰ *filename, uint64_t ਵਿੱਥ, uint64_t sizelimit, ਪੂਰਨ flags)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (!filename)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (loopcxt_init(&lc, 0))
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (loopcxt_ਲੱਭੋ_by_backing_file(&lc, filename, ਵਿੱਥ, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ਵਾਪਸ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ਪੂਰਨ loopdev_ਗਿਣਤੀ_by_backing_file(ਅਚਲ ਅੱਖਰ *filename, ਅੱਖਰ **loopdev)
{
	ਬਣਤਰ loopdev_cxt lc;
	ਪੂਰਨ ਗਿਣਤੀ_2 = 0, rc;

	ਜੇ (!filename)
		ਵਾਪਸ -1;

	rc = loopcxt_init(&lc, 0);
	ਜੇ (rc)
		ਵਾਪਸ rc;
	ਜੇ (loopcxt_init_iterator(&lc, LOOPITER_FL_ਵਰਤਿਆ))
		ਵਾਪਸ -1;

	ਜਦੋਂ(loopcxt_ਅਗਲਾ(&lc) == 0) {
		ਅੱਖਰ *backing = loopcxt_get_backing_file(&lc);

		ਜੇ (!backing || strcmp(backing, filename) != 0) {
			ਮੁਕਤ(backing);
			ਜਾਰੀ;
		}

		ਮੁਕਤ(backing);
		ਜੇ (loopdev && ਗਿਣਤੀ_2 == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		ਗਿਣਤੀ_2++;
	}

	loopcxt_deinit(&lc);

	ਜੇ (loopdev && ਗਿਣਤੀ_2 > 1) {
		ਮੁਕਤ(*loopdev);
		*loopdev = ਨਿਰਾ;
	}
	ਵਾਪਸ ਗਿਣਤੀ_2;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_LOOPDEV
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਜੇ (argc < 2)
		ਜਾਓ usage;

	ਜੇ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		ਲਿਖੋ("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ਨਹੀਂਤਾਂ
		ਜਾਓ usage;

	ਵਾਪਸ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ

