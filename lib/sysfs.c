/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <libgen.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "sysfs.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "buffer.h"

ಸ್ಥಿರ ಶೂನ್ಯ sysfs_blkdev_deinit_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ  sysfs_blkdev_enoent_redirect(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_ಖಾಲಿಗಳು_MASKNAMES;

#ವ್ಯಾಖ್ಯಾನಿಸು ULSYSFS_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULSYSFS_DEBUG_CXT	(1 << 2)

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

ಶೂನ್ಯ ul_sysfs_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (ulsysfs_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ರಚನೆ ಮಾರ್ಗ_cxt *ul_new_sysfs_ಮಾರ್ಗ(dev_t devno, ರಚನೆ ಮಾರ್ಗ_cxt *ಪೋಷಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ul_new_ಮಾರ್ಗ(ಖಾಲಿ);

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ)
		ul_ಮಾರ್ಗ_ಗಣ_ಪೂರ್ವಪ್ರತ್ಯಯ(pc, ಪೂರ್ವಪ್ರತ್ಯಯ);

	ಆದರೆ (sysfs_blkdev_init_ಮಾರ್ಗ(pc, devno, ಪೋಷಕ) != 0) {
		ul_unref_ಮಾರ್ಗ(pc);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ಹಿಂತಿರುಗಿ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ಪೂರ್ಣಾಂಕ sysfs_blkdev_init_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, dev_t devno, ರಚನೆ ಮಾರ್ಗ_cxt *ಪೋಷಕ)
{
	ರಚನೆ sysfs_blkdev *blk;
	ಪೂರ್ಣಾಂಕ rc;
	ಅಕ್ಷರ buf[ಗಾತ್ರ(ಮಾರ್ಗ_SYS_DEVBLOCK)
		 + ಗಾತ್ರ(stringify_ಮೌಲ್ಯ(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, ಗಾತ್ರ(buf), ಮಾರ್ಗ_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_ಮಾರ್ಗ_ಗಣ_dir(pc, buf);
	ಆದರೆ (rc)
		ಹಿಂತಿರುಗಿ rc;

	/* make sure path exists */
	rc = ul_ಮಾರ್ಗ_get_dirfd(pc);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_ಮಾರ್ಗ_get_dialect(pc);
	ಆದರೆ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ sysfs_blkdev));
		ಆದರೆ (!blk)
			ಹಿಂತಿರುಗಿ -ENOMEM;

		ul_ಮಾರ್ಗ_ಗಣ_dialect(pc, blk, sysfs_blkdev_deinit_ಮಾರ್ಗ);
		ul_ಮಾರ್ಗ_ಗಣ_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_ಗಣ_ಪೋಷಕ(pc, ಪೋಷಕ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ sysfs_blkdev_deinit_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ರಚನೆ sysfs_blkdev *blk;

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_ಮಾರ್ಗ_get_dialect(pc);
	ಆದರೆ (!blk)
		ಹಿಂತಿರುಗಿ;

	ul_unref_ಮಾರ್ಗ(blk->ಪೋಷಕ);
	ಬಿಡುಗಡೆ(blk);

	ul_ಮಾರ್ಗ_ಗಣ_dialect(pc, ಖಾಲಿ, ಖಾಲಿ);
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_ಗಣ_ಪೋಷಕ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ರಚನೆ ಮಾರ್ಗ_cxt *ಪೋಷಕ)
{
	ರಚನೆ sysfs_blkdev *blk = ul_ಮಾರ್ಗ_get_dialect(pc);

	ಆದರೆ (!pc || !blk)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (blk->ಪೋಷಕ) {
		ul_unref_ಮಾರ್ಗ(blk->ಪೋಷಕ);
		blk->ಪೋಷಕ = ಖಾಲಿ;
	}

	ಆದರೆ (ಪೋಷಕ) {
		ul_ref_ಮಾರ್ಗ(ಪೋಷಕ);
		blk->ಪೋಷಕ = ಪೋಷಕ;
	} ಇಲ್ಲದಿದ್ದರೆ
		blk->ಪೋಷಕ = ಖಾಲಿ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ಹಿಂತಿರುಗಿ 0;
}

ರಚನೆ ಮಾರ್ಗ_cxt *sysfs_blkdev_get_ಪೋಷಕ(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ರಚನೆ sysfs_blkdev *blk = ul_ಮಾರ್ಗ_get_dialect(pc);
	ಹಿಂತಿರುಗಿ blk ? blk->ಪೋಷಕ : ಖಾಲಿ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sysfs_blkdev_enoent_redirect(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ *dirfd)
{
	ರಚನೆ sysfs_blkdev *blk = ul_ಮಾರ್ಗ_get_dialect(pc);

	ಆದರೆ (blk && blk->ಪೋಷಕ && ಮಾರ್ಗ) {
		*dirfd = ul_ಮಾರ್ಗ_get_dirfd(blk->ಪೋಷಕ);
		ಆದರೆ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", ಮಾರ್ಗ));
			ಹಿಂತಿರುಗಿ 0;
		}
	}
	ಹಿಂತಿರುಗಿ 1;	/* no redirect */
}

ಅಕ್ಷರ *sysfs_blkdev_get_name(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ಅಕ್ಷರ link[PATH_MAX];
	ಅಕ್ಷರ *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_ಮಾರ್ಗ_readlink(pc, link, ಗಾತ್ರ(link), ಖಾಲಿ);
	ಆದರೆ (sz < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	name = strrchr(link, '/');
	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	name++;
	sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	ಆದರೆ ((size_t) sz + 1 > bufsiz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಸ್ಮರಣೆ_ನಕಲು(buf, name, sz + 1);
	sysfs_devname_sys_ಗಮ್ಯ_dev(buf);
	ಹಿಂತಿರುಗಿ buf;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_is_ವಿಭಜನೆ_dirent(DIR *dir, ರಚನೆ dirent *d, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೋಷಕ_name)
{
	ಅಕ್ಷರ ಮಾರ್ಗ[NAME_MAX + 6 + 1];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _DIRENT_HAVE_D_TYPE
	ಆದರೆ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
	size_t len = 0;

	ಆದರೆ (ಪೋಷಕ_name) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ಪೋಷಕ_name;

		/* /dev/sda --> "sda" */
		ಆದರೆ (*ಪೋಷಕ_name == '/') {
			p = strrchr(ಪೋಷಕ_name, '/');
			ಆದರೆ (!p)
				ಹಿಂತಿರುಗಿ 0;
			p++;
		}

		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p);
		ಆದರೆ ((ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(d->d_name) <= len) || (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p, d->d_name, len) != 0))
			len = 0;
	}

	ಆದರೆ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ಹಿಂತಿರುಗಿ ((*(d->d_name + len) == 'p' && ಅಂಕಿಯೇ(*(d->d_name + len + 1)))
			|| ಅಂಕಿಯೇ(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ), "%s/start", d->d_name);

	ಹಿಂತಿರುಗಿ faccessat(dirfd(dir), ಮಾರ್ಗ, R_OK, 0) == 0;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_ಎಣಿಕೆ_partitions(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	DIR *dir;
	ರಚನೆ dirent *d;
	ಪೂರ್ಣಾಂಕ r = 0;

	dir = ul_ಮಾರ್ಗ_opendir(pc, ಖಾಲಿ);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ ((d = xreaddir(dir))) {
		ಆದರೆ (sysfs_blkdev_is_ವಿಭಜನೆ_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ಹಿಂತಿರುಗಿ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_ಗಮ್ಯ_devno(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ partno)
{
	DIR *dir;
	ರಚನೆ dirent *d;
	dev_t devno = 0;

	dir = ul_ಮಾರ್ಗ_opendir(pc, ಖಾಲಿ);
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ ((d = xreaddir(dir))) {
		ಪೂರ್ಣಾಂಕ n;

		ಆದರೆ (!sysfs_blkdev_is_ವಿಭಜನೆ_dirent(dir, d, ಖಾಲಿ))
			ಮುಂದುವರಿ;

		ಆದರೆ (ul_ಮಾರ್ಗ_readf_s32(pc, &n, "%s/partition", d->d_name))
			ಮುಂದುವರಿ;

		ಆದರೆ (n == partno) {
			ಆದರೆ (ul_ಮಾರ್ಗ_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ನಿಲ್ಲಿಸು;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ಪೂರ್ಣಾಂಕ) partno, (ಪೂರ್ಣಾಂಕ) devno));
	ಹಿಂತಿರುಗಿ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
ಅಕ್ಷರ *sysfs_blkdev_get_slave(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	DIR *dir;
	ರಚನೆ dirent *d;
	ಅಕ್ಷರ *name = ಖಾಲಿ;

	dir = ul_ಮಾರ್ಗ_opendir(pc, "slaves");
	ಆದರೆ (!dir)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆಗ ((d = xreaddir(dir))) {
		ಆದರೆ (name)
			ಹೋಗು err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ಹಿಂತಿರುಗಿ name;
err:
	ಬಿಡುಗಡೆ(name);
	closedir(dir);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


#ವ್ಯಾಖ್ಯಾನಿಸು SUBSYSTEM_LINKNAME	"/subsystem"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 *
 * The function check if <chain>/subsystem symlink exists, if yes then returns
 * basename of the readlink result, and remove the last subdirectory from the
 * <chain> path.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *get_subsystem(ಅಕ್ಷರ *chain, ಅಕ್ಷರ *buf, size_t bufsz)
{
	size_t len;
	ಅಕ್ಷರ *p;

	ಆದರೆ (!chain || !*chain)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(chain);
	ಆದರೆ (len + ಗಾತ್ರ(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಮಾಡು {
		ssize_t sz;

		/* append "/subsystem" to the path */
		ಸ್ಮರಣೆ_ನಕಲು(chain + len, SUBSYSTEM_LINKNAME, ಗಾತ್ರ(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ಆದರೆ (p) {
			*p = '\0';
			len = p - chain;
		}

		ಆದರೆ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ಹಿಂತಿರುಗಿ basename(buf);
		}

	} ಆಗ (p);

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
ಅಕ್ಷರ *sysfs_blkdev_get_devchain(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ರಚನೆ ul_ಬಫರ್ tmp = UL_INIT_ಬಫರ್;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ssz = ul_ಮಾರ್ಗ_readlink(pc, buf, bufsz, ಖಾಲಿ);
	ಆದರೆ (ssz <= 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ ((p = ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(pc)))
		ul_ಬಫರ್_append_string(&tmp, p);

	ul_ಬಫರ್_append_string(&tmp, ಮಾರ್ಗ_SYS_DEVBLOCK "/");
	ul_ಬಫರ್_append_ದತ್ತಾಂಶ(&tmp, buf, ssz);

	p = ul_ಬಫರ್_get_string(&tmp, &sz, ಖಾಲಿ);
	ಆದರೆ (p && sz <= bufsz) {
		ಸ್ಮರಣೆ_ನಕಲು(buf, p, sz);
		res = buf;
	}
	ul_ಬಫರ್_free_ದತ್ತಾಂಶ(&tmp);
	ಹಿಂತಿರುಗಿ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ಪೂರ್ಣಾಂಕ sysfs_blkdev_ಮುಂದಿನ_subsystem(ರಚನೆ ಮಾರ್ಗ_cxt *pc __attribute__((ಬಳಸದ)),
			 ಅಕ್ಷರ *devchain, ಅಕ್ಷರ **subsys)
{
	ಅಕ್ಷರ subbuf[PATH_MAX];
	ಅಕ್ಷರ *sub;

	ಆದರೆ (!subsys || !devchain)
		ಹಿಂತಿರುಗಿ -EINVAL;

	*subsys = ಖಾಲಿ;

	ಆಗ ((sub = get_subsystem(devchain, subbuf, ಗಾತ್ರ(subbuf)))) {
		*subsys = strdup(sub);
		ಆದರೆ (!*subsys)
			ಹಿಂತಿರುಗಿ -ENOMEM;
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ 1;
}

#ವ್ಯಾಖ್ಯಾನಿಸು REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sysfs_devchain_is_removable(ಅಕ್ಷರ *chain)
{
	size_t len;
	ಅಕ್ಷರ buf[20];
	ಅಕ್ಷರ *p;

	ಆದರೆ (!chain || !*chain)
		ಹಿಂತಿರುಗಿ 0;

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(chain);
	ಆದರೆ (len + ಗಾತ್ರ(REMOVABLE_FILENAME) > PATH_MAX)
		ಹಿಂತಿರುಗಿ 0;

	ಮಾಡು {
		ಪೂರ್ಣಾಂಕ fd, rc;

		/* append "/removable" to the path */
		ಸ್ಮರಣೆ_ನಕಲು(chain + len, REMOVABLE_FILENAME, ಗಾತ್ರ(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ಆದರೆ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ನಿಲ್ಲಿಸು;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ಆದರೆ (fd != -1) {
			rc = ul_read_all(fd, buf, ಗಾತ್ರ(buf));
			close(fd);

			ಆದರೆ (rc > 0) {
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(buf, "fixed", min(rc, 5)) == 0) {
					ಹಿಂತಿರುಗಿ 0;
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(buf, "removable", min(rc, 9)) == 0) {
					ಹಿಂತಿರುಗಿ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ಆದರೆ (p) {
			*p = '\0';
			len = p - chain;
		}

	} ಆಗ (p);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_is_hotpluggable(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಅಕ್ಷರ buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, ಗಾತ್ರ(buf));
	ಹಿಂತಿರುಗಿ sysfs_devchain_is_removable(chain);
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_is_removable(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪೂರ್ಣಾಂಕ rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(pc, &rc, "removable") == 0)
		ಹಿಂತಿರುಗಿ rc;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_dm_wholedisk(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *diskname,
                            size_t len, dev_t *diskdevno)
{
	ಪೂರ್ಣಾಂಕ rc = 0;
	ಅಕ್ಷರ *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (diskname && len)
		xstrncpy(diskname, name, len);

	ಆದರೆ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(pc), name, ಖಾಲಿ);
		ಆದರೆ (!*diskdevno)
			rc = -1;
	}

	ಬಿಡುಗಡೆ(name);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ಪೂರ್ಣಾಂಕ sysfs_blkdev_get_wholedisk(	ರಚನೆ ಮಾರ್ಗ_cxt *pc,
				ಅಕ್ಷರ *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ಪೂರ್ಣಾಂಕ is_part = 0;

	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ -1;

	is_part = ul_ಮಾರ್ಗ_access(pc, F_OK, "partition") == 0;
	ಆದರೆ (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		ಅಕ್ಷರ *uuid = ಖಾಲಿ, *tmp, *ಪೂರ್ವಪ್ರತ್ಯಯ;

		ul_ಮಾರ್ಗ_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		ಪೂರ್ವಪ್ರತ್ಯಯ = uuid ? strsep(&tmp, "-") : ಖಾಲಿ;

		ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ && c_strncasecmp(ಪೂರ್ವಪ್ರತ್ಯಯ, "part", 4) == 0)
			is_part = 1;
		ಬಿಡುಗಡೆ(uuid);

		ಆದರೆ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			ಹೋಗು ಮುಗಿದ;

		is_part = 0;
	}

	ಆದರೆ (!is_part) {
		/*
		 * unpartitioned device
		 */
		ಆದರೆ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			ಹೋಗು err;
		ಆದರೆ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		ಅಕ್ಷರ linkpath[PATH_MAX];
		ಅಕ್ಷರ *name;
		ssize_t	linklen;

		linklen = ul_ಮಾರ್ಗ_readlink(pc, linkpath, ಗಾತ್ರ(linkpath), ಖಾಲಿ);
		ಆದರೆ (linklen < 0)
			ಹೋಗು err;

		stripoff_ಕೊನೆಯ_component(linkpath);	  /* dirname */
		name = stripoff_ಕೊನೆಯ_component(linkpath);   /* basename */
		ಆದರೆ (!name)
			ಹೋಗು err;

		sysfs_devname_sys_ಗಮ್ಯ_dev(name);
		ಆದರೆ (diskname && len)
			xstrncpy(diskname, name, len);

		ಆದರೆ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(pc), name, ಖಾಲಿ);
			ಆದರೆ (!*diskdevno)
				ಹೋಗು err;
		}
	}

ಮುಗಿದ:
	ಹಿಂತಿರುಗಿ 0;
err:
	ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ sysfs_devno_ಗಮ್ಯ_wholedisk(dev_t devno, ಅಕ್ಷರ *diskname,
		             size_t len, dev_t *diskdevno)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (!devno)
		ಹಿಂತಿರುಗಿ -EINVAL;
	pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns 1 if the device is a hidden device-mapper device -- i.e. a device
 * that exists only as a building block for another DM target and should not
 * be opened by user-space tools (blkid, mkfs, etc.) during udev processing.
 *
 * Unlike sysfs_devno_is_dm_private(), this does NOT flag Stratis devices.
 * Stratis places its own UUID ("stratis-1-private*") in the DM uuid field,
 * but its devices are legitimately opened by tools such as mkfs.xfs to
 * obtain device geometry.
 *
 * The @uuid (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ಪೂರ್ಣಾಂಕ sysfs_devno_is_dm_hidden(dev_t devno, ಅಕ್ಷರ **uuid,
			     ಸ್ಥಿರಾಂಕ ರಚನೆ ul_vfs_ops *vfs)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ಖಾಲಿ;
	ಅಕ್ಷರ *id = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc = 0;

	pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (!pc)
		ಹೋಗು ಮುಗಿದ;
	ul_ಮಾರ್ಗ_refer_vfs(pc, vfs);
	ಆದರೆ (ul_ಮಾರ್ಗ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ಹೋಗು ಮುಗಿದ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(id, "LVM-", 4) == 0) {
		ಅಕ್ಷರ *p = strrchr(id + 4, '-');

		ಆದರೆ (p && *(p + 1))
			rc = 1;
	}
ಮುಗಿದ:
	ul_unref_ಮಾರ್ಗ(pc);
	ಆದರೆ (uuid)
		*uuid = id;
	ಇಲ್ಲದಿದ್ದರೆ
		ಬಿಡುಗಡೆ(id);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ಪೂರ್ಣಾಂಕ sysfs_devno_is_dm_ಖಾಸಗಿ(dev_t devno, ಅಕ್ಷರ **uuid,
			      ಸ್ಥಿರಾಂಕ ರಚನೆ ul_vfs_ops *vfs)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ಖಾಲಿ;
	ಅಕ್ಷರ *id = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc = 0;

	pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (!pc)
		ಹೋಗು ಮುಗಿದ;
	ul_ಮಾರ್ಗ_refer_vfs(pc, vfs);
	ಆದರೆ (ul_ಮಾರ್ಗ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ಹೋಗು ಮುಗಿದ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(id, "LVM-", 4) == 0) {
		ಅಕ್ಷರ *p = strrchr(id + 4, '-');

		ಆದರೆ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
ಮುಗಿದ:
	ul_unref_ಮಾರ್ಗ(pc);
	ಆದರೆ (uuid)
		*uuid = id;
	ಇಲ್ಲದಿದ್ದರೆ
		ಬಿಡುಗಡೆ(id);
	ಹಿಂತಿರುಗಿ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ಪೂರ್ಣಾಂಕ sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ಆದರೆ (sysfs_devno_ಗಮ್ಯ_wholedisk(devno, ಖಾಲಿ, 0, &disk) != 0)
		ಹಿಂತಿರುಗಿ -1;

	ಹಿಂತಿರುಗಿ devno == disk;
}


ಪೂರ್ಣಾಂಕ sysfs_blkdev_scsi_get_hctl(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಪೂರ್ಣಾಂಕ *h, ಪೂರ್ಣಾಂಕ *c, ಪೂರ್ಣಾಂಕ *t, ಪೂರ್ಣಾಂಕ *l)
{
	ಅಕ್ಷರ buf[PATH_MAX], *hctl;
	ರಚನೆ sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_ಮಾರ್ಗ_get_dialect(pc);

	ಆದರೆ (!blk || blk->hctl_error)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಆದರೆ (blk->has_hctl)
		ಹೋಗು ಮುಗಿದ;

	blk->hctl_error = 1;
	len = ul_ಮಾರ್ಗ_readlink(pc, buf, ಗಾತ್ರ(buf), "device");
	ಆದರೆ (len < 0)
		ಹಿಂತಿರುಗಿ len;

	hctl = strrchr(buf, '/');
	ಆದರೆ (!hctl)
		ಹಿಂತಿರುಗಿ -1;
	hctl++;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_ಗುರಿ, &blk->scsi_lun) != 4)
		ಹಿಂತಿರುಗಿ -1;

	blk->has_hctl = 1;
ಮುಗಿದ:
	ಆದರೆ (h)
		*h = blk->scsi_host;
	ಆದರೆ (c)
		*c = blk->scsi_channel;
	ಆದರೆ (t)
		*t = blk->scsi_ಗುರಿ;
	ಆದರೆ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಅಕ್ಷರ *scsi_host_attribute_ಮಾರ್ಗ(
			ರಚನೆ ಮಾರ್ಗ_cxt *pc,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type,
			ಅಕ್ಷರ *buf,
			size_t bufsz,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr)
{
	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ host;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ;

	ಆದರೆ (sysfs_blkdev_scsi_get_hctl(pc, &host, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಪೂರ್ವಪ್ರತ್ಯಯ = ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(pc);
	ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಪೂರ್ವಪ್ರತ್ಯಯ = "";

	ಆದರೆ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ_SYS_CLASS, type, host, attr);
	ಇಲ್ಲದಿದ್ದರೆ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ_SYS_CLASS, type, host);

	ಹಿಂತಿರುಗಿ (len < 0 || (size_t) len >= bufsz) ? ಖಾಲಿ : buf;
}

ಅಕ್ಷರ *sysfs_blkdev_scsi_host_strdup_attribute(ರಚನೆ ಮಾರ್ಗ_cxt *pc,
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr)
{
	ಅಕ್ಷರ buf[1024];
	ಪೂರ್ಣಾಂಕ rc;
	FILE *f;

	ಆದರೆ (!attr || !type ||
	    !scsi_host_attribute_ಮಾರ್ಗ(pc, type, buf, ಗಾತ್ರ(buf), attr))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ಹಿಂತಿರುಗಿ rc == 1 ? strdup(buf) : ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_scsi_host_is(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type)
{
	ಅಕ್ಷರ buf[PATH_MAX];
	ರಚನೆ stat st;

	ಆದರೆ (!type || !scsi_host_attribute_ಮಾರ್ಗ(pc, type,
				buf, ಗಾತ್ರ(buf), ಖಾಲಿ))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ಸ್ಥಿರ ಅಕ್ಷರ *scsi_attribute_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc,
		ಅಕ್ಷರ *buf, size_t bufsz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr)
{
	ಪೂರ್ಣಾಂಕ len, h, c, t, l;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ;

	ಆದರೆ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಪೂರ್ವಪ್ರತ್ಯಯ = ul_ಮಾರ್ಗ_get_ಪೂರ್ವಪ್ರತ್ಯಯ(pc);
	ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಪೂರ್ವಪ್ರತ್ಯಯ = "";

	ಆದರೆ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ_SYS_SCSI,
				h,c,t,l, attr);
	ಇಲ್ಲದಿದ್ದರೆ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ_SYS_SCSI,
				h,c,t,l);
	ಹಿಂತಿರುಗಿ (len < 0 || (size_t) len >= bufsz) ? ಖಾಲಿ : buf;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_scsi_has_attribute(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *attr)
{
	ಅಕ್ಷರ ಮಾರ್ಗ[PATH_MAX];
	ರಚನೆ stat st;

	ಆದರೆ (!scsi_attribute_ಮಾರ್ಗ(pc, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ), attr))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ stat(ಮಾರ್ಗ, &st) == 0;
}

ಪೂರ್ಣಾಂಕ sysfs_blkdev_scsi_ಮಾರ್ಗ_ಒಳಗೊಂಡಿದೆ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ)
{
	ಅಕ್ಷರ ಮಾರ್ಗ[PATH_MAX], linkc[PATH_MAX];
	ರಚನೆ stat st;
	ssize_t len;

	ಆದರೆ (!scsi_attribute_ಮಾರ್ಗ(pc, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ), ಖಾಲಿ))
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (stat(ಮಾರ್ಗ, &st) != 0)
		ಹಿಂತಿರುಗಿ 0;

	len = readlink(ಮಾರ್ಗ, linkc, ಗಾತ್ರ(linkc) - 1);
	ಆದರೆ (len < 0)
		ಹಿಂತಿರುಗಿ 0;

	linkc[len] = '\0';
	ಹಿಂತಿರುಗಿ ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(linkc, ನಮೂನೆ) != ಖಾಲಿ;
}

ಸ್ಥಿರ dev_t read_devno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	FILE *f;
	ಪೂರ್ಣಾಂಕ maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(ಮಾರ್ಗ, "r" UL_CLOEXECSTR);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ಹಿಂತಿರುಗಿ dev;
}

ಪೂರ್ಣಾಂಕ sysfs_devname_is_hidden(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಅಕ್ಷರ buf[PATH_MAX];
	ಪೂರ್ಣಾಂಕ rc = 0, hidden = 0, len;
	FILE *f;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("/dev/", name, 5) == 0)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಪೂರ್ವಪ್ರತ್ಯಯ = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, ಗಾತ್ರ(buf),
			"%s" ಮಾರ್ಗ_SYS_ಖಂಡ "/%s/hidden",
			ಪೂರ್ವಪ್ರತ್ಯಯ, name);

	ಆದರೆ (len < 0 || (size_t) len + 1 > ಗಾತ್ರ(buf))
		ಹಿಂತಿರುಗಿ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ಆದರೆ (!f)
		ಹಿಂತಿರುಗಿ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ಹಿಂತಿರುಗಿ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೋಷಕ)
{
	ಅಕ್ಷರ buf[PATH_MAX];
	ಅಕ್ಷರ *_name = ಖಾಲಿ, *ಪೋಷಕ_2 = ಖಾಲಿ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ಪೂರ್ಣಾಂಕ len;

	ಆದರೆ (!ಪೂರ್ವಪ್ರತ್ಯಯ)
		ಪೂರ್ವಪ್ರತ್ಯಯ = "";

	ಪ್ರತಿಜ್ಞಾ(name);

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ರಚನೆ stat st;

		ಆದರೆ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			ಹೋಗು ಮುಗಿದ;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ಆದರೆ (!_name)
		ಹೋಗು ಮುಗಿದ;
	sysfs_devname_dev_ಗಮ್ಯ_sys(_name);

	ಆದರೆ (ಪೋಷಕ) {
		ಪೋಷಕ_2 = strdup(ಪೋಷಕ);
		ಆದರೆ (!ಪೋಷಕ_2)
			ಹೋಗು ಮುಗಿದ;
	}

	ಆದರೆ (ಪೋಷಕ && ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_ಗಮ್ಯ_sys(ಪೋಷಕ_2);
		len = snprintf(buf, ಗಾತ್ರ(buf),
				"%s" ಮಾರ್ಗ_SYS_ಖಂಡ "/%s/%s/dev",
				ಪೂರ್ವಪ್ರತ್ಯಯ,	ಪೋಷಕ_2, _name);
		ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
			ಹೋಗು ಮುಗಿದ;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		ಹೋಗು ಮುಗಿದ;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, ಗಾತ್ರ(buf),
			"%s" ಮಾರ್ಗ_SYS_ಖಂಡ "/%s/dev",
			ಪೂರ್ವಪ್ರತ್ಯಯ, _name);
	ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
		ಹೋಗು ಮುಗಿದ;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ಆದರೆ (!dev && ಪೋಷಕ && ul_startswith(name, ಪೋಷಕ)) {
		len = snprintf(buf, ಗಾತ್ರ(buf),
				"%s" ಮಾರ್ಗ_SYS_ಖಂಡ "/%s/%s/dev",
				ಪೂರ್ವಪ್ರತ್ಯಯ, ಪೋಷಕ_2, _name);
		ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
			ಹೋಗು ಮುಗಿದ;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ಆದರೆ (!dev) {
		len = snprintf(buf, ಗಾತ್ರ(buf),
				"%s" ಮಾರ್ಗ_SYS_ಖಂಡ "/%s/device/dev",
				ಪೂರ್ವಪ್ರತ್ಯಯ, _name);
		ಆದರೆ (len < 0 || (size_t) len >= ಗಾತ್ರ(buf))
			ಹೋಗು ಮುಗಿದ;
		dev = read_devno(buf);
	}
ಮುಗಿದ:
	ಬಿಡುಗಡೆ(_name);
	ಬಿಡುಗಡೆ(ಪೋಷಕ_2);
	ಹಿಂತಿರುಗಿ dev;
}

dev_t sysfs_devname_ಗಮ್ಯ_devno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಹಿಂತಿರುಗಿ __sysfs_devname_to_devno(ಖಾಲಿ, name, ಖಾಲಿ);
}

ಅಕ್ಷರ *sysfs_blkdev_get_ಮಾರ್ಗ(ರಚನೆ ಮಾರ್ಗ_cxt *pc, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	ಅಕ್ಷರ *res = ಖಾಲಿ;
	size_t sz;
	ರಚನೆ stat st;

	ಆದರೆ (!name)
		ಹೋಗು ಮುಗಿದ;

	sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	ಆದರೆ (sz + ಗಾತ್ರ("/dev/") > bufsiz)
		ಹೋಗು ಮುಗಿದ;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	ಸ್ಮರಣೆ_ನಕಲು(buf, "/dev/", 5);

	ಆದರೆ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
ಮುಗಿದ:
	ಹಿಂತಿರುಗಿ res;
}

dev_t sysfs_blkdev_get_devno(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಹಿಂತಿರುಗಿ ((ರಚನೆ sysfs_blkdev *) ul_ಮಾರ್ಗ_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
ಅಕ್ಷರ *sysfs_devno_ಗಮ್ಯ_devpath(dev_t devno, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (pc) {
		res = sysfs_blkdev_get_ಮಾರ್ಗ(pc, buf, bufsiz);
		ul_unref_ಮಾರ್ಗ(pc);
	}
	ಹಿಂತಿರುಗಿ res;
}

ಅಕ್ಷರ *sysfs_devno_ಗಮ್ಯ_devname(dev_t devno, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಅಕ್ಷರ *res = ಖಾಲಿ;

	ಆದರೆ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_ಮಾರ್ಗ(pc);
	}
	ಹಿಂತಿರುಗಿ res;
}

ಪೂರ್ಣಾಂಕ sysfs_devno_ಎಣಿಕೆ_partitions(dev_t devno)
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಪೂರ್ಣಾಂಕ n = 0;

	ಆದರೆ (pc) {
		ಅಕ್ಷರ buf[PATH_MAX + 1];
		ಅಕ್ಷರ *name = sysfs_blkdev_get_name(pc, buf, ಗಾತ್ರ(buf));

		n = sysfs_blkdev_ಎಣಿಕೆ_partitions(pc, name);
		ul_unref_ಮಾರ್ಗ(pc);
	}
	ಹಿಂತಿರುಗಿ n;
}

ಅಕ್ಷರ *sysfs_chrdev_devno_ಗಮ್ಯ_devname(dev_t devno, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ಅಕ್ಷರ link[PATH_MAX];
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	ಅಕ್ಷರ *name;
	ssize_t	sz;

	pc = ul_new_ಮಾರ್ಗ(ಮಾರ್ಗ_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ಆದರೆ (!pc)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_ಮಾರ್ಗ_readlink(pc, link, ಗಾತ್ರ(link), ಖಾಲಿ);
	ul_unref_ಮಾರ್ಗ(pc);

	ಆದರೆ (sz < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	name = strrchr(link, '/');
	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	name++;
	sz = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	ಆದರೆ ((size_t) sz + 1 > bufsiz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಸ್ಮರಣೆ_ನಕಲು(buf, name, sz + 1);
	sysfs_devname_sys_ಗಮ್ಯ_dev(buf);
	ಹಿಂತಿರುಗಿ buf;

}

ಎಣಿಕೆ sysfs_byteorder sysfs_get_byteorder(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಅಕ್ಷರ buf[BUFSIZ];
	ಎಣಿಕೆ sysfs_byteorder ret;

	rc = ul_ಮಾರ್ಗ_read_ಬಫರ್(pc, buf, ಗಾತ್ರ(buf), ಮಾರ್ಗ_SYS_CPU_BYTEORDER);
	ಆದರೆ (rc < 0)
		ಹೋಗು unknown;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(buf, "little", ಗಾತ್ರ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		ಹೋಗು ಹೊರಹೋಗು;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(buf, "big", ಗಾತ್ರ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		ಹೋಗು ಹೊರಹೋಗು;
	}

unknown:
#ಆದರೆ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#ಅಥವಾಆದರೆ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ Unknown byte ಕ್ರಮ
#ಆದರೆ_ಅಂತ್ಯ

ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ sysfs_get_ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು(ರಚನೆ ಮಾರ್ಗ_cxt *pc)
{
	ಪೂರ್ಣಾಂಕ rc;
	ಪೂರ್ಣಾಂಕ ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು;

	rc = ul_ಮಾರ್ಗ_scanf(pc, ಮಾರ್ಗ_SYS_ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು, "%d", &ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು);
	ಆದರೆ (rc < 0)
		ಹಿಂತಿರುಗಿ rc;
	ಆದರೆ (ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;
	ಹಿಂತಿರುಗಿ ವಿಳಾಸ_ಬಿಟ್ಟುಗಳು;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_SYSFS
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ರಚನೆ ಮಾರ್ಗ_cxt *pc;
	ಅಕ್ಷರ *devname;
	dev_t devno, disk_devno;
	ಅಕ್ಷರ ಮಾರ್ಗ[PATH_MAX], *sub, *chain;
	ಅಕ್ಷರ diskname[32];
	ಪೂರ್ಣಾಂಕ i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ಆದರೆ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_ಗಮ್ಯ_devno(devname);

	ಆದರೆ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	ಬರೆ("non-context:\n");
	ಬರೆ(" DEVNO:   %u (%u:%u)\n", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) devno, major(devno), minor(devno));
	ಬರೆ(" DEVNAME: %s\n", sysfs_devno_ಗಮ್ಯ_devname(devno, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ)));
	ಬರೆ(" DEVPATH: %s\n", sysfs_devno_ಗಮ್ಯ_devpath(devno, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ)));

	sysfs_devno_ಗಮ್ಯ_wholedisk(devno, diskname, ಗಾತ್ರ(diskname), &disk_devno);
	ಬರೆ(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) disk_devno, major(disk_devno), minor(disk_devno));
	ಬರೆ(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_ಮಾರ್ಗ(devno, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (!pc)
		ಹೋಗು ಮುಗಿದ;

	ಬರೆ("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	ಬರೆ(" DEVNO:   %u (%u:%u)\n", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) devno, major(devno), minor(devno));
	ಬರೆ(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ)));
	ಬರೆ(" DEVPATH: %s\n", sysfs_blkdev_get_ಮಾರ್ಗ(pc, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ)));

	sysfs_devno_ಗಮ್ಯ_wholedisk(devno, diskname, ಗಾತ್ರ(diskname), &disk_devno);
	ಬರೆ(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) disk_devno, major(disk_devno), minor(disk_devno));
	ಬರೆ(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_ಮಾರ್ಗ_access(pc, F_OK, "partition") == 0;
	ಬರೆ(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ಆದರೆ (is_part && disk_devno) {
		ರಚನೆ ಮಾರ್ಗ_cxt *disk_pc =  ul_new_sysfs_ಮಾರ್ಗ(disk_devno, ಖಾಲಿ, ಖಾಲಿ);
		sysfs_blkdev_ಗಣ_ಪೋಷಕ(pc, disk_pc);

		ul_unref_ಮಾರ್ಗ(disk_pc);
	}

	ಬರೆ(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	ಬರೆ(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	ಬರೆ(" SLAVES: %d\n", ul_ಮಾರ್ಗ_ಎಣಿಕೆ_dirents(pc, "slaves"));

	ಆದರೆ (!is_part) {
		ಬರೆ("First 5 partitions:\n");
		ಗಾಗಿ (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_ಗಮ್ಯ_devno(pc, i);
			ಆದರೆ (dev)
				ಬರೆ("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ಆದರೆ (ul_ಮಾರ್ಗ_read_u64(pc, &u64, "size") != 0)
		ಬರೆ(" (!) read SIZE failed\n");
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ(" SIZE: %"PRIu64"\n", u64);

	ಆದರೆ (ul_ಮಾರ್ಗ_read_s32(pc, &i, "queue/hw_sector_size"))
		ಬರೆ(" (!) read SECTOR failed\n");
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, ಮಾರ್ಗ, ಗಾತ್ರ(ಮಾರ್ಗ));
	ಬರೆ(" SUBSYSTEMS:\n");

	ಆಗ (chain && sysfs_blkdev_ಮುಂದಿನ_subsystem(pc, chain, &sub) == 0) {
		ಬರೆ("\t%s\n", sub);
		ಬಿಡುಗಡೆ(sub);
	}

	rc = EXIT_SUCCESS;
ಮುಗಿದ:
	ul_unref_ಮಾರ್ಗ(pc);
	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_SYSFS */
