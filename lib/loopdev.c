
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
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/mman.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <dirent.h>

#അന്തര്ഭാവ "linux_version.h"
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "timeutils.h"
#അന്തര്ഭാവ "sysfs.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "loopdev.h"
#അന്തര്ഭാവ "canonicalize.h"
#അന്തര്ഭാവ "blkdev.h"
#അന്തര്ഭാവ "debug.h"
#അന്തര്ഭാവ "fileutils.h"

#define LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
സ്ഥിതം UL_DEBUG_DEFINE_MASK(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_EMPTY_MASKNAMES;

#define LOOPDEV_DEBUG_INIT	(1 << 1)
#define LOOPDEV_DEBUG_CXT	(1 << 2)
#define LOOPDEV_DEBUG_ITER	(1 << 3)
#define LOOPDEV_DEBUG_SETUP	(1 << 4)

#define DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

സ്ഥിതം ശൂന്യം loopdev_init_debug(ശൂന്യം)
{
	എങ്കിൽ (loopdev_debug_mask)
		തിരികെ;
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
		പൂർണ്ണസംഖ്യ _c = 0, _e;					\
		ചെയ്യുക {						\
			errno = 0;				\
			_e = x;					\
			എങ്കിൽ (_e == 0 || errno != EAGAIN)		\
				നിർത്തുക;				\
			എങ്കിൽ (_c >= LOOPDEV_MAX_TRIES)		\
				നിർത്തുക;				\
			xusleep(250000);			\
			_c++;					\
		} ആകുമ്പോൾ (1);					\
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
പൂർണ്ണസംഖ്യ loopcxt_set_device(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *device)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;

	എങ്കിൽ (lc->fd >= 0) {
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
	memset(&lc->config, 0, വലുപ്പം(lc->config));

	/* set new */
	എങ്കിൽ (device) {
		എങ്കിൽ (*device != '/') {
			സ്ഥിരം അക്ഷരം *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			എങ്കിൽ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				എങ്കിൽ (strlen(device) < 5)
					തിരികെ -1;
				device += 4;
				dir = _PATH_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, വലുപ്പം(lc->device), "%s%s",
				dir, device);
		} അല്ലെങ്കിൽ
			xstrncpy(lc->device, device, വലുപ്പം(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_path(lc->sysfs);
	lc->sysfs = ശൂന്യ;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ loopcxt_has_device(ഘടന loopdev_cxt *lc)
{
	തിരികെ lc && *lc->device;
}

dev_t loopcxt_get_devno(ഘടന loopdev_cxt *lc)
{
	എങ്കിൽ (!lc || !loopcxt_has_device(lc))
		തിരികെ 0;
	എങ്കിൽ (!lc->devno)
		lc->devno = sysfs_devname_to_devno(lc->device);
	തിരികെ lc->devno;
}

പൂർണ്ണസംഖ്യ loopcxt_is_lost(ഘടന loopdev_cxt *lc)
{
	എങ്കിൽ (!lc || !loopcxt_has_device(lc))
		തിരികെ 0;
	എങ്കിൽ (lc->is_lost)
		തിരികെ 1;

	lc->is_lost = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	തിരികെ lc->is_lost;
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
പൂർണ്ണസംഖ്യ loopcxt_init(ഘടന loopdev_cxt *lc, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ rc;
	ഘടന stat st;
	ഘടന loopdev_cxt dummy = UL_LOOPDEVCXT_EMPTY;

	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	memcpy(lc, &dummy, വലുപ്പം(dummy));
	lc->flags = flags;

	rc = loopcxt_set_device(lc, ശൂന്യ);
	എങ്കിൽ (rc)
		തിരികെ rc;

	എങ്കിൽ (stat(_PATH_SYS_BLOCK, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	എങ്കിൽ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_version() >= KERNEL_VERSION(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	എങ്കിൽ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(_PATH_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	തിരികെ 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
ശൂന്യം loopcxt_deinit(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ errsv = errno;

	എങ്കിൽ (!lc)
		തിരികെ;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	സ്വതന്ത്രം(lc->filename);
	lc->filename = ശൂന്യ;

	ignore_result( loopcxt_set_device(lc, ശൂന്യ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
അക്ഷരം *loopcxt_strdup_device(ഘടന loopdev_cxt *lc)
{
	എങ്കിൽ (!lc || !*lc->device)
		തിരികെ ശൂന്യ;
	തിരികെ strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
സ്ഥിരം അക്ഷരം *loopcxt_get_device(ഘടന loopdev_cxt *lc)
{
	തിരികെ lc && *lc->device ? lc->device : ശൂന്യ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
സ്ഥിതം ഘടന path_cxt *loopcxt_get_sysfs(ഘടന loopdev_cxt *lc)
{
	എങ്കിൽ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		എങ്കിൽ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			തിരികെ ശൂന്യ;
		}

		lc->sysfs = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
		എങ്കിൽ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	തിരികെ lc->sysfs;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ __loopcxt_get_fd(ഘടന loopdev_cxt *lc, mode_t mode)
{
	പൂർണ്ണസംഖ്യ old = -1;

	എങ്കിൽ (!lc || !*lc->device)
		തിരികെ -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	എങ്കിൽ (lc->fd >= 0 && mode == O_RDWR && lc->mode == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	എങ്കിൽ (lc->fd < 0) {
		lc->mode = mode;
		lc->fd = open(lc->device, lc->mode | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				mode == O_RDONLY ? "ro" :
				mode == O_RDWR ? "rw" : "??"));

		എങ്കിൽ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	എങ്കിൽ (old >= 0)
		close(old);
	തിരികെ lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
പൂർണ്ണസംഖ്യ loopcxt_get_fd(ഘടന loopdev_cxt *lc)
{
	തിരികെ __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_init_iterator(ഘടന loopdev_cxt *lc, പൂർണ്ണസംഖ്യ flags)
{
	ഘടന loopdev_iter *iter;
	ഘടന stat st;

	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, വലുപ്പം(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	എങ്കിൽ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		എങ്കിൽ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(_PATH_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	തിരികെ 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_deinit_iterator(ഘടന loopdev_cxt *lc)
{
	ഘടന loopdev_iter *iter;

	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	സ്വതന്ത്രം(iter->minors);
	എങ്കിൽ (iter->proc)
		fclose(iter->proc);
	എങ്കിൽ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, വലുപ്പം(*iter));
	തിരികെ 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ loopiter_set_device(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *device)
{
	പൂർണ്ണസംഖ്യ rc = loopcxt_set_device(lc, device);
	പൂർണ്ണസംഖ്യ used;

	എങ്കിൽ (rc)
		തിരികെ rc;

	എങ്കിൽ (!(lc->iter.flags & LOOPITER_FL_USED) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		തിരികെ 0;	/* caller does not care about device status */

	used = loopcxt_get_offset(lc, ശൂന്യ) == 0;

	എങ്കിൽ ((lc->iter.flags & LOOPITER_FL_USED) && used)
		തിരികെ 0;

	എങ്കിൽ ((lc->iter.flags & LOOPITER_FL_FREE) && !used)
		തിരികെ 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_result( loopcxt_set_device(lc, ശൂന്യ) );
	തിരികെ 1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cmpnum(സ്ഥിരം ശൂന്യം *p1, സ്ഥിരം ശൂന്യം *p2)
{
	തിരികെ (((* (സ്ഥിരം പൂർണ്ണസംഖ്യ *) p1) > (* (സ്ഥിരം പൂർണ്ണസംഖ്യ *) p2)) -
			((* (സ്ഥിരം പൂർണ്ണസംഖ്യ *) p1) < (* (സ്ഥിരം പൂർണ്ണസംഖ്യ *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ loop_scandir(സ്ഥിരം അക്ഷരം *dirname, പൂർണ്ണസംഖ്യ **ary, പൂർണ്ണസംഖ്യ hasprefix)
{
	DIR *dir;
	ഘടന dirent *d;
	unsigned പൂർണ്ണസംഖ്യ n, എണ്ണം = 0, arylen = 0;

	എങ്കിൽ (!dirname || !ary)
		തിരികെ 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	എങ്കിൽ (!dir)
		തിരികെ 0;
	സ്വതന്ത്രം(*ary);
	*ary = ശൂന്യ;

	ആകുമ്പോൾ((d = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
		എങ്കിൽ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			തുടരുക;
#endif
		എങ്കിൽ (is_dotdir_dirent(d))
			തുടരുക;

		എങ്കിൽ (hasprefix) {
			/* /dev/loop<N> */
			എങ്കിൽ (sscanf(d->d_name, "loop%u", &n) != 1)
				തുടരുക;
		} അല്ലെങ്കിൽ {
			/* /dev/loop/<N> */
			അക്ഷരം *end = ശൂന്യ;

			errno = 0;
			n = strtol(d->d_name, &end, 10);
			എങ്കിൽ (d->d_name == end || (end && *end) || errno)
				തുടരുക;
		}
		എങ്കിൽ (n < LOOPDEV_DEFAULT_NNODES)
			തുടരുക;			/* ignore loop<0..7> */

		എങ്കിൽ (എണ്ണം + 1 > arylen) {
			പൂർണ്ണസംഖ്യ *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, വലുപ്പം(പൂർണ്ണസംഖ്യ));
			എങ്കിൽ (!tmp) {
				സ്വതന്ത്രം(*ary);
				*ary = ശൂന്യ;
				closedir(dir);
				തിരികെ -1;
			}
			*ary = tmp;
		}
		എങ്കിൽ (*ary)
			(*ary)[എണ്ണം++] = n;
	}
	എങ്കിൽ (എണ്ണം && *ary)
		qsort(*ary, എണ്ണം, വലുപ്പം(പൂർണ്ണസംഖ്യ), cmpnum);

	closedir(dir);
	തിരികെ എണ്ണം;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ loopcxt_next_from_proc(ഘടന loopdev_cxt *lc)
{
	ഘടന loopdev_iter *iter = &lc->iter;
	അക്ഷരം buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	എങ്കിൽ (!iter->proc)
		iter->proc = fopen(_PATH_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	എങ്കിൽ (!iter->proc)
		തിരികെ 1;

	ആകുമ്പോൾ (fgets(buf, വലുപ്പം(buf), iter->proc)) {
		unsigned പൂർണ്ണസംഖ്യ m;
		അക്ഷരം name[128 + 1];


		എങ്കിൽ (sscanf(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			തുടരുക;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		എങ്കിൽ (loopiter_set_device(lc, name) == 0)
			തിരികെ 0;
	}

	തിരികെ 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ loopcxt_next_from_sysfs(ഘടന loopdev_cxt *lc)
{
	ഘടന loopdev_iter *iter = &lc->iter;
	ഘടന dirent *d;
	പൂർണ്ണസംഖ്യ fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	എങ്കിൽ (!iter->sysblock)
		iter->sysblock = opendir(_PATH_SYS_BLOCK);

	എങ്കിൽ (!iter->sysblock)
		തിരികെ 1;

	fd = dirfd(iter->sysblock);

	ആകുമ്പോൾ ((d = readdir(iter->sysblock))) {
		അക്ഷരം name[NAME_MAX + 18 + 1];
		ഘടന stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		എങ്കിൽ (is_dotdir_dirent(d) || strncmp(d->d_name, "loop", 4) != 0)
			തുടരുക;

		snprintf(name, വലുപ്പം(name), "%s/loop/backing_file", d->d_name);
		എങ്കിൽ (fstatat(fd, name, &st, 0) != 0)
			തുടരുക;

		എങ്കിൽ (loopiter_set_device(lc, d->d_name) == 0)
			തിരികെ 0;
	}

	തിരികെ 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
പൂർണ്ണസംഖ്യ loopcxt_next(ഘടന loopdev_cxt *lc)
{
	ഘടന loopdev_iter *iter;

	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;


	iter = &lc->iter;
	എങ്കിൽ (iter->done)
		തിരികെ 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	എങ്കിൽ (iter->flags & LOOPITER_FL_USED) {
		പൂർണ്ണസംഖ്യ rc;

		എങ്കിൽ (loopcxt_sysfs_available(lc))
			rc = loopcxt_next_from_sysfs(lc);
		അല്ലെങ്കിൽ
			rc = loopcxt_next_from_proc(lc);
		എങ്കിൽ (rc == 0)
			തിരികെ 0;
		പോകുക done;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	എങ്കിൽ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		വേണ്ടി (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			അക്ഷരം name[16];
			snprintf(name, വലുപ്പം(name), "loop%d", iter->ncur);

			എങ്കിൽ (loopiter_set_device(lc, name) == 0)
				തിരികെ 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	എങ്കിൽ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(_PATH_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	വേണ്ടി (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		അക്ഷരം name[16];
		snprintf(name, വലുപ്പം(name), "loop%d", iter->minors[iter->ncur]);

		എങ്കിൽ (loopiter_set_device(lc, name) == 0)
			തിരികെ 0;
	}
done:
	loopcxt_deinit_iterator(lc);
	തിരികെ 1;
}

/*
 * @device: path to device
 */
പൂർണ്ണസംഖ്യ is_loopdev(സ്ഥിരം അക്ഷരം *device)
{
	ഘടന stat st;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	അല്ലെങ്കിൽ എങ്കിൽ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	അല്ലെങ്കിൽ എങ്കിൽ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		അക്ഷരം name[PATH_MAX], *cn, *p = ശൂന്യ;

		snprintf(name, വലുപ്പം(name), _PATH_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_path(name);
		എങ്കിൽ (cn)
			p = stripoff_last_component(cn);
		rc = p && ul_startswith(p, "loop");
		സ്വതന്ത്രം(cn);
	}

	എങ്കിൽ (rc == 0)
		errno = ENODEV;
	തിരികെ rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ഘടന loop_info64 *loopcxt_get_info(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ fd;

	എങ്കിൽ (!lc || lc->info_failed) {
		errno = EINVAL;
		തിരികെ ശൂന്യ;
	}
	errno = 0;
	എങ്കിൽ (lc->has_info)
		തിരികെ &lc->config.info;

	fd = loopcxt_get_fd(lc);
	എങ്കിൽ (fd < 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		തിരികെ &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	തിരികെ ശൂന്യ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
അക്ഷരം *loopcxt_get_backing_file(ഘടന loopdev_cxt *lc)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_path_read_string(sysfs, &res, "loop/backing_file");

	എങ്കിൽ (!res && loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);

		എങ്കിൽ (lo) {
			lo->lo_file_name[LO_NAME_SIZE - 2] = '*';
			lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
			res = strdup((അക്ഷരം *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	തിരികെ res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
അക്ഷരം *loopcxt_get_refname(ഘടന loopdev_cxt *lc)
{
	അക്ഷരം *res = ശൂന്യ;
	ഘടന loop_info64 *lo = loopcxt_get_info(lc);

	എങ്കിൽ (lo) {
		lo->lo_file_name[LO_NAME_SIZE - 1] = '\0';
		res = strdup((അക്ഷരം *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	തിരികെ res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_offset(ഘടന loopdev_cxt *lc, uint64_t *offset)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);
	പൂർണ്ണസംഖ്യ rc = -EINVAL;

	എങ്കിൽ (sysfs)
		എങ്കിൽ (ul_path_read_u64(sysfs, offset, "loop/offset") == 0)
			rc = 0;

	എങ്കിൽ (rc && loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);
		എങ്കിൽ (lo) {
			എങ്കിൽ (offset)
				*offset = lo->lo_offset;
			rc = 0;
		} അല്ലെങ്കിൽ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_blocksize(ഘടന loopdev_cxt *lc, uint64_t *blocksize)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);
	പൂർണ്ണസംഖ്യ rc = -EINVAL;

	എങ്കിൽ (sysfs)
		എങ്കിൽ (ul_path_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	എങ്കിൽ (rc) {
		പൂർണ്ണസംഖ്യ fd = loopcxt_get_fd(lc);
		പൂർണ്ണസംഖ്യ sz = 0;

		എങ്കിൽ (fd < 0)
			തിരികെ -EINVAL;
		rc = blkdev_get_sector_size(fd, &sz);
		എങ്കിൽ (rc)
			തിരികെ rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_sizelimit(ഘടന loopdev_cxt *lc, uint64_t *size)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);
	പൂർണ്ണസംഖ്യ rc = -EINVAL;

	എങ്കിൽ (sysfs)
		എങ്കിൽ (ul_path_read_u64(sysfs, size, "loop/sizelimit") == 0)
			rc = 0;

	എങ്കിൽ (rc && loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);
		എങ്കിൽ (lo) {
			എങ്കിൽ (size)
				*size = lo->lo_sizelimit;
			rc = 0;
		} അല്ലെങ്കിൽ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_encrypt_type(ഘടന loopdev_cxt *lc, uint32_t *type)
{
	ഘടന loop_info64 *lo = loopcxt_get_info(lc);
	പൂർണ്ണസംഖ്യ rc;

	/* not provided by sysfs */
	എങ്കിൽ (lo) {
		എങ്കിൽ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} അല്ലെങ്കിൽ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
സ്ഥിരം അക്ഷരം *loopcxt_get_crypt_name(ഘടന loopdev_cxt *lc)
{
	ഘടന loop_info64 *lo = loopcxt_get_info(lc);

	എങ്കിൽ (lo)
		തിരികെ (അക്ഷരം *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	തിരികെ ശൂന്യ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_backing_devno(ഘടന loopdev_cxt *lc, dev_t *devno)
{
	ഘടന loop_info64 *lo = loopcxt_get_info(lc);
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (lo) {
		എങ്കിൽ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} അല്ലെങ്കിൽ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
പൂർണ്ണസംഖ്യ loopcxt_get_backing_inode(ഘടന loopdev_cxt *lc, ino_t *ino)
{
	ഘടന loop_info64 *lo = loopcxt_get_info(lc);
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (lo) {
		എങ്കിൽ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} അല്ലെങ്കിൽ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	തിരികെ rc;
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
പൂർണ്ണസംഖ്യ loopmod_supports_partscan(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ rc, ret = 0;
	FILE *f;

	എങ്കിൽ (get_linux_version() >= KERNEL_VERSION(3,2,0))
		തിരികെ 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	എങ്കിൽ (!f)
		തിരികെ 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	തിരികെ rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
പൂർണ്ണസംഖ്യ loopcxt_is_partscan(ഘടന loopdev_cxt *lc)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);

	എങ്കിൽ (sysfs) {
		/* kernel >= 3.2 */
		പൂർണ്ണസംഖ്യ fl;
		എങ്കിൽ (ul_path_read_s32(sysfs, &fl, "loop/partscan") == 0)
			തിരികെ fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	തിരികെ loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
പൂർണ്ണസംഖ്യ loopcxt_is_autoclear(ഘടന loopdev_cxt *lc)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);

	എങ്കിൽ (sysfs) {
		പൂർണ്ണസംഖ്യ fl;
		എങ്കിൽ (ul_path_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			തിരികെ fl;
	}

	എങ്കിൽ (loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);
		എങ്കിൽ (lo)
			തിരികെ lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	തിരികെ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
പൂർണ്ണസംഖ്യ loopcxt_is_readonly(ഘടന loopdev_cxt *lc)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);

	എങ്കിൽ (sysfs) {
		പൂർണ്ണസംഖ്യ fl;
		എങ്കിൽ (ul_path_read_s32(sysfs, &fl, "ro") == 0)
			തിരികെ fl;
	}

	എങ്കിൽ (loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);
		എങ്കിൽ (lo)
			തിരികെ lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	തിരികെ 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
പൂർണ്ണസംഖ്യ loopcxt_is_dio(ഘടന loopdev_cxt *lc)
{
	ഘടന path_cxt *sysfs = loopcxt_get_sysfs(lc);

	എങ്കിൽ (sysfs) {
		പൂർണ്ണസംഖ്യ fl;
		എങ്കിൽ (ul_path_read_s32(sysfs, &fl, "loop/dio") == 0)
			തിരികെ fl;
	}
	എങ്കിൽ (loopcxt_ioctl_enabled(lc)) {
		ഘടന loop_info64 *lo = loopcxt_get_info(lc);
		എങ്കിൽ (lo)
			തിരികെ lo->lo_flags & LO_FLAGS_DIRECT_IO;
	}
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ loopcxt_is_used(ഘടന loopdev_cxt *lc,
		    ഘടന stat *st,
		    സ്ഥിരം അക്ഷരം *backing_file,
		    uint64_t offset,
		    uint64_t sizelimit,
		    പൂർണ്ണസംഖ്യ flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	എങ്കിൽ (!lc)
		തിരികെ 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	എങ്കിൽ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		എങ്കിൽ (ino == st->st_ino && dev == st->st_dev)
			പോകുക found;

		/* don't use filename if we have devno and inode */
		തിരികെ 0;
	}

	/* poor man's solution */
	എങ്കിൽ (backing_file) {
		അക്ഷരം *name = loopcxt_get_backing_file(lc);
		പൂർണ്ണസംഖ്യ rc = name && strcmp(name, backing_file) == 0;

		സ്വതന്ത്രം(name);
		എങ്കിൽ (rc)
			പോകുക found;
	}

	തിരികെ 0;
found:
	എങ്കിൽ (flags & LOOPDEV_FL_OFFSET) {
		uint64_t off = 0;

		പൂർണ്ണസംഖ്യ rc = loopcxt_get_offset(lc, &off) == 0 && off == offset;

		എങ്കിൽ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			തിരികെ loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		തിരികെ rc;
	}
	തിരികെ 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
പൂർണ്ണസംഖ്യ loopcxt_set_offset(ഘടന loopdev_cxt *lc, uint64_t offset)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;
	lc->config.info.lo_offset = offset;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, offset));
	തിരികെ 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
പൂർണ്ണസംഖ്യ loopcxt_set_sizelimit(ഘടന loopdev_cxt *lc, uint64_t sizelimit)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	തിരികെ 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
പൂർണ്ണസംഖ്യ loopcxt_set_blocksize(ഘടന loopdev_cxt *lc, uint64_t blocksize)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	തിരികെ 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
പൂർണ്ണസംഖ്യ loopcxt_set_flags(ഘടന loopdev_cxt *lc, uint32_t flags)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (unsigned) flags));
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ loopcxt_set_refname(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *refname)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;

	memset(lc->config.info.lo_file_name, 0, വലുപ്പം(lc->config.info.lo_file_name));
	എങ്കിൽ (refname)
		xstrncpy((അക്ഷരം *)lc->config.info.lo_file_name, refname, LO_NAME_SIZE);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (അക്ഷരം *)lc->config.info.lo_file_name));
	തിരികെ 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
പൂർണ്ണസംഖ്യ loopcxt_set_backing_file(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *filename)
{
	എങ്കിൽ (!lc)
		തിരികെ -EINVAL;

	എങ്കിൽ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	അല്ലെങ്കിൽ
		lc->filename = ul_canonicalize_path(filename);
	എങ്കിൽ (!lc->filename)
		തിരികെ -errno;

	എങ്കിൽ (!lc->config.info.lo_file_name[0])
		loopcxt_set_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ loopcxt_check_size(ഘടന loopdev_cxt *lc, പൂർണ്ണസംഖ്യ file_fd)
{
	uint64_t size, expected_size;
	പൂർണ്ണസംഖ്യ dev_fd;
	ഘടന stat st;

	എങ്കിൽ (!lc->config.info.lo_offset && !lc->config.info.lo_sizelimit)
		തിരികെ 0;

	എങ്കിൽ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		തിരികെ -errno;
	}
	എങ്കിൽ (S_ISBLK(st.st_mode)) {
		എങ്കിൽ (blkdev_get_size(file_fd, &expected_size)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			തിരികെ -errno;
		}
	} അല്ലെങ്കിൽ
		expected_size = st.st_size;

	എങ്കിൽ (expected_size == 0 || expected_size <= lc->config.info.lo_offset) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		തിരികെ 0;	/* ignore this error */
	}

	എങ്കിൽ (lc->config.info.lo_offset > 0)
		expected_size -= lc->config.info.lo_offset;

	എങ്കിൽ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_size)
		expected_size = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	എങ്കിൽ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		തിരികെ -errno;
	}

	എങ്കിൽ (blkdev_get_size(dev_fd, &size)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		തിരികെ -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	എങ്കിൽ (expected_size % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_size = (expected_size >> 9) << 9;
	}

	എങ്കിൽ (expected_size != size) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      size, expected_size));

		എങ്കിൽ (loopcxt_ioctl_capacity(lc)) {
			/* ioctl not available */
			എങ്കിൽ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			തിരികെ -errno;
		}

		എങ്കിൽ (blkdev_get_size(dev_fd, &size))
			തിരികെ -errno;

		എങ്കിൽ (expected_size != size) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					size, expected_size));
			തിരികെ -errno;
		}
	}

	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ loopcxt_setup_device(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, mode = O_RDWR;
	പൂർണ്ണസംഖ്യ rc = -1, cnt = 0;
	പൂർണ്ണസംഖ്യ errsv = 0;
	പൂർണ്ണസംഖ്യ fallback = 0;

	എങ്കിൽ (!lc || !*lc->device || !lc->filename)
		തിരികെ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	എങ്കിൽ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		mode = O_RDONLY;

	എങ്കിൽ (lc->config.info.lo_flags & LO_FLAGS_DIRECT_IO)
		flags |= O_DIRECT;
	എങ്കിൽ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
	അല്ലെങ്കിൽ
		file_fd = open(lc->filename, mode | flags);

	എങ്കിൽ (file_fd < 0 && mode != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		mode = O_RDONLY;
		എങ്കിൽ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, mode | flags, 0);
		അല്ലെങ്കിൽ
			file_fd = open(lc->filename, mode | flags);
	}
	എങ്കിൽ (file_fd < 0) {
		DBG_OBJ(SETUP, lc, ul_debug("open backing file failed: %m"));
		തിരികെ -errno;
	}
	DBG_OBJ(SETUP, lc, ul_debug("backing file open: OK"));

	എങ്കിൽ (mode == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	അല്ലെങ്കിൽ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	ചെയ്യുക {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, mode);
		എങ്കിൽ (dev_fd >= 0 || lc->control_ok == 0)
			നിർത്തുക;
		എങ്കിൽ (errno != EACCES && errno != ENOENT)
			നിർത്തുക;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} ആകുമ്പോൾ (cnt++ < 16);

	എങ്കിൽ (dev_fd < 0) {
		rc = -errno;
		പോകുക err;
	}

	DBG_OBJ(SETUP, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	എങ്കിൽ (lc->blocksize > 0)
		lc->config.block_size = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	എങ്കിൽ (rc != 0) {
		errsv = errno;
		എങ്കിൽ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			പോകുക err;
		}
		fallback = 1;
	} അല്ലെങ്കിൽ {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	എങ്കിൽ (fallback) {
		എങ്കിൽ (ioctl(dev_fd, LOOP_SET_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD failed: %m"));
			പോകുക err;
		}

		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_FD: OK"));

		എങ്കിൽ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			പോകുക err;
		}

		എങ്കിൽ ((rc = loopcxt_ioctl_status(lc)) < 0) {
			errsv = -rc;
			പോകുക err;
		}
	}

	എങ്കിൽ ((rc = loopcxt_check_size(lc, file_fd)))
		പോകുക err;

	close(file_fd);

	memset(&lc->config, 0, വലുപ്പം(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(SETUP, lc, ul_debug("success [rc=0]"));
	തിരികെ 0;
err:
	എങ്കിൽ (file_fd >= 0)
		close(file_fd);
	എങ്കിൽ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	എങ്കിൽ (errsv)
		errno = errsv;

	DBG_OBJ(SETUP, lc, ul_debug("failed [rc=%d]", rc));
	തിരികെ rc;
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
പൂർണ്ണസംഖ്യ loopcxt_ioctl_status(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	എങ്കിൽ (dev_fd < 0)
		തിരികെ -errno;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_SET_STATUS64, &lc->config.info) );
	എങ്കിൽ (rc != 0) {
		DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		തിരികെ rc;
	}

	DBG_OBJ(SETUP, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ loopcxt_ioctl_capacity(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ rc, fd = loopcxt_get_fd(lc);

	എങ്കിൽ (fd < 0)
		തിരികെ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_CAPACITY, 0) );
	എങ്കിൽ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		തിരികെ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ loopcxt_ioctl_dio(ഘടന loopdev_cxt *lc, unsigned long use_dio)
{
	പൂർണ്ണസംഖ്യ rc, fd = loopcxt_get_fd(lc);

	എങ്കിൽ (fd < 0)
		തിരികെ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_SET_DIRECT_IO, use_dio) );
	എങ്കിൽ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		തിരികെ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	തിരികെ 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
പൂർണ്ണസംഖ്യ loopcxt_ioctl_blocksize(ഘടന loopdev_cxt *lc, uint64_t blocksize)
{
	പൂർണ്ണസംഖ്യ rc, fd = loopcxt_get_fd(lc);

	എങ്കിൽ (fd < 0)
		തിരികെ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_SET_BLOCK_SIZE, (unsigned long) blocksize) );
	എങ്കിൽ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		തിരികെ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ loopcxt_get_device_nr(ഘടന loopdev_cxt *lc, പൂർണ്ണസംഖ്യ *nr)
{
	സ്ഥിരം അക്ഷരം *p, *dev;
	പൂർണ്ണസംഖ്യ rc = -EINVAL;

	errno = 0;
	എങ്കിൽ (!lc || !nr)
		തിരികെ rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	എങ്കിൽ (!dev)
		പോകുക done;

	p = strrchr(dev, '/');
	എങ്കിൽ (!p)
		പോകുക done;

	എങ്കിൽ (sscanf(p, "/loop%d", nr) != 1 && sscanf(p, "/%d", nr) != 1)
		പോകുക done;

	എങ്കിൽ (*nr < 0)
		പോകുക done;
	rc = 0;
done:
	എങ്കിൽ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ loopcxt_detach_device(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ rc, fd = loopcxt_get_fd(lc);

	എങ്കിൽ (fd < 0)
		തിരികെ -EINVAL;

	DBG_OBJ(SETUP, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	എങ്കിൽ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		തിരികെ rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ loopcxt_remove_device(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ rc = -EINVAL;
	പൂർണ്ണസംഖ്യ ctl, nr = -1;

	എങ്കിൽ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		പോകുക done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	എങ്കിൽ (rc)
		പോകുക done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	എങ്കിൽ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ loopcxt_add_device(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ rc = -EINVAL;
	പൂർണ്ണസംഖ്യ ctl, nr = -1;

	എങ്കിൽ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		പോകുക done;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	എങ്കിൽ (rc)
		പോകുക done;

	ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	എങ്കിൽ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_ADD, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
done:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
പൂർണ്ണസംഖ്യ loopcxt_find_unused(ഘടന loopdev_cxt *lc)
{
	പൂർണ്ണസംഖ്യ rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	എങ്കിൽ (lc->flags & LOOPDEV_FL_CONTROL) {
		പൂർണ്ണസംഖ്യ ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(_PATH_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		എങ്കിൽ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		അല്ലെങ്കിൽ
			rc = -errno;
		എങ്കിൽ (rc >= 0) {
			അക്ഷരം name[16];
			snprintf(name, വലുപ്പം(name), "loop%d", rc);

			rc = loopiter_set_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		എങ്കിൽ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	എങ്കിൽ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		എങ്കിൽ (rc)
			തിരികെ rc;

		rc = loopcxt_next(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		എങ്കിൽ (rc)
			തിരികെ -ENOENT;
	}
	തിരികെ rc;
}



/*
 * Return: TRUE/FALSE
 */
പൂർണ്ണസംഖ്യ loopdev_is_autoclear(സ്ഥിരം അക്ഷരം *device)
{
	ഘടന loopdev_cxt lc;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!device)
		തിരികെ 0;

	rc = loopcxt_init(&lc, 0);
	എങ്കിൽ (!rc)
		rc = loopcxt_set_device(&lc, device);
	എങ്കിൽ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	തിരികെ rc;
}

അക്ഷരം *loopdev_get_backing_file(സ്ഥിരം അക്ഷരം *device)
{
	ഘടന loopdev_cxt lc;
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (!device)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (loopcxt_init(&lc, 0))
		തിരികെ ശൂന്യ;
	എങ്കിൽ (loopcxt_set_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	തിരികെ res;
}

/*
 * Returns: TRUE/FALSE
 */
പൂർണ്ണസംഖ്യ loopdev_has_backing_file(സ്ഥിരം അക്ഷരം *device)
{
	അക്ഷരം *tmp = loopdev_get_backing_file(device);

	എങ്കിൽ (tmp) {
		സ്വതന്ത്രം(tmp);
		തിരികെ 1;
	}
	തിരികെ 0;
}

/*
 * Returns: TRUE/FALSE
 */
പൂർണ്ണസംഖ്യ loopdev_is_used(സ്ഥിരം അക്ഷരം *device, സ്ഥിരം അക്ഷരം *filename,
		    uint64_t offset, uint64_t sizelimit, പൂർണ്ണസംഖ്യ flags)
{
	ഘടന loopdev_cxt lc;
	ഘടന stat st;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (!device || !filename)
		തിരികെ 0;

	rc = loopcxt_init(&lc, 0);
	എങ്കിൽ (!rc)
		rc = loopcxt_set_device(&lc, device);
	എങ്കിൽ (rc)
		തിരികെ rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_used(&lc, rc ? &st : ശൂന്യ, filename, offset, sizelimit, flags);

	loopcxt_deinit(&lc);
	തിരികെ rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
പൂർണ്ണസംഖ്യ loopdev_detach(സ്ഥിരം അക്ഷരം *device)
{
	ഘടന loopdev_cxt lc;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!device)
		തിരികെ -EINVAL;

	rc = loopcxt_init(&lc, 0);
	എങ്കിൽ (!rc)
		rc = loopcxt_set_device(&lc, device);
	എങ്കിൽ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	തിരികെ rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
പൂർണ്ണസംഖ്യ loopcxt_find_by_backing_file(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *filename,
				 uint64_t offset, uint64_t sizelimit, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ rc, hasst;
	ഘടന stat st;

	എങ്കിൽ (!filename)
		തിരികെ -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	എങ്കിൽ (rc)
		തിരികെ rc;

	ആകുമ്പോൾ ((rc = loopcxt_next(lc)) == 0) {

		എങ്കിൽ (loopcxt_is_used(lc, hasst ? &st : ശൂന്യ,
				    filename, offset, sizelimit, flags))
			നിർത്തുക;
	}

	loopcxt_deinit_iterator(lc);
	തിരികെ rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
പൂർണ്ണസംഖ്യ loopcxt_find_overlap(ഘടന loopdev_cxt *lc, സ്ഥിരം അക്ഷരം *filename,
			   uint64_t offset, uint64_t sizelimit)
{
	പൂർണ്ണസംഖ്യ rc, hasst;
	ഘടന stat st;

	എങ്കിൽ (!filename)
		തിരികെ -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_USED);
	എങ്കിൽ (rc)
		തിരികെ rc;

	ആകുമ്പോൾ ((rc = loopcxt_next(lc)) == 0) {
		uint64_t lc_sizelimit, lc_offset;

		rc = loopcxt_is_used(lc, hasst ? &st : ശൂന്യ,
				     filename, offset, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		എങ്കിൽ (rc <= 0)
			തുടരുക;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_offset(lc, &lc_offset);
		എങ്കിൽ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			തുടരുക;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		എങ്കിൽ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			തുടരുക;
		}

		/* full match */
		എങ്കിൽ (lc_sizelimit == sizelimit && lc_offset == offset) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			പോകുക found;
		}

		/* overlap */
		എങ്കിൽ (lc_sizelimit != 0 && offset >= lc_offset + lc_sizelimit)
			തുടരുക;
		എങ്കിൽ (sizelimit != 0 && offset + sizelimit <= lc_offset)
			തുടരുക;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			പോകുക found;
	}

	എങ്കിൽ (rc == 1)
		rc = 0;	/* not found */
found:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	തിരികെ rc;
}

/*
 * Returns allocated string with device name
 */
അക്ഷരം *loopdev_find_by_backing_file(സ്ഥിരം അക്ഷരം *filename, uint64_t offset, uint64_t sizelimit, പൂർണ്ണസംഖ്യ flags)
{
	ഘടന loopdev_cxt lc;
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (!filename)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (loopcxt_init(&lc, 0))
		തിരികെ ശൂന്യ;
	എങ്കിൽ (loopcxt_find_by_backing_file(&lc, filename, offset, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	തിരികെ res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
പൂർണ്ണസംഖ്യ loopdev_എണ്ണം_by_backing_file(സ്ഥിരം അക്ഷരം *filename, അക്ഷരം **loopdev)
{
	ഘടന loopdev_cxt lc;
	പൂർണ്ണസംഖ്യ എണ്ണം = 0, rc;

	എങ്കിൽ (!filename)
		തിരികെ -1;

	rc = loopcxt_init(&lc, 0);
	എങ്കിൽ (rc)
		തിരികെ rc;
	എങ്കിൽ (loopcxt_init_iterator(&lc, LOOPITER_FL_USED))
		തിരികെ -1;

	ആകുമ്പോൾ(loopcxt_next(&lc) == 0) {
		അക്ഷരം *backing = loopcxt_get_backing_file(&lc);

		എങ്കിൽ (!backing || strcmp(backing, filename) != 0) {
			സ്വതന്ത്രം(backing);
			തുടരുക;
		}

		സ്വതന്ത്രം(backing);
		എങ്കിൽ (loopdev && എണ്ണം == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		എണ്ണം++;
	}

	loopcxt_deinit(&lc);

	എങ്കിൽ (loopdev && എണ്ണം > 1) {
		സ്വതന്ത്രം(*loopdev);
		*loopdev = ശൂന്യ;
	}
	തിരികെ എണ്ണം;
}

#ifdef TEST_PROGRAM_LOOPDEV
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	എങ്കിൽ (argc < 2)
		പോകുക usage;

	എങ്കിൽ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		printf("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	അല്ലെങ്കിൽ
		പോകുക usage;

	തിരികെ EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			program_invocation_short_name);
	തിരികെ EXIT_FAILURE;
}
#endif

