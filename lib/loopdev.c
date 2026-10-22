
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
#شامل <stdio.h>
#شامل <stdint.h>
#شامل <string.h>
#شامل <ctype.h>
#شامل <fcntl.h>
#شامل <stdlib.h>
#شامل <unistd.h>
#شامل <sys/ioctl.h>
#شامل <sys/stat.h>
#شامل <sys/mman.h>
#شامل <inttypes.h>
#شامل <dirent.h>

#شامل "linux_version.h"
#شامل "c.h"
#شامل "timeutils.h"
#شامل "sysfs.h"
#شامل "pathnames.h"
#شامل "loopdev.h"
#شامل "canonicalize.h"
#شامل "blkdev.h"
#شامل "debug.h"
#شامل "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ایستا UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ایستا تهی loopdev_init_debug(تهی)
{
	اگر (loopdev_debug_mask)
		بازگشت;
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
		صحیح _c = 0, _e;					\
		انجامبده {						\
			errno = 0;				\
			_e = x;					\
			اگر (_e == 0 || errno != EAGAIN)		\
				بشکن;				\
			اگر (_c >= LOOPDEV_MAX_TRIES)		\
				بشکن;				\
			xusleep(250000);			\
			_c++;					\
		} وقتی (1);					\
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
صحیح loopcxt_set_device(ساختار loopdev_cxt *lc, ثابت نویسه *device)
{
	اگر (!lc)
		بازگشت -EINVAL;

	اگر (lc->fd >= 0) {
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
	memset(&lc->config, 0, اندازه(lc->config));

	/* set new */
	اگر (device) {
		اگر (*device != '/') {
			ثابت نویسه *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			اگر (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				اگر (strlen(device) < 5)
					بازگشت -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, اندازه(lc->device), "%s%s",
				dir, device);
		} وگرنه
			xstrncpy(lc->device, device, اندازه(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = پوچ;
	بازگشت 0;
}

صحیح loopcxt_has_device(ساختار loopdev_cxt *lc)
{
	بازگشت lc && *lc->device;
}

dev_t loopcxt_get_devno(ساختار loopdev_cxt *lc)
{
	اگر (!lc || !loopcxt_has_device(lc))
		بازگشت 0;
	اگر (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	بازگشت lc->devno;
}

صحیح loopcxt_is_lost(ساختار loopdev_cxt *lc)
{
	اگر (!lc || !loopcxt_has_device(lc))
		بازگشت 0;
	اگر (lc->is_lost)
		بازگشت 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	بازگشت lc->is_lost;
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
صحیح loopcxt_init(ساختار loopdev_cxt *lc, صحیح flags)
{
	صحیح rc;
	ساختار stat st;
	ساختار loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	اگر (!lc)
		بازگشت -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, اندازه(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, پوچ);
	اگر (rc)
		بازگشت rc;

	اگر (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	اگر (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	بازگشت 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
تهی loopcxt_deinit(ساختار loopdev_cxt *lc)
{
	صحیح errsv = errno;

	اگر (!lc)
		بازگشت;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	آزاد(lc->filename);
	lc->filename = پوچ;

	ignore_result( loopcxt_set_device(lc, پوچ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
نویسه *loopcxt_strdup_device(ساختار loopdev_cxt *lc)
{
	اگر (!lc || !*lc->device)
		بازگشت پوچ;
	بازگشت strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ثابت نویسه *loopcxt_get_device(ساختار loopdev_cxt *lc)
{
	بازگشت lc && *lc->device ? lc->device : پوچ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ایستا ساختار path_cxt *loopcxt_get_sysfs(ساختار loopdev_cxt *lc)
{
	اگر (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		بازگشت پوچ;

	اگر (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		اگر (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			بازگشت پوچ;
		}

		lc->sysfs = ul_new_sysfs_path(devno, پوچ, پوچ);
		اگر (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	بازگشت lc->sysfs;
}

ایستا صحیح __loopcxt_get_fd(ساختار loopdev_cxt *lc, mode_t mode)
{
	صحیح old = -1;

	اگر (!lc || !*lc->device)
		بازگشت -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	اگر (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	اگر (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		اگر (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	اگر (old >= 0)
		close(old);
	بازگشت lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
صحیح loopcxt_get_fd(ساختار loopdev_cxt *lc)
{
	بازگشت __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_init_iterator(ساختار loopdev_cxt *lc, صحیح flags)
{
	ساختار loopdev_iter *iter;
	ساختار stat st;

	اگر (!lc)
		بازگشت -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, اندازه(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	اگر (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		اگر (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	بازگشت 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_deinit_iterator(ساختار loopdev_cxt *lc)
{
	ساختار loopdev_iter *iter;

	اگر (!lc)
		بازگشت -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	آزاد(iter->minors);
	اگر (iter->proc)
		fclose(iter->proc);
	اگر (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, اندازه(*iter));
	بازگشت 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ایستا صحیح loopiter_set_device(ساختار loopdev_cxt *lc, ثابت نویسه *device)
{
	صحیح rc = loopcxt_set_device(lc, device);
	صحیح used;

	اگر (rc)
		بازگشت rc;

	اگر (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		بازگشت 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, پوچ) == 0;

	اگر ((lc->iter.flags & LOOPITER_FL_USED) && used)
		بازگشت 0;

	اگر ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		بازگشت 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, پوچ) );
	بازگشت 1;
}

ایستا صحیح cmpnum(ثابت تهی *p1, ثابت تهی *p2)
{
	بازگشت (((* (ثابت صحیح *) p1) > (* (ثابت صحیح *) p2)) -
			((* (ثابت صحیح *) p1) < (* (ثابت صحیح *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ایستا صحیح loop_scandir(ثابت نویسه *dirname, صحیح **ary, صحیح hasprefix)
{
	DIR *dir;
	ساختار dirent *d;
	unsigned صحیح n, شمار = 0, arylen = 0;

	اگر (!dirname || !ary)
		بازگشت 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	اگر (!dir)
		بازگشت 0;
	آزاد(*ary);
	*ary = پوچ;

	وقتی((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ادامه;
#endif
		اگر (is_dotdir_dirent(d))
			ادامه;

		اگر (hasprefix) {
			/* /dev/loop<N> */
			اگر (sscanf(d->d_name, "loop%u", &n) != 1)
				ادامه;
		} وگرنه {
			/* /dev/loop/<N> */
			نویسه *end = پوچ;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			اگر (d->d_name == end || (end && *end) || errno)
				ادامه;
		}
		اگر (n < LOOPDEV_DEFAULT_NNODES)
			ادامه;			/* ignore loop<0..7> */

		اگر (شمار + 1 > arylen) {
			صحیح *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, اندازه(صحیح));
			اگر (!tmp) {
				آزاد(*ary);
				*ary = پوچ;
				closedir(dir);
				بازگشت -1;
			}
			*ary = tmp;
		}
		اگر (*ary)
			(*ary)[شمار++] = n;
	}
	اگر (شمار && *ary)
		qsort(*ary, شمار, اندازه(صحیح), cmpnum);

	closedir(dir);
	بازگشت شمار;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ایستا صحیح loopcxt_next_from_proc(ساختار loopdev_cxt *lc)
{
	ساختار loopdev_iter *iter = &lc->iter;
	نویسه buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	اگر (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	اگر (!iter->proc)
		بازگشت 1;

	وقتی (fgets(buf, اندازه(buf), iter->proc)) {
		unsigned صحیح m;
		نویسه name[128 + 1];


		اگر (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ادامه;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		اگر (loopiter_set_device(lc, name) == 0)
			بازگشت 0;
	}

	بازگشت 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ایستا صحیح loopcxt_next_from_sysfs(ساختار loopdev_cxt *lc)
{
	ساختار loopdev_iter *iter = &lc->iter;
	ساختار dirent *d;
	صحیح fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	اگر (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	اگر (!iter->sysblock)
		بازگشت 1;

	fd = dirfd(iter->sysblock);

	وقتی ((d = readdir(iter->sysblock))) {
		نویسه name[NAME_MAX + 18 + 1];
		ساختار stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		اگر (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			ادامه;

		snprintf(name, اندازه(name), "%s/loop/backing_file", d->d_name);
		اگر (fstatat(fd, name, &st, 0) != 0)
			ادامه;

		اگر (loopiter_set_device(lc, d->d_name) == 0)
			بازگشت 0;
	}

	بازگشت 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
صحیح loopcxt_next(ساختار loopdev_cxt *lc)
{
	ساختار loopdev_iter *iter;

	اگر (!lc)
		بازگشت -EINVAL;


	iter = &lc->iter;
	اگر (iter->done)
		بازگشت 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	اگر (iter->flags & LOOPITER_FL_USED) {
		صحیح rc;

		اگر (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		وگرنه
			rc = loopcxt_next_from_proc(lc);
		اگر (rc == 0)
			بازگشت 0;
		برو done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	اگر (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		برای (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			نویسه name[16];
			snprintf(name, اندازه(name), "loop%d", iter->ncur);

			اگر (loopiter_set_device(lc, name) == 0)
				بازگشت 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	اگر (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	برای (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		نویسه name[16];
		snprintf(name, اندازه(name), "loop%d", iter->minors[iter->ncur]);

		اگر (loopiter_set_device(lc, name) == 0)
			بازگشت 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	بازگشت 1;
}

/*
 * @device: path to device
 */
صحیح is_loopdev(ثابت نویسه *device)
{
	ساختار stat st;
	صحیح rc = 0;

	اگر (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	وگرنه اگر (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	وگرنه اگر (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		نویسه name[PATH_MAX], *cn, *p = پوچ;

		snprintf(name, اندازه(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		اگر (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		آزاد(cn);
	}

	اگر (rc == 0)
		errno = ENODEV;
	بازگشت rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ساختار loop_info64 *loopcxt_get_info(ساختار loopdev_cxt *lc)
{
	صحیح fd;

	اگر (!lc || lc->info_failed) {
		errno = EINVAL;
		بازگشت پوچ;
	}
	errno = 0;
	اگر (lc->has_info)
		بازگشت &lc->config.info;

	fd = loopcxt_get_fd(lc);
	اگر (fd < 0)
		بازگشت پوچ;

	اگر (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		بازگشت &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	بازگشت پوچ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
نویسه *loopcxt_get_backing_file(ساختار loopdev_cxt *lc)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);
	نویسه *res = پوچ;

	اگر (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	اگر (!res && loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);

		اگر (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((نویسه *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	بازگشت res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
نویسه *loopcxt_get_refname(ساختار loopdev_cxt *lc)
{
	نویسه *res = پوچ;
	ساختار loop_info64 *lo = loopcxt_get_info(lc);

	اگر (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((نویسه *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	بازگشت res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_offset(ساختار loopdev_cxt *lc, uint64_t *offset)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	اگر (rc && loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo) {
			اگر (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} وگرنه
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	بازگشت rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_blocksize(ساختار loopdev_cxt *lc, uint64_t *blocksize)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	اگر (rc) {
		صحیح fd = loopcxt_get_fd(lc);
		صحیح sz = 0;

		اگر (fd < 0)
			بازگشت -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		اگر (rc)
			بازگشت rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	بازگشت rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_sizelimit(ساختار loopdev_cxt *lc, uint64_t *size)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	اگر (rc && loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo) {
			اگر (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} وگرنه
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	بازگشت rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_encrypt_type(ساختار loopdev_cxt *lc, uint32_t *type)
{
	ساختار loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	/* not provided by sysfs */
	اگر (lo) {
		اگر (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} وگرنه
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	بازگشت rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ثابت نویسه *loopcxt_get_crypt_name(ساختار loopdev_cxt *lc)
{
	ساختار loop_info64 *lo = loopcxt_get_info(lc);

	اگر (lo)
		بازگشت (نویسه *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	بازگشت پوچ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_backing_devno(ساختار loopdev_cxt *lc, dev_t *devno)
{
	ساختار loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	اگر (lo) {
		اگر (devno)
			*devno = lo->lo_device;
		rc = 0;
	} وگرنه
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	بازگشت rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_backing_inode(ساختار loopdev_cxt *lc, ino_t *ino)
{
	ساختار loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	اگر (lo) {
		اگر (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} وگرنه
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	بازگشت rc;
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
صحیح loopmod_supports_partscan(تهی)
{
	صحیح rc, ret = 0;
	FILE *f;

	اگر (get_linux_version() >= KERNEL_VERSION(3,2,0))
		بازگشت 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	اگر (!f)
		بازگشت 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	بازگشت rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
صحیح loopcxt_is_partscan(ساختار loopdev_cxt *lc)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		/* kernel >= 3.2 */
		صحیح fl;
		اگر (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			بازگشت fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	بازگشت loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
صحیح loopcxt_is_autoclear(ساختار loopdev_cxt *lc)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			بازگشت fl;
	}

	اگر (loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			بازگشت lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	بازگشت 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
صحیح loopcxt_is_readonly(ساختار loopdev_cxt *lc)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			بازگشت fl;
	}

	اگر (loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			بازگشت lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	بازگشت 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
صحیح loopcxt_is_dio(ساختار loopdev_cxt *lc)
{
	ساختار path_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			بازگشت fl;
	}
	اگر (loopcxt_ioctl_enabled(lc)) {
		ساختار loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			بازگشت lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	بازگشت 0;
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
صحیح loopcxt_is_used(ساختار loopdev_cxt *lc,
		    ساختار stat *st,
		    ثابت نویسه *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    صحیح flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	اگر (!lc)
		بازگشت 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	اگر (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		اگر (ino == st->st_ino && dev == st->st_dev)
			برو found;

		/* don't use filename if we have devno and inode */
		بازگشت 0;
	}

	/* poor man's solution */
	اگر (backing_file) {
		نویسه *name = loopcxt_get_backing_file(lc);
		صحیح rc = name && strcmp(name, backing_file) == 0;

		آزاد(name);
		اگر (rc)
			برو found;
	}

	بازگشت 0;
found:
	اگر (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		صحیح rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		اگر (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			بازگشت loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		بازگشت rc;
	}
	بازگشت 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_set_offset(ساختار loopdev_cxt *lc, uint64_t offset)
{
	اگر (!lc)
		بازگشت -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	بازگشت 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_set_sizelimit(ساختار loopdev_cxt *lc, uint64_t sizelimit)
{
	اگر (!lc)
		بازگشت -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	بازگشت 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_set_blocksize(ساختار loopdev_cxt *lc, uint64_t blocksize)
{
	اگر (!lc)
		بازگشت -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	بازگشت 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح loopcxt_set_flags(ساختار loopdev_cxt *lc, uint32_t flags)
{
	اگر (!lc)
		بازگشت -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	بازگشت 0;
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
صحیح loopcxt_set_refname(ساختار loopdev_cxt *lc, ثابت نویسه *refname)
{
	اگر (!lc)
		بازگشت -EINVAL;

	memset(lc->config.info.lo_file_name, 0, اندازه(lc->config.info.lo_file_name));
	اگر (refname)
		xstrncpy((نویسه *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (نویسه *)lc->config.info.lo_file_name));
	بازگشت 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح loopcxt_set_backing_file(ساختار loopdev_cxt *lc, ثابت نویسه *filename)
{
	اگر (!lc)
		بازگشت -EINVAL;

	اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	وگرنه
		lc->filename = ul_canonicalize_path(filename);
	اگر (!lc->filename)
		بازگشت -errno;

	اگر (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	بازگشت 0;
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
ایستا صحیح loopcxt_check_size(ساختار loopdev_cxt *lc, صحیح file_fd)
{
	uint64_t size, expected_size;
	صحیح dev_fd;
	ساختار stat st;

	اگر (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		بازگشت 0;

	اگر (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		بازگشت -errno;
	}
	اگر (S_ISBLK(st.st_mode)) {
		اگر (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			بازگشت -errno;
		}
	} وگرنه
		expected_size = st.st_size;

	اگر (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		بازگشت 0;	/* ignore this error */
	}

	اگر (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	اگر (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	اگر (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		بازگشت -errno;
	}

	اگر (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		بازگشت -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	اگر (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	اگر (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		اگر (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			اگر (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			بازگشت -errno;
		}

		اگر (blkdev_get_size(dev_fd, &size))
			بازگشت -errno;

		اگر (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			بازگشت -errno;
		}
	}

	بازگشت 0;
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
صحیح loopcxt_setup_device(ساختار loopdev_cxt *lc)
{
	صحیح file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	صحیح rc = -1, cnt = 0;
	صحیح errsv = 0;
	صحیح fallback = 0;

	اگر (!lc || !*lc->device || !lc->filename)
		بازگشت -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	اگر (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	اگر (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	وگرنه
		file_fd = open(lc->filename, mode | flags);

	اگر (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		وگرنه
			file_fd = open(lc->filename, mode | flags);
	}
	اگر (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		بازگشت -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	اگر (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	وگرنه
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	انجامبده {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		اگر (dev_fd >= 0 || lc->control_ok == 0)
			بشکن;
		اگر (errno != EACCES && errno != ENOENT)
			بشکن;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} وقتی (cnt++ < 16);

	اگر (dev_fd < 0) {
		rc = -errno;
		برو err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	اگر (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	اگر (rc != 0) {
		errsv = errno;
		اگر (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			برو err;
		}
		fallback = 1;
	} وگرنه {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	اگر (fallback) {
		اگر (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			برو err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		اگر (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			برو err;
		}

		اگر ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			برو err;
		}
	}

	اگر ((rc = loopcxt_check_size(lc, file_fd)))
		برو err;

	close(file_fd);

	memset(&lc->config, 0, اندازه(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	بازگشت 0;
err:
	اگر (file_fd >= 0)
		close(file_fd);
	اگر (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	اگر (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	بازگشت rc;
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
صحیح loopcxt_ioctl_status(ساختار loopdev_cxt *lc)
{
	صحیح dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	اگر (dev_fd < 0)
		بازگشت -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	اگر (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		بازگشت rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	بازگشت 0;
}

صحیح loopcxt_ioctl_capacity(ساختار loopdev_cxt *lc)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		بازگشت -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		بازگشت rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	بازگشت 0;
}

صحیح loopcxt_ioctl_dio(ساختار loopdev_cxt *lc, unsigned long use_dio)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		بازگشت -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		بازگشت rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	بازگشت 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
صحیح loopcxt_ioctl_blocksize(ساختار loopdev_cxt *lc, uint64_t blocksize)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		بازگشت -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		بازگشت rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	بازگشت 0;
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
ایستا صحیح loopcxt_get_device_nr(ساختار loopdev_cxt *lc, صحیح *nr)
{
	ثابت نویسه *p, *dev;
	صحیح rc = -EINVAL;

	errno = 0;
	اگر (!lc || !nr)
		بازگشت rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	اگر (!dev)
		برو done;

	p = strrchr(dev, '/');
	اگر (!p)
		برو done;

	اگر (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		برو done;

	اگر (*nr < 0)
		برو done;
	rc = 0;
done:
	اگر (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	بازگشت rc;
}

صحیح loopcxt_detach_device(ساختار loopdev_cxt *lc)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		بازگشت -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		بازگشت rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	بازگشت 0;
}

صحیح loopcxt_remove_device(ساختار loopdev_cxt *lc)
{
	صحیح rc = -EINVAL;
	صحیح ctl, nr = -1;

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		برو done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	اگر (rc)
		برو done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	اگر (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	بازگشت rc;
}

صحیح loopcxt_add_device(ساختار loopdev_cxt *lc)
{
	صحیح rc = -EINVAL;
	صحیح ctl, nr = -1;

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		برو done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	اگر (rc)
		برو done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	اگر (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	بازگشت rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
صحیح loopcxt_find_unused(ساختار loopdev_cxt *lc)
{
	صحیح rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	اگر (lc->flags & LOOPDEV_FL_CONTROL) {
		صحیح ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		اگر (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		وگرنه
			rc = -errno;
		اگر (rc >= 0) {
			نویسه name[16];
			snprintf(name, اندازه(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		اگر (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	اگر (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		اگر (rc)
			بازگشت rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		اگر (rc)
			بازگشت -ENOENT;
	}
	بازگشت rc;
}



/*
 * Return: TRUE/FALSE
 */
صحیح loopdev_is_autoclear(ثابت نویسه *device)
{
	ساختار loopdev_cxt lc;
	صحیح rc;

	اگر (!device)
		بازگشت 0;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_set_device(&lc, device);
	اگر (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	بازگشت rc;
}

نویسه *loopdev_get_backing_file(ثابت نویسه *device)
{
	ساختار loopdev_cxt lc;
	نویسه *res = پوچ;

	اگر (!device)
		بازگشت پوچ;
	اگر (loopcxt_init(&lc, 0))
		بازگشت پوچ;
	اگر (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	بازگشت res;
}

/*
 * Returns: TRUE/FALSE
 */
صحیح loopdev_has_backing_file(ثابت نویسه *device)
{
	نویسه *tmp = loopdev_get_backing_file(device);

	اگر (tmp) {
		آزاد(tmp);
		بازگشت 1;
	}
	بازگشت 0;
}

/*
 * Returns: TRUE/FALSE
 */
صحیح loopdev_is_used(ثابت نویسه *device, ثابت نویسه *filename,
		    uint64_t offset, uint64_t sizelimit, صحیح flags)
{
	ساختار loopdev_cxt lc;
	ساختار stat st;
	صحیح rc = 0;

	اگر (!device || !filename)
		بازگشت 0;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_set_device(&lc, device);
	اگر (rc)
		بازگشت rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : پوچ, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	بازگشت rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
صحیح loopdev_detach(ثابت نویسه *device)
{
	ساختار loopdev_cxt lc;
	صحیح rc;

	اگر (!device)
		بازگشت -EINVAL;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_set_device(&lc, device);
	اگر (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	بازگشت rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
صحیح loopcxt_find_by_backing_file(ساختار loopdev_cxt *lc, ثابت نویسه *filename,
				 uint64_t offset, uint64_t sizelimit, صحیح flags)
{
	صحیح rc, hasst;
	ساختار stat st;

	اگر (!filename)
		بازگشت -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	اگر (rc)
		بازگشت rc;

	وقتی ((rc = loopcxt_next(lc)) == 0) {

		اگر (loopcxt_is_used(lc, hasst ? &st : پوچ,
				    filename, offset, sizelimit, flags))
			بشکن;
	}

	loopcxt_deinit_iterator(lc);
	بازگشت rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
صحیح loopcxt_find_overlap(ساختار loopdev_cxt *lc, ثابت نویسه *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	صحیح rc, hasst;
	ساختار stat st;

	اگر (!filename)
		بازگشت -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	اگر (rc)
		بازگشت rc;

	وقتی ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : پوچ,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		اگر (rc <= 0)
			ادامه;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		اگر (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ادامه;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		اگر (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ادامه;
		}

		/* full match */
		اگر (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			برو found;
		}

		/* overlap */
		اگر (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			ادامه;
		اگر (sizelimit != 0 && offset + sizelimit <= lc_offset)
			ادامه;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			برو found;
	}

	اگر (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	بازگشت rc;
}

/*
 * Returns allocated string with device name
 */
نویسه *loopdev_find_by_backing_file(ثابت نویسه *filename, uint64_t offset, uint64_t sizelimit, صحیح flags)
{
	ساختار loopdev_cxt lc;
	نویسه *res = پوچ;

	اگر (!filename)
		بازگشت پوچ;

	اگر (loopcxt_init(&lc, 0))
		بازگشت پوچ;
	اگر (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	بازگشت res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
صحیح loopdev_شمار_by_backing_file(ثابت نویسه *filename, نویسه **loopdev)
{
	ساختار loopdev_cxt lc;
	صحیح شمار = 0, rc;

	اگر (!filename)
		بازگشت -1;

	rc = loopcxt_init(&lc, 0);
	اگر (rc)
		بازگشت rc;
	اگر (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		بازگشت -1;

	وقتی(loopcxt_next(&lc) == 0) {
		نویسه *backing = loopcxt_get_backing_file(&lc);

		اگر (!backing || strcmp(backing, filename) != 0) {
			آزاد(backing);
			ادامه;
		}

		آزاد(backing);
		اگر (loopdev && شمار == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		شمار++;
	}

	loopcxt_deinit(&lc);

	اگر (loopdev && شمار > 1) {
		آزاد(*loopdev);
		*loopdev = پوچ;
	}
	بازگشت شمار;
}

#ifdef TEST_PROGRAM_LOOPDEV
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	اگر (argc < 2)
		برو usage;

	اگر (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	وگرنه
		برو usage;

	بازگشت EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	بازگشت EXIT_FAILURE;
}
#endif

