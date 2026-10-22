/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sysfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "buffer.h"

ਸਥਿਰ ਖਾਲੀ sysfs_blkdev_deinit_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc);
ਸਥਿਰ ਪੂਰਨ  sysfs_blkdev_enoent_redirect(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULSYSFS_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULSYSFS_DEBUG_CXT	(1 << 2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

ਖਾਲੀ ul_sysfs_init_debug(ਖਾਲੀ)
{
	ਜੇ (ulsysfs_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ਬਣਤਰ ਰਸਤਾ_cxt *ul_new_sysfs_ਰਸਤਾ(dev_t devno, ਬਣਤਰ ਰਸਤਾ_cxt *ਮਾਪਾ, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ul_new_ਰਸਤਾ(ਨਿਰਾ);

	ਜੇ (!pc)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (ਅਗੇਤਰ)
		ul_ਰਸਤਾ_ਸੈੱਟ_ਅਗੇਤਰ(pc, ਅਗੇਤਰ);

	ਜੇ (sysfs_blkdev_init_ਰਸਤਾ(pc, devno, ਮਾਪਾ) != 0) {
		ul_unref_ਰਸਤਾ(pc);
		ਵਾਪਸ ਨਿਰਾ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ਵਾਪਸ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ਪੂਰਨ sysfs_blkdev_init_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, dev_t devno, ਬਣਤਰ ਰਸਤਾ_cxt *ਮਾਪਾ)
{
	ਬਣਤਰ sysfs_blkdev *blk;
	ਪੂਰਨ rc;
	ਅੱਖਰ buf[ਆਕਾਰ(ਰਸਤਾ_SYS_DEVBLOCK)
		 + ਆਕਾਰ(stringify_ਮੁੱਲ(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, ਆਕਾਰ(buf), ਰਸਤਾ_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_ਰਸਤਾ_ਸੈੱਟ_dir(pc, buf);
	ਜੇ (rc)
		ਵਾਪਸ rc;

	/* make sure path exists */
	rc = ul_ਰਸਤਾ_get_dirfd(pc);
	ਜੇ (rc < 0)
		ਵਾਪਸ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_ਰਸਤਾ_get_dialect(pc);
	ਜੇ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ sysfs_blkdev));
		ਜੇ (!blk)
			ਵਾਪਸ -ENOMEM;

		ul_ਰਸਤਾ_ਸੈੱਟ_dialect(pc, blk, sysfs_blkdev_deinit_ਰਸਤਾ);
		ul_ਰਸਤਾ_ਸੈੱਟ_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_ਸੈੱਟ_ਮਾਪਾ(pc, ਮਾਪਾ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ sysfs_blkdev_deinit_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਬਣਤਰ sysfs_blkdev *blk;

	ਜੇ (!pc)
		ਵਾਪਸ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_ਰਸਤਾ_get_dialect(pc);
	ਜੇ (!blk)
		ਵਾਪਸ;

	ul_unref_ਰਸਤਾ(blk->ਮਾਪਾ);
	ਮੁਕਤ(blk);

	ul_ਰਸਤਾ_ਸੈੱਟ_dialect(pc, ਨਿਰਾ, ਨਿਰਾ);
}

ਪੂਰਨ sysfs_blkdev_ਸੈੱਟ_ਮਾਪਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ ਰਸਤਾ_cxt *ਮਾਪਾ)
{
	ਬਣਤਰ sysfs_blkdev *blk = ul_ਰਸਤਾ_get_dialect(pc);

	ਜੇ (!pc || !blk)
		ਵਾਪਸ -EINVAL;

	ਜੇ (blk->ਮਾਪਾ) {
		ul_unref_ਰਸਤਾ(blk->ਮਾਪਾ);
		blk->ਮਾਪਾ = ਨਿਰਾ;
	}

	ਜੇ (ਮਾਪਾ) {
		ul_ref_ਰਸਤਾ(ਮਾਪਾ);
		blk->ਮਾਪਾ = ਮਾਪਾ;
	} ਨਹੀਂਤਾਂ
		blk->ਮਾਪਾ = ਨਿਰਾ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ਵਾਪਸ 0;
}

ਬਣਤਰ ਰਸਤਾ_cxt *sysfs_blkdev_get_ਮਾਪਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਬਣਤਰ sysfs_blkdev *blk = ul_ਰਸਤਾ_get_dialect(pc);
	ਵਾਪਸ blk ? blk->ਮਾਪਾ : ਨਿਰਾ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ਸਥਿਰ ਪੂਰਨ sysfs_blkdev_enoent_redirect(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਪੂਰਨ *dirfd)
{
	ਬਣਤਰ sysfs_blkdev *blk = ul_ਰਸਤਾ_get_dialect(pc);

	ਜੇ (blk && blk->ਮਾਪਾ && ਰਸਤਾ) {
		*dirfd = ul_ਰਸਤਾ_get_dirfd(blk->ਮਾਪਾ);
		ਜੇ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", ਰਸਤਾ));
			ਵਾਪਸ 0;
		}
	}
	ਵਾਪਸ 1;	/* no redirect */
}

ਅੱਖਰ *sysfs_blkdev_get_name(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਅੱਖਰ link[PATH_MAX];
	ਅੱਖਰ *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_ਰਸਤਾ_readlink(pc, link, ਆਕਾਰ(link), ਨਿਰਾ);
	ਜੇ (sz < 0)
		ਵਾਪਸ ਨਿਰਾ;

	name = strrchr(link, '/');
	ਜੇ (!name)
		ਵਾਪਸ ਨਿਰਾ;

	name++;
	sz = ਲੜੀ_ਲੰਬਾਈ(name);
	ਜੇ ((size_t) sz + 1 > bufsiz)
		ਵਾਪਸ ਨਿਰਾ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, name, sz + 1);
	sysfs_devname_sys_ਮੰਜ਼ਿਲ_dev(buf);
	ਵਾਪਸ buf;
}

ਪੂਰਨ sysfs_blkdev_is_ਹਿੱਸੇ_ਕਰੋ_dirent(DIR *dir, ਬਣਤਰ dirent *d, ਅਚਲ ਅੱਖਰ *ਮਾਪਾ_name)
{
	ਅੱਖਰ ਰਸਤਾ[NAME_MAX + 6 + 1];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _DIRENT_HAVE_D_TYPE
	ਜੇ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
	size_t len = 0;

	ਜੇ (ਮਾਪਾ_name) {
		ਅਚਲ ਅੱਖਰ *p = ਮਾਪਾ_name;

		/* /dev/sda --> "sda" */
		ਜੇ (*ਮਾਪਾ_name == '/') {
			p = strrchr(ਮਾਪਾ_name, '/');
			ਜੇ (!p)
				ਵਾਪਸ 0;
			p++;
		}

		len = ਲੜੀ_ਲੰਬਾਈ(p);
		ਜੇ ((ਲੜੀ_ਲੰਬਾਈ(d->d_name) <= len) || (ਲੜੀ_ਤੁਲਨਾ(p, d->d_name, len) != 0))
			len = 0;
	}

	ਜੇ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ਵਾਪਸ ((*(d->d_name + len) == 'p' && ਅੰਕ_ਹੈ(*(d->d_name + len + 1)))
			|| ਅੰਕ_ਹੈ(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ), "%s/start", d->d_name);

	ਵਾਪਸ faccessat(dirfd(dir), ਰਸਤਾ, R_OK, 0) == 0;
}

ਪੂਰਨ sysfs_blkdev_ਗਿਣਤੀ_partitions(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *devname)
{
	DIR *dir;
	ਬਣਤਰ dirent *d;
	ਪੂਰਨ r = 0;

	dir = ul_ਰਸਤਾ_opendir(pc, ਨਿਰਾ);
	ਜੇ (!dir)
		ਵਾਪਸ 0;

	ਜਦੋਂ ((d = xreaddir(dir))) {
		ਜੇ (sysfs_blkdev_is_ਹਿੱਸੇ_ਕਰੋ_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ਵਾਪਸ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_ਮੰਜ਼ਿਲ_devno(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ partno)
{
	DIR *dir;
	ਬਣਤਰ dirent *d;
	dev_t devno = 0;

	dir = ul_ਰਸਤਾ_opendir(pc, ਨਿਰਾ);
	ਜੇ (!dir)
		ਵਾਪਸ 0;

	ਜਦੋਂ ((d = xreaddir(dir))) {
		ਪੂਰਨ n;

		ਜੇ (!sysfs_blkdev_is_ਹਿੱਸੇ_ਕਰੋ_dirent(dir, d, ਨਿਰਾ))
			ਜਾਰੀ;

		ਜੇ (ul_ਰਸਤਾ_readf_s32(pc, &n, "%s/partition", d->d_name))
			ਜਾਰੀ;

		ਜੇ (n == partno) {
			ਜੇ (ul_ਰਸਤਾ_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ਰੋਕੋ;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ਪੂਰਨ) partno, (ਪੂਰਨ) devno));
	ਵਾਪਸ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
ਅੱਖਰ *sysfs_blkdev_get_slave(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	DIR *dir;
	ਬਣਤਰ dirent *d;
	ਅੱਖਰ *name = ਨਿਰਾ;

	dir = ul_ਰਸਤਾ_opendir(pc, "slaves");
	ਜੇ (!dir)
		ਵਾਪਸ ਨਿਰਾ;

	ਜਦੋਂ ((d = xreaddir(dir))) {
		ਜੇ (name)
			ਜਾਓ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ਵਾਪਸ name;
err:
	ਮੁਕਤ(name);
	closedir(dir);
	ਵਾਪਸ ਨਿਰਾ;
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SUBSYSTEM_LINKNAME	"/subsystem"

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
ਸਥਿਰ ਅੱਖਰ *get_subsystem(ਅੱਖਰ *chain, ਅੱਖਰ *buf, size_t bufsz)
{
	size_t len;
	ਅੱਖਰ *p;

	ਜੇ (!chain || !*chain)
		ਵਾਪਸ ਨਿਰਾ;

	len = ਲੜੀ_ਲੰਬਾਈ(chain);
	ਜੇ (len + ਆਕਾਰ(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ਵਾਪਸ ਨਿਰਾ;

	ਕਰੋ {
		ssize_t sz;

		/* append "/subsystem" to the path */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(chain + len, SUBSYSTEM_LINKNAME, ਆਕਾਰ(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ਜੇ (p) {
			*p = '\0';
			len = p - chain;
		}

		ਜੇ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ਵਾਪਸ basename(buf);
		}

	} ਜਦੋਂ (p);

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
ਅੱਖਰ *sysfs_blkdev_get_devchain(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ਬਣਤਰ ul_ਬਫਰ tmp = UL_INIT_ਬਫਰ;
	ਅਚਲ ਅੱਖਰ *p;
	ਅੱਖਰ *res = ਨਿਰਾ;

	ssz = ul_ਰਸਤਾ_readlink(pc, buf, bufsz, ਨਿਰਾ);
	ਜੇ (ssz <= 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ ((p = ul_ਰਸਤਾ_get_ਅਗੇਤਰ(pc)))
		ul_ਬਫਰ_append_string(&tmp, p);

	ul_ਬਫਰ_append_string(&tmp, ਰਸਤਾ_SYS_DEVBLOCK "/");
	ul_ਬਫਰ_append_ਅੰਕੜੇ(&tmp, buf, ssz);

	p = ul_ਬਫਰ_get_string(&tmp, &sz, ਨਿਰਾ);
	ਜੇ (p && sz <= bufsz) {
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, p, sz);
		res = buf;
	}
	ul_ਬਫਰ_free_ਅੰਕੜੇ(&tmp);
	ਵਾਪਸ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ਪੂਰਨ sysfs_blkdev_ਅਗਲਾ_subsystem(ਬਣਤਰ ਰਸਤਾ_cxt *pc __attribute__((ਅਣਵਰਤਿਆ)),
			 ਅੱਖਰ *devchain, ਅੱਖਰ **subsys)
{
	ਅੱਖਰ subbuf[PATH_MAX];
	ਅੱਖਰ *sub;

	ਜੇ (!subsys || !devchain)
		ਵਾਪਸ -EINVAL;

	*subsys = ਨਿਰਾ;

	ਜਦੋਂ ((sub = get_subsystem(devchain, subbuf, ਆਕਾਰ(subbuf)))) {
		*subsys = strdup(sub);
		ਜੇ (!*subsys)
			ਵਾਪਸ -ENOMEM;
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ 1;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ਸਥਿਰ ਪੂਰਨ sysfs_devchain_is_removable(ਅੱਖਰ *chain)
{
	size_t len;
	ਅੱਖਰ buf[20];
	ਅੱਖਰ *p;

	ਜੇ (!chain || !*chain)
		ਵਾਪਸ 0;

	len = ਲੜੀ_ਲੰਬਾਈ(chain);
	ਜੇ (len + ਆਕਾਰ(REMOVABLE_FILENAME) > PATH_MAX)
		ਵਾਪਸ 0;

	ਕਰੋ {
		ਪੂਰਨ fd, rc;

		/* append "/removable" to the path */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(chain + len, REMOVABLE_FILENAME, ਆਕਾਰ(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ਜੇ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ਰੋਕੋ;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ਜੇ (fd != -1) {
			rc = ul_read_all(fd, buf, ਆਕਾਰ(buf));
			close(fd);

			ਜੇ (rc > 0) {
				ਜੇ (ਲੜੀ_ਤੁਲਨਾ(buf, "fixed", min(rc, 5)) == 0) {
					ਵਾਪਸ 0;
				} ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(buf, "removable", min(rc, 9)) == 0) {
					ਵਾਪਸ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ਜੇ (p) {
			*p = '\0';
			len = p - chain;
		}

	} ਜਦੋਂ (p);

	ਵਾਪਸ 0;
}

ਪੂਰਨ sysfs_blkdev_is_hotpluggable(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਅੱਖਰ buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, ਆਕਾਰ(buf));
	ਵਾਪਸ sysfs_devchain_is_removable(chain);
}

ਪੂਰਨ sysfs_blkdev_is_removable(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪੂਰਨ rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ਜੇ (ul_ਰਸਤਾ_read_s32(pc, &rc, "removable") == 0)
		ਵਾਪਸ rc;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ get_dm_wholedisk(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *diskname,
                            size_t len, dev_t *diskdevno)
{
	ਪੂਰਨ rc = 0;
	ਅੱਖਰ *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ਜੇ (!name)
		ਵਾਪਸ -1;

	ਜੇ (diskname && len)
		xstrncpy(diskname, name, len);

	ਜੇ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_ਰਸਤਾ_get_ਅਗੇਤਰ(pc), name, ਨਿਰਾ);
		ਜੇ (!*diskdevno)
			rc = -1;
	}

	ਮੁਕਤ(name);
	ਵਾਪਸ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ਪੂਰਨ sysfs_blkdev_get_wholedisk(	ਬਣਤਰ ਰਸਤਾ_cxt *pc,
				ਅੱਖਰ *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ਪੂਰਨ is_part = 0;

	ਜੇ (!pc)
		ਵਾਪਸ -1;

	is_part = ul_ਰਸਤਾ_access(pc, F_OK, "partition") == 0;
	ਜੇ (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		ਅੱਖਰ *uuid = ਨਿਰਾ, *tmp, *ਅਗੇਤਰ;

		ul_ਰਸਤਾ_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		ਅਗੇਤਰ = uuid ? strsep(&tmp, "-") : ਨਿਰਾ;

		ਜੇ (ਅਗੇਤਰ && c_strncasecmp(ਅਗੇਤਰ, "part", 4) == 0)
			is_part = 1;
		ਮੁਕਤ(uuid);

		ਜੇ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			ਜਾਓ ਪੂਰਾ;

		is_part = 0;
	}

	ਜੇ (!is_part) {
		/*
		 * unpartitioned device
		 */
		ਜੇ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			ਜਾਓ err;
		ਜੇ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ਨਹੀਂਤਾਂ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		ਅੱਖਰ linkpath[PATH_MAX];
		ਅੱਖਰ *name;
		ssize_t	linklen;

		linklen = ul_ਰਸਤਾ_readlink(pc, linkpath, ਆਕਾਰ(linkpath), ਨਿਰਾ);
		ਜੇ (linklen < 0)
			ਜਾਓ err;

		stripoff_ਆਖਰੀ_component(linkpath);	  /* dirname */
		name = stripoff_ਆਖਰੀ_component(linkpath);   /* basename */
		ਜੇ (!name)
			ਜਾਓ err;

		sysfs_devname_sys_ਮੰਜ਼ਿਲ_dev(name);
		ਜੇ (diskname && len)
			xstrncpy(diskname, name, len);

		ਜੇ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_ਰਸਤਾ_get_ਅਗੇਤਰ(pc), name, ਨਿਰਾ);
			ਜੇ (!*diskdevno)
				ਜਾਓ err;
		}
	}

ਪੂਰਾ:
	ਵਾਪਸ 0;
err:
	ਵਾਪਸ -1;
}

ਪੂਰਨ sysfs_devno_ਮੰਜ਼ਿਲ_wholedisk(dev_t devno, ਅੱਖਰ *diskname,
		             size_t len, dev_t *diskdevno)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	ਪੂਰਨ rc = 0;

	ਜੇ (!devno)
		ਵਾਪਸ -EINVAL;
	pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (!pc)
		ਵਾਪਸ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ rc;
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
ਪੂਰਨ sysfs_devno_is_dm_hidden(dev_t devno, ਅੱਖਰ **uuid,
			     ਅਚਲ ਬਣਤਰ ul_vfs_ops *vfs)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ਨਿਰਾ;
	ਅੱਖਰ *id = ਨਿਰਾ;
	ਪੂਰਨ rc = 0;

	pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (!pc)
		ਜਾਓ ਪੂਰਾ;
	ul_ਰਸਤਾ_refer_vfs(pc, vfs);
	ਜੇ (ul_ਰਸਤਾ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ਜਾਓ ਪੂਰਾ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(id, "LVM-", 4) == 0) {
		ਅੱਖਰ *p = strrchr(id + 4, '-');

		ਜੇ (p && *(p + 1))
			rc = 1;
	}
ਪੂਰਾ:
	ul_unref_ਰਸਤਾ(pc);
	ਜੇ (uuid)
		*uuid = id;
	ਨਹੀਂਤਾਂ
		ਮੁਕਤ(id);
	ਵਾਪਸ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ਪੂਰਨ sysfs_devno_is_dm_ਨਿੱਜੀ(dev_t devno, ਅੱਖਰ **uuid,
			      ਅਚਲ ਬਣਤਰ ul_vfs_ops *vfs)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ਨਿਰਾ;
	ਅੱਖਰ *id = ਨਿਰਾ;
	ਪੂਰਨ rc = 0;

	pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (!pc)
		ਜਾਓ ਪੂਰਾ;
	ul_ਰਸਤਾ_refer_vfs(pc, vfs);
	ਜੇ (ul_ਰਸਤਾ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ਜਾਓ ਪੂਰਾ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(id, "LVM-", 4) == 0) {
		ਅੱਖਰ *p = strrchr(id + 4, '-');

		ਜੇ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
ਪੂਰਾ:
	ul_unref_ਰਸਤਾ(pc);
	ਜੇ (uuid)
		*uuid = id;
	ਨਹੀਂਤਾਂ
		ਮੁਕਤ(id);
	ਵਾਪਸ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ਪੂਰਨ sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ਜੇ (sysfs_devno_ਮੰਜ਼ਿਲ_wholedisk(devno, ਨਿਰਾ, 0, &disk) != 0)
		ਵਾਪਸ -1;

	ਵਾਪਸ devno == disk;
}


ਪੂਰਨ sysfs_blkdev_scsi_get_hctl(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ *h, ਪੂਰਨ *c, ਪੂਰਨ *t, ਪੂਰਨ *l)
{
	ਅੱਖਰ buf[PATH_MAX], *hctl;
	ਬਣਤਰ sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_ਰਸਤਾ_get_dialect(pc);

	ਜੇ (!blk || blk->hctl_error)
		ਵਾਪਸ -EINVAL;
	ਜੇ (blk->has_hctl)
		ਜਾਓ ਪੂਰਾ;

	blk->hctl_error = 1;
	len = ul_ਰਸਤਾ_readlink(pc, buf, ਆਕਾਰ(buf), "device");
	ਜੇ (len < 0)
		ਵਾਪਸ len;

	hctl = strrchr(buf, '/');
	ਜੇ (!hctl)
		ਵਾਪਸ -1;
	hctl++;

	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_ਨਿਸ਼ਾਨਾ, &blk->scsi_lun) != 4)
		ਵਾਪਸ -1;

	blk->has_hctl = 1;
ਪੂਰਾ:
	ਜੇ (h)
		*h = blk->scsi_host;
	ਜੇ (c)
		*c = blk->scsi_channel;
	ਜੇ (t)
		*t = blk->scsi_ਨਿਸ਼ਾਨਾ;
	ਜੇ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ਵਾਪਸ 0;
}


ਸਥਿਰ ਅੱਖਰ *scsi_host_attribute_ਰਸਤਾ(
			ਬਣਤਰ ਰਸਤਾ_cxt *pc,
			ਅਚਲ ਅੱਖਰ *type,
			ਅੱਖਰ *buf,
			size_t bufsz,
			ਅਚਲ ਅੱਖਰ *attr)
{
	ਪੂਰਨ len;
	ਪੂਰਨ host;
	ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ;

	ਜੇ (sysfs_blkdev_scsi_get_hctl(pc, &host, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ))
		ਵਾਪਸ ਨਿਰਾ;

	ਅਗੇਤਰ = ul_ਰਸਤਾ_get_ਅਗੇਤਰ(pc);
	ਜੇ (!ਅਗੇਤਰ)
		ਅਗੇਤਰ = "";

	ਜੇ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				ਅਗੇਤਰ, ਰਸਤਾ_SYS_CLASS, type, host, attr);
	ਨਹੀਂਤਾਂ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				ਅਗੇਤਰ, ਰਸਤਾ_SYS_CLASS, type, host);

	ਵਾਪਸ (len < 0 || (size_t) len >= bufsz) ? ਨਿਰਾ : buf;
}

ਅੱਖਰ *sysfs_blkdev_scsi_host_strdup_attribute(ਬਣਤਰ ਰਸਤਾ_cxt *pc,
			ਅਚਲ ਅੱਖਰ *type, ਅਚਲ ਅੱਖਰ *attr)
{
	ਅੱਖਰ buf[1024];
	ਪੂਰਨ rc;
	FILE *f;

	ਜੇ (!attr || !type ||
	    !scsi_host_attribute_ਰਸਤਾ(pc, type, buf, ਆਕਾਰ(buf), attr))
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ਵਾਪਸ ਨਿਰਾ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ਵਾਪਸ rc == 1 ? strdup(buf) : ਨਿਰਾ;
}

ਪੂਰਨ sysfs_blkdev_scsi_host_is(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *type)
{
	ਅੱਖਰ buf[PATH_MAX];
	ਬਣਤਰ stat st;

	ਜੇ (!type || !scsi_host_attribute_ਰਸਤਾ(pc, type,
				buf, ਆਕਾਰ(buf), ਨਿਰਾ))
		ਵਾਪਸ 0;

	ਵਾਪਸ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ਸਥਿਰ ਅੱਖਰ *scsi_attribute_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc,
		ਅੱਖਰ *buf, size_t bufsz, ਅਚਲ ਅੱਖਰ *attr)
{
	ਪੂਰਨ len, h, c, t, l;
	ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ;

	ਜੇ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਅਗੇਤਰ = ul_ਰਸਤਾ_get_ਅਗੇਤਰ(pc);
	ਜੇ (!ਅਗੇਤਰ)
		ਅਗੇਤਰ = "";

	ਜੇ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				ਅਗੇਤਰ, ਰਸਤਾ_SYS_SCSI,
				h,c,t,l, attr);
	ਨਹੀਂਤਾਂ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				ਅਗੇਤਰ, ਰਸਤਾ_SYS_SCSI,
				h,c,t,l);
	ਵਾਪਸ (len < 0 || (size_t) len >= bufsz) ? ਨਿਰਾ : buf;
}

ਪੂਰਨ sysfs_blkdev_scsi_has_attribute(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *attr)
{
	ਅੱਖਰ ਰਸਤਾ[PATH_MAX];
	ਬਣਤਰ stat st;

	ਜੇ (!scsi_attribute_ਰਸਤਾ(pc, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ), attr))
		ਵਾਪਸ 0;

	ਵਾਪਸ stat(ਰਸਤਾ, &st) == 0;
}

ਪੂਰਨ sysfs_blkdev_scsi_ਰਸਤਾ_ਸ਼ਾਮਲ_ਹੈ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ)
{
	ਅੱਖਰ ਰਸਤਾ[PATH_MAX], linkc[PATH_MAX];
	ਬਣਤਰ stat st;
	ssize_t len;

	ਜੇ (!scsi_attribute_ਰਸਤਾ(pc, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ), ਨਿਰਾ))
		ਵਾਪਸ 0;

	ਜੇ (stat(ਰਸਤਾ, &st) != 0)
		ਵਾਪਸ 0;

	len = readlink(ਰਸਤਾ, linkc, ਆਕਾਰ(linkc) - 1);
	ਜੇ (len < 0)
		ਵਾਪਸ 0;

	linkc[len] = '\0';
	ਵਾਪਸ ਲੜੀ_ਖੋਜ(linkc, ਨਮੂਨਾ) != ਨਿਰਾ;
}

ਸਥਿਰ dev_t read_devno(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	FILE *f;
	ਪੂਰਨ maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(ਰਸਤਾ, "r" UL_CLOEXECSTR);
	ਜੇ (!f)
		ਵਾਪਸ 0;

	ਜੇ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ਵਾਪਸ dev;
}

ਪੂਰਨ sysfs_devname_is_hidden(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਅਚਲ ਅੱਖਰ *name)
{
	ਅੱਖਰ buf[PATH_MAX];
	ਪੂਰਨ rc = 0, hidden = 0, len;
	FILE *f;

	ਜੇ (ਲੜੀ_ਤੁਲਨਾ("/dev/", name, 5) == 0)
		ਵਾਪਸ 0;

	ਜੇ (!ਅਗੇਤਰ)
		ਅਗੇਤਰ = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, ਆਕਾਰ(buf),
			"%s" ਰਸਤਾ_SYS_ਖੰਡ "/%s/hidden",
			ਅਗੇਤਰ, name);

	ਜੇ (len < 0 || (size_t) len + 1 > ਆਕਾਰ(buf))
		ਵਾਪਸ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ਜੇ (!f)
		ਵਾਪਸ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ਵਾਪਸ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮਾਪਾ)
{
	ਅੱਖਰ buf[PATH_MAX];
	ਅੱਖਰ *_name = ਨਿਰਾ, *ਮਾਪਾ_2 = ਨਿਰਾ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ਪੂਰਨ len;

	ਜੇ (!ਅਗੇਤਰ)
		ਅਗੇਤਰ = "";

	ਪ੍ਰਤਿਜ੍ਞਾ(name);

	ਜੇ (ਲੜੀ_ਤੁਲਨਾ("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ਬਣਤਰ stat st;

		ਜੇ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			ਜਾਓ ਪੂਰਾ;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ਜੇ (!_name)
		ਜਾਓ ਪੂਰਾ;
	sysfs_devname_dev_ਮੰਜ਼ਿਲ_sys(_name);

	ਜੇ (ਮਾਪਾ) {
		ਮਾਪਾ_2 = strdup(ਮਾਪਾ);
		ਜੇ (!ਮਾਪਾ_2)
			ਜਾਓ ਪੂਰਾ;
	}

	ਜੇ (ਮਾਪਾ && ਲੜੀ_ਤੁਲਨਾ("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_ਮੰਜ਼ਿਲ_sys(ਮਾਪਾ_2);
		len = snprintf(buf, ਆਕਾਰ(buf),
				"%s" ਰਸਤਾ_SYS_ਖੰਡ "/%s/%s/dev",
				ਅਗੇਤਰ,	ਮਾਪਾ_2, _name);
		ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
			ਜਾਓ ਪੂਰਾ;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		ਜਾਓ ਪੂਰਾ;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, ਆਕਾਰ(buf),
			"%s" ਰਸਤਾ_SYS_ਖੰਡ "/%s/dev",
			ਅਗੇਤਰ, _name);
	ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
		ਜਾਓ ਪੂਰਾ;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ਜੇ (!dev && ਮਾਪਾ && ul_startswith(name, ਮਾਪਾ)) {
		len = snprintf(buf, ਆਕਾਰ(buf),
				"%s" ਰਸਤਾ_SYS_ਖੰਡ "/%s/%s/dev",
				ਅਗੇਤਰ, ਮਾਪਾ_2, _name);
		ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
			ਜਾਓ ਪੂਰਾ;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ਜੇ (!dev) {
		len = snprintf(buf, ਆਕਾਰ(buf),
				"%s" ਰਸਤਾ_SYS_ਖੰਡ "/%s/device/dev",
				ਅਗੇਤਰ, _name);
		ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
			ਜਾਓ ਪੂਰਾ;
		dev = read_devno(buf);
	}
ਪੂਰਾ:
	ਮੁਕਤ(_name);
	ਮੁਕਤ(ਮਾਪਾ_2);
	ਵਾਪਸ dev;
}

dev_t sysfs_devname_ਮੰਜ਼ਿਲ_devno(ਅਚਲ ਅੱਖਰ *name)
{
	ਵਾਪਸ __sysfs_devname_to_devno(ਨਿਰਾ, name, ਨਿਰਾ);
}

ਅੱਖਰ *sysfs_blkdev_get_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਅਚਲ ਅੱਖਰ *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	ਅੱਖਰ *res = ਨਿਰਾ;
	size_t sz;
	ਬਣਤਰ stat st;

	ਜੇ (!name)
		ਜਾਓ ਪੂਰਾ;

	sz = ਲੜੀ_ਲੰਬਾਈ(name);
	ਜੇ (sz + ਆਕਾਰ("/dev/") > bufsiz)
		ਜਾਓ ਪੂਰਾ;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, "/dev/", 5);

	ਜੇ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
ਪੂਰਾ:
	ਵਾਪਸ res;
}

dev_t sysfs_blkdev_get_devno(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਵਾਪਸ ((ਬਣਤਰ sysfs_blkdev *) ul_ਰਸਤਾ_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
ਅੱਖਰ *sysfs_devno_ਮੰਜ਼ਿਲ_devpath(dev_t devno, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (pc) {
		res = sysfs_blkdev_get_ਰਸਤਾ(pc, buf, bufsiz);
		ul_unref_ਰਸਤਾ(pc);
	}
	ਵਾਪਸ res;
}

ਅੱਖਰ *sysfs_devno_ਮੰਜ਼ਿਲ_devname(dev_t devno, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਅੱਖਰ *res = ਨਿਰਾ;

	ਜੇ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_ਰਸਤਾ(pc);
	}
	ਵਾਪਸ res;
}

ਪੂਰਨ sysfs_devno_ਗਿਣਤੀ_partitions(dev_t devno)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਪੂਰਨ n = 0;

	ਜੇ (pc) {
		ਅੱਖਰ buf[PATH_MAX + 1];
		ਅੱਖਰ *name = sysfs_blkdev_get_name(pc, buf, ਆਕਾਰ(buf));

		n = sysfs_blkdev_ਗਿਣਤੀ_partitions(pc, name);
		ul_unref_ਰਸਤਾ(pc);
	}
	ਵਾਪਸ n;
}

ਅੱਖਰ *sysfs_chrdev_devno_ਮੰਜ਼ਿਲ_devname(dev_t devno, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਅੱਖਰ link[PATH_MAX];
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	ਅੱਖਰ *name;
	ssize_t	sz;

	pc = ul_new_ਰਸਤਾ(ਰਸਤਾ_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ਜੇ (!pc)
		ਵਾਪਸ ਨਿਰਾ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_ਰਸਤਾ_readlink(pc, link, ਆਕਾਰ(link), ਨਿਰਾ);
	ul_unref_ਰਸਤਾ(pc);

	ਜੇ (sz < 0)
		ਵਾਪਸ ਨਿਰਾ;

	name = strrchr(link, '/');
	ਜੇ (!name)
		ਵਾਪਸ ਨਿਰਾ;

	name++;
	sz = ਲੜੀ_ਲੰਬਾਈ(name);
	ਜੇ ((size_t) sz + 1 > bufsiz)
		ਵਾਪਸ ਨਿਰਾ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(buf, name, sz + 1);
	sysfs_devname_sys_ਮੰਜ਼ਿਲ_dev(buf);
	ਵਾਪਸ buf;

}

ਗਿਣਤੀ sysfs_byteorder sysfs_get_byteorder(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪੂਰਨ rc;
	ਅੱਖਰ buf[BUFSIZ];
	ਗਿਣਤੀ sysfs_byteorder ret;

	rc = ul_ਰਸਤਾ_read_ਬਫਰ(pc, buf, ਆਕਾਰ(buf), ਰਸਤਾ_SYS_CPU_BYTEORDER);
	ਜੇ (rc < 0)
		ਜਾਓ unknown;

	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(buf, "little", ਆਕਾਰ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		ਜਾਓ ਬਾਹਰਲਾ;
	} ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(buf, "big", ਆਕਾਰ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

unknown:
#ਜੇ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#ਜਾਂਜੇ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ Unknown byte ਕ੍ਰਮ
#ਜੇ_ਅੰਤ

ਬਾਹਰਲਾ:
	ਵਾਪਸ ret;
}

ਪੂਰਨ sysfs_get_ਪਤਾ_ਬਿੱਟ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪੂਰਨ rc;
	ਪੂਰਨ ਪਤਾ_ਬਿੱਟ;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ_SYS_ਪਤਾ_ਬਿੱਟ, "%d", &ਪਤਾ_ਬਿੱਟ);
	ਜੇ (rc < 0)
		ਵਾਪਸ rc;
	ਜੇ (ਪਤਾ_ਬਿੱਟ < 0)
		ਵਾਪਸ -EINVAL;
	ਵਾਪਸ ਪਤਾ_ਬਿੱਟ;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_SYSFS
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc;
	ਅੱਖਰ *devname;
	dev_t devno, disk_devno;
	ਅੱਖਰ ਰਸਤਾ[PATH_MAX], *sub, *chain;
	ਅੱਖਰ diskname[32];
	ਪੂਰਨ i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ਜੇ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_ਮੰਜ਼ਿਲ_devno(devname);

	ਜੇ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	ਲਿਖੋ("non-context:\n");
	ਲਿਖੋ(" DEVNO:   %u (%u:%u)\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) devno, major(devno), minor(devno));
	ਲਿਖੋ(" DEVNAME: %s\n", sysfs_devno_ਮੰਜ਼ਿਲ_devname(devno, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ)));
	ਲਿਖੋ(" DEVPATH: %s\n", sysfs_devno_ਮੰਜ਼ਿਲ_devpath(devno, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ)));

	sysfs_devno_ਮੰਜ਼ਿਲ_wholedisk(devno, diskname, ਆਕਾਰ(diskname), &disk_devno);
	ਲਿਖੋ(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) disk_devno, major(disk_devno), minor(disk_devno));
	ਲਿਖੋ(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_ਰਸਤਾ(devno, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (!pc)
		ਜਾਓ ਪੂਰਾ;

	ਲਿਖੋ("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	ਲਿਖੋ(" DEVNO:   %u (%u:%u)\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) devno, major(devno), minor(devno));
	ਲਿਖੋ(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ)));
	ਲਿਖੋ(" DEVPATH: %s\n", sysfs_blkdev_get_ਰਸਤਾ(pc, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ)));

	sysfs_devno_ਮੰਜ਼ਿਲ_wholedisk(devno, diskname, ਆਕਾਰ(diskname), &disk_devno);
	ਲਿਖੋ(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) disk_devno, major(disk_devno), minor(disk_devno));
	ਲਿਖੋ(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_ਰਸਤਾ_access(pc, F_OK, "partition") == 0;
	ਲਿਖੋ(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ਜੇ (is_part && disk_devno) {
		ਬਣਤਰ ਰਸਤਾ_cxt *disk_pc =  ul_new_sysfs_ਰਸਤਾ(disk_devno, ਨਿਰਾ, ਨਿਰਾ);
		sysfs_blkdev_ਸੈੱਟ_ਮਾਪਾ(pc, disk_pc);

		ul_unref_ਰਸਤਾ(disk_pc);
	}

	ਲਿਖੋ(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	ਲਿਖੋ(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	ਲਿਖੋ(" SLAVES: %d\n", ul_ਰਸਤਾ_ਗਿਣਤੀ_dirents(pc, "slaves"));

	ਜੇ (!is_part) {
		ਲਿਖੋ("First 5 partitions:\n");
		ਲਈ (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_ਮੰਜ਼ਿਲ_devno(pc, i);
			ਜੇ (dev)
				ਲਿਖੋ("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ਜੇ (ul_ਰਸਤਾ_read_u64(pc, &u64, "size") != 0)
		ਲਿਖੋ(" (!) read SIZE failed\n");
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ(" SIZE: %"PRIu64"\n", u64);

	ਜੇ (ul_ਰਸਤਾ_read_s32(pc, &i, "queue/hw_sector_size"))
		ਲਿਖੋ(" (!) read SECTOR failed\n");
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, ਰਸਤਾ, ਆਕਾਰ(ਰਸਤਾ));
	ਲਿਖੋ(" SUBSYSTEMS:\n");

	ਜਦੋਂ (chain && sysfs_blkdev_ਅਗਲਾ_subsystem(pc, chain, &sub) == 0) {
		ਲਿਖੋ("\t%s\n", sub);
		ਮੁਕਤ(sub);
	}

	rc = EXIT_SUCCESS;
ਪੂਰਾ:
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_SYSFS */
