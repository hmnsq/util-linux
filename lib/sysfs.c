/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <libgen.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "sysfs.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "buffer.h"

স্থির শূন্য sysfs_blkdev_deinit_path(কাঠামো path_cxt *pc);
স্থির পূর্ণসংখ্যা  sysfs_blkdev_enoent_redirect(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

শূন্য ul_sysfs_init_debug(শূন্য)
{
	যদি (ulsysfs_debug_mask)
		ফেরত;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

কাঠামো path_cxt *ul_new_sysfs_path(dev_t devno, কাঠামো path_cxt *parent, ধ্রুবক অক্ষর *prefix)
{
	কাঠামো path_cxt *pc = ul_new_path(ফাঁকা);

	যদি (!pc)
		ফেরত ফাঁকা;
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ফেরত ফাঁকা;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ফেরত pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
পূর্ণসংখ্যা sysfs_blkdev_init_path(কাঠামো path_cxt *pc, dev_t devno, কাঠামো path_cxt *parent)
{
	কাঠামো sysfs_blkdev *blk;
	পূর্ণসংখ্যা rc;
	অক্ষর buf[আকার(_PATH_SYS_DEVBLOCK)
		 + আকার(stringify_মান(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, আকার(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	যদি (rc)
		ফেরত rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	যদি (rc < 0)
		ফেরত rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	যদি (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, আকার(কাঠামো sysfs_blkdev));
		যদি (!blk)
			ফেরত -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ফেরত 0;
}

স্থির শূন্য sysfs_blkdev_deinit_path(কাঠামো path_cxt *pc)
{
	কাঠামো sysfs_blkdev *blk;

	যদি (!pc)
		ফেরত;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	যদি (!blk)
		ফেরত;

	ul_unref_path(blk->parent);
	মুক্ত(blk);

	ul_path_set_dialect(pc, ফাঁকা, ফাঁকা);
}

পূর্ণসংখ্যা sysfs_blkdev_set_parent(কাঠামো path_cxt *pc, কাঠামো path_cxt *parent)
{
	কাঠামো sysfs_blkdev *blk = ul_path_get_dialect(pc);

	যদি (!pc || !blk)
		ফেরত -EINVAL;

	যদি (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = ফাঁকা;
	}

	যদি (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} অন্যথা
		blk->parent = ফাঁকা;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ফেরত 0;
}

কাঠামো path_cxt *sysfs_blkdev_get_parent(কাঠামো path_cxt *pc)
{
	কাঠামো sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ফেরত blk ? blk->parent : ফাঁকা;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
স্থির পূর্ণসংখ্যা sysfs_blkdev_enoent_redirect(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, পূর্ণসংখ্যা *dirfd)
{
	কাঠামো sysfs_blkdev *blk = ul_path_get_dialect(pc);

	যদি (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		যদি (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ফেরত 0;
		}
	}
	ফেরত 1;	/* no redirect */
}

অক্ষর *sysfs_blkdev_get_name(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsiz)
{
	অক্ষর link[PATH_MAX];
	অক্ষর *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, আকার(link), ফাঁকা);
	যদি (sz < 0)
		ফেরত ফাঁকা;

	name = strrchr(link, '/');
	যদি (!name)
		ফেরত ফাঁকা;

	name++;
	sz = strlen(name);
	যদি ((size_t) sz + 1 > bufsiz)
		ফেরত ফাঁকা;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ফেরত buf;
}

পূর্ণসংখ্যা sysfs_blkdev_is_partition_dirent(DIR *dir, কাঠামো dirent *d, ধ্রুবক অক্ষর *parent_name)
{
	অক্ষর path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ফেরত 0;
#endif
	size_t len = 0;

	যদি (parent_name) {
		ধ্রুবক অক্ষর *p = parent_name;

		/* /dev/sda --> "sda" */
		যদি (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			যদি (!p)
				ফেরত 0;
			p++;
		}

		len = strlen(p);
		যদি ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	যদি (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ফেরত ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, আকার(path), "%s/start", d->d_name);

	ফেরত faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

পূর্ণসংখ্যা sysfs_blkdev_গুনতি_partitions(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *devname)
{
	DIR *dir;
	কাঠামো dirent *d;
	পূর্ণসংখ্যা r = 0;

	dir = ul_path_opendir(pc, ফাঁকা);
	যদি (!dir)
		ফেরত 0;

	যখন ((d = xreaddir(dir))) {
		যদি (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ফেরত r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(কাঠামো path_cxt *pc, পূর্ণসংখ্যা partno)
{
	DIR *dir;
	কাঠামো dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, ফাঁকা);
	যদি (!dir)
		ফেরত 0;

	যখন ((d = xreaddir(dir))) {
		পূর্ণসংখ্যা n;

		যদি (!sysfs_blkdev_is_partition_dirent(dir, d, ফাঁকা))
			চালু;

		যদি (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			চালু;

		যদি (n == partno) {
			যদি (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				বিরতি;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (পূর্ণসংখ্যা) partno, (পূর্ণসংখ্যা) devno));
	ফেরত devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
অক্ষর *sysfs_blkdev_get_slave(কাঠামো path_cxt *pc)
{
	DIR *dir;
	কাঠামো dirent *d;
	অক্ষর *name = ফাঁকা;

	dir = ul_path_opendir(pc, "slaves");
	যদি (!dir)
		ফেরত ফাঁকা;

	যখন ((d = xreaddir(dir))) {
		যদি (name)
			যাও err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ফেরত name;
err:
	মুক্ত(name);
	closedir(dir);
	ফেরত ফাঁকা;
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
স্থির অক্ষর *get_subsystem(অক্ষর *chain, অক্ষর *buf, size_t bufsz)
{
	size_t len;
	অক্ষর *p;

	যদি (!chain || !*chain)
		ফেরত ফাঁকা;

	len = strlen(chain);
	যদি (len + আকার(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ফেরত ফাঁকা;

	করো {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, আকার(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		যদি (p) {
			*p = '\0';
			len = p - chain;
		}

		যদি (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ফেরত basename(buf);
		}

	} যখন (p);

	ফেরত ফাঁকা;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
অক্ষর *sysfs_blkdev_get_devchain(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	কাঠামো ul_বাফার tmp = UL_INIT_বাফার;
	ধ্রুবক অক্ষর *p;
	অক্ষর *res = ফাঁকা;

	ssz = ul_path_readlink(pc, buf, bufsz, ফাঁকা);
	যদি (ssz <= 0)
		ফেরত ফাঁকা;

	যদি ((p = ul_path_get_prefix(pc)))
		ul_বাফার_append_string(&tmp, p);

	ul_বাফার_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_বাফার_append_তথ্য(&tmp, buf, ssz);

	p = ul_বাফার_get_string(&tmp, &sz, ফাঁকা);
	যদি (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_বাফার_free_তথ্য(&tmp);
	ফেরত res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
পূর্ণসংখ্যা sysfs_blkdev_পরবর্তী_subsystem(কাঠামো path_cxt *pc __attribute__((unused)),
			 অক্ষর *devchain, অক্ষর **subsys)
{
	অক্ষর subbuf[PATH_MAX];
	অক্ষর *sub;

	যদি (!subsys || !devchain)
		ফেরত -EINVAL;

	*subsys = ফাঁকা;

	যখন ((sub = get_subsystem(devchain, subbuf, আকার(subbuf)))) {
		*subsys = strdup(sub);
		যদি (!*subsys)
			ফেরত -ENOMEM;
		ফেরত 0;
	}

	ফেরত 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
স্থির পূর্ণসংখ্যা sysfs_devchain_is_removable(অক্ষর *chain)
{
	size_t len;
	অক্ষর buf[20];
	অক্ষর *p;

	যদি (!chain || !*chain)
		ফেরত 0;

	len = strlen(chain);
	যদি (len + আকার(REMOVABLE_FILENAME) > PATH_MAX)
		ফেরত 0;

	করো {
		পূর্ণসংখ্যা fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, আকার(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		যদি (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			বিরতি;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		যদি (fd != -1) {
			rc = ul_read_all(fd, buf, আকার(buf));
			close(fd);

			যদি (rc > 0) {
				যদি (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ফেরত 0;
				} অন্যথা যদি (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ফেরত 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		যদি (p) {
			*p = '\0';
			len = p - chain;
		}

	} যখন (p);

	ফেরত 0;
}

পূর্ণসংখ্যা sysfs_blkdev_is_hotpluggable(কাঠামো path_cxt *pc)
{
	অক্ষর buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, আকার(buf));
	ফেরত sysfs_devchain_is_removable(chain);
}

পূর্ণসংখ্যা sysfs_blkdev_is_removable(কাঠামো path_cxt *pc)
{
	পূর্ণসংখ্যা rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	যদি (ul_path_read_s32(pc, &rc, "removable") == 0)
		ফেরত rc;

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা get_dm_wholedisk(কাঠামো path_cxt *pc, অক্ষর *diskname,
                            size_t len, dev_t *diskdevno)
{
	পূর্ণসংখ্যা rc = 0;
	অক্ষর *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	যদি (!name)
		ফেরত -1;

	যদি (diskname && len)
		xstrncpy(diskname, name, len);

	যদি (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ফাঁকা);
		যদি (!*diskdevno)
			rc = -1;
	}

	মুক্ত(name);
	ফেরত rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
পূর্ণসংখ্যা sysfs_blkdev_get_wholedisk(	কাঠামো path_cxt *pc,
				অক্ষর *diskname,
				size_t len,
				dev_t *diskdevno)
{
	পূর্ণসংখ্যা is_part = 0;

	যদি (!pc)
		ফেরত -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	যদি (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		অক্ষর *uuid = ফাঁকা, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : ফাঁকা;

		যদি (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		মুক্ত(uuid);

		যদি (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			যাও done;

		is_part = 0;
	}

	যদি (!is_part) {
		/*
		 * unpartitioned device
		 */
		যদি (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			যাও err;
		যদি (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} অন্যথা {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		অক্ষর linkpath[PATH_MAX];
		অক্ষর *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, আকার(linkpath), ফাঁকা);
		যদি (linklen < 0)
			যাও err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		যদি (!name)
			যাও err;

		sysfs_devname_sys_to_dev(name);
		যদি (diskname && len)
			xstrncpy(diskname, name, len);

		যদি (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ফাঁকা);
			যদি (!*diskdevno)
				যাও err;
		}
	}

done:
	ফেরত 0;
err:
	ফেরত -1;
}

পূর্ণসংখ্যা sysfs_devno_to_wholedisk(dev_t devno, অক্ষর *diskname,
		             size_t len, dev_t *diskdevno)
{
	কাঠামো path_cxt *pc;
	পূর্ণসংখ্যা rc = 0;

	যদি (!devno)
		ফেরত -EINVAL;
	pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	যদি (!pc)
		ফেরত -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ফেরত rc;
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
পূর্ণসংখ্যা sysfs_devno_is_dm_hidden(dev_t devno, অক্ষর **uuid,
			     ধ্রুবক কাঠামো ul_vfs_ops *vfs)
{
	কাঠামো path_cxt *pc = ফাঁকা;
	অক্ষর *id = ফাঁকা;
	পূর্ণসংখ্যা rc = 0;

	pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	যদি (!pc)
		যাও done;
	ul_path_refer_vfs(pc, vfs);
	যদি (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		যাও done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	যদি (strncmp(id, "LVM-", 4) == 0) {
		অক্ষর *p = strrchr(id + 4, '-');

		যদি (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	যদি (uuid)
		*uuid = id;
	অন্যথা
		মুক্ত(id);
	ফেরত rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
পূর্ণসংখ্যা sysfs_devno_is_dm_private(dev_t devno, অক্ষর **uuid,
			      ধ্রুবক কাঠামো ul_vfs_ops *vfs)
{
	কাঠামো path_cxt *pc = ফাঁকা;
	অক্ষর *id = ফাঁকা;
	পূর্ণসংখ্যা rc = 0;

	pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	যদি (!pc)
		যাও done;
	ul_path_refer_vfs(pc, vfs);
	যদি (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		যাও done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	যদি (strncmp(id, "LVM-", 4) == 0) {
		অক্ষর *p = strrchr(id + 4, '-');

		যদি (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} অন্যথা যদি (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	যদি (uuid)
		*uuid = id;
	অন্যথা
		মুক্ত(id);
	ফেরত rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
পূর্ণসংখ্যা sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	যদি (sysfs_devno_to_wholedisk(devno, ফাঁকা, 0, &disk) != 0)
		ফেরত -1;

	ফেরত devno == disk;
}


পূর্ণসংখ্যা sysfs_blkdev_scsi_get_hctl(কাঠামো path_cxt *pc, পূর্ণসংখ্যা *h, পূর্ণসংখ্যা *c, পূর্ণসংখ্যা *t, পূর্ণসংখ্যা *l)
{
	অক্ষর buf[PATH_MAX], *hctl;
	কাঠামো sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	যদি (!blk || blk->hctl_error)
		ফেরত -EINVAL;
	যদি (blk->has_hctl)
		যাও done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, আকার(buf), "device");
	যদি (len < 0)
		ফেরত len;

	hctl = strrchr(buf, '/');
	যদি (!hctl)
		ফেরত -1;
	hctl++;

	যদি (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ফেরত -1;

	blk->has_hctl = 1;
done:
	যদি (h)
		*h = blk->scsi_host;
	যদি (c)
		*c = blk->scsi_channel;
	যদি (t)
		*t = blk->scsi_target;
	যদি (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ফেরত 0;
}


স্থির অক্ষর *scsi_host_attribute_path(
			কাঠামো path_cxt *pc,
			ধ্রুবক অক্ষর *type,
			অক্ষর *buf,
			size_t bufsz,
			ধ্রুবক অক্ষর *attr)
{
	পূর্ণসংখ্যা len;
	পূর্ণসংখ্যা host;
	ধ্রুবক অক্ষর *prefix;

	যদি (sysfs_blkdev_scsi_get_hctl(pc, &host, ফাঁকা, ফাঁকা, ফাঁকা))
		ফেরত ফাঁকা;

	prefix = ul_path_get_prefix(pc);
	যদি (!prefix)
		prefix = "";

	যদি (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	অন্যথা
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ফেরত (len < 0 || (size_t) len >= bufsz) ? ফাঁকা : buf;
}

অক্ষর *sysfs_blkdev_scsi_host_strdup_attribute(কাঠামো path_cxt *pc,
			ধ্রুবক অক্ষর *type, ধ্রুবক অক্ষর *attr)
{
	অক্ষর buf[1024];
	পূর্ণসংখ্যা rc;
	FILE *f;

	যদি (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, আকার(buf), attr))
		ফেরত ফাঁকা;

	যদি (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ফেরত ফাঁকা;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ফেরত rc == 1 ? strdup(buf) : ফাঁকা;
}

পূর্ণসংখ্যা sysfs_blkdev_scsi_host_is(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *type)
{
	অক্ষর buf[PATH_MAX];
	কাঠামো stat st;

	যদি (!type || !scsi_host_attribute_path(pc, type,
				buf, আকার(buf), ফাঁকা))
		ফেরত 0;

	ফেরত stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

স্থির অক্ষর *scsi_attribute_path(কাঠামো path_cxt *pc,
		অক্ষর *buf, size_t bufsz, ধ্রুবক অক্ষর *attr)
{
	পূর্ণসংখ্যা len, h, c, t, l;
	ধ্রুবক অক্ষর *prefix;

	যদি (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ফেরত ফাঁকা;

	prefix = ul_path_get_prefix(pc);
	যদি (!prefix)
		prefix = "";

	যদি (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	অন্যথা
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ফেরত (len < 0 || (size_t) len >= bufsz) ? ফাঁকা : buf;
}

পূর্ণসংখ্যা sysfs_blkdev_scsi_has_attribute(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *attr)
{
	অক্ষর path[PATH_MAX];
	কাঠামো stat st;

	যদি (!scsi_attribute_path(pc, path, আকার(path), attr))
		ফেরত 0;

	ফেরত stat(path, &st) == 0;
}

পূর্ণসংখ্যা sysfs_blkdev_scsi_path_contains(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *pattern)
{
	অক্ষর path[PATH_MAX], linkc[PATH_MAX];
	কাঠামো stat st;
	ssize_t len;

	যদি (!scsi_attribute_path(pc, path, আকার(path), ফাঁকা))
		ফেরত 0;

	যদি (stat(path, &st) != 0)
		ফেরত 0;

	len = readlink(path, linkc, আকার(linkc) - 1);
	যদি (len < 0)
		ফেরত 0;

	linkc[len] = '\0';
	ফেরত strstr(linkc, pattern) != ফাঁকা;
}

স্থির dev_t read_devno(ধ্রুবক অক্ষর *path)
{
	FILE *f;
	পূর্ণসংখ্যা maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	যদি (!f)
		ফেরত 0;

	যদি (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ফেরত dev;
}

পূর্ণসংখ্যা sysfs_devname_is_hidden(ধ্রুবক অক্ষর *prefix, ধ্রুবক অক্ষর *name)
{
	অক্ষর buf[PATH_MAX];
	পূর্ণসংখ্যা rc = 0, hidden = 0, len;
	FILE *f;

	যদি (strncmp("/dev/", name, 5) == 0)
		ফেরত 0;

	যদি (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, আকার(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	যদি (len < 0 || (size_t) len + 1 > আকার(buf))
		ফেরত 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	যদি (!f)
		ফেরত 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ফেরত rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ধ্রুবক অক্ষর *prefix, ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *parent)
{
	অক্ষর buf[PATH_MAX];
	অক্ষর *_name = ফাঁকা, *_parent = ফাঁকা;	/* name as encoded in sysfs */
	dev_t dev = 0;
	পূর্ণসংখ্যা len;

	যদি (!prefix)
		prefix = "";

	প্রতিজ্ঞা(name);

	যদি (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		কাঠামো stat st;

		যদি (stat(name, &st) == 0) {
			dev = st.st_rdev;
			যাও done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	যদি (!_name)
		যাও done;
	sysfs_devname_dev_to_sys(_name);

	যদি (parent) {
		_parent = strdup(parent);
		যদি (!_parent)
			যাও done;
	}

	যদি (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, আকার(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		যদি (len < 0 || (size_t) len >= আকার(buf))
			যাও done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		যাও done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, আকার(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	যদি (len < 0 || (size_t) len >= আকার(buf))
		যাও done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	যদি (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, আকার(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		যদি (len < 0 || (size_t) len >= আকার(buf))
			যাও done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	যদি (!dev) {
		len = snprintf(buf, আকার(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		যদি (len < 0 || (size_t) len >= আকার(buf))
			যাও done;
		dev = read_devno(buf);
	}
done:
	মুক্ত(_name);
	মুক্ত(_parent);
	ফেরত dev;
}

dev_t sysfs_devname_to_devno(ধ্রুবক অক্ষর *name)
{
	ফেরত __sysfs_devname_to_devno(ফাঁকা, name, ফাঁকা);
}

অক্ষর *sysfs_blkdev_get_path(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsiz)
{
	ধ্রুবক অক্ষর *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	অক্ষর *res = ফাঁকা;
	size_t sz;
	কাঠামো stat st;

	যদি (!name)
		যাও done;

	sz = strlen(name);
	যদি (sz + আকার("/dev/") > bufsiz)
		যাও done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	যদি (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ফেরত res;
}

dev_t sysfs_blkdev_get_devno(কাঠামো path_cxt *pc)
{
	ফেরত ((কাঠামো sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
অক্ষর *sysfs_devno_to_devpath(dev_t devno, অক্ষর *buf, size_t bufsiz)
{
	কাঠামো path_cxt *pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	অক্ষর *res = ফাঁকা;

	যদি (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ফেরত res;
}

অক্ষর *sysfs_devno_to_devname(dev_t devno, অক্ষর *buf, size_t bufsiz)
{
	কাঠামো path_cxt *pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	অক্ষর *res = ফাঁকা;

	যদি (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ফেরত res;
}

পূর্ণসংখ্যা sysfs_devno_গুনতি_partitions(dev_t devno)
{
	কাঠামো path_cxt *pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	পূর্ণসংখ্যা n = 0;

	যদি (pc) {
		অক্ষর buf[PATH_MAX + 1];
		অক্ষর *name = sysfs_blkdev_get_name(pc, buf, আকার(buf));

		n = sysfs_blkdev_গুনতি_partitions(pc, name);
		ul_unref_path(pc);
	}
	ফেরত n;
}

অক্ষর *sysfs_chrdev_devno_to_devname(dev_t devno, অক্ষর *buf, size_t bufsiz)
{
	অক্ষর link[PATH_MAX];
	কাঠামো path_cxt *pc;
	অক্ষর *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	যদি (!pc)
		ফেরত ফাঁকা;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, আকার(link), ফাঁকা);
	ul_unref_path(pc);

	যদি (sz < 0)
		ফেরত ফাঁকা;

	name = strrchr(link, '/');
	যদি (!name)
		ফেরত ফাঁকা;

	name++;
	sz = strlen(name);
	যদি ((size_t) sz + 1 > bufsiz)
		ফেরত ফাঁকা;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ফেরত buf;

}

গণনা sysfs_byteorder sysfs_get_byteorder(কাঠামো path_cxt *pc)
{
	পূর্ণসংখ্যা rc;
	অক্ষর buf[BUFSIZ];
	গণনা sysfs_byteorder ret;

	rc = ul_path_read_বাফার(pc, buf, আকার(buf), _PATH_SYS_CPU_BYTEORDER);
	যদি (rc < 0)
		যাও unknown;

	যদি (strncmp(buf, "little", আকার(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		যাও out;
	} অন্যথা যদি (strncmp(buf, "big", আকার(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		যাও out;
	}

unknown:
#যদি __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#নতুবা_যদি __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#অন্যথা
#error Unknown byte order
#endif

out:
	ফেরত ret;
}

পূর্ণসংখ্যা sysfs_get_address_bits(কাঠামো path_cxt *pc)
{
	পূর্ণসংখ্যা rc;
	পূর্ণসংখ্যা address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	যদি (rc < 0)
		ফেরত rc;
	যদি (address_bits < 0)
		ফেরত -EINVAL;
	ফেরত address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdlib.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	কাঠামো path_cxt *pc;
	অক্ষর *devname;
	dev_t devno, disk_devno;
	অক্ষর path[PATH_MAX], *sub, *chain;
	অক্ষর diskname[32];
	পূর্ণসংখ্যা i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	যদি (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	যদি (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণসংখ্যা) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, আকার(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, আকার(path)));

	sysfs_devno_to_wholedisk(devno, diskname, আকার(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণসংখ্যা) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, ফাঁকা, ফাঁকা);
	যদি (!pc)
		যাও done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণসংখ্যা) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, আকার(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, আকার(path)));

	sysfs_devno_to_wholedisk(devno, diskname, আকার(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned পূর্ণসংখ্যা) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	যদি (is_part && disk_devno) {
		কাঠামো path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, ফাঁকা, ফাঁকা);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_গুনতি_dirents(pc, "slaves"));

	যদি (!is_part) {
		printf("First 5 partitions:\n");
		জন্য (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			যদি (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	যদি (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	অন্যথা
		printf(" SIZE: %"PRIu64"\n", u64);

	যদি (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	অন্যথা
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, আকার(path));
	printf(" SUBSYSTEMS:\n");

	যখন (chain && sysfs_blkdev_পরবর্তী_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		মুক্ত(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ফেরত rc;
}
#endif /* TEST_PROGRAM_SYSFS */
