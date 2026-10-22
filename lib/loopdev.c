
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
	གལ་ཏེ (loopdev_debug_mask)
		ཕྱིར;
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
		ཧྲིལ _c = 0, _e;					\
		do {						\
			errno = 0;				\
			_e = x;					\
			གལ་ཏེ (_e == 0 || errno != EAGAIN)		\
				bha_nga;				\
			གལ་ཏེ (_c >= LOOPDEV_MAX_TRIES)		\
				bha_nga;				\
			xusleep(250000);			\
			_c++;					\
		} སྐབས (1);					\
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
ཧྲིལ loopcxt_set_device(sa_nracanaa loopdev_cxt *lc, const var_na *device)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;

	གལ་ཏེ (lc->fd >= 0) {
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
	གལ་ཏེ (device) {
		གལ་ཏེ (*device != '/') {
			const var_na *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			གལ་ཏེ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				གལ་ཏེ (strlen(device) < 5)
					ཕྱིར -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, parimaa_na(lc->device), "%s%s",
				dir, device);
		} དེ་མིན
			xstrncpy(lc->device, device, parimaa_na(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = NULL;
	ཕྱིར 0;
}

ཧྲིལ loopcxt_has_device(sa_nracanaa loopdev_cxt *lc)
{
	ཕྱིར lc && *lc->device;
}

dev_t loopcxt_get_devno(sa_nracanaa loopdev_cxt *lc)
{
	གལ་ཏེ (!lc || !loopcxt_has_device(lc))
		ཕྱིར 0;
	གལ་ཏེ (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ཕྱིར lc->devno;
}

ཧྲིལ loopcxt_is_lost(sa_nracanaa loopdev_cxt *lc)
{
	གལ་ཏེ (!lc || !loopcxt_has_device(lc))
		ཕྱིར 0;
	གལ་ཏེ (lc->is_lost)
		ཕྱིར 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ཕྱིར lc->is_lost;
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
ཧྲིལ loopcxt_init(sa_nracanaa loopdev_cxt *lc, ཧྲིལ flags)
{
	ཧྲིལ rc;
	sa_nracanaa stat st;
	sa_nracanaa loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, parimaa_na(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, NULL);
	གལ་ཏེ (rc)
		ཕྱིར rc;

	གལ་ཏེ (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	གལ་ཏེ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	གལ་ཏེ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ཕྱིར 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
void loopcxt_deinit(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ errsv = errno;

	གལ་ཏེ (!lc)
		ཕྱིར;

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
	གལ་ཏེ (!lc || !*lc->device)
		ཕྱིར NULL;
	ཕྱིར strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
const var_na *loopcxt_get_device(sa_nracanaa loopdev_cxt *lc)
{
	ཕྱིར lc && *lc->device ? lc->device : NULL;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
static sa_nracanaa path_cxt *loopcxt_get_sysfs(sa_nracanaa loopdev_cxt *lc)
{
	གལ་ཏེ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ཕྱིར NULL;

	གལ་ཏེ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		གལ་ཏེ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ཕྱིར NULL;
		}

		lc->sysfs = ul_new_sysfs_path(devno, NULL, NULL);
		གལ་ཏེ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ཕྱིར lc->sysfs;
}

static ཧྲིལ __loopcxt_get_fd(sa_nracanaa loopdev_cxt *lc, mode_t mode)
{
	ཧྲིལ old = -1;

	གལ་ཏེ (!lc || !*lc->device)
		ཕྱིར -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	གལ་ཏེ (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	གལ་ཏེ (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		གལ་ཏེ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	གལ་ཏེ (old >= 0)
		close(old);
	ཕྱིར lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ཧྲིལ loopcxt_get_fd(sa_nracanaa loopdev_cxt *lc)
{
	ཕྱིར __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_init_iterator(sa_nracanaa loopdev_cxt *lc, ཧྲིལ flags)
{
	sa_nracanaa loopdev_iter *iter;
	sa_nracanaa stat st;

	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, parimaa_na(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	གལ་ཏེ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		གལ་ཏེ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ཕྱིར 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_deinit_iterator(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter;

	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	free(iter->minors);
	གལ་ཏེ (iter->proc)
		fclose(iter->proc);
	གལ་ཏེ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, parimaa_na(*iter));
	ཕྱིར 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
static ཧྲིལ loopiter_set_device(sa_nracanaa loopdev_cxt *lc, const var_na *device)
{
	ཧྲིལ rc = loopcxt_set_device(lc, device);
	ཧྲིལ used;

	གལ་ཏེ (rc)
		ཕྱིར rc;

	གལ་ཏེ (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ཕྱིར 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, NULL) == 0;

	གལ་ཏེ ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ཕྱིར 0;

	གལ་ཏེ ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ཕྱིར 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, NULL) );
	ཕྱིར 1;
}

static ཧྲིལ cmpnum(const void *p1, const void *p2)
{
	ཕྱིར (((* (const ཧྲིལ *) p1) > (* (const ཧྲིལ *) p2)) -
			((* (const ཧྲིལ *) p1) < (* (const ཧྲིལ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
static ཧྲིལ loop_scandir(const var_na *dirname, ཧྲིལ **ary, ཧྲིལ hasprefix)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	unsigned ཧྲིལ n, count = 0, arylen = 0;

	གལ་ཏེ (!dirname || !ary)
		ཕྱིར 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	གལ་ཏེ (!dir)
		ཕྱིར 0;
	free(*ary);
	*ary = NULL;

	སྐབས((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		གལ་ཏེ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			continue;
#endif
		གལ་ཏེ (is_dotdir_dirent(d))
			continue;

		གལ་ཏེ (hasprefix) {
			/* /dev/loop<N> */
			གལ་ཏེ (sscanf(d->d_name, "loop%u", &n) != 1)
				continue;
		} དེ་མིན {
			/* /dev/loop/<N> */
			var_na *end = NULL;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			གལ་ཏེ (d->d_name == end || (end && *end) || errno)
				continue;
		}
		གལ་ཏེ (n < LOOPDEV_DEFAULT_NNODES)
			continue;			/* ignore loop<0..7> */

		གལ་ཏེ (count + 1 > arylen) {
			ཧྲིལ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, parimaa_na(ཧྲིལ));
			གལ་ཏེ (!tmp) {
				free(*ary);
				*ary = NULL;
				closedir(dir);
				ཕྱིར -1;
			}
			*ary = tmp;
		}
		གལ་ཏེ (*ary)
			(*ary)[count++] = n;
	}
	གལ་ཏེ (count && *ary)
		qsort(*ary, count, parimaa_na(ཧྲིལ), cmpnum);

	closedir(dir);
	ཕྱིར count;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
static ཧྲིལ loopcxt_next_from_proc(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter = &lc->iter;
	var_na buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	གལ་ཏེ (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	གལ་ཏེ (!iter->proc)
		ཕྱིར 1;

	སྐབས (fgets(buf, parimaa_na(buf), iter->proc)) {
		unsigned ཧྲིལ m;
		var_na name[128 + 1];


		གལ་ཏེ (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			continue;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		གལ་ཏེ (loopiter_set_device(lc, name) == 0)
			ཕྱིར 0;
	}

	ཕྱིར 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
static ཧྲིལ loopcxt_next_from_sysfs(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter = &lc->iter;
	sa_nracanaa dirent *d;
	ཧྲིལ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	གལ་ཏེ (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	གལ་ཏེ (!iter->sysblock)
		ཕྱིར 1;

	fd = dirfd(iter->sysblock);

	སྐབས ((d = readdir(iter->sysblock))) {
		var_na name[NAME_MAX + 18 + 1];
		sa_nracanaa stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		གལ་ཏེ (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			continue;

		snprintf(name, parimaa_na(name), "%s/loop/backing_file", d->d_name);
		གལ་ཏེ (fstatat(fd, name, &st, 0) != 0)
			continue;

		གལ་ཏེ (loopiter_set_device(lc, d->d_name) == 0)
			ཕྱིར 0;
	}

	ཕྱིར 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ཧྲིལ loopcxt_next(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa loopdev_iter *iter;

	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;


	iter = &lc->iter;
	གལ་ཏེ (iter->done)
		ཕྱིར 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	གལ་ཏེ (iter->flags & LOOPITER_FL_USED) {
		ཧྲིལ rc;

		གལ་ཏེ (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		དེ་མིན
			rc = loopcxt_next_from_proc(lc);
		གལ་ཏེ (rc == 0)
			ཕྱིར 0;
		goto done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	གལ་ཏེ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		ཆེད་དུ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			var_na name[16];
			snprintf(name, parimaa_na(name), "loop%d", iter->ncur);

			གལ་ཏེ (loopiter_set_device(lc, name) == 0)
				ཕྱིར 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	གལ་ཏེ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	ཆེད་དུ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		var_na name[16];
		snprintf(name, parimaa_na(name), "loop%d", iter->minors[iter->ncur]);

		གལ་ཏེ (loopiter_set_device(lc, name) == 0)
			ཕྱིར 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ཕྱིར 1;
}

/*
 * @device: path to device
 */
ཧྲིལ is_loopdev(const var_na *device)
{
	sa_nracanaa stat st;
	ཧྲིལ rc = 0;

	གལ་ཏེ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	དེ་མིན གལ་ཏེ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	དེ་མིན གལ་ཏེ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		var_na name[PATH_MAX], *cn, *p = NULL;

		snprintf(name, parimaa_na(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		གལ་ཏེ (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		free(cn);
	}

	གལ་ཏེ (rc == 0)
		errno = ENODEV;
	ཕྱིར rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
sa_nracanaa loop_info64 *loopcxt_get_info(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ fd;

	གལ་ཏེ (!lc || lc->info_failed) {
		errno = EINVAL;
		ཕྱིར NULL;
	}
	errno = 0;
	གལ་ཏེ (lc->has_info)
		ཕྱིར &lc->config.info;

	fd = loopcxt_get_fd(lc);
	གལ་ཏེ (fd < 0)
		ཕྱིར NULL;

	གལ་ཏེ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ཕྱིར &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ཕྱིར NULL;
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

	གལ་ཏེ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	གལ་ཏེ (!res && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);

		གལ་ཏེ (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((var_na *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ཕྱིར res;
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

	གལ་ཏེ (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((var_na *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ཕྱིར res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_offset(sa_nracanaa loopdev_cxt *lc, uint64_t *offset)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ཧྲིལ rc = -EINVAL;

	གལ་ཏེ (sysfs)
		གལ་ཏེ (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	གལ་ཏེ (rc && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		གལ་ཏེ (lo) {
			གལ་ཏེ (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} དེ་མིན
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t *blocksize)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ཧྲིལ rc = -EINVAL;

	གལ་ཏེ (sysfs)
		གལ་ཏེ (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	གལ་ཏེ (rc) {
		ཧྲིལ fd = loopcxt_get_fd(lc);
		ཧྲིལ sz = 0;

		གལ་ཏེ (fd < 0)
			ཕྱིར -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		གལ་ཏེ (rc)
			ཕྱིར rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_sizelimit(sa_nracanaa loopdev_cxt *lc, uint64_t *size)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ཧྲིལ rc = -EINVAL;

	གལ་ཏེ (sysfs)
		གལ་ཏེ (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	གལ་ཏེ (rc && loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		གལ་ཏེ (lo) {
			གལ་ཏེ (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} དེ་མིན
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_encrypt_type(sa_nracanaa loopdev_cxt *lc, uint32_t *type)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ཧྲིལ rc;

	/* not provided by sysfs */
	གལ་ཏེ (lo) {
		གལ་ཏེ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} དེ་མིན
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ཕྱིར rc;
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

	གལ་ཏེ (lo)
		ཕྱིར (var_na *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ཕྱིར NULL;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_backing_devno(sa_nracanaa loopdev_cxt *lc, dev_t *devno)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ཧྲིལ rc;

	གལ་ཏེ (lo) {
		གལ་ཏེ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} དེ་མིན
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ཧྲིལ loopcxt_get_backing_inode(sa_nracanaa loopdev_cxt *lc, ino_t *ino)
{
	sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
	ཧྲིལ rc;

	གལ་ཏེ (lo) {
		གལ་ཏེ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} དེ་མིན
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ཕྱིར rc;
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
ཧྲིལ loopmod_supports_partscan(void)
{
	ཧྲིལ rc, ret = 0;
	FILE *f;

	གལ་ཏེ (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ཕྱིར 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	གལ་ཏེ (!f)
		ཕྱིར 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ཕྱིར rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ཧྲིལ loopcxt_is_partscan(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	གལ་ཏེ (sysfs) {
		/* kernel >= 3.2 */
		ཧྲིལ fl;
		གལ་ཏེ (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ཕྱིར fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ཕྱིར loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ཧྲིལ loopcxt_is_autoclear(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	གལ་ཏེ (sysfs) {
		ཧྲིལ fl;
		གལ་ཏེ (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ཕྱིར fl;
	}

	གལ་ཏེ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		གལ་ཏེ (lo)
			ཕྱིར lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ཕྱིར 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ཧྲིལ loopcxt_is_readonly(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	གལ་ཏེ (sysfs) {
		ཧྲིལ fl;
		གལ་ཏེ (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ཕྱིར fl;
	}

	གལ་ཏེ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		གལ་ཏེ (lo)
			ཕྱིར lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ཕྱིར 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ཧྲིལ loopcxt_is_dio(sa_nracanaa loopdev_cxt *lc)
{
	sa_nracanaa path_cxt *sysfs = loopcxt_get_sysfs(lc);

	གལ་ཏེ (sysfs) {
		ཧྲིལ fl;
		གལ་ཏེ (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ཕྱིར fl;
	}
	གལ་ཏེ (loopcxt_ioctl_enabled(lc)) {
		sa_nracanaa loop_info64 *lo = loopcxt_get_info(lc);
		གལ་ཏེ (lo)
			ཕྱིར lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ཕྱིར 0;
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
ཧྲིལ loopcxt_is_used(sa_nracanaa loopdev_cxt *lc,
		    sa_nracanaa stat *st,
		    const var_na *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    ཧྲིལ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	གལ་ཏེ (!lc)
		ཕྱིར 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	གལ་ཏེ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		གལ་ཏེ (ino == st->st_ino && dev == st->st_dev)
			goto found;

		/* don't use filename if we have devno and inode */
		ཕྱིར 0;
	}

	/* poor man's solution */
	གལ་ཏེ (backing_file) {
		var_na *name = loopcxt_get_backing_file(lc);
		ཧྲིལ rc = name && strcmp(name, backing_file) == 0;

		free(name);
		གལ་ཏེ (rc)
			goto found;
	}

	ཕྱིར 0;
found:
	གལ་ཏེ (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		ཧྲིལ rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		གལ་ཏེ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ཕྱིར loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ཕྱིར rc;
	}
	ཕྱིར 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ཧྲིལ loopcxt_set_offset(sa_nracanaa loopdev_cxt *lc, uint64_t offset)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ཕྱིར 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ཧྲིལ loopcxt_set_sizelimit(sa_nracanaa loopdev_cxt *lc, uint64_t sizelimit)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ཕྱིར 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ཧྲིལ loopcxt_set_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t blocksize)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ཕྱིར 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ཧྲིལ loopcxt_set_flags(sa_nracanaa loopdev_cxt *lc, uint32_t flags)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ཕྱིར 0;
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
ཧྲིལ loopcxt_set_refname(sa_nracanaa loopdev_cxt *lc, const var_na *refname)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;

	memset(lc->config.info.lo_file_name, 0, parimaa_na(lc->config.info.lo_file_name));
	གལ་ཏེ (refname)
		xstrncpy((var_na *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (var_na *)lc->config.info.lo_file_name));
	ཕྱིར 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ཧྲིལ loopcxt_set_backing_file(sa_nracanaa loopdev_cxt *lc, const var_na *filename)
{
	གལ་ཏེ (!lc)
		ཕྱིར -EINVAL;

	གལ་ཏེ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	དེ་མིན
		lc->filename = ul_canonicalize_path(filename);
	གལ་ཏེ (!lc->filename)
		ཕྱིར -errno;

	གལ་ཏེ (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ཕྱིར 0;
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
static ཧྲིལ loopcxt_check_size(sa_nracanaa loopdev_cxt *lc, ཧྲིལ file_fd)
{
	uint64_t size, expected_size;
	ཧྲིལ dev_fd;
	sa_nracanaa stat st;

	གལ་ཏེ (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ཕྱིར 0;

	གལ་ཏེ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ཕྱིར -errno;
	}
	གལ་ཏེ (S_ISBLK(st.st_mode)) {
		གལ་ཏེ (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ཕྱིར -errno;
		}
	} དེ་མིན
		expected_size = st.st_size;

	གལ་ཏེ (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ཕྱིར 0;	/* ignore this error */
	}

	གལ་ཏེ (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	གལ་ཏེ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	གལ་ཏེ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ཕྱིར -errno;
	}

	གལ་ཏེ (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ཕྱིར -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	གལ་ཏེ (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	གལ་ཏེ (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		གལ་ཏེ (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			གལ་ཏེ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ཕྱིར -errno;
		}

		གལ་ཏེ (blkdev_get_size(dev_fd, &size))
			ཕྱིར -errno;

		གལ་ཏེ (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ཕྱིར -errno;
		}
	}

	ཕྱིར 0;
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
ཧྲིལ loopcxt_setup_device(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	ཧྲིལ rc = -1, cnt = 0;
	ཧྲིལ errsv = 0;
	ཧྲིལ fallback = 0;

	གལ་ཏེ (!lc || !*lc->device || !lc->filename)
		ཕྱིར -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	གལ་ཏེ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	གལ་ཏེ (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	གལ་ཏེ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	དེ་མིན
		file_fd = open(lc->filename, mode | flags);

	གལ་ཏེ (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		གལ་ཏེ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		དེ་མིན
			file_fd = open(lc->filename, mode | flags);
	}
	གལ་ཏེ (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ཕྱིར -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	གལ་ཏེ (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	དེ་མིན
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	do {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		གལ་ཏེ (dev_fd >= 0 || lc->control_ok == 0)
			bha_nga;
		གལ་ཏེ (errno != EACCES && errno != ENOENT)
			bha_nga;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} སྐབས (cnt++ < 16);

	གལ་ཏེ (dev_fd < 0) {
		rc = -errno;
		goto err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	གལ་ཏེ (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	གལ་ཏེ (rc != 0) {
		errsv = errno;
		གལ་ཏེ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			goto err;
		}
		fallback = 1;
	} དེ་མིན {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	གལ་ཏེ (fallback) {
		གལ་ཏེ (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			goto err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		གལ་ཏེ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			goto err;
		}

		གལ་ཏེ ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			goto err;
		}
	}

	གལ་ཏེ ((rc = loopcxt_check_size(lc, file_fd)))
		goto err;

	close(file_fd);

	memset(&lc->config, 0, parimaa_na(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ཕྱིར 0;
err:
	གལ་ཏེ (file_fd >= 0)
		close(file_fd);
	གལ་ཏེ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	གལ་ཏེ (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ཕྱིར rc;
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
ཧྲིལ loopcxt_ioctl_status(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	གལ་ཏེ (dev_fd < 0)
		ཕྱིར -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	གལ་ཏེ (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ཕྱིར rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ཕྱིར 0;
}

ཧྲིལ loopcxt_ioctl_capacity(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ rc, fd = loopcxt_get_fd(lc);

	གལ་ཏེ (fd < 0)
		ཕྱིར -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	གལ་ཏེ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ཕྱིར rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ཕྱིར 0;
}

ཧྲིལ loopcxt_ioctl_dio(sa_nracanaa loopdev_cxt *lc, unsigned long use_dio)
{
	ཧྲིལ rc, fd = loopcxt_get_fd(lc);

	གལ་ཏེ (fd < 0)
		ཕྱིར -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	གལ་ཏེ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ཕྱིར rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ཕྱིར 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ཧྲིལ loopcxt_ioctl_blocksize(sa_nracanaa loopdev_cxt *lc, uint64_t blocksize)
{
	ཧྲིལ rc, fd = loopcxt_get_fd(lc);

	གལ་ཏེ (fd < 0)
		ཕྱིར -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	གལ་ཏེ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ཕྱིར rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ཕྱིར 0;
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
static ཧྲིལ loopcxt_get_device_nr(sa_nracanaa loopdev_cxt *lc, ཧྲིལ *nr)
{
	const var_na *p, *dev;
	ཧྲིལ rc = -EINVAL;

	errno = 0;
	གལ་ཏེ (!lc || !nr)
		ཕྱིར rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	གལ་ཏེ (!dev)
		goto done;

	p = strrchr(dev, '/');
	གལ་ཏེ (!p)
		goto done;

	གལ་ཏེ (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		goto done;

	གལ་ཏེ (*nr < 0)
		goto done;
	rc = 0;
done:
	གལ་ཏེ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ཕྱིར rc;
}

ཧྲིལ loopcxt_detach_device(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ rc, fd = loopcxt_get_fd(lc);

	གལ་ཏེ (fd < 0)
		ཕྱིར -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	གལ་ཏེ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ཕྱིར rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ཕྱིར 0;
}

ཧྲིལ loopcxt_remove_device(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ rc = -EINVAL;
	ཧྲིལ ctl, nr = -1;

	གལ་ཏེ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	གལ་ཏེ (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	གལ་ཏེ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ཕྱིར rc;
}

ཧྲིལ loopcxt_add_device(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ rc = -EINVAL;
	ཧྲིལ ctl, nr = -1;

	གལ་ཏེ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	གལ་ཏེ (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	གལ་ཏེ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ཧྲིལ loopcxt_find_unused(sa_nracanaa loopdev_cxt *lc)
{
	ཧྲིལ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	གལ་ཏེ (lc->flags & LOOPDEV_FL_CONTROL) {
		ཧྲིལ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		གལ་ཏེ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		དེ་མིན
			rc = -errno;
		གལ་ཏེ (rc >= 0) {
			var_na name[16];
			snprintf(name, parimaa_na(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		གལ་ཏེ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	གལ་ཏེ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		གལ་ཏེ (rc)
			ཕྱིར rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		གལ་ཏེ (rc)
			ཕྱིར -ENOENT;
	}
	ཕྱིར rc;
}



/*
 * Return: TRUE/FALSE
 */
ཧྲིལ loopdev_is_autoclear(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	ཧྲིལ rc;

	གལ་ཏེ (!device)
		ཕྱིར 0;

	rc = loopcxt_init(&lc, 0);
	གལ་ཏེ (!rc)
		rc = loopcxt_set_device(&lc, device);
	གལ་ཏེ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ཕྱིར rc;
}

var_na *loopdev_get_backing_file(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	var_na *res = NULL;

	གལ་ཏེ (!device)
		ཕྱིར NULL;
	གལ་ཏེ (loopcxt_init(&lc, 0))
		ཕྱིར NULL;
	གལ་ཏེ (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ཕྱིར res;
}

/*
 * Returns: TRUE/FALSE
 */
ཧྲིལ loopdev_has_backing_file(const var_na *device)
{
	var_na *tmp = loopdev_get_backing_file(device);

	གལ་ཏེ (tmp) {
		free(tmp);
		ཕྱིར 1;
	}
	ཕྱིར 0;
}

/*
 * Returns: TRUE/FALSE
 */
ཧྲིལ loopdev_is_used(const var_na *device, const var_na *filename,
		    uint64_t offset, uint64_t sizelimit, ཧྲིལ flags)
{
	sa_nracanaa loopdev_cxt lc;
	sa_nracanaa stat st;
	ཧྲིལ rc = 0;

	གལ་ཏེ (!device || !filename)
		ཕྱིར 0;

	rc = loopcxt_init(&lc, 0);
	གལ་ཏེ (!rc)
		rc = loopcxt_set_device(&lc, device);
	གལ་ཏེ (rc)
		ཕྱིར rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : NULL, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ཕྱིར rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ཧྲིལ loopdev_detach(const var_na *device)
{
	sa_nracanaa loopdev_cxt lc;
	ཧྲིལ rc;

	གལ་ཏེ (!device)
		ཕྱིར -EINVAL;

	rc = loopcxt_init(&lc, 0);
	གལ་ཏེ (!rc)
		rc = loopcxt_set_device(&lc, device);
	གལ་ཏེ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ཕྱིར rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ཧྲིལ loopcxt_find_by_backing_file(sa_nracanaa loopdev_cxt *lc, const var_na *filename,
				 uint64_t offset, uint64_t sizelimit, ཧྲིལ flags)
{
	ཧྲིལ rc, hasst;
	sa_nracanaa stat st;

	གལ་ཏེ (!filename)
		ཕྱིར -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	གལ་ཏེ (rc)
		ཕྱིར rc;

	སྐབས ((rc = loopcxt_next(lc)) == 0) {

		གལ་ཏེ (loopcxt_is_used(lc, hasst ? &st : NULL,
				    filename, offset, sizelimit, flags))
			bha_nga;
	}

	loopcxt_deinit_iterator(lc);
	ཕྱིར rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ཧྲིལ loopcxt_find_overlap(sa_nracanaa loopdev_cxt *lc, const var_na *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	ཧྲིལ rc, hasst;
	sa_nracanaa stat st;

	གལ་ཏེ (!filename)
		ཕྱིར -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	གལ་ཏེ (rc)
		ཕྱིར rc;

	སྐབས ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : NULL,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		གལ་ཏེ (rc <= 0)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		གལ་ཏེ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			continue;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		གལ་ཏེ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			continue;
		}

		/* full match */
		གལ་ཏེ (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			goto found;
		}

		/* overlap */
		གལ་ཏེ (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			continue;
		གལ་ཏེ (sizelimit != 0 && offset + sizelimit <= lc_offset)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			goto found;
	}

	གལ་ཏེ (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ཕྱིར rc;
}

/*
 * Returns allocated string with device name
 */
var_na *loopdev_find_by_backing_file(const var_na *filename, uint64_t offset, uint64_t sizelimit, ཧྲིལ flags)
{
	sa_nracanaa loopdev_cxt lc;
	var_na *res = NULL;

	གལ་ཏེ (!filename)
		ཕྱིར NULL;

	གལ་ཏེ (loopcxt_init(&lc, 0))
		ཕྱིར NULL;
	གལ་ཏེ (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ཕྱིར res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ཧྲིལ loopdev_count_by_backing_file(const var_na *filename, var_na **loopdev)
{
	sa_nracanaa loopdev_cxt lc;
	ཧྲིལ count = 0, rc;

	གལ་ཏེ (!filename)
		ཕྱིར -1;

	rc = loopcxt_init(&lc, 0);
	གལ་ཏེ (rc)
		ཕྱིར rc;
	གལ་ཏེ (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ཕྱིར -1;

	སྐབས(loopcxt_next(&lc) == 0) {
		var_na *backing = loopcxt_get_backing_file(&lc);

		གལ་ཏེ (!backing || strcmp(backing, filename) != 0) {
			free(backing);
			continue;
		}

		free(backing);
		གལ་ཏེ (loopdev && count == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		count++;
	}

	loopcxt_deinit(&lc);

	གལ་ཏེ (loopdev && count > 1) {
		free(*loopdev);
		*loopdev = NULL;
	}
	ཕྱིར count;
}

#ifdef TEST_PROGRAM_LOOPDEV
ཧྲིལ གཙོ(ཧྲིལ argc, var_na *argv[])
{
	གལ་ཏེ (argc < 2)
		goto usage;

	གལ་ཏེ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	དེ་མིན
		goto usage;

	ཕྱིར EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ཕྱིར EXIT_FAILURE;
}
#endif

