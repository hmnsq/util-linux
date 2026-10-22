
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
	如果 (loopdev_debug_mask)
		返回;
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
		整数 _c = 0, _e;					\
		do {						\
			errno = 0;				\
			_e = x;					\
			如果 (_e == 0 || errno != EAGAIN)		\
				break;				\
			如果 (_c >= LOOPDEV_MAX_TRIES)		\
				break;				\
			xusleep(250000);			\
			_c++;					\
		} 当 (1);					\
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
整数 loopcxt_set_device(struct loopdev_cxt *lc, const char *device)
{
	如果 (!lc)
		返回 -EINVAL;

	如果 (lc->fd >= 0) {
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
	如果 (device) {
		如果 (*device != '/') {
			const char *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			如果 (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				如果 (strlen(device) < 5)
					返回 -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, sizeof(lc->device), "%s%s",
				dir, device);
		} 否则
			xstrncpy(lc->device, device, sizeof(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = NULL;
	返回 0;
}

整数 loopcxt_has_device(struct loopdev_cxt *lc)
{
	返回 lc && *lc->device;
}

dev_t loopcxt_get_devno(struct loopdev_cxt *lc)
{
	如果 (!lc || !loopcxt_has_device(lc))
		返回 0;
	如果 (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	返回 lc->devno;
}

整数 loopcxt_is_lost(struct loopdev_cxt *lc)
{
	如果 (!lc || !loopcxt_has_device(lc))
		返回 0;
	如果 (lc->is_lost)
		返回 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	返回 lc->is_lost;
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
整数 loopcxt_init(struct loopdev_cxt *lc, 整数 flags)
{
	整数 rc;
	struct stat st;
	struct loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	如果 (!lc)
		返回 -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, sizeof(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, NULL);
	如果 (rc)
		返回 rc;

	如果 (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	如果 (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	如果 (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	返回 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
void loopcxt_deinit(struct loopdev_cxt *lc)
{
	整数 errsv = errno;

	如果 (!lc)
		返回;

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
	如果 (!lc || !*lc->device)
		返回 NULL;
	返回 strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
const char *loopcxt_get_device(struct loopdev_cxt *lc)
{
	返回 lc && *lc->device ? lc->device : NULL;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
static struct path_cxt *loopcxt_get_sysfs(struct loopdev_cxt *lc)
{
	如果 (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		返回 NULL;

	如果 (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		如果 (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			返回 NULL;
		}

		lc->sysfs = ul_new_sysfs_path(devno, NULL, NULL);
		如果 (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	返回 lc->sysfs;
}

static 整数 __loopcxt_get_fd(struct loopdev_cxt *lc, mode_t mode)
{
	整数 old = -1;

	如果 (!lc || !*lc->device)
		返回 -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	如果 (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	如果 (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		如果 (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	如果 (old >= 0)
		close(old);
	返回 lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
整数 loopcxt_get_fd(struct loopdev_cxt *lc)
{
	返回 __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_init_iterator(struct loopdev_cxt *lc, 整数 flags)
{
	struct loopdev_iter *iter;
	struct stat st;

	如果 (!lc)
		返回 -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, sizeof(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	如果 (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		如果 (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	返回 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_deinit_iterator(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter;

	如果 (!lc)
		返回 -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	free(iter->minors);
	如果 (iter->proc)
		fclose(iter->proc);
	如果 (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, sizeof(*iter));
	返回 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
static 整数 loopiter_set_device(struct loopdev_cxt *lc, const char *device)
{
	整数 rc = loopcxt_set_device(lc, device);
	整数 used;

	如果 (rc)
		返回 rc;

	如果 (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		返回 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, NULL) == 0;

	如果 ((lc->iter.flags & LOOPITER_FL_USED) && used)
		返回 0;

	如果 ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		返回 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, NULL) );
	返回 1;
}

static 整数 cmpnum(const void *p1, const void *p2)
{
	返回 (((* (const 整数 *) p1) > (* (const 整数 *) p2)) -
			((* (const 整数 *) p1) < (* (const 整数 *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
static 整数 loop_scandir(const char *dirname, 整数 **ary, 整数 hasprefix)
{
	DIR *dir;
	struct dirent *d;
	unsigned 整数 n, count = 0, arylen = 0;

	如果 (!dirname || !ary)
		返回 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	如果 (!dir)
		返回 0;
	free(*ary);
	*ary = NULL;

	当((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		如果 (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			continue;
#endif
		如果 (is_dotdir_dirent(d))
			continue;

		如果 (hasprefix) {
			/* /dev/loop<N> */
			如果 (sscanf(d->d_name, "loop%u", &n) != 1)
				continue;
		} 否则 {
			/* /dev/loop/<N> */
			char *end = NULL;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			如果 (d->d_name == end || (end && *end) || errno)
				continue;
		}
		如果 (n < LOOPDEV_DEFAULT_NNODES)
			continue;			/* ignore loop<0..7> */

		如果 (count + 1 > arylen) {
			整数 *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, sizeof(整数));
			如果 (!tmp) {
				free(*ary);
				*ary = NULL;
				closedir(dir);
				返回 -1;
			}
			*ary = tmp;
		}
		如果 (*ary)
			(*ary)[count++] = n;
	}
	如果 (count && *ary)
		qsort(*ary, count, sizeof(整数), cmpnum);

	closedir(dir);
	返回 count;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
static 整数 loopcxt_next_from_proc(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter = &lc->iter;
	char buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	如果 (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	如果 (!iter->proc)
		返回 1;

	当 (fgets(buf, sizeof(buf), iter->proc)) {
		unsigned 整数 m;
		char name[128 + 1];


		如果 (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			continue;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		如果 (loopiter_set_device(lc, name) == 0)
			返回 0;
	}

	返回 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
static 整数 loopcxt_next_from_sysfs(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter = &lc->iter;
	struct dirent *d;
	整数 fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	如果 (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	如果 (!iter->sysblock)
		返回 1;

	fd = dirfd(iter->sysblock);

	当 ((d = readdir(iter->sysblock))) {
		char name[NAME_MAX + 18 + 1];
		struct stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		如果 (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			continue;

		snprintf(name, sizeof(name), "%s/loop/backing_file", d->d_name);
		如果 (fstatat(fd, name, &st, 0) != 0)
			continue;

		如果 (loopiter_set_device(lc, d->d_name) == 0)
			返回 0;
	}

	返回 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
整数 loopcxt_next(struct loopdev_cxt *lc)
{
	struct loopdev_iter *iter;

	如果 (!lc)
		返回 -EINVAL;


	iter = &lc->iter;
	如果 (iter->done)
		返回 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	如果 (iter->flags & LOOPITER_FL_USED) {
		整数 rc;

		如果 (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		否则
			rc = loopcxt_next_from_proc(lc);
		如果 (rc == 0)
			返回 0;
		goto done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	如果 (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		为 (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			char name[16];
			snprintf(name, sizeof(name), "loop%d", iter->ncur);

			如果 (loopiter_set_device(lc, name) == 0)
				返回 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	如果 (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	为 (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		char name[16];
		snprintf(name, sizeof(name), "loop%d", iter->minors[iter->ncur]);

		如果 (loopiter_set_device(lc, name) == 0)
			返回 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	返回 1;
}

/*
 * @device: path to device
 */
整数 is_loopdev(const char *device)
{
	struct stat st;
	整数 rc = 0;

	如果 (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	否则 如果 (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	否则 如果 (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		char name[PATH_MAX], *cn, *p = NULL;

		snprintf(name, sizeof(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		如果 (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		free(cn);
	}

	如果 (rc == 0)
		errno = ENODEV;
	返回 rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
struct loop_info64 *loopcxt_get_info(struct loopdev_cxt *lc)
{
	整数 fd;

	如果 (!lc || lc->info_failed) {
		errno = EINVAL;
		返回 NULL;
	}
	errno = 0;
	如果 (lc->has_info)
		返回 &lc->config.info;

	fd = loopcxt_get_fd(lc);
	如果 (fd < 0)
		返回 NULL;

	如果 (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		返回 &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	返回 NULL;
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

	如果 (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	如果 (!res && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);

		如果 (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((char *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	返回 res;
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

	如果 (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((char *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	返回 res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_offset(struct loopdev_cxt *lc, uint64_t *offset)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	整数 rc = -EINVAL;

	如果 (sysfs)
		如果 (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	如果 (rc && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		如果 (lo) {
			如果 (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} 否则
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	返回 rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_blocksize(struct loopdev_cxt *lc, uint64_t *blocksize)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	整数 rc = -EINVAL;

	如果 (sysfs)
		如果 (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	如果 (rc) {
		整数 fd = loopcxt_get_fd(lc);
		整数 sz = 0;

		如果 (fd < 0)
			返回 -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		如果 (rc)
			返回 rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	返回 rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_sizelimit(struct loopdev_cxt *lc, uint64_t *size)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);
	整数 rc = -EINVAL;

	如果 (sysfs)
		如果 (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	如果 (rc && loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		如果 (lo) {
			如果 (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} 否则
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	返回 rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_encrypt_type(struct loopdev_cxt *lc, uint32_t *type)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	整数 rc;

	/* not provided by sysfs */
	如果 (lo) {
		如果 (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} 否则
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	返回 rc;
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

	如果 (lo)
		返回 (char *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	返回 NULL;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_backing_devno(struct loopdev_cxt *lc, dev_t *devno)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	整数 rc;

	如果 (lo) {
		如果 (devno)
			*devno = lo->lo_device;
		rc = 0;
	} 否则
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	返回 rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
整数 loopcxt_get_backing_inode(struct loopdev_cxt *lc, ino_t *ino)
{
	struct loop_info64 *lo = loopcxt_get_info(lc);
	整数 rc;

	如果 (lo) {
		如果 (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} 否则
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	返回 rc;
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
整数 loopmod_supports_partscan(void)
{
	整数 rc, ret = 0;
	FILE *f;

	如果 (get_linux_version() >= KERNEL_VERSION(3,2,0))
		返回 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	如果 (!f)
		返回 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	返回 rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
整数 loopcxt_is_partscan(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	如果 (sysfs) {
		/* kernel >= 3.2 */
		整数 fl;
		如果 (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			返回 fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	返回 loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
整数 loopcxt_is_autoclear(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	如果 (sysfs) {
		整数 fl;
		如果 (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			返回 fl;
	}

	如果 (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		如果 (lo)
			返回 lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	返回 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
整数 loopcxt_is_readonly(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	如果 (sysfs) {
		整数 fl;
		如果 (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			返回 fl;
	}

	如果 (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		如果 (lo)
			返回 lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	返回 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
整数 loopcxt_is_dio(struct loopdev_cxt *lc)
{
	struct path_cxt *sysfs = loopcxt_get_sysfs(lc);

	如果 (sysfs) {
		整数 fl;
		如果 (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			返回 fl;
	}
	如果 (loopcxt_ioctl_enabled(lc)) {
		struct loop_info64 *lo = loopcxt_get_info(lc);
		如果 (lo)
			返回 lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	返回 0;
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
整数 loopcxt_is_used(struct loopdev_cxt *lc,
		    struct stat *st,
		    const char *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    整数 flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	如果 (!lc)
		返回 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	如果 (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		如果 (ino == st->st_ino && dev == st->st_dev)
			goto found;

		/* don't use filename if we have devno and inode */
		返回 0;
	}

	/* poor man's solution */
	如果 (backing_file) {
		char *name = loopcxt_get_backing_file(lc);
		整数 rc = name && strcmp(name, backing_file) == 0;

		free(name);
		如果 (rc)
			goto found;
	}

	返回 0;
found:
	如果 (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		整数 rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		如果 (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			返回 loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		返回 rc;
	}
	返回 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
整数 loopcxt_set_offset(struct loopdev_cxt *lc, uint64_t offset)
{
	如果 (!lc)
		返回 -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	返回 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
整数 loopcxt_set_sizelimit(struct loopdev_cxt *lc, uint64_t sizelimit)
{
	如果 (!lc)
		返回 -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	返回 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
整数 loopcxt_set_blocksize(struct loopdev_cxt *lc, uint64_t blocksize)
{
	如果 (!lc)
		返回 -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	返回 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
整数 loopcxt_set_flags(struct loopdev_cxt *lc, uint32_t flags)
{
	如果 (!lc)
		返回 -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	返回 0;
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
整数 loopcxt_set_refname(struct loopdev_cxt *lc, const char *refname)
{
	如果 (!lc)
		返回 -EINVAL;

	memset(lc->config.info.lo_file_name, 0, sizeof(lc->config.info.lo_file_name));
	如果 (refname)
		xstrncpy((char *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (char *)lc->config.info.lo_file_name));
	返回 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
整数 loopcxt_set_backing_file(struct loopdev_cxt *lc, const char *filename)
{
	如果 (!lc)
		返回 -EINVAL;

	如果 (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	否则
		lc->filename = ul_canonicalize_path(filename);
	如果 (!lc->filename)
		返回 -errno;

	如果 (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	返回 0;
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
static 整数 loopcxt_check_size(struct loopdev_cxt *lc, 整数 file_fd)
{
	uint64_t size, expected_size;
	整数 dev_fd;
	struct stat st;

	如果 (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		返回 0;

	如果 (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		返回 -errno;
	}
	如果 (S_ISBLK(st.st_mode)) {
		如果 (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			返回 -errno;
		}
	} 否则
		expected_size = st.st_size;

	如果 (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		返回 0;	/* ignore this error */
	}

	如果 (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	如果 (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	如果 (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		返回 -errno;
	}

	如果 (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		返回 -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	如果 (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	如果 (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		如果 (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			如果 (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			返回 -errno;
		}

		如果 (blkdev_get_size(dev_fd, &size))
			返回 -errno;

		如果 (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			返回 -errno;
		}
	}

	返回 0;
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
整数 loopcxt_setup_device(struct loopdev_cxt *lc)
{
	整数 file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	整数 rc = -1, cnt = 0;
	整数 errsv = 0;
	整数 fallback = 0;

	如果 (!lc || !*lc->device || !lc->filename)
		返回 -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	如果 (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	如果 (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	如果 (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	否则
		file_fd = open(lc->filename, mode | flags);

	如果 (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		如果 (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		否则
			file_fd = open(lc->filename, mode | flags);
	}
	如果 (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		返回 -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	如果 (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	否则
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	do {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		如果 (dev_fd >= 0 || lc->control_ok == 0)
			break;
		如果 (errno != EACCES && errno != ENOENT)
			break;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} 当 (cnt++ < 16);

	如果 (dev_fd < 0) {
		rc = -errno;
		goto err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	如果 (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	如果 (rc != 0) {
		errsv = errno;
		如果 (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			goto err;
		}
		fallback = 1;
	} 否则 {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	如果 (fallback) {
		如果 (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			goto err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		如果 (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			goto err;
		}

		如果 ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			goto err;
		}
	}

	如果 ((rc = loopcxt_check_size(lc, file_fd)))
		goto err;

	close(file_fd);

	memset(&lc->config, 0, sizeof(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	返回 0;
err:
	如果 (file_fd >= 0)
		close(file_fd);
	如果 (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	如果 (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	返回 rc;
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
整数 loopcxt_ioctl_status(struct loopdev_cxt *lc)
{
	整数 dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	如果 (dev_fd < 0)
		返回 -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	如果 (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		返回 rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	返回 0;
}

整数 loopcxt_ioctl_capacity(struct loopdev_cxt *lc)
{
	整数 rc, fd = loopcxt_get_fd(lc);

	如果 (fd < 0)
		返回 -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	如果 (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		返回 rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	返回 0;
}

整数 loopcxt_ioctl_dio(struct loopdev_cxt *lc, unsigned long use_dio)
{
	整数 rc, fd = loopcxt_get_fd(lc);

	如果 (fd < 0)
		返回 -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	如果 (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		返回 rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	返回 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
整数 loopcxt_ioctl_blocksize(struct loopdev_cxt *lc, uint64_t blocksize)
{
	整数 rc, fd = loopcxt_get_fd(lc);

	如果 (fd < 0)
		返回 -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	如果 (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		返回 rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	返回 0;
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
static 整数 loopcxt_get_device_nr(struct loopdev_cxt *lc, 整数 *nr)
{
	const char *p, *dev;
	整数 rc = -EINVAL;

	errno = 0;
	如果 (!lc || !nr)
		返回 rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	如果 (!dev)
		goto done;

	p = strrchr(dev, '/');
	如果 (!p)
		goto done;

	如果 (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		goto done;

	如果 (*nr < 0)
		goto done;
	rc = 0;
done:
	如果 (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	返回 rc;
}

整数 loopcxt_detach_device(struct loopdev_cxt *lc)
{
	整数 rc, fd = loopcxt_get_fd(lc);

	如果 (fd < 0)
		返回 -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	如果 (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		返回 rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	返回 0;
}

整数 loopcxt_remove_device(struct loopdev_cxt *lc)
{
	整数 rc = -EINVAL;
	整数 ctl, nr = -1;

	如果 (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	如果 (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	如果 (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	返回 rc;
}

整数 loopcxt_add_device(struct loopdev_cxt *lc)
{
	整数 rc = -EINVAL;
	整数 ctl, nr = -1;

	如果 (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		goto done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	如果 (rc)
		goto done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	如果 (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	返回 rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
整数 loopcxt_find_unused(struct loopdev_cxt *lc)
{
	整数 rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	如果 (lc->flags & LOOPDEV_FL_CONTROL) {
		整数 ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		如果 (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		否则
			rc = -errno;
		如果 (rc >= 0) {
			char name[16];
			snprintf(name, sizeof(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		如果 (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	如果 (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		如果 (rc)
			返回 rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		如果 (rc)
			返回 -ENOENT;
	}
	返回 rc;
}



/*
 * Return: TRUE/FALSE
 */
整数 loopdev_is_autoclear(const char *device)
{
	struct loopdev_cxt lc;
	整数 rc;

	如果 (!device)
		返回 0;

	rc = loopcxt_init(&lc, 0);
	如果 (!rc)
		rc = loopcxt_set_device(&lc, device);
	如果 (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	返回 rc;
}

char *loopdev_get_backing_file(const char *device)
{
	struct loopdev_cxt lc;
	char *res = NULL;

	如果 (!device)
		返回 NULL;
	如果 (loopcxt_init(&lc, 0))
		返回 NULL;
	如果 (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	返回 res;
}

/*
 * Returns: TRUE/FALSE
 */
整数 loopdev_has_backing_file(const char *device)
{
	char *tmp = loopdev_get_backing_file(device);

	如果 (tmp) {
		free(tmp);
		返回 1;
	}
	返回 0;
}

/*
 * Returns: TRUE/FALSE
 */
整数 loopdev_is_used(const char *device, const char *filename,
		    uint64_t offset, uint64_t sizelimit, 整数 flags)
{
	struct loopdev_cxt lc;
	struct stat st;
	整数 rc = 0;

	如果 (!device || !filename)
		返回 0;

	rc = loopcxt_init(&lc, 0);
	如果 (!rc)
		rc = loopcxt_set_device(&lc, device);
	如果 (rc)
		返回 rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : NULL, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	返回 rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
整数 loopdev_detach(const char *device)
{
	struct loopdev_cxt lc;
	整数 rc;

	如果 (!device)
		返回 -EINVAL;

	rc = loopcxt_init(&lc, 0);
	如果 (!rc)
		rc = loopcxt_set_device(&lc, device);
	如果 (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	返回 rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
整数 loopcxt_find_by_backing_file(struct loopdev_cxt *lc, const char *filename,
				 uint64_t offset, uint64_t sizelimit, 整数 flags)
{
	整数 rc, hasst;
	struct stat st;

	如果 (!filename)
		返回 -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	如果 (rc)
		返回 rc;

	当 ((rc = loopcxt_next(lc)) == 0) {

		如果 (loopcxt_is_used(lc, hasst ? &st : NULL,
				    filename, offset, sizelimit, flags))
			break;
	}

	loopcxt_deinit_iterator(lc);
	返回 rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
整数 loopcxt_find_overlap(struct loopdev_cxt *lc, const char *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	整数 rc, hasst;
	struct stat st;

	如果 (!filename)
		返回 -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	如果 (rc)
		返回 rc;

	当 ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : NULL,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		如果 (rc <= 0)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		如果 (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			continue;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		如果 (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			continue;
		}

		/* full match */
		如果 (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			goto found;
		}

		/* overlap */
		如果 (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			continue;
		如果 (sizelimit != 0 && offset + sizelimit <= lc_offset)
			continue;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			goto found;
	}

	如果 (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	返回 rc;
}

/*
 * Returns allocated string with device name
 */
char *loopdev_find_by_backing_file(const char *filename, uint64_t offset, uint64_t sizelimit, 整数 flags)
{
	struct loopdev_cxt lc;
	char *res = NULL;

	如果 (!filename)
		返回 NULL;

	如果 (loopcxt_init(&lc, 0))
		返回 NULL;
	如果 (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	返回 res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
整数 loopdev_count_by_backing_file(const char *filename, char **loopdev)
{
	struct loopdev_cxt lc;
	整数 count = 0, rc;

	如果 (!filename)
		返回 -1;

	rc = loopcxt_init(&lc, 0);
	如果 (rc)
		返回 rc;
	如果 (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		返回 -1;

	当(loopcxt_next(&lc) == 0) {
		char *backing = loopcxt_get_backing_file(&lc);

		如果 (!backing || strcmp(backing, filename) != 0) {
			free(backing);
			continue;
		}

		free(backing);
		如果 (loopdev && count == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		count++;
	}

	loopcxt_deinit(&lc);

	如果 (loopdev && count > 1) {
		free(*loopdev);
		*loopdev = NULL;
	}
	返回 count;
}

#ifdef TEST_PROGRAM_LOOPDEV
整数 主要(整数 argc, char *argv[])
{
	如果 (argc < 2)
		goto usage;

	如果 (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	否则
		goto usage;

	返回 EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	返回 EXIT_FAILURE;
}
#endif

