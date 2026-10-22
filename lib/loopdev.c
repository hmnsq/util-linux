
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
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/ioctl.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/mman.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <dirent.h>

#অন্তর্ভাৱ "linux_version.h"
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "timeutils.h"
#অন্তর্ভাৱ "sysfs.h"
#অন্তর্ভাৱ "pathnames.h"
#অন্তর্ভাৱ "loopdev.h"
#অন্তর্ভাৱ "canonicalize.h"
#অন্তর্ভাৱ "blkdev.h"
#অন্তর্ভাৱ "debug.h"
#অন্তর্ভাৱ "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

স্থির শূন্য loopdev_init_debug(শূন্য)
{
	যদি (loopdev_debug_mask)
		ঘূৰাই;
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
		পূর্ণাঙ্ক _c = 0, _e;					\
		কৰা {						\
			errno = 0;				\
			_e = x;					\
			যদি (_e == 0 || errno != EAGAIN)		\
				ভঙ্গ;				\
			যদি (_c >= LOOPDEV_MAX_TRIES)		\
				ভঙ্গ;				\
			xusleep(250000);			\
			_c++;					\
		} যেতিয়ালৈ (1);					\
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
পূর্ণাঙ্ক loopcxt_set_device(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *device)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;

	যদি (lc->fd >= 0) {
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
	memset(&lc->config, 0, পরিমাণ(lc->config));

	/* set new */
	যদি (device) {
		যদি (*device != '/') {
			ধ্রুৱ ৱর্ণ *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			যদি (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				যদি (strlen(device) < 5)
					ঘূৰাই -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, পরিমাণ(lc->device), "%s%s",
				dir, device);
		} অন্যথা
			xstrncpy(lc->device, device, পরিমাণ(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = রিক্ত;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক loopcxt_has_device(সংরচনা loopdev_cxt *lc)
{
	ঘূৰাই lc && *lc->device;
}

dev_t loopcxt_get_devno(সংরচনা loopdev_cxt *lc)
{
	যদি (!lc || !loopcxt_has_device(lc))
		ঘূৰাই 0;
	যদি (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ঘূৰাই lc->devno;
}

পূর্ণাঙ্ক loopcxt_is_lost(সংরচনা loopdev_cxt *lc)
{
	যদি (!lc || !loopcxt_has_device(lc))
		ঘূৰাই 0;
	যদি (lc->is_lost)
		ঘূৰাই 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ঘূৰাই lc->is_lost;
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
পূর্ণাঙ্ক loopcxt_init(সংরচনা loopdev_cxt *lc, পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক rc;
	সংরচনা stat st;
	সংরচনা loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	যদি (!lc)
		ঘূৰাই -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, পরিমাণ(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, রিক্ত);
	যদি (rc)
		ঘূৰাই rc;

	যদি (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	যদি (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	যদি (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ঘূৰাই 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
শূন্য loopcxt_deinit(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক errsv = errno;

	যদি (!lc)
		ঘূৰাই;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	মুক্ত(lc->filename);
	lc->filename = রিক্ত;

	ignore_result( loopcxt_set_device(lc, রিক্ত) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
ৱর্ণ *loopcxt_strdup_device(সংরচনা loopdev_cxt *lc)
{
	যদি (!lc || !*lc->device)
		ঘূৰাই রিক্ত;
	ঘূৰাই strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ধ্রুৱ ৱর্ণ *loopcxt_get_device(সংরচনা loopdev_cxt *lc)
{
	ঘূৰাই lc && *lc->device ? lc->device : রিক্ত;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
স্থির সংরচনা path_cxt *loopcxt_get_sysfs(সংরচনা loopdev_cxt *lc)
{
	যদি (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ঘূৰাই রিক্ত;

	যদি (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		যদি (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ঘূৰাই রিক্ত;
		}

		lc->sysfs = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
		যদি (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ঘূৰাই lc->sysfs;
}

স্থির পূর্ণাঙ্ক __loopcxt_get_fd(সংরচনা loopdev_cxt *lc, mode_t mode)
{
	পূর্ণাঙ্ক old = -1;

	যদি (!lc || !*lc->device)
		ঘূৰাই -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	যদি (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	যদি (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		যদি (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	যদি (old >= 0)
		close(old);
	ঘূৰাই lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
পূর্ণাঙ্ক loopcxt_get_fd(সংরচনা loopdev_cxt *lc)
{
	ঘূৰাই __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_init_iterator(সংরচনা loopdev_cxt *lc, পূর্ণাঙ্ক flags)
{
	সংরচনা loopdev_iter *iter;
	সংরচনা stat st;

	যদি (!lc)
		ঘূৰাই -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, পরিমাণ(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	যদি (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		যদি (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ঘূৰাই 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_deinit_iterator(সংরচনা loopdev_cxt *lc)
{
	সংরচনা loopdev_iter *iter;

	যদি (!lc)
		ঘূৰাই -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	মুক্ত(iter->minors);
	যদি (iter->proc)
		fclose(iter->proc);
	যদি (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, পরিমাণ(*iter));
	ঘূৰাই 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
স্থির পূর্ণাঙ্ক loopiter_set_device(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *device)
{
	পূর্ণাঙ্ক rc = loopcxt_set_device(lc, device);
	পূর্ণাঙ্ক used;

	যদি (rc)
		ঘূৰাই rc;

	যদি (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ঘূৰাই 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, রিক্ত) == 0;

	যদি ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ঘূৰাই 0;

	যদি ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ঘূৰাই 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, রিক্ত) );
	ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক cmpnum(ধ্রুৱ শূন্য *p1, ধ্রুৱ শূন্য *p2)
{
	ঘূৰাই (((* (ধ্রুৱ পূর্ণাঙ্ক *) p1) > (* (ধ্রুৱ পূর্ণাঙ্ক *) p2)) -
			((* (ধ্রুৱ পূর্ণাঙ্ক *) p1) < (* (ধ্রুৱ পূর্ণাঙ্ক *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
স্থির পূর্ণাঙ্ক loop_scandir(ধ্রুৱ ৱর্ণ *dirname, পূর্ণাঙ্ক **ary, পূর্ণাঙ্ক hasprefix)
{
	DIR *dir;
	সংরচনা dirent *d;
	unsigned পূর্ণাঙ্ক n, গুনতি = 0, arylen = 0;

	যদি (!dirname || !ary)
		ঘূৰাই 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	যদি (!dir)
		ঘূৰাই 0;
	মুক্ত(*ary);
	*ary = রিক্ত;

	যেতিয়ালৈ((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			অনুৱর্ত;
#endif
		যদি (is_dotdir_dirent(d))
			অনুৱর্ত;

		যদি (hasprefix) {
			/* /dev/loop<N> */
			যদি (sscanf(d->d_name, "loop%u", &n) != 1)
				অনুৱর্ত;
		} অন্যথা {
			/* /dev/loop/<N> */
			ৱর্ণ *end = রিক্ত;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			যদি (d->d_name == end || (end && *end) || errno)
				অনুৱর্ত;
		}
		যদি (n < LOOPDEV_DEFAULT_NNODES)
			অনুৱর্ত;			/* ignore loop<0..7> */

		যদি (গুনতি + 1 > arylen) {
			পূর্ণাঙ্ক *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, পরিমাণ(পূর্ণাঙ্ক));
			যদি (!tmp) {
				মুক্ত(*ary);
				*ary = রিক্ত;
				closedir(dir);
				ঘূৰাই -1;
			}
			*ary = tmp;
		}
		যদি (*ary)
			(*ary)[গুনতি++] = n;
	}
	যদি (গুনতি && *ary)
		qsort(*ary, গুনতি, পরিমাণ(পূর্ণাঙ্ক), cmpnum);

	closedir(dir);
	ঘূৰাই গুনতি;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
স্থির পূর্ণাঙ্ক loopcxt_next_from_proc(সংরচনা loopdev_cxt *lc)
{
	সংরচনা loopdev_iter *iter = &lc->iter;
	ৱর্ণ buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	যদি (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	যদি (!iter->proc)
		ঘূৰাই 1;

	যেতিয়ালৈ (fgets(buf, পরিমাণ(buf), iter->proc)) {
		unsigned পূর্ণাঙ্ক m;
		ৱর্ণ name[128 + 1];


		যদি (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			অনুৱর্ত;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		যদি (loopiter_set_device(lc, name) == 0)
			ঘূৰাই 0;
	}

	ঘূৰাই 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
স্থির পূর্ণাঙ্ক loopcxt_next_from_sysfs(সংরচনা loopdev_cxt *lc)
{
	সংরচনা loopdev_iter *iter = &lc->iter;
	সংরচনা dirent *d;
	পূর্ণাঙ্ক fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	যদি (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	যদি (!iter->sysblock)
		ঘূৰাই 1;

	fd = dirfd(iter->sysblock);

	যেতিয়ালৈ ((d = readdir(iter->sysblock))) {
		ৱর্ণ name[NAME_MAX + 18 + 1];
		সংরচনা stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		যদি (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			অনুৱর্ত;

		snprintf(name, পরিমাণ(name), "%s/loop/backing_file", d->d_name);
		যদি (fstatat(fd, name, &st, 0) != 0)
			অনুৱর্ত;

		যদি (loopiter_set_device(lc, d->d_name) == 0)
			ঘূৰাই 0;
	}

	ঘূৰাই 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
পূর্ণাঙ্ক loopcxt_next(সংরচনা loopdev_cxt *lc)
{
	সংরচনা loopdev_iter *iter;

	যদি (!lc)
		ঘূৰাই -EINVAL;


	iter = &lc->iter;
	যদি (iter->done)
		ঘূৰাই 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	যদি (iter->flags & LOOPITER_FL_USED) {
		পূর্ণাঙ্ক rc;

		যদি (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		অন্যথা
			rc = loopcxt_next_from_proc(lc);
		যদি (rc == 0)
			ঘূৰাই 0;
		গচ্ছ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	যদি (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		বাবে (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			ৱর্ণ name[16];
			snprintf(name, পরিমাণ(name), "loop%d", iter->ncur);

			যদি (loopiter_set_device(lc, name) == 0)
				ঘূৰাই 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	যদি (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	বাবে (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		ৱর্ণ name[16];
		snprintf(name, পরিমাণ(name), "loop%d", iter->minors[iter->ncur]);

		যদি (loopiter_set_device(lc, name) == 0)
			ঘূৰাই 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ঘূৰাই 1;
}

/*
 * @device: path to device
 */
পূর্ণাঙ্ক is_loopdev(ধ্রুৱ ৱর্ণ *device)
{
	সংরচনা stat st;
	পূর্ণাঙ্ক rc = 0;

	যদি (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	অন্যথা যদি (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	অন্যথা যদি (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		ৱর্ণ name[PATH_MAX], *cn, *p = রিক্ত;

		snprintf(name, পরিমাণ(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		যদি (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		মুক্ত(cn);
	}

	যদি (rc == 0)
		errno = ENODEV;
	ঘূৰাই rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
সংরচনা loop_info64 *loopcxt_get_info(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক fd;

	যদি (!lc || lc->info_failed) {
		errno = EINVAL;
		ঘূৰাই রিক্ত;
	}
	errno = 0;
	যদি (lc->has_info)
		ঘূৰাই &lc->config.info;

	fd = loopcxt_get_fd(lc);
	যদি (fd < 0)
		ঘূৰাই রিক্ত;

	যদি (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ঘূৰাই &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ঘূৰাই রিক্ত;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
ৱর্ণ *loopcxt_get_backing_file(সংরচনা loopdev_cxt *lc)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ৱর্ণ *res = রিক্ত;

	যদি (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	যদি (!res && loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);

		যদি (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((ৱর্ণ *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ঘূৰাই res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
ৱর্ণ *loopcxt_get_refname(সংরচনা loopdev_cxt *lc)
{
	ৱর্ণ *res = রিক্ত;
	সংরচনা loop_info64 *lo = loopcxt_get_info(lc);

	যদি (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((ৱর্ণ *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ঘূৰাই res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_offset(সংরচনা loopdev_cxt *lc, uint64_t *offset)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণাঙ্ক rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	যদি (rc && loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo) {
			যদি (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} অন্যথা
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_blocksize(সংরচনা loopdev_cxt *lc, uint64_t *blocksize)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণাঙ্ক rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	যদি (rc) {
		পূর্ণাঙ্ক fd = loopcxt_get_fd(lc);
		পূর্ণাঙ্ক sz = 0;

		যদি (fd < 0)
			ঘূৰাই -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		যদি (rc)
			ঘূৰাই rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_sizelimit(সংরচনা loopdev_cxt *lc, uint64_t *size)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণাঙ্ক rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	যদি (rc && loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo) {
			যদি (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} অন্যথা
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_encrypt_type(সংরচনা loopdev_cxt *lc, uint32_t *type)
{
	সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণাঙ্ক rc;

	/* not provided by sysfs */
	যদি (lo) {
		যদি (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ধ্রুৱ ৱর্ণ *loopcxt_get_crypt_name(সংরচনা loopdev_cxt *lc)
{
	সংরচনা loop_info64 *lo = loopcxt_get_info(lc);

	যদি (lo)
		ঘূৰাই (ৱর্ণ *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ঘূৰাই রিক্ত;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_backing_devno(সংরচনা loopdev_cxt *lc, dev_t *devno)
{
	সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণাঙ্ক rc;

	যদি (lo) {
		যদি (devno)
			*devno = lo->lo_device;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণাঙ্ক loopcxt_get_backing_inode(সংরচনা loopdev_cxt *lc, ino_t *ino)
{
	সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণাঙ্ক rc;

	যদি (lo) {
		যদি (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ঘূৰাই rc;
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
পূর্ণাঙ্ক loopmod_supports_partscan(শূন্য)
{
	পূর্ণাঙ্ক rc, ret = 0;
	FILE *f;

	যদি (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ঘূৰাই 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	যদি (!f)
		ঘূৰাই 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ঘূৰাই rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
পূর্ণাঙ্ক loopcxt_is_partscan(সংরচনা loopdev_cxt *lc)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		/* kernel >= 3.2 */
		পূর্ণাঙ্ক fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ঘূৰাই fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ঘূৰাই loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
পূর্ণাঙ্ক loopcxt_is_autoclear(সংরচনা loopdev_cxt *lc)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণাঙ্ক fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ঘূৰাই fl;
	}

	যদি (loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ঘূৰাই lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ঘূৰাই 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
পূর্ণাঙ্ক loopcxt_is_readonly(সংরচনা loopdev_cxt *lc)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণাঙ্ক fl;
		যদি (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ঘূৰাই fl;
	}

	যদি (loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ঘূৰাই lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ঘূৰাই 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
পূর্ণাঙ্ক loopcxt_is_dio(সংরচনা loopdev_cxt *lc)
{
	সংরচনা path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণাঙ্ক fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ঘূৰাই fl;
	}
	যদি (loopcxt_ioctl_enabled(lc)) {
		সংরচনা loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ঘূৰাই lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ঘূৰাই 0;
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
পূর্ণাঙ্ক loopcxt_is_used(সংরচনা loopdev_cxt *lc,
		    সংরচনা stat *st,
		    ধ্রুৱ ৱর্ণ *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    পূর্ণাঙ্ক flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	যদি (!lc)
		ঘূৰাই 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	যদি (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		যদি (ino == st->st_ino && dev == st->st_dev)
			গচ্ছ found;

		/* don't use filename if we have devno and inode */
		ঘূৰাই 0;
	}

	/* poor man's solution */
	যদি (backing_file) {
		ৱর্ণ *name = loopcxt_get_backing_file(lc);
		পূর্ণাঙ্ক rc = name && strcmp(name, backing_file) == 0;

		মুক্ত(name);
		যদি (rc)
			গচ্ছ found;
	}

	ঘূৰাই 0;
found:
	যদি (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		পূর্ণাঙ্ক rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		যদি (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ঘূৰাই loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ঘূৰাই rc;
	}
	ঘূৰাই 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণাঙ্ক loopcxt_set_offset(সংরচনা loopdev_cxt *lc, uint64_t offset)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ঘূৰাই 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণাঙ্ক loopcxt_set_sizelimit(সংরচনা loopdev_cxt *lc, uint64_t sizelimit)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ঘূৰাই 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণাঙ্ক loopcxt_set_blocksize(সংরচনা loopdev_cxt *lc, uint64_t blocksize)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ঘূৰাই 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণাঙ্ক loopcxt_set_flags(সংরচনা loopdev_cxt *lc, uint32_t flags)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ঘূৰাই 0;
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
পূর্ণাঙ্ক loopcxt_set_refname(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *refname)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;

	memset(lc->config.info.lo_file_name, 0, পরিমাণ(lc->config.info.lo_file_name));
	যদি (refname)
		xstrncpy((ৱর্ণ *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (ৱর্ণ *)lc->config.info.lo_file_name));
	ঘূৰাই 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণাঙ্ক loopcxt_set_backing_file(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *filename)
{
	যদি (!lc)
		ঘূৰাই -EINVAL;

	যদি (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	অন্যথা
		lc->filename = ul_canonicalize_path(filename);
	যদি (!lc->filename)
		ঘূৰাই -errno;

	যদি (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক loopcxt_check_size(সংরচনা loopdev_cxt *lc, পূর্ণাঙ্ক file_fd)
{
	uint64_t size, expected_size;
	পূর্ণাঙ্ক dev_fd;
	সংরচনা stat st;

	যদি (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ঘূৰাই 0;

	যদি (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ঘূৰাই -errno;
	}
	যদি (S_ISBLK(st.st_mode)) {
		যদি (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ঘূৰাই -errno;
		}
	} অন্যথা
		expected_size = st.st_size;

	যদি (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ঘূৰাই 0;	/* ignore this error */
	}

	যদি (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	যদি (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	যদি (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ঘূৰাই -errno;
	}

	যদি (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ঘূৰাই -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	যদি (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	যদি (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		যদি (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			যদি (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ঘূৰাই -errno;
		}

		যদি (blkdev_get_size(dev_fd, &size))
			ঘূৰাই -errno;

		যদি (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ঘূৰাই -errno;
		}
	}

	ঘূৰাই 0;
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
পূর্ণাঙ্ক loopcxt_setup_device(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	পূর্ণাঙ্ক rc = -1, cnt = 0;
	পূর্ণাঙ্ক errsv = 0;
	পূর্ণাঙ্ক fallback = 0;

	যদি (!lc || !*lc->device || !lc->filename)
		ঘূৰাই -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	যদি (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	যদি (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	যদি (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	অন্যথা
		file_fd = open(lc->filename, mode | flags);

	যদি (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		যদি (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		অন্যথা
			file_fd = open(lc->filename, mode | flags);
	}
	যদি (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ঘূৰাই -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	যদি (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	অন্যথা
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	কৰা {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		যদি (dev_fd >= 0 || lc->control_ok == 0)
			ভঙ্গ;
		যদি (errno != EACCES && errno != ENOENT)
			ভঙ্গ;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} যেতিয়ালৈ (cnt++ < 16);

	যদি (dev_fd < 0) {
		rc = -errno;
		গচ্ছ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	যদি (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	যদি (rc != 0) {
		errsv = errno;
		যদি (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			গচ্ছ err;
		}
		fallback = 1;
	} অন্যথা {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	যদি (fallback) {
		যদি (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			গচ্ছ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		যদি (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			গচ্ছ err;
		}

		যদি ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			গচ্ছ err;
		}
	}

	যদি ((rc = loopcxt_check_size(lc, file_fd)))
		গচ্ছ err;

	close(file_fd);

	memset(&lc->config, 0, পরিমাণ(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ঘূৰাই 0;
err:
	যদি (file_fd >= 0)
		close(file_fd);
	যদি (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	যদি (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ঘূৰাই rc;
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
পূর্ণাঙ্ক loopcxt_ioctl_status(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	যদি (dev_fd < 0)
		ঘূৰাই -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	যদি (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ঘূৰাই rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ঘূৰাই 0;
}

পূর্ণাঙ্ক loopcxt_ioctl_capacity(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ঘূৰাই -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ঘূৰাই rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ঘূৰাই 0;
}

পূর্ণাঙ্ক loopcxt_ioctl_dio(সংরচনা loopdev_cxt *lc, unsigned long use_dio)
{
	পূর্ণাঙ্ক rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ঘূৰাই -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ঘূৰাই rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ঘূৰাই 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
পূর্ণাঙ্ক loopcxt_ioctl_blocksize(সংরচনা loopdev_cxt *lc, uint64_t blocksize)
{
	পূর্ণাঙ্ক rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ঘূৰাই -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ঘূৰাই rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক loopcxt_get_device_nr(সংরচনা loopdev_cxt *lc, পূর্ণাঙ্ক *nr)
{
	ধ্রুৱ ৱর্ণ *p, *dev;
	পূর্ণাঙ্ক rc = -EINVAL;

	errno = 0;
	যদি (!lc || !nr)
		ঘূৰাই rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	যদি (!dev)
		গচ্ছ done;

	p = strrchr(dev, '/');
	যদি (!p)
		গচ্ছ done;

	যদি (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		গচ্ছ done;

	যদি (*nr < 0)
		গচ্ছ done;
	rc = 0;
done:
	যদি (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ঘূৰাই rc;
}

পূর্ণাঙ্ক loopcxt_detach_device(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ঘূৰাই -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ঘূৰাই rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ঘূৰাই 0;
}

পূর্ণাঙ্ক loopcxt_remove_device(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক rc = -EINVAL;
	পূর্ণাঙ্ক ctl, nr = -1;

	যদি (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		গচ্ছ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	যদি (rc)
		গচ্ছ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	যদি (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ঘূৰাই rc;
}

পূর্ণাঙ্ক loopcxt_add_device(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক rc = -EINVAL;
	পূর্ণাঙ্ক ctl, nr = -1;

	যদি (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		গচ্ছ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	যদি (rc)
		গচ্ছ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	যদি (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
পূর্ণাঙ্ক loopcxt_find_unused(সংরচনা loopdev_cxt *lc)
{
	পূর্ণাঙ্ক rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	যদি (lc->flags & LOOPDEV_FL_CONTROL) {
		পূর্ণাঙ্ক ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		যদি (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		অন্যথা
			rc = -errno;
		যদি (rc >= 0) {
			ৱর্ণ name[16];
			snprintf(name, পরিমাণ(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		যদি (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	যদি (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		যদি (rc)
			ঘূৰাই rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		যদি (rc)
			ঘূৰাই -ENOENT;
	}
	ঘূৰাই rc;
}



/*
 * Return: TRUE/FALSE
 */
পূর্ণাঙ্ক loopdev_is_autoclear(ধ্রুৱ ৱর্ণ *device)
{
	সংরচনা loopdev_cxt lc;
	পূর্ণাঙ্ক rc;

	যদি (!device)
		ঘূৰাই 0;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ঘূৰাই rc;
}

ৱর্ণ *loopdev_get_backing_file(ধ্রুৱ ৱর্ণ *device)
{
	সংরচনা loopdev_cxt lc;
	ৱর্ণ *res = রিক্ত;

	যদি (!device)
		ঘূৰাই রিক্ত;
	যদি (loopcxt_init(&lc, 0))
		ঘূৰাই রিক্ত;
	যদি (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ঘূৰাই res;
}

/*
 * Returns: TRUE/FALSE
 */
পূর্ণাঙ্ক loopdev_has_backing_file(ধ্রুৱ ৱর্ণ *device)
{
	ৱর্ণ *tmp = loopdev_get_backing_file(device);

	যদি (tmp) {
		মুক্ত(tmp);
		ঘূৰাই 1;
	}
	ঘূৰাই 0;
}

/*
 * Returns: TRUE/FALSE
 */
পূর্ণাঙ্ক loopdev_is_used(ধ্রুৱ ৱর্ণ *device, ধ্রুৱ ৱর্ণ *filename,
		    uint64_t offset, uint64_t sizelimit, পূর্ণাঙ্ক flags)
{
	সংরচনা loopdev_cxt lc;
	সংরচনা stat st;
	পূর্ণাঙ্ক rc = 0;

	যদি (!device || !filename)
		ঘূৰাই 0;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (rc)
		ঘূৰাই rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : রিক্ত, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ঘূৰাই rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
পূর্ণাঙ্ক loopdev_detach(ধ্রুৱ ৱর্ণ *device)
{
	সংরচনা loopdev_cxt lc;
	পূর্ণাঙ্ক rc;

	যদি (!device)
		ঘূৰাই -EINVAL;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ঘূৰাই rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
পূর্ণাঙ্ক loopcxt_find_by_backing_file(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *filename,
				 uint64_t offset, uint64_t sizelimit, পূর্ণাঙ্ক flags)
{
	পূর্ণাঙ্ক rc, hasst;
	সংরচনা stat st;

	যদি (!filename)
		ঘূৰাই -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	যদি (rc)
		ঘূৰাই rc;

	যেতিয়ালৈ ((rc = loopcxt_next(lc)) == 0) {

		যদি (loopcxt_is_used(lc, hasst ? &st : রিক্ত,
				    filename, offset, sizelimit, flags))
			ভঙ্গ;
	}

	loopcxt_deinit_iterator(lc);
	ঘূৰাই rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
পূর্ণাঙ্ক loopcxt_find_overlap(সংরচনা loopdev_cxt *lc, ধ্রুৱ ৱর্ণ *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	পূর্ণাঙ্ক rc, hasst;
	সংরচনা stat st;

	যদি (!filename)
		ঘূৰাই -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	যদি (rc)
		ঘূৰাই rc;

	যেতিয়ালৈ ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : রিক্ত,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		যদি (rc <= 0)
			অনুৱর্ত;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		যদি (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			অনুৱর্ত;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		যদি (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			অনুৱর্ত;
		}

		/* full match */
		যদি (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			গচ্ছ found;
		}

		/* overlap */
		যদি (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			অনুৱর্ত;
		যদি (sizelimit != 0 && offset + sizelimit <= lc_offset)
			অনুৱর্ত;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			গচ্ছ found;
	}

	যদি (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ঘূৰাই rc;
}

/*
 * Returns allocated string with device name
 */
ৱর্ণ *loopdev_find_by_backing_file(ধ্রুৱ ৱর্ণ *filename, uint64_t offset, uint64_t sizelimit, পূর্ণাঙ্ক flags)
{
	সংরচনা loopdev_cxt lc;
	ৱর্ণ *res = রিক্ত;

	যদি (!filename)
		ঘূৰাই রিক্ত;

	যদি (loopcxt_init(&lc, 0))
		ঘূৰাই রিক্ত;
	যদি (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ঘূৰাই res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
পূর্ণাঙ্ক loopdev_গুনতি_by_backing_file(ধ্রুৱ ৱর্ণ *filename, ৱর্ণ **loopdev)
{
	সংরচনা loopdev_cxt lc;
	পূর্ণাঙ্ক গুনতি = 0, rc;

	যদি (!filename)
		ঘূৰাই -1;

	rc = loopcxt_init(&lc, 0);
	যদি (rc)
		ঘূৰাই rc;
	যদি (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ঘূৰাই -1;

	যেতিয়ালৈ(loopcxt_next(&lc) == 0) {
		ৱর্ণ *backing = loopcxt_get_backing_file(&lc);

		যদি (!backing || strcmp(backing, filename) != 0) {
			মুক্ত(backing);
			অনুৱর্ত;
		}

		মুক্ত(backing);
		যদি (loopdev && গুনতি == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		গুনতি++;
	}

	loopcxt_deinit(&lc);

	যদি (loopdev && গুনতি > 1) {
		মুক্ত(*loopdev);
		*loopdev = রিক্ত;
	}
	ঘূৰাই গুনতি;
}

#ifdef TEST_PROGRAM_LOOPDEV
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	যদি (argc < 2)
		গচ্ছ usage;

	যদি (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	অন্যথা
		গচ্ছ usage;

	ঘূৰাই EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ঘূৰাই EXIT_FAILURE;
}
#endif

