
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
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/mman.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <dirent.h>

#অন্তর্ভাব "linux_version.h"
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "timeutils.h"
#অন্তর্ভাব "sysfs.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "loopdev.h"
#অন্তর্ভাব "canonicalize.h"
#অন্তর্ভাব "blkdev.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "fileutils.h"

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
		ফেরত;
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
		পূর্ণসংখ্যা _c = 0, _e;					\
		করো {						\
			errno = 0;				\
			_e = x;					\
			যদি (_e == 0 || errno != EAGAIN)		\
				বিরতি;				\
			যদি (_c >= LOOPDEV_MAX_TRIES)		\
				বিরতি;				\
			xusleep(250000);			\
			_c++;					\
		} যখন (1);					\
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
পূর্ণসংখ্যা loopcxt_set_device(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *device)
{
	যদি (!lc)
		ফেরত -EINVAL;

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
	memset(&lc->config, 0, আকার(lc->config));

	/* set new */
	যদি (device) {
		যদি (*device != '/') {
			ধ্রুবক অক্ষর *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			যদি (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				যদি (strlen(device) < 5)
					ফেরত -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, আকার(lc->device), "%s%s",
				dir, device);
		} অন্যথা
			xstrncpy(lc->device, device, আকার(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = ফাঁকা;
	ফেরত 0;
}

পূর্ণসংখ্যা loopcxt_has_device(কাঠামো loopdev_cxt *lc)
{
	ফেরত lc && *lc->device;
}

dev_t loopcxt_get_devno(কাঠামো loopdev_cxt *lc)
{
	যদি (!lc || !loopcxt_has_device(lc))
		ফেরত 0;
	যদি (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ফেরত lc->devno;
}

পূর্ণসংখ্যা loopcxt_is_lost(কাঠামো loopdev_cxt *lc)
{
	যদি (!lc || !loopcxt_has_device(lc))
		ফেরত 0;
	যদি (lc->is_lost)
		ফেরত 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ফেরত lc->is_lost;
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
পূর্ণসংখ্যা loopcxt_init(কাঠামো loopdev_cxt *lc, পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা rc;
	কাঠামো stat st;
	কাঠামো loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	যদি (!lc)
		ফেরত -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, আকার(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, ফাঁকা);
	যদি (rc)
		ফেরত rc;

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

	ফেরত 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
শূন্য loopcxt_deinit(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা errsv = errno;

	যদি (!lc)
		ফেরত;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	মুক্ত(lc->filename);
	lc->filename = ফাঁকা;

	ignore_ফলাফল( loopcxt_set_device(lc, ফাঁকা) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
অক্ষর *loopcxt_strdup_device(কাঠামো loopdev_cxt *lc)
{
	যদি (!lc || !*lc->device)
		ফেরত ফাঁকা;
	ফেরত strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ধ্রুবক অক্ষর *loopcxt_get_device(কাঠামো loopdev_cxt *lc)
{
	ফেরত lc && *lc->device ? lc->device : ফাঁকা;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
স্থির কাঠামো path_cxt *loopcxt_get_sysfs(কাঠামো loopdev_cxt *lc)
{
	যদি (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ফেরত ফাঁকা;

	যদি (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		যদি (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ফেরত ফাঁকা;
		}

		lc->sysfs = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
		যদি (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ফেরত lc->sysfs;
}

স্থির পূর্ণসংখ্যা __loopcxt_get_fd(কাঠামো loopdev_cxt *lc, mode_t mode)
{
	পূর্ণসংখ্যা old = -1;

	যদি (!lc || !*lc->device)
		ফেরত -EINVAL;

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
	ফেরত lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
পূর্ণসংখ্যা loopcxt_get_fd(কাঠামো loopdev_cxt *lc)
{
	ফেরত __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_init_iterator(কাঠামো loopdev_cxt *lc, পূর্ণসংখ্যা flags)
{
	কাঠামো loopdev_iter *iter;
	কাঠামো stat st;

	যদি (!lc)
		ফেরত -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, আকার(*iter));
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
	ফেরত 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_deinit_iterator(কাঠামো loopdev_cxt *lc)
{
	কাঠামো loopdev_iter *iter;

	যদি (!lc)
		ফেরত -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	মুক্ত(iter->minors);
	যদি (iter->proc)
		fclose(iter->proc);
	যদি (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, আকার(*iter));
	ফেরত 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
স্থির পূর্ণসংখ্যা loopiter_set_device(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *device)
{
	পূর্ণসংখ্যা rc = loopcxt_set_device(lc, device);
	পূর্ণসংখ্যা used;

	যদি (rc)
		ফেরত rc;

	যদি (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ফেরত 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, ফাঁকা) == 0;

	যদি ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ফেরত 0;

	যদি ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ফেরত 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_ফলাফল( loopcxt_set_device(lc, ফাঁকা) );
	ফেরত 1;
}

স্থির পূর্ণসংখ্যা cmpnum(ধ্রুবক শূন্য *p1, ধ্রুবক শূন্য *p2)
{
	ফেরত (((* (ধ্রুবক পূর্ণসংখ্যা *) p1) > (* (ধ্রুবক পূর্ণসংখ্যা *) p2)) -
			((* (ধ্রুবক পূর্ণসংখ্যা *) p1) < (* (ধ্রুবক পূর্ণসংখ্যা *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
স্থির পূর্ণসংখ্যা loop_scandir(ধ্রুবক অক্ষর *dirname, পূর্ণসংখ্যা **ary, পূর্ণসংখ্যা hasprefix)
{
	DIR *dir;
	কাঠামো dirent *d;
	unsigned পূর্ণসংখ্যা n, গুনতি = 0, arylen = 0;

	যদি (!dirname || !ary)
		ফেরত 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	যদি (!dir)
		ফেরত 0;
	মুক্ত(*ary);
	*ary = ফাঁকা;

	যখন((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			চালু;
#endif
		যদি (is_dotdir_dirent(d))
			চালু;

		যদি (hasprefix) {
			/* /dev/loop<N> */
			যদি (sscanf(d->d_name, "loop%u", &n) != 1)
				চালু;
		} অন্যথা {
			/* /dev/loop/<N> */
			অক্ষর *end = ফাঁকা;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			যদি (d->d_name == end || (end && *end) || errno)
				চালু;
		}
		যদি (n < LOOPDEV_DEFAULT_NNODES)
			চালু;			/* ignore loop<0..7> */

		যদি (গুনতি + 1 > arylen) {
			পূর্ণসংখ্যা *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, আকার(পূর্ণসংখ্যা));
			যদি (!tmp) {
				মুক্ত(*ary);
				*ary = ফাঁকা;
				closedir(dir);
				ফেরত -1;
			}
			*ary = tmp;
		}
		যদি (*ary)
			(*ary)[গুনতি++] = n;
	}
	যদি (গুনতি && *ary)
		qsort(*ary, গুনতি, আকার(পূর্ণসংখ্যা), cmpnum);

	closedir(dir);
	ফেরত গুনতি;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
স্থির পূর্ণসংখ্যা loopcxt_পরবর্তী_from_proc(কাঠামো loopdev_cxt *lc)
{
	কাঠামো loopdev_iter *iter = &lc->iter;
	অক্ষর buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	যদি (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	যদি (!iter->proc)
		ফেরত 1;

	যখন (fgets(buf, আকার(buf), iter->proc)) {
		unsigned পূর্ণসংখ্যা m;
		অক্ষর name[128 + 1];


		যদি (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			চালু;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		যদি (loopiter_set_device(lc, name) == 0)
			ফেরত 0;
	}

	ফেরত 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
স্থির পূর্ণসংখ্যা loopcxt_পরবর্তী_from_sysfs(কাঠামো loopdev_cxt *lc)
{
	কাঠামো loopdev_iter *iter = &lc->iter;
	কাঠামো dirent *d;
	পূর্ণসংখ্যা fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	যদি (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	যদি (!iter->sysblock)
		ফেরত 1;

	fd = dirfd(iter->sysblock);

	যখন ((d = readdir(iter->sysblock))) {
		অক্ষর name[NAME_MAX + 18 + 1];
		কাঠামো stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		যদি (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			চালু;

		snprintf(name, আকার(name), "%s/loop/backing_file", d->d_name);
		যদি (fstatat(fd, name, &st, 0) != 0)
			চালু;

		যদি (loopiter_set_device(lc, d->d_name) == 0)
			ফেরত 0;
	}

	ফেরত 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
পূর্ণসংখ্যা loopcxt_পরবর্তী(কাঠামো loopdev_cxt *lc)
{
	কাঠামো loopdev_iter *iter;

	যদি (!lc)
		ফেরত -EINVAL;


	iter = &lc->iter;
	যদি (iter->done)
		ফেরত 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	যদি (iter->flags & LOOPITER_FL_USED) {
		পূর্ণসংখ্যা rc;

		যদি (loopcxt_sysfs_available(lc))
			rc = loopcxt_পরবর্তী_from_sysfs(lc);
		অন্যথা
			rc = loopcxt_পরবর্তী_from_proc(lc);
		যদি (rc == 0)
			ফেরত 0;
		যাও done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	যদি (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		জন্য (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			অক্ষর name[16];
			snprintf(name, আকার(name), "loop%d", iter->ncur);

			যদি (loopiter_set_device(lc, name) == 0)
				ফেরত 0;
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
	জন্য (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		অক্ষর name[16];
		snprintf(name, আকার(name), "loop%d", iter->minors[iter->ncur]);

		যদি (loopiter_set_device(lc, name) == 0)
			ফেরত 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ফেরত 1;
}

/*
 * @device: path to device
 */
পূর্ণসংখ্যা is_loopdev(ধ্রুবক অক্ষর *device)
{
	কাঠামো stat st;
	পূর্ণসংখ্যা rc = 0;

	যদি (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	অন্যথা যদি (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	অন্যথা যদি (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		অক্ষর name[PATH_MAX], *cn, *p = ফাঁকা;

		snprintf(name, আকার(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		যদি (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		মুক্ত(cn);
	}

	যদি (rc == 0)
		errno = ENODEV;
	ফেরত rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
কাঠামো loop_info64 *loopcxt_get_info(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা fd;

	যদি (!lc || lc->info_failed) {
		errno = EINVAL;
		ফেরত ফাঁকা;
	}
	errno = 0;
	যদি (lc->has_info)
		ফেরত &lc->config.info;

	fd = loopcxt_get_fd(lc);
	যদি (fd < 0)
		ফেরত ফাঁকা;

	যদি (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ফেরত &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ফেরত ফাঁকা;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
অক্ষর *loopcxt_get_backing_file(কাঠামো loopdev_cxt *lc)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);
	অক্ষর *res = ফাঁকা;

	যদি (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	যদি (!res && loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);

		যদি (lo) {
			lo->lo_file_name[LO_NAME_আকার - 2] = '*';
			lo->lo_file_name[LO_NAME_আকার - 1] = '\0';
			res = strdup((অক্ষর *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ফেরত res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
অক্ষর *loopcxt_get_refname(কাঠামো loopdev_cxt *lc)
{
	অক্ষর *res = ফাঁকা;
	কাঠামো loop_info64 *lo = loopcxt_get_info(lc);

	যদি (lo) {
		lo->lo_file_name[LO_NAME_আকার - 1] = '\0';
		res = strdup((অক্ষর *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ফেরত res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_offset(কাঠামো loopdev_cxt *lc, uint64_t *offset)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণসংখ্যা rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	যদি (rc && loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo) {
			যদি (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} অন্যথা
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ফেরত rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_blocksize(কাঠামো loopdev_cxt *lc, uint64_t *blocksize)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণসংখ্যা rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	যদি (rc) {
		পূর্ণসংখ্যা fd = loopcxt_get_fd(lc);
		পূর্ণসংখ্যা sz = 0;

		যদি (fd < 0)
			ফেরত -EINVAL;
		rc = blkdev_get_sector_আকার(fd, &sz);
		যদি (rc)
			ফেরত rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ফেরত rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_sizelimit(কাঠামো loopdev_cxt *lc, uint64_t *আকার_2)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);
	পূর্ণসংখ্যা rc = -EINVAL;

	যদি (sysfs)
		যদি (ul_path_read_u64(sysfs, আকার_2, "loop/sizelimit") == 0)
			rc = 0;

	যদি (rc && loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo) {
			যদি (আকার_2)
				*আকার_2 = lo->lo_sizelimit;
			rc = 0;
		} অন্যথা
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ফেরত rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_encrypt_type(কাঠামো loopdev_cxt *lc, uint32_t *type)
{
	কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণসংখ্যা rc;

	/* not provided by sysfs */
	যদি (lo) {
		যদি (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ফেরত rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ধ্রুবক অক্ষর *loopcxt_get_crypt_name(কাঠামো loopdev_cxt *lc)
{
	কাঠামো loop_info64 *lo = loopcxt_get_info(lc);

	যদি (lo)
		ফেরত (অক্ষর *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ফেরত ফাঁকা;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_backing_devno(কাঠামো loopdev_cxt *lc, dev_t *devno)
{
	কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণসংখ্যা rc;

	যদি (lo) {
		যদি (devno)
			*devno = lo->lo_device;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ফেরত rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
পূর্ণসংখ্যা loopcxt_get_backing_inode(কাঠামো loopdev_cxt *lc, ino_t *ino)
{
	কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
	পূর্ণসংখ্যা rc;

	যদি (lo) {
		যদি (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} অন্যথা
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ফেরত rc;
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
পূর্ণসংখ্যা loopmod_supports_partscan(শূন্য)
{
	পূর্ণসংখ্যা rc, ret = 0;
	FILE *f;

	যদি (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ফেরত 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	যদি (!f)
		ফেরত 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ফেরত rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
পূর্ণসংখ্যা loopcxt_is_partscan(কাঠামো loopdev_cxt *lc)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		/* kernel >= 3.2 */
		পূর্ণসংখ্যা fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ফেরত fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ফেরত loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
পূর্ণসংখ্যা loopcxt_is_autoclear(কাঠামো loopdev_cxt *lc)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণসংখ্যা fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ফেরত fl;
	}

	যদি (loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ফেরত lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ফেরত 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
পূর্ণসংখ্যা loopcxt_is_readonly(কাঠামো loopdev_cxt *lc)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণসংখ্যা fl;
		যদি (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ফেরত fl;
	}

	যদি (loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ফেরত lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ফেরত 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
পূর্ণসংখ্যা loopcxt_is_dio(কাঠামো loopdev_cxt *lc)
{
	কাঠামো path_cxt *sysfs = loopcxt_get_sysfs(lc);

	যদি (sysfs) {
		পূর্ণসংখ্যা fl;
		যদি (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ফেরত fl;
	}
	যদি (loopcxt_ioctl_enabled(lc)) {
		কাঠামো loop_info64 *lo = loopcxt_get_info(lc);
		যদি (lo)
			ফেরত lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ফেরত 0;
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
পূর্ণসংখ্যা loopcxt_is_used(কাঠামো loopdev_cxt *lc,
		    কাঠামো stat *st,
		    ধ্রুবক অক্ষর *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    পূর্ণসংখ্যা flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	যদি (!lc)
		ফেরত 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	যদি (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		যদি (ino == st->st_ino && dev == st->st_dev)
			যাও found;

		/* don't use filename if we have devno and inode */
		ফেরত 0;
	}

	/* poor man's solution */
	যদি (backing_file) {
		অক্ষর *name = loopcxt_get_backing_file(lc);
		পূর্ণসংখ্যা rc = name && strcmp(name, backing_file) == 0;

		মুক্ত(name);
		যদি (rc)
			যাও found;
	}

	ফেরত 0;
found:
	যদি (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		পূর্ণসংখ্যা rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		যদি (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ফেরত loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ফেরত rc;
	}
	ফেরত 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণসংখ্যা loopcxt_set_offset(কাঠামো loopdev_cxt *lc, uint64_t offset)
{
	যদি (!lc)
		ফেরত -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ফেরত 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণসংখ্যা loopcxt_set_sizelimit(কাঠামো loopdev_cxt *lc, uint64_t sizelimit)
{
	যদি (!lc)
		ফেরত -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ফেরত 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
পূর্ণসংখ্যা loopcxt_set_blocksize(কাঠামো loopdev_cxt *lc, uint64_t blocksize)
{
	যদি (!lc)
		ফেরত -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ফেরত 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণসংখ্যা loopcxt_set_flags(কাঠামো loopdev_cxt *lc, uint32_t flags)
{
	যদি (!lc)
		ফেরত -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ফেরত 0;
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
পূর্ণসংখ্যা loopcxt_set_refname(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *refname)
{
	যদি (!lc)
		ফেরত -EINVAL;

	memset(lc->config.info.lo_file_name, 0, আকার(lc->config.info.lo_file_name));
	যদি (refname)
		xstrncpy((অক্ষর *)lc->config.info.lo_file_name, refname, LO_NAME_আকার);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (অক্ষর *)lc->config.info.lo_file_name));
	ফেরত 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
পূর্ণসংখ্যা loopcxt_set_backing_file(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *filename)
{
	যদি (!lc)
		ফেরত -EINVAL;

	যদি (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	অন্যথা
		lc->filename = ul_canonicalize_path(filename);
	যদি (!lc->filename)
		ফেরত -errno;

	যদি (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ফেরত 0;
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
স্থির পূর্ণসংখ্যা loopcxt_check_আকার(কাঠামো loopdev_cxt *lc, পূর্ণসংখ্যা file_fd)
{
	uint64_t আকার_2, expected_আকার;
	পূর্ণসংখ্যা dev_fd;
	কাঠামো stat st;

	যদি (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ফেরত 0;

	যদি (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ফেরত -errno;
	}
	যদি (S_ISBLK(st.st_mode)) {
		যদি (blkdev_get_আকার(file_fd, &expected_আকার)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ফেরত -errno;
		}
	} অন্যথা
		expected_আকার = st.st_size;

	যদি (expected_আকার == 0 || expected_আকার <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ফেরত 0;	/* ignore this error */
	}

	যদি (lc->config.info.lo_offset > 0)
		expected_আকার -= lc->config.info.lo_offset;

	যদি (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_আকার)
		expected_আকার = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	যদি (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ফেরত -errno;
	}

	যদি (blkdev_get_আকার(dev_fd, &আকার_2)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ফেরত -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	যদি (expected_আকার % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_আকার = (expected_আকার >> 9) << 9;
	}

	যদি (expected_আকার != আকার_2) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      আকার_2, expected_আকার));

		যদি (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			যদি (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ফেরত -errno;
		}

		যদি (blkdev_get_আকার(dev_fd, &আকার_2))
			ফেরত -errno;

		যদি (expected_আকার != আকার_2) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					আকার_2, expected_আকার));
			ফেরত -errno;
		}
	}

	ফেরত 0;
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
পূর্ণসংখ্যা loopcxt_setup_device(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	পূর্ণসংখ্যা rc = -1, cnt = 0;
	পূর্ণসংখ্যা errsv = 0;
	পূর্ণসংখ্যা fallback = 0;

	যদি (!lc || !*lc->device || !lc->filename)
		ফেরত -EINVAL;

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
		ফেরত -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	যদি (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	অন্যথা
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	করো {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		যদি (dev_fd >= 0 || lc->control_ok == 0)
			বিরতি;
		যদি (errno != EACCES && errno != ENOENT)
			বিরতি;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} যখন (cnt++ < 16);

	যদি (dev_fd < 0) {
		rc = -errno;
		যাও err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	যদি (lc->blocksize > 0)
		lc->config.block_আকার = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	যদি (rc != 0) {
		errsv = errno;
		যদি (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			যাও err;
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
			যাও err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		যদি (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			যাও err;
		}

		যদি ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			যাও err;
		}
	}

	যদি ((rc = loopcxt_check_আকার(lc, file_fd)))
		যাও err;

	close(file_fd);

	memset(&lc->config, 0, আকার(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ফেরত 0;
err:
	যদি (file_fd >= 0)
		close(file_fd);
	যদি (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	যদি (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ফেরত rc;
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
পূর্ণসংখ্যা loopcxt_ioctl_status(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	যদি (dev_fd < 0)
		ফেরত -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	যদি (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ফেরত rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ফেরত 0;
}

পূর্ণসংখ্যা loopcxt_ioctl_capacity(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ফেরত -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ফেরত rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ফেরত 0;
}

পূর্ণসংখ্যা loopcxt_ioctl_dio(কাঠামো loopdev_cxt *lc, unsigned long use_dio)
{
	পূর্ণসংখ্যা rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ফেরত -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ফেরত rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ফেরত 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
পূর্ণসংখ্যা loopcxt_ioctl_blocksize(কাঠামো loopdev_cxt *lc, uint64_t blocksize)
{
	পূর্ণসংখ্যা rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ফেরত -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_আকার, (unsigned long) blocksize) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ফেরত rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ফেরত 0;
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
স্থির পূর্ণসংখ্যা loopcxt_get_device_nr(কাঠামো loopdev_cxt *lc, পূর্ণসংখ্যা *nr)
{
	ধ্রুবক অক্ষর *p, *dev;
	পূর্ণসংখ্যা rc = -EINVAL;

	errno = 0;
	যদি (!lc || !nr)
		ফেরত rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	যদি (!dev)
		যাও done;

	p = strrchr(dev, '/');
	যদি (!p)
		যাও done;

	যদি (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		যাও done;

	যদি (*nr < 0)
		যাও done;
	rc = 0;
done:
	যদি (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ফেরত rc;
}

পূর্ণসংখ্যা loopcxt_detach_device(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা rc, fd = loopcxt_get_fd(lc);

	যদি (fd < 0)
		ফেরত -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	যদি (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ফেরত rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ফেরত 0;
}

পূর্ণসংখ্যা loopcxt_remove_device(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা rc = -EINVAL;
	পূর্ণসংখ্যা ctl, nr = -1;

	যদি (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		যাও done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	যদি (rc)
		যাও done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	যদি (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ফেরত rc;
}

পূর্ণসংখ্যা loopcxt_add_device(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা rc = -EINVAL;
	পূর্ণসংখ্যা ctl, nr = -1;

	যদি (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		যাও done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	যদি (rc)
		যাও done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	যদি (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ফেরত rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
পূর্ণসংখ্যা loopcxt_find_unused(কাঠামো loopdev_cxt *lc)
{
	পূর্ণসংখ্যা rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	যদি (lc->flags & LOOPDEV_FL_CONTROL) {
		পূর্ণসংখ্যা ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		যদি (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		অন্যথা
			rc = -errno;
		যদি (rc >= 0) {
			অক্ষর name[16];
			snprintf(name, আকার(name), "loop%d", rc);

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
			ফেরত rc;

		rc = loopcxt_পরবর্তী(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		যদি (rc)
			ফেরত -ENOENT;
	}
	ফেরত rc;
}



/*
 * Return: TRUE/FALSE
 */
পূর্ণসংখ্যা loopdev_is_autoclear(ধ্রুবক অক্ষর *device)
{
	কাঠামো loopdev_cxt lc;
	পূর্ণসংখ্যা rc;

	যদি (!device)
		ফেরত 0;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ফেরত rc;
}

অক্ষর *loopdev_get_backing_file(ধ্রুবক অক্ষর *device)
{
	কাঠামো loopdev_cxt lc;
	অক্ষর *res = ফাঁকা;

	যদি (!device)
		ফেরত ফাঁকা;
	যদি (loopcxt_init(&lc, 0))
		ফেরত ফাঁকা;
	যদি (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ফেরত res;
}

/*
 * Returns: TRUE/FALSE
 */
পূর্ণসংখ্যা loopdev_has_backing_file(ধ্রুবক অক্ষর *device)
{
	অক্ষর *tmp = loopdev_get_backing_file(device);

	যদি (tmp) {
		মুক্ত(tmp);
		ফেরত 1;
	}
	ফেরত 0;
}

/*
 * Returns: TRUE/FALSE
 */
পূর্ণসংখ্যা loopdev_is_used(ধ্রুবক অক্ষর *device, ধ্রুবক অক্ষর *filename,
		    uint64_t offset, uint64_t sizelimit, পূর্ণসংখ্যা flags)
{
	কাঠামো loopdev_cxt lc;
	কাঠামো stat st;
	পূর্ণসংখ্যা rc = 0;

	যদি (!device || !filename)
		ফেরত 0;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (rc)
		ফেরত rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : ফাঁকা, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ফেরত rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
পূর্ণসংখ্যা loopdev_detach(ধ্রুবক অক্ষর *device)
{
	কাঠামো loopdev_cxt lc;
	পূর্ণসংখ্যা rc;

	যদি (!device)
		ফেরত -EINVAL;

	rc = loopcxt_init(&lc, 0);
	যদি (!rc)
		rc = loopcxt_set_device(&lc, device);
	যদি (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ফেরত rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
পূর্ণসংখ্যা loopcxt_find_by_backing_file(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *filename,
				 uint64_t offset, uint64_t sizelimit, পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা rc, hasst;
	কাঠামো stat st;

	যদি (!filename)
		ফেরত -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	যদি (rc)
		ফেরত rc;

	যখন ((rc = loopcxt_পরবর্তী(lc)) == 0) {

		যদি (loopcxt_is_used(lc, hasst ? &st : ফাঁকা,
				    filename, offset, sizelimit, flags))
			বিরতি;
	}

	loopcxt_deinit_iterator(lc);
	ফেরত rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
পূর্ণসংখ্যা loopcxt_find_overlap(কাঠামো loopdev_cxt *lc, ধ্রুবক অক্ষর *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	পূর্ণসংখ্যা rc, hasst;
	কাঠামো stat st;

	যদি (!filename)
		ফেরত -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	যদি (rc)
		ফেরত rc;

	যখন ((rc = loopcxt_পরবর্তী(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : ফাঁকা,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		যদি (rc <= 0)
			চালু;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		যদি (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			চালু;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		যদি (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			চালু;
		}

		/* full match */
		যদি (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			যাও found;
		}

		/* overlap */
		যদি (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			চালু;
		যদি (sizelimit != 0 && offset + sizelimit <= lc_offset)
			চালু;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			যাও found;
	}

	যদি (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ফেরত rc;
}

/*
 * Returns allocated string with device name
 */
অক্ষর *loopdev_find_by_backing_file(ধ্রুবক অক্ষর *filename, uint64_t offset, uint64_t sizelimit, পূর্ণসংখ্যা flags)
{
	কাঠামো loopdev_cxt lc;
	অক্ষর *res = ফাঁকা;

	যদি (!filename)
		ফেরত ফাঁকা;

	যদি (loopcxt_init(&lc, 0))
		ফেরত ফাঁকা;
	যদি (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ফেরত res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
পূর্ণসংখ্যা loopdev_গুনতি_by_backing_file(ধ্রুবক অক্ষর *filename, অক্ষর **loopdev)
{
	কাঠামো loopdev_cxt lc;
	পূর্ণসংখ্যা গুনতি = 0, rc;

	যদি (!filename)
		ফেরত -1;

	rc = loopcxt_init(&lc, 0);
	যদি (rc)
		ফেরত rc;
	যদি (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ফেরত -1;

	যখন(loopcxt_পরবর্তী(&lc) == 0) {
		অক্ষর *backing = loopcxt_get_backing_file(&lc);

		যদি (!backing || strcmp(backing, filename) != 0) {
			মুক্ত(backing);
			চালু;
		}

		মুক্ত(backing);
		যদি (loopdev && গুনতি == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		গুনতি++;
	}

	loopcxt_deinit(&lc);

	যদি (loopdev && গুনতি > 1) {
		মুক্ত(*loopdev);
		*loopdev = ফাঁকা;
	}
	ফেরত গুনতি;
}

#ifdef TEST_PROGRAM_LOOPDEV
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	যদি (argc < 2)
		যাও usage;

	যদি (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	অন্যথা
		যাও usage;

	ফেরত EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ফেরত EXIT_FAILURE;
}
#endif

