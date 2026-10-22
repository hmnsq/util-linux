
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

#ವ್ಯಾಖ್ಯಾನಿಸು LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು LOOPDEV_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು LOOPDEV_DEBUG_CXT	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು LOOPDEV_DEBUG_ITER	(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು LOOPDEV_DEBUG_ಸಿದ್ಧತೆ	(1 << 4)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ loopdev_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (loopdev_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#ವ್ಯಾಖ್ಯಾನಿಸು loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#ವ್ಯಾಖ್ಯಾನಿಸು loopcxt_sysfs_ಲಭ್ಯ(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#ವ್ಯಾಖ್ಯಾನಿಸು repeat_on_eagain(x) __extension__ ({			\
		ಪೂರ್ಣಾಂಕ _c = 0, _e;					\
		ಮಾಡು {						\
			errno = 0;				\
			_e = x;					\
			ಆದರೆ (_e == 0 || errno != EAGAIN)		\
				ನಿಲ್ಲಿಸು;				\
			ಆದರೆ (_c >= LOOPDEV_MAX_TRIES)		\
				ನಿಲ್ಲಿಸು;				\
			xusleep(250000);			\
			_c++;					\
		} ಆಗ (1);					\
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
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_device(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (lc->fd >= 0) {
		close(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_ಕಳೆದುಹೋದ = 0;
	lc->devno = 0;
	lc->ಬಹುಲಕ = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	memset(&lc->config, 0, ಗಾತ್ರ(lc->config));

	/* set new */
	ಆದರೆ (device) {
		ಆದರೆ (*device != '/') {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ಆದರೆ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(device) < 5)
					ಹಿಂತಿರುಗಿ -1;
				device += 4;
				dir = ಮಾರ್ಗ_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, ಗಾತ್ರ(lc->device), "%s%s",
				dir, device);
		} ಇಲ್ಲದಿದ್ದರೆ
			xstrncpy(lc->device, device, ಗಾತ್ರ(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_ಮಾರ್ಗ(lc->sysfs);
	lc->sysfs = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ loopcxt_has_device(ರಚನೆ loopdev_cxt *lc)
{
	ಹಿಂತಿರುಗಿ lc && *lc->device;
}

dev_t loopcxt_get_devno(ರಚನೆ loopdev_cxt *lc)
{
	ಆದರೆ (!lc || !loopcxt_has_device(lc))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (!lc->devno)
		lc->devno = sysfs_devname_ಗಮ್ಯ_devno(lc->device);
	ಹಿಂತಿರುಗಿ lc->devno;
}

ಪೂರ್ಣಾಂಕ loopcxt_is_ಕಳೆದುಹೋದ(ರಚನೆ loopdev_cxt *lc)
{
	ಆದರೆ (!lc || !loopcxt_has_device(lc))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (lc->is_ಕಳೆದುಹೋದ)
		ಹಿಂತಿರುಗಿ 1;

	lc->is_ಕಳೆದುಹೋದ = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ಹಿಂತಿರುಗಿ lc->is_ಕಳೆದುಹೋದ;
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
ಪೂರ್ಣಾಂಕ loopcxt_init(ರಚನೆ loopdev_cxt *lc, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ rc;
	ರಚನೆ stat st;
	ರಚನೆ loopdev_cxt dummy = UL_LOOPDEVCXT_ಖಾಲಿಗಳು;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	ಸ್ಮರಣೆ_ನಕಲು(lc, &dummy, ಗಾತ್ರ(dummy));
	lc->flags = flags;

	rc = loopcxt_ಗಣ_device(lc, ಖಾಲಿ);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	ಆದರೆ (stat(ಮಾರ್ಗ_SYS_ಖಂಡ, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ಆದರೆ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_ಆವೃತ್ತಿ() >= KERNEL_ಆವೃತ್ತಿ(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ಆದರೆ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(ಮಾರ್ಗ_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
ಶೂನ್ಯ loopcxt_deinit(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ errsv = errno;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	ಬಿಡುಗಡೆ(lc->filename);
	lc->filename = ಖಾಲಿ;

	ignore_ಫಲಿತಾಂಶ( loopcxt_ಗಣ_device(lc, ಖಾಲಿ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
ಅಕ್ಷರ *loopcxt_strdup_device(ರಚನೆ loopdev_cxt *lc)
{
	ಆದರೆ (!lc || !*lc->device)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *loopcxt_get_device(ರಚನೆ loopdev_cxt *lc)
{
	ಹಿಂತಿರುಗಿ lc && *lc->device ? lc->device : ಖಾಲಿ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ಸ್ಥಿರ ರಚನೆ ಮಾರ್ಗ_cxt *loopcxt_get_sysfs(ರಚನೆ loopdev_cxt *lc)
{
	ಆದರೆ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ಆದರೆ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}

		lc->sysfs = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
		ಆದರೆ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ಹಿಂತಿರುಗಿ lc->sysfs;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __loopcxt_get_fd(ರಚನೆ loopdev_cxt *lc, mode_t ಬಹುಲಕ)
{
	ಪೂರ್ಣಾಂಕ old = -1;

	ಆದರೆ (!lc || !*lc->device)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ಆದರೆ (lc->fd >= 0 && ಬಹುಲಕ == O_RDWR && lc->ಬಹುಲಕ == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ಆದರೆ (lc->fd < 0) {
		lc->ಬಹುಲಕ = ಬಹುಲಕ;
		lc->fd = open(lc->device, lc->ಬಹುಲಕ | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				ಬಹುಲಕ == O_RDONLY ? "ro" :
				ಬಹುಲಕ == O_RDWR ? "rw" : "??"));

		ಆದರೆ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ಆದರೆ (old >= 0)
		close(old);
	ಹಿಂತಿರುಗಿ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ಪೂರ್ಣಾಂಕ loopcxt_get_fd(ರಚನೆ loopdev_cxt *lc)
{
	ಹಿಂತಿರುಗಿ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_init_iterator(ರಚನೆ loopdev_cxt *lc, ಪೂರ್ಣಾಂಕ flags)
{
	ರಚನೆ loopdev_iter *iter;
	ರಚನೆ stat st;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, ಗಾತ್ರ(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ಆದರೆ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ಆದರೆ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(ಮಾರ್ಗ_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_deinit_iterator(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ loopdev_iter *iter;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	ಬಿಡುಗಡೆ(iter->minors);
	ಆದರೆ (iter->proc)
		fclose(iter->proc);
	ಆದರೆ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, ಗಾತ್ರ(*iter));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loopiter_ಗಣ_device(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ಪೂರ್ಣಾಂಕ rc = loopcxt_ಗಣ_device(lc, device);
	ಪೂರ್ಣಾಂಕ ಬಳಸಿದ;

	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	ಆದರೆ (!(lc->iter.flags & LOOPITER_FL_ಬಳಸಿದ) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ಹಿಂತಿರುಗಿ 0;	/* caller does not care about device status */

	ಬಳಸಿದ = loopcxt_get_ಸ್ಥಾನಾಂತರ(lc, ಖಾಲಿ) == 0;

	ಆದರೆ ((lc->iter.flags & LOOPITER_FL_ಬಳಸಿದ) && ಬಳಸಿದ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ ((lc->iter.flags & LOOPITER_FL_FREE) && !ಬಳಸಿದ)
		ಹಿಂತಿರುಗಿ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_ಫಲಿತಾಂಶ( loopcxt_ಗಣ_device(lc, ಖಾಲಿ) );
	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmpnum(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p1, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p2)
{
	ಹಿಂತಿರುಗಿ (((* (ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *) p1) > (* (ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *) p2)) -
			((* (ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *) p1) < (* (ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loop_scandir(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirname, ಪೂರ್ಣಾಂಕ **ary, ಪೂರ್ಣಾಂಕ hasprefix)
{
	DIR *dir;
	ರಚನೆ dirent *d;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n, ಎಣಿಕೆ_2 = 0, arylen = 0;

	ಆದರೆ (!dirname || !ary)
		ಹಿಂತಿರುಗಿ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ 0;
	ಬಿಡುಗಡೆ(*ary);
	*ary = ಖಾಲಿ;

	ಆಗ((d = readdir(dir))) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
		ಆದರೆ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (is_dotdir_dirent(d))
			ಮುಂದುವರಿ;

		ಆದರೆ (hasprefix) {
			/* /dev/loop<N> */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(d->d_name, "loop%u", &n) != 1)
				ಮುಂದುವರಿ;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* /dev/loop/<N> */
			ಅಕ್ಷರ *ಕೊನೆ = ಖಾಲಿ;

			errno = 0;
			n = strtol(d->d_name, &ಕೊನೆ, 10);
			ಆದರೆ (d->d_name == ಕೊನೆ || (ಕೊನೆ && *ಕೊನೆ) || errno)
				ಮುಂದುವರಿ;
		}
		ಆದರೆ (n < LOOPDEV_DEFAULT_NNODES)
			ಮುಂದುವರಿ;			/* ignore loop<0..7> */

		ಆದರೆ (ಎಣಿಕೆ_2 + 1 > arylen) {
			ಪೂರ್ಣಾಂಕ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
			ಆದರೆ (!tmp) {
				ಬಿಡುಗಡೆ(*ary);
				*ary = ಖಾಲಿ;
				closedir(dir);
				ಹಿಂತಿರುಗಿ -1;
			}
			*ary = tmp;
		}
		ಆದರೆ (*ary)
			(*ary)[ಎಣಿಕೆ_2++] = n;
	}
	ಆದರೆ (ಎಣಿಕೆ_2 && *ary)
		qsort(*ary, ಎಣಿಕೆ_2, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ), cmpnum);

	closedir(dir);
	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loopcxt_ಮುಂದಿನ_from_proc(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ loopdev_iter *iter = &lc->iter;
	ಅಕ್ಷರ buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ಆದರೆ (!iter->proc)
		iter->proc = fopen(ಮಾರ್ಗ_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ಆದರೆ (!iter->proc)
		ಹಿಂತಿರುಗಿ 1;

	ಆಗ (fgets(buf, ಗಾತ್ರ(buf), iter->proc)) {
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ m;
		ಅಕ್ಷರ name[128 + 1];


		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ಮುಂದುವರಿ;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ಆದರೆ (loopiter_ಗಣ_device(lc, name) == 0)
			ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loopcxt_ಮುಂದಿನ_from_sysfs(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ loopdev_iter *iter = &lc->iter;
	ರಚನೆ dirent *d;
	ಪೂರ್ಣಾಂಕ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ಆದರೆ (!iter->sysblock)
		iter->sysblock = opendir(ಮಾರ್ಗ_SYS_ಖಂಡ);

	ಆದರೆ (!iter->sysblock)
		ಹಿಂತಿರುಗಿ 1;

	fd = dirfd(iter->sysblock);

	ಆಗ ((d = readdir(iter->sysblock))) {
		ಅಕ್ಷರ name[NAME_MAX + 18 + 1];
		ರಚನೆ stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ಆದರೆ (is_dotdir_dirent(d) || ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(d->d_name, "loop", 4) != 0)
			ಮುಂದುವರಿ;

		snprintf(name, ಗಾತ್ರ(name), "%s/loop/backing_file", d->d_name);
		ಆದರೆ (fstatat(fd, name, &st, 0) != 0)
			ಮುಂದುವರಿ;

		ಆದರೆ (loopiter_ಗಣ_device(lc, d->d_name) == 0)
			ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಮುಂದಿನ(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ loopdev_iter *iter;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;


	iter = &lc->iter;
	ಆದರೆ (iter->ಮುಗಿದ)
		ಹಿಂತಿರುಗಿ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ಆದರೆ (iter->flags & LOOPITER_FL_ಬಳಸಿದ) {
		ಪೂರ್ಣಾಂಕ rc;

		ಆದರೆ (loopcxt_sysfs_ಲಭ್ಯ(lc))
			rc = loopcxt_ಮುಂದಿನ_from_sysfs(lc);
		ಇಲ್ಲದಿದ್ದರೆ
			rc = loopcxt_ಮುಂದಿನ_from_proc(lc);
		ಆದರೆ (rc == 0)
			ಹಿಂತಿರುಗಿ 0;
		ಹೋಗು ಮುಗಿದ;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ಆದರೆ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		ಗಾಗಿ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			ಅಕ್ಷರ name[16];
			snprintf(name, ಗಾತ್ರ(name), "loop%d", iter->ncur);

			ಆದರೆ (loopiter_ಗಣ_device(lc, name) == 0)
				ಹಿಂತಿರುಗಿ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ಆದರೆ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(ಮಾರ್ಗ_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	ಗಾಗಿ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		ಅಕ್ಷರ name[16];
		snprintf(name, ಗಾತ್ರ(name), "loop%d", iter->minors[iter->ncur]);

		ಆದರೆ (loopiter_ಗಣ_device(lc, name) == 0)
			ಹಿಂತಿರುಗಿ 0;
	}
ಮುಗಿದ:
	loopcxt_deinit_iterator(lc);
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * @device: path to device
 */
ಪೂರ್ಣಾಂಕ is_loopdev(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ರಚನೆ stat st;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		ಅಕ್ಷರ name[PATH_MAX], *cn, *p = ಖಾಲಿ;

		snprintf(name, ಗಾತ್ರ(name), ಮಾರ್ಗ_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_ಮಾರ್ಗ(name);
		ಆದರೆ (cn)
			p = stripoff_ಕೊನೆಯ_component(cn);
		rc = p && ul_startswith(p, "loop");
		ಬಿಡುಗಡೆ(cn);
	}

	ಆದರೆ (rc == 0)
		errno = ENODEV;
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ರಚನೆ loop_info64 *loopcxt_get_info(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ fd;

	ಆದರೆ (!lc || lc->info_failed) {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	errno = 0;
	ಆದರೆ (lc->has_info)
		ಹಿಂತಿರುಗಿ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ಹಿಂತಿರುಗಿ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
ಅಕ್ಷರ *loopcxt_get_backing_file(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_ಮಾರ್ಗ_read_string(sysfs, &res, "loop/backing_file");

	ಆದರೆ (!res && loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);

		ಆದರೆ (lo) {
			lo->lo_file_name[LO_NAME_ಗಾತ್ರ - 2] = '*';
			lo->lo_file_name[LO_NAME_ಗಾತ್ರ - 1] = '\0';
			res = strdup((ಅಕ್ಷರ *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ಹಿಂತಿರುಗಿ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
ಅಕ್ಷರ *loopcxt_get_refname(ರಚನೆ loopdev_cxt *lc)
{
	ಅಕ್ಷರ *res = ಖಾಲಿ;
	ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);

	ಆದರೆ (lo) {
		lo->lo_file_name[LO_NAME_ಗಾತ್ರ - 1] = '\0';
		res = strdup((ಅಕ್ಷರ *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ಹಿಂತಿರುಗಿ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_ಸ್ಥಾನಾಂತರ(ರಚನೆ loopdev_cxt *lc, uint64_t *ಸ್ಥಾನಾಂತರ)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;

	ಆದರೆ (sysfs)
		ಆದರೆ (ul_ಮಾರ್ಗ_read_u64(sysfs, ಸ್ಥಾನಾಂತರ, "loop/offset") == 0)
			rc = 0;

	ಆದರೆ (rc && loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
		ಆದರೆ (lo) {
			ಆದರೆ (ಸ್ಥಾನಾಂತರ)
				*ಸ್ಥಾನಾಂತರ = lo->lo_ಸ್ಥಾನಾಂತರ;
			rc = 0;
		} ಇಲ್ಲದಿದ್ದರೆ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_blocksize(ರಚನೆ loopdev_cxt *lc, uint64_t *blocksize)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;

	ಆದರೆ (sysfs)
		ಆದರೆ (ul_ಮಾರ್ಗ_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ಆದರೆ (rc) {
		ಪೂರ್ಣಾಂಕ fd = loopcxt_get_fd(lc);
		ಪೂರ್ಣಾಂಕ sz = 0;

		ಆದರೆ (fd < 0)
			ಹಿಂತಿರುಗಿ -EINVAL;
		rc = blkdev_get_sector_ಗಾತ್ರ(fd, &sz);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_sizelimit(ರಚನೆ loopdev_cxt *lc, uint64_t *ಗಾತ್ರ_2)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;

	ಆದರೆ (sysfs)
		ಆದರೆ (ul_ಮಾರ್ಗ_read_u64(sysfs, ಗಾತ್ರ_2, "loop/sizelimit") == 0)
			rc = 0;

	ಆದರೆ (rc && loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
		ಆದರೆ (lo) {
			ಆದರೆ (ಗಾತ್ರ_2)
				*ಗಾತ್ರ_2 = lo->lo_sizelimit;
			rc = 0;
		} ಇಲ್ಲದಿದ್ದರೆ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_encrypt_type(ರಚನೆ loopdev_cxt *lc, uint32_t *type)
{
	ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಂಕ rc;

	/* not provided by sysfs */
	ಆದರೆ (lo) {
		ಆದರೆ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ಇಲ್ಲದಿದ್ದರೆ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *loopcxt_get_crypt_name(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);

	ಆದರೆ (lo)
		ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_backing_devno(ರಚನೆ loopdev_cxt *lc, dev_t *devno)
{
	ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (lo) {
		ಆದರೆ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ಇಲ್ಲದಿದ್ದರೆ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ಪೂರ್ಣಾಂಕ loopcxt_get_backing_inode(ರಚನೆ loopdev_cxt *lc, ino_t *ino)
{
	ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (lo) {
		ಆದರೆ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ಇಲ್ಲದಿದ್ದರೆ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ loopmod_supports_partscan(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ rc, ret = 0;
	FILE *f;

	ಆದರೆ (get_linux_ಆವೃತ್ತಿ() >= KERNEL_ಆವೃತ್ತಿ(3,2,0))
		ಹಿಂತಿರುಗಿ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ಹಿಂತಿರುಗಿ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ಪೂರ್ಣಾಂಕ loopcxt_is_partscan(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಆದರೆ (sysfs) {
		/* kernel >= 3.2 */
		ಪೂರ್ಣಾಂಕ fl;
		ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ಹಿಂತಿರುಗಿ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ಹಿಂತಿರುಗಿ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ಪೂರ್ಣಾಂಕ loopcxt_is_autoclear(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಆದರೆ (sysfs) {
		ಪೂರ್ಣಾಂಕ fl;
		ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ಹಿಂತಿರುಗಿ fl;
	}

	ಆದರೆ (loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
		ಆದರೆ (lo)
			ಹಿಂತಿರುಗಿ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ಪೂರ್ಣಾಂಕ loopcxt_is_readonly(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಆದರೆ (sysfs) {
		ಪೂರ್ಣಾಂಕ fl;
		ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(sysfs, &fl, "ro") == 0)
			ಹಿಂತಿರುಗಿ fl;
	}

	ಆದರೆ (loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
		ಆದರೆ (lo)
			ಹಿಂತಿರುಗಿ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ಪೂರ್ಣಾಂಕ loopcxt_is_dio(ರಚನೆ loopdev_cxt *lc)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *sysfs = loopcxt_get_sysfs(lc);

	ಆದರೆ (sysfs) {
		ಪೂರ್ಣಾಂಕ fl;
		ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(sysfs, &fl, "loop/dio") == 0)
			ಹಿಂತಿರುಗಿ fl;
	}
	ಆದರೆ (loopcxt_ioctl_enabled(lc)) {
		ರಚನೆ loop_info64 *lo = loopcxt_get_info(lc);
		ಆದರೆ (lo)
			ಹಿಂತಿರುಗಿ lo->lo_flags & LO_FLAGS_ನೇರ_IO;
	}
	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ loopcxt_is_ಬಳಸಿದ(ರಚನೆ loopdev_cxt *lc,
		    ರಚನೆ stat *st,
		    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *backing_file,
		    uint64_t ಸ್ಥಾನಾಂತರ,
		    uint64_t sizelimit,
		    ಪೂರ್ಣಾಂಕ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ಆದರೆ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ಆದರೆ (ino == st->st_ino && dev == st->st_dev)
			ಹೋಗು ಸಿಕ್ಕಿತು;

		/* don't use filename if we have devno and inode */
		ಹಿಂತಿರುಗಿ 0;
	}

	/* poor man's solution */
	ಆದರೆ (backing_file) {
		ಅಕ್ಷರ *name = loopcxt_get_backing_file(lc);
		ಪೂರ್ಣಾಂಕ rc = name && strcmp(name, backing_file) == 0;

		ಬಿಡುಗಡೆ(name);
		ಆದರೆ (rc)
			ಹೋಗು ಸಿಕ್ಕಿತು;
	}

	ಹಿಂತಿರುಗಿ 0;
ಸಿಕ್ಕಿತು:
	ಆದರೆ (flags & LOOPDEV_FL_ಸ್ಥಾನಾಂತರ) {
		uint64_t off = 0;

		ಪೂರ್ಣಾಂಕ rc = loopcxt_get_ಸ್ಥಾನಾಂತರ(lc, &off) == 0 && off == ಸ್ಥಾನಾಂತರ;

		ಆದರೆ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ಹಿಂತಿರುಗಿ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ಹಿಂತಿರುಗಿ rc;
	}
	ಹಿಂತಿರುಗಿ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_ಸ್ಥಾನಾಂತರ(ರಚನೆ loopdev_cxt *lc, uint64_t ಸ್ಥಾನಾಂತರ)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;
	lc->config.info.lo_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, ಸ್ಥಾನಾಂತರ));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_sizelimit(ರಚನೆ loopdev_cxt *lc, uint64_t sizelimit)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_blocksize(ರಚನೆ loopdev_cxt *lc, uint64_t blocksize)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_flags(ರಚನೆ loopdev_cxt *lc, uint32_t flags)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (ಚಿಹ್ನೆರಹಿತ) flags));
	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_refname(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *refname)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, ಗಾತ್ರ(lc->config.info.lo_file_name));
	ಆದರೆ (refname)
		xstrncpy((ಅಕ್ಷರ *)lc->config.info.lo_file_name, refname, LO_NAME_ಗಾತ್ರ);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (ಅಕ್ಷರ *)lc->config.info.lo_file_name));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಗಣ_backing_file(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename)
{
	ಆದರೆ (!lc)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ಇಲ್ಲದಿದ್ದರೆ
		lc->filename = ul_canonicalize_ಮಾರ್ಗ(filename);
	ಆದರೆ (!lc->filename)
		ಹಿಂತಿರುಗಿ -errno;

	ಆದರೆ (!lc->config.info.lo_file_name[0])
		loopcxt_ಗಣ_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loopcxt_check_ಗಾತ್ರ(ರಚನೆ loopdev_cxt *lc, ಪೂರ್ಣಾಂಕ file_fd)
{
	uint64_t ಗಾತ್ರ_2, expected_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ dev_fd;
	ರಚನೆ stat st;

	ಆದರೆ (!lc->config.info.lo_ಸ್ಥಾನಾಂತರ && !lc->config.info.lo_sizelimit)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ಹಿಂತಿರುಗಿ -errno;
	}
	ಆದರೆ (S_ISBLK(st.st_mode)) {
		ಆದರೆ (blkdev_get_ಗಾತ್ರ(file_fd, &expected_ಗಾತ್ರ)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ಹಿಂತಿರುಗಿ -errno;
		}
	} ಇಲ್ಲದಿದ್ದರೆ
		expected_ಗಾತ್ರ = st.st_size;

	ಆದರೆ (expected_ಗಾತ್ರ == 0 || expected_ಗಾತ್ರ <= lc->config.info.lo_ಸ್ಥಾನಾಂತರ) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ಹಿಂತಿರುಗಿ 0;	/* ignore this error */
	}

	ಆದರೆ (lc->config.info.lo_ಸ್ಥಾನಾಂತರ > 0)
		expected_ಗಾತ್ರ -= lc->config.info.lo_ಸ್ಥಾನಾಂತರ;

	ಆದರೆ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_ಗಾತ್ರ)
		expected_ಗಾತ್ರ = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ಆದರೆ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ಹಿಂತಿರುಗಿ -errno;
	}

	ಆದರೆ (blkdev_get_ಗಾತ್ರ(dev_fd, &ಗಾತ್ರ_2)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ಹಿಂತಿರುಗಿ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ಆದರೆ (expected_ಗಾತ್ರ % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_ಗಾತ್ರ = (expected_ಗಾತ್ರ >> 9) << 9;
	}

	ಆದರೆ (expected_ಗಾತ್ರ != ಗಾತ್ರ_2) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      ಗಾತ್ರ_2, expected_ಗಾತ್ರ));

		ಆದರೆ (loopcxt_ioctl_ಸಾಮರ್ಥ್ಯ(lc)) {
			/* ioctl not available */
			ಆದರೆ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ಹಿಂತಿರುಗಿ -errno;
		}

		ಆದರೆ (blkdev_get_ಗಾತ್ರ(dev_fd, &ಗಾತ್ರ_2))
			ಹಿಂತಿರುಗಿ -errno;

		ಆದರೆ (expected_ಗಾತ್ರ != ಗಾತ್ರ_2) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					ಗಾತ್ರ_2, expected_ಗಾತ್ರ));
			ಹಿಂತಿರುಗಿ -errno;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ loopcxt_ಸಿದ್ಧತೆ_device(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, ಬಹುಲಕ = O_RDWR;
	ಪೂರ್ಣಾಂಕ rc = -1, cnt = 0;
	ಪೂರ್ಣಾಂಕ errsv = 0;
	ಪೂರ್ಣಾಂಕ fallback = 0;

	ಆದರೆ (!lc || !*lc->device || !lc->filename)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ಆದರೆ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		ಬಹುಲಕ = O_RDONLY;

	ಆದರೆ (lc->config.info.lo_flags & LO_FLAGS_ನೇರ_IO)
		flags |= O_ನೇರ;
	ಆದರೆ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, ಬಹುಲಕ | flags, 0);
	ಇಲ್ಲದಿದ್ದರೆ
		file_fd = open(lc->filename, ಬಹುಲಕ | flags);

	ಆದರೆ (file_fd < 0 && ಬಹುಲಕ != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		ಬಹುಲಕ = O_RDONLY;
		ಆದರೆ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, ಬಹುಲಕ | flags, 0);
		ಇಲ್ಲದಿದ್ದರೆ
			file_fd = open(lc->filename, ಬಹುಲಕ | flags);
	}
	ಆದರೆ (file_fd < 0) {
		DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("open backing file failed: %m"));
		ಹಿಂತಿರುಗಿ -errno;
	}
	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("backing file open: OK"));

	ಆದರೆ (ಬಹುಲಕ == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ಇಲ್ಲದಿದ್ದರೆ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	ಮಾಡು {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, ಬಹುಲಕ);
		ಆದರೆ (dev_fd >= 0 || lc->control_ok == 0)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (errno != EACCES && errno != ENOENT)
			ನಿಲ್ಲಿಸು;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ಆಗ (cnt++ < 16);

	ಆದರೆ (dev_fd < 0) {
		rc = -errno;
		ಹೋಗು err;
	}

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ಆದರೆ (lc->blocksize > 0)
		lc->config.ಖಂಡ_ಗಾತ್ರ = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ಆದರೆ (rc != 0) {
		errsv = errno;
		ಆದರೆ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			ಹೋಗು err;
		}
		fallback = 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ಆದರೆ (fallback) {
		ಆದರೆ (ioctl(dev_fd, LOOP_ಗಣ_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_SET_FD failed: %m"));
			ಹೋಗು err;
		}

		DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_SET_FD: OK"));

		ಆದರೆ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			ಹೋಗು err;
		}

		ಆದರೆ ((rc = loopcxt_ioctl_ಸ್ಥಿತಿ(lc)) < 0) {
			errsv = -rc;
			ಹೋಗು err;
		}
	}

	ಆದರೆ ((rc = loopcxt_check_ಗಾತ್ರ(lc, file_fd)))
		ಹೋಗು err;

	close(file_fd);

	memset(&lc->config, 0, ಗಾತ್ರ(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("success [rc=0]"));
	ಹಿಂತಿರುಗಿ 0;
err:
	ಆದರೆ (file_fd >= 0)
		close(file_fd);
	ಆದರೆ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ಆದರೆ (errsv)
		errno = errsv;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("failed [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ loopcxt_ioctl_ಸ್ಥಿತಿ(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ಆದರೆ (dev_fd < 0)
		ಹಿಂತಿರುಗಿ -errno;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_ಗಣ_STATUS64, &lc->config.info) );
	ಆದರೆ (rc != 0) {
		DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ಹಿಂತಿರುಗಿ rc;
	}

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ loopcxt_ioctl_ಸಾಮರ್ಥ್ಯ(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ rc, fd = loopcxt_get_fd(lc);

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_ಗಣ_ಸಾಮರ್ಥ್ಯ, 0) );
	ಆದರೆ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ಹಿಂತಿರುಗಿ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ loopcxt_ioctl_dio(ರಚನೆ loopdev_cxt *lc, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ use_dio)
{
	ಪೂರ್ಣಾಂಕ rc, fd = loopcxt_get_fd(lc);

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_ಗಣ_ನೇರ_IO, use_dio) );
	ಆದರೆ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ಹಿಂತಿರುಗಿ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ಪೂರ್ಣಾಂಕ loopcxt_ioctl_blocksize(ರಚನೆ loopdev_cxt *lc, uint64_t blocksize)
{
	ಪೂರ್ಣಾಂಕ rc, fd = loopcxt_get_fd(lc);

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_ಗಣ_ಖಂಡ_ಗಾತ್ರ, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) blocksize) );
	ಆದರೆ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ಹಿಂತಿರುಗಿ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ loopcxt_get_device_nr(ರಚನೆ loopdev_cxt *lc, ಪೂರ್ಣಾಂಕ *nr)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, *dev;
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;

	errno = 0;
	ಆದರೆ (!lc || !nr)
		ಹಿಂತಿರುಗಿ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ಆದರೆ (!dev)
		ಹೋಗು ಮುಗಿದ;

	p = strrchr(dev, '/');
	ಆದರೆ (!p)
		ಹೋಗು ಮುಗಿದ;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(p, "/loop%d", nr) != 1 && ಅಕ್ಷರಸರಣಿ_ಓದು(p, "/%d", nr) != 1)
		ಹೋಗು ಮುಗಿದ;

	ಆದರೆ (*nr < 0)
		ಹೋಗು ಮುಗಿದ;
	rc = 0;
ಮುಗಿದ:
	ಆದರೆ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ loopcxt_detach_device(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ rc, fd = loopcxt_get_fd(lc);

	ಆದರೆ (fd < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(ಸಿದ್ಧತೆ, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ಆದರೆ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ಹಿಂತಿರುಗಿ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ loopcxt_remove_device(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;
	ಪೂರ್ಣಾಂಕ ctl, nr = -1;

	ಆದರೆ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ಹೋಗು ಮುಗಿದ;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ಆದರೆ (rc)
		ಹೋಗು ಮುಗಿದ;

	ctl = open(ಮಾರ್ಗ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ಆದರೆ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
ಮುಗಿದ:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ loopcxt_ಕೂಡಿಸು_device(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;
	ಪೂರ್ಣಾಂಕ ctl, nr = -1;

	ಆದರೆ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ಹೋಗು ಮುಗಿದ;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ಆದರೆ (rc)
		ಹೋಗು ಮುಗಿದ;

	ctl = open(ಮಾರ್ಗ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ಆದರೆ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ಕೂಡಿಸು, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
ಮುಗಿದ:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಕಂಡುಹಿಡಿ_ಬಳಸದ(ರಚನೆ loopdev_cxt *lc)
{
	ಪೂರ್ಣಾಂಕ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ಆದರೆ (lc->flags & LOOPDEV_FL_CONTROL) {
		ಪೂರ್ಣಾಂಕ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(ಮಾರ್ಗ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ಆದರೆ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ಇಲ್ಲದಿದ್ದರೆ
			rc = -errno;
		ಆದರೆ (rc >= 0) {
			ಅಕ್ಷರ name[16];
			snprintf(name, ಗಾತ್ರ(name), "loop%d", rc);

			rc = loopiter_ಗಣ_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ಆದರೆ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ಆದರೆ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;

		rc = loopcxt_ಮುಂದಿನ(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ -ENOENT;
	}
	ಹಿಂತಿರುಗಿ rc;
}



/*
 * Return: TRUE/FALSE
 */
ಪೂರ್ಣಾಂಕ loopdev_is_autoclear(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ರಚನೆ loopdev_cxt lc;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!device)
		ಹಿಂತಿರುಗಿ 0;

	rc = loopcxt_init(&lc, 0);
	ಆದರೆ (!rc)
		rc = loopcxt_ಗಣ_device(&lc, device);
	ಆದರೆ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ಹಿಂತಿರುಗಿ rc;
}

ಅಕ್ಷರ *loopdev_get_backing_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ರಚನೆ loopdev_cxt lc;
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (!device)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (loopcxt_init(&lc, 0))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (loopcxt_ಗಣ_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ಹಿಂತಿರುಗಿ res;
}

/*
 * Returns: TRUE/FALSE
 */
ಪೂರ್ಣಾಂಕ loopdev_has_backing_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ಅಕ್ಷರ *tmp = loopdev_get_backing_file(device);

	ಆದರೆ (tmp) {
		ಬಿಡುಗಡೆ(tmp);
		ಹಿಂತಿರುಗಿ 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Returns: TRUE/FALSE
 */
ಪೂರ್ಣಾಂಕ loopdev_is_ಬಳಸಿದ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
		    uint64_t ಸ್ಥಾನಾಂತರ, uint64_t sizelimit, ಪೂರ್ಣಾಂಕ flags)
{
	ರಚನೆ loopdev_cxt lc;
	ರಚನೆ stat st;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (!device || !filename)
		ಹಿಂತಿರುಗಿ 0;

	rc = loopcxt_init(&lc, 0);
	ಆದರೆ (!rc)
		rc = loopcxt_ಗಣ_device(&lc, device);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_ಬಳಸಿದ(&lc, rc ? &st : ಖಾಲಿ, filename, ಸ್ಥಾನಾಂತರ, sizelimit, flags);

	loopcxt_deinit(&lc);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ಪೂರ್ಣಾಂಕ loopdev_detach(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *device)
{
	ರಚನೆ loopdev_cxt lc;
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!device)
		ಹಿಂತಿರುಗಿ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ಆದರೆ (!rc)
		rc = loopcxt_ಗಣ_device(&lc, device);
	ಆದರೆ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಕಂಡುಹಿಡಿ_by_backing_file(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
				 uint64_t ಸ್ಥಾನಾಂತರ, uint64_t sizelimit, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ rc, hasst;
	ರಚನೆ stat st;

	ಆದರೆ (!filename)
		ಹಿಂತಿರುಗಿ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ಬಳಸಿದ);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	ಆಗ ((rc = loopcxt_ಮುಂದಿನ(lc)) == 0) {

		ಆದರೆ (loopcxt_is_ಬಳಸಿದ(lc, hasst ? &st : ಖಾಲಿ,
				    filename, ಸ್ಥಾನಾಂತರ, sizelimit, flags))
			ನಿಲ್ಲಿಸು;
	}

	loopcxt_deinit_iterator(lc);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ಪೂರ್ಣಾಂಕ loopcxt_ಕಂಡುಹಿಡಿ_overlap(ರಚನೆ loopdev_cxt *lc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename,
			   uint64_t ಸ್ಥಾನಾಂತರ, uint64_t sizelimit)
{
	ಪೂರ್ಣಾಂಕ rc, hasst;
	ರಚನೆ stat st;

	ಆದರೆ (!filename)
		ಹಿಂತಿರುಗಿ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ಬಳಸಿದ);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	ಆಗ ((rc = loopcxt_ಮುಂದಿನ(lc)) == 0) {
		uint64_t lc_sizelimit, lc_ಸ್ಥಾನಾಂತರ;

		rc = loopcxt_is_ಬಳಸಿದ(lc, hasst ? &st : ಖಾಲಿ,
				     filename, ಸ್ಥಾನಾಂತರ, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ಆದರೆ (rc <= 0)
			ಮುಂದುವರಿ;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_ಸ್ಥಾನಾಂತರ(lc, &lc_ಸ್ಥಾನಾಂತರ);
		ಆದರೆ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ಮುಂದುವರಿ;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ಆದರೆ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ಮುಂದುವರಿ;
		}

		/* full match */
		ಆದರೆ (lc_sizelimit == sizelimit && lc_ಸ್ಥಾನಾಂತರ == ಸ್ಥಾನಾಂತರ) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			ಹೋಗು ಸಿಕ್ಕಿತು;
		}

		/* overlap */
		ಆದರೆ (lc_sizelimit != 0 && ಸ್ಥಾನಾಂತರ >= lc_ಸ್ಥಾನಾಂತರ + lc_sizelimit)
			ಮುಂದುವರಿ;
		ಆದರೆ (sizelimit != 0 && ಸ್ಥಾನಾಂತರ + sizelimit <= lc_ಸ್ಥಾನಾಂತರ)
			ಮುಂದುವರಿ;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			ಹೋಗು ಸಿಕ್ಕಿತು;
	}

	ಆದರೆ (rc == 1)
		rc = 0;	/* not found */
ಸಿಕ್ಕಿತು:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns allocated string with device name
 */
ಅಕ್ಷರ *loopdev_ಕಂಡುಹಿಡಿ_by_backing_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, uint64_t ಸ್ಥಾನಾಂತರ, uint64_t sizelimit, ಪೂರ್ಣಾಂಕ flags)
{
	ರಚನೆ loopdev_cxt lc;
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (!filename)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (loopcxt_init(&lc, 0))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (loopcxt_ಕಂಡುಹಿಡಿ_by_backing_file(&lc, filename, ಸ್ಥಾನಾಂತರ, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ಹಿಂತಿರುಗಿ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ಪೂರ್ಣಾಂಕ loopdev_ಎಣಿಕೆ_by_backing_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, ಅಕ್ಷರ **loopdev)
{
	ರಚನೆ loopdev_cxt lc;
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0, rc;

	ಆದರೆ (!filename)
		ಹಿಂತಿರುಗಿ -1;

	rc = loopcxt_init(&lc, 0);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;
	ಆದರೆ (loopcxt_init_iterator(&lc, LOOPITER_FL_ಬಳಸಿದ))
		ಹಿಂತಿರುಗಿ -1;

	ಆಗ(loopcxt_ಮುಂದಿನ(&lc) == 0) {
		ಅಕ್ಷರ *backing = loopcxt_get_backing_file(&lc);

		ಆದರೆ (!backing || strcmp(backing, filename) != 0) {
			ಬಿಡುಗಡೆ(backing);
			ಮುಂದುವರಿ;
		}

		ಬಿಡುಗಡೆ(backing);
		ಆದರೆ (loopdev && ಎಣಿಕೆ_2 == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		ಎಣಿಕೆ_2++;
	}

	loopcxt_deinit(&lc);

	ಆದರೆ (loopdev && ಎಣಿಕೆ_2 > 1) {
		ಬಿಡುಗಡೆ(*loopdev);
		*loopdev = ಖಾಲಿ;
	}
	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_LOOPDEV
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಆದರೆ (argc < 2)
		ಹೋಗು usage;

	ಆದರೆ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		ಬರೆ("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ಇಲ್ಲದಿದ್ದರೆ
		ಹೋಗು usage;

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ

