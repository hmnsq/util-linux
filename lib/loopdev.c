
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
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>

#ଅନ୍ତର୍ଭାଵ "linux_version.h"
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "sysfs.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "loopdev.h"
#ଅନ୍ତର୍ଭାଵ "canonicalize.h"
#ଅନ୍ତର୍ଭାଵ "blkdev.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ସ୍ଥିର UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ସ୍ଥିର ଶୂନ୍ୟ loopdev_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (loopdev_debug_mask)
		ଫେରନ୍ତୁ;
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
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା _c = 0, _e;					\
		କର {						\
			errno = 0;				\
			_e = x;					\
			ଯଦି (_e == 0 || errno != EAGAIN)		\
				ଭାଙ୍ଗ;				\
			ଯଦି (_c >= LOOPDEV_MAX_TRIES)		\
				ଭାଙ୍ଗ;				\
			xusleep(250000);			\
			_c++;					\
		} ଯେତେବେଳେ (1);					\
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_device(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *device)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (lc->fd >= 0) {
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
	memset(&lc->config, 0, ଆକାର(lc->config));

	/* set new */
	ଯଦି (device) {
		ଯଦି (*device != '/') {
			ଅଚଳ ଅକ୍ଷର *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ଯଦି (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ଯଦି (strlen(device) < 5)
					ଫେରନ୍ତୁ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, ଆକାର(lc->device), "%s%s",
				dir, device);
		} ଅନ୍ୟଥା
			xstrncpy(lc->device, device, ଆକାର(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = ଖାଲି;
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_has_device(ସଂରଚନା loopdev_cxt *lc)
{
	ଫେରନ୍ତୁ lc && *lc->device;
}

dev_t loopcxt_get_devno(ସଂରଚନା loopdev_cxt *lc)
{
	ଯଦି (!lc || !loopcxt_has_device(lc))
		ଫେରନ୍ତୁ 0;
	ଯଦି (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	ଫେରନ୍ତୁ lc->devno;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_lost(ସଂରଚନା loopdev_cxt *lc)
{
	ଯଦି (!lc || !loopcxt_has_device(lc))
		ଫେରନ୍ତୁ 0;
	ଯଦି (lc->is_lost)
		ଫେରନ୍ତୁ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	ଫେରନ୍ତୁ lc->is_lost;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_init(ସଂରଚନା loopdev_cxt *lc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ସଂରଚନା stat st;
	ସଂରଚନା loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, ଆକାର(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, ଖାଲି);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	ଯଦି (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ଯଦି (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ଯଦି (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
ଶୂନ୍ୟ loopcxt_deinit(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା errsv = errno;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	ମୁକ୍ତ(lc->filename);
	lc->filename = ଖାଲି;

	ignore_result( loopcxt_set_device(lc, ଖାଲି) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
ଅକ୍ଷର *loopcxt_strdup_device(ସଂରଚନା loopdev_cxt *lc)
{
	ଯଦି (!lc || !*lc->device)
		ଫେରନ୍ତୁ ଖାଲି;
	ଫେରନ୍ତୁ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ଅଚଳ ଅକ୍ଷର *loopcxt_get_device(ସଂରଚନା loopdev_cxt *lc)
{
	ଫେରନ୍ତୁ lc && *lc->device ? lc->device : ଖାଲି;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ସ୍ଥିର ସଂରଚନା path_cxt *loopcxt_get_sysfs(ସଂରଚନା loopdev_cxt *lc)
{
	ଯଦି (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ଯଦି (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			ଫେରନ୍ତୁ ଖାଲି;
		}

		lc->sysfs = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
		ଯଦି (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	ଫେରନ୍ତୁ lc->sysfs;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __loopcxt_get_fd(ସଂରଚନା loopdev_cxt *lc, mode_t mode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old = -1;

	ଯଦି (!lc || !*lc->device)
		ଫେରନ୍ତୁ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ଯଦି (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ଯଦି (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		ଯଦି (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ଯଦି (old >= 0)
		close(old);
	ଫେରନ୍ତୁ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_fd(ସଂରଚନା loopdev_cxt *lc)
{
	ଫେରନ୍ତୁ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_init_iterator(ସଂରଚନା loopdev_cxt *lc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ସଂରଚନା loopdev_iter *iter;
	ସଂରଚନା stat st;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, ଆକାର(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ଯଦି (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ଯଦି (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_deinit_iterator(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା loopdev_iter *iter;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	ମୁକ୍ତ(iter->minors);
	ଯଦି (iter->proc)
		fclose(iter->proc);
	ଯଦି (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, ଆକାର(*iter));
	ଫେରନ୍ତୁ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopiter_set_device(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *device)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = loopcxt_set_device(lc, device);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା used;

	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	ଯଦି (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		ଫେରନ୍ତୁ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, ଖାଲି) == 0;

	ଯଦି ((lc->iter.flags & LOOPITER_FL_USED) && used)
		ଫେରନ୍ତୁ 0;

	ଯଦି ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		ଫେରନ୍ତୁ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, ଖାଲି) );
	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmpnum(ଅଚଳ ଶୂନ୍ୟ *p1, ଅଚଳ ଶୂନ୍ୟ *p2)
{
	ଫେରନ୍ତୁ (((* (ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) p1) > (* (ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) p2)) -
			((* (ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) p1) < (* (ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loop_scandir(ଅଚଳ ଅକ୍ଷର *dirname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା **ary, ପୂର୍ଣ୍ଣସଂଖ୍ୟା hasprefix)
{
	DIR *dir;
	ସଂରଚନା dirent *d;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଗଣନା_2 = 0, arylen = 0;

	ଯଦି (!dirname || !ary)
		ଫେରନ୍ତୁ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ 0;
	ମୁକ୍ତ(*ary);
	*ary = ଖାଲି;

	ଯେତେବେଳେ((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		ଯଦି (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ଜାରି;
#endif
		ଯଦି (is_dotdir_dirent(d))
			ଜାରି;

		ଯଦି (hasprefix) {
			/* /dev/loop<N> */
			ଯଦି (sscanf(d->d_name, "loop%u", &n) != 1)
				ଜାରି;
		} ଅନ୍ୟଥା {
			/* /dev/loop/<N> */
			ଅକ୍ଷର *end = ଖାଲି;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			ଯଦି (d->d_name == end || (end && *end) || errno)
				ଜାରି;
		}
		ଯଦି (n < LOOPDEV_DEFAULT_NNODES)
			ଜାରି;			/* ignore loop<0..7> */

		ଯଦି (ଗଣନା_2 + 1 > arylen) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
			ଯଦି (!tmp) {
				ମୁକ୍ତ(*ary);
				*ary = ଖାଲି;
				closedir(dir);
				ଫେରନ୍ତୁ -1;
			}
			*ary = tmp;
		}
		ଯଦି (*ary)
			(*ary)[ଗଣନା_2++] = n;
	}
	ଯଦି (ଗଣନା_2 && *ary)
		qsort(*ary, ଗଣନା_2, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା), cmpnum);

	closedir(dir);
	ଫେରନ୍ତୁ ଗଣନା_2;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_next_from_proc(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା loopdev_iter *iter = &lc->iter;
	ଅକ୍ଷର buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ଯଦି (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ଯଦି (!iter->proc)
		ଫେରନ୍ତୁ 1;

	ଯେତେବେଳେ (fgets(buf, ଆକାର(buf), iter->proc)) {
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା m;
		ଅକ୍ଷର name[128 + 1];


		ଯଦି (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ଜାରି;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ଯଦି (loopiter_set_device(lc, name) == 0)
			ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_next_from_sysfs(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା loopdev_iter *iter = &lc->iter;
	ସଂରଚନା dirent *d;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ଯଦି (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	ଯଦି (!iter->sysblock)
		ଫେରନ୍ତୁ 1;

	fd = dirfd(iter->sysblock);

	ଯେତେବେଳେ ((d = readdir(iter->sysblock))) {
		ଅକ୍ଷର name[NAME_MAX + 18 + 1];
		ସଂରଚନା stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ଯଦି (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			ଜାରି;

		snprintf(name, ଆକାର(name), "%s/loop/backing_file", d->d_name);
		ଯଦି (fstatat(fd, name, &st, 0) != 0)
			ଜାରି;

		ଯଦି (loopiter_set_device(lc, d->d_name) == 0)
			ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_next(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା loopdev_iter *iter;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;


	iter = &lc->iter;
	ଯଦି (iter->done)
		ଫେରନ୍ତୁ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ଯଦି (iter->flags & LOOPITER_FL_USED) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

		ଯଦି (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		ଅନ୍ୟଥା
			rc = loopcxt_next_from_proc(lc);
		ଯଦି (rc == 0)
			ଫେରନ୍ତୁ 0;
		ଯାଅ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ଯଦି (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		ପାଇଁ (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			ଅକ୍ଷର name[16];
			snprintf(name, ଆକାର(name), "loop%d", iter->ncur);

			ଯଦି (loopiter_set_device(lc, name) == 0)
				ଫେରନ୍ତୁ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ଯଦି (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	ପାଇଁ (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		ଅକ୍ଷର name[16];
		snprintf(name, ଆକାର(name), "loop%d", iter->minors[iter->ncur]);

		ଯଦି (loopiter_set_device(lc, name) == 0)
			ଫେରନ୍ତୁ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	ଫେରନ୍ତୁ 1;
}

/*
 * @device: path to device
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_loopdev(ଅଚଳ ଅକ୍ଷର *device)
{
	ସଂରଚନା stat st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ଅନ୍ୟଥା ଯଦି (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ଅନ୍ୟଥା ଯଦି (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		ଅକ୍ଷର name[PATH_MAX], *cn, *p = ଖାଲି;

		snprintf(name, ଆକାର(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		ଯଦି (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		ମୁକ୍ତ(cn);
	}

	ଯଦି (rc == 0)
		errno = ENODEV;
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ସଂରଚନା loop_info64 *loopcxt_get_info(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	ଯଦି (!lc || lc->info_failed) {
		errno = EINVAL;
		ଫେରନ୍ତୁ ଖାଲି;
	}
	errno = 0;
	ଯଦି (lc->has_info)
		ଫେରନ୍ତୁ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		ଫେରନ୍ତୁ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
ଅକ୍ଷର *loopcxt_get_backing_file(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	ଯଦି (!res && loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);

		ଯଦି (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((ଅକ୍ଷର *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	ଫେରନ୍ତୁ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
ଅକ୍ଷର *loopcxt_get_refname(ସଂରଚନା loopdev_cxt *lc)
{
	ଅକ୍ଷର *res = ଖାଲି;
	ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);

	ଯଦି (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((ଅକ୍ଷର *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	ଫେରନ୍ତୁ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_offset(ସଂରଚନା loopdev_cxt *lc, uint64_t *offset)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;

	ଯଦି (sysfs)
		ଯଦି (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	ଯଦି (rc && loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
		ଯଦି (lo) {
			ଯଦି (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} ଅନ୍ୟଥା
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_blocksize(ସଂରଚନା loopdev_cxt *lc, uint64_t *blocksize)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;

	ଯଦି (sysfs)
		ଯଦି (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ଯଦି (rc) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = loopcxt_get_fd(lc);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = 0;

		ଯଦି (fd < 0)
			ଫେରନ୍ତୁ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_sizelimit(ସଂରଚନା loopdev_cxt *lc, uint64_t *size)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;

	ଯଦି (sysfs)
		ଯଦି (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	ଯଦି (rc && loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
		ଯଦି (lo) {
			ଯଦି (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} ଅନ୍ୟଥା
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_encrypt_type(ସଂରଚନା loopdev_cxt *lc, uint32_t *type)
{
	ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	/* not provided by sysfs */
	ଯଦି (lo) {
		ଯଦି (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ଅନ୍ୟଥା
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ଅଚଳ ଅକ୍ଷର *loopcxt_get_crypt_name(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);

	ଯଦି (lo)
		ଫେରନ୍ତୁ (ଅକ୍ଷର *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_backing_devno(ସଂରଚନା loopdev_cxt *lc, dev_t *devno)
{
	ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (lo) {
		ଯଦି (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ଅନ୍ୟଥା
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_backing_inode(ସଂରଚନା loopdev_cxt *lc, ino_t *ino)
{
	ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (lo) {
		ଯଦି (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ଅନ୍ୟଥା
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopmod_supports_partscan(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, ret = 0;
	FILE *f;

	ଯଦି (get_linux_version() >= KERNEL_VERSION(3,2,0))
		ଫେରନ୍ତୁ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ଯଦି (!f)
		ଫେରନ୍ତୁ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	ଫେରନ୍ତୁ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_partscan(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ଯଦି (sysfs) {
		/* kernel >= 3.2 */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fl;
		ଯଦି (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			ଫେରନ୍ତୁ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	ଫେରନ୍ତୁ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_autoclear(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ଯଦି (sysfs) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fl;
		ଯଦି (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			ଫେରନ୍ତୁ fl;
	}

	ଯଦି (loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
		ଯଦି (lo)
			ଫେରନ୍ତୁ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_readonly(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ଯଦି (sysfs) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fl;
		ଯଦି (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			ଫେରନ୍ତୁ fl;
	}

	ଯଦି (loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
		ଯଦି (lo)
			ଫେରନ୍ତୁ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_dio(ସଂରଚନା loopdev_cxt *lc)
{
	ସଂରଚନା path_cxt *sysfs = loopcxt_get_sysfs(lc);

	ଯଦି (sysfs) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fl;
		ଯଦି (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			ଫେରନ୍ତୁ fl;
	}
	ଯଦି (loopcxt_ioctl_enabled(lc)) {
		ସଂରଚନା loop_info64 *lo = loopcxt_get_info(lc);
		ଯଦି (lo)
			ଫେରନ୍ତୁ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_is_used(ସଂରଚନା loopdev_cxt *lc,
		    ସଂରଚନା stat *st,
		    ଅଚଳ ଅକ୍ଷର *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ଯଦି (!lc)
		ଫେରନ୍ତୁ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ଯଦି (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ଯଦି (ino == st->st_ino && dev == st->st_dev)
			ଯାଅ found;

		/* don't use filename if we have devno and inode */
		ଫେରନ୍ତୁ 0;
	}

	/* poor man's solution */
	ଯଦି (backing_file) {
		ଅକ୍ଷର *name = loopcxt_get_backing_file(lc);
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = name && strcmp(name, backing_file) == 0;

		ମୁକ୍ତ(name);
		ଯଦି (rc)
			ଯାଅ found;
	}

	ଫେରନ୍ତୁ 0;
found:
	ଯଦି (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		ଯଦି (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			ଫେରନ୍ତୁ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		ଫେରନ୍ତୁ rc;
	}
	ଫେରନ୍ତୁ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_offset(ସଂରଚନା loopdev_cxt *lc, uint64_t offset)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	ଫେରନ୍ତୁ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_sizelimit(ସଂରଚନା loopdev_cxt *lc, uint64_t sizelimit)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	ଫେରନ୍ତୁ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_blocksize(ସଂରଚନା loopdev_cxt *lc, uint64_t blocksize)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_flags(ସଂରଚନା loopdev_cxt *lc, uint32_t flags)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_refname(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *refname)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, ଆକାର(lc->config.info.lo_file_name));
	ଯଦି (refname)
		xstrncpy((ଅକ୍ଷର *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (ଅକ୍ଷର *)lc->config.info.lo_file_name));
	ଫେରନ୍ତୁ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_set_backing_file(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *filename)
{
	ଯଦି (!lc)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ଅନ୍ୟଥା
		lc->filename = ul_canonicalize_path(filename);
	ଯଦି (!lc->filename)
		ଫେରନ୍ତୁ -errno;

	ଯଦି (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_check_size(ସଂରଚନା loopdev_cxt *lc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_fd)
{
	uint64_t size, expected_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev_fd;
	ସଂରଚନା stat st;

	ଯଦି (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		ଫେରନ୍ତୁ 0;

	ଯଦି (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		ଫେରନ୍ତୁ -errno;
	}
	ଯଦି (S_ISBLK(st.st_mode)) {
		ଯଦି (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			ଫେରନ୍ତୁ -errno;
		}
	} ଅନ୍ୟଥା
		expected_size = st.st_size;

	ଯଦି (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		ଫେରନ୍ତୁ 0;	/* ignore this error */
	}

	ଯଦି (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	ଯଦି (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ଯଦି (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		ଫେରନ୍ତୁ -errno;
	}

	ଯଦି (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		ଫେରନ୍ତୁ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ଯଦି (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	ଯଦି (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		ଯଦି (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			ଯଦି (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			ଫେରନ୍ତୁ -errno;
		}

		ଯଦି (blkdev_get_size(dev_fd, &size))
			ଫେରନ୍ତୁ -errno;

		ଯଦି (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			ଫେରନ୍ତୁ -errno;
		}
	}

	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_setup_device(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -1, cnt = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା errsv = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fallback = 0;

	ଯଦି (!lc || !*lc->device || !lc->filename)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ଯଦି (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	ଯଦି (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	ଯଦି (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	ଅନ୍ୟଥା
		file_fd = open(lc->filename, mode | flags);

	ଯଦି (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		ଯଦି (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		ଅନ୍ୟଥା
			file_fd = open(lc->filename, mode | flags);
	}
	ଯଦି (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		ଫେରନ୍ତୁ -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	ଯଦି (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ଅନ୍ୟଥା
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	କର {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		ଯଦି (dev_fd >= 0 || lc->control_ok == 0)
			ଭାଙ୍ଗ;
		ଯଦି (errno != EACCES && errno != ENOENT)
			ଭାଙ୍ଗ;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ଯେତେବେଳେ (cnt++ < 16);

	ଯଦି (dev_fd < 0) {
		rc = -errno;
		ଯାଅ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ଯଦି (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ଯଦି (rc != 0) {
		errsv = errno;
		ଯଦି (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			ଯାଅ err;
		}
		fallback = 1;
	} ଅନ୍ୟଥା {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ଯଦି (fallback) {
		ଯଦି (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			ଯାଅ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		ଯଦି (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			ଯାଅ err;
		}

		ଯଦି ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			ଯାଅ err;
		}
	}

	ଯଦି ((rc = loopcxt_check_size(lc, file_fd)))
		ଯାଅ err;

	close(file_fd);

	memset(&lc->config, 0, ଆକାର(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	ଫେରନ୍ତୁ 0;
err:
	ଯଦି (file_fd >= 0)
		close(file_fd);
	ଯଦି (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ଯଦି (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_ioctl_status(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ଯଦି (dev_fd < 0)
		ଫେରନ୍ତୁ -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	ଯଦି (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		ଫେରନ୍ତୁ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_ioctl_capacity(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, fd = loopcxt_get_fd(lc);

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	ଯଦି (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		ଫେରନ୍ତୁ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_ioctl_dio(ସଂରଚନା loopdev_cxt *lc, unsigned long use_dio)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, fd = loopcxt_get_fd(lc);

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	ଯଦି (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		ଫେରନ୍ତୁ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	ଫେରନ୍ତୁ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_ioctl_blocksize(ସଂରଚନା loopdev_cxt *lc, uint64_t blocksize)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, fd = loopcxt_get_fd(lc);

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	ଯଦି (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		ଫେରନ୍ତୁ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_get_device_nr(ସଂରଚନା loopdev_cxt *lc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nr)
{
	ଅଚଳ ଅକ୍ଷର *p, *dev;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;

	errno = 0;
	ଯଦି (!lc || !nr)
		ଫେରନ୍ତୁ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ଯଦି (!dev)
		ଯାଅ done;

	p = strrchr(dev, '/');
	ଯଦି (!p)
		ଯାଅ done;

	ଯଦି (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		ଯାଅ done;

	ଯଦି (*nr < 0)
		ଯାଅ done;
	rc = 0;
done:
	ଯଦି (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_detach_device(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, fd = loopcxt_get_fd(lc);

	ଯଦି (fd < 0)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ଯଦି (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		ଫେରନ୍ତୁ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_remove_device(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctl, nr = -1;

	ଯଦି (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ଯାଅ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ଯଦି (rc)
		ଯାଅ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ଯଦି (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_add_device(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctl, nr = -1;

	ଯଦି (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		ଯାଅ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ଯଦି (rc)
		ଯାଅ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ଯଦି (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_find_unused(ସଂରଚନା loopdev_cxt *lc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ଯଦି (lc->flags & LOOPDEV_FL_CONTROL) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ଯଦି (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ଅନ୍ୟଥା
			rc = -errno;
		ଯଦି (rc >= 0) {
			ଅକ୍ଷର name[16];
			snprintf(name, ଆକାର(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ଯଦି (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ଯଦି (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ଯଦି (rc)
			ଫେରନ୍ତୁ -ENOENT;
	}
	ଫେରନ୍ତୁ rc;
}



/*
 * Return: TRUE/FALSE
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopdev_is_autoclear(ଅଚଳ ଅକ୍ଷର *device)
{
	ସଂରଚନା loopdev_cxt lc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!device)
		ଫେରନ୍ତୁ 0;

	rc = loopcxt_init(&lc, 0);
	ଯଦି (!rc)
		rc = loopcxt_set_device(&lc, device);
	ଯଦି (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	ଫେରନ୍ତୁ rc;
}

ଅକ୍ଷର *loopdev_get_backing_file(ଅଚଳ ଅକ୍ଷର *device)
{
	ସଂରଚନା loopdev_cxt lc;
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (!device)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (loopcxt_init(&lc, 0))
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	ଫେରନ୍ତୁ res;
}

/*
 * Returns: TRUE/FALSE
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopdev_has_backing_file(ଅଚଳ ଅକ୍ଷର *device)
{
	ଅକ୍ଷର *tmp = loopdev_get_backing_file(device);

	ଯଦି (tmp) {
		ମୁକ୍ତ(tmp);
		ଫେରନ୍ତୁ 1;
	}
	ଫେରନ୍ତୁ 0;
}

/*
 * Returns: TRUE/FALSE
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopdev_is_used(ଅଚଳ ଅକ୍ଷର *device, ଅଚଳ ଅକ୍ଷର *filename,
		    uint64_t offset, uint64_t sizelimit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ସଂରଚନା loopdev_cxt lc;
	ସଂରଚନା stat st;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (!device || !filename)
		ଫେରନ୍ତୁ 0;

	rc = loopcxt_init(&lc, 0);
	ଯଦି (!rc)
		rc = loopcxt_set_device(&lc, device);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : ଖାଲି, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopdev_detach(ଅଚଳ ଅକ୍ଷର *device)
{
	ସଂରଚନା loopdev_cxt lc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!device)
		ଫେରନ୍ତୁ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ଯଦି (!rc)
		rc = loopcxt_set_device(&lc, device);
	ଯଦି (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_find_by_backing_file(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *filename,
				 uint64_t offset, uint64_t sizelimit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, hasst;
	ସଂରଚନା stat st;

	ଯଦି (!filename)
		ଫେରନ୍ତୁ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	ଯେତେବେଳେ ((rc = loopcxt_next(lc)) == 0) {

		ଯଦି (loopcxt_is_used(lc, hasst ? &st : ଖାଲି,
				    filename, offset, sizelimit, flags))
			ଭାଙ୍ଗ;
	}

	loopcxt_deinit_iterator(lc);
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopcxt_find_overlap(ସଂରଚନା loopdev_cxt *lc, ଅଚଳ ଅକ୍ଷର *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, hasst;
	ସଂରଚନା stat st;

	ଯଦି (!filename)
		ଫେରନ୍ତୁ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	ଯେତେବେଳେ ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : ଖାଲି,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ଯଦି (rc <= 0)
			ଜାରି;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		ଯଦି (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ଜାରି;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ଯଦି (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ଜାରି;
		}

		/* full match */
		ଯଦି (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			ଯାଅ found;
		}

		/* overlap */
		ଯଦି (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			ଜାରି;
		ଯଦି (sizelimit != 0 && offset + sizelimit <= lc_offset)
			ଜାରି;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			ଯାଅ found;
	}

	ଯଦି (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns allocated string with device name
 */
ଅକ୍ଷର *loopdev_find_by_backing_file(ଅଚଳ ଅକ୍ଷର *filename, uint64_t offset, uint64_t sizelimit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ସଂରଚନା loopdev_cxt lc;
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (!filename)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (loopcxt_init(&lc, 0))
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	ଫେରନ୍ତୁ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା loopdev_ଗଣନା_by_backing_file(ଅଚଳ ଅକ୍ଷର *filename, ଅକ୍ଷର **loopdev)
{
	ସଂରଚନା loopdev_cxt lc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0, rc;

	ଯଦି (!filename)
		ଫେରନ୍ତୁ -1;

	rc = loopcxt_init(&lc, 0);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;
	ଯଦି (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		ଫେରନ୍ତୁ -1;

	ଯେତେବେଳେ(loopcxt_next(&lc) == 0) {
		ଅକ୍ଷର *backing = loopcxt_get_backing_file(&lc);

		ଯଦି (!backing || strcmp(backing, filename) != 0) {
			ମୁକ୍ତ(backing);
			ଜାରି;
		}

		ମୁକ୍ତ(backing);
		ଯଦି (loopdev && ଗଣନା_2 == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		ଗଣନା_2++;
	}

	loopcxt_deinit(&lc);

	ଯଦି (loopdev && ଗଣନା_2 > 1) {
		ମୁକ୍ତ(*loopdev);
		*loopdev = ଖାଲି;
	}
	ଫେରନ୍ତୁ ଗଣନା_2;
}

#ifdef TEST_PROGRAM_LOOPDEV
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଯଦି (argc < 2)
		ଯାଅ usage;

	ଯଦି (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ଅନ୍ୟଥା
		ଯାଅ usage;

	ଫେରନ୍ତୁ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif

