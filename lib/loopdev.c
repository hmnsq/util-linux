
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

#تعریف_کرو LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ساکن UL_DEBUG_DEFINE_نقاب(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو LOOPDEV_DEBUG_INIT	(1 << 1)
#تعریف_کرو LOOPDEV_DEBUG_CXT	(1 << 2)
#تعریف_کرو LOOPDEV_DEBUG_ITER	(1 << 3)
#تعریف_کرو LOOPDEV_DEBUG_تیاری	(1 << 4)

#تعریف_کرو DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ساکن خالی loopdev_init_debug(خالی)
{
	اگر (loopdev_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#تعریف_کرو loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#تعریف_کرو loopcxt_sysfs_دستیاب(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#تعریف_کرو repeat_on_eagain(x) __extension__ ({			\
		صحیح _c = 0, _e;					\
		کرو {						\
			errno = 0;				\
			_e = x;					\
			اگر (_e == 0 || errno != EAGAIN)		\
				توڑو;				\
			اگر (_c >= LOOPDEV_MAX_TRIES)		\
				توڑو;				\
			xusleep(250000);			\
			_c++;					\
		} جب (1);					\
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
صحیح loopcxt_سیٹ_device(ساخت loopdev_cxt *lc, ثابت حرف *device)
{
	اگر (!lc)
		واپس -EINVAL;

	اگر (lc->fd >= 0) {
		close(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_گمشدہ = 0;
	lc->devno = 0;
	lc->منوال = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	memset(&lc->config, 0, حجم(lc->config));

	/* set new */
	اگر (device) {
		اگر (*device != '/') {
			ثابت حرف *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			اگر (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				اگر (لڑی_لمبائی(device) < 5)
					واپس -1;
				device += 4;
				dir = راستہ_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, حجم(lc->device), "%s%s",
				dir, device);
		} ورنہ
			xstrncpy(lc->device, device, حجم(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_راستہ(lc->sysfs);
	lc->sysfs = تہی;
	واپس 0;
}

صحیح loopcxt_has_device(ساخت loopdev_cxt *lc)
{
	واپس lc && *lc->device;
}

dev_t loopcxt_get_devno(ساخت loopdev_cxt *lc)
{
	اگر (!lc || !loopcxt_has_device(lc))
		واپس 0;
	اگر (!lc->devno)
		lc->devno = sysfs_devname_مقصد_devno(lc->device);
	واپس lc->devno;
}

صحیح loopcxt_is_گمشدہ(ساخت loopdev_cxt *lc)
{
	اگر (!lc || !loopcxt_has_device(lc))
		واپس 0;
	اگر (lc->is_گمشدہ)
		واپس 1;

	lc->is_گمشدہ = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	واپس lc->is_گمشدہ;
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
صحیح loopcxt_init(ساخت loopdev_cxt *lc, صحیح flags)
{
	صحیح rc;
	ساخت stat st;
	ساخت loopdev_cxt dummy = UL_LOOPDEVCXT_خالی_خانے;

	اگر (!lc)
		واپس -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	حافظہ_نقل(lc, &dummy, حجم(dummy));
	lc->flags = flags;

	rc = loopcxt_سیٹ_device(lc, تہی);
	اگر (rc)
		واپس rc;

	اگر (stat(راستہ_SYS_بلاک, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	اگر (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_نسخہ() >= KERNEL_نسخہ(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(راستہ_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	واپس 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
خالی loopcxt_deinit(ساخت loopdev_cxt *lc)
{
	صحیح errsv = errno;

	اگر (!lc)
		واپس;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	آزاد(lc->filename);
	lc->filename = تہی;

	ignore_نتیجہ( loopcxt_سیٹ_device(lc, تہی) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
حرف *loopcxt_strdup_device(ساخت loopdev_cxt *lc)
{
	اگر (!lc || !*lc->device)
		واپس تہی;
	واپس strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ثابت حرف *loopcxt_get_device(ساخت loopdev_cxt *lc)
{
	واپس lc && *lc->device ? lc->device : تہی;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ساکن ساخت راستہ_cxt *loopcxt_get_sysfs(ساخت loopdev_cxt *lc)
{
	اگر (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		واپس تہی;

	اگر (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		اگر (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			واپس تہی;
		}

		lc->sysfs = ul_new_sysfs_راستہ(devno, تہی, تہی);
		اگر (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	واپس lc->sysfs;
}

ساکن صحیح __loopcxt_get_fd(ساخت loopdev_cxt *lc, mode_t منوال)
{
	صحیح old = -1;

	اگر (!lc || !*lc->device)
		واپس -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	اگر (lc->fd >= 0 && منوال == O_RDWR && lc->منوال == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	اگر (lc->fd < 0) {
		lc->منوال = منوال;
		lc->fd = open(lc->device, lc->منوال | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				منوال == O_RDONLY ? "ro" :
				منوال == O_RDWR ? "rw" : "??"));

		اگر (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	اگر (old >= 0)
		close(old);
	واپس lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
صحیح loopcxt_get_fd(ساخت loopdev_cxt *lc)
{
	واپس __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_init_iterator(ساخت loopdev_cxt *lc, صحیح flags)
{
	ساخت loopdev_iter *iter;
	ساخت stat st;

	اگر (!lc)
		واپس -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, حجم(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	اگر (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		اگر (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(راستہ_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	واپس 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_deinit_iterator(ساخت loopdev_cxt *lc)
{
	ساخت loopdev_iter *iter;

	اگر (!lc)
		واپس -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	آزاد(iter->minors);
	اگر (iter->proc)
		fclose(iter->proc);
	اگر (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, حجم(*iter));
	واپس 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ساکن صحیح loopiter_سیٹ_device(ساخت loopdev_cxt *lc, ثابت حرف *device)
{
	صحیح rc = loopcxt_سیٹ_device(lc, device);
	صحیح مستعمل;

	اگر (rc)
		واپس rc;

	اگر (!(lc->iter.flags & LOOPITER_FL_مستعمل) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		واپس 0;	/* caller does not care about device status */

	مستعمل = loopcxt_get_انحراف(lc, تہی) == 0;

	اگر ((lc->iter.flags & LOOPITER_FL_مستعمل) && مستعمل)
		واپس 0;

	اگر ((lc->iter.flags & LOOPITER_FL_FREE) && !مستعمل)
		واپس 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_نتیجہ( loopcxt_سیٹ_device(lc, تہی) );
	واپس 1;
}

ساکن صحیح cmpnum(ثابت خالی *p1, ثابت خالی *p2)
{
	واپس (((* (ثابت صحیح *) p1) > (* (ثابت صحیح *) p2)) -
			((* (ثابت صحیح *) p1) < (* (ثابت صحیح *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ساکن صحیح loop_scandir(ثابت حرف *dirname, صحیح **ary, صحیح hasprefix)
{
	DIR *dir;
	ساخت dirent *d;
	غیرعلامتی صحیح n, گنتی = 0, arylen = 0;

	اگر (!dirname || !ary)
		واپس 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	اگر (!dir)
		واپس 0;
	آزاد(*ary);
	*ary = تہی;

	جب((d = readdir(dir))) {
#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
		اگر (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			جاری;
#اگر_ختم
		اگر (is_dotdir_dirent(d))
			جاری;

		اگر (hasprefix) {
			/* /dev/loop<N> */
			اگر (لڑی_سے_پڑھو(d->d_name, "loop%u", &n) != 1)
				جاری;
		} ورنہ {
			/* /dev/loop/<N> */
			حرف *انتہا = تہی;

			errno = 0;
			n = strtol(d->d_name, &انتہا, 10);
			اگر (d->d_name == انتہا || (انتہا && *انتہا) || errno)
				جاری;
		}
		اگر (n < LOOPDEV_DEFAULT_NNODES)
			جاری;			/* ignore loop<0..7> */

		اگر (گنتی + 1 > arylen) {
			صحیح *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, حجم(صحیح));
			اگر (!tmp) {
				آزاد(*ary);
				*ary = تہی;
				closedir(dir);
				واپس -1;
			}
			*ary = tmp;
		}
		اگر (*ary)
			(*ary)[گنتی++] = n;
	}
	اگر (گنتی && *ary)
		qsort(*ary, گنتی, حجم(صحیح), cmpnum);

	closedir(dir);
	واپس گنتی;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ساکن صحیح loopcxt_اگلا_from_proc(ساخت loopdev_cxt *lc)
{
	ساخت loopdev_iter *iter = &lc->iter;
	حرف buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	اگر (!iter->proc)
		iter->proc = fopen(راستہ_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	اگر (!iter->proc)
		واپس 1;

	جب (fgets(buf, حجم(buf), iter->proc)) {
		غیرعلامتی صحیح m;
		حرف name[128 + 1];


		اگر (لڑی_سے_پڑھو(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			جاری;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		اگر (loopiter_سیٹ_device(lc, name) == 0)
			واپس 0;
	}

	واپس 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ساکن صحیح loopcxt_اگلا_from_sysfs(ساخت loopdev_cxt *lc)
{
	ساخت loopdev_iter *iter = &lc->iter;
	ساخت dirent *d;
	صحیح fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	اگر (!iter->sysblock)
		iter->sysblock = opendir(راستہ_SYS_بلاک);

	اگر (!iter->sysblock)
		واپس 1;

	fd = dirfd(iter->sysblock);

	جب ((d = readdir(iter->sysblock))) {
		حرف name[NAME_MAX + 18 + 1];
		ساخت stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		اگر (is_dotdir_dirent(d) || لڑی_موازنہ(d->d_name, "loop", 4) != 0)
			جاری;

		snprintf(name, حجم(name), "%s/loop/backing_file", d->d_name);
		اگر (fstatat(fd, name, &st, 0) != 0)
			جاری;

		اگر (loopiter_سیٹ_device(lc, d->d_name) == 0)
			واپس 0;
	}

	واپس 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
صحیح loopcxt_اگلا(ساخت loopdev_cxt *lc)
{
	ساخت loopdev_iter *iter;

	اگر (!lc)
		واپس -EINVAL;


	iter = &lc->iter;
	اگر (iter->مکمل)
		واپس 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	اگر (iter->flags & LOOPITER_FL_مستعمل) {
		صحیح rc;

		اگر (loopcxt_sysfs_دستیاب(lc))
			rc = loopcxt_اگلا_from_sysfs(lc);
		ورنہ
			rc = loopcxt_اگلا_from_proc(lc);
		اگر (rc == 0)
			واپس 0;
		جاؤ مکمل;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	اگر (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		برائے (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", iter->ncur);

			اگر (loopiter_سیٹ_device(lc, name) == 0)
				واپس 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	اگر (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(راستہ_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	برائے (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		حرف name[16];
		snprintf(name, حجم(name), "loop%d", iter->minors[iter->ncur]);

		اگر (loopiter_سیٹ_device(lc, name) == 0)
			واپس 0;
	}
مکمل:
	loopcxt_deinit_iterator(lc);
	واپس 1;
}

/*
 * @device: path to device
 */
صحیح is_loopdev(ثابت حرف *device)
{
	ساخت stat st;
	صحیح rc = 0;

	اگر (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	ورنہ اگر (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	ورنہ اگر (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		حرف name[PATH_MAX], *cn, *p = تہی;

		snprintf(name, حجم(name), راستہ_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_راستہ(name);
		اگر (cn)
			p = stripoff_آخری_component(cn);
		rc = p && ul_startswith(p, "loop");
		آزاد(cn);
	}

	اگر (rc == 0)
		errno = ENODEV;
	واپس rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
ساخت loop_info64 *loopcxt_get_info(ساخت loopdev_cxt *lc)
{
	صحیح fd;

	اگر (!lc || lc->info_failed) {
		errno = EINVAL;
		واپس تہی;
	}
	errno = 0;
	اگر (lc->has_info)
		واپس &lc->config.info;

	fd = loopcxt_get_fd(lc);
	اگر (fd < 0)
		واپس تہی;

	اگر (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		واپس &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	واپس تہی;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
حرف *loopcxt_get_backing_file(ساخت loopdev_cxt *lc)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	حرف *res = تہی;

	اگر (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_راستہ_read_string(sysfs, &res, "loop/backing_file");

	اگر (!res && loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);

		اگر (lo) {
			lo->lo_file_name[LO_NAME_سائز - 2] = '*';
			lo->lo_file_name[LO_NAME_سائز - 1] = '\0';
			res = strdup((حرف *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	واپس res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
حرف *loopcxt_get_refname(ساخت loopdev_cxt *lc)
{
	حرف *res = تہی;
	ساخت loop_info64 *lo = loopcxt_get_info(lc);

	اگر (lo) {
		lo->lo_file_name[LO_NAME_سائز - 1] = '\0';
		res = strdup((حرف *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	واپس res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_انحراف(ساخت loopdev_cxt *lc, uint64_t *انحراف)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_راستہ_read_u64(sysfs, انحراف, "loop/offset") == 0)
			rc = 0;

	اگر (rc && loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo) {
			اگر (انحراف)
				*انحراف = lo->lo_انحراف;
			rc = 0;
		} ورنہ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	واپس rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_blocksize(ساخت loopdev_cxt *lc, uint64_t *blocksize)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_راستہ_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	اگر (rc) {
		صحیح fd = loopcxt_get_fd(lc);
		صحیح sz = 0;

		اگر (fd < 0)
			واپس -EINVAL;
		rc = blkdev_get_sector_سائز(fd, &sz);
		اگر (rc)
			واپس rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	واپس rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_sizelimit(ساخت loopdev_cxt *lc, uint64_t *سائز)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحیح rc = -EINVAL;

	اگر (sysfs)
		اگر (ul_راستہ_read_u64(sysfs, سائز, "loop/sizelimit") == 0)
			rc = 0;

	اگر (rc && loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo) {
			اگر (سائز)
				*سائز = lo->lo_sizelimit;
			rc = 0;
		} ورنہ
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	واپس rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_encrypt_type(ساخت loopdev_cxt *lc, uint32_t *type)
{
	ساخت loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	/* not provided by sysfs */
	اگر (lo) {
		اگر (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} ورنہ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	واپس rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ثابت حرف *loopcxt_get_crypt_name(ساخت loopdev_cxt *lc)
{
	ساخت loop_info64 *lo = loopcxt_get_info(lc);

	اگر (lo)
		واپس (حرف *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	واپس تہی;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_backing_devno(ساخت loopdev_cxt *lc, dev_t *devno)
{
	ساخت loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	اگر (lo) {
		اگر (devno)
			*devno = lo->lo_device;
		rc = 0;
	} ورنہ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	واپس rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
صحیح loopcxt_get_backing_inode(ساخت loopdev_cxt *lc, ino_t *ino)
{
	ساخت loop_info64 *lo = loopcxt_get_info(lc);
	صحیح rc;

	اگر (lo) {
		اگر (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} ورنہ
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	واپس rc;
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
صحیح loopmod_supports_partscan(خالی)
{
	صحیح rc, ret = 0;
	FILE *f;

	اگر (get_linux_نسخہ() >= KERNEL_نسخہ(3,2,0))
		واپس 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	اگر (!f)
		واپس 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	واپس rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
صحیح loopcxt_is_partscan(ساخت loopdev_cxt *lc)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		/* kernel >= 3.2 */
		صحیح fl;
		اگر (ul_راستہ_read_s32(sysfs, &fl, "loop/partscan") == 0)
			واپس fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	واپس loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
صحیح loopcxt_is_autoclear(ساخت loopdev_cxt *lc)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_راستہ_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			واپس fl;
	}

	اگر (loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			واپس lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	واپس 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
صحیح loopcxt_is_readonly(ساخت loopdev_cxt *lc)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_راستہ_read_s32(sysfs, &fl, "ro") == 0)
			واپس fl;
	}

	اگر (loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			واپس lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	واپس 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
صحیح loopcxt_is_dio(ساخت loopdev_cxt *lc)
{
	ساخت راستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	اگر (sysfs) {
		صحیح fl;
		اگر (ul_راستہ_read_s32(sysfs, &fl, "loop/dio") == 0)
			واپس fl;
	}
	اگر (loopcxt_ioctl_enabled(lc)) {
		ساخت loop_info64 *lo = loopcxt_get_info(lc);
		اگر (lo)
			واپس lo->lo_flags & LO_FLAGS_براہ_راست_IO;
	}
	واپس 0;
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
صحیح loopcxt_is_مستعمل(ساخت loopdev_cxt *lc,
		    ساخت stat *st,
		    ثابت حرف *backing_file,
		    uint64_t انحراف,
		    uint64_t sizelimit,
		    صحیح flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	اگر (!lc)
		واپس 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	اگر (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		اگر (ino == st->st_ino && dev == st->st_dev)
			جاؤ ملا;

		/* don't use filename if we have devno and inode */
		واپس 0;
	}

	/* poor man's solution */
	اگر (backing_file) {
		حرف *name = loopcxt_get_backing_file(lc);
		صحیح rc = name && strcmp(name, backing_file) == 0;

		آزاد(name);
		اگر (rc)
			جاؤ ملا;
	}

	واپس 0;
ملا:
	اگر (flags & LOOPDEV_FL_انحراف) {
		uint64_t off = 0;

		صحیح rc = loopcxt_get_انحراف(lc, &off) == 0 && off == انحراف;

		اگر (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			واپس loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		واپس rc;
	}
	واپس 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_سیٹ_انحراف(ساخت loopdev_cxt *lc, uint64_t انحراف)
{
	اگر (!lc)
		واپس -EINVAL;
	lc->config.info.lo_انحراف = انحراف;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, انحراف));
	واپس 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_سیٹ_sizelimit(ساخت loopdev_cxt *lc, uint64_t sizelimit)
{
	اگر (!lc)
		واپس -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	واپس 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحیح loopcxt_سیٹ_blocksize(ساخت loopdev_cxt *lc, uint64_t blocksize)
{
	اگر (!lc)
		واپس -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	واپس 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح loopcxt_سیٹ_flags(ساخت loopdev_cxt *lc, uint32_t flags)
{
	اگر (!lc)
		واپس -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (غیرعلامتی) flags));
	واپس 0;
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
صحیح loopcxt_سیٹ_refname(ساخت loopdev_cxt *lc, ثابت حرف *refname)
{
	اگر (!lc)
		واپس -EINVAL;

	memset(lc->config.info.lo_file_name, 0, حجم(lc->config.info.lo_file_name));
	اگر (refname)
		xstrncpy((حرف *)lc->config.info.lo_file_name, refname, LO_NAME_سائز);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (حرف *)lc->config.info.lo_file_name));
	واپس 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحیح loopcxt_سیٹ_backing_file(ساخت loopdev_cxt *lc, ثابت حرف *filename)
{
	اگر (!lc)
		واپس -EINVAL;

	اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	ورنہ
		lc->filename = ul_canonicalize_راستہ(filename);
	اگر (!lc->filename)
		واپس -errno;

	اگر (!lc->config.info.lo_file_name[0])
		loopcxt_سیٹ_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	واپس 0;
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
ساکن صحیح loopcxt_check_سائز(ساخت loopdev_cxt *lc, صحیح file_fd)
{
	uint64_t سائز, expected_سائز;
	صحیح dev_fd;
	ساخت stat st;

	اگر (!lc->config.info.lo_انحراف && !lc->config.info.lo_sizelimit)
		واپس 0;

	اگر (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		واپس -errno;
	}
	اگر (S_ISBLK(st.st_mode)) {
		اگر (blkdev_get_سائز(file_fd, &expected_سائز)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			واپس -errno;
		}
	} ورنہ
		expected_سائز = st.st_size;

	اگر (expected_سائز == 0 || expected_سائز <= lc->config.info.lo_انحراف) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		واپس 0;	/* ignore this error */
	}

	اگر (lc->config.info.lo_انحراف > 0)
		expected_سائز -= lc->config.info.lo_انحراف;

	اگر (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_سائز)
		expected_سائز = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	اگر (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		واپس -errno;
	}

	اگر (blkdev_get_سائز(dev_fd, &سائز)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		واپس -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	اگر (expected_سائز % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_سائز = (expected_سائز >> 9) << 9;
	}

	اگر (expected_سائز != سائز) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      سائز, expected_سائز));

		اگر (loopcxt_ioctl_استعداد(lc)) {
			/* ioctl not available */
			اگر (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			واپس -errno;
		}

		اگر (blkdev_get_سائز(dev_fd, &سائز))
			واپس -errno;

		اگر (expected_سائز != سائز) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					سائز, expected_سائز));
			واپس -errno;
		}
	}

	واپس 0;
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
صحیح loopcxt_تیاری_device(ساخت loopdev_cxt *lc)
{
	صحیح file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, منوال = O_RDWR;
	صحیح rc = -1, cnt = 0;
	صحیح errsv = 0;
	صحیح fallback = 0;

	اگر (!lc || !*lc->device || !lc->filename)
		واپس -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	اگر (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		منوال = O_RDONLY;

	اگر (lc->config.info.lo_flags & LO_FLAGS_براہ_راست_IO)
		flags |= O_براہ_راست;
	اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, منوال | flags, 0);
	ورنہ
		file_fd = open(lc->filename, منوال | flags);

	اگر (file_fd < 0 && منوال != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		منوال = O_RDONLY;
		اگر (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, منوال | flags, 0);
		ورنہ
			file_fd = open(lc->filename, منوال | flags);
	}
	اگر (file_fd < 0) {
		DBG_OBJ(تیاری, lc, ul_debug("open backing file failed: %m"));
		واپس -errno;
	}
	DBG_OBJ(تیاری, lc, ul_debug("backing file open: OK"));

	اگر (منوال == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	ورنہ
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	کرو {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, منوال);
		اگر (dev_fd >= 0 || lc->control_ok == 0)
			توڑو;
		اگر (errno != EACCES && errno != ENOENT)
			توڑو;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} جب (cnt++ < 16);

	اگر (dev_fd < 0) {
		rc = -errno;
		جاؤ err;
	}

	DBG_OBJ(تیاری, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	اگر (lc->blocksize > 0)
		lc->config.بلاک_سائز = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	اگر (rc != 0) {
		errsv = errno;
		اگر (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(تیاری, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			جاؤ err;
		}
		fallback = 1;
	} ورنہ {
		DBG_OBJ(تیاری, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	اگر (fallback) {
		اگر (ioctl(dev_fd, LOOP_سیٹ_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_FD failed: %m"));
			جاؤ err;
		}

		DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_FD: OK"));

		اگر (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			جاؤ err;
		}

		اگر ((rc = loopcxt_ioctl_حالت(lc)) < 0) {
			errsv = -rc;
			جاؤ err;
		}
	}

	اگر ((rc = loopcxt_check_سائز(lc, file_fd)))
		جاؤ err;

	close(file_fd);

	memset(&lc->config, 0, حجم(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(تیاری, lc, ul_debug("success [rc=0]"));
	واپس 0;
err:
	اگر (file_fd >= 0)
		close(file_fd);
	اگر (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	اگر (errsv)
		errno = errsv;

	DBG_OBJ(تیاری, lc, ul_debug("failed [rc=%d]", rc));
	واپس rc;
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
صحیح loopcxt_ioctl_حالت(ساخت loopdev_cxt *lc)
{
	صحیح dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	اگر (dev_fd < 0)
		واپس -errno;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_سیٹ_STATUS64, &lc->config.info) );
	اگر (rc != 0) {
		DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		واپس rc;
	}

	DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	واپس 0;
}

صحیح loopcxt_ioctl_استعداد(ساخت loopdev_cxt *lc)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		واپس -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_سیٹ_استعداد, 0) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		واپس rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	واپس 0;
}

صحیح loopcxt_ioctl_dio(ساخت loopdev_cxt *lc, غیرعلامتی طویل use_dio)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		واپس -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_سیٹ_براہ_راست_IO, use_dio) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		واپس rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	واپس 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
صحیح loopcxt_ioctl_blocksize(ساخت loopdev_cxt *lc, uint64_t blocksize)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		واپس -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_سیٹ_بلاک_سائز, (غیرعلامتی طویل) blocksize) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		واپس rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	واپس 0;
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
ساکن صحیح loopcxt_get_device_nr(ساخت loopdev_cxt *lc, صحیح *nr)
{
	ثابت حرف *p, *dev;
	صحیح rc = -EINVAL;

	errno = 0;
	اگر (!lc || !nr)
		واپس rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	اگر (!dev)
		جاؤ مکمل;

	p = strrchr(dev, '/');
	اگر (!p)
		جاؤ مکمل;

	اگر (لڑی_سے_پڑھو(p, "/loop%d", nr) != 1 && لڑی_سے_پڑھو(p, "/%d", nr) != 1)
		جاؤ مکمل;

	اگر (*nr < 0)
		جاؤ مکمل;
	rc = 0;
مکمل:
	اگر (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	واپس rc;
}

صحیح loopcxt_detach_device(ساخت loopdev_cxt *lc)
{
	صحیح rc, fd = loopcxt_get_fd(lc);

	اگر (fd < 0)
		واپس -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	اگر (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		واپس rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	واپس 0;
}

صحیح loopcxt_remove_device(ساخت loopdev_cxt *lc)
{
	صحیح rc = -EINVAL;
	صحیح ctl, nr = -1;

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		جاؤ مکمل;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	اگر (rc)
		جاؤ مکمل;

	ctl = open(راستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	اگر (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
مکمل:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	واپس rc;
}

صحیح loopcxt_جوڑو_device(ساخت loopdev_cxt *lc)
{
	صحیح rc = -EINVAL;
	صحیح ctl, nr = -1;

	اگر (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		جاؤ مکمل;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	اگر (rc)
		جاؤ مکمل;

	ctl = open(راستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	اگر (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_جوڑو, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
مکمل:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	واپس rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
صحیح loopcxt_ڈھونڈو_غیر_مستعمل(ساخت loopdev_cxt *lc)
{
	صحیح rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	اگر (lc->flags & LOOPDEV_FL_CONTROL) {
		صحیح ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(راستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		اگر (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		ورنہ
			rc = -errno;
		اگر (rc >= 0) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", rc);

			rc = loopiter_سیٹ_device(lc, name);
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
			واپس rc;

		rc = loopcxt_اگلا(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		اگر (rc)
			واپس -ENOENT;
	}
	واپس rc;
}



/*
 * Return: TRUE/FALSE
 */
صحیح loopdev_is_autoclear(ثابت حرف *device)
{
	ساخت loopdev_cxt lc;
	صحیح rc;

	اگر (!device)
		واپس 0;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	اگر (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	واپس rc;
}

حرف *loopdev_get_backing_file(ثابت حرف *device)
{
	ساخت loopdev_cxt lc;
	حرف *res = تہی;

	اگر (!device)
		واپس تہی;
	اگر (loopcxt_init(&lc, 0))
		واپس تہی;
	اگر (loopcxt_سیٹ_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	واپس res;
}

/*
 * Returns: TRUE/FALSE
 */
صحیح loopdev_has_backing_file(ثابت حرف *device)
{
	حرف *tmp = loopdev_get_backing_file(device);

	اگر (tmp) {
		آزاد(tmp);
		واپس 1;
	}
	واپس 0;
}

/*
 * Returns: TRUE/FALSE
 */
صحیح loopdev_is_مستعمل(ثابت حرف *device, ثابت حرف *filename,
		    uint64_t انحراف, uint64_t sizelimit, صحیح flags)
{
	ساخت loopdev_cxt lc;
	ساخت stat st;
	صحیح rc = 0;

	اگر (!device || !filename)
		واپس 0;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	اگر (rc)
		واپس rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_مستعمل(&lc, rc ? &st : تہی, filename, انحراف, sizelimit, flags);

	loopcxt_deinit(&lc);
	واپس rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
صحیح loopdev_detach(ثابت حرف *device)
{
	ساخت loopdev_cxt lc;
	صحیح rc;

	اگر (!device)
		واپس -EINVAL;

	rc = loopcxt_init(&lc, 0);
	اگر (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	اگر (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	واپس rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
صحیح loopcxt_ڈھونڈو_by_backing_file(ساخت loopdev_cxt *lc, ثابت حرف *filename,
				 uint64_t انحراف, uint64_t sizelimit, صحیح flags)
{
	صحیح rc, hasst;
	ساخت stat st;

	اگر (!filename)
		واپس -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_مستعمل);
	اگر (rc)
		واپس rc;

	جب ((rc = loopcxt_اگلا(lc)) == 0) {

		اگر (loopcxt_is_مستعمل(lc, hasst ? &st : تہی,
				    filename, انحراف, sizelimit, flags))
			توڑو;
	}

	loopcxt_deinit_iterator(lc);
	واپس rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
صحیح loopcxt_ڈھونڈو_overlap(ساخت loopdev_cxt *lc, ثابت حرف *filename,
			   uint64_t انحراف, uint64_t sizelimit)
{
	صحیح rc, hasst;
	ساخت stat st;

	اگر (!filename)
		واپس -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_مستعمل);
	اگر (rc)
		واپس rc;

	جب ((rc = loopcxt_اگلا(lc)) == 0) {
		uint64_t lc_sizelimit, lc_انحراف;

		rc = loopcxt_is_مستعمل(lc, hasst ? &st : تہی,
				     filename, انحراف, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		اگر (rc <= 0)
			جاری;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_انحراف(lc, &lc_انحراف);
		اگر (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			جاری;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		اگر (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			جاری;
		}

		/* full match */
		اگر (lc_sizelimit == sizelimit && lc_انحراف == انحراف) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			جاؤ ملا;
		}

		/* overlap */
		اگر (lc_sizelimit != 0 && انحراف >= lc_انحراف + lc_sizelimit)
			جاری;
		اگر (sizelimit != 0 && انحراف + sizelimit <= lc_انحراف)
			جاری;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			جاؤ ملا;
	}

	اگر (rc == 1)
		rc = 0;	/* not found */
ملا:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	واپس rc;
}

/*
 * Returns allocated string with device name
 */
حرف *loopdev_ڈھونڈو_by_backing_file(ثابت حرف *filename, uint64_t انحراف, uint64_t sizelimit, صحیح flags)
{
	ساخت loopdev_cxt lc;
	حرف *res = تہی;

	اگر (!filename)
		واپس تہی;

	اگر (loopcxt_init(&lc, 0))
		واپس تہی;
	اگر (loopcxt_ڈھونڈو_by_backing_file(&lc, filename, انحراف, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	واپس res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
صحیح loopdev_گنتی_by_backing_file(ثابت حرف *filename, حرف **loopdev)
{
	ساخت loopdev_cxt lc;
	صحیح گنتی = 0, rc;

	اگر (!filename)
		واپس -1;

	rc = loopcxt_init(&lc, 0);
	اگر (rc)
		واپس rc;
	اگر (loopcxt_init_iterator(&lc, LOOPITER_FL_مستعمل))
		واپس -1;

	جب(loopcxt_اگلا(&lc) == 0) {
		حرف *backing = loopcxt_get_backing_file(&lc);

		اگر (!backing || strcmp(backing, filename) != 0) {
			آزاد(backing);
			جاری;
		}

		آزاد(backing);
		اگر (loopdev && گنتی == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		گنتی++;
	}

	loopcxt_deinit(&lc);

	اگر (loopdev && گنتی > 1) {
		آزاد(*loopdev);
		*loopdev = تہی;
	}
	واپس گنتی;
}

#اگر_تعریف_شدہ TEST_پروگرام_LOOPDEV
صحیح مین(صحیح argc, حرف *argv[])
{
	اگر (argc < 2)
		جاؤ usage;

	اگر (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		لکھو("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	ورنہ
		جاؤ usage;

	واپس EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			پروگرام_invocation_short_name);
	واپس EXIT_FAILURE;
}
#اگر_ختم

