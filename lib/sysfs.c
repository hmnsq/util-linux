/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#شامل <ctype.h>
#شامل <inttypes.h>
#شامل <libgen.h>
#شامل <fcntl.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "pathnames.h"
#شامل "sysfs.h"
#شامل "fileutils.h"
#شامل "all-io.h"
#شامل "debug.h"
#شامل "strutils.h"
#شامل "buffer.h"

ایستا تهی sysfs_blkdev_deinit_path(ساختار path_cxt *pc);
ایستا صحیح  sysfs_blkdev_enoent_redirect(ساختار path_cxt *pc, ثابت نویسه *path, صحیح *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ایستا UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

تهی ul_sysfs_init_debug(تهی)
{
	اگر (ulsysfs_debug_mask)
		بازگشت;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ساختار path_cxt *ul_new_sysfs_path(dev_t devno, ساختار path_cxt *parent, ثابت نویسه *prefix)
{
	ساختار path_cxt *pc = ul_new_path(پوچ);

	اگر (!pc)
		بازگشت پوچ;
	اگر (prefix)
		ul_path_set_prefix(pc, prefix);

	اگر (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		بازگشت پوچ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	بازگشت pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحیح sysfs_blkdev_init_path(ساختار path_cxt *pc, dev_t devno, ساختار path_cxt *parent)
{
	ساختار sysfs_blkdev *blk;
	صحیح rc;
	نویسه buf[اندازه(_PATH_SYS_DEVBLOCK)
		 + اندازه(stringify_مقدار(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, اندازه(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	اگر (rc)
		بازگشت rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	اگر (rc < 0)
		بازگشت rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	اگر (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, اندازه(ساختار sysfs_blkdev));
		اگر (!blk)
			بازگشت -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	بازگشت 0;
}

ایستا تهی sysfs_blkdev_deinit_path(ساختار path_cxt *pc)
{
	ساختار sysfs_blkdev *blk;

	اگر (!pc)
		بازگشت;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	اگر (!blk)
		بازگشت;

	ul_unref_path(blk->parent);
	آزاد(blk);

	ul_path_set_dialect(pc, پوچ, پوچ);
}

صحیح sysfs_blkdev_set_parent(ساختار path_cxt *pc, ساختار path_cxt *parent)
{
	ساختار sysfs_blkdev *blk = ul_path_get_dialect(pc);

	اگر (!pc || !blk)
		بازگشت -EINVAL;

	اگر (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = پوچ;
	}

	اگر (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} وگرنه
		blk->parent = پوچ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	بازگشت 0;
}

ساختار path_cxt *sysfs_blkdev_get_parent(ساختار path_cxt *pc)
{
	ساختار sysfs_blkdev *blk = ul_path_get_dialect(pc);
	بازگشت blk ? blk->parent : پوچ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ایستا صحیح sysfs_blkdev_enoent_redirect(ساختار path_cxt *pc, ثابت نویسه *path, صحیح *dirfd)
{
	ساختار sysfs_blkdev *blk = ul_path_get_dialect(pc);

	اگر (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		اگر (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			بازگشت 0;
		}
	}
	بازگشت 1;	/* no redirect */
}

نویسه *sysfs_blkdev_get_name(ساختار path_cxt *pc, نویسه *buf, size_t bufsiz)
{
	نویسه link[PATH_MAX];
	نویسه *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, اندازه(link), پوچ);
	اگر (sz < 0)
		بازگشت پوچ;

	name = strrchr(link, '/');
	اگر (!name)
		بازگشت پوچ;

	name++;
	sz = strlen(name);
	اگر ((size_t) sz + 1 > bufsiz)
		بازگشت پوچ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	بازگشت buf;
}

صحیح sysfs_blkdev_is_partition_dirent(DIR *dir, ساختار dirent *d, ثابت نویسه *parent_name)
{
	نویسه path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		بازگشت 0;
#endif
	size_t len = 0;

	اگر (parent_name) {
		ثابت نویسه *p = parent_name;

		/* /dev/sda --> "sda" */
		اگر (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			اگر (!p)
				بازگشت 0;
			p++;
		}

		len = strlen(p);
		اگر ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	اگر (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		بازگشت ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, اندازه(path), "%s/start", d->d_name);

	بازگشت faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

صحیح sysfs_blkdev_شمار_partitions(ساختار path_cxt *pc, ثابت نویسه *devname)
{
	DIR *dir;
	ساختار dirent *d;
	صحیح r = 0;

	dir = ul_path_opendir(pc, پوچ);
	اگر (!dir)
		بازگشت 0;

	وقتی ((d = xreaddir(dir))) {
		اگر (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	بازگشت r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(ساختار path_cxt *pc, صحیح partno)
{
	DIR *dir;
	ساختار dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, پوچ);
	اگر (!dir)
		بازگشت 0;

	وقتی ((d = xreaddir(dir))) {
		صحیح n;

		اگر (!sysfs_blkdev_is_partition_dirent(dir, d, پوچ))
			ادامه;

		اگر (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			ادامه;

		اگر (n == partno) {
			اگر (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				بشکن;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (صحیح) partno, (صحیح) devno));
	بازگشت devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
نویسه *sysfs_blkdev_get_slave(ساختار path_cxt *pc)
{
	DIR *dir;
	ساختار dirent *d;
	نویسه *name = پوچ;

	dir = ul_path_opendir(pc, "slaves");
	اگر (!dir)
		بازگشت پوچ;

	وقتی ((d = xreaddir(dir))) {
		اگر (name)
			برو err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	بازگشت name;
err:
	آزاد(name);
	closedir(dir);
	بازگشت پوچ;
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
ایستا نویسه *get_subsystem(نویسه *chain, نویسه *buf, size_t bufsz)
{
	size_t len;
	نویسه *p;

	اگر (!chain || !*chain)
		بازگشت پوچ;

	len = strlen(chain);
	اگر (len + اندازه(SUBSYSTEM_LINKNAME) > PATH_MAX)
		بازگشت پوچ;

	انجامبده {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, اندازه(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		اگر (p) {
			*p = '\0';
			len = p - chain;
		}

		اگر (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			بازگشت basename(buf);
		}

	} وقتی (p);

	بازگشت پوچ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
نویسه *sysfs_blkdev_get_devchain(ساختار path_cxt *pc, نویسه *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ساختار ul_buffer tmp = UL_INIT_BUFFER;
	ثابت نویسه *p;
	نویسه *res = پوچ;

	ssz = ul_path_readlink(pc, buf, bufsz, پوچ);
	اگر (ssz <= 0)
		بازگشت پوچ;

	اگر ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, پوچ);
	اگر (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	بازگشت res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
صحیح sysfs_blkdev_next_subsystem(ساختار path_cxt *pc __attribute__((unused)),
			 نویسه *devchain, نویسه **subsys)
{
	نویسه subbuf[PATH_MAX];
	نویسه *sub;

	اگر (!subsys || !devchain)
		بازگشت -EINVAL;

	*subsys = پوچ;

	وقتی ((sub = get_subsystem(devchain, subbuf, اندازه(subbuf)))) {
		*subsys = strdup(sub);
		اگر (!*subsys)
			بازگشت -ENOMEM;
		بازگشت 0;
	}

	بازگشت 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ایستا صحیح sysfs_devchain_is_removable(نویسه *chain)
{
	size_t len;
	نویسه buf[20];
	نویسه *p;

	اگر (!chain || !*chain)
		بازگشت 0;

	len = strlen(chain);
	اگر (len + اندازه(REMOVABLE_FILENAME) > PATH_MAX)
		بازگشت 0;

	انجامبده {
		صحیح fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, اندازه(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		اگر (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			بشکن;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		اگر (fd != -1) {
			rc = ul_read_all(fd, buf, اندازه(buf));
			close(fd);

			اگر (rc > 0) {
				اگر (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					بازگشت 0;
				} وگرنه اگر (strncmp(buf, "removable", min(rc, 9)) == 0) {
					بازگشت 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		اگر (p) {
			*p = '\0';
			len = p - chain;
		}

	} وقتی (p);

	بازگشت 0;
}

صحیح sysfs_blkdev_is_hotpluggable(ساختار path_cxt *pc)
{
	نویسه buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, اندازه(buf));
	بازگشت sysfs_devchain_is_removable(chain);
}

صحیح sysfs_blkdev_is_removable(ساختار path_cxt *pc)
{
	صحیح rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	اگر (ul_path_read_s32(pc, &rc, "removable") == 0)
		بازگشت rc;

	بازگشت 0;
}

ایستا صحیح get_dm_wholedisk(ساختار path_cxt *pc, نویسه *diskname,
                            size_t len, dev_t *diskdevno)
{
	صحیح rc = 0;
	نویسه *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	اگر (!name)
		بازگشت -1;

	اگر (diskname && len)
		xstrncpy(diskname, name, len);

	اگر (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, پوچ);
		اگر (!*diskdevno)
			rc = -1;
	}

	آزاد(name);
	بازگشت rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
صحیح sysfs_blkdev_get_wholedisk(	ساختار path_cxt *pc,
				نویسه *diskname,
				size_t len,
				dev_t *diskdevno)
{
	صحیح is_part = 0;

	اگر (!pc)
		بازگشت -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	اگر (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		نویسه *uuid = پوچ, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : پوچ;

		اگر (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		آزاد(uuid);

		اگر (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			برو done;

		is_part = 0;
	}

	اگر (!is_part) {
		/*
		 * unpartitioned device
		 */
		اگر (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			برو err;
		اگر (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} وگرنه {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		نویسه linkpath[PATH_MAX];
		نویسه *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, اندازه(linkpath), پوچ);
		اگر (linklen < 0)
			برو err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		اگر (!name)
			برو err;

		sysfs_devname_sys_to_dev(name);
		اگر (diskname && len)
			xstrncpy(diskname, name, len);

		اگر (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, پوچ);
			اگر (!*diskdevno)
				برو err;
		}
	}

done:
	بازگشت 0;
err:
	بازگشت -1;
}

صحیح sysfs_devno_to_wholedisk(dev_t devno, نویسه *diskname,
		             size_t len, dev_t *diskdevno)
{
	ساختار path_cxt *pc;
	صحیح rc = 0;

	اگر (!devno)
		بازگشت -EINVAL;
	pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	اگر (!pc)
		بازگشت -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	بازگشت rc;
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
صحیح sysfs_devno_is_dm_hidden(dev_t devno, نویسه **uuid,
			     ثابت ساختار ul_vfs_ops *vfs)
{
	ساختار path_cxt *pc = پوچ;
	نویسه *id = پوچ;
	صحیح rc = 0;

	pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	اگر (!pc)
		برو done;
	ul_path_refer_vfs(pc, vfs);
	اگر (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		برو done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	اگر (strncmp(id, "LVM-", 4) == 0) {
		نویسه *p = strrchr(id + 4, '-');

		اگر (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	اگر (uuid)
		*uuid = id;
	وگرنه
		آزاد(id);
	بازگشت rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
صحیح sysfs_devno_is_dm_private(dev_t devno, نویسه **uuid,
			      ثابت ساختار ul_vfs_ops *vfs)
{
	ساختار path_cxt *pc = پوچ;
	نویسه *id = پوچ;
	صحیح rc = 0;

	pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	اگر (!pc)
		برو done;
	ul_path_refer_vfs(pc, vfs);
	اگر (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		برو done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	اگر (strncmp(id, "LVM-", 4) == 0) {
		نویسه *p = strrchr(id + 4, '-');

		اگر (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} وگرنه اگر (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	اگر (uuid)
		*uuid = id;
	وگرنه
		آزاد(id);
	بازگشت rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
صحیح sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	اگر (sysfs_devno_to_wholedisk(devno, پوچ, 0, &disk) != 0)
		بازگشت -1;

	بازگشت devno == disk;
}


صحیح sysfs_blkdev_scsi_get_hctl(ساختار path_cxt *pc, صحیح *h, صحیح *c, صحیح *t, صحیح *l)
{
	نویسه buf[PATH_MAX], *hctl;
	ساختار sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	اگر (!blk || blk->hctl_error)
		بازگشت -EINVAL;
	اگر (blk->has_hctl)
		برو done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, اندازه(buf), "device");
	اگر (len < 0)
		بازگشت len;

	hctl = strrchr(buf, '/');
	اگر (!hctl)
		بازگشت -1;
	hctl++;

	اگر (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		بازگشت -1;

	blk->has_hctl = 1;
done:
	اگر (h)
		*h = blk->scsi_host;
	اگر (c)
		*c = blk->scsi_channel;
	اگر (t)
		*t = blk->scsi_target;
	اگر (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	بازگشت 0;
}


ایستا نویسه *scsi_host_attribute_path(
			ساختار path_cxt *pc,
			ثابت نویسه *type,
			نویسه *buf,
			size_t bufsz,
			ثابت نویسه *attr)
{
	صحیح len;
	صحیح host;
	ثابت نویسه *prefix;

	اگر (sysfs_blkdev_scsi_get_hctl(pc, &host, پوچ, پوچ, پوچ))
		بازگشت پوچ;

	prefix = ul_path_get_prefix(pc);
	اگر (!prefix)
		prefix = "";

	اگر (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	وگرنه
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	بازگشت (len < 0 || (size_t) len >= bufsz) ? پوچ : buf;
}

نویسه *sysfs_blkdev_scsi_host_strdup_attribute(ساختار path_cxt *pc,
			ثابت نویسه *type, ثابت نویسه *attr)
{
	نویسه buf[1024];
	صحیح rc;
	FILE *f;

	اگر (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, اندازه(buf), attr))
		بازگشت پوچ;

	اگر (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		بازگشت پوچ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	بازگشت rc == 1 ? strdup(buf) : پوچ;
}

صحیح sysfs_blkdev_scsi_host_is(ساختار path_cxt *pc, ثابت نویسه *type)
{
	نویسه buf[PATH_MAX];
	ساختار stat st;

	اگر (!type || !scsi_host_attribute_path(pc, type,
				buf, اندازه(buf), پوچ))
		بازگشت 0;

	بازگشت stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ایستا نویسه *scsi_attribute_path(ساختار path_cxt *pc,
		نویسه *buf, size_t bufsz, ثابت نویسه *attr)
{
	صحیح len, h, c, t, l;
	ثابت نویسه *prefix;

	اگر (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		بازگشت پوچ;

	prefix = ul_path_get_prefix(pc);
	اگر (!prefix)
		prefix = "";

	اگر (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	وگرنه
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	بازگشت (len < 0 || (size_t) len >= bufsz) ? پوچ : buf;
}

صحیح sysfs_blkdev_scsi_has_attribute(ساختار path_cxt *pc, ثابت نویسه *attr)
{
	نویسه path[PATH_MAX];
	ساختار stat st;

	اگر (!scsi_attribute_path(pc, path, اندازه(path), attr))
		بازگشت 0;

	بازگشت stat(path, &st) == 0;
}

صحیح sysfs_blkdev_scsi_path_contains(ساختار path_cxt *pc, ثابت نویسه *pattern)
{
	نویسه path[PATH_MAX], linkc[PATH_MAX];
	ساختار stat st;
	ssize_t len;

	اگر (!scsi_attribute_path(pc, path, اندازه(path), پوچ))
		بازگشت 0;

	اگر (stat(path, &st) != 0)
		بازگشت 0;

	len = readlink(path, linkc, اندازه(linkc) - 1);
	اگر (len < 0)
		بازگشت 0;

	linkc[len] = '\0';
	بازگشت strstr(linkc, pattern) != پوچ;
}

ایستا dev_t read_devno(ثابت نویسه *path)
{
	FILE *f;
	صحیح maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	اگر (!f)
		بازگشت 0;

	اگر (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	بازگشت dev;
}

صحیح sysfs_devname_is_hidden(ثابت نویسه *prefix, ثابت نویسه *name)
{
	نویسه buf[PATH_MAX];
	صحیح rc = 0, hidden = 0, len;
	FILE *f;

	اگر (strncmp("/dev/", name, 5) == 0)
		بازگشت 0;

	اگر (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, اندازه(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	اگر (len < 0 || (size_t) len + 1 > اندازه(buf))
		بازگشت 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	اگر (!f)
		بازگشت 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	بازگشت rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ثابت نویسه *prefix, ثابت نویسه *name, ثابت نویسه *parent)
{
	نویسه buf[PATH_MAX];
	نویسه *_name = پوچ, *_parent = پوچ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	صحیح len;

	اگر (!prefix)
		prefix = "";

	أكد(name);

	اگر (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ساختار stat st;

		اگر (stat(name, &st) == 0) {
			dev = st.st_rdev;
			برو done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	اگر (!_name)
		برو done;
	sysfs_devname_dev_to_sys(_name);

	اگر (parent) {
		_parent = strdup(parent);
		اگر (!_parent)
			برو done;
	}

	اگر (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, اندازه(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		اگر (len < 0 || (size_t) len >= اندازه(buf))
			برو done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		برو done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, اندازه(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	اگر (len < 0 || (size_t) len >= اندازه(buf))
		برو done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	اگر (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, اندازه(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		اگر (len < 0 || (size_t) len >= اندازه(buf))
			برو done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	اگر (!dev) {
		len = snprintf(buf, اندازه(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		اگر (len < 0 || (size_t) len >= اندازه(buf))
			برو done;
		dev = read_devno(buf);
	}
done:
	آزاد(_name);
	آزاد(_parent);
	بازگشت dev;
}

dev_t sysfs_devname_to_devno(ثابت نویسه *name)
{
	بازگشت __sysfs_devname_to_devno(پوچ, name, پوچ);
}

نویسه *sysfs_blkdev_get_path(ساختار path_cxt *pc, نویسه *buf, size_t bufsiz)
{
	ثابت نویسه *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	نویسه *res = پوچ;
	size_t sz;
	ساختار stat st;

	اگر (!name)
		برو done;

	sz = strlen(name);
	اگر (sz + اندازه("/dev/") > bufsiz)
		برو done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	اگر (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	بازگشت res;
}

dev_t sysfs_blkdev_get_devno(ساختار path_cxt *pc)
{
	بازگشت ((ساختار sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
نویسه *sysfs_devno_to_devpath(dev_t devno, نویسه *buf, size_t bufsiz)
{
	ساختار path_cxt *pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	نویسه *res = پوچ;

	اگر (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	بازگشت res;
}

نویسه *sysfs_devno_to_devname(dev_t devno, نویسه *buf, size_t bufsiz)
{
	ساختار path_cxt *pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	نویسه *res = پوچ;

	اگر (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	بازگشت res;
}

صحیح sysfs_devno_شمار_partitions(dev_t devno)
{
	ساختار path_cxt *pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	صحیح n = 0;

	اگر (pc) {
		نویسه buf[PATH_MAX + 1];
		نویسه *name = sysfs_blkdev_get_name(pc, buf, اندازه(buf));

		n = sysfs_blkdev_شمار_partitions(pc, name);
		ul_unref_path(pc);
	}
	بازگشت n;
}

نویسه *sysfs_chrdev_devno_to_devname(dev_t devno, نویسه *buf, size_t bufsiz)
{
	نویسه link[PATH_MAX];
	ساختار path_cxt *pc;
	نویسه *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	اگر (!pc)
		بازگشت پوچ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, اندازه(link), پوچ);
	ul_unref_path(pc);

	اگر (sz < 0)
		بازگشت پوچ;

	name = strrchr(link, '/');
	اگر (!name)
		بازگشت پوچ;

	name++;
	sz = strlen(name);
	اگر ((size_t) sz + 1 > bufsiz)
		بازگشت پوچ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	بازگشت buf;

}

شمارش sysfs_byteorder sysfs_get_byteorder(ساختار path_cxt *pc)
{
	صحیح rc;
	نویسه buf[BUFSIZ];
	شمارش sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, اندازه(buf), _PATH_SYS_CPU_BYTEORDER);
	اگر (rc < 0)
		برو unknown;

	اگر (strncmp(buf, "little", اندازه(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		برو out;
	} وگرنه اگر (strncmp(buf, "big", اندازه(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		برو out;
	}

unknown:
#اگر __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#وگرنهاگر __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#وگرنه
#error Unknown byte order
#endif

out:
	بازگشت ret;
}

صحیح sysfs_get_address_bits(ساختار path_cxt *pc)
{
	صحیح rc;
	صحیح address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	اگر (rc < 0)
		بازگشت rc;
	اگر (address_bits < 0)
		بازگشت -EINVAL;
	بازگشت address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#شامل <errno.h>
#شامل <stdlib.h>

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ساختار path_cxt *pc;
	نویسه *devname;
	dev_t devno, disk_devno;
	نویسه path[PATH_MAX], *sub, *chain;
	نویسه diskname[32];
	صحیح i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	اگر (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	اگر (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned صحیح) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, اندازه(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, اندازه(path)));

	sysfs_devno_to_wholedisk(devno, diskname, اندازه(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned صحیح) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, پوچ, پوچ);
	اگر (!pc)
		برو done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned صحیح) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, اندازه(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, اندازه(path)));

	sysfs_devno_to_wholedisk(devno, diskname, اندازه(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned صحیح) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	اگر (is_part && disk_devno) {
		ساختار path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, پوچ, پوچ);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_شمار_dirents(pc, "slaves"));

	اگر (!is_part) {
		printf("First 5 partitions:\n");
		برای (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			اگر (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	اگر (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	وگرنه
		printf(" SIZE: %"PRIu64"\n", u64);

	اگر (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	وگرنه
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, اندازه(path));
	printf(" SUBSYSTEMS:\n");

	وقتی (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		آزاد(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	بازگشت rc;
}
#endif /* TEST_PROGRAM_SYSFS */
