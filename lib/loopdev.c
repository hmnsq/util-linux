
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <dirent.h>

#include "linux_version.h"
#include "c.h"
#include "timeutils.h"
#include "sysfs.h"
#include "pathnames.h"
#include "loopdev.h"
#include "canonicalize.h"
#include "blkdev.h"
#include "debug.h"
#include "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
static UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

static void loopdev_init_debug(void)
{
	ប្រសិនបើ (loopdev_debug_mask)
		ត្រឡប់;
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
		ចំនួនគត់ _c = 0, _e;					\
		do {						\
			errno = 0;				\
			_e = x;					\
			ប្រសិនបើ (_e == 0 || errno != EAGAIN)		\
				bha_nga;				\
			ប្រសិនបើ (_c >= LOOPDEV_MAX_TRIES)		\
				bha_nga;				\
			xusleep(250000);			\
			_c++;					\
		} ខណៈពេល (1);					\
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
ចំនួនគត់ loopcxt_set_device(sa_nracanaa loopdev_cxt *lc, const var_na *device)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (lc->fd >= 0) {
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
	memset(&lc->config, 0, parimaa_na(lc->config));

	/* set new */
	ប្រសិនបើ (device) {
		ប្រសិនបើ (*device != '/') {
			const var_na *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ប្រសិនបើ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ប្រសិនបើ (strlen(device) < 5)
					ត្រឡប់ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, parimaa_na(lc->device), "%s%s",
				dir, device);
		} បើមិនដូច្នេះ
			xstrncpy(lc->device, device, parimaa_na(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = NULL;
	ត្រឡប់ 0;
}

ចំនួនគត់ loopcxt_has_device(sa_nracanaa loopdev_cxt *lc)
{
	ត្រឡប់ lc && *lc->device;
}

dev_t loopcxt_get_devno(sa_nracanaa loopdev_cxt *lc)
{
	ប្រសិនបើ (!lc || !loopcxt_has_device(lc))
		ត្រឡប់ 0;
	ប្រសិនបើ (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ត្រឡប់ lc->devno;
}

ចំនួនគត់ loopcxt_is_lost(sa_nracanaa loopdev_cxt *lc)
{
	ប្រសិនបើ (!lc || !loopcxt_has_device(lc))
		ត្រឡប់ 0;
	ប្រសិនបើ (lc->is_lost)
		ត្រឡប់ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ត្រឡប់ lc->is_lost;
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
ចំនួនគត់ loopcxt_init(sa_nracanaa loopdev_cxt *lc, ចំនួនគត់ flags)
{
	ចំនួនគត់ rc;
	sa_nracanaa stat st;
	sa_nracanaa loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, parimaa_na(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, NULL);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	ប្រសិនបើ (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ប្រសិនបើ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ប្រសិនបើ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ត្រឡប់ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
void loopcxt_deinit(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ errsv = errno;

	ប្រសិនបើ (!lc)
		ត្រឡប់;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	free(lc->filename);
	lc->filename = NULL;

	ignore_result( loopcxt_set_device(lc, NULL) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
var_na *loopcxt_strdup_device(sa_nracanaa loopdev_cxt *lc)
{
	ប្រសិនបើ (!lc || !*lc->device)
		ត្រឡប់ NULL;
	ត្រឡប់ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
const var_na *loopcxt_get_device(sa_nracanaa loopdev_cxt *lc)
{
	ត្រឡប់ lc && *lc->device ? lc->device : NULL;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
static sa_nracanaa path_cxt *loopcxt_get_sysfs(sa_nracanaa loopdev_cxt *lc)
{
	ប្រសិនបើ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ត្រឡប់ NULL;

	ប្រសិនបើ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ប្រសិនបើ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ត្រឡប់ NULL;
		}

		lc->sysfs = ul_new_sysfs_path(devno, NULL, NULL);
		ប្រសិនបើ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ត្រឡប់ lc->sysfs;
}

static ចំនួនគត់ __loopcxt_get_fd(sa_nracanaa loopdev_cxt *lc, mode_t mode)
{
	ចំនួនគត់ old = -1;

	ប្រសិនបើ (!lc || !*lc->device)
		ត្រឡប់ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ប្រសិនបើ (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ប្រសិនបើ (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		ប្រសិនបើ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ប្រសិនបើ (old >= 0)
		close(old);
	ត្រឡប់ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ចំនួនគត់ loopcxt_get_fd(sa_nracanaa loopdev_cxt *lc)
{
	ត្រឡប់ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_init_iterator(sa_nracanaa loopdev_cxt *lc, ចំនួនគត់ flags)
{
	sa_nracanaa loopdev_iter *iter;
	sa_nracanaa stat st;

	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, parimaa_na(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ប្រសិនបើ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ប្រសិនបើ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ត្រឡប់ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_deinit_iterator(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter;

	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	free(iter->minors);
	ប្រសិនបើ (iter->proc)
		fclose(iter->proc);
	ប្រសិនបើ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, parimaa_na(*iter));
	ត្រឡប់ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
static ចំនួនគត់ loopiter_set_device(sa_nracanaa loopdev_cxt *lc, const var_na *device)
{
	ចំនួនគត់ rc = loopcxt_set_device(lc, device);
	ចំនួនគត់ used;

	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	ប្រសិនបើ (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ត្រឡប់ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, NULL) == 0;

	ប្រសិនបើ ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ត្រឡប់ 0;

	ប្រសិនបើ ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ត្រឡប់ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, NULL) );
	ត្រឡប់ 1;
}

static ចំនួនគត់ cmpnum(const void *p1, const void *p2)
{
	ត្រឡប់ (((* (const ចំនួនគត់ *) p1) > (* (const ចំនួនគត់ *) p2)) -
			((* (const ចំនួនគត់ *) p1) < (* (const ចំនួនគត់ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
static ចំនួនគត់ loop_scandir(const var_na *dirname, ចំនួនគត់ **ary, ចំនួនគត់ hasprefix)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	unsigned ចំនួនគត់ n, count = 0, arylen = 0;

	ប្រសិនបើ (!dirname || !ary)
		ត្រឡប់ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ប្រសិនបើ (!dir)
		ត្រឡប់ 0;
	free(*ary);
	*ary = NULL;

	ខណៈពេល((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		ប្រសិនបើ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			continue;
#endif
		ប្រសិនបើ (is_dotdir_dirent(d))
			continue;

		ប្រសិនបើ (hasprefix) {
			/* /dev/loop<N> */
			ប្រសិនបើ (sscanf(d->d_name, "loop%u", &n) != 1)
				continue;
		} បើមិនដូច្នេះ {
			/* /dev/loop/<N> */
			var_na *end = NULL;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			ប្រសិនបើ (d->d_name == end || (end && *end) || errno)
				continue;
		}
		ប្រសិនបើ (n < LOOPDEV_DEFAULT_NNODES)
			continue;			/* ignore loop<0..7> */

		ប្រសិនបើ (count + 1 > arylen) {
			ចំនួនគត់ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, parimaa_na(ចំនួនគត់));
			ប្រសិនបើ (!tmp) {
				free(*ary);
				*ary = NULL;
				closedir(dir);
				ត្រឡប់ -1;
			}
			*ary = tmp;
		}
		ប្រសិនបើ (*ary)
			(*ary)[count++] = n;
	}
	ប្រសិនបើ (count && *ary)
		qsort(*ary, count, parimaa_na(ចំនួនគត់), cmpnum);

	closedir(dir);
	ត្រឡប់ count;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
static ចំនួនគត់ loopcxt_next_from_proc(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter = &lc->iter;
	var_na buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ប្រសិនបើ (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ប្រសិនបើ (!iter->proc)
		ត្រឡប់ 1;

	ខណៈពេល (fgets(buf, parimaa_na(buf), iter->proc)) {
		unsigned ចំនួនគត់ m;
		var_na name[128 + 1];


		ប្រសិនបើ (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			continue;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ប្រសិនបើ (loopiter_set_device(lc, name) == 0)
			ត្រឡប់ 0;
	}

	ត្រឡប់ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
static ចំនួនគត់ loopcxt_next_from_sysfs(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter = &lc->iter;
	sa_nracanaa dirent *d;
	ចំនួនគត់ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ប្រសិនបើ (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	ប្រសិនបើ (!iter->sysblock)
		ត្រឡប់ 1;

	fd = dirfd(iter->sysblock);

	ខណៈពេល ((d = readdir(iter->sysblock))) {
		var_na name[NAME_MAX + 18 + 1];
		sa_nracanaa stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ប្រសិនបើ (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			continue;

		snprintf(name, parimaa_na(name), "%s/loop/backing_file", d->d_name);
		ប្រសិនបើ (fstatat(fd, name, &st, 0) != 0)
			continue;

		ប្រសិនបើ (loopiter_set_device(lc, d->d_name) == 0)
			ត្រឡប់ 0;
	}

	ត្រឡប់ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ចំនួនគត់ loopcxt_next(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter;

	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;


	iter = &lc->iter;
	ប្រសិនបើ (iter->done)
		ត្រឡប់ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ប្រសិនបើ (iter->flags & LOOPITER_FL_USED) {
		ចំនួនគត់ rc;

		ប្រសិនបើ (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		បើមិនដូច្នេះ
			rc = loopcxt_next_from_proc(lc);
		ប្រសិនបើ (rc == 0)
			ត្រឡប់ 0;
		goto done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ប្រសិនបើ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		សម្រាប់ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			var_na name[16];
			snprintf(name, parimaa_na(name), "loop%d", iter->ncur);

			ប្រសិនបើ (loopiter_set_device(lc, name) == 0)
				ត្រឡប់ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ប្រសិនបើ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	សម្រាប់ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		var_na name[16];
		snprintf(name, parimaa_na(name), "loop%d", iter->minors[iter->ncur]);

		ប្រសិនបើ (loopiter_set_device(lc, name) == 0)
			ត្រឡប់ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ត្រឡប់ 1;
}

/*
 * @device: path to device
 */
ចំនួនគត់ is_loopdev(const var_na *device)
{
	sa_nracanaa stat st;
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	បើមិនដូច្នេះ ប្រសិនបើ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	បើមិនដូច្នេះ ប្រសិនបើ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		var_na name[PATH_MAX], *cn, *p = NULL;

		snprintf(name, parimaa_na(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		ប្រសិនបើ (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		free(cn);
	}

	ប្រសិនបើ (rc == 0)
		errno = ENODEV;
	ត្រឡប់ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
sa_nracanaa loop_info64 *loopcxt_get_info(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ fd;

	ប្រសិនបើ (!lc || lc->info_failed) {
		errno = EINVAL;
		ត្រឡប់ NULL;
	}
	errno = 0;
	ប្រសិនបើ (lc->has_info)
		ត្រឡប់ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ NULL;

	ប្រសិនបើ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ត្រឡប់ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ត្រឡប់ NULL;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
var_na *loopcxt_get_backing_file(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	var_na *res = NULL;

	ប្រសិនបើ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	ប្រសិនបើ (!res && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);

		ប្រសិនបើ (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((var_na *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ត្រឡប់ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
var_na *loopcxt_get_refname(sa_nracanaa loopdev_cxt *lc)
{
	var_na *res = NULL;
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);

	ប្រសិនបើ (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((var_na *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ត្រឡប់ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_offset(sa_nracanaa loopdev_cxt *lc, uint64_t *offset)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ចំនួនគត់ rc = -EINVAL;

	ប្រសិនបើ (sysfs)
		ប្រសិនបើ (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	ប្រសិនបើ (rc && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		ប្រសិនបើ (lo) {
			ប្រសិនបើ (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} បើមិនដូច្នេះ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t *blocksize)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ចំនួនគត់ rc = -EINVAL;

	ប្រសិនបើ (sysfs)
		ប្រសិនបើ (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ប្រសិនបើ (rc) {
		ចំនួនគត់ fd = loopcxt_get_fd(lc);
		ចំនួនគត់ sz = 0;

		ប្រសិនបើ (fd < 0)
			ត្រឡប់ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		ប្រសិនបើ (rc)
			ត្រឡប់ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_sizelimit(sa_nracanaa loopdev_cxt *lc, uint64_t *size)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ចំនួនគត់ rc = -EINVAL;

	ប្រសិនបើ (sysfs)
		ប្រសិនបើ (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	ប្រសិនបើ (rc && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		ប្រសិនបើ (lo) {
			ប្រសិនបើ (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} បើមិនដូច្នេះ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_encrypt_type(sa_nracanaa loopdev_cxt *lc, uint32_t *type)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ចំនួនគត់ rc;

	/* not provided by sysfs */
	ប្រសិនបើ (lo) {
		ប្រសិនបើ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} បើមិនដូច្នេះ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
const var_na *loopcxt_get_crypt_name(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);

	ប្រសិនបើ (lo)
		ត្រឡប់ (var_na *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ត្រឡប់ NULL;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_backing_devno(sa_nracanaa loopdev_cxt *lc, dev_t *devno)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ចំនួនគត់ rc;

	ប្រសិនបើ (lo) {
		ប្រសិនបើ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} បើមិនដូច្នេះ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ចំនួនគត់ loopcxt_get_backing_inode(sa_nracanaa loopdev_cxt *lc, ino_t *ino)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ចំនួនគត់ rc;

	ប្រសិនបើ (lo) {
		ប្រសិនបើ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} បើមិនដូច្នេះ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ត្រឡប់ rc;
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
ចំនួនគត់ loopmod_supports_partscan(void)
{
	ចំនួនគត់ rc, ret = 0;
	FILE *f;

	ប្រសិនបើ (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ត្រឡប់ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ប្រសិនបើ (!f)
		ត្រឡប់ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ត្រឡប់ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ចំនួនគត់ loopcxt_is_partscan(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ប្រសិនបើ (sysfs) {
		/* kernel >= 3.2 */
		ចំនួនគត់ fl;
		ប្រសិនបើ (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ត្រឡប់ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ត្រឡប់ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ចំនួនគត់ loopcxt_is_autoclear(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ប្រសិនបើ (sysfs) {
		ចំនួនគត់ fl;
		ប្រសិនបើ (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ត្រឡប់ fl;
	}

	ប្រសិនបើ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		ប្រសិនបើ (lo)
			ត្រឡប់ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ត្រឡប់ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ចំនួនគត់ loopcxt_is_readonly(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ប្រសិនបើ (sysfs) {
		ចំនួនគត់ fl;
		ប្រសិនបើ (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ត្រឡប់ fl;
	}

	ប្រសិនបើ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		ប្រសិនបើ (lo)
			ត្រឡប់ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ត្រឡប់ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ចំនួនគត់ loopcxt_is_dio(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ប្រសិនបើ (sysfs) {
		ចំនួនគត់ fl;
		ប្រសិនបើ (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ត្រឡប់ fl;
	}
	ប្រសិនបើ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		ប្រសិនបើ (lo)
			ត្រឡប់ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ត្រឡប់ 0;
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
ចំនួនគត់ loopcxt_is_used(sa_nracanaa loopdev_cxt *lc,
		    sa_nracanaa stat *st,
		    const var_na *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    ចំនួនគត់ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ប្រសិនបើ (!lc)
		ត្រឡប់ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ប្រសិនបើ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ប្រសិនបើ (ino == st->st_ino && dev == st->st_dev)
			goto found;

		/* don't use filename if we have devno and inode */
		ត្រឡប់ 0;
	}

	/* poor man's solution */
	ប្រសិនបើ (backing_file) {
		var_na *name = loopcxt_get_backing_file(lc);
		ចំនួនគត់ rc = name && strcmp(name, backing_file) == 0;

		free(name);
		ប្រសិនបើ (rc)
			goto found;
	}

	ត្រឡប់ 0;
found:
	ប្រសិនបើ (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		ចំនួនគត់ rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		ប្រសិនបើ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ត្រឡប់ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ត្រឡប់ rc;
	}
	ត្រឡប់ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ចំនួនគត់ loopcxt_set_offset(sa_nracanaa loopdev_cxt *lc, uint64_t offset)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ត្រឡប់ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ចំនួនគត់ loopcxt_set_sizelimit(sa_nracanaa loopdev_cxt *lc, uint64_t sizelimit)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ត្រឡប់ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ចំនួនគត់ loopcxt_set_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t blocksize)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ត្រឡប់ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ចំនួនគត់ loopcxt_set_flags(sa_nracanaa loopdev_cxt *lc, uint32_t flags)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ត្រឡប់ 0;
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
ចំនួនគត់ loopcxt_set_refname(sa_nracanaa loopdev_cxt *lc, const var_na *refname)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, parimaa_na(lc->config.info.lo_file_name));
	ប្រសិនបើ (refname)
		xstrncpy((var_na *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (var_na *)lc->config.info.lo_file_name));
	ត្រឡប់ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ចំនួនគត់ loopcxt_set_backing_file(sa_nracanaa loopdev_cxt *lc, const var_na *filename)
{
	ប្រសិនបើ (!lc)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	បើមិនដូច្នេះ
		lc->filename = ul_canonicalize_path(filename);
	ប្រសិនបើ (!lc->filename)
		ត្រឡប់ -errno;

	ប្រសិនបើ (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ត្រឡប់ 0;
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
static ចំនួនគត់ loopcxt_check_size(sa_nracanaa loopdev_cxt *lc, ចំនួនគត់ file_fd)
{
	uint64_t size, expected_size;
	ចំនួនគត់ dev_fd;
	sa_nracanaa stat st;

	ប្រសិនបើ (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ត្រឡប់ 0;

	ប្រសិនបើ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ត្រឡប់ -errno;
	}
	ប្រសិនបើ (S_ISBLK(st.st_mode)) {
		ប្រសិនបើ (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ត្រឡប់ -errno;
		}
	} បើមិនដូច្នេះ
		expected_size = st.st_size;

	ប្រសិនបើ (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ត្រឡប់ 0;	/* ignore this error */
	}

	ប្រសិនបើ (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	ប្រសិនបើ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ប្រសិនបើ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ត្រឡប់ -errno;
	}

	ប្រសិនបើ (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ត្រឡប់ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ប្រសិនបើ (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	ប្រសិនបើ (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		ប្រសិនបើ (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			ប្រសិនបើ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ត្រឡប់ -errno;
		}

		ប្រសិនបើ (blkdev_get_size(dev_fd, &size))
			ត្រឡប់ -errno;

		ប្រសិនបើ (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ត្រឡប់ -errno;
		}
	}

	ត្រឡប់ 0;
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
ចំនួនគត់ loopcxt_setup_device(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	ចំនួនគត់ rc = -1, cnt = 0;
	ចំនួនគត់ errsv = 0;
	ចំនួនគត់ fallback = 0;

	ប្រសិនបើ (!lc || !*lc->device || !lc->filename)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ប្រសិនបើ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	ប្រសិនបើ (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	ប្រសិនបើ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	បើមិនដូច្នេះ
		file_fd = open(lc->filename, mode | flags);

	ប្រសិនបើ (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		ប្រសិនបើ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		បើមិនដូច្នេះ
			file_fd = open(lc->filename, mode | flags);
	}
	ប្រសិនបើ (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ត្រឡប់ -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	ប្រសិនបើ (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	បើមិនដូច្នេះ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	do {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		ប្រសិនបើ (dev_fd >= 0 || lc->control_ok == 0)
			bha_nga;
		ប្រសិនបើ (errno != EACCES && errno != ENOENT)
			bha_nga;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ខណៈពេល (cnt++ < 16);

	ប្រសិនបើ (dev_fd < 0) {
		rc = -errno;
		goto err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ប្រសិនបើ (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ប្រសិនបើ (rc != 0) {
		errsv = errno;
		ប្រសិនបើ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			goto err;
		}
		fallback = 1;
	} បើមិនដូច្នេះ {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ប្រសិនបើ (fallback) {
		ប្រសិនបើ (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			goto err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		ប្រសិនបើ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			goto err;
		}

		ប្រសិនបើ ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			goto err;
		}
	}

	ប្រសិនបើ ((rc = loopcxt_check_size(lc, file_fd)))
		goto err;

	close(file_fd);

	memset(&lc->config, 0, parimaa_na(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ត្រឡប់ 0;
err:
	ប្រសិនបើ (file_fd >= 0)
		close(file_fd);
	ប្រសិនបើ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ប្រសិនបើ (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ត្រឡប់ rc;
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
ចំនួនគត់ loopcxt_ioctl_status(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ប្រសិនបើ (dev_fd < 0)
		ត្រឡប់ -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	ប្រសិនបើ (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ត្រឡប់ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ត្រឡប់ 0;
}

ចំនួនគត់ loopcxt_ioctl_capacity(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ rc, fd = loopcxt_get_fd(lc);

	ប្រសិនបើ (fd < 0)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	ប្រសិនបើ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ត្រឡប់ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ត្រឡប់ 0;
}

ចំនួនគត់ loopcxt_ioctl_dio(sa_nracanaa loopdev_cxt *lc, unsigned long use_dio)
{
	ចំនួនគត់ rc, fd = loopcxt_get_fd(lc);

	ប្រសិនបើ (fd < 0)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	ប្រសិនបើ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ត្រឡប់ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ត្រឡប់ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ចំនួនគត់ loopcxt_ioctl_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t blocksize)
{
	ចំនួនគត់ rc, fd = loopcxt_get_fd(lc);

	ប្រសិនបើ (fd < 0)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	ប្រសិនបើ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ត្រឡប់ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ត្រឡប់ 0;
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
static ចំនួនគត់ loopcxt_get_device_nr(sa_nracanaa loopdev_cxt *lc, ចំនួនគត់ *nr)
{
	const var_na *p, *dev;
	ចំនួនគត់ rc = -EINVAL;

	errno = 0;
	ប្រសិនបើ (!lc || !nr)
		ត្រឡប់ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ប្រសិនបើ (!dev)
		goto done;

	p = strrchr(dev, '/');
	ប្រសិនបើ (!p)
		goto done;

	ប្រសិនបើ (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		goto done;

	ប្រសិនបើ (*nr < 0)
		goto done;
	rc = 0;
done:
	ប្រសិនបើ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ត្រឡប់ rc;
}

ចំនួនគត់ loopcxt_detach_device(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ rc, fd = loopcxt_get_fd(lc);

	ប្រសិនបើ (fd < 0)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ប្រសិនបើ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ត្រឡប់ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ត្រឡប់ 0;
}

ចំនួនគត់ loopcxt_remove_device(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ rc = -EINVAL;
	ចំនួនគត់ ctl, nr = -1;

	ប្រសិនបើ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ប្រសិនបើ (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ប្រសិនបើ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ត្រឡប់ rc;
}

ចំនួនគត់ loopcxt_add_device(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ rc = -EINVAL;
	ចំនួនគត់ ctl, nr = -1;

	ប្រសិនបើ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ប្រសិនបើ (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ប្រសិនបើ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ចំនួនគត់ loopcxt_find_unused(sa_nracanaa loopdev_cxt *lc)
{
	ចំនួនគត់ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ប្រសិនបើ (lc->flags & LOOPDEV_FL_CONTROL) {
		ចំនួនគត់ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ប្រសិនបើ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		បើមិនដូច្នេះ
			rc = -errno;
		ប្រសិនបើ (rc >= 0) {
			var_na name[16];
			snprintf(name, parimaa_na(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ប្រសិនបើ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ប្រសិនបើ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ប្រសិនបើ (rc)
			ត្រឡប់ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ប្រសិនបើ (rc)
			ត្រឡប់ -ENOENT;
	}
	ត្រឡប់ rc;
}



/*
 * Return: TRUE/FALSE
 */
ចំនួនគត់ loopdev_is_autoclear(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	ចំនួនគត់ rc;

	ប្រសិនបើ (!device)
		ត្រឡប់ 0;

	rc = loopcxt_init(&lc, 0);
	ប្រសិនបើ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ប្រសិនបើ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ត្រឡប់ rc;
}

var_na *loopdev_get_backing_file(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	var_na *res = NULL;

	ប្រសិនបើ (!device)
		ត្រឡប់ NULL;
	ប្រសិនបើ (loopcxt_init(&lc, 0))
		ត្រឡប់ NULL;
	ប្រសិនបើ (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ត្រឡប់ res;
}

/*
 * Returns: TRUE/FALSE
 */
ចំនួនគត់ loopdev_has_backing_file(const var_na *device)
{
	var_na *tmp = loopdev_get_backing_file(device);

	ប្រសិនបើ (tmp) {
		free(tmp);
		ត្រឡប់ 1;
	}
	ត្រឡប់ 0;
}

/*
 * Returns: TRUE/FALSE
 */
ចំនួនគត់ loopdev_is_used(const var_na *device, const var_na *filename,
		    uint64_t offset, uint64_t sizelimit, ចំនួនគត់ flags)
{
	sa_nracanaa loopdev_cxt lc;
	sa_nracanaa stat st;
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (!device || !filename)
		ត្រឡប់ 0;

	rc = loopcxt_init(&lc, 0);
	ប្រសិនបើ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : NULL, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ត្រឡប់ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ចំនួនគត់ loopdev_detach(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	ចំនួនគត់ rc;

	ប្រសិនបើ (!device)
		ត្រឡប់ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ប្រសិនបើ (!rc)
		rc = loopcxt_set_device(&lc, device);
	ប្រសិនបើ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ត្រឡប់ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ចំនួនគត់ loopcxt_find_by_backing_file(sa_nracanaa loopdev_cxt *lc, const var_na *filename,
				 uint64_t offset, uint64_t sizelimit, ចំនួនគត់ flags)
{
	ចំនួនគត់ rc, hasst;
	sa_nracanaa stat st;

	ប្រសិនបើ (!filename)
		ត្រឡប់ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	ខណៈពេល ((rc = loopcxt_next(lc)) == 0) {

		ប្រសិនបើ (loopcxt_is_used(lc, hasst ? &st : NULL,
				    filename, offset, sizelimit, flags))
			bha_nga;
	}

	loopcxt_deinit_iterator(lc);
	ត្រឡប់ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ចំនួនគត់ loopcxt_find_overlap(sa_nracanaa loopdev_cxt *lc, const var_na *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	ចំនួនគត់ rc, hasst;
	sa_nracanaa stat st;

	ប្រសិនបើ (!filename)
		ត្រឡប់ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	ខណៈពេល ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : NULL,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ប្រសិនបើ (rc <= 0)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		ប្រសិនបើ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			continue;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ប្រសិនបើ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			continue;
		}

		/* full match */
		ប្រសិនបើ (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			goto found;
		}

		/* overlap */
		ប្រសិនបើ (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			continue;
		ប្រសិនបើ (sizelimit != 0 && offset + sizelimit <= lc_offset)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			goto found;
	}

	ប្រសិនបើ (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ត្រឡប់ rc;
}

/*
 * Returns allocated string with device name
 */
var_na *loopdev_find_by_backing_file(const var_na *filename, uint64_t offset, uint64_t sizelimit, ចំនួនគត់ flags)
{
	sa_nracanaa loopdev_cxt lc;
	var_na *res = NULL;

	ប្រសិនបើ (!filename)
		ត្រឡប់ NULL;

	ប្រសិនបើ (loopcxt_init(&lc, 0))
		ត្រឡប់ NULL;
	ប្រសិនបើ (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ត្រឡប់ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ចំនួនគត់ loopdev_count_by_backing_file(const var_na *filename, var_na **loopdev)
{
	sa_nracanaa loopdev_cxt lc;
	ចំនួនគត់ count = 0, rc;

	ប្រសិនបើ (!filename)
		ត្រឡប់ -1;

	rc = loopcxt_init(&lc, 0);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;
	ប្រសិនបើ (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ត្រឡប់ -1;

	ខណៈពេល(loopcxt_next(&lc) == 0) {
		var_na *backing = loopcxt_get_backing_file(&lc);

		ប្រសិនបើ (!backing || strcmp(backing, filename) != 0) {
			free(backing);
			continue;
		}

		free(backing);
		ប្រសិនបើ (loopdev && count == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		count++;
	}

	loopcxt_deinit(&lc);

	ប្រសិនបើ (loopdev && count > 1) {
		free(*loopdev);
		*loopdev = NULL;
	}
	ត្រឡប់ count;
}

#ifdef TEST_PROGRAM_LOOPDEV
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	ប្រសិនបើ (argc < 2)
		goto usage;

	ប្រសិនបើ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	បើមិនដូច្នេះ
		goto usage;

	ត្រឡប់ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ត្រឡប់ EXIT_FAILURE;
}
#endif

