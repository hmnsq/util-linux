
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
	nếu (loopdev_debug_mask)
		trả_về;
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
		số_nguyên _c = 0, _e;					\
		do {						\
			errno = 0;				\
			_e = x;					\
			nếu (_e == 0 || errno != EAGAIN)		\
				break;				\
			nếu (_c >= LOOPDEV_MAX_TRIES)		\
				break;				\
			xusleep(250000);			\
			_c++;					\
		} trong_khi (1);					\
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
số_nguyên loopcxt_set_device(struct loopdev_cxt *lc, const char *device)
{
	nếu (!lc)
		trả_về -EINVAL;

	nếu (lc->fd >= 0) {
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
	memset(&lc->config, 0, sizeof(lc->config));

	/* set new */
	nếu (device) {
		nếu (*device != '/') {
			const char *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			nếu (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				nếu (strlen(device) < 5)
					trả_về -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, sizeof(lc->device), "%s%s",
				dir, device);
		} không_thì
			xstrncpy(lc->device, device, sizeof(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = NULL;
	trả_về 0;
}

số_nguyên loopcxt_has_device(struct loopdev_cxt *lc)
{
	trả_về lc && *lc->device;
}

dev_t loopcxt_get_devno(struct loopdev_cxt *lc)
{
	nếu (!lc || !loopcxt_has_device(lc))
		trả_về 0;
	nếu (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	trả_về lc->devno;
}

số_nguyên loopcxt_is_lost(struct loopdev_cxt *lc)
{
	nếu (!lc || !loopcxt_has_device(lc))
		trả_về 0;
	nếu (lc->is_lost)
		trả_về 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	trả_về lc->is_lost;
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
số_nguyên loopcxt_init(struct loopdev_cxt *lc, số_nguyên flags)
{
	số_nguyên rc;
	struct stat st;
	struct loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	nếu (!lc)
		trả_về -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, sizeof(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, NULL);
	nếu (rc)
		trả_về rc;

	nếu (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	nếu (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	nếu (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	trả_về 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
void loopcxt_deinit(struct loopdev_cxt *lc)
{
	số_nguyên errsv = errno;

	nếu (!lc)
		trả_về;

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
char *loopcxt_strdup_device(struct loopdev_cxt *lc)
{
	nếu (!lc || !*lc->device)
		trả_về NULL;
	trả_về strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
const char *loopcxt_get_device(struct loopdev_cxt *lc)
{
	trả_về lc && *lc->device ? lc->device : NULL;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
static struct path_cxt *loopcxt_get_sysfs(struct loopdev_cxt *lc)
{
	nếu (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		trả_về NULL;

	nếu (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		nếu (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			trả_về NULL;
		}

		lc->sysfs = ul_new_sysfs_path(devno, NULL, NULL);
		nếu (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	trả_về lc->sysfs;
}

static số_nguyên __loopcxt_get_fd(struct loopdev_cxt *lc, mode_t mode)
{
	số_nguyên old = -1;

	nếu (!lc || !*lc->device)
		trả_về -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	nếu (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	nếu (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		nếu (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	nếu (old >= 0)
		close(old);
	trả_về lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
số_nguyên loopcxt_get_fd(struct loopdev_cxt *lc)
{
	trả_về __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_init_iterator(struct loopdev_cxt *lc, số_nguyên flags)
{
	struct loopdev_iter *iter;
	struct stat st;

	nếu (!lc)
		trả_về -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, sizeof(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	nếu (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		nếu (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	trả_về 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_deinit_iterator(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter;

	nếu (!lc)
		trả_về -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	free(iter->minors);
	nếu (iter->proc)
		fclose(iter->proc);
	nếu (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, sizeof(*iter));
	trả_về 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
static số_nguyên loopiter_set_device(struct loopdev_cxt *lc, const char *device)
{
	số_nguyên rc = loopcxt_set_device(lc, device);
	số_nguyên used;

	nếu (rc)
		trả_về rc;

	nếu (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		trả_về 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, NULL) == 0;

	nếu ((lc->iter.flags & LOOPITER_FL_USED) && used)
		trả_về 0;

	nếu ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		trả_về 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, NULL) );
	trả_về 1;
}

static số_nguyên cmpnum(const void *p1, const void *p2)
{
	trả_về (((* (const số_nguyên *) p1) > (* (const số_nguyên *) p2)) -
			((* (const số_nguyên *) p1) < (* (const số_nguyên *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
static số_nguyên loop_scandir(const char *dirname, số_nguyên **ary, số_nguyên hasprefix)
{
	DIR *dir;
	struct dirent *d;
	unsigned số_nguyên n, count = 0, arylen = 0;

	nếu (!dirname || !ary)
		trả_về 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	nếu (!dir)
		trả_về 0;
	free(*ary);
	*ary = NULL;

	trong_khi((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		nếu (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			continue;
#endif
		nếu (is_dotdir_dirent(d))
			continue;

		nếu (hasprefix) {
			/* /dev/loop<N> */
			nếu (sscanf(d->d_name, "loop%u", &n) != 1)
				continue;
		} không_thì {
			/* /dev/loop/<N> */
			char *end = NULL;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			nếu (d->d_name == end || (end && *end) || errno)
				continue;
		}
		nếu (n < LOOPDEV_DEFAULT_NNODES)
			continue;			/* ignore loop<0..7> */

		nếu (count + 1 > arylen) {
			số_nguyên *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, sizeof(số_nguyên));
			nếu (!tmp) {
				free(*ary);
				*ary = NULL;
				closedir(dir);
				trả_về -1;
			}
			*ary = tmp;
		}
		nếu (*ary)
			(*ary)[count++] = n;
	}
	nếu (count && *ary)
		qsort(*ary, count, sizeof(số_nguyên), cmpnum);

	closedir(dir);
	trả_về count;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
static số_nguyên loopcxt_next_from_proc(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter = &lc->iter;
	char buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	nếu (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	nếu (!iter->proc)
		trả_về 1;

	trong_khi (fgets(buf, sizeof(buf), iter->proc)) {
		unsigned số_nguyên m;
		char name[128 + 1];


		nếu (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			continue;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		nếu (loopiter_set_device(lc, name) == 0)
			trả_về 0;
	}

	trả_về 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
static số_nguyên loopcxt_next_from_sysfs(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter = &lc->iter;
	struct dirent *d;
	số_nguyên fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	nếu (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	nếu (!iter->sysblock)
		trả_về 1;

	fd = dirfd(iter->sysblock);

	trong_khi ((d = readdir(iter->sysblock))) {
		char name[NAME_MAX + 18 + 1];
		struct stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		nếu (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			continue;

		snprintf(name, sizeof(name), "%s/loop/backing_file", d->d_name);
		nếu (fstatat(fd, name, &st, 0) != 0)
			continue;

		nếu (loopiter_set_device(lc, d->d_name) == 0)
			trả_về 0;
	}

	trả_về 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
số_nguyên loopcxt_next(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter;

	nếu (!lc)
		trả_về -EINVAL;


	iter = &lc->iter;
	nếu (iter->done)
		trả_về 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	nếu (iter->flags & LOOPITER_FL_USED) {
		số_nguyên rc;

		nếu (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		không_thì
			rc = loopcxt_next_from_proc(lc);
		nếu (rc == 0)
			trả_về 0;
		goto done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	nếu (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		cho (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			char name[16];
			snprintf(name, sizeof(name), "loop%d", iter->ncur);

			nếu (loopiter_set_device(lc, name) == 0)
				trả_về 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	nếu (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	cho (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		char name[16];
		snprintf(name, sizeof(name), "loop%d", iter->minors[iter->ncur]);

		nếu (loopiter_set_device(lc, name) == 0)
			trả_về 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	trả_về 1;
}

/*
 * @device: path to device
 */
số_nguyên is_loopdev(const char *device)
{
	struct stat st;
	số_nguyên rc = 0;

	nếu (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	không_thì nếu (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	không_thì nếu (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		char name[PATH_MAX], *cn, *p = NULL;

		snprintf(name, sizeof(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		nếu (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		free(cn);
	}

	nếu (rc == 0)
		errno = ENODEV;
	trả_về rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
struct loop_info64 *loopcxt_get_info(struct loopdev_cxt *lc)
{
	số_nguyên fd;

	nếu (!lc || lc->info_failed) {
		errno = EINVAL;
		trả_về NULL;
	}
	errno = 0;
	nếu (lc->has_info)
		trả_về &lc->config.info;

	fd = loopcxt_get_fd(lc);
	nếu (fd < 0)
		trả_về NULL;

	nếu (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		trả_về &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	trả_về NULL;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
char *loopcxt_get_backing_file(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	char *res = NULL;

	nếu (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	nếu (!res && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);

		nếu (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((char *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	trả_về res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
char *loopcxt_get_refname(struct loopdev_cxt *lc)
{
	char *res = NULL;
	struct loop_info64 *lo = loopcxt_get_info(lc);

	nếu (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((char *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	trả_về res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_offset(struct loopdev_cxt *lc, uint64_t *offset)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	số_nguyên rc = -EINVAL;

	nếu (sysfs)
		nếu (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	nếu (rc && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		nếu (lo) {
			nếu (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} không_thì
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	trả_về rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_blocksize(struct loopdev_cxt *lc, uint64_t *blocksize)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	số_nguyên rc = -EINVAL;

	nếu (sysfs)
		nếu (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	nếu (rc) {
		số_nguyên fd = loopcxt_get_fd(lc);
		số_nguyên sz = 0;

		nếu (fd < 0)
			trả_về -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		nếu (rc)
			trả_về rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	trả_về rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_sizelimit(struct loopdev_cxt *lc, uint64_t *size)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	số_nguyên rc = -EINVAL;

	nếu (sysfs)
		nếu (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	nếu (rc && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		nếu (lo) {
			nếu (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} không_thì
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	trả_về rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_encrypt_type(struct loopdev_cxt *lc, uint32_t *type)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	số_nguyên rc;

	/* not provided by sysfs */
	nếu (lo) {
		nếu (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} không_thì
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	trả_về rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
const char *loopcxt_get_crypt_name(struct loopdev_cxt *lc)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);

	nếu (lo)
		trả_về (char *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	trả_về NULL;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_backing_devno(struct loopdev_cxt *lc, dev_t *devno)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	số_nguyên rc;

	nếu (lo) {
		nếu (devno)
			*devno = lo->lo_device;
		rc = 0;
	} không_thì
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	trả_về rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
số_nguyên loopcxt_get_backing_inode(struct loopdev_cxt *lc, ino_t *ino)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	số_nguyên rc;

	nếu (lo) {
		nếu (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} không_thì
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	trả_về rc;
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
số_nguyên loopmod_supports_partscan(void)
{
	số_nguyên rc, ret = 0;
	FILE *f;

	nếu (get_linux_version() >= KERNEL_VERSION(3,2,0))
		trả_về 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	nếu (!f)
		trả_về 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	trả_về rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
số_nguyên loopcxt_is_partscan(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	nếu (sysfs) {
		/* kernel >= 3.2 */
		số_nguyên fl;
		nếu (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			trả_về fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	trả_về loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
số_nguyên loopcxt_is_autoclear(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	nếu (sysfs) {
		số_nguyên fl;
		nếu (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			trả_về fl;
	}

	nếu (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		nếu (lo)
			trả_về lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	trả_về 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
số_nguyên loopcxt_is_readonly(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	nếu (sysfs) {
		số_nguyên fl;
		nếu (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			trả_về fl;
	}

	nếu (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		nếu (lo)
			trả_về lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	trả_về 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
số_nguyên loopcxt_is_dio(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	nếu (sysfs) {
		số_nguyên fl;
		nếu (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			trả_về fl;
	}
	nếu (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		nếu (lo)
			trả_về lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	trả_về 0;
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
số_nguyên loopcxt_is_used(struct loopdev_cxt *lc,
		    struct stat *st,
		    const char *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    số_nguyên flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	nếu (!lc)
		trả_về 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	nếu (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		nếu (ino == st->st_ino && dev == st->st_dev)
			goto found;

		/* don't use filename if we have devno and inode */
		trả_về 0;
	}

	/* poor man's solution */
	nếu (backing_file) {
		char *name = loopcxt_get_backing_file(lc);
		số_nguyên rc = name && strcmp(name, backing_file) == 0;

		free(name);
		nếu (rc)
			goto found;
	}

	trả_về 0;
found:
	nếu (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		số_nguyên rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		nếu (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			trả_về loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		trả_về rc;
	}
	trả_về 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
số_nguyên loopcxt_set_offset(struct loopdev_cxt *lc, uint64_t offset)
{
	nếu (!lc)
		trả_về -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	trả_về 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
số_nguyên loopcxt_set_sizelimit(struct loopdev_cxt *lc, uint64_t sizelimit)
{
	nếu (!lc)
		trả_về -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	trả_về 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
số_nguyên loopcxt_set_blocksize(struct loopdev_cxt *lc, uint64_t blocksize)
{
	nếu (!lc)
		trả_về -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	trả_về 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
số_nguyên loopcxt_set_flags(struct loopdev_cxt *lc, uint32_t flags)
{
	nếu (!lc)
		trả_về -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	trả_về 0;
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
số_nguyên loopcxt_set_refname(struct loopdev_cxt *lc, const char *refname)
{
	nếu (!lc)
		trả_về -EINVAL;

	memset(lc->config.info.lo_file_name, 0, sizeof(lc->config.info.lo_file_name));
	nếu (refname)
		xstrncpy((char *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (char *)lc->config.info.lo_file_name));
	trả_về 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
số_nguyên loopcxt_set_backing_file(struct loopdev_cxt *lc, const char *filename)
{
	nếu (!lc)
		trả_về -EINVAL;

	nếu (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	không_thì
		lc->filename = ul_canonicalize_path(filename);
	nếu (!lc->filename)
		trả_về -errno;

	nếu (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	trả_về 0;
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
static số_nguyên loopcxt_check_size(struct loopdev_cxt *lc, số_nguyên file_fd)
{
	uint64_t size, expected_size;
	số_nguyên dev_fd;
	struct stat st;

	nếu (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		trả_về 0;

	nếu (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		trả_về -errno;
	}
	nếu (S_ISBLK(st.st_mode)) {
		nếu (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			trả_về -errno;
		}
	} không_thì
		expected_size = st.st_size;

	nếu (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		trả_về 0;	/* ignore this error */
	}

	nếu (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	nếu (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	nếu (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		trả_về -errno;
	}

	nếu (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		trả_về -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	nếu (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	nếu (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		nếu (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			nếu (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			trả_về -errno;
		}

		nếu (blkdev_get_size(dev_fd, &size))
			trả_về -errno;

		nếu (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			trả_về -errno;
		}
	}

	trả_về 0;
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
số_nguyên loopcxt_setup_device(struct loopdev_cxt *lc)
{
	số_nguyên file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	số_nguyên rc = -1, cnt = 0;
	số_nguyên errsv = 0;
	số_nguyên fallback = 0;

	nếu (!lc || !*lc->device || !lc->filename)
		trả_về -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	nếu (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	nếu (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	nếu (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	không_thì
		file_fd = open(lc->filename, mode | flags);

	nếu (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		nếu (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		không_thì
			file_fd = open(lc->filename, mode | flags);
	}
	nếu (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		trả_về -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	nếu (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	không_thì
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	do {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		nếu (dev_fd >= 0 || lc->control_ok == 0)
			break;
		nếu (errno != EACCES && errno != ENOENT)
			break;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} trong_khi (cnt++ < 16);

	nếu (dev_fd < 0) {
		rc = -errno;
		goto err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	nếu (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	nếu (rc != 0) {
		errsv = errno;
		nếu (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			goto err;
		}
		fallback = 1;
	} không_thì {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	nếu (fallback) {
		nếu (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			goto err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		nếu (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			goto err;
		}

		nếu ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			goto err;
		}
	}

	nếu ((rc = loopcxt_check_size(lc, file_fd)))
		goto err;

	close(file_fd);

	memset(&lc->config, 0, sizeof(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	trả_về 0;
err:
	nếu (file_fd >= 0)
		close(file_fd);
	nếu (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	nếu (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	trả_về rc;
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
số_nguyên loopcxt_ioctl_status(struct loopdev_cxt *lc)
{
	số_nguyên dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	nếu (dev_fd < 0)
		trả_về -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	nếu (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		trả_về rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	trả_về 0;
}

số_nguyên loopcxt_ioctl_capacity(struct loopdev_cxt *lc)
{
	số_nguyên rc, fd = loopcxt_get_fd(lc);

	nếu (fd < 0)
		trả_về -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	nếu (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		trả_về rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	trả_về 0;
}

số_nguyên loopcxt_ioctl_dio(struct loopdev_cxt *lc, unsigned long use_dio)
{
	số_nguyên rc, fd = loopcxt_get_fd(lc);

	nếu (fd < 0)
		trả_về -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	nếu (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		trả_về rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	trả_về 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
số_nguyên loopcxt_ioctl_blocksize(struct loopdev_cxt *lc, uint64_t blocksize)
{
	số_nguyên rc, fd = loopcxt_get_fd(lc);

	nếu (fd < 0)
		trả_về -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	nếu (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		trả_về rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	trả_về 0;
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
static số_nguyên loopcxt_get_device_nr(struct loopdev_cxt *lc, số_nguyên *nr)
{
	const char *p, *dev;
	số_nguyên rc = -EINVAL;

	errno = 0;
	nếu (!lc || !nr)
		trả_về rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	nếu (!dev)
		goto done;

	p = strrchr(dev, '/');
	nếu (!p)
		goto done;

	nếu (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		goto done;

	nếu (*nr < 0)
		goto done;
	rc = 0;
done:
	nếu (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	trả_về rc;
}

số_nguyên loopcxt_detach_device(struct loopdev_cxt *lc)
{
	số_nguyên rc, fd = loopcxt_get_fd(lc);

	nếu (fd < 0)
		trả_về -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	nếu (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		trả_về rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	trả_về 0;
}

số_nguyên loopcxt_remove_device(struct loopdev_cxt *lc)
{
	số_nguyên rc = -EINVAL;
	số_nguyên ctl, nr = -1;

	nếu (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	nếu (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	nếu (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	trả_về rc;
}

số_nguyên loopcxt_add_device(struct loopdev_cxt *lc)
{
	số_nguyên rc = -EINVAL;
	số_nguyên ctl, nr = -1;

	nếu (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	nếu (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	nếu (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	trả_về rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
số_nguyên loopcxt_find_unused(struct loopdev_cxt *lc)
{
	số_nguyên rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	nếu (lc->flags & LOOPDEV_FL_CONTROL) {
		số_nguyên ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		nếu (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		không_thì
			rc = -errno;
		nếu (rc >= 0) {
			char name[16];
			snprintf(name, sizeof(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		nếu (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	nếu (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		nếu (rc)
			trả_về rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		nếu (rc)
			trả_về -ENOENT;
	}
	trả_về rc;
}



/*
 * Return: TRUE/FALSE
 */
số_nguyên loopdev_is_autoclear(const char *device)
{
	struct loopdev_cxt lc;
	số_nguyên rc;

	nếu (!device)
		trả_về 0;

	rc = loopcxt_init(&lc, 0);
	nếu (!rc)
		rc = loopcxt_set_device(&lc, device);
	nếu (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	trả_về rc;
}

char *loopdev_get_backing_file(const char *device)
{
	struct loopdev_cxt lc;
	char *res = NULL;

	nếu (!device)
		trả_về NULL;
	nếu (loopcxt_init(&lc, 0))
		trả_về NULL;
	nếu (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	trả_về res;
}

/*
 * Returns: TRUE/FALSE
 */
số_nguyên loopdev_has_backing_file(const char *device)
{
	char *tmp = loopdev_get_backing_file(device);

	nếu (tmp) {
		free(tmp);
		trả_về 1;
	}
	trả_về 0;
}

/*
 * Returns: TRUE/FALSE
 */
số_nguyên loopdev_is_used(const char *device, const char *filename,
		    uint64_t offset, uint64_t sizelimit, số_nguyên flags)
{
	struct loopdev_cxt lc;
	struct stat st;
	số_nguyên rc = 0;

	nếu (!device || !filename)
		trả_về 0;

	rc = loopcxt_init(&lc, 0);
	nếu (!rc)
		rc = loopcxt_set_device(&lc, device);
	nếu (rc)
		trả_về rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : NULL, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	trả_về rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
số_nguyên loopdev_detach(const char *device)
{
	struct loopdev_cxt lc;
	số_nguyên rc;

	nếu (!device)
		trả_về -EINVAL;

	rc = loopcxt_init(&lc, 0);
	nếu (!rc)
		rc = loopcxt_set_device(&lc, device);
	nếu (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	trả_về rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
số_nguyên loopcxt_find_by_backing_file(struct loopdev_cxt *lc, const char *filename,
				 uint64_t offset, uint64_t sizelimit, số_nguyên flags)
{
	số_nguyên rc, hasst;
	struct stat st;

	nếu (!filename)
		trả_về -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	nếu (rc)
		trả_về rc;

	trong_khi ((rc = loopcxt_next(lc)) == 0) {

		nếu (loopcxt_is_used(lc, hasst ? &st : NULL,
				    filename, offset, sizelimit, flags))
			break;
	}

	loopcxt_deinit_iterator(lc);
	trả_về rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
số_nguyên loopcxt_find_overlap(struct loopdev_cxt *lc, const char *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	số_nguyên rc, hasst;
	struct stat st;

	nếu (!filename)
		trả_về -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	nếu (rc)
		trả_về rc;

	trong_khi ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : NULL,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		nếu (rc <= 0)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		nếu (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			continue;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		nếu (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			continue;
		}

		/* full match */
		nếu (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			goto found;
		}

		/* overlap */
		nếu (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			continue;
		nếu (sizelimit != 0 && offset + sizelimit <= lc_offset)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			goto found;
	}

	nếu (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	trả_về rc;
}

/*
 * Returns allocated string with device name
 */
char *loopdev_find_by_backing_file(const char *filename, uint64_t offset, uint64_t sizelimit, số_nguyên flags)
{
	struct loopdev_cxt lc;
	char *res = NULL;

	nếu (!filename)
		trả_về NULL;

	nếu (loopcxt_init(&lc, 0))
		trả_về NULL;
	nếu (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	trả_về res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
số_nguyên loopdev_count_by_backing_file(const char *filename, char **loopdev)
{
	struct loopdev_cxt lc;
	số_nguyên count = 0, rc;

	nếu (!filename)
		trả_về -1;

	rc = loopcxt_init(&lc, 0);
	nếu (rc)
		trả_về rc;
	nếu (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		trả_về -1;

	trong_khi(loopcxt_next(&lc) == 0) {
		char *backing = loopcxt_get_backing_file(&lc);

		nếu (!backing || strcmp(backing, filename) != 0) {
			free(backing);
			continue;
		}

		free(backing);
		nếu (loopdev && count == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		count++;
	}

	loopcxt_deinit(&lc);

	nếu (loopdev && count > 1) {
		free(*loopdev);
		*loopdev = NULL;
	}
	trả_về count;
}

#ifdef TEST_PROGRAM_LOOPDEV
số_nguyên main(số_nguyên argc, char *argv[])
{
	nếu (argc < 2)
		goto usage;

	nếu (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	không_thì
		goto usage;

	trả_về EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	trả_về EXIT_FAILURE;
}
#endif

