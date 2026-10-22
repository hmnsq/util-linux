/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#ضمن <ctype.h>
#ضمن <inttypes.h>
#ضمن <libgen.h>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "pathnames.h"
#ضمن "sysfs.h"
#ضمن "fileutils.h"
#ضمن "all-io.h"
#ضمن "debug.h"
#ضمن "strutils.h"
#ضمن "buffer.h"

ساكن فراغ sysfs_blkdev_deinit_path(بنية path_cxt *pc);
ساكن صحيح  sysfs_blkdev_enoent_redirect(بنية path_cxt *pc, ثابت حرف *path, صحيح *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

فراغ ul_sysfs_init_debug(فراغ)
{
	אם (ulsysfs_debug_mask)
		החזר;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

بنية path_cxt *ul_new_sysfs_path(dev_t devno, بنية path_cxt *parent, ثابت حرف *prefix)
{
	بنية path_cxt *pc = ul_new_path(عدم);

	אם (!pc)
		החזר عدم;
	אם (prefix)
		ul_path_set_prefix(pc, prefix);

	אם (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		החזר عدم;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	החזר pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحيح sysfs_blkdev_init_path(بنية path_cxt *pc, dev_t devno, بنية path_cxt *parent)
{
	بنية sysfs_blkdev *blk;
	صحيح rc;
	حرف buf[حجم(_PATH_SYS_DEVBLOCK)
		 + حجم(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, حجم(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	אם (rc)
		החזר rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	אם (rc < 0)
		החזר rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	אם (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, حجم(بنية sysfs_blkdev));
		אם (!blk)
			החזר -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	החזר 0;
}

ساكن فراغ sysfs_blkdev_deinit_path(بنية path_cxt *pc)
{
	بنية sysfs_blkdev *blk;

	אם (!pc)
		החזר;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	אם (!blk)
		החזר;

	ul_unref_path(blk->parent);
	حرر(blk);

	ul_path_set_dialect(pc, عدم, عدم);
}

صحيح sysfs_blkdev_set_parent(بنية path_cxt *pc, بنية path_cxt *parent)
{
	بنية sysfs_blkdev *blk = ul_path_get_dialect(pc);

	אם (!pc || !blk)
		החזר -EINVAL;

	אם (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = عدم;
	}

	אם (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} אחרת
		blk->parent = عدم;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	החזר 0;
}

بنية path_cxt *sysfs_blkdev_get_parent(بنية path_cxt *pc)
{
	بنية sysfs_blkdev *blk = ul_path_get_dialect(pc);
	החזר blk ? blk->parent : عدم;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ساكن صحيح sysfs_blkdev_enoent_redirect(بنية path_cxt *pc, ثابت حرف *path, صحيح *dirfd)
{
	بنية sysfs_blkdev *blk = ul_path_get_dialect(pc);

	אם (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		אם (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			החזר 0;
		}
	}
	החזר 1;	/* no redirect */
}

حرف *sysfs_blkdev_get_name(بنية path_cxt *pc, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	حرف *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, حجم(link), عدم);
	אם (sz < 0)
		החזר عدم;

	name = strrchr(link, '/');
	אם (!name)
		החזר عدم;

	name++;
	sz = strlen(name);
	אם ((size_t) sz + 1 > bufsiz)
		החזר عدم;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	החזר buf;
}

صحيح sysfs_blkdev_is_partition_dirent(DIR *dir, بنية dirent *d, ثابت حرف *parent_name)
{
	حرف path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	אם (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		החזר 0;
#endif
	size_t len = 0;

	אם (parent_name) {
		ثابت حرف *p = parent_name;

		/* /dev/sda --> "sda" */
		אם (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			אם (!p)
				החזר 0;
			p++;
		}

		len = strlen(p);
		אם ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	אם (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		החזר ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, حجم(path), "%s/start", d->d_name);

	החזר faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

صحيح sysfs_blkdev_count_partitions(بنية path_cxt *pc, ثابت حرف *devname)
{
	DIR *dir;
	بنية dirent *d;
	صحيح r = 0;

	dir = ul_path_opendir(pc, عدم);
	אם (!dir)
		החזר 0;

	כלעוד ((d = xreaddir(dir))) {
		אם (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	החזר r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(بنية path_cxt *pc, صحيح partno)
{
	DIR *dir;
	بنية dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, عدم);
	אם (!dir)
		החזר 0;

	כלעוד ((d = xreaddir(dir))) {
		صحيح n;

		אם (!sysfs_blkdev_is_partition_dirent(dir, d, عدم))
			استمر;

		אם (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			استمر;

		אם (n == partno) {
			אם (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				اكسر;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (صحيح) partno, (صحيح) devno));
	החזר devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
حرف *sysfs_blkdev_get_slave(بنية path_cxt *pc)
{
	DIR *dir;
	بنية dirent *d;
	حرف *name = عدم;

	dir = ul_path_opendir(pc, "slaves");
	אם (!dir)
		החזר عدم;

	כלעוד ((d = xreaddir(dir))) {
		אם (name)
			اذهب err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	החזר name;
err:
	حرر(name);
	closedir(dir);
	החזר عدم;
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
ساكن حرف *get_subsystem(حرف *chain, حرف *buf, size_t bufsz)
{
	size_t len;
	حرف *p;

	אם (!chain || !*chain)
		החזר عدم;

	len = strlen(chain);
	אם (len + حجم(SUBSYSTEM_LINKNAME) > PATH_MAX)
		החזר عدم;

	עשה {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, حجم(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		אם (p) {
			*p = '\0';
			len = p - chain;
		}

		אם (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			החזר basename(buf);
		}

	} כלעוד (p);

	החזר عدم;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
حرف *sysfs_blkdev_get_devchain(بنية path_cxt *pc, حرف *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	بنية ul_buffer tmp = UL_INIT_BUFFER;
	ثابت حرف *p;
	حرف *res = عدم;

	ssz = ul_path_readlink(pc, buf, bufsz, عدم);
	אם (ssz <= 0)
		החזר عدم;

	אם ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, عدم);
	אם (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	החזר res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
صحيح sysfs_blkdev_next_subsystem(بنية path_cxt *pc __attribute__((unused)),
			 حرف *devchain, حرف **subsys)
{
	حرف subbuf[PATH_MAX];
	حرف *sub;

	אם (!subsys || !devchain)
		החזר -EINVAL;

	*subsys = عدم;

	כלעוד ((sub = get_subsystem(devchain, subbuf, حجم(subbuf)))) {
		*subsys = strdup(sub);
		אם (!*subsys)
			החזר -ENOMEM;
		החזר 0;
	}

	החזר 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ساكن صحيح sysfs_devchain_is_removable(حرف *chain)
{
	size_t len;
	حرف buf[20];
	حرف *p;

	אם (!chain || !*chain)
		החזר 0;

	len = strlen(chain);
	אם (len + حجم(REMOVABLE_FILENAME) > PATH_MAX)
		החזר 0;

	עשה {
		صحيح fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, حجم(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		אם (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			اكسر;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		אם (fd != -1) {
			rc = ul_read_all(fd, buf, حجم(buf));
			close(fd);

			אם (rc > 0) {
				אם (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					החזר 0;
				} אחרת אם (strncmp(buf, "removable", min(rc, 9)) == 0) {
					החזר 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		אם (p) {
			*p = '\0';
			len = p - chain;
		}

	} כלעוד (p);

	החזר 0;
}

صحيح sysfs_blkdev_is_hotpluggable(بنية path_cxt *pc)
{
	حرف buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, حجم(buf));
	החזר sysfs_devchain_is_removable(chain);
}

صحيح sysfs_blkdev_is_removable(بنية path_cxt *pc)
{
	صحيح rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	אם (ul_path_read_s32(pc, &rc, "removable") == 0)
		החזר rc;

	החזר 0;
}

ساكن صحيح get_dm_wholedisk(بنية path_cxt *pc, حرف *diskname,
                            size_t len, dev_t *diskdevno)
{
	صحيح rc = 0;
	حرف *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	אם (!name)
		החזר -1;

	אם (diskname && len)
		xstrncpy(diskname, name, len);

	אם (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, عدم);
		אם (!*diskdevno)
			rc = -1;
	}

	حرر(name);
	החזר rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
صحيح sysfs_blkdev_get_wholedisk(	بنية path_cxt *pc,
				حرف *diskname,
				size_t len,
				dev_t *diskdevno)
{
	صحيح is_part = 0;

	אם (!pc)
		החזר -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	אם (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		حرف *uuid = عدم, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : عدم;

		אם (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		حرر(uuid);

		אם (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			اذهب done;

		is_part = 0;
	}

	אם (!is_part) {
		/*
		 * unpartitioned device
		 */
		אם (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			اذهب err;
		אם (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} אחרת {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		حرف linkpath[PATH_MAX];
		حرف *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, حجم(linkpath), عدم);
		אם (linklen < 0)
			اذهب err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		אם (!name)
			اذهب err;

		sysfs_devname_sys_to_dev(name);
		אם (diskname && len)
			xstrncpy(diskname, name, len);

		אם (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, عدم);
			אם (!*diskdevno)
				اذهب err;
		}
	}

done:
	החזר 0;
err:
	החזר -1;
}

صحيح sysfs_devno_to_wholedisk(dev_t devno, حرف *diskname,
		             size_t len, dev_t *diskdevno)
{
	بنية path_cxt *pc;
	صحيح rc = 0;

	אם (!devno)
		החזר -EINVAL;
	pc = ul_new_sysfs_path(devno, عدم, عدم);
	אם (!pc)
		החזר -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	החזר rc;
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
صحيح sysfs_devno_is_dm_hidden(dev_t devno, حرف **uuid,
			     ثابت بنية ul_vfs_ops *vfs)
{
	بنية path_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_path(devno, عدم, عدم);
	אם (!pc)
		اذهب done;
	ul_path_refer_vfs(pc, vfs);
	אם (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	אם (strncmp(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		אם (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	אם (uuid)
		*uuid = id;
	אחרת
		حرر(id);
	החזר rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
صحيح sysfs_devno_is_dm_private(dev_t devno, حرف **uuid,
			      ثابت بنية ul_vfs_ops *vfs)
{
	بنية path_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_path(devno, عدم, عدم);
	אם (!pc)
		اذهب done;
	ul_path_refer_vfs(pc, vfs);
	אם (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	אם (strncmp(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		אם (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} אחרת אם (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	אם (uuid)
		*uuid = id;
	אחרת
		حرر(id);
	החזר rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
صحيح sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	אם (sysfs_devno_to_wholedisk(devno, عدم, 0, &disk) != 0)
		החזר -1;

	החזר devno == disk;
}


صحيح sysfs_blkdev_scsi_get_hctl(بنية path_cxt *pc, صحيح *h, صحيح *c, صحيح *t, صحيح *l)
{
	حرف buf[PATH_MAX], *hctl;
	بنية sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	אם (!blk || blk->hctl_error)
		החזר -EINVAL;
	אם (blk->has_hctl)
		اذهب done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, حجم(buf), "device");
	אם (len < 0)
		החזר len;

	hctl = strrchr(buf, '/');
	אם (!hctl)
		החזר -1;
	hctl++;

	אם (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		החזר -1;

	blk->has_hctl = 1;
done:
	אם (h)
		*h = blk->scsi_host;
	אם (c)
		*c = blk->scsi_channel;
	אם (t)
		*t = blk->scsi_target;
	אם (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	החזר 0;
}


ساكن حرف *scsi_host_attribute_path(
			بنية path_cxt *pc,
			ثابت حرف *type,
			حرف *buf,
			size_t bufsz,
			ثابت حرف *attr)
{
	صحيح len;
	صحيح host;
	ثابت حرف *prefix;

	אם (sysfs_blkdev_scsi_get_hctl(pc, &host, عدم, عدم, عدم))
		החזר عدم;

	prefix = ul_path_get_prefix(pc);
	אם (!prefix)
		prefix = "";

	אם (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	אחרת
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	החזר (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

حرف *sysfs_blkdev_scsi_host_strdup_attribute(بنية path_cxt *pc,
			ثابت حرف *type, ثابت حرف *attr)
{
	حرف buf[1024];
	صحيح rc;
	FILE *f;

	אם (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, حجم(buf), attr))
		החזר عدم;

	אם (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		החזר عدم;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	החזר rc == 1 ? strdup(buf) : عدم;
}

صحيح sysfs_blkdev_scsi_host_is(بنية path_cxt *pc, ثابت حرف *type)
{
	حرف buf[PATH_MAX];
	بنية stat st;

	אם (!type || !scsi_host_attribute_path(pc, type,
				buf, حجم(buf), عدم))
		החזר 0;

	החזר stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ساكن حرف *scsi_attribute_path(بنية path_cxt *pc,
		حرف *buf, size_t bufsz, ثابت حرف *attr)
{
	صحيح len, h, c, t, l;
	ثابت حرف *prefix;

	אם (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		החזר عدم;

	prefix = ul_path_get_prefix(pc);
	אם (!prefix)
		prefix = "";

	אם (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	אחרת
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	החזר (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

صحيح sysfs_blkdev_scsi_has_attribute(بنية path_cxt *pc, ثابت حرف *attr)
{
	حرف path[PATH_MAX];
	بنية stat st;

	אם (!scsi_attribute_path(pc, path, حجم(path), attr))
		החזר 0;

	החזר stat(path, &st) == 0;
}

صحيح sysfs_blkdev_scsi_path_contains(بنية path_cxt *pc, ثابت حرف *pattern)
{
	حرف path[PATH_MAX], linkc[PATH_MAX];
	بنية stat st;
	ssize_t len;

	אם (!scsi_attribute_path(pc, path, حجم(path), عدم))
		החזר 0;

	אם (stat(path, &st) != 0)
		החזר 0;

	len = readlink(path, linkc, حجم(linkc) - 1);
	אם (len < 0)
		החזר 0;

	linkc[len] = '\0';
	החזר strstr(linkc, pattern) != عدم;
}

ساكن dev_t read_devno(ثابت حرف *path)
{
	FILE *f;
	صحيح maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	אם (!f)
		החזר 0;

	אם (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	החזר dev;
}

صحيح sysfs_devname_is_hidden(ثابت حرف *prefix, ثابت حرف *name)
{
	حرف buf[PATH_MAX];
	صحيح rc = 0, hidden = 0, len;
	FILE *f;

	אם (strncmp("/dev/", name, 5) == 0)
		החזר 0;

	אם (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, حجم(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	אם (len < 0 || (size_t) len + 1 > حجم(buf))
		החזר 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	אם (!f)
		החזר 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	החזר rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ثابت حرف *prefix, ثابت حرف *name, ثابت حرف *parent)
{
	حرف buf[PATH_MAX];
	حرف *_name = عدم, *_parent = عدم;	/* name as encoded in sysfs */
	dev_t dev = 0;
	صحيح len;

	אם (!prefix)
		prefix = "";

	أكد(name);

	אם (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		بنية stat st;

		אם (stat(name, &st) == 0) {
			dev = st.st_rdev;
			اذهب done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	אם (!_name)
		اذهب done;
	sysfs_devname_dev_to_sys(_name);

	אם (parent) {
		_parent = strdup(parent);
		אם (!_parent)
			اذهب done;
	}

	אם (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, حجم(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		אם (len < 0 || (size_t) len >= حجم(buf))
			اذهب done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		اذهب done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, حجم(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	אם (len < 0 || (size_t) len >= حجم(buf))
		اذهب done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	אם (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, حجم(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		אם (len < 0 || (size_t) len >= حجم(buf))
			اذهب done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	אם (!dev) {
		len = snprintf(buf, حجم(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		אם (len < 0 || (size_t) len >= حجم(buf))
			اذهب done;
		dev = read_devno(buf);
	}
done:
	حرر(_name);
	حرر(_parent);
	החזר dev;
}

dev_t sysfs_devname_to_devno(ثابت حرف *name)
{
	החזר __sysfs_devname_to_devno(عدم, name, عدم);
}

حرف *sysfs_blkdev_get_path(بنية path_cxt *pc, حرف *buf, size_t bufsiz)
{
	ثابت حرف *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	حرف *res = عدم;
	size_t sz;
	بنية stat st;

	אם (!name)
		اذهب done;

	sz = strlen(name);
	אם (sz + حجم("/dev/") > bufsiz)
		اذهب done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	אם (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	החזר res;
}

dev_t sysfs_blkdev_get_devno(بنية path_cxt *pc)
{
	החזר ((بنية sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
حرف *sysfs_devno_to_devpath(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية path_cxt *pc = ul_new_sysfs_path(devno, عدم, عدم);
	حرف *res = عدم;

	אם (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	החזר res;
}

حرف *sysfs_devno_to_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية path_cxt *pc = ul_new_sysfs_path(devno, عدم, عدم);
	حرف *res = عدم;

	אם (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	החזר res;
}

صحيح sysfs_devno_count_partitions(dev_t devno)
{
	بنية path_cxt *pc = ul_new_sysfs_path(devno, عدم, عدم);
	صحيح n = 0;

	אם (pc) {
		حرف buf[PATH_MAX + 1];
		حرف *name = sysfs_blkdev_get_name(pc, buf, حجم(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	החזר n;
}

حرف *sysfs_chrdev_devno_to_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	بنية path_cxt *pc;
	حرف *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	אם (!pc)
		החזר عدم;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, حجم(link), عدم);
	ul_unref_path(pc);

	אם (sz < 0)
		החזר عدم;

	name = strrchr(link, '/');
	אם (!name)
		החזר عدم;

	name++;
	sz = strlen(name);
	אם ((size_t) sz + 1 > bufsiz)
		החזר عدم;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	החזר buf;

}

تعداد sysfs_byteorder sysfs_get_byteorder(بنية path_cxt *pc)
{
	صحيح rc;
	حرف buf[BUFSIZ];
	تعداد sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, حجم(buf), _PATH_SYS_CPU_BYTEORDER);
	אם (rc < 0)
		اذهب unknown;

	אם (strncmp(buf, "little", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		اذهب out;
	} אחרת אם (strncmp(buf, "big", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		اذهب out;
	}

unknown:
#אם __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#وإلاإذا __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#אחרת
#error Unknown byte order
#endif

out:
	החזר ret;
}

صحيح sysfs_get_address_bits(بنية path_cxt *pc)
{
	صحيح rc;
	صحيح address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	אם (rc < 0)
		החזר rc;
	אם (address_bits < 0)
		החזר -EINVAL;
	החזר address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#ضمن <errno.h>
#ضمن <stdlib.h>

صحيح main(صحيح argc, حرف *argv[])
{
	بنية path_cxt *pc;
	حرف *devname;
	dev_t devno, disk_devno;
	حرف path[PATH_MAX], *sub, *chain;
	حرف diskname[32];
	صحيح i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	אם (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	אם (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned صحيح) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, حجم(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, حجم(path)));

	sysfs_devno_to_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, عدم, عدم);
	אם (!pc)
		اذهب done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned صحيح) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, حجم(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, حجم(path)));

	sysfs_devno_to_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	אם (is_part && disk_devno) {
		بنية path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, عدم, عدم);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	אם (!is_part) {
		printf("First 5 partitions:\n");
		עבור (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			אם (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	אם (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	אחרת
		printf(" SIZE: %"PRIu64"\n", u64);

	אם (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	אחרת
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, حجم(path));
	printf(" SUBSYSTEMS:\n");

	כלעוד (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		حرر(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	החזר rc;
}
#endif /* TEST_PROGRAM_SYSFS */
