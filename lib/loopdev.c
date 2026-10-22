
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
#ضمن <stdio.h>
#ضمن <stdint.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ضمن <fcntl.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/stat.h>
#ضمن <sys/mman.h>
#ضمن <inttypes.h>
#ضمن <dirent.h>

#ضمن "linux_version.h"
#ضمن "c.h"
#ضمن "timeutils.h"
#ضمن "sysfs.h"
#ضمن "pathnames.h"
#ضمن "loopdev.h"
#ضمن "canonicalize.h"
#ضمن "blkdev.h"
#ضمن "debug.h"
#ضمن "fileutils.h"

#تعریف_کرو LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_نقاب(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_خالی_تھاواں_MASKNAMES;

#تعریف_کرو LOOPDEV_DEBUG_INIT	(1 << 1)
#تعریف_کرو LOOPDEV_DEBUG_CXT	(1 << 2)
#تعریف_کرو LOOPDEV_DEBUG_ITER	(1 << 3)
#تعریف_کرو LOOPDEV_DEBUG_تیاری	(1 << 4)

#تعریف_کرو DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

ساكن فراغ loopdev_init_debug(فراغ)
{
	إذا (loopdev_debug_نقاب)
		أرجع;
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
		صحيح _c = 0, _e;					\
		افعل {						\
			errno = 0;				\
			_e = x;					\
			إذا (_e == 0 || errno != EAGAIN)		\
				اكسر;				\
			إذا (_c >= LOOPDEV_MAX_TRIES)		\
				اكسر;				\
			xusleep(250000);			\
			_c++;					\
		} طالما (1);					\
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
صحيح loopcxt_سیٹ_device(بنية loopdev_cxt *lc, ثابت حرف *device)
{
	إذا (!lc)
		أرجع -EINVAL;

	إذا (lc->fd >= 0) {
		close(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_گواچا = 0;
	lc->devno = 0;
	lc->منوال = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	memset(&lc->config, 0, حجم(lc->config));

	/* set new */
	إذا (device) {
		إذا (*device != '/') {
			ثابت حرف *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			إذا (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				إذا (لڑی_لمبائی(device) < 5)
					أرجع -1;
				device += 4;
				dir = رستہ_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, حجم(lc->device), "%s%s",
				dir, device);
		} وإلا
			xstrncpy(lc->device, device, حجم(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_رستہ(lc->sysfs);
	lc->sysfs = عدم;
	أرجع 0;
}

صحيح loopcxt_has_device(بنية loopdev_cxt *lc)
{
	أرجع lc && *lc->device;
}

dev_t loopcxt_get_devno(بنية loopdev_cxt *lc)
{
	إذا (!lc || !loopcxt_has_device(lc))
		أرجع 0;
	إذا (!lc->devno)
		lc->devno = sysfs_devname_منزل_devno(lc->device);
	أرجع lc->devno;
}

صحيح loopcxt_is_گواچا(بنية loopdev_cxt *lc)
{
	إذا (!lc || !loopcxt_has_device(lc))
		أرجع 0;
	إذا (lc->is_گواچا)
		أرجع 1;

	lc->is_گواچا = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	أرجع lc->is_گواچا;
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
صحيح loopcxt_init(بنية loopdev_cxt *lc, صحيح flags)
{
	صحيح rc;
	بنية stat st;
	بنية loopdev_cxt dummy = UL_LOOPDEVCXT_خالی_تھاواں;

	إذا (!lc)
		أرجع -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	حافظہ_نقل(lc, &dummy, حجم(dummy));
	lc->flags = flags;

	rc = loopcxt_سیٹ_device(lc, عدم);
	إذا (rc)
		أرجع rc;

	إذا (stat(رستہ_SYS_بلاک, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	إذا (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_نسخہ() >= KERNEL_نسخہ(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	إذا (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(رستہ_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	أرجع 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
فراغ loopcxt_deinit(بنية loopdev_cxt *lc)
{
	صحيح errsv = errno;

	إذا (!lc)
		أرجع;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	حرر(lc->filename);
	lc->filename = عدم;

	ignore_نتیجہ( loopcxt_سیٹ_device(lc, عدم) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
حرف *loopcxt_strdup_device(بنية loopdev_cxt *lc)
{
	إذا (!lc || !*lc->device)
		أرجع عدم;
	أرجع strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
ثابت حرف *loopcxt_get_device(بنية loopdev_cxt *lc)
{
	أرجع lc && *lc->device ? lc->device : عدم;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
ساكن بنية رستہ_cxt *loopcxt_get_sysfs(بنية loopdev_cxt *lc)
{
	إذا (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		أرجع عدم;

	إذا (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		إذا (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			أرجع عدم;
		}

		lc->sysfs = ul_new_sysfs_رستہ(devno, عدم, عدم);
		إذا (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	أرجع lc->sysfs;
}

ساكن صحيح __loopcxt_get_fd(بنية loopdev_cxt *lc, mode_t منوال)
{
	صحيح old = -1;

	إذا (!lc || !*lc->device)
		أرجع -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	إذا (lc->fd >= 0 && منوال == O_RDWR && lc->منوال == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	إذا (lc->fd < 0) {
		lc->منوال = منوال;
		lc->fd = open(lc->device, lc->منوال | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				منوال == O_RDONLY ? "ro" :
				منوال == O_RDWR ? "rw" : "??"));

		إذا (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	إذا (old >= 0)
		close(old);
	أرجع lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
صحيح loopcxt_get_fd(بنية loopdev_cxt *lc)
{
	أرجع __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_init_iterator(بنية loopdev_cxt *lc, صحيح flags)
{
	بنية loopdev_iter *iter;
	بنية stat st;

	إذا (!lc)
		أرجع -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, حجم(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	إذا (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		إذا (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(رستہ_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	أرجع 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_deinit_iterator(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter;

	إذا (!lc)
		أرجع -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	حرر(iter->minors);
	إذا (iter->proc)
		fclose(iter->proc);
	إذا (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, حجم(*iter));
	أرجع 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
ساكن صحيح loopiter_سیٹ_device(بنية loopdev_cxt *lc, ثابت حرف *device)
{
	صحيح rc = loopcxt_سیٹ_device(lc, device);
	صحيح ورتیا;

	إذا (rc)
		أرجع rc;

	إذا (!(lc->iter.flags & LOOPITER_FL_ورتیا) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		أرجع 0;	/* caller does not care about device status */

	ورتیا = loopcxt_get_وتھ(lc, عدم) == 0;

	إذا ((lc->iter.flags & LOOPITER_FL_ورتیا) && ورتیا)
		أرجع 0;

	إذا ((lc->iter.flags & LOOPITER_FL_FREE) && !ورتیا)
		أرجع 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_نتیجہ( loopcxt_سیٹ_device(lc, عدم) );
	أرجع 1;
}

ساكن صحيح cmpnum(ثابت فراغ *p1, ثابت فراغ *p2)
{
	أرجع (((* (ثابت صحيح *) p1) > (* (ثابت صحيح *) p2)) -
			((* (ثابت صحيح *) p1) < (* (ثابت صحيح *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
ساكن صحيح loop_scandir(ثابت حرف *dirname, صحيح **ary, صحيح hasprefix)
{
	DIR *dir;
	بنية dirent *d;
	بنانشانی صحيح n, گنتی = 0, arylen = 0;

	إذا (!dirname || !ary)
		أرجع 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	إذا (!dir)
		أرجع 0;
	حرر(*ary);
	*ary = عدم;

	طالما((d = readdir(dir))) {
#جے_تعریف_شدہ _DIRENT_HAVE_D_TYPE
		إذا (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			استمر;
#جے_انت
		إذا (is_dotdir_dirent(d))
			استمر;

		إذا (hasprefix) {
			/* /dev/loop<N> */
			إذا (لڑی_توں_پڑھو(d->d_name, "loop%u", &n) != 1)
				استمر;
		} وإلا {
			/* /dev/loop/<N> */
			حرف *سرا = عدم;

			errno = 0;
			n = strtol(d->d_name, &سرا, 10);
			إذا (d->d_name == سرا || (سرا && *سرا) || errno)
				استمر;
		}
		إذا (n < LOOPDEV_DEFAULT_NNODES)
			استمر;			/* ignore loop<0..7> */

		إذا (گنتی + 1 > arylen) {
			صحيح *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, حجم(صحيح));
			إذا (!tmp) {
				حرر(*ary);
				*ary = عدم;
				closedir(dir);
				أرجع -1;
			}
			*ary = tmp;
		}
		إذا (*ary)
			(*ary)[گنتی++] = n;
	}
	إذا (گنتی && *ary)
		qsort(*ary, گنتی, حجم(صحيح), cmpnum);

	closedir(dir);
	أرجع گنتی;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
ساكن صحيح loopcxt_اگلا_from_proc(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter = &lc->iter;
	حرف buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	إذا (!iter->proc)
		iter->proc = fopen(رستہ_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	إذا (!iter->proc)
		أرجع 1;

	طالما (fgets(buf, حجم(buf), iter->proc)) {
		بنانشانی صحيح m;
		حرف name[128 + 1];


		إذا (لڑی_توں_پڑھو(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			استمر;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		إذا (loopiter_سیٹ_device(lc, name) == 0)
			أرجع 0;
	}

	أرجع 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
ساكن صحيح loopcxt_اگلا_from_sysfs(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter = &lc->iter;
	بنية dirent *d;
	صحيح fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	إذا (!iter->sysblock)
		iter->sysblock = opendir(رستہ_SYS_بلاک);

	إذا (!iter->sysblock)
		أرجع 1;

	fd = dirfd(iter->sysblock);

	طالما ((d = readdir(iter->sysblock))) {
		حرف name[NAME_MAX + 18 + 1];
		بنية stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		إذا (is_dotdir_dirent(d) || لڑی_مقابلہ(d->d_name, "loop", 4) != 0)
			استمر;

		snprintf(name, حجم(name), "%s/loop/backing_file", d->d_name);
		إذا (fstatat(fd, name, &st, 0) != 0)
			استمر;

		إذا (loopiter_سیٹ_device(lc, d->d_name) == 0)
			أرجع 0;
	}

	أرجع 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
صحيح loopcxt_اگلا(بنية loopdev_cxt *lc)
{
	بنية loopdev_iter *iter;

	إذا (!lc)
		أرجع -EINVAL;


	iter = &lc->iter;
	إذا (iter->پورا)
		أرجع 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	إذا (iter->flags & LOOPITER_FL_ورتیا) {
		صحيح rc;

		إذا (loopcxt_sysfs_دستیاب(lc))
			rc = loopcxt_اگلا_from_sysfs(lc);
		وإلا
			rc = loopcxt_اگلا_from_proc(lc);
		إذا (rc == 0)
			أرجع 0;
		اذهب پورا;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	إذا (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		لكل (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", iter->ncur);

			إذا (loopiter_سیٹ_device(lc, name) == 0)
				أرجع 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	إذا (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(رستہ_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	لكل (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		حرف name[16];
		snprintf(name, حجم(name), "loop%d", iter->minors[iter->ncur]);

		إذا (loopiter_سیٹ_device(lc, name) == 0)
			أرجع 0;
	}
پورا:
	loopcxt_deinit_iterator(lc);
	أرجع 1;
}

/*
 * @device: path to device
 */
صحيح is_loopdev(ثابت حرف *device)
{
	بنية stat st;
	صحيح rc = 0;

	إذا (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	وإلا إذا (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	وإلا إذا (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		حرف name[PATH_MAX], *cn, *p = عدم;

		snprintf(name, حجم(name), رستہ_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_رستہ(name);
		إذا (cn)
			p = stripoff_آخری_component(cn);
		rc = p && ul_startswith(p, "loop");
		حرر(cn);
	}

	إذا (rc == 0)
		errno = ENODEV;
	أرجع rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
بنية loop_info64 *loopcxt_get_info(بنية loopdev_cxt *lc)
{
	صحيح fd;

	إذا (!lc || lc->info_failed) {
		errno = EINVAL;
		أرجع عدم;
	}
	errno = 0;
	إذا (lc->has_info)
		أرجع &lc->config.info;

	fd = loopcxt_get_fd(lc);
	إذا (fd < 0)
		أرجع عدم;

	إذا (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		أرجع &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	أرجع عدم;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
حرف *loopcxt_get_backing_file(بنية loopdev_cxt *lc)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	حرف *res = عدم;

	إذا (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_رستہ_read_string(sysfs, &res, "loop/backing_file");

	إذا (!res && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);

		إذا (lo) {
			lo->lo_file_name[LO_NAME_ناپ - 2] = '*';
			lo->lo_file_name[LO_NAME_ناپ - 1] = '\0';
			res = strdup((حرف *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	أرجع res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
حرف *loopcxt_get_refname(بنية loopdev_cxt *lc)
{
	حرف *res = عدم;
	بنية loop_info64 *lo = loopcxt_get_info(lc);

	إذا (lo) {
		lo->lo_file_name[LO_NAME_ناپ - 1] = '\0';
		res = strdup((حرف *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	أرجع res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_وتھ(بنية loopdev_cxt *lc, uint64_t *وتھ)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	إذا (sysfs)
		إذا (ul_رستہ_read_u64(sysfs, وتھ, "loop/offset") == 0)
			rc = 0;

	إذا (rc && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		إذا (lo) {
			إذا (وتھ)
				*وتھ = lo->lo_وتھ;
			rc = 0;
		} وإلا
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	أرجع rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_blocksize(بنية loopdev_cxt *lc, uint64_t *blocksize)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	إذا (sysfs)
		إذا (ul_رستہ_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	إذا (rc) {
		صحيح fd = loopcxt_get_fd(lc);
		صحيح sz = 0;

		إذا (fd < 0)
			أرجع -EINVAL;
		rc = blkdev_get_sector_ناپ(fd, &sz);
		إذا (rc)
			أرجع rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	أرجع rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_sizelimit(بنية loopdev_cxt *lc, uint64_t *ناپ)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);
	صحيح rc = -EINVAL;

	إذا (sysfs)
		إذا (ul_رستہ_read_u64(sysfs, ناپ, "loop/sizelimit") == 0)
			rc = 0;

	إذا (rc && loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		إذا (lo) {
			إذا (ناپ)
				*ناپ = lo->lo_sizelimit;
			rc = 0;
		} وإلا
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	أرجع rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_encrypt_type(بنية loopdev_cxt *lc, uint32_t *type)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	/* not provided by sysfs */
	إذا (lo) {
		إذا (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} وإلا
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	أرجع rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
ثابت حرف *loopcxt_get_crypt_name(بنية loopdev_cxt *lc)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);

	إذا (lo)
		أرجع (حرف *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	أرجع عدم;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_backing_devno(بنية loopdev_cxt *lc, dev_t *devno)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	إذا (lo) {
		إذا (devno)
			*devno = lo->lo_device;
		rc = 0;
	} وإلا
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	أرجع rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
صحيح loopcxt_get_backing_inode(بنية loopdev_cxt *lc, ino_t *ino)
{
	بنية loop_info64 *lo = loopcxt_get_info(lc);
	صحيح rc;

	إذا (lo) {
		إذا (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} وإلا
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	أرجع rc;
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
صحيح loopmod_supports_partscan(فراغ)
{
	صحيح rc, ret = 0;
	FILE *f;

	إذا (get_linux_نسخہ() >= KERNEL_نسخہ(3,2,0))
		أرجع 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	أرجع rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
صحيح loopcxt_is_partscan(بنية loopdev_cxt *lc)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	إذا (sysfs) {
		/* kernel >= 3.2 */
		صحيح fl;
		إذا (ul_رستہ_read_s32(sysfs, &fl, "loop/partscan") == 0)
			أرجع fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	أرجع loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
صحيح loopcxt_is_autoclear(بنية loopdev_cxt *lc)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	إذا (sysfs) {
		صحيح fl;
		إذا (ul_رستہ_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			أرجع fl;
	}

	إذا (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		إذا (lo)
			أرجع lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	أرجع 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
صحيح loopcxt_is_readonly(بنية loopdev_cxt *lc)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	إذا (sysfs) {
		صحيح fl;
		إذا (ul_رستہ_read_s32(sysfs, &fl, "ro") == 0)
			أرجع fl;
	}

	إذا (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		إذا (lo)
			أرجع lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	أرجع 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
صحيح loopcxt_is_dio(بنية loopdev_cxt *lc)
{
	بنية رستہ_cxt *sysfs = loopcxt_get_sysfs(lc);

	إذا (sysfs) {
		صحيح fl;
		إذا (ul_رستہ_read_s32(sysfs, &fl, "loop/dio") == 0)
			أرجع fl;
	}
	إذا (loopcxt_ioctl_enabled(lc)) {
		بنية loop_info64 *lo = loopcxt_get_info(lc);
		إذا (lo)
			أرجع lo->lo_flags & LO_FLAGS_سدھا_IO;
	}
	أرجع 0;
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
صحيح loopcxt_is_ورتیا(بنية loopdev_cxt *lc,
		    بنية stat *st,
		    ثابت حرف *backing_file,
		    uint64_t وتھ,
		    uint64_t sizelimit,
		    صحيح flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	إذا (!lc)
		أرجع 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	إذا (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		إذا (ino == st->st_ino && dev == st->st_dev)
			اذهب لبھیا;

		/* don't use filename if we have devno and inode */
		أرجع 0;
	}

	/* poor man's solution */
	إذا (backing_file) {
		حرف *name = loopcxt_get_backing_file(lc);
		صحيح rc = name && strcmp(name, backing_file) == 0;

		حرر(name);
		إذا (rc)
			اذهب لبھیا;
	}

	أرجع 0;
لبھیا:
	إذا (flags & LOOPDEV_FL_وتھ) {
		uint64_t off = 0;

		صحيح rc = loopcxt_get_وتھ(lc, &off) == 0 && off == وتھ;

		إذا (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			أرجع loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		أرجع rc;
	}
	أرجع 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_سیٹ_وتھ(بنية loopdev_cxt *lc, uint64_t وتھ)
{
	إذا (!lc)
		أرجع -EINVAL;
	lc->config.info.lo_وتھ = وتھ;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, وتھ));
	أرجع 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_سیٹ_sizelimit(بنية loopdev_cxt *lc, uint64_t sizelimit)
{
	إذا (!lc)
		أرجع -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	أرجع 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
صحيح loopcxt_سیٹ_blocksize(بنية loopdev_cxt *lc, uint64_t blocksize)
{
	إذا (!lc)
		أرجع -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	أرجع 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح loopcxt_سیٹ_flags(بنية loopdev_cxt *lc, uint32_t flags)
{
	إذا (!lc)
		أرجع -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (بنانشانی) flags));
	أرجع 0;
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
صحيح loopcxt_سیٹ_refname(بنية loopdev_cxt *lc, ثابت حرف *refname)
{
	إذا (!lc)
		أرجع -EINVAL;

	memset(lc->config.info.lo_file_name, 0, حجم(lc->config.info.lo_file_name));
	إذا (refname)
		xstrncpy((حرف *)lc->config.info.lo_file_name, refname, LO_NAME_ناپ);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (حرف *)lc->config.info.lo_file_name));
	أرجع 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
صحيح loopcxt_سیٹ_backing_file(بنية loopdev_cxt *lc, ثابت حرف *filename)
{
	إذا (!lc)
		أرجع -EINVAL;

	إذا (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	وإلا
		lc->filename = ul_canonicalize_رستہ(filename);
	إذا (!lc->filename)
		أرجع -errno;

	إذا (!lc->config.info.lo_file_name[0])
		loopcxt_سیٹ_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	أرجع 0;
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
ساكن صحيح loopcxt_check_ناپ(بنية loopdev_cxt *lc, صحيح file_fd)
{
	uint64_t ناپ, expected_ناپ;
	صحيح dev_fd;
	بنية stat st;

	إذا (!lc->config.info.lo_وتھ && !lc->config.info.lo_sizelimit)
		أرجع 0;

	إذا (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		أرجع -errno;
	}
	إذا (S_ISBLK(st.st_mode)) {
		إذا (blkdev_get_ناپ(file_fd, &expected_ناپ)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			أرجع -errno;
		}
	} وإلا
		expected_ناپ = st.st_size;

	إذا (expected_ناپ == 0 || expected_ناپ <= lc->config.info.lo_وتھ) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		أرجع 0;	/* ignore this error */
	}

	إذا (lc->config.info.lo_وتھ > 0)
		expected_ناپ -= lc->config.info.lo_وتھ;

	إذا (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_ناپ)
		expected_ناپ = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	إذا (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		أرجع -errno;
	}

	إذا (blkdev_get_ناپ(dev_fd, &ناپ)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		أرجع -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	إذا (expected_ناپ % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_ناپ = (expected_ناپ >> 9) << 9;
	}

	إذا (expected_ناپ != ناپ) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      ناپ, expected_ناپ));

		إذا (loopcxt_ioctl_سمرتھا(lc)) {
			/* ioctl not available */
			إذا (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			أرجع -errno;
		}

		إذا (blkdev_get_ناپ(dev_fd, &ناپ))
			أرجع -errno;

		إذا (expected_ناپ != ناپ) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					ناپ, expected_ناپ));
			أرجع -errno;
		}
	}

	أرجع 0;
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
صحيح loopcxt_تیاری_device(بنية loopdev_cxt *lc)
{
	صحيح file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, منوال = O_RDWR;
	صحيح rc = -1, cnt = 0;
	صحيح errsv = 0;
	صحيح fallback = 0;

	إذا (!lc || !*lc->device || !lc->filename)
		أرجع -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	إذا (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		منوال = O_RDONLY;

	إذا (lc->config.info.lo_flags & LO_FLAGS_سدھا_IO)
		flags |= O_سدھا;
	إذا (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, منوال | flags, 0);
	وإلا
		file_fd = open(lc->filename, منوال | flags);

	إذا (file_fd < 0 && منوال != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		منوال = O_RDONLY;
		إذا (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, منوال | flags, 0);
		وإلا
			file_fd = open(lc->filename, منوال | flags);
	}
	إذا (file_fd < 0) {
		DBG_OBJ(تیاری, lc, ul_debug("open backing file failed: %m"));
		أرجع -errno;
	}
	DBG_OBJ(تیاری, lc, ul_debug("backing file open: OK"));

	إذا (منوال == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	وإلا
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	افعل {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, منوال);
		إذا (dev_fd >= 0 || lc->control_ok == 0)
			اكسر;
		إذا (errno != EACCES && errno != ENOENT)
			اكسر;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} طالما (cnt++ < 16);

	إذا (dev_fd < 0) {
		rc = -errno;
		اذهب err;
	}

	DBG_OBJ(تیاری, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	إذا (lc->blocksize > 0)
		lc->config.بلاک_ناپ = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	إذا (rc != 0) {
		errsv = errno;
		إذا (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(تیاری, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			اذهب err;
		}
		fallback = 1;
	} وإلا {
		DBG_OBJ(تیاری, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	إذا (fallback) {
		إذا (ioctl(dev_fd, LOOP_سیٹ_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_FD failed: %m"));
			اذهب err;
		}

		DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_FD: OK"));

		إذا (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			اذهب err;
		}

		إذا ((rc = loopcxt_ioctl_حالت(lc)) < 0) {
			errsv = -rc;
			اذهب err;
		}
	}

	إذا ((rc = loopcxt_check_ناپ(lc, file_fd)))
		اذهب err;

	close(file_fd);

	memset(&lc->config, 0, حجم(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(تیاری, lc, ul_debug("success [rc=0]"));
	أرجع 0;
err:
	إذا (file_fd >= 0)
		close(file_fd);
	إذا (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	إذا (errsv)
		errno = errsv;

	DBG_OBJ(تیاری, lc, ul_debug("failed [rc=%d]", rc));
	أرجع rc;
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
صحيح loopcxt_ioctl_حالت(بنية loopdev_cxt *lc)
{
	صحيح dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	إذا (dev_fd < 0)
		أرجع -errno;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_سیٹ_STATUS64, &lc->config.info) );
	إذا (rc != 0) {
		DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		أرجع rc;
	}

	DBG_OBJ(تیاری, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	أرجع 0;
}

صحيح loopcxt_ioctl_سمرتھا(بنية loopdev_cxt *lc)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	إذا (fd < 0)
		أرجع -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_سیٹ_سمرتھا, 0) );
	إذا (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		أرجع rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	أرجع 0;
}

صحيح loopcxt_ioctl_dio(بنية loopdev_cxt *lc, بنانشانی لما use_dio)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	إذا (fd < 0)
		أرجع -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_سیٹ_سدھا_IO, use_dio) );
	إذا (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		أرجع rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	أرجع 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
صحيح loopcxt_ioctl_blocksize(بنية loopdev_cxt *lc, uint64_t blocksize)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	إذا (fd < 0)
		أرجع -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_سیٹ_بلاک_ناپ, (بنانشانی لما) blocksize) );
	إذا (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		أرجع rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	أرجع 0;
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
ساكن صحيح loopcxt_get_device_nr(بنية loopdev_cxt *lc, صحيح *nr)
{
	ثابت حرف *p, *dev;
	صحيح rc = -EINVAL;

	errno = 0;
	إذا (!lc || !nr)
		أرجع rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	إذا (!dev)
		اذهب پورا;

	p = strrchr(dev, '/');
	إذا (!p)
		اذهب پورا;

	إذا (لڑی_توں_پڑھو(p, "/loop%d", nr) != 1 && لڑی_توں_پڑھو(p, "/%d", nr) != 1)
		اذهب پورا;

	إذا (*nr < 0)
		اذهب پورا;
	rc = 0;
پورا:
	إذا (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	أرجع rc;
}

صحيح loopcxt_detach_device(بنية loopdev_cxt *lc)
{
	صحيح rc, fd = loopcxt_get_fd(lc);

	إذا (fd < 0)
		أرجع -EINVAL;

	DBG_OBJ(تیاری, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	إذا (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		أرجع rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	أرجع 0;
}

صحيح loopcxt_remove_device(بنية loopdev_cxt *lc)
{
	صحيح rc = -EINVAL;
	صحيح ctl, nr = -1;

	إذا (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		اذهب پورا;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	إذا (rc)
		اذهب پورا;

	ctl = open(رستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	إذا (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
پورا:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	أرجع rc;
}

صحيح loopcxt_جوڑو_device(بنية loopdev_cxt *lc)
{
	صحيح rc = -EINVAL;
	صحيح ctl, nr = -1;

	إذا (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		اذهب پورا;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	إذا (rc)
		اذهب پورا;

	ctl = open(رستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	إذا (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_جوڑو, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
پورا:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	أرجع rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
صحيح loopcxt_لبھو_بن_ورتیا(بنية loopdev_cxt *lc)
{
	صحيح rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	إذا (lc->flags & LOOPDEV_FL_CONTROL) {
		صحيح ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(رستہ_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		إذا (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		وإلا
			rc = -errno;
		إذا (rc >= 0) {
			حرف name[16];
			snprintf(name, حجم(name), "loop%d", rc);

			rc = loopiter_سیٹ_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		إذا (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	إذا (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		إذا (rc)
			أرجع rc;

		rc = loopcxt_اگلا(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		إذا (rc)
			أرجع -ENOENT;
	}
	أرجع rc;
}



/*
 * Return: TRUE/FALSE
 */
صحيح loopdev_is_autoclear(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	صحيح rc;

	إذا (!device)
		أرجع 0;

	rc = loopcxt_init(&lc, 0);
	إذا (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	إذا (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	أرجع rc;
}

حرف *loopdev_get_backing_file(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	حرف *res = عدم;

	إذا (!device)
		أرجع عدم;
	إذا (loopcxt_init(&lc, 0))
		أرجع عدم;
	إذا (loopcxt_سیٹ_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	أرجع res;
}

/*
 * Returns: TRUE/FALSE
 */
صحيح loopdev_has_backing_file(ثابت حرف *device)
{
	حرف *tmp = loopdev_get_backing_file(device);

	إذا (tmp) {
		حرر(tmp);
		أرجع 1;
	}
	أرجع 0;
}

/*
 * Returns: TRUE/FALSE
 */
صحيح loopdev_is_ورتیا(ثابت حرف *device, ثابت حرف *filename,
		    uint64_t وتھ, uint64_t sizelimit, صحيح flags)
{
	بنية loopdev_cxt lc;
	بنية stat st;
	صحيح rc = 0;

	إذا (!device || !filename)
		أرجع 0;

	rc = loopcxt_init(&lc, 0);
	إذا (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	إذا (rc)
		أرجع rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_ورتیا(&lc, rc ? &st : عدم, filename, وتھ, sizelimit, flags);

	loopcxt_deinit(&lc);
	أرجع rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
صحيح loopdev_detach(ثابت حرف *device)
{
	بنية loopdev_cxt lc;
	صحيح rc;

	إذا (!device)
		أرجع -EINVAL;

	rc = loopcxt_init(&lc, 0);
	إذا (!rc)
		rc = loopcxt_سیٹ_device(&lc, device);
	إذا (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	أرجع rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
صحيح loopcxt_لبھو_by_backing_file(بنية loopdev_cxt *lc, ثابت حرف *filename,
				 uint64_t وتھ, uint64_t sizelimit, صحيح flags)
{
	صحيح rc, hasst;
	بنية stat st;

	إذا (!filename)
		أرجع -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ورتیا);
	إذا (rc)
		أرجع rc;

	طالما ((rc = loopcxt_اگلا(lc)) == 0) {

		إذا (loopcxt_is_ورتیا(lc, hasst ? &st : عدم,
				    filename, وتھ, sizelimit, flags))
			اكسر;
	}

	loopcxt_deinit_iterator(lc);
	أرجع rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
صحيح loopcxt_لبھو_overlap(بنية loopdev_cxt *lc, ثابت حرف *filename,
			   uint64_t وتھ, uint64_t sizelimit)
{
	صحيح rc, hasst;
	بنية stat st;

	إذا (!filename)
		أرجع -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_ورتیا);
	إذا (rc)
		أرجع rc;

	طالما ((rc = loopcxt_اگلا(lc)) == 0) {
		uint64_t lc_sizelimit, lc_وتھ;

		rc = loopcxt_is_ورتیا(lc, hasst ? &st : عدم,
				     filename, وتھ, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		إذا (rc <= 0)
			استمر;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_وتھ(lc, &lc_وتھ);
		إذا (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			استمر;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		إذا (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			استمر;
		}

		/* full match */
		إذا (lc_sizelimit == sizelimit && lc_وتھ == وتھ) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			اذهب لبھیا;
		}

		/* overlap */
		إذا (lc_sizelimit != 0 && وتھ >= lc_وتھ + lc_sizelimit)
			استمر;
		إذا (sizelimit != 0 && وتھ + sizelimit <= lc_وتھ)
			استمر;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			اذهب لبھیا;
	}

	إذا (rc == 1)
		rc = 0;	/* not found */
لبھیا:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	أرجع rc;
}

/*
 * Returns allocated string with device name
 */
حرف *loopdev_لبھو_by_backing_file(ثابت حرف *filename, uint64_t وتھ, uint64_t sizelimit, صحيح flags)
{
	بنية loopdev_cxt lc;
	حرف *res = عدم;

	إذا (!filename)
		أرجع عدم;

	إذا (loopcxt_init(&lc, 0))
		أرجع عدم;
	إذا (loopcxt_لبھو_by_backing_file(&lc, filename, وتھ, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	أرجع res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
صحيح loopdev_گنتی_by_backing_file(ثابت حرف *filename, حرف **loopdev)
{
	بنية loopdev_cxt lc;
	صحيح گنتی = 0, rc;

	إذا (!filename)
		أرجع -1;

	rc = loopcxt_init(&lc, 0);
	إذا (rc)
		أرجع rc;
	إذا (loopcxt_init_iterator(&lc, LOOPITER_FL_ورتیا))
		أرجع -1;

	طالما(loopcxt_اگلا(&lc) == 0) {
		حرف *backing = loopcxt_get_backing_file(&lc);

		إذا (!backing || strcmp(backing, filename) != 0) {
			حرر(backing);
			استمر;
		}

		حرر(backing);
		إذا (loopdev && گنتی == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		گنتی++;
	}

	loopcxt_deinit(&lc);

	إذا (loopdev && گنتی > 1) {
		حرر(*loopdev);
		*loopdev = عدم;
	}
	أرجع گنتی;
}

#جے_تعریف_شدہ TEST_پروگرام_LOOPDEV
صحيح مکھ(صحيح argc, حرف *argv[])
{
	إذا (argc < 2)
		اذهب usage;

	إذا (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		لکھو("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	وإلا
		اذهب usage;

	أرجع EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			پروگرام_invocation_short_name);
	أرجع EXIT_FAILURE;
}
#جے_انت

