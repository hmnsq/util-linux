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

ಸ್ಥಿರ ಶೂನ್ಯ sysfs_blkdev_deinit_path(ಸಂರಚನಾ path_cxt *pc);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ  sysfs_blkdev_enoent_redirect(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

ಶೂನ್ಯ ul_sysfs_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (ulsysfs_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ಸಂರಚನಾ path_cxt *ul_new_sysfs_path(dev_t devno, ಸಂರಚನಾ path_cxt *parent, ಧ್ರುವ ವರ್ಣ *prefix)
{
	ಸಂರಚನಾ path_cxt *pc = ul_new_path(ರಿಕ್ತ);

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (prefix)
		ul_path_set_prefix(pc, prefix);

	ಯದಿ (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ಪ್ರತ್ಯಾಗಮ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_init_path(ಸಂರಚನಾ path_cxt *pc, dev_t devno, ಸಂರಚನಾ path_cxt *parent)
{
	ಸಂರಚನಾ sysfs_blkdev *blk;
	ಪೂರ್ಣಾಙ್ಕ rc;
	ವರ್ಣ buf[ಪರಿಮಾಣ(_PATH_SYS_DEVBLOCK)
		 + ಪರಿಮಾಣ(stringify_ಮೌಲ್ಯ(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, ಪರಿಮಾಣ(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	ಯದಿ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ sysfs_blkdev));
		ಯದಿ (!blk)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ sysfs_blkdev_deinit_path(ಸಂರಚನಾ path_cxt *pc)
{
	ಸಂರಚನಾ sysfs_blkdev *blk;

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	ಯದಿ (!blk)
		ಪ್ರತ್ಯಾಗಮ;

	ul_unref_path(blk->parent);
	ಮುಕ್ತ(blk);

	ul_path_set_dialect(pc, ರಿಕ್ತ, ರಿಕ್ತ);
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_set_parent(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ path_cxt *parent)
{
	ಸಂರಚನಾ sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ಯದಿ (!pc || !blk)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = ರಿಕ್ತ;
	}

	ಯದಿ (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} ಅನ್ಯಥಾ
		blk->parent = ರಿಕ್ತ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸಂರಚನಾ path_cxt *sysfs_blkdev_get_parent(ಸಂರಚನಾ path_cxt *pc)
{
	ಸಂರಚನಾ sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ಪ್ರತ್ಯಾಗಮ blk ? blk->parent : ರಿಕ್ತ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_enoent_redirect(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, ಪೂರ್ಣಾಙ್ಕ *dirfd)
{
	ಸಂರಚನಾ sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ಯದಿ (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		ಯದಿ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}
	ಪ್ರತ್ಯಾಗಮ 1;	/* no redirect */
}

ವರ್ಣ *sysfs_blkdev_get_name(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsiz)
{
	ವರ್ಣ link[PATH_MAX];
	ವರ್ಣ *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, ಪರಿಮಾಣ(link), ರಿಕ್ತ);
	ಯದಿ (sz < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	name = strrchr(link, '/');
	ಯದಿ (!name)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	name++;
	sz = strlen(name);
	ಯದಿ ((size_t) sz + 1 > bufsiz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ಪ್ರತ್ಯಾಗಮ buf;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_is_partition_dirent(DIR *dir, ಸಂರಚನಾ dirent *d, ಧ್ರುವ ವರ್ಣ *parent_name)
{
	ವರ್ಣ path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	ಯದಿ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif
	size_t len = 0;

	ಯದಿ (parent_name) {
		ಧ್ರುವ ವರ್ಣ *p = parent_name;

		/* /dev/sda --> "sda" */
		ಯದಿ (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			ಯದಿ (!p)
				ಪ್ರತ್ಯಾಗಮ 0;
			p++;
		}

		len = strlen(p);
		ಯದಿ ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	ಯದಿ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ಪ್ರತ್ಯಾಗಮ ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, ಪರಿಮಾಣ(path), "%s/start", d->d_name);

	ಪ್ರತ್ಯಾಗಮ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_ಎಣಿಕೆ_partitions(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *devname)
{
	DIR *dir;
	ಸಂರಚನಾ dirent *d;
	ಪೂರ್ಣಾಙ್ಕ r = 0;

	dir = ul_path_opendir(pc, ರಿಕ್ತ);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್ ((d = xreaddir(dir))) {
		ಯದಿ (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ partno)
{
	DIR *dir;
	ಸಂರಚನಾ dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, ರಿಕ್ತ);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್ ((d = xreaddir(dir))) {
		ಪೂರ್ಣಾಙ್ಕ n;

		ಯದಿ (!sysfs_blkdev_is_partition_dirent(dir, d, ರಿಕ್ತ))
			ಅನುವರ್ತ;

		ಯದಿ (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			ಅನುವರ್ತ;

		ಯದಿ (n == partno) {
			ಯದಿ (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ಭಙ್ಗ;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ಪೂರ್ಣಾಙ್ಕ) partno, (ಪೂರ್ಣಾಙ್ಕ) devno));
	ಪ್ರತ್ಯಾಗಮ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
ವರ್ಣ *sysfs_blkdev_get_slave(ಸಂರಚನಾ path_cxt *pc)
{
	DIR *dir;
	ಸಂರಚನಾ dirent *d;
	ವರ್ಣ *name = ರಿಕ್ತ;

	dir = ul_path_opendir(pc, "slaves");
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯಾವತ್ ((d = xreaddir(dir))) {
		ಯದಿ (name)
			ಗಚ್ಛ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ name;
err:
	ಮುಕ್ತ(name);
	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


#define SUBSYSTEM_LINKNAME	"/subsystem"

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
ಸ್ಥಿರ ವರ್ಣ *get_subsystem(ವರ್ಣ *chain, ವರ್ಣ *buf, size_t bufsz)
{
	size_t len;
	ವರ್ಣ *p;

	ಯದಿ (!chain || !*chain)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	len = strlen(chain);
	ಯದಿ (len + ಪರಿಮಾಣ(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಕುರು {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, ಪರಿಮಾಣ(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ಯದಿ (p) {
			*p = '\0';
			len = p - chain;
		}

		ಯದಿ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ಪ್ರತ್ಯಾಗಮ basename(buf);
		}

	} ಯಾವತ್ (p);

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
ವರ್ಣ *sysfs_blkdev_get_devchain(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ಸಂರಚನಾ ul_buffer tmp = UL_INIT_BUFFER;
	ಧ್ರುವ ವರ್ಣ *p;
	ವರ್ಣ *res = ರಿಕ್ತ;

	ssz = ul_path_readlink(pc, buf, bufsz, ರಿಕ್ತ);
	ಯದಿ (ssz <= 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, ರಿಕ್ತ);
	ಯದಿ (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ಪ್ರತ್ಯಾಗಮ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_next_subsystem(ಸಂರಚನಾ path_cxt *pc __attribute__((unused)),
			 ವರ್ಣ *devchain, ವರ್ಣ **subsys)
{
	ವರ್ಣ subbuf[PATH_MAX];
	ವರ್ಣ *sub;

	ಯದಿ (!subsys || !devchain)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	*subsys = ರಿಕ್ತ;

	ಯಾವತ್ ((sub = get_subsystem(devchain, subbuf, ಪರಿಮಾಣ(subbuf)))) {
		*subsys = strdup(sub);
		ಯದಿ (!*subsys)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sysfs_devchain_is_removable(ವರ್ಣ *chain)
{
	size_t len;
	ವರ್ಣ buf[20];
	ವರ್ಣ *p;

	ಯದಿ (!chain || !*chain)
		ಪ್ರತ್ಯಾಗಮ 0;

	len = strlen(chain);
	ಯದಿ (len + ಪರಿಮಾಣ(REMOVABLE_FILENAME) > PATH_MAX)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಕುರು {
		ಪೂರ್ಣಾಙ್ಕ fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, ಪರಿಮಾಣ(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ಯದಿ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ಭಙ್ಗ;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ಯದಿ (fd != -1) {
			rc = ul_read_all(fd, buf, ಪರಿಮಾಣ(buf));
			close(fd);

			ಯದಿ (rc > 0) {
				ಯದಿ (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ಪ್ರತ್ಯಾಗಮ 0;
				} ಅನ್ಯಥಾ ಯದಿ (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ಪ್ರತ್ಯಾಗಮ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ಯದಿ (p) {
			*p = '\0';
			len = p - chain;
		}

	} ಯಾವತ್ (p);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_is_hotpluggable(ಸಂರಚನಾ path_cxt *pc)
{
	ವರ್ಣ buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, ಪರಿಮಾಣ(buf));
	ಪ್ರತ್ಯಾಗಮ sysfs_devchain_is_removable(chain);
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_is_removable(ಸಂರಚನಾ path_cxt *pc)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ಯದಿ (ul_path_read_s32(pc, &rc, "removable") == 0)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ get_dm_wholedisk(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *diskname,
                            size_t len, dev_t *diskdevno)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;
	ವರ್ಣ *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ಯದಿ (!name)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (diskname && len)
		xstrncpy(diskname, name, len);

	ಯದಿ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ರಿಕ್ತ);
		ಯದಿ (!*diskdevno)
			rc = -1;
	}

	ಮುಕ್ತ(name);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_get_wholedisk(	ಸಂರಚನಾ path_cxt *pc,
				ವರ್ಣ *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ಪೂರ್ಣಾಙ್ಕ is_part = 0;

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	ಯದಿ (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		ವರ್ಣ *uuid = ರಿಕ್ತ, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : ರಿಕ್ತ;

		ಯದಿ (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		ಮುಕ್ತ(uuid);

		ಯದಿ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			ಗಚ್ಛ done;

		is_part = 0;
	}

	ಯದಿ (!is_part) {
		/*
		 * unpartitioned device
		 */
		ಯದಿ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			ಗಚ್ಛ err;
		ಯದಿ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ಅನ್ಯಥಾ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		ವರ್ಣ linkpath[PATH_MAX];
		ವರ್ಣ *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, ಪರಿಮಾಣ(linkpath), ರಿಕ್ತ);
		ಯದಿ (linklen < 0)
			ಗಚ್ಛ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		ಯದಿ (!name)
			ಗಚ್ಛ err;

		sysfs_devname_sys_to_dev(name);
		ಯದಿ (diskname && len)
			xstrncpy(diskname, name, len);

		ಯದಿ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ರಿಕ್ತ);
			ಯದಿ (!*diskdevno)
				ಗಚ್ಛ err;
		}
	}

done:
	ಪ್ರತ್ಯಾಗಮ 0;
err:
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_devno_to_wholedisk(dev_t devno, ವರ್ಣ *diskname,
		             size_t len, dev_t *diskdevno)
{
	ಸಂರಚನಾ path_cxt *pc;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (!devno)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ rc;
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
ಪೂರ್ಣಾಙ್ಕ sysfs_devno_is_dm_hidden(dev_t devno, ವರ್ಣ **uuid,
			     ಧ್ರುವ ಸಂರಚನಾ ul_vfs_ops *vfs)
{
	ಸಂರಚನಾ path_cxt *pc = ರಿಕ್ತ;
	ವರ್ಣ *id = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (!pc)
		ಗಚ್ಛ done;
	ul_path_refer_vfs(pc, vfs);
	ಯದಿ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ಗಚ್ಛ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ಯದಿ (strncmp(id, "LVM-", 4) == 0) {
		ವರ್ಣ *p = strrchr(id + 4, '-');

		ಯದಿ (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	ಯದಿ (uuid)
		*uuid = id;
	ಅನ್ಯಥಾ
		ಮುಕ್ತ(id);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ಪೂರ್ಣಾಙ್ಕ sysfs_devno_is_dm_private(dev_t devno, ವರ್ಣ **uuid,
			      ಧ್ರುವ ಸಂರಚನಾ ul_vfs_ops *vfs)
{
	ಸಂರಚನಾ path_cxt *pc = ರಿಕ್ತ;
	ವರ್ಣ *id = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (!pc)
		ಗಚ್ಛ done;
	ul_path_refer_vfs(pc, vfs);
	ಯದಿ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ಗಚ್ಛ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ಯದಿ (strncmp(id, "LVM-", 4) == 0) {
		ವರ್ಣ *p = strrchr(id + 4, '-');

		ಯದಿ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ಅನ್ಯಥಾ ಯದಿ (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	ಯದಿ (uuid)
		*uuid = id;
	ಅನ್ಯಥಾ
		ಮುಕ್ತ(id);
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ಪೂರ್ಣಾಙ್ಕ sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ಯದಿ (sysfs_devno_to_wholedisk(devno, ರಿಕ್ತ, 0, &disk) != 0)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತ್ಯಾಗಮ devno == disk;
}


ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_scsi_get_hctl(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ *h, ಪೂರ್ಣಾಙ್ಕ *c, ಪೂರ್ಣಾಙ್ಕ *t, ಪೂರ್ಣಾಙ್ಕ *l)
{
	ವರ್ಣ buf[PATH_MAX], *hctl;
	ಸಂರಚನಾ sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	ಯದಿ (!blk || blk->hctl_error)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (blk->has_hctl)
		ಗಚ್ಛ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, ಪರಿಮಾಣ(buf), "device");
	ಯದಿ (len < 0)
		ಪ್ರತ್ಯಾಗಮ len;

	hctl = strrchr(buf, '/');
	ಯದಿ (!hctl)
		ಪ್ರತ್ಯಾಗಮ -1;
	hctl++;

	ಯದಿ (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ಪ್ರತ್ಯಾಗಮ -1;

	blk->has_hctl = 1;
done:
	ಯದಿ (h)
		*h = blk->scsi_host;
	ಯದಿ (c)
		*c = blk->scsi_channel;
	ಯದಿ (t)
		*t = blk->scsi_target;
	ಯದಿ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ವರ್ಣ *scsi_host_attribute_path(
			ಸಂರಚನಾ path_cxt *pc,
			ಧ್ರುವ ವರ್ಣ *type,
			ವರ್ಣ *buf,
			size_t bufsz,
			ಧ್ರುವ ವರ್ಣ *attr)
{
	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ host;
	ಧ್ರುವ ವರ್ಣ *prefix;

	ಯದಿ (sysfs_blkdev_scsi_get_hctl(pc, &host, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	prefix = ul_path_get_prefix(pc);
	ಯದಿ (!prefix)
		prefix = "";

	ಯದಿ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	ಅನ್ಯಥಾ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ಪ್ರತ್ಯಾಗಮ (len < 0 || (size_t) len >= bufsz) ? ರಿಕ್ತ : buf;
}

ವರ್ಣ *sysfs_blkdev_scsi_host_strdup_attribute(ಸಂರಚನಾ path_cxt *pc,
			ಧ್ರುವ ವರ್ಣ *type, ಧ್ರುವ ವರ್ಣ *attr)
{
	ವರ್ಣ buf[1024];
	ಪೂರ್ಣಾಙ್ಕ rc;
	FILE *f;

	ಯದಿ (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, ಪರಿಮಾಣ(buf), attr))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ಪ್ರತ್ಯಾಗಮ rc == 1 ? strdup(buf) : ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_scsi_host_is(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *type)
{
	ವರ್ಣ buf[PATH_MAX];
	ಸಂರಚನಾ stat st;

	ಯದಿ (!type || !scsi_host_attribute_path(pc, type,
				buf, ಪರಿಮಾಣ(buf), ರಿಕ್ತ))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ಸ್ಥಿರ ವರ್ಣ *scsi_attribute_path(ಸಂರಚನಾ path_cxt *pc,
		ವರ್ಣ *buf, size_t bufsz, ಧ್ರುವ ವರ್ಣ *attr)
{
	ಪೂರ್ಣಾಙ್ಕ len, h, c, t, l;
	ಧ್ರುವ ವರ್ಣ *prefix;

	ಯದಿ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	prefix = ul_path_get_prefix(pc);
	ಯದಿ (!prefix)
		prefix = "";

	ಯದಿ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	ಅನ್ಯಥಾ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ಪ್ರತ್ಯಾಗಮ (len < 0 || (size_t) len >= bufsz) ? ರಿಕ್ತ : buf;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_scsi_has_attribute(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *attr)
{
	ವರ್ಣ path[PATH_MAX];
	ಸಂರಚನಾ stat st;

	ಯದಿ (!scsi_attribute_path(pc, path, ಪರಿಮಾಣ(path), attr))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ stat(path, &st) == 0;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_blkdev_scsi_path_contains(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *pattern)
{
	ವರ್ಣ path[PATH_MAX], linkc[PATH_MAX];
	ಸಂರಚನಾ stat st;
	ssize_t len;

	ಯದಿ (!scsi_attribute_path(pc, path, ಪರಿಮಾಣ(path), ರಿಕ್ತ))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (stat(path, &st) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	len = readlink(path, linkc, ಪರಿಮಾಣ(linkc) - 1);
	ಯದಿ (len < 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	linkc[len] = '\0';
	ಪ್ರತ್ಯಾಗಮ strstr(linkc, pattern) != ರಿಕ್ತ;
}

ಸ್ಥಿರ dev_t read_devno(ಧ್ರುವ ವರ್ಣ *path)
{
	FILE *f;
	ಪೂರ್ಣಾಙ್ಕ maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ಪ್ರತ್ಯಾಗಮ dev;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_devname_is_hidden(ಧ್ರುವ ವರ್ಣ *prefix, ಧ್ರುವ ವರ್ಣ *name)
{
	ವರ್ಣ buf[PATH_MAX];
	ಪೂರ್ಣಾಙ್ಕ rc = 0, hidden = 0, len;
	FILE *f;

	ಯದಿ (strncmp("/dev/", name, 5) == 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, ಪರಿಮಾಣ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	ಯದಿ (len < 0 || (size_t) len + 1 > ಪರಿಮಾಣ(buf))
		ಪ್ರತ್ಯಾಗಮ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ಪ್ರತ್ಯಾಗಮ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ಧ್ರುವ ವರ್ಣ *prefix, ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *parent)
{
	ವರ್ಣ buf[PATH_MAX];
	ವರ್ಣ *_name = ರಿಕ್ತ, *_parent = ರಿಕ್ತ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ಪೂರ್ಣಾಙ್ಕ len;

	ಯದಿ (!prefix)
		prefix = "";

	ಪ್ರತಿಜ್ಞಾ(name);

	ಯದಿ (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ಸಂರಚನಾ stat st;

		ಯದಿ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			ಗಚ್ಛ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ಯದಿ (!_name)
		ಗಚ್ಛ done;
	sysfs_devname_dev_to_sys(_name);

	ಯದಿ (parent) {
		_parent = strdup(parent);
		ಯದಿ (!_parent)
			ಗಚ್ಛ done;
	}

	ಯದಿ (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, ಪರಿಮಾಣ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
			ಗಚ್ಛ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		ಗಚ್ಛ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, ಪರಿಮಾಣ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
		ಗಚ್ಛ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ಯದಿ (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, ಪರಿಮಾಣ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
			ಗಚ್ಛ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ಯದಿ (!dev) {
		len = snprintf(buf, ಪರಿಮಾಣ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
			ಗಚ್ಛ done;
		dev = read_devno(buf);
	}
done:
	ಮುಕ್ತ(_name);
	ಮುಕ್ತ(_parent);
	ಪ್ರತ್ಯಾಗಮ dev;
}

dev_t sysfs_devname_to_devno(ಧ್ರುವ ವರ್ಣ *name)
{
	ಪ್ರತ್ಯಾಗಮ __sysfs_devname_to_devno(ರಿಕ್ತ, name, ರಿಕ್ತ);
}

ವರ್ಣ *sysfs_blkdev_get_path(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsiz)
{
	ಧ್ರುವ ವರ್ಣ *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	ವರ್ಣ *res = ರಿಕ್ತ;
	size_t sz;
	ಸಂರಚನಾ stat st;

	ಯದಿ (!name)
		ಗಚ್ಛ done;

	sz = strlen(name);
	ಯದಿ (sz + ಪರಿಮಾಣ("/dev/") > bufsiz)
		ಗಚ್ಛ done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	ಯದಿ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ಪ್ರತ್ಯಾಗಮ res;
}

dev_t sysfs_blkdev_get_devno(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತ್ಯಾಗಮ ((ಸಂರಚನಾ sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
ವರ್ಣ *sysfs_devno_to_devpath(dev_t devno, ವರ್ಣ *buf, size_t bufsiz)
{
	ಸಂರಚನಾ path_cxt *pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ಪ್ರತ್ಯಾಗಮ res;
}

ವರ್ಣ *sysfs_devno_to_devname(dev_t devno, ವರ್ಣ *buf, size_t bufsiz)
{
	ಸಂರಚನಾ path_cxt *pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ವರ್ಣ *res = ರಿಕ್ತ;

	ಯದಿ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ಪ್ರತ್ಯಾಗಮ res;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_devno_ಎಣಿಕೆ_partitions(dev_t devno)
{
	ಸಂರಚನಾ path_cxt *pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ಪೂರ್ಣಾಙ್ಕ n = 0;

	ಯದಿ (pc) {
		ವರ್ಣ buf[PATH_MAX + 1];
		ವರ್ಣ *name = sysfs_blkdev_get_name(pc, buf, ಪರಿಮಾಣ(buf));

		n = sysfs_blkdev_ಎಣಿಕೆ_partitions(pc, name);
		ul_unref_path(pc);
	}
	ಪ್ರತ್ಯಾಗಮ n;
}

ವರ್ಣ *sysfs_chrdev_devno_to_devname(dev_t devno, ವರ್ಣ *buf, size_t bufsiz)
{
	ವರ್ಣ link[PATH_MAX];
	ಸಂರಚನಾ path_cxt *pc;
	ವರ್ಣ *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, ಪರಿಮಾಣ(link), ರಿಕ್ತ);
	ul_unref_path(pc);

	ಯದಿ (sz < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	name = strrchr(link, '/');
	ಯದಿ (!name)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	name++;
	sz = strlen(name);
	ಯದಿ ((size_t) sz + 1 > bufsiz)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ಪ್ರತ್ಯಾಗಮ buf;

}

ಗಣನಾ sysfs_byteorder sysfs_get_byteorder(ಸಂರಚನಾ path_cxt *pc)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ವರ್ಣ buf[BUFSIZ];
	ಗಣನಾ sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, ಪರಿಮಾಣ(buf), _PATH_SYS_CPU_BYTEORDER);
	ಯದಿ (rc < 0)
		ಗಚ್ಛ unknown;

	ಯದಿ (strncmp(buf, "little", ಪರಿಮಾಣ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		ಗಚ್ಛ out;
	} ಅನ್ಯಥಾ ಯದಿ (strncmp(buf, "big", ಪರಿಮಾಣ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		ಗಚ್ಛ out;
	}

unknown:
#ಯದಿ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#ನೋಚೇದ್ಯದಿ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ಅನ್ಯಥಾ
#error Unknown byte order
#endif

out:
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ sysfs_get_address_bits(ಸಂರಚನಾ path_cxt *pc)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಪೂರ್ಣಾಙ್ಕ address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ rc;
	ಯದಿ (address_bits < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಪ್ರತ್ಯಾಗಮ address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಸಂರಚನಾ path_cxt *pc;
	ವರ್ಣ *devname;
	dev_t devno, disk_devno;
	ವರ್ಣ path[PATH_MAX], *sub, *chain;
	ವರ್ಣ diskname[32];
	ಪೂರ್ಣಾಙ್ಕ i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ಯದಿ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	ಯದಿ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ಪೂರ್ಣಾಙ್ಕ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, ಪರಿಮಾಣ(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, ಪರಿಮಾಣ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, ಪರಿಮಾಣ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned ಪೂರ್ಣಾಙ್ಕ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (!pc)
		ಗಚ್ಛ done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ಪೂರ್ಣಾಙ್ಕ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, ಪರಿಮಾಣ(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, ಪರಿಮಾಣ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, ಪರಿಮಾಣ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned ಪೂರ್ಣಾಙ್ಕ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ಯದಿ (is_part && disk_devno) {
		ಸಂರಚನಾ path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, ರಿಕ್ತ, ರಿಕ್ತ);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_ಎಣಿಕೆ_dirents(pc, "slaves"));

	ಯದಿ (!is_part) {
		printf("First 5 partitions:\n");
		ಪ್ರತಿ (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			ಯದಿ (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ಯದಿ (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	ಅನ್ಯಥಾ
		printf(" SIZE: %"PRIu64"\n", u64);

	ಯದಿ (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	ಅನ್ಯಥಾ
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, ಪರಿಮಾಣ(path));
	printf(" SUBSYSTEMS:\n");

	ಯಾವತ್ (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		ಮುಕ್ತ(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
