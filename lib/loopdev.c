
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
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/mman.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <dirent.h>

#అన్తర్భావ "linux_version.h"
#అన్తర్భావ "c.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "sysfs.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "loopdev.h"
#అన్తర్భావ "canonicalize.h"
#అన్తర్భావ "blkdev.h"
#అన్తర్భావ "debug.h"
#అన్తర్భావ "fileutils.h"

#నిర్వచించు LOOPDEV_MAX_TRIES	10

/*
 * Debug stuff (based on include/debug.h)
 */
స్థిర UL_DEBUG_DEFINE_ముసుగు(loopdev);
UL_DEBUG_DEFINE_MASKNAMES(loopdev) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు LOOPDEV_DEBUG_INIT	(1 << 1)
#నిర్వచించు LOOPDEV_DEBUG_CXT	(1 << 2)
#నిర్వచించు LOOPDEV_DEBUG_ITER	(1 << 3)
#నిర్వచించు LOOPDEV_DEBUG_అమరిక	(1 << 4)

#నిర్వచించు DBG(m, x)		__UL_DBG(loopdev, LOOPDEV_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(loopdev, LOOPDEV_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(loopdev, LOOPDEV_DEBUG_, m, x)

స్థిర శూన్య loopdev_init_debug(శూన్య)
{
	ఒకవేళ (loopdev_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(loopdev, LOOPDEV_DEBUG_, 0, LOOPDEV_DEBUG);
}

/*
 * see loopcxt_init()
 */
#నిర్వచించు loopcxt_ioctl_enabled(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOIOCTL))
#నిర్వచించు loopcxt_sysfs_అందుబాటు(_lc)	(!((_lc)->flags & LOOPDEV_FL_NOSYSFS)) \
					 && !loopcxt_ioctl_enabled(_lc)

/*
 * Calls @x and repeat on EAGAIN
 */
#నిర్వచించు repeat_on_eagain(x) __extension__ ({			\
		పూర్ణాంకం _c = 0, _e;					\
		చేయి {						\
			errno = 0;				\
			_e = x;					\
			ఒకవేళ (_e == 0 || errno != EAGAIN)		\
				ఆపు;				\
			ఒకవేళ (_c >= LOOPDEV_MAX_TRIES)		\
				ఆపు;				\
			xusleep(250000);			\
			_c++;					\
		} అయితే (1);					\
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
పూర్ణాంకం loopcxt_సమితి_device(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *device)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;

	ఒకవేళ (lc->fd >= 0) {
		close(lc->fd);
		DBG_OBJ(CXT, lc, ul_debug("closing old open fd"));
	}
	lc->fd = -1;
	lc->is_కోల్పోయిన = 0;
	lc->devno = 0;
	lc->బహుళకం = O_RDONLY;
	lc->blocksize = 0;
	lc->has_info = 0;
	lc->info_failed = 0;
	*lc->device = '\0';
	memset(&lc->config, 0, పరిమాణం(lc->config));

	/* set new */
	ఒకవేళ (device) {
		ఒకవేళ (*device != '/') {
			స్థిరరాశి అక్షరం *dir = _PATH_DEV;

			/* compose device name for /dev/loop<n> or /dev/loop/<n> */
			ఒకవేళ (lc->flags & LOOPDEV_FL_DEVSUBDIR) {
				ఒకవేళ (అక్షరశ్రేణి_పొడవు(device) < 5)
					తిరిగి -1;
				device += 4;
				dir = మార్గం_DEV_LOOP "/";	/* _PATH_DEV uses trailing slash */
			}
			snprintf(lc->device, పరిమాణం(lc->device), "%s%s",
				dir, device);
		} లేకపోతే
			xstrncpy(lc->device, device, పరిమాణం(lc->device));

		DBG_OBJ(CXT, lc, ul_debug("%s name assigned", device));
	}

	ul_unref_మార్గం(lc->sysfs);
	lc->sysfs = ఖాళీ;
	తిరిగి 0;
}

పూర్ణాంకం loopcxt_has_device(నిర్మాణం loopdev_cxt *lc)
{
	తిరిగి lc && *lc->device;
}

dev_t loopcxt_get_devno(నిర్మాణం loopdev_cxt *lc)
{
	ఒకవేళ (!lc || !loopcxt_has_device(lc))
		తిరిగి 0;
	ఒకవేళ (!lc->devno)
		lc->devno = sysfs_devname_గమ్యం_devno(lc->device);
	తిరిగి lc->devno;
}

పూర్ణాంకం loopcxt_is_కోల్పోయిన(నిర్మాణం loopdev_cxt *lc)
{
	ఒకవేళ (!lc || !loopcxt_has_device(lc))
		తిరిగి 0;
	ఒకవేళ (lc->is_కోల్పోయిన)
		తిరిగి 1;

	lc->is_కోల్పోయిన = access(lc->device, F_OK) != 0
			&& loopcxt_get_devno(lc) != 0;

	తిరిగి lc->is_కోల్పోయిన;
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
పూర్ణాంకం loopcxt_init(నిర్మాణం loopdev_cxt *lc, పూర్ణాంకం flags)
{
	పూర్ణాంకం rc;
	నిర్మాణం stat st;
	నిర్మాణం loopdev_cxt dummy = UL_LOOPDEVCXT_ఖాళీలు;

	ఒకవేళ (!lc)
		తిరిగి -EINVAL;

	loopdev_init_debug();
	DBG_OBJ(CXT, lc, ul_debug("initialize context"));

	జ్ఞాపకం_నకలు(lc, &dummy, పరిమాణం(dummy));
	lc->flags = flags;

	rc = loopcxt_సమితి_device(lc, ఖాళీ);
	ఒకవేళ (rc)
		తిరిగి rc;

	ఒకవేళ (stat(మార్గం_SYS_ఖండం, &st) || !S_ISDIR(st.st_mode)) {
		lc->flags |= LOOPDEV_FL_NOSYSFS;
		lc->flags &= ~LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: disable /sys usage"));
	}

	ఒకవేళ (!(lc->flags & LOOPDEV_FL_NOSYSFS) &&
	    get_linux_సంస్కరణ() >= KERNEL_సంస్కరణ(2,6,37)) {
		/*
		 * Use only sysfs for basic information about loop devices
		 */
		lc->flags |= LOOPDEV_FL_NOIOCTL;
		DBG_OBJ(CXT, lc, ul_debug("init: ignore ioctls"));
	}

	ఒకవేళ (!(lc->flags & LOOPDEV_FL_CONTROL) && !stat(మార్గం_DEV_LOOPCTL, &st)) {
		lc->flags |= LOOPDEV_FL_CONTROL;
		DBG_OBJ(CXT, lc, ul_debug("init: loop-control detected "));
	}

	తిరిగి 0;
}

/*
 * @lc: context
 *
 * Deinitialize loop context
 */
శూన్య loopcxt_deinit(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం errsv = errno;

	ఒకవేళ (!lc)
		తిరిగి;

	DBG_OBJ(CXT, lc, ul_debug("de-initialize"));

	విడుదల(lc->filename);
	lc->filename = ఖాళీ;

	ignore_ఫలితం( loopcxt_సమితి_device(lc, ఖాళీ) );
	loopcxt_deinit_iterator(lc);

	errno = errsv;
}

/*
 * @lc: context
 *
 * Returns newly allocated device path.
 */
అక్షరం *loopcxt_strdup_device(నిర్మాణం loopdev_cxt *lc)
{
	ఒకవేళ (!lc || !*lc->device)
		తిరిగి ఖాళీ;
	తిరిగి strdup(lc->device);
}

/*
 * @lc: context
 *
 * Returns pointer device name in the @lc struct.
 */
స్థిరరాశి అక్షరం *loopcxt_get_device(నిర్మాణం loopdev_cxt *lc)
{
	తిరిగి lc && *lc->device ? lc->device : ఖాళీ;
}

/*
 * @lc: context
 *
 * Returns pointer to the sysfs context (see lib/sysfs.c)
 */
స్థిర నిర్మాణం మార్గం_cxt *loopcxt_get_sysfs(నిర్మాణం loopdev_cxt *lc)
{
	ఒకవేళ (!lc || !*lc->device || (lc->flags & LOOPDEV_FL_NOSYSFS))
		తిరిగి ఖాళీ;

	ఒకవేళ (!lc->sysfs) {
		dev_t devno = loopcxt_get_devno(lc);
		ఒకవేళ (!devno) {
			DBG_OBJ(CXT, lc, ul_debug("sysfs: failed devname to devno"));
			తిరిగి ఖాళీ;
		}

		lc->sysfs = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
		ఒకవేళ (!lc->sysfs)
			DBG_OBJ(CXT, lc, ul_debug("sysfs: init failed"));
	}

	తిరిగి lc->sysfs;
}

స్థిర పూర్ణాంకం __loopcxt_get_fd(నిర్మాణం loopdev_cxt *lc, mode_t బహుళకం)
{
	పూర్ణాంకం old = -1;

	ఒకవేళ (!lc || !*lc->device)
		తిరిగి -EINVAL;

	/* It's okay to return a FD with read-write permissions if someone
	 * asked for read-only, but you shouldn't do the opposite.
	 *
	 * (O_RDONLY is a widely usable default.)
	 */
	ఒకవేళ (lc->fd >= 0 && బహుళకం == O_RDWR && lc->బహుళకం == O_RDONLY) {
		DBG_OBJ(CXT, lc, ul_debug("closing already open device (mode mismatch)"));
		old = lc->fd;
		lc->fd = -1;
	}

	ఒకవేళ (lc->fd < 0) {
		lc->బహుళకం = బహుళకం;
		lc->fd = open(lc->device, lc->బహుళకం | O_CLOEXEC);
		DBG_OBJ(CXT, lc, ul_debug("open %s [%s]: %m", lc->device,
				బహుళకం == O_RDONLY ? "ro" :
				బహుళకం == O_RDWR ? "rw" : "??"));

		ఒకవేళ (lc->fd < 0 && old >= 0) {
			/* restore original on error */
			lc->fd = old;
			old = -1;
		}
	}

	ఒకవేళ (old >= 0)
		close(old);
	తిరిగి lc->fd;
}

/* default is read-only file descriptor, it's enough for all ioctls */
పూర్ణాంకం loopcxt_get_fd(నిర్మాణం loopdev_cxt *lc)
{
	తిరిగి __loopcxt_get_fd(lc, O_RDONLY);
}

/*
 * @lc: context
 * @flags: LOOPITER_FL_* flags
 *
 * Iterator can be used to scan list of the free or used loop devices.
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_init_iterator(నిర్మాణం loopdev_cxt *lc, పూర్ణాంకం flags)
{
	నిర్మాణం loopdev_iter *iter;
	నిర్మాణం stat st;

	ఒకవేళ (!lc)
		తిరిగి -EINVAL;


	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("initialize"));

	/* always zeroize
	 */
	memset(iter, 0, పరిమాణం(*iter));
	iter->ncur = -1;
	iter->flags = flags;
	iter->default_check = 1;

	ఒకవేళ (!lc->extra_check) {
		/*
		 * Check for /dev/loop/<N> subdirectory
		 */
		ఒకవేళ (!(lc->flags & LOOPDEV_FL_DEVSUBDIR) &&
		    stat(మార్గం_DEV_LOOP, &st) == 0 && S_ISDIR(st.st_mode))
			lc->flags |= LOOPDEV_FL_DEVSUBDIR;

		lc->extra_check = 1;
	}
	తిరిగి 0;
}

/*
 * @lc: context
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_deinit_iterator(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం loopdev_iter *iter;

	ఒకవేళ (!lc)
		తిరిగి -EINVAL;

	iter = &lc->iter;
	DBG_OBJ(ITER, iter, ul_debug("de-initialize"));

	విడుదల(iter->minors);
	ఒకవేళ (iter->proc)
		fclose(iter->proc);
	ఒకవేళ (iter->sysblock)
		closedir(iter->sysblock);

	memset(iter, 0, పరిమాణం(*iter));
	తిరిగి 0;
}

/*
 * Same as loopcxt_set_device, but also checks if the device is
 * associated with any file.
 *
 * Returns: <0 on error, 0 on success, 1 device does not match with
 *         LOOPITER_FL_{USED,FREE} flags.
 */
స్థిర పూర్ణాంకం loopiter_సమితి_device(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *device)
{
	పూర్ణాంకం rc = loopcxt_సమితి_device(lc, device);
	పూర్ణాంకం వాడిన;

	ఒకవేళ (rc)
		తిరిగి rc;

	ఒకవేళ (!(lc->iter.flags & LOOPITER_FL_వాడిన) &&
	    !(lc->iter.flags & LOOPITER_FL_FREE))
		తిరిగి 0;	/* caller does not care about device status */

	వాడిన = loopcxt_get_స్థానభ్రంశం(lc, ఖాళీ) == 0;

	ఒకవేళ ((lc->iter.flags & LOOPITER_FL_వాడిన) && వాడిన)
		తిరిగి 0;

	ఒకవేళ ((lc->iter.flags & LOOPITER_FL_FREE) && !వాడిన)
		తిరిగి 0;

	DBG_OBJ(ITER, &lc->iter, ul_debug("failed to use %s device", lc->device));

	ignore_ఫలితం( loopcxt_సమితి_device(lc, ఖాళీ) );
	తిరిగి 1;
}

స్థిర పూర్ణాంకం cmpnum(స్థిరరాశి శూన్య *p1, స్థిరరాశి శూన్య *p2)
{
	తిరిగి (((* (స్థిరరాశి పూర్ణాంకం *) p1) > (* (స్థిరరాశి పూర్ణాంకం *) p2)) -
			((* (స్థిరరాశి పూర్ణాంకం *) p1) < (* (స్థిరరాశి పూర్ణాంకం *) p2)));
}

/*
 * The classic scandir() is more expensive and less portable.
 * We needn't full loop device names -- loop numbers (loop<N>)
 * are enough.
 */
స్థిర పూర్ణాంకం loop_scandir(స్థిరరాశి అక్షరం *dirname, పూర్ణాంకం **ary, పూర్ణాంకం hasprefix)
{
	DIR *dir;
	నిర్మాణం dirent *d;
	గుర్తులేని పూర్ణాంకం n, లెక్క = 0, arylen = 0;

	ఒకవేళ (!dirname || !ary)
		తిరిగి 0;

	DBG(ITER, ul_debug("scan dir: %s", dirname));

	dir = opendir(dirname);
	ఒకవేళ (!dir)
		తిరిగి 0;
	విడుదల(*ary);
	*ary = ఖాళీ;

	అయితే((d = readdir(dir))) {
#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
		ఒకవేళ (d->d_type != DT_BLK && d->d_type != DT_UNKNOWN &&
		    d->d_type != DT_LNK)
			కొనసాగు;
#అయితే_ముగింపు
		ఒకవేళ (is_dotdir_dirent(d))
			కొనసాగు;

		ఒకవేళ (hasprefix) {
			/* /dev/loop<N> */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(d->d_name, "loop%u", &n) != 1)
				కొనసాగు;
		} లేకపోతే {
			/* /dev/loop/<N> */
			అక్షరం *చివర = ఖాళీ;

			errno = 0;
			n = strtol(d->d_name, &చివర, 10);
			ఒకవేళ (d->d_name == చివర || (చివర && *చివర) || errno)
				కొనసాగు;
		}
		ఒకవేళ (n < LOOPDEV_DEFAULT_NNODES)
			కొనసాగు;			/* ignore loop<0..7> */

		ఒకవేళ (లెక్క + 1 > arylen) {
			పూర్ణాంకం *tmp;

			arylen += 1;

			tmp = reallocarray(*ary, arylen, పరిమాణం(పూర్ణాంకం));
			ఒకవేళ (!tmp) {
				విడుదల(*ary);
				*ary = ఖాళీ;
				closedir(dir);
				తిరిగి -1;
			}
			*ary = tmp;
		}
		ఒకవేళ (*ary)
			(*ary)[లెక్క++] = n;
	}
	ఒకవేళ (లెక్క && *ary)
		qsort(*ary, లెక్క, పరిమాణం(పూర్ణాంకం), cmpnum);

	closedir(dir);
	తిరిగి లెక్క;
}

/*
 * Set the next *used* loop device according to /proc/partitions.
 *
 * Loop devices smaller than 512 bytes are invisible for this function.
 */
స్థిర పూర్ణాంకం loopcxt_తదుపరి_from_proc(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం loopdev_iter *iter = &lc->iter;
	అక్షరం buf[BUFSIZ];

	DBG_OBJ(ITER, iter, ul_debug("scan /proc/partitions"));

	ఒకవేళ (!iter->proc)
		iter->proc = fopen(మార్గం_PROC_PARTITIONS, "r" UL_CLOEXECSTR);
	ఒకవేళ (!iter->proc)
		తిరిగి 1;

	అయితే (fgets(buf, పరిమాణం(buf), iter->proc)) {
		గుర్తులేని పూర్ణాంకం m;
		అక్షరం name[128 + 1];


		ఒకవేళ (అక్షరశ్రేణి_చదువు(buf, " %u %*s %*s %128[^\n ]",
			   &m, name) != 2 || m != LOOPDEV_MAJOR)
			కొనసాగు;

		DBG_OBJ(ITER, iter, ul_debug("checking %s", name));

		ఒకవేళ (loopiter_సమితి_device(lc, name) == 0)
			తిరిగి 0;
	}

	తిరిగి 1;
}

/*
 * Set the next *used* loop device according to
 * /sys/block/loopN/loop/backing_file (kernel >= 2.6.37 is required).
 *
 * This is preferred method.
 */
స్థిర పూర్ణాంకం loopcxt_తదుపరి_from_sysfs(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం loopdev_iter *iter = &lc->iter;
	నిర్మాణం dirent *d;
	పూర్ణాంకం fd;

	DBG_OBJ(ITER, iter, ul_debug("scanning /sys/block"));

	ఒకవేళ (!iter->sysblock)
		iter->sysblock = opendir(మార్గం_SYS_ఖండం);

	ఒకవేళ (!iter->sysblock)
		తిరిగి 1;

	fd = dirfd(iter->sysblock);

	అయితే ((d = readdir(iter->sysblock))) {
		అక్షరం name[NAME_MAX + 18 + 1];
		నిర్మాణం stat st;

		DBG_OBJ(ITER, iter, ul_debug("check %s", d->d_name));

		ఒకవేళ (is_dotdir_dirent(d) || అక్షరశ్రేణి_పోలిక(d->d_name, "loop", 4) != 0)
			కొనసాగు;

		snprintf(name, పరిమాణం(name), "%s/loop/backing_file", d->d_name);
		ఒకవేళ (fstatat(fd, name, &st, 0) != 0)
			కొనసాగు;

		ఒకవేళ (loopiter_సమితి_device(lc, d->d_name) == 0)
			తిరిగి 0;
	}

	తిరిగి 1;
}

/*
 * @lc: context, has to initialized by loopcxt_init_iterator()
 *
 * Returns: 0 on success, < 0 on error, 1 at the end of scanning. The details
 *          about the current loop device are available by
 *          loopcxt_get_{fd,backing_file,device,offset, ...} functions.
 */
పూర్ణాంకం loopcxt_తదుపరి(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం loopdev_iter *iter;

	ఒకవేళ (!lc)
		తిరిగి -EINVAL;


	iter = &lc->iter;
	ఒకవేళ (iter->పూర్తి)
		తిరిగి 1;

	DBG_OBJ(ITER, iter, ul_debug("next"));

	/* A) Look for used loop devices in /proc/partitions ("losetup -a" only)
	 */
	ఒకవేళ (iter->flags & LOOPITER_FL_వాడిన) {
		పూర్ణాంకం rc;

		ఒకవేళ (loopcxt_sysfs_అందుబాటు(lc))
			rc = loopcxt_తదుపరి_from_sysfs(lc);
		లేకపోతే
			rc = loopcxt_తదుపరి_from_proc(lc);
		ఒకవేళ (rc == 0)
			తిరిగి 0;
		వెళ్ళు పూర్తి;
	}

	/* B) Classic way, try first eight loop devices (default number
	 *    of loop devices). This is enough for 99% of all cases.
	 */
	ఒకవేళ (iter->default_check) {
		DBG_OBJ(ITER, iter, ul_debug("next: default check"));
		కొరకు (++iter->ncur; iter->ncur < LOOPDEV_DEFAULT_NNODES;
							iter->ncur++) {
			అక్షరం name[16];
			snprintf(name, పరిమాణం(name), "loop%d", iter->ncur);

			ఒకవేళ (loopiter_సమితి_device(lc, name) == 0)
				తిరిగి 0;
		}
		iter->default_check = 0;
	}

	/* C) the worst possibility, scan whole /dev or /dev/loop/<N>
	 */
	ఒకవేళ (!iter->minors) {
		DBG_OBJ(ITER, iter, ul_debug("next: scanning /dev"));
		iter->nminors = (lc->flags & LOOPDEV_FL_DEVSUBDIR) ?
			loop_scandir(మార్గం_DEV_LOOP, &iter->minors, 0) :
			loop_scandir(_PATH_DEV, &iter->minors, 1);
		iter->ncur = -1;
	}
	కొరకు (++iter->ncur; iter->ncur < iter->nminors; iter->ncur++) {
		అక్షరం name[16];
		snprintf(name, పరిమాణం(name), "loop%d", iter->minors[iter->ncur]);

		ఒకవేళ (loopiter_సమితి_device(lc, name) == 0)
			తిరిగి 0;
	}
పూర్తి:
	loopcxt_deinit_iterator(lc);
	తిరిగి 1;
}

/*
 * @device: path to device
 */
పూర్ణాంకం is_loopdev(స్థిరరాశి అక్షరం *device)
{
	నిర్మాణం stat st;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (!device || stat(device, &st) != 0 || !S_ISBLK(st.st_mode))
		rc = 0;
	లేకపోతే ఒకవేళ (major(st.st_rdev) == LOOPDEV_MAJOR)
		rc = 1;
	లేకపోతే ఒకవేళ (sysfs_devno_is_wholedisk(st.st_rdev)) {
		/* It's possible that kernel creates a device with a different
		 * major number ... check by /sys it's really loop device.
		 */
		అక్షరం name[PATH_MAX], *cn, *p = ఖాళీ;

		snprintf(name, పరిమాణం(name), మార్గం_SYS_DEVBLOCK "/%u:%u",
				major(st.st_rdev), minor(st.st_rdev));
		cn = ul_canonicalize_మార్గం(name);
		ఒకవేళ (cn)
			p = stripoff_చివరి_component(cn);
		rc = p && ul_startswith(p, "loop");
		విడుదల(cn);
	}

	ఒకవేళ (rc == 0)
		errno = ENODEV;
	తిరిగి rc;
}

/*
 * @lc: context
 *
 * Returns result from LOOP_GET_STAT64 ioctl or NULL on error.
 */
నిర్మాణం loop_info64 *loopcxt_get_info(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం fd;

	ఒకవేళ (!lc || lc->info_failed) {
		errno = EINVAL;
		తిరిగి ఖాళీ;
	}
	errno = 0;
	ఒకవేళ (lc->has_info)
		తిరిగి &lc->config.info;

	fd = loopcxt_get_fd(lc);
	ఒకవేళ (fd < 0)
		తిరిగి ఖాళీ;

	ఒకవేళ (ioctl(fd, LOOP_GET_STATUS64, &lc->config.info) == 0) {
		lc->has_info = 1;
		lc->info_failed = 0;
		DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 OK"));
		తిరిగి &lc->config.info;
	}

	lc->info_failed = 1;
	DBG_OBJ(CXT, lc, ul_debug("reading loop_info64 FAILED"));

	తిరిగి ఖాళీ;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with path to the file associated
 * with the current loop device.
 */
అక్షరం *loopcxt_get_backing_file(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (sysfs)
		/*
		 * This is always preferred, the loop_info64
		 * has too small buffer for the filename.
		 */
		ul_మార్గం_read_string(sysfs, &res, "loop/backing_file");

	ఒకవేళ (!res && loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);

		ఒకవేళ (lo) {
			lo->lo_file_name[LO_NAME_కొలత - 2] = '*';
			lo->lo_file_name[LO_NAME_కొలత - 1] = '\0';
			res = strdup((అక్షరం *) lo->lo_file_name);
		}
	}

	DBG_OBJ(CXT, lc, ul_debug("get_backing_file [%s]", res));
	తిరిగి res;
}

/*
 * @lc: context
 *
 * Returns (allocated) string with loop reference. The same as backing file by
 * default.
 */
అక్షరం *loopcxt_get_refname(నిర్మాణం loopdev_cxt *lc)
{
	అక్షరం *res = ఖాళీ;
	నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);

	ఒకవేళ (lo) {
		lo->lo_file_name[LO_NAME_కొలత - 1] = '\0';
		res = strdup((అక్షరం *) lo->lo_file_name);
	}

	DBG_OBJ(CXT, lc, ul_debug("get_refname [%s]", res));
	తిరిగి res;
}

/*
 * @lc: context
 * @offset: returns offset number for the given device
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_స్థానభ్రంశం(నిర్మాణం loopdev_cxt *lc, uint64_t *స్థానభ్రంశం)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);
	పూర్ణాంకం rc = -EINVAL;

	ఒకవేళ (sysfs)
		ఒకవేళ (ul_మార్గం_read_u64(sysfs, స్థానభ్రంశం, "loop/offset") == 0)
			rc = 0;

	ఒకవేళ (rc && loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
		ఒకవేళ (lo) {
			ఒకవేళ (స్థానభ్రంశం)
				*స్థానభ్రంశం = lo->lo_స్థానభ్రంశం;
			rc = 0;
		} లేకపోతే
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_offset [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * @lc: context
 * @blocksize: returns logical blocksize for the given device
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_blocksize(నిర్మాణం loopdev_cxt *lc, uint64_t *blocksize)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);
	పూర్ణాంకం rc = -EINVAL;

	ఒకవేళ (sysfs)
		ఒకవేళ (ul_మార్గం_read_u64(sysfs, blocksize, "queue/logical_block_size") == 0)
			rc = 0;

	/* Fallback based on BLKSSZGET ioctl */
	ఒకవేళ (rc) {
		పూర్ణాంకం fd = loopcxt_get_fd(lc);
		పూర్ణాంకం sz = 0;

		ఒకవేళ (fd < 0)
			తిరిగి -EINVAL;
		rc = blkdev_get_sector_కొలత(fd, &sz);
		ఒకవేళ (rc)
			తిరిగి rc;

		*blocksize = sz;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_blocksize [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * @lc: context
 * @sizelimit: returns size limit for the given device
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_sizelimit(నిర్మాణం loopdev_cxt *lc, uint64_t *కొలత)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);
	పూర్ణాంకం rc = -EINVAL;

	ఒకవేళ (sysfs)
		ఒకవేళ (ul_మార్గం_read_u64(sysfs, కొలత, "loop/sizelimit") == 0)
			rc = 0;

	ఒకవేళ (rc && loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
		ఒకవేళ (lo) {
			ఒకవేళ (కొలత)
				*కొలత = lo->lo_sizelimit;
			rc = 0;
		} లేకపోతే
			rc = -errno;
	}

	DBG_OBJ(CXT, lc, ul_debug("get_sizelimit [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * @lc: context
 * @type: returns encryption type
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_encrypt_type(నిర్మాణం loopdev_cxt *lc, uint32_t *type)
{
	నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
	పూర్ణాంకం rc;

	/* not provided by sysfs */
	ఒకవేళ (lo) {
		ఒకవేళ (type)
			*type = lo->lo_encrypt_type;
		rc = 0;
	} లేకపోతే
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_encrypt_type [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * @lc: context
 *
 * Cryptoloop is DEPRECATED!
 *
 * Returns: <0 on error, 0 on success
 */
స్థిరరాశి అక్షరం *loopcxt_get_crypt_name(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);

	ఒకవేళ (lo)
		తిరిగి (అక్షరం *) lo->lo_crypt_name;

	DBG_OBJ(CXT, lc, ul_debug("get_crypt_name failed"));
	తిరిగి ఖాళీ;
}

/*
 * @lc: context
 * @devno: returns backing file devno
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_backing_devno(నిర్మాణం loopdev_cxt *lc, dev_t *devno)
{
	నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
	పూర్ణాంకం rc;

	ఒకవేళ (lo) {
		ఒకవేళ (devno)
			*devno = lo->lo_device;
		rc = 0;
	} లేకపోతే
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_devno [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * @lc: context
 * @ino: returns backing file inode
 *
 * Returns: <0 on error, 0 on success
 */
పూర్ణాంకం loopcxt_get_backing_inode(నిర్మాణం loopdev_cxt *lc, ino_t *ino)
{
	నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
	పూర్ణాంకం rc;

	ఒకవేళ (lo) {
		ఒకవేళ (ino)
			*ino = lo->lo_inode;
		rc = 0;
	} లేకపోతే
		rc = -errno;

	DBG_OBJ(CXT, lc, ul_debug("get_backing_inode [rc=%d]", rc));
	తిరిగి rc;
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
పూర్ణాంకం loopmod_supports_partscan(శూన్య)
{
	పూర్ణాంకం rc, ret = 0;
	FILE *f;

	ఒకవేళ (get_linux_సంస్కరణ() >= KERNEL_సంస్కరణ(3,2,0))
		తిరిగి 1;

	f = fopen("/sys/module/loop/parameters/max_part", "r" UL_CLOEXECSTR);
	ఒకవేళ (!f)
		తిరిగి 0;
	rc = fscanf(f, "%d", &ret);
	fclose(f);
	తిరిగి rc == 1 ? ret : 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the partscan flags is set *or* (for old kernels) partitions
 * scanning is enabled for all loop devices.
 */
పూర్ణాంకం loopcxt_is_partscan(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);

	ఒకవేళ (sysfs) {
		/* kernel >= 3.2 */
		పూర్ణాంకం fl;
		ఒకవేళ (ul_మార్గం_read_s32(sysfs, &fl, "loop/partscan") == 0)
			తిరిగి fl;
	}

	/* old kernels (including kernels without loopN/loop/<flags> directory */
	తిరిగి loopmod_supports_partscan();
}

/*
 * @lc: context
 *
 * Returns: 1 if the autoclear flags is set.
 */
పూర్ణాంకం loopcxt_is_autoclear(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);

	ఒకవేళ (sysfs) {
		పూర్ణాంకం fl;
		ఒకవేళ (ul_మార్గం_read_s32(sysfs, &fl, "loop/autoclear") == 0)
			తిరిగి fl;
	}

	ఒకవేళ (loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
		ఒకవేళ (lo)
			తిరిగి lo->lo_flags & LO_FLAGS_AUTOCLEAR;
	}
	తిరిగి 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the readonly flags is set.
 */
పూర్ణాంకం loopcxt_is_readonly(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);

	ఒకవేళ (sysfs) {
		పూర్ణాంకం fl;
		ఒకవేళ (ul_మార్గం_read_s32(sysfs, &fl, "ro") == 0)
			తిరిగి fl;
	}

	ఒకవేళ (loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
		ఒకవేళ (lo)
			తిరిగి lo->lo_flags & LO_FLAGS_READ_ONLY;
	}
	తిరిగి 0;
}

/*
 * @lc: context
 *
 * Returns: 1 if the dio flags is set.
 */
పూర్ణాంకం loopcxt_is_dio(నిర్మాణం loopdev_cxt *lc)
{
	నిర్మాణం మార్గం_cxt *sysfs = loopcxt_get_sysfs(lc);

	ఒకవేళ (sysfs) {
		పూర్ణాంకం fl;
		ఒకవేళ (ul_మార్గం_read_s32(sysfs, &fl, "loop/dio") == 0)
			తిరిగి fl;
	}
	ఒకవేళ (loopcxt_ioctl_enabled(lc)) {
		నిర్మాణం loop_info64 *lo = loopcxt_get_info(lc);
		ఒకవేళ (lo)
			తిరిగి lo->lo_flags & LO_FLAGS_ప్రత్యక్ష_IO;
	}
	తిరిగి 0;
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
పూర్ణాంకం loopcxt_is_వాడిన(నిర్మాణం loopdev_cxt *lc,
		    నిర్మాణం stat *st,
		    స్థిరరాశి అక్షరం *backing_file,
		    uint64_t స్థానభ్రంశం,
		    uint64_t sizelimit,
		    పూర్ణాంకం flags)
{
	ino_t ino = 0;
	dev_t dev = 0;

	ఒకవేళ (!lc)
		తిరిగి 0;

	DBG_OBJ(CXT, lc, ul_debug("checking %s vs. %s",
				loopcxt_get_device(lc),
				backing_file));

	ఒకవేళ (st && loopcxt_get_backing_inode(lc, &ino) == 0 &&
		  loopcxt_get_backing_devno(lc, &dev) == 0) {

		ఒకవేళ (ino == st->st_ino && dev == st->st_dev)
			వెళ్ళు దొరికింది;

		/* don't use filename if we have devno and inode */
		తిరిగి 0;
	}

	/* poor man's solution */
	ఒకవేళ (backing_file) {
		అక్షరం *name = loopcxt_get_backing_file(lc);
		పూర్ణాంకం rc = name && strcmp(name, backing_file) == 0;

		విడుదల(name);
		ఒకవేళ (rc)
			వెళ్ళు దొరికింది;
	}

	తిరిగి 0;
దొరికింది:
	ఒకవేళ (flags & LOOPDEV_FL_స్థానభ్రంశం) {
		uint64_t off = 0;

		పూర్ణాంకం rc = loopcxt_get_స్థానభ్రంశం(lc, &off) == 0 && off == స్థానభ్రంశం;

		ఒకవేళ (rc && flags & LOOPDEV_FL_SIZELIMIT) {
			uint64_t sz = 0;

			తిరిగి loopcxt_get_sizelimit(lc, &sz) == 0 && sz == sizelimit;
		}
		తిరిగి rc;
	}
	తిరిగి 1;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
పూర్ణాంకం loopcxt_సమితి_స్థానభ్రంశం(నిర్మాణం loopdev_cxt *lc, uint64_t స్థానభ్రంశం)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;
	lc->config.info.lo_స్థానభ్రంశం = స్థానభ్రంశం;

	DBG_OBJ(CXT, lc, ul_debug("set offset=%"PRIu64, స్థానభ్రంశం));
	తిరిగి 0;
}

/*
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
పూర్ణాంకం loopcxt_సమితి_sizelimit(నిర్మాణం loopdev_cxt *lc, uint64_t sizelimit)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;
	lc->config.info.lo_sizelimit = sizelimit;

	DBG_OBJ(CXT, lc, ul_debug("set sizelimit=%"PRIu64, sizelimit));
	తిరిగి 0;
}

/*
 * The blocksize will be used by loopcxt_set_device(). For already exiting
 * devices use  loopcxt_ioctl_blocksize().
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 */
పూర్ణాంకం loopcxt_సమితి_blocksize(నిర్మాణం loopdev_cxt *lc, uint64_t blocksize)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;
	lc->blocksize = blocksize;

	DBG_OBJ(CXT, lc, ul_debug("set blocksize=%"PRIu64, blocksize));
	తిరిగి 0;
}

/*
 * @lc: context
 * @flags: kernel LO_FLAGS_{READ_ONLY,USE_AOPS,AUTOCLEAR} flags
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
పూర్ణాంకం loopcxt_సమితి_flags(నిర్మాణం loopdev_cxt *lc, uint32_t flags)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;
	lc->config.info.lo_flags = flags;

	DBG_OBJ(CXT, lc, ul_debug("set flags=%u", (గుర్తులేని) flags));
	తిరిగి 0;
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
పూర్ణాంకం loopcxt_సమితి_refname(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *refname)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;

	memset(lc->config.info.lo_file_name, 0, పరిమాణం(lc->config.info.lo_file_name));
	ఒకవేళ (refname)
		xstrncpy((అక్షరం *)lc->config.info.lo_file_name, refname, LO_NAME_కొలత);

	DBG_OBJ(CXT, lc, ul_debug("set refname=%s", (అక్షరం *)lc->config.info.lo_file_name));
	తిరిగి 0;
}

/*
 * @lc: context
 * @filename: backing file path (the path will be canonicalized)
 *
 * The setting is removed by loopcxt_set_device() loopcxt_next()!
 *
 * Returns: 0 on success, <0 on error.
 */
పూర్ణాంకం loopcxt_సమితి_backing_file(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *filename)
{
	ఒకవేళ (!lc)
		తిరిగి -EINVAL;

	ఒకవేళ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		lc->filename = strdup(filename);
	లేకపోతే
		lc->filename = ul_canonicalize_మార్గం(filename);
	ఒకవేళ (!lc->filename)
		తిరిగి -errno;

	ఒకవేళ (!lc->config.info.lo_file_name[0])
		loopcxt_సమితి_refname(lc, lc->filename);

	DBG_OBJ(CXT, lc, ul_debug("set backing file=%s", lc->filename));
	తిరిగి 0;
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
స్థిర పూర్ణాంకం loopcxt_check_కొలత(నిర్మాణం loopdev_cxt *lc, పూర్ణాంకం file_fd)
{
	uint64_t కొలత, expected_కొలత;
	పూర్ణాంకం dev_fd;
	నిర్మాణం stat st;

	ఒకవేళ (!lc->config.info.lo_స్థానభ్రంశం && !lc->config.info.lo_sizelimit)
		తిరిగి 0;

	ఒకవేళ (fstat(file_fd, &st)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to fstat backing file"));
		తిరిగి -errno;
	}
	ఒకవేళ (S_ISBLK(st.st_mode)) {
		ఒకవేళ (blkdev_get_కొలత(file_fd, &expected_కొలత)) {
			DBG_OBJ(CXT, lc, ul_debug("failed to determine device size"));
			తిరిగి -errno;
		}
	} లేకపోతే
		expected_కొలత = st.st_size;

	ఒకవేళ (expected_కొలత == 0 || expected_కొలత <= lc->config.info.lo_స్థానభ్రంశం) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine expected size"));
		తిరిగి 0;	/* ignore this error */
	}

	ఒకవేళ (lc->config.info.lo_స్థానభ్రంశం > 0)
		expected_కొలత -= lc->config.info.lo_స్థానభ్రంశం;

	ఒకవేళ (lc->config.info.lo_sizelimit > 0 && lc->config.info.lo_sizelimit < expected_కొలత)
		expected_కొలత = lc->config.info.lo_sizelimit;

	dev_fd = loopcxt_get_fd(lc);
	ఒకవేళ (dev_fd < 0) {
		DBG_OBJ(CXT, lc, ul_debug("failed to get loop FD"));
		తిరిగి -errno;
	}

	ఒకవేళ (blkdev_get_కొలత(dev_fd, &కొలత)) {
		DBG_OBJ(CXT, lc, ul_debug("failed to determine loopdev size"));
		తిరిగి -errno;
	}

	/* It's block device, so, align to 512-byte sectors */
	ఒకవేళ (expected_కొలత % 512) {
		DBG_OBJ(CXT, lc, ul_debug("expected size misaligned to 512-byte sectors"));
		expected_కొలత = (expected_కొలత >> 9) << 9;
	}

	ఒకవేళ (expected_కొలత != కొలత) {
		DBG_OBJ(CXT, lc, ul_debug("warning: loopdev and expected "
				      "size mismatch (%ju/%ju)",
				      కొలత, expected_కొలత));

		ఒకవేళ (loopcxt_ioctl_సామర్థ్యం(lc)) {
			/* ioctl not available */
			ఒకవేళ (errno == ENOTTY || errno == EINVAL)
				errno = ERANGE;
			తిరిగి -errno;
		}

		ఒకవేళ (blkdev_get_కొలత(dev_fd, &కొలత))
			తిరిగి -errno;

		ఒకవేళ (expected_కొలత != కొలత) {
			errno = ERANGE;
			DBG_OBJ(CXT, lc, ul_debug("failed to set loopdev size, "
					"size: %ju, expected: %ju",
					కొలత, expected_కొలత));
			తిరిగి -errno;
		}
	}

	తిరిగి 0;
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
పూర్ణాంకం loopcxt_అమరిక_device(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం file_fd, dev_fd;
	mode_t flags = O_CLOEXEC, బహుళకం = O_RDWR;
	పూర్ణాంకం rc = -1, cnt = 0;
	పూర్ణాంకం errsv = 0;
	పూర్ణాంకం fallback = 0;

	ఒకవేళ (!lc || !*lc->device || !lc->filename)
		తిరిగి -EINVAL;

	DBG_OBJ(అమరిక, lc, ul_debug("device setup requested"));

	/*
	 * Open backing file and device
	 */
	ఒకవేళ (lc->config.info.lo_flags & LO_FLAGS_READ_ONLY)
		బహుళకం = O_RDONLY;

	ఒకవేళ (lc->config.info.lo_flags & LO_FLAGS_ప్రత్యక్ష_IO)
		flags |= O_ప్రత్యక్ష;
	ఒకవేళ (lc->flags & LOOPDEV_FL_NOFOLLOW)
		file_fd = ul_open_no_symlinks(lc->filename, బహుళకం | flags, 0);
	లేకపోతే
		file_fd = open(lc->filename, బహుళకం | flags);

	ఒకవేళ (file_fd < 0 && బహుళకం != O_RDONLY
	    && (errno == EROFS || errno == EACCES)) {
		బహుళకం = O_RDONLY;
		ఒకవేళ (lc->flags & LOOPDEV_FL_NOFOLLOW)
			file_fd = ul_open_no_symlinks(lc->filename, బహుళకం | flags, 0);
		లేకపోతే
			file_fd = open(lc->filename, బహుళకం | flags);
	}
	ఒకవేళ (file_fd < 0) {
		DBG_OBJ(అమరిక, lc, ul_debug("open backing file failed: %m"));
		తిరిగి -errno;
	}
	DBG_OBJ(అమరిక, lc, ul_debug("backing file open: OK"));

	ఒకవేళ (బహుళకం == O_RDONLY)
		lc->config.info.lo_flags |= LO_FLAGS_READ_ONLY;	/* kernel loopdev mode */
	లేకపోతే
		lc->config.info.lo_flags &= ~LO_FLAGS_READ_ONLY;

	చేయి {
		errno = 0;

		dev_fd = __loopcxt_get_fd(lc, బహుళకం);
		ఒకవేళ (dev_fd >= 0 || lc->control_ok == 0)
			ఆపు;
		ఒకవేళ (errno != EACCES && errno != ENOENT)
			ఆపు;
		/* We have permissions to open /dev/loop-control, but open
		 * /dev/loopN failed with EACCES, it's probably because udevd
		 * does not applied chown yet. Let's wait a moment. */
		xusleep(25000);
	} అయితే (cnt++ < 16);

	ఒకవేళ (dev_fd < 0) {
		rc = -errno;
		వెళ్ళు err;
	}

	DBG_OBJ(అమరిక, lc, ul_debug("device open: OK"));

	/*
	 * Atomic way to configure all by one ioctl call
	 * -- since Linux v5.8-rc1, commit 3448914e8cc550ba792d4ccc74471d1ca4293aae
	 */
	lc->config.fd = file_fd;
	ఒకవేళ (lc->blocksize > 0)
		lc->config.ఖండం_కొలత = lc->blocksize;

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_CONFIGURE, &lc->config) );
	ఒకవేళ (rc != 0) {
		errsv = errno;
		ఒకవేళ (errno != EINVAL && errno != ENOTTY && errno != ENOSYS) {
			DBG_OBJ(అమరిక, lc, ul_debug("LOOP_CONFIGURE failed: %m"));
			వెళ్ళు err;
		}
		fallback = 1;
	} లేకపోతే {
		DBG_OBJ(అమరిక, lc, ul_debug("LOOP_CONFIGURE: OK"));
	}

	/*
	 * Old deprecated way; first assign backing file FD and then in the
	 * second step set loop device properties.
	 */
	ఒకవేళ (fallback) {
		ఒకవేళ (ioctl(dev_fd, LOOP_సమితి_FD, file_fd) < 0) {
			rc = -errno;
			errsv = errno;
			DBG_OBJ(అమరిక, lc, ul_debug("LOOP_SET_FD failed: %m"));
			వెళ్ళు err;
		}

		DBG_OBJ(అమరిక, lc, ul_debug("LOOP_SET_FD: OK"));

		ఒకవేళ (lc->blocksize > 0
			&& (rc = loopcxt_ioctl_blocksize(lc, lc->blocksize)) < 0) {
			errsv = -rc;
			వెళ్ళు err;
		}

		ఒకవేళ ((rc = loopcxt_ioctl_స్థితి(lc)) < 0) {
			errsv = -rc;
			వెళ్ళు err;
		}
	}

	ఒకవేళ ((rc = loopcxt_check_కొలత(lc, file_fd)))
		వెళ్ళు err;

	close(file_fd);

	memset(&lc->config, 0, పరిమాణం(lc->config));
	lc->has_info = 0;
	lc->info_failed = 0;

	DBG_OBJ(అమరిక, lc, ul_debug("success [rc=0]"));
	తిరిగి 0;
err:
	ఒకవేళ (file_fd >= 0)
		close(file_fd);
	ఒకవేళ (dev_fd >= 0 && rc != -EBUSY)
		ioctl(dev_fd, LOOP_CLR_FD, 0);
	ఒకవేళ (errsv)
		errno = errsv;

	DBG_OBJ(అమరిక, lc, ul_debug("failed [rc=%d]", rc));
	తిరిగి rc;
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
పూర్ణాంకం loopcxt_ioctl_స్థితి(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం dev_fd, rc;

	errno = 0;
	dev_fd = loopcxt_get_fd(lc);

	ఒకవేళ (dev_fd < 0)
		తిరిగి -errno;

	DBG_OBJ(అమరిక, lc, ul_debug("calling LOOP_SET_STATUS64"));

	rc = repeat_on_eagain( ioctl(dev_fd, LOOP_సమితి_STATUS64, &lc->config.info) );
	ఒకవేళ (rc != 0) {
		DBG_OBJ(అమరిక, lc, ul_debug("LOOP_SET_STATUS64 failed: %m"));
		తిరిగి rc;
	}

	DBG_OBJ(అమరిక, lc, ul_debug("LOOP_SET_STATUS64: OK"));
	తిరిగి 0;
}

పూర్ణాంకం loopcxt_ioctl_సామర్థ్యం(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం rc, fd = loopcxt_get_fd(lc);

	ఒకవేళ (fd < 0)
		తిరిగి -EINVAL;

	DBG_OBJ(అమరిక, lc, ul_debug("calling LOOP_SET_CAPACITY"));

	/* Kernels prior to v2.6.30 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_సమితి_సామర్థ్యం, 0) );
	ఒకవేళ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_CAPACITY failed: %m"));
		తిరిగి rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("capacity set"));
	తిరిగి 0;
}

పూర్ణాంకం loopcxt_ioctl_dio(నిర్మాణం loopdev_cxt *lc, గుర్తులేని దీర్ఘ use_dio)
{
	పూర్ణాంకం rc, fd = loopcxt_get_fd(lc);

	ఒకవేళ (fd < 0)
		తిరిగి -EINVAL;

	DBG_OBJ(అమరిక, lc, ul_debug("calling LOOP_SET_DIRECT_IO"));

	/* Kernels prior to v4.4 don't support this ioctl */
	rc = repeat_on_eagain( ioctl(fd, LOOP_సమితి_ప్రత్యక్ష_IO, use_dio) );
	ఒకవేళ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_DIRECT_IO failed: %m"));
		తిరిగి rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("direct io set"));
	తిరిగి 0;
}

/*
 * Kernel uses "unsigned long" as ioctl arg, but we use u64 for all sizes to
 * keep loopdev internal API simple.
 */
పూర్ణాంకం loopcxt_ioctl_blocksize(నిర్మాణం loopdev_cxt *lc, uint64_t blocksize)
{
	పూర్ణాంకం rc, fd = loopcxt_get_fd(lc);

	ఒకవేళ (fd < 0)
		తిరిగి -EINVAL;

	DBG_OBJ(అమరిక, lc, ul_debug("calling LOOP_SET_BLOCK_SIZE"));

	rc = repeat_on_eagain(
		ioctl(fd, LOOP_సమితి_ఖండం_కొలత, (గుర్తులేని దీర్ఘ) blocksize) );
	ఒకవేళ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_SET_BLOCK_SIZE failed: %m"));
		తిరిగి rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("logical block size set"));
	తిరిగి 0;
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
స్థిర పూర్ణాంకం loopcxt_get_device_nr(నిర్మాణం loopdev_cxt *lc, పూర్ణాంకం *nr)
{
	స్థిరరాశి అక్షరం *p, *dev;
	పూర్ణాంకం rc = -EINVAL;

	errno = 0;
	ఒకవేళ (!lc || !nr)
		తిరిగి rc;
	*nr = -1;

	dev = loopcxt_get_device(lc);
	ఒకవేళ (!dev)
		వెళ్ళు పూర్తి;

	p = strrchr(dev, '/');
	ఒకవేళ (!p)
		వెళ్ళు పూర్తి;

	ఒకవేళ (అక్షరశ్రేణి_చదువు(p, "/loop%d", nr) != 1 && అక్షరశ్రేణి_చదువు(p, "/%d", nr) != 1)
		వెళ్ళు పూర్తి;

	ఒకవేళ (*nr < 0)
		వెళ్ళు పూర్తి;
	rc = 0;
పూర్తి:
	ఒకవేళ (rc && !errno)
		errno = -rc;
	DBG_OBJ(CXT, lc, ul_debug("get_device_nr [nr=%d]", *nr));
	తిరిగి rc;
}

పూర్ణాంకం loopcxt_detach_device(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం rc, fd = loopcxt_get_fd(lc);

	ఒకవేళ (fd < 0)
		తిరిగి -EINVAL;

	DBG_OBJ(అమరిక, lc, ul_debug("calling LOOP_CLR_FD"));

	rc = repeat_on_eagain( ioctl(fd, LOOP_CLR_FD, 0) );
	ఒకవేళ (rc != 0) {
		DBG_OBJ(CXT, lc, ul_debug("LOOP_CLR_FD failed: %m"));
		తిరిగి rc;
	}

	DBG_OBJ(CXT, lc, ul_debug("device removed"));
	తిరిగి 0;
}

పూర్ణాంకం loopcxt_remove_device(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం rc = -EINVAL;
	పూర్ణాంకం ctl, nr = -1;

	ఒకవేళ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		వెళ్ళు పూర్తి;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ఒకవేళ (rc)
		వెళ్ళు పూర్తి;

	ctl = open(మార్గం_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ఒకవేళ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("remove_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_REMOVE, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
పూర్తి:
	DBG_OBJ(CXT, lc, ul_debug("remove_device done [rc=%d]", rc));
	తిరిగి rc;
}

పూర్ణాంకం loopcxt_కూడు_device(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం rc = -EINVAL;
	పూర్ణాంకం ctl, nr = -1;

	ఒకవేళ (!(lc->flags & LOOPDEV_FL_CONTROL)) {
		rc = -ENOSYS;
		వెళ్ళు పూర్తి;
	}

	rc = loopcxt_get_device_nr(lc, &nr);
	ఒకవేళ (rc)
		వెళ్ళు పూర్తి;

	ctl = open(మార్గం_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
	ఒకవేళ (ctl >= 0) {
		DBG_OBJ(CXT, lc, ul_debug("add_device %d", nr));
		rc = ioctl(ctl, LOOP_CTL_కూడు, nr);
		close(ctl);
	}
	lc->control_ok = rc >= 0 ? 1 : 0;
పూర్తి:
	DBG_OBJ(CXT, lc, ul_debug("add_device done [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * Note that LOOP_CTL_GET_FREE ioctl is supported since kernel 3.1. In older
 * kernels we have to check all loop devices to found unused one.
 *
 * See kernel commit 770fe30a46a12b6fb6b63fbe1737654d28e8484.
 *
 * Returns: 0 = success, < 0 error
 */
పూర్ణాంకం loopcxt_కనుగొను_వాడని(నిర్మాణం loopdev_cxt *lc)
{
	పూర్ణాంకం rc = -1;

	DBG_OBJ(CXT, lc, ul_debug("find_unused requested"));

	ఒకవేళ (lc->flags & LOOPDEV_FL_CONTROL) {
		పూర్ణాంకం ctl;

		DBG_OBJ(CXT, lc, ul_debug("using loop-control"));

		ctl = open(మార్గం_DEV_LOOPCTL, O_RDWR|O_CLOEXEC);
		ఒకవేళ (ctl >= 0)
			rc = ioctl(ctl, LOOP_CTL_GET_FREE);
		లేకపోతే
			rc = -errno;
		ఒకవేళ (rc >= 0) {
			అక్షరం name[16];
			snprintf(name, పరిమాణం(name), "loop%d", rc);

			rc = loopiter_సమితి_device(lc, name);
		}
		lc->control_ok = ctl >= 0 && rc == 0 ? 1 : 0;
		ఒకవేళ (ctl >= 0)
			close(ctl);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by loop-control [rc=%d]", rc));
	}

	ఒకవేళ (rc < 0 && rc != -EACCES) {
		DBG_OBJ(CXT, lc, ul_debug("using loop scan"));
		rc = loopcxt_init_iterator(lc, LOOPITER_FL_FREE);
		ఒకవేళ (rc)
			తిరిగి rc;

		rc = loopcxt_తదుపరి(lc);
		loopcxt_deinit_iterator(lc);
		DBG_OBJ(CXT, lc, ul_debug("find_unused by scan [rc=%d]", rc));
		ఒకవేళ (rc)
			తిరిగి -ENOENT;
	}
	తిరిగి rc;
}



/*
 * Return: TRUE/FALSE
 */
పూర్ణాంకం loopdev_is_autoclear(స్థిరరాశి అక్షరం *device)
{
	నిర్మాణం loopdev_cxt lc;
	పూర్ణాంకం rc;

	ఒకవేళ (!device)
		తిరిగి 0;

	rc = loopcxt_init(&lc, 0);
	ఒకవేళ (!rc)
		rc = loopcxt_సమితి_device(&lc, device);
	ఒకవేళ (!rc)
		rc = loopcxt_is_autoclear(&lc);

	loopcxt_deinit(&lc);
	తిరిగి rc;
}

అక్షరం *loopdev_get_backing_file(స్థిరరాశి అక్షరం *device)
{
	నిర్మాణం loopdev_cxt lc;
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (!device)
		తిరిగి ఖాళీ;
	ఒకవేళ (loopcxt_init(&lc, 0))
		తిరిగి ఖాళీ;
	ఒకవేళ (loopcxt_సమితి_device(&lc, device) == 0)
		res = loopcxt_get_backing_file(&lc);

	loopcxt_deinit(&lc);
	తిరిగి res;
}

/*
 * Returns: TRUE/FALSE
 */
పూర్ణాంకం loopdev_has_backing_file(స్థిరరాశి అక్షరం *device)
{
	అక్షరం *tmp = loopdev_get_backing_file(device);

	ఒకవేళ (tmp) {
		విడుదల(tmp);
		తిరిగి 1;
	}
	తిరిగి 0;
}

/*
 * Returns: TRUE/FALSE
 */
పూర్ణాంకం loopdev_is_వాడిన(స్థిరరాశి అక్షరం *device, స్థిరరాశి అక్షరం *filename,
		    uint64_t స్థానభ్రంశం, uint64_t sizelimit, పూర్ణాంకం flags)
{
	నిర్మాణం loopdev_cxt lc;
	నిర్మాణం stat st;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (!device || !filename)
		తిరిగి 0;

	rc = loopcxt_init(&lc, 0);
	ఒకవేళ (!rc)
		rc = loopcxt_సమితి_device(&lc, device);
	ఒకవేళ (rc)
		తిరిగి rc;

	rc = !stat(filename, &st);
	rc = loopcxt_is_వాడిన(&lc, rc ? &st : ఖాళీ, filename, స్థానభ్రంశం, sizelimit, flags);

	loopcxt_deinit(&lc);
	తిరిగి rc;
}

/*
 * Returns: 0 = success, < 0 error
 */
పూర్ణాంకం loopdev_detach(స్థిరరాశి అక్షరం *device)
{
	నిర్మాణం loopdev_cxt lc;
	పూర్ణాంకం rc;

	ఒకవేళ (!device)
		తిరిగి -EINVAL;

	rc = loopcxt_init(&lc, 0);
	ఒకవేళ (!rc)
		rc = loopcxt_సమితి_device(&lc, device);
	ఒకవేళ (!rc)
		rc = loopcxt_detach_device(&lc);
	loopcxt_deinit(&lc);
	తిరిగి rc;
}

/*
 * Returns: 0 = success, < 0 error, 1 not found
 */
పూర్ణాంకం loopcxt_కనుగొను_by_backing_file(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *filename,
				 uint64_t స్థానభ్రంశం, uint64_t sizelimit, పూర్ణాంకం flags)
{
	పూర్ణాంకం rc, hasst;
	నిర్మాణం stat st;

	ఒకవేళ (!filename)
		తిరిగి -EINVAL;

	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_వాడిన);
	ఒకవేళ (rc)
		తిరిగి rc;

	అయితే ((rc = loopcxt_తదుపరి(lc)) == 0) {

		ఒకవేళ (loopcxt_is_వాడిన(lc, hasst ? &st : ఖాళీ,
				    filename, స్థానభ్రంశం, sizelimit, flags))
			ఆపు;
	}

	loopcxt_deinit_iterator(lc);
	తిరిగి rc;
}

/*
 * Returns: 0 = not found, < 0 error, 1 found, 2 found full size and offset match
 */
పూర్ణాంకం loopcxt_కనుగొను_overlap(నిర్మాణం loopdev_cxt *lc, స్థిరరాశి అక్షరం *filename,
			   uint64_t స్థానభ్రంశం, uint64_t sizelimit)
{
	పూర్ణాంకం rc, hasst;
	నిర్మాణం stat st;

	ఒకవేళ (!filename)
		తిరిగి -EINVAL;

	DBG_OBJ(CXT, lc, ul_debug("find_overlap requested"));
	hasst = !stat(filename, &st);

	rc = loopcxt_init_iterator(lc, LOOPITER_FL_వాడిన);
	ఒకవేళ (rc)
		తిరిగి rc;

	అయితే ((rc = loopcxt_తదుపరి(lc)) == 0) {
		uint64_t lc_sizelimit, lc_స్థానభ్రంశం;

		rc = loopcxt_is_వాడిన(lc, hasst ? &st : ఖాళీ,
				     filename, స్థానభ్రంశం, sizelimit, 0);
		/*
		 * Either the loopdev is unused or we've got an error which can
		 * happen when we are racing with device autoclear. Just ignore
		 * this loopdev...
		 */
		ఒకవేళ (rc <= 0)
			కొనసాగు;

		DBG_OBJ(CXT, lc, ul_debug("found %s backed by %s",
			loopcxt_get_device(lc), filename));

		rc = loopcxt_get_స్థానభ్రంశం(lc, &lc_స్థానభ్రంశం);
		ఒకవేళ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get offset for device %s",
				loopcxt_get_device(lc)));
			కొనసాగు;
		}
		rc = loopcxt_get_sizelimit(lc, &lc_sizelimit);
		ఒకవేళ (rc) {
			DBG_OBJ(CXT, lc, ul_debug("failed to get sizelimit for device %s",
				loopcxt_get_device(lc)));
			కొనసాగు;
		}

		/* full match */
		ఒకవేళ (lc_sizelimit == sizelimit && lc_స్థానభ్రంశం == స్థానభ్రంశం) {
			DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s (full match)",
						loopcxt_get_device(lc)));
			rc = 2;
			వెళ్ళు దొరికింది;
		}

		/* overlap */
		ఒకవేళ (lc_sizelimit != 0 && స్థానభ్రంశం >= lc_స్థానభ్రంశం + lc_sizelimit)
			కొనసాగు;
		ఒకవేళ (sizelimit != 0 && స్థానభ్రంశం + sizelimit <= lc_స్థానభ్రంశం)
			కొనసాగు;

		DBG_OBJ(CXT, lc, ul_debug("overlapping loop device %s",
			loopcxt_get_device(lc)));
			rc = 1;
			వెళ్ళు దొరికింది;
	}

	ఒకవేళ (rc == 1)
		rc = 0;	/* not found */
దొరికింది:
	loopcxt_deinit_iterator(lc);
	DBG_OBJ(CXT, lc, ul_debug("find_overlap done [rc=%d]", rc));
	తిరిగి rc;
}

/*
 * Returns allocated string with device name
 */
అక్షరం *loopdev_కనుగొను_by_backing_file(స్థిరరాశి అక్షరం *filename, uint64_t స్థానభ్రంశం, uint64_t sizelimit, పూర్ణాంకం flags)
{
	నిర్మాణం loopdev_cxt lc;
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (!filename)
		తిరిగి ఖాళీ;

	ఒకవేళ (loopcxt_init(&lc, 0))
		తిరిగి ఖాళీ;
	ఒకవేళ (loopcxt_కనుగొను_by_backing_file(&lc, filename, స్థానభ్రంశం, sizelimit, flags) == 0)
		res = loopcxt_strdup_device(&lc);
	loopcxt_deinit(&lc);

	తిరిగి res;
}

/*
 * Returns number of loop devices associated with @file, if only one loop
 * device is associated with the given @filename and @loopdev is not NULL then
 * @loopdev returns name of the device.
 */
పూర్ణాంకం loopdev_లెక్క_by_backing_file(స్థిరరాశి అక్షరం *filename, అక్షరం **loopdev)
{
	నిర్మాణం loopdev_cxt lc;
	పూర్ణాంకం లెక్క = 0, rc;

	ఒకవేళ (!filename)
		తిరిగి -1;

	rc = loopcxt_init(&lc, 0);
	ఒకవేళ (rc)
		తిరిగి rc;
	ఒకవేళ (loopcxt_init_iterator(&lc, LOOPITER_FL_వాడిన))
		తిరిగి -1;

	అయితే(loopcxt_తదుపరి(&lc) == 0) {
		అక్షరం *backing = loopcxt_get_backing_file(&lc);

		ఒకవేళ (!backing || strcmp(backing, filename) != 0) {
			విడుదల(backing);
			కొనసాగు;
		}

		విడుదల(backing);
		ఒకవేళ (loopdev && లెక్క == 0)
			*loopdev = loopcxt_strdup_device(&lc);
		లెక్క++;
	}

	loopcxt_deinit(&lc);

	ఒకవేళ (loopdev && లెక్క > 1) {
		విడుదల(*loopdev);
		*loopdev = ఖాళీ;
	}
	తిరిగి లెక్క;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_LOOPDEV
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	ఒకవేళ (argc < 2)
		వెళ్ళు usage;

	ఒకవేళ (strcmp(argv[1], "--is-loopdev") == 0 && argc == 3)
		రాయి("%s: %s\n", argv[2], is_loopdev(argv[2]) ? "OK" : "FAIL");
	లేకపోతే
		వెళ్ళు usage;

	తిరిగి EXIT_SUCCESS;
usage:
	fprintf(stderr, "usage: %1$s --is-loopdev <dev>\n",
			కార్యక్రమం_invocation_short_name);
	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు

