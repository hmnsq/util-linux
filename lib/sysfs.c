/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <libgen.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "sysfs.h"
#அந்தர்பாவ "fileutils.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "debug.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "buffer.h"

நிலையான வெறுமை sysfs_blkdev_deinit_path(கட்டமைப்பு path_cxt *pc);
நிலையான முழு  sysfs_blkdev_enoent_redirect(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, முழு *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
நிலையான UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

வெறுமை ul_sysfs_init_debug(வெறுமை)
{
	எனில் (ulsysfs_debug_mask)
		திரும்பு;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

கட்டமைப்பு path_cxt *ul_new_sysfs_path(dev_t devno, கட்டமைப்பு path_cxt *parent, மாறிலி எழுத்து *prefix)
{
	கட்டமைப்பு path_cxt *pc = ul_new_path(வெற்று);

	எனில் (!pc)
		திரும்பு வெற்று;
	எனில் (prefix)
		ul_path_set_prefix(pc, prefix);

	எனில் (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		திரும்பு வெற்று;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	திரும்பு pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
முழு sysfs_blkdev_init_path(கட்டமைப்பு path_cxt *pc, dev_t devno, கட்டமைப்பு path_cxt *parent)
{
	கட்டமைப்பு sysfs_blkdev *blk;
	முழு rc;
	எழுத்து buf[அளவு(_PATH_SYS_DEVBLOCK)
		 + அளவு(stringify_மதிப்பு(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, அளவு(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	எனில் (rc)
		திரும்பு rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	எனில் (rc < 0)
		திரும்பு rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	எனில் (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, அளவு(கட்டமைப்பு sysfs_blkdev));
		எனில் (!blk)
			திரும்பு -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	திரும்பு 0;
}

நிலையான வெறுமை sysfs_blkdev_deinit_path(கட்டமைப்பு path_cxt *pc)
{
	கட்டமைப்பு sysfs_blkdev *blk;

	எனில் (!pc)
		திரும்பு;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	எனில் (!blk)
		திரும்பு;

	ul_unref_path(blk->parent);
	விடுவி(blk);

	ul_path_set_dialect(pc, வெற்று, வெற்று);
}

முழு sysfs_blkdev_set_parent(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு path_cxt *parent)
{
	கட்டமைப்பு sysfs_blkdev *blk = ul_path_get_dialect(pc);

	எனில் (!pc || !blk)
		திரும்பு -EINVAL;

	எனில் (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = வெற்று;
	}

	எனில் (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} இல்லையெனில்
		blk->parent = வெற்று;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	திரும்பு 0;
}

கட்டமைப்பு path_cxt *sysfs_blkdev_get_parent(கட்டமைப்பு path_cxt *pc)
{
	கட்டமைப்பு sysfs_blkdev *blk = ul_path_get_dialect(pc);
	திரும்பு blk ? blk->parent : வெற்று;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
நிலையான முழு sysfs_blkdev_enoent_redirect(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, முழு *dirfd)
{
	கட்டமைப்பு sysfs_blkdev *blk = ul_path_get_dialect(pc);

	எனில் (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		எனில் (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			திரும்பு 0;
		}
	}
	திரும்பு 1;	/* no redirect */
}

எழுத்து *sysfs_blkdev_get_name(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsiz)
{
	எழுத்து link[PATH_MAX];
	எழுத்து *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, அளவு(link), வெற்று);
	எனில் (sz < 0)
		திரும்பு வெற்று;

	name = strrchr(link, '/');
	எனில் (!name)
		திரும்பு வெற்று;

	name++;
	sz = strlen(name);
	எனில் ((size_t) sz + 1 > bufsiz)
		திரும்பு வெற்று;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	திரும்பு buf;
}

முழு sysfs_blkdev_is_partition_dirent(DIR *dir, கட்டமைப்பு dirent *d, மாறிலி எழுத்து *parent_name)
{
	எழுத்து path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	எனில் (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		திரும்பு 0;
#endif
	size_t len = 0;

	எனில் (parent_name) {
		மாறிலி எழுத்து *p = parent_name;

		/* /dev/sda --> "sda" */
		எனில் (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			எனில் (!p)
				திரும்பு 0;
			p++;
		}

		len = strlen(p);
		எனில் ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	எனில் (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		திரும்பு ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, அளவு(path), "%s/start", d->d_name);

	திரும்பு faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

முழு sysfs_blkdev_மொத்தம்_partitions(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *devname)
{
	DIR *dir;
	கட்டமைப்பு dirent *d;
	முழு r = 0;

	dir = ul_path_opendir(pc, வெற்று);
	எனில் (!dir)
		திரும்பு 0;

	போது ((d = xreaddir(dir))) {
		எனில் (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	திரும்பு r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(கட்டமைப்பு path_cxt *pc, முழு partno)
{
	DIR *dir;
	கட்டமைப்பு dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, வெற்று);
	எனில் (!dir)
		திரும்பு 0;

	போது ((d = xreaddir(dir))) {
		முழு n;

		எனில் (!sysfs_blkdev_is_partition_dirent(dir, d, வெற்று))
			தொடர்;

		எனில் (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			தொடர்;

		எனில் (n == partno) {
			எனில் (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				நிறுத்து;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (முழு) partno, (முழு) devno));
	திரும்பு devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
எழுத்து *sysfs_blkdev_get_slave(கட்டமைப்பு path_cxt *pc)
{
	DIR *dir;
	கட்டமைப்பு dirent *d;
	எழுத்து *name = வெற்று;

	dir = ul_path_opendir(pc, "slaves");
	எனில் (!dir)
		திரும்பு வெற்று;

	போது ((d = xreaddir(dir))) {
		எனில் (name)
			செல் err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	திரும்பு name;
err:
	விடுவி(name);
	closedir(dir);
	திரும்பு வெற்று;
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
நிலையான எழுத்து *get_subsystem(எழுத்து *chain, எழுத்து *buf, size_t bufsz)
{
	size_t len;
	எழுத்து *p;

	எனில் (!chain || !*chain)
		திரும்பு வெற்று;

	len = strlen(chain);
	எனில் (len + அளவு(SUBSYSTEM_LINKNAME) > PATH_MAX)
		திரும்பு வெற்று;

	செய் {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, அளவு(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		எனில் (p) {
			*p = '\0';
			len = p - chain;
		}

		எனில் (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			திரும்பு basename(buf);
		}

	} போது (p);

	திரும்பு வெற்று;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
எழுத்து *sysfs_blkdev_get_devchain(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	கட்டமைப்பு ul_buffer tmp = UL_INIT_BUFFER;
	மாறிலி எழுத்து *p;
	எழுத்து *res = வெற்று;

	ssz = ul_path_readlink(pc, buf, bufsz, வெற்று);
	எனில் (ssz <= 0)
		திரும்பு வெற்று;

	எனில் ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, வெற்று);
	எனில் (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	திரும்பு res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
முழு sysfs_blkdev_next_subsystem(கட்டமைப்பு path_cxt *pc __attribute__((unused)),
			 எழுத்து *devchain, எழுத்து **subsys)
{
	எழுத்து subbuf[PATH_MAX];
	எழுத்து *sub;

	எனில் (!subsys || !devchain)
		திரும்பு -EINVAL;

	*subsys = வெற்று;

	போது ((sub = get_subsystem(devchain, subbuf, அளவு(subbuf)))) {
		*subsys = strdup(sub);
		எனில் (!*subsys)
			திரும்பு -ENOMEM;
		திரும்பு 0;
	}

	திரும்பு 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
நிலையான முழு sysfs_devchain_is_removable(எழுத்து *chain)
{
	size_t len;
	எழுத்து buf[20];
	எழுத்து *p;

	எனில் (!chain || !*chain)
		திரும்பு 0;

	len = strlen(chain);
	எனில் (len + அளவு(REMOVABLE_FILENAME) > PATH_MAX)
		திரும்பு 0;

	செய் {
		முழு fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, அளவு(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		எனில் (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			நிறுத்து;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		எனில் (fd != -1) {
			rc = ul_read_all(fd, buf, அளவு(buf));
			close(fd);

			எனில் (rc > 0) {
				எனில் (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					திரும்பு 0;
				} இல்லையெனில் எனில் (strncmp(buf, "removable", min(rc, 9)) == 0) {
					திரும்பு 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		எனில் (p) {
			*p = '\0';
			len = p - chain;
		}

	} போது (p);

	திரும்பு 0;
}

முழு sysfs_blkdev_is_hotpluggable(கட்டமைப்பு path_cxt *pc)
{
	எழுத்து buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, அளவு(buf));
	திரும்பு sysfs_devchain_is_removable(chain);
}

முழு sysfs_blkdev_is_removable(கட்டமைப்பு path_cxt *pc)
{
	முழு rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	எனில் (ul_path_read_s32(pc, &rc, "removable") == 0)
		திரும்பு rc;

	திரும்பு 0;
}

நிலையான முழு get_dm_wholedisk(கட்டமைப்பு path_cxt *pc, எழுத்து *diskname,
                            size_t len, dev_t *diskdevno)
{
	முழு rc = 0;
	எழுத்து *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	எனில் (!name)
		திரும்பு -1;

	எனில் (diskname && len)
		xstrncpy(diskname, name, len);

	எனில் (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, வெற்று);
		எனில் (!*diskdevno)
			rc = -1;
	}

	விடுவி(name);
	திரும்பு rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
முழு sysfs_blkdev_get_wholedisk(	கட்டமைப்பு path_cxt *pc,
				எழுத்து *diskname,
				size_t len,
				dev_t *diskdevno)
{
	முழு is_part = 0;

	எனில் (!pc)
		திரும்பு -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	எனில் (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		எழுத்து *uuid = வெற்று, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : வெற்று;

		எனில் (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		விடுவி(uuid);

		எனில் (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			செல் done;

		is_part = 0;
	}

	எனில் (!is_part) {
		/*
		 * unpartitioned device
		 */
		எனில் (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			செல் err;
		எனில் (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} இல்லையெனில் {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		எழுத்து linkpath[PATH_MAX];
		எழுத்து *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, அளவு(linkpath), வெற்று);
		எனில் (linklen < 0)
			செல் err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		எனில் (!name)
			செல் err;

		sysfs_devname_sys_to_dev(name);
		எனில் (diskname && len)
			xstrncpy(diskname, name, len);

		எனில் (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, வெற்று);
			எனில் (!*diskdevno)
				செல் err;
		}
	}

done:
	திரும்பு 0;
err:
	திரும்பு -1;
}

முழு sysfs_devno_to_wholedisk(dev_t devno, எழுத்து *diskname,
		             size_t len, dev_t *diskdevno)
{
	கட்டமைப்பு path_cxt *pc;
	முழு rc = 0;

	எனில் (!devno)
		திரும்பு -EINVAL;
	pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எனில் (!pc)
		திரும்பு -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	திரும்பு rc;
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
முழு sysfs_devno_is_dm_hidden(dev_t devno, எழுத்து **uuid,
			     மாறிலி கட்டமைப்பு ul_vfs_ops *vfs)
{
	கட்டமைப்பு path_cxt *pc = வெற்று;
	எழுத்து *id = வெற்று;
	முழு rc = 0;

	pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எனில் (!pc)
		செல் done;
	ul_path_refer_vfs(pc, vfs);
	எனில் (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		செல் done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	எனில் (strncmp(id, "LVM-", 4) == 0) {
		எழுத்து *p = strrchr(id + 4, '-');

		எனில் (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	எனில் (uuid)
		*uuid = id;
	இல்லையெனில்
		விடுவி(id);
	திரும்பு rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
முழு sysfs_devno_is_dm_private(dev_t devno, எழுத்து **uuid,
			      மாறிலி கட்டமைப்பு ul_vfs_ops *vfs)
{
	கட்டமைப்பு path_cxt *pc = வெற்று;
	எழுத்து *id = வெற்று;
	முழு rc = 0;

	pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எனில் (!pc)
		செல் done;
	ul_path_refer_vfs(pc, vfs);
	எனில் (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		செல் done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	எனில் (strncmp(id, "LVM-", 4) == 0) {
		எழுத்து *p = strrchr(id + 4, '-');

		எனில் (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} இல்லையெனில் எனில் (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	எனில் (uuid)
		*uuid = id;
	இல்லையெனில்
		விடுவி(id);
	திரும்பு rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
முழு sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	எனில் (sysfs_devno_to_wholedisk(devno, வெற்று, 0, &disk) != 0)
		திரும்பு -1;

	திரும்பு devno == disk;
}


முழு sysfs_blkdev_scsi_get_hctl(கட்டமைப்பு path_cxt *pc, முழு *h, முழு *c, முழு *t, முழு *l)
{
	எழுத்து buf[PATH_MAX], *hctl;
	கட்டமைப்பு sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	எனில் (!blk || blk->hctl_error)
		திரும்பு -EINVAL;
	எனில் (blk->has_hctl)
		செல் done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, அளவு(buf), "device");
	எனில் (len < 0)
		திரும்பு len;

	hctl = strrchr(buf, '/');
	எனில் (!hctl)
		திரும்பு -1;
	hctl++;

	எனில் (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		திரும்பு -1;

	blk->has_hctl = 1;
done:
	எனில் (h)
		*h = blk->scsi_host;
	எனில் (c)
		*c = blk->scsi_channel;
	எனில் (t)
		*t = blk->scsi_target;
	எனில் (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	திரும்பு 0;
}


நிலையான எழுத்து *scsi_host_attribute_path(
			கட்டமைப்பு path_cxt *pc,
			மாறிலி எழுத்து *type,
			எழுத்து *buf,
			size_t bufsz,
			மாறிலி எழுத்து *attr)
{
	முழு len;
	முழு host;
	மாறிலி எழுத்து *prefix;

	எனில் (sysfs_blkdev_scsi_get_hctl(pc, &host, வெற்று, வெற்று, வெற்று))
		திரும்பு வெற்று;

	prefix = ul_path_get_prefix(pc);
	எனில் (!prefix)
		prefix = "";

	எனில் (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	இல்லையெனில்
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	திரும்பு (len < 0 || (size_t) len >= bufsz) ? வெற்று : buf;
}

எழுத்து *sysfs_blkdev_scsi_host_strdup_attribute(கட்டமைப்பு path_cxt *pc,
			மாறிலி எழுத்து *type, மாறிலி எழுத்து *attr)
{
	எழுத்து buf[1024];
	முழு rc;
	FILE *f;

	எனில் (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, அளவு(buf), attr))
		திரும்பு வெற்று;

	எனில் (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		திரும்பு வெற்று;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	திரும்பு rc == 1 ? strdup(buf) : வெற்று;
}

முழு sysfs_blkdev_scsi_host_is(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *type)
{
	எழுத்து buf[PATH_MAX];
	கட்டமைப்பு stat st;

	எனில் (!type || !scsi_host_attribute_path(pc, type,
				buf, அளவு(buf), வெற்று))
		திரும்பு 0;

	திரும்பு stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

நிலையான எழுத்து *scsi_attribute_path(கட்டமைப்பு path_cxt *pc,
		எழுத்து *buf, size_t bufsz, மாறிலி எழுத்து *attr)
{
	முழு len, h, c, t, l;
	மாறிலி எழுத்து *prefix;

	எனில் (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		திரும்பு வெற்று;

	prefix = ul_path_get_prefix(pc);
	எனில் (!prefix)
		prefix = "";

	எனில் (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	இல்லையெனில்
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	திரும்பு (len < 0 || (size_t) len >= bufsz) ? வெற்று : buf;
}

முழு sysfs_blkdev_scsi_has_attribute(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *attr)
{
	எழுத்து path[PATH_MAX];
	கட்டமைப்பு stat st;

	எனில் (!scsi_attribute_path(pc, path, அளவு(path), attr))
		திரும்பு 0;

	திரும்பு stat(path, &st) == 0;
}

முழு sysfs_blkdev_scsi_path_contains(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *pattern)
{
	எழுத்து path[PATH_MAX], linkc[PATH_MAX];
	கட்டமைப்பு stat st;
	ssize_t len;

	எனில் (!scsi_attribute_path(pc, path, அளவு(path), வெற்று))
		திரும்பு 0;

	எனில் (stat(path, &st) != 0)
		திரும்பு 0;

	len = readlink(path, linkc, அளவு(linkc) - 1);
	எனில் (len < 0)
		திரும்பு 0;

	linkc[len] = '\0';
	திரும்பு strstr(linkc, pattern) != வெற்று;
}

நிலையான dev_t read_devno(மாறிலி எழுத்து *path)
{
	FILE *f;
	முழு maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	எனில் (!f)
		திரும்பு 0;

	எனில் (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	திரும்பு dev;
}

முழு sysfs_devname_is_hidden(மாறிலி எழுத்து *prefix, மாறிலி எழுத்து *name)
{
	எழுத்து buf[PATH_MAX];
	முழு rc = 0, hidden = 0, len;
	FILE *f;

	எனில் (strncmp("/dev/", name, 5) == 0)
		திரும்பு 0;

	எனில் (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, அளவு(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	எனில் (len < 0 || (size_t) len + 1 > அளவு(buf))
		திரும்பு 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	எனில் (!f)
		திரும்பு 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	திரும்பு rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(மாறிலி எழுத்து *prefix, மாறிலி எழுத்து *name, மாறிலி எழுத்து *parent)
{
	எழுத்து buf[PATH_MAX];
	எழுத்து *_name = வெற்று, *_parent = வெற்று;	/* name as encoded in sysfs */
	dev_t dev = 0;
	முழு len;

	எனில் (!prefix)
		prefix = "";

	ப்ரதிஜ்ஞா(name);

	எனில் (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		கட்டமைப்பு stat st;

		எனில் (stat(name, &st) == 0) {
			dev = st.st_rdev;
			செல் done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	எனில் (!_name)
		செல் done;
	sysfs_devname_dev_to_sys(_name);

	எனில் (parent) {
		_parent = strdup(parent);
		எனில் (!_parent)
			செல் done;
	}

	எனில் (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, அளவு(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		எனில் (len < 0 || (size_t) len >= அளவு(buf))
			செல் done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		செல் done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, அளவு(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	எனில் (len < 0 || (size_t) len >= அளவு(buf))
		செல் done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	எனில் (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, அளவு(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		எனில் (len < 0 || (size_t) len >= அளவு(buf))
			செல் done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	எனில் (!dev) {
		len = snprintf(buf, அளவு(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		எனில் (len < 0 || (size_t) len >= அளவு(buf))
			செல் done;
		dev = read_devno(buf);
	}
done:
	விடுவி(_name);
	விடுவி(_parent);
	திரும்பு dev;
}

dev_t sysfs_devname_to_devno(மாறிலி எழுத்து *name)
{
	திரும்பு __sysfs_devname_to_devno(வெற்று, name, வெற்று);
}

எழுத்து *sysfs_blkdev_get_path(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsiz)
{
	மாறிலி எழுத்து *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	எழுத்து *res = வெற்று;
	size_t sz;
	கட்டமைப்பு stat st;

	எனில் (!name)
		செல் done;

	sz = strlen(name);
	எனில் (sz + அளவு("/dev/") > bufsiz)
		செல் done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	எனில் (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	திரும்பு res;
}

dev_t sysfs_blkdev_get_devno(கட்டமைப்பு path_cxt *pc)
{
	திரும்பு ((கட்டமைப்பு sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
எழுத்து *sysfs_devno_to_devpath(dev_t devno, எழுத்து *buf, size_t bufsiz)
{
	கட்டமைப்பு path_cxt *pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எழுத்து *res = வெற்று;

	எனில் (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	திரும்பு res;
}

எழுத்து *sysfs_devno_to_devname(dev_t devno, எழுத்து *buf, size_t bufsiz)
{
	கட்டமைப்பு path_cxt *pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எழுத்து *res = வெற்று;

	எனில் (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	திரும்பு res;
}

முழு sysfs_devno_மொத்தம்_partitions(dev_t devno)
{
	கட்டமைப்பு path_cxt *pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	முழு n = 0;

	எனில் (pc) {
		எழுத்து buf[PATH_MAX + 1];
		எழுத்து *name = sysfs_blkdev_get_name(pc, buf, அளவு(buf));

		n = sysfs_blkdev_மொத்தம்_partitions(pc, name);
		ul_unref_path(pc);
	}
	திரும்பு n;
}

எழுத்து *sysfs_chrdev_devno_to_devname(dev_t devno, எழுத்து *buf, size_t bufsiz)
{
	எழுத்து link[PATH_MAX];
	கட்டமைப்பு path_cxt *pc;
	எழுத்து *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	எனில் (!pc)
		திரும்பு வெற்று;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, அளவு(link), வெற்று);
	ul_unref_path(pc);

	எனில் (sz < 0)
		திரும்பு வெற்று;

	name = strrchr(link, '/');
	எனில் (!name)
		திரும்பு வெற்று;

	name++;
	sz = strlen(name);
	எனில் ((size_t) sz + 1 > bufsiz)
		திரும்பு வெற்று;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	திரும்பு buf;

}

எண்ணிக்கை sysfs_byteorder sysfs_get_byteorder(கட்டமைப்பு path_cxt *pc)
{
	முழு rc;
	எழுத்து buf[BUFSIZ];
	எண்ணிக்கை sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, அளவு(buf), _PATH_SYS_CPU_BYTEORDER);
	எனில் (rc < 0)
		செல் unknown;

	எனில் (strncmp(buf, "little", அளவு(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		செல் out;
	} இல்லையெனில் எனில் (strncmp(buf, "big", அளவு(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		செல் out;
	}

unknown:
#எனில் __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#அல்லதெனில் __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#இல்லையெனில்
#error Unknown byte order
#endif

out:
	திரும்பு ret;
}

முழு sysfs_get_address_bits(கட்டமைப்பு path_cxt *pc)
{
	முழு rc;
	முழு address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	எனில் (rc < 0)
		திரும்பு rc;
	எனில் (address_bits < 0)
		திரும்பு -EINVAL;
	திரும்பு address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdlib.h>

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு path_cxt *pc;
	எழுத்து *devname;
	dev_t devno, disk_devno;
	எழுத்து path[PATH_MAX], *sub, *chain;
	எழுத்து diskname[32];
	முழு i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	எனில் (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	எனில் (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned முழு) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, அளவு(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, அளவு(path)));

	sysfs_devno_to_wholedisk(devno, diskname, அளவு(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned முழு) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, வெற்று, வெற்று);
	எனில் (!pc)
		செல் done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned முழு) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, அளவு(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, அளவு(path)));

	sysfs_devno_to_wholedisk(devno, diskname, அளவு(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned முழு) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	எனில் (is_part && disk_devno) {
		கட்டமைப்பு path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, வெற்று, வெற்று);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_மொத்தம்_dirents(pc, "slaves"));

	எனில் (!is_part) {
		printf("First 5 partitions:\n");
		க்கு (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			எனில் (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	எனில் (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	இல்லையெனில்
		printf(" SIZE: %"PRIu64"\n", u64);

	எனில் (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	இல்லையெனில்
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, அளவு(path));
	printf(" SUBSYSTEMS:\n");

	போது (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		விடுவி(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	திரும்பு rc;
}
#endif /* TEST_PROGRAM_SYSFS */
