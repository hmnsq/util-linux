
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
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/mman.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <dirent.h>

#અન્તર્ભાવ "linux_version.h"
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "sysfs.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "loopdev.h"
#અન્તર્ભાવ "canonicalize.h"
#અન્તર્ભાવ "blkdev.h"
#અન્તર્ભાવ "debug.h"
#અન્તર્ભાવ "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
સ્થિર UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

સ્થિર શૂન્ય loopdev_init_debug(શૂન્ય)
{
	જો (loopdev_debug_mask)
		પાછા;
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
		પૂર્ણાંક _c = 0, _e;					\
		કરો {						\
			errno = 0;				\
			_e = x;					\
			જો (_e == 0 || errno != EAGAIN)		\
				તોડો;				\
			જો (_c >= LOOPDEV_MAX_TRIES)		\
				તોડો;				\
			xusleep(250000);			\
			_c++;					\
		} જ્યારે (1);					\
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
પૂર્ણાંક loopcxt_set_device(સંરચના loopdev_cxt *lc, અચલ અક્ષર *device)
{
	જો (!lc)
		પાછા -EINVAL;

	જો (lc->fd >= 0) {
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
	memset(&lc->config, 0, કદ(lc->config));

	/* set new */
	જો (device) {
		જો (*device != '/') {
			અચલ અક્ષર *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			જો (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				જો (strlen(device) < 5)
					પાછા -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, કદ(lc->device), "%s%s",
				dir, device);
		} નહીંતર
			xstrncpy(lc->device, device, કદ(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = ખાલી;
	પાછા 0;
}

પૂર્ણાંક loopcxt_has_device(સંરચના loopdev_cxt *lc)
{
	પાછા lc && *lc->device;
}

dev_t loopcxt_get_devno(સંરચના loopdev_cxt *lc)
{
	જો (!lc || !loopcxt_has_device(lc))
		પાછા 0;
	જો (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	પાછા lc->devno;
}

પૂર્ણાંક loopcxt_is_lost(સંરચના loopdev_cxt *lc)
{
	જો (!lc || !loopcxt_has_device(lc))
		પાછા 0;
	જો (lc->is_lost)
		પાછા 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	પાછા lc->is_lost;
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
પૂર્ણાંક loopcxt_init(સંરચના loopdev_cxt *lc, પૂર્ણાંક flags)
{
	પૂર્ણાંક rc;
	સંરચના stat st;
	સંરચના loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	જો (!lc)
		પાછા -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, કદ(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, ખાલી);
	જો (rc)
		પાછા rc;

	જો (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	જો (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	જો (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	પાછા 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
શૂન્ય loopcxt_deinit(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક errsv = errno;

	જો (!lc)
		પાછા;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	મુક્ત(lc->filename);
	lc->filename = ખાલી;

	ignore_result( loopcxt_set_device(lc, ખાલી) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
અક્ષર *loopcxt_strdup_device(સંરચના loopdev_cxt *lc)
{
	જો (!lc || !*lc->device)
		પાછા ખાલી;
	પાછા strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
અચલ અક્ષર *loopcxt_get_device(સંરચના loopdev_cxt *lc)
{
	પાછા lc && *lc->device ? lc->device : ખાલી;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
સ્થિર સંરચના path_cxt *loopcxt_get_sysfs(સંરચના loopdev_cxt *lc)
{
	જો (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		પાછા ખાલી;

	જો (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		જો (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			પાછા ખાલી;
		}

		lc->sysfs = ul_new_sysfs_path(devno, ખાલી, ખાલી);
		જો (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	પાછા lc->sysfs;
}

સ્થિર પૂર્ણાંક __loopcxt_get_fd(સંરચના loopdev_cxt *lc, mode_t mode)
{
	પૂર્ણાંક old = -1;

	જો (!lc || !*lc->device)
		પાછા -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	જો (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	જો (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		જો (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	જો (old >= 0)
		close(old);
	પાછા lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
પૂર્ણાંક loopcxt_get_fd(સંરચના loopdev_cxt *lc)
{
	પાછા __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_init_iterator(સંરચના loopdev_cxt *lc, પૂર્ણાંક flags)
{
	સંરચના loopdev_iter *iter;
	સંરચના stat st;

	જો (!lc)
		પાછા -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, કદ(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	જો (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		જો (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	પાછા 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_deinit_iterator(સંરચના loopdev_cxt *lc)
{
	સંરચના loopdev_iter *iter;

	જો (!lc)
		પાછા -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	મુક્ત(iter->minors);
	જો (iter->proc)
		fclose(iter->proc);
	જો (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, કદ(*iter));
	પાછા 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
સ્થિર પૂર્ણાંક loopiter_set_device(સંરચના loopdev_cxt *lc, અચલ અક્ષર *device)
{
	પૂર્ણાંક rc = loopcxt_set_device(lc, device);
	પૂર્ણાંક used;

	જો (rc)
		પાછા rc;

	જો (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		પાછા 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, ખાલી) == 0;

	જો ((lc->iter.flags & LOOPITER_FL_USED) && used)
		પાછા 0;

	જો ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		પાછા 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, ખાલી) );
	પાછા 1;
}

સ્થિર પૂર્ણાંક cmpnum(અચલ શૂન્ય *p1, અચલ શૂન્ય *p2)
{
	પાછા (((* (અચલ પૂર્ણાંક *) p1) > (* (અચલ પૂર્ણાંક *) p2)) -
			((* (અચલ પૂર્ણાંક *) p1) < (* (અચલ પૂર્ણાંક *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
સ્થિર પૂર્ણાંક loop_scandir(અચલ અક્ષર *dirname, પૂર્ણાંક **ary, પૂર્ણાંક hasprefix)
{
	DIR *dir;
	સંરચના dirent *d;
	unsigned પૂર્ણાંક n, ગણતરી = 0, arylen = 0;

	જો (!dirname || !ary)
		પાછા 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	જો (!dir)
		પાછા 0;
	મુક્ત(*ary);
	*ary = ખાલી;

	જ્યારે((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		જો (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			ચાલુ;
#endif
		જો (is_dotdir_dirent(d))
			ચાલુ;

		જો (hasprefix) {
			/* /dev/loop<N> */
			જો (sscanf(d->d_name, "loop%u", &n) != 1)
				ચાલુ;
		} નહીંતર {
			/* /dev/loop/<N> */
			અક્ષર *end = ખાલી;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			જો (d->d_name == end || (end && *end) || errno)
				ચાલુ;
		}
		જો (n < LOOPDEV_DEFAULT_NNODES)
			ચાલુ;			/* ignore loop<0..7> */

		જો (ગણતરી + 1 > arylen) {
			પૂર્ણાંક *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, કદ(પૂર્ણાંક));
			જો (!tmp) {
				મુક્ત(*ary);
				*ary = ખાલી;
				closedir(dir);
				પાછા -1;
			}
			*ary = tmp;
		}
		જો (*ary)
			(*ary)[ગણતરી++] = n;
	}
	જો (ગણતરી && *ary)
		qsort(*ary, ગણતરી, કદ(પૂર્ણાંક), cmpnum);

	closedir(dir);
	પાછા ગણતરી;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
સ્થિર પૂર્ણાંક loopcxt_next_from_proc(સંરચના loopdev_cxt *lc)
{
	સંરચના loopdev_iter *iter = &lc->iter;
	અક્ષર buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	જો (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	જો (!iter->proc)
		પાછા 1;

	જ્યારે (fgets(buf, કદ(buf), iter->proc)) {
		unsigned પૂર્ણાંક m;
		અક્ષર name[128 + 1];


		જો (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			ચાલુ;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		જો (loopiter_set_device(lc, name) == 0)
			પાછા 0;
	}

	પાછા 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
સ્થિર પૂર્ણાંક loopcxt_next_from_sysfs(સંરચના loopdev_cxt *lc)
{
	સંરચના loopdev_iter *iter = &lc->iter;
	સંરચના dirent *d;
	પૂર્ણાંક fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	જો (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	જો (!iter->sysblock)
		પાછા 1;

	fd = dirfd(iter->sysblock);

	જ્યારે ((d = readdir(iter->sysblock))) {
		અક્ષર name[NAME_MAX + 18 + 1];
		સંરચના stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		જો (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			ચાલુ;

		snprintf(name, કદ(name), "%s/loop/backing_file", d->d_name);
		જો (fstatat(fd, name, &st, 0) != 0)
			ચાલુ;

		જો (loopiter_set_device(lc, d->d_name) == 0)
			પાછા 0;
	}

	પાછા 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
પૂર્ણાંક loopcxt_next(સંરચના loopdev_cxt *lc)
{
	સંરચના loopdev_iter *iter;

	જો (!lc)
		પાછા -EINVAL;


	iter = &lc->iter;
	જો (iter->done)
		પાછા 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	જો (iter->flags & LOOPITER_FL_USED) {
		પૂર્ણાંક rc;

		જો (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		નહીંતર
			rc = loopcxt_next_from_proc(lc);
		જો (rc == 0)
			પાછા 0;
		જાઓ done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	જો (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		માટે (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			અક્ષર name[16];
			snprintf(name, કદ(name), "loop%d", iter->ncur);

			જો (loopiter_set_device(lc, name) == 0)
				પાછા 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	જો (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	માટે (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		અક્ષર name[16];
		snprintf(name, કદ(name), "loop%d", iter->minors[iter->ncur]);

		જો (loopiter_set_device(lc, name) == 0)
			પાછા 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	પાછા 1;
}

/*
 * @device: path to device
 */
પૂર્ણાંક is_loopdev(અચલ અક્ષર *device)
{
	સંરચના stat st;
	પૂર્ણાંક rc = 0;

	જો (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	નહીંતર જો (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	નહીંતર જો (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		અક્ષર name[PATH_MAX], *cn, *p = ખાલી;

		snprintf(name, કદ(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		જો (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		મુક્ત(cn);
	}

	જો (rc == 0)
		errno = ENODEV;
	પાછા rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
સંરચના loop_info64 *loopcxt_get_info(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક fd;

	જો (!lc || lc->info_failed) {
		errno = EINVAL;
		પાછા ખાલી;
	}
	errno = 0;
	જો (lc->has_info)
		પાછા &lc->config.info;

	fd = loopcxt_get_fd(lc);
	જો (fd < 0)
		પાછા ખાલી;

	જો (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		પાછા &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	પાછા ખાલી;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
અક્ષર *loopcxt_get_backing_file(સંરચના loopdev_cxt *lc)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);
	અક્ષર *res = ખાલી;

	જો (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	જો (!res && loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);

		જો (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((અક્ષર *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	પાછા res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
અક્ષર *loopcxt_get_refname(સંરચના loopdev_cxt *lc)
{
	અક્ષર *res = ખાલી;
	સંરચના loop_info64 *lo = loopcxt_get_info(lc);

	જો (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((અક્ષર *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	પાછા res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_offset(સંરચના loopdev_cxt *lc, uint64_t *offset)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);
	પૂર્ણાંક rc = -EINVAL;

	જો (sysfs)
		જો (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	જો (rc && loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);
		જો (lo) {
			જો (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} નહીંતર
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	પાછા rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_blocksize(સંરચના loopdev_cxt *lc, uint64_t *blocksize)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);
	પૂર્ણાંક rc = -EINVAL;

	જો (sysfs)
		જો (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	જો (rc) {
		પૂર્ણાંક fd = loopcxt_get_fd(lc);
		પૂર્ણાંક sz = 0;

		જો (fd < 0)
			પાછા -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		જો (rc)
			પાછા rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	પાછા rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_sizelimit(સંરચના loopdev_cxt *lc, uint64_t *size)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);
	પૂર્ણાંક rc = -EINVAL;

	જો (sysfs)
		જો (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	જો (rc && loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);
		જો (lo) {
			જો (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} નહીંતર
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	પાછા rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_encrypt_type(સંરચના loopdev_cxt *lc, uint32_t *type)
{
	સંરચના loop_info64 *lo = loopcxt_get_info(lc);
	પૂર્ણાંક rc;

	/* not provided by sysfs */
	જો (lo) {
		જો (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} નહીંતર
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	પાછા rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
અચલ અક્ષર *loopcxt_get_crypt_name(સંરચના loopdev_cxt *lc)
{
	સંરચના loop_info64 *lo = loopcxt_get_info(lc);

	જો (lo)
		પાછા (અક્ષર *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	પાછા ખાલી;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_backing_devno(સંરચના loopdev_cxt *lc, dev_t *devno)
{
	સંરચના loop_info64 *lo = loopcxt_get_info(lc);
	પૂર્ણાંક rc;

	જો (lo) {
		જો (devno)
			*devno = lo->lo_device;
		rc = 0;
	} નહીંતર
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	પાછા rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
પૂર્ણાંક loopcxt_get_backing_inode(સંરચના loopdev_cxt *lc, ino_t *ino)
{
	સંરચના loop_info64 *lo = loopcxt_get_info(lc);
	પૂર્ણાંક rc;

	જો (lo) {
		જો (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} નહીંતર
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	પાછા rc;
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
પૂર્ણાંક loopmod_supports_partscan(શૂન્ય)
{
	પૂર્ણાંક rc, ret = 0;
	FILE *f;

	જો (get_linux_version() >= KERNEL_VERSION(3,2,0))
		પાછા 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	જો (!f)
		પાછા 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	પાછા rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
પૂર્ણાંક loopcxt_is_partscan(સંરચના loopdev_cxt *lc)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);

	જો (sysfs) {
		/* kernel >= 3.2 */
		પૂર્ણાંક fl;
		જો (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			પાછા fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	પાછા loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
પૂર્ણાંક loopcxt_is_autoclear(સંરચના loopdev_cxt *lc)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);

	જો (sysfs) {
		પૂર્ણાંક fl;
		જો (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			પાછા fl;
	}

	જો (loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);
		જો (lo)
			પાછા lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	પાછા 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
પૂર્ણાંક loopcxt_is_readonly(સંરચના loopdev_cxt *lc)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);

	જો (sysfs) {
		પૂર્ણાંક fl;
		જો (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			પાછા fl;
	}

	જો (loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);
		જો (lo)
			પાછા lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	પાછા 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
પૂર્ણાંક loopcxt_is_dio(સંરચના loopdev_cxt *lc)
{
	સંરચના path_cxt *sysfs = loopcxt_get_sysfs(lc);

	જો (sysfs) {
		પૂર્ણાંક fl;
		જો (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			પાછા fl;
	}
	જો (loopcxt_ioctl_enabled(lc)) {
		સંરચના loop_info64 *lo = loopcxt_get_info(lc);
		જો (lo)
			પાછા lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	પાછા 0;
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
પૂર્ણાંક loopcxt_is_used(સંરચના loopdev_cxt *lc,
		    સંરચના stat *st,
		    અચલ અક્ષર *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    પૂર્ણાંક flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	જો (!lc)
		પાછા 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	જો (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		જો (ino == st->st_ino && dev == st->st_dev)
			જાઓ found;

		/* don't use filename if we have devno and inode */
		પાછા 0;
	}

	/* poor man's solution */
	જો (backing_file) {
		અક્ષર *name = loopcxt_get_backing_file(lc);
		પૂર્ણાંક rc = name && strcmp(name, backing_file) == 0;

		મુક્ત(name);
		જો (rc)
			જાઓ found;
	}

	પાછા 0;
found:
	જો (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		પૂર્ણાંક rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		જો (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			પાછા loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		પાછા rc;
	}
	પાછા 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
પૂર્ણાંક loopcxt_set_offset(સંરચના loopdev_cxt *lc, uint64_t offset)
{
	જો (!lc)
		પાછા -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	પાછા 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
પૂર્ણાંક loopcxt_set_sizelimit(સંરચના loopdev_cxt *lc, uint64_t sizelimit)
{
	જો (!lc)
		પાછા -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	પાછા 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
પૂર્ણાંક loopcxt_set_blocksize(સંરચના loopdev_cxt *lc, uint64_t blocksize)
{
	જો (!lc)
		પાછા -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	પાછા 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
પૂર્ણાંક loopcxt_set_flags(સંરચના loopdev_cxt *lc, uint32_t flags)
{
	જો (!lc)
		પાછા -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	પાછા 0;
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
પૂર્ણાંક loopcxt_set_refname(સંરચના loopdev_cxt *lc, અચલ અક્ષર *refname)
{
	જો (!lc)
		પાછા -EINVAL;

	memset(lc->config.info.lo_file_name, 0, કદ(lc->config.info.lo_file_name));
	જો (refname)
		xstrncpy((અક્ષર *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (અક્ષર *)lc->config.info.lo_file_name));
	પાછા 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
પૂર્ણાંક loopcxt_set_backing_file(સંરચના loopdev_cxt *lc, અચલ અક્ષર *filename)
{
	જો (!lc)
		પાછા -EINVAL;

	જો (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	નહીંતર
		lc->filename = ul_canonicalize_path(filename);
	જો (!lc->filename)
		પાછા -errno;

	જો (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	પાછા 0;
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
સ્થિર પૂર્ણાંક loopcxt_check_size(સંરચના loopdev_cxt *lc, પૂર્ણાંક file_fd)
{
	uint64_t size, expected_size;
	પૂર્ણાંક dev_fd;
	સંરચના stat st;

	જો (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		પાછા 0;

	જો (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		પાછા -errno;
	}
	જો (S_ISBLK(st.st_mode)) {
		જો (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			પાછા -errno;
		}
	} નહીંતર
		expected_size = st.st_size;

	જો (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		પાછા 0;	/* ignore this error */
	}

	જો (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	જો (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	જો (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		પાછા -errno;
	}

	જો (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		પાછા -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	જો (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	જો (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		જો (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			જો (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			પાછા -errno;
		}

		જો (blkdev_get_size(dev_fd, &size))
			પાછા -errno;

		જો (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			પાછા -errno;
		}
	}

	પાછા 0;
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
પૂર્ણાંક loopcxt_setup_device(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	પૂર્ણાંક rc = -1, cnt = 0;
	પૂર્ણાંક errsv = 0;
	પૂર્ણાંક fallback = 0;

	જો (!lc || !*lc->device || !lc->filename)
		પાછા -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	જો (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	જો (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	જો (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	નહીંતર
		file_fd = open(lc->filename, mode | flags);

	જો (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		જો (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		નહીંતર
			file_fd = open(lc->filename, mode | flags);
	}
	જો (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		પાછા -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	જો (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	નહીંતર
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	કરો {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		જો (dev_fd >= 0 || lc->control_ok == 0)
			તોડો;
		જો (errno != EACCES && errno != ENOENT)
			તોડો;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} જ્યારે (cnt++ < 16);

	જો (dev_fd < 0) {
		rc = -errno;
		જાઓ err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	જો (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	જો (rc != 0) {
		errsv = errno;
		જો (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			જાઓ err;
		}
		fallback = 1;
	} નહીંતર {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	જો (fallback) {
		જો (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			જાઓ err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		જો (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			જાઓ err;
		}

		જો ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			જાઓ err;
		}
	}

	જો ((rc = loopcxt_check_size(lc, file_fd)))
		જાઓ err;

	close(file_fd);

	memset(&lc->config, 0, કદ(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	પાછા 0;
err:
	જો (file_fd >= 0)
		close(file_fd);
	જો (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	જો (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	પાછા rc;
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
પૂર્ણાંક loopcxt_ioctl_status(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	જો (dev_fd < 0)
		પાછા -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	જો (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		પાછા rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	પાછા 0;
}

પૂર્ણાંક loopcxt_ioctl_capacity(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક rc, fd = loopcxt_get_fd(lc);

	જો (fd < 0)
		પાછા -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	જો (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		પાછા rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	પાછા 0;
}

પૂર્ણાંક loopcxt_ioctl_dio(સંરચના loopdev_cxt *lc, unsigned long use_dio)
{
	પૂર્ણાંક rc, fd = loopcxt_get_fd(lc);

	જો (fd < 0)
		પાછા -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	જો (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		પાછા rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	પાછા 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
પૂર્ણાંક loopcxt_ioctl_blocksize(સંરચના loopdev_cxt *lc, uint64_t blocksize)
{
	પૂર્ણાંક rc, fd = loopcxt_get_fd(lc);

	જો (fd < 0)
		પાછા -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	જો (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		પાછા rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	પાછા 0;
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
સ્થિર પૂર્ણાંક loopcxt_get_device_nr(સંરચના loopdev_cxt *lc, પૂર્ણાંક *nr)
{
	અચલ અક્ષર *p, *dev;
	પૂર્ણાંક rc = -EINVAL;

	errno = 0;
	જો (!lc || !nr)
		પાછા rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	જો (!dev)
		જાઓ done;

	p = strrchr(dev, '/');
	જો (!p)
		જાઓ done;

	જો (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		જાઓ done;

	જો (*nr < 0)
		જાઓ done;
	rc = 0;
done:
	જો (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	પાછા rc;
}

પૂર્ણાંક loopcxt_detach_device(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક rc, fd = loopcxt_get_fd(lc);

	જો (fd < 0)
		પાછા -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	જો (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		પાછા rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	પાછા 0;
}

પૂર્ણાંક loopcxt_remove_device(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક rc = -EINVAL;
	પૂર્ણાંક ctl, nr = -1;

	જો (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		જાઓ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	જો (rc)
		જાઓ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	જો (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	પાછા rc;
}

પૂર્ણાંક loopcxt_add_device(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક rc = -EINVAL;
	પૂર્ણાંક ctl, nr = -1;

	જો (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		જાઓ done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	જો (rc)
		જાઓ done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	જો (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	પાછા rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
પૂર્ણાંક loopcxt_find_unused(સંરચના loopdev_cxt *lc)
{
	પૂર્ણાંક rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	જો (lc->flags & LOOPDEV_FL_CONTROL) {
		પૂર્ણાંક ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		જો (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		નહીંતર
			rc = -errno;
		જો (rc >= 0) {
			અક્ષર name[16];
			snprintf(name, કદ(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		જો (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	જો (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		જો (rc)
			પાછા rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		જો (rc)
			પાછા -ENOENT;
	}
	પાછા rc;
}



/*
 * Return: TRUE/FALSE
 */
પૂર્ણાંક loopdev_is_autoclear(અચલ અક્ષર *device)
{
	સંરચના loopdev_cxt lc;
	પૂર્ણાંક rc;

	જો (!device)
		પાછા 0;

	rc = loopcxt_init(&lc, 0);
	જો (!rc)
		rc = loopcxt_set_device(&lc, device);
	જો (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	પાછા rc;
}

અક્ષર *loopdev_get_backing_file(અચલ અક્ષર *device)
{
	સંરચના loopdev_cxt lc;
	અક્ષર *res = ખાલી;

	જો (!device)
		પાછા ખાલી;
	જો (loopcxt_init(&lc, 0))
		પાછા ખાલી;
	જો (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	પાછા res;
}

/*
 * Returns: TRUE/FALSE
 */
પૂર્ણાંક loopdev_has_backing_file(અચલ અક્ષર *device)
{
	અક્ષર *tmp = loopdev_get_backing_file(device);

	જો (tmp) {
		મુક્ત(tmp);
		પાછા 1;
	}
	પાછા 0;
}

/*
 * Returns: TRUE/FALSE
 */
પૂર્ણાંક loopdev_is_used(અચલ અક્ષર *device, અચલ અક્ષર *filename,
		    uint64_t offset, uint64_t sizelimit, પૂર્ણાંક flags)
{
	સંરચના loopdev_cxt lc;
	સંરચના stat st;
	પૂર્ણાંક rc = 0;

	જો (!device || !filename)
		પાછા 0;

	rc = loopcxt_init(&lc, 0);
	જો (!rc)
		rc = loopcxt_set_device(&lc, device);
	જો (rc)
		પાછા rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : ખાલી, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	પાછા rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
પૂર્ણાંક loopdev_detach(અચલ અક્ષર *device)
{
	સંરચના loopdev_cxt lc;
	પૂર્ણાંક rc;

	જો (!device)
		પાછા -EINVAL;

	rc = loopcxt_init(&lc, 0);
	જો (!rc)
		rc = loopcxt_set_device(&lc, device);
	જો (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	પાછા rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
પૂર્ણાંક loopcxt_find_by_backing_file(સંરચના loopdev_cxt *lc, અચલ અક્ષર *filename,
				 uint64_t offset, uint64_t sizelimit, પૂર્ણાંક flags)
{
	પૂર્ણાંક rc, hasst;
	સંરચના stat st;

	જો (!filename)
		પાછા -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	જો (rc)
		પાછા rc;

	જ્યારે ((rc = loopcxt_next(lc)) == 0) {

		જો (loopcxt_is_used(lc, hasst ? &st : ખાલી,
				    filename, offset, sizelimit, flags))
			તોડો;
	}

	loopcxt_deinit_iterator(lc);
	પાછા rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
પૂર્ણાંક loopcxt_find_overlap(સંરચના loopdev_cxt *lc, અચલ અક્ષર *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	પૂર્ણાંક rc, hasst;
	સંરચના stat st;

	જો (!filename)
		પાછા -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	જો (rc)
		પાછા rc;

	જ્યારે ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : ખાલી,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		જો (rc <= 0)
			ચાલુ;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		જો (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			ચાલુ;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		જો (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			ચાલુ;
		}

		/* full match */
		જો (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			જાઓ found;
		}

		/* overlap */
		જો (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			ચાલુ;
		જો (sizelimit != 0 && offset + sizelimit <= lc_offset)
			ચાલુ;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			જાઓ found;
	}

	જો (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	પાછા rc;
}

/*
 * Returns allocated string with device name
 */
અક્ષર *loopdev_find_by_backing_file(અચલ અક્ષર *filename, uint64_t offset, uint64_t sizelimit, પૂર્ણાંક flags)
{
	સંરચના loopdev_cxt lc;
	અક્ષર *res = ખાલી;

	જો (!filename)
		પાછા ખાલી;

	જો (loopcxt_init(&lc, 0))
		પાછા ખાલી;
	જો (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	પાછા res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
પૂર્ણાંક loopdev_ગણતરી_by_backing_file(અચલ અક્ષર *filename, અક્ષર **loopdev)
{
	સંરચના loopdev_cxt lc;
	પૂર્ણાંક ગણતરી = 0, rc;

	જો (!filename)
		પાછા -1;

	rc = loopcxt_init(&lc, 0);
	જો (rc)
		પાછા rc;
	જો (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		પાછા -1;

	જ્યારે(loopcxt_next(&lc) == 0) {
		અક્ષર *backing = loopcxt_get_backing_file(&lc);

		જો (!backing || strcmp(backing, filename) != 0) {
			મુક્ત(backing);
			ચાલુ;
		}

		મુક્ત(backing);
		જો (loopdev && ગણતરી == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		ગણતરી++;
	}

	loopcxt_deinit(&lc);

	જો (loopdev && ગણતરી > 1) {
		મુક્ત(*loopdev);
		*loopdev = ખાલી;
	}
	પાછા ગણતરી;
}

#ifdef TEST_PROGRAM_LOOPDEV
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	જો (argc < 2)
		જાઓ usage;

	જો (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	નહીંતર
		જાઓ usage;

	પાછા EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	પાછા EXIT_FAILURE;
}
#endif

