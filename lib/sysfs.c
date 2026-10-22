/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <libgen.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "sysfs.h"
#અન્તર્ભાવ "fileutils.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "debug.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "buffer.h"

સ્થિર શૂન્ય sysfs_blkdev_deinit_path(સંરચના path_cxt *pc);
સ્થિર પૂર્ણાંક  sysfs_blkdev_enoent_redirect(સંરચના path_cxt *pc, અચલ અક્ષર *path, પૂર્ણાંક *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
સ્થિર UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

શૂન્ય ul_sysfs_init_debug(શૂન્ય)
{
	જો (ulsysfs_debug_mask)
		પાછા;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

સંરચના path_cxt *ul_new_sysfs_path(dev_t devno, સંરચના path_cxt *parent, અચલ અક્ષર *prefix)
{
	સંરચના path_cxt *pc = ul_new_path(ખાલી);

	જો (!pc)
		પાછા ખાલી;
	જો (prefix)
		ul_path_set_prefix(pc, prefix);

	જો (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		પાછા ખાલી;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	પાછા pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
પૂર્ણાંક sysfs_blkdev_init_path(સંરચના path_cxt *pc, dev_t devno, સંરચના path_cxt *parent)
{
	સંરચના sysfs_blkdev *blk;
	પૂર્ણાંક rc;
	અક્ષર buf[કદ(_PATH_SYS_DEVBLOCK)
		 + કદ(stringify_કિંમત(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, કદ(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	જો (rc)
		પાછા rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	જો (rc < 0)
		પાછા rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	જો (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, કદ(સંરચના sysfs_blkdev));
		જો (!blk)
			પાછા -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	પાછા 0;
}

સ્થિર શૂન્ય sysfs_blkdev_deinit_path(સંરચના path_cxt *pc)
{
	સંરચના sysfs_blkdev *blk;

	જો (!pc)
		પાછા;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	જો (!blk)
		પાછા;

	ul_unref_path(blk->parent);
	મુક્ત(blk);

	ul_path_set_dialect(pc, ખાલી, ખાલી);
}

પૂર્ણાંક sysfs_blkdev_set_parent(સંરચના path_cxt *pc, સંરચના path_cxt *parent)
{
	સંરચના sysfs_blkdev *blk = ul_path_get_dialect(pc);

	જો (!pc || !blk)
		પાછા -EINVAL;

	જો (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = ખાલી;
	}

	જો (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} નહીંતર
		blk->parent = ખાલી;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	પાછા 0;
}

સંરચના path_cxt *sysfs_blkdev_get_parent(સંરચના path_cxt *pc)
{
	સંરચના sysfs_blkdev *blk = ul_path_get_dialect(pc);
	પાછા blk ? blk->parent : ખાલી;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
સ્થિર પૂર્ણાંક sysfs_blkdev_enoent_redirect(સંરચના path_cxt *pc, અચલ અક્ષર *path, પૂર્ણાંક *dirfd)
{
	સંરચના sysfs_blkdev *blk = ul_path_get_dialect(pc);

	જો (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		જો (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			પાછા 0;
		}
	}
	પાછા 1;	/* no redirect */
}

અક્ષર *sysfs_blkdev_get_name(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsiz)
{
	અક્ષર link[PATH_MAX];
	અક્ષર *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, કદ(link), ખાલી);
	જો (sz < 0)
		પાછા ખાલી;

	name = strrchr(link, '/');
	જો (!name)
		પાછા ખાલી;

	name++;
	sz = strlen(name);
	જો ((size_t) sz + 1 > bufsiz)
		પાછા ખાલી;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	પાછા buf;
}

પૂર્ણાંક sysfs_blkdev_is_partition_dirent(DIR *dir, સંરચના dirent *d, અચલ અક્ષર *parent_name)
{
	અક્ષર path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	જો (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		પાછા 0;
#endif
	size_t len = 0;

	જો (parent_name) {
		અચલ અક્ષર *p = parent_name;

		/* /dev/sda --> "sda" */
		જો (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			જો (!p)
				પાછા 0;
			p++;
		}

		len = strlen(p);
		જો ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	જો (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		પાછા ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, કદ(path), "%s/start", d->d_name);

	પાછા faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

પૂર્ણાંક sysfs_blkdev_ગણતરી_partitions(સંરચના path_cxt *pc, અચલ અક્ષર *devname)
{
	DIR *dir;
	સંરચના dirent *d;
	પૂર્ણાંક r = 0;

	dir = ul_path_opendir(pc, ખાલી);
	જો (!dir)
		પાછા 0;

	જ્યારે ((d = xreaddir(dir))) {
		જો (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	પાછા r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(સંરચના path_cxt *pc, પૂર્ણાંક partno)
{
	DIR *dir;
	સંરચના dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, ખાલી);
	જો (!dir)
		પાછા 0;

	જ્યારે ((d = xreaddir(dir))) {
		પૂર્ણાંક n;

		જો (!sysfs_blkdev_is_partition_dirent(dir, d, ખાલી))
			ચાલુ;

		જો (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			ચાલુ;

		જો (n == partno) {
			જો (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				તોડો;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (પૂર્ણાંક) partno, (પૂર્ણાંક) devno));
	પાછા devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
અક્ષર *sysfs_blkdev_get_slave(સંરચના path_cxt *pc)
{
	DIR *dir;
	સંરચના dirent *d;
	અક્ષર *name = ખાલી;

	dir = ul_path_opendir(pc, "slaves");
	જો (!dir)
		પાછા ખાલી;

	જ્યારે ((d = xreaddir(dir))) {
		જો (name)
			જાઓ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	પાછા name;
err:
	મુક્ત(name);
	closedir(dir);
	પાછા ખાલી;
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
સ્થિર અક્ષર *get_subsystem(અક્ષર *chain, અક્ષર *buf, size_t bufsz)
{
	size_t len;
	અક્ષર *p;

	જો (!chain || !*chain)
		પાછા ખાલી;

	len = strlen(chain);
	જો (len + કદ(SUBSYSTEM_LINKNAME) > PATH_MAX)
		પાછા ખાલી;

	કરો {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, કદ(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		જો (p) {
			*p = '\0';
			len = p - chain;
		}

		જો (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			પાછા basename(buf);
		}

	} જ્યારે (p);

	પાછા ખાલી;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
અક્ષર *sysfs_blkdev_get_devchain(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	સંરચના ul_buffer tmp = UL_INIT_BUFFER;
	અચલ અક્ષર *p;
	અક્ષર *res = ખાલી;

	ssz = ul_path_readlink(pc, buf, bufsz, ખાલી);
	જો (ssz <= 0)
		પાછા ખાલી;

	જો ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, ખાલી);
	જો (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	પાછા res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
પૂર્ણાંક sysfs_blkdev_next_subsystem(સંરચના path_cxt *pc __attribute__((unused)),
			 અક્ષર *devchain, અક્ષર **subsys)
{
	અક્ષર subbuf[PATH_MAX];
	અક્ષર *sub;

	જો (!subsys || !devchain)
		પાછા -EINVAL;

	*subsys = ખાલી;

	જ્યારે ((sub = get_subsystem(devchain, subbuf, કદ(subbuf)))) {
		*subsys = strdup(sub);
		જો (!*subsys)
			પાછા -ENOMEM;
		પાછા 0;
	}

	પાછા 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
સ્થિર પૂર્ણાંક sysfs_devchain_is_removable(અક્ષર *chain)
{
	size_t len;
	અક્ષર buf[20];
	અક્ષર *p;

	જો (!chain || !*chain)
		પાછા 0;

	len = strlen(chain);
	જો (len + કદ(REMOVABLE_FILENAME) > PATH_MAX)
		પાછા 0;

	કરો {
		પૂર્ણાંક fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, કદ(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		જો (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			તોડો;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		જો (fd != -1) {
			rc = ul_read_all(fd, buf, કદ(buf));
			close(fd);

			જો (rc > 0) {
				જો (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					પાછા 0;
				} નહીંતર જો (strncmp(buf, "removable", min(rc, 9)) == 0) {
					પાછા 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		જો (p) {
			*p = '\0';
			len = p - chain;
		}

	} જ્યારે (p);

	પાછા 0;
}

પૂર્ણાંક sysfs_blkdev_is_hotpluggable(સંરચના path_cxt *pc)
{
	અક્ષર buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, કદ(buf));
	પાછા sysfs_devchain_is_removable(chain);
}

પૂર્ણાંક sysfs_blkdev_is_removable(સંરચના path_cxt *pc)
{
	પૂર્ણાંક rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	જો (ul_path_read_s32(pc, &rc, "removable") == 0)
		પાછા rc;

	પાછા 0;
}

સ્થિર પૂર્ણાંક get_dm_wholedisk(સંરચના path_cxt *pc, અક્ષર *diskname,
                            size_t len, dev_t *diskdevno)
{
	પૂર્ણાંક rc = 0;
	અક્ષર *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	જો (!name)
		પાછા -1;

	જો (diskname && len)
		xstrncpy(diskname, name, len);

	જો (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ખાલી);
		જો (!*diskdevno)
			rc = -1;
	}

	મુક્ત(name);
	પાછા rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
પૂર્ણાંક sysfs_blkdev_get_wholedisk(	સંરચના path_cxt *pc,
				અક્ષર *diskname,
				size_t len,
				dev_t *diskdevno)
{
	પૂર્ણાંક is_part = 0;

	જો (!pc)
		પાછા -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	જો (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		અક્ષર *uuid = ખાલી, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : ખાલી;

		જો (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		મુક્ત(uuid);

		જો (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			જાઓ done;

		is_part = 0;
	}

	જો (!is_part) {
		/*
		 * unpartitioned device
		 */
		જો (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			જાઓ err;
		જો (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} નહીંતર {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		અક્ષર linkpath[PATH_MAX];
		અક્ષર *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, કદ(linkpath), ખાલી);
		જો (linklen < 0)
			જાઓ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		જો (!name)
			જાઓ err;

		sysfs_devname_sys_to_dev(name);
		જો (diskname && len)
			xstrncpy(diskname, name, len);

		જો (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ખાલી);
			જો (!*diskdevno)
				જાઓ err;
		}
	}

done:
	પાછા 0;
err:
	પાછા -1;
}

પૂર્ણાંક sysfs_devno_to_wholedisk(dev_t devno, અક્ષર *diskname,
		             size_t len, dev_t *diskdevno)
{
	સંરચના path_cxt *pc;
	પૂર્ણાંક rc = 0;

	જો (!devno)
		પાછા -EINVAL;
	pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	જો (!pc)
		પાછા -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	પાછા rc;
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
પૂર્ણાંક sysfs_devno_is_dm_hidden(dev_t devno, અક્ષર **uuid,
			     અચલ સંરચના ul_vfs_ops *vfs)
{
	સંરચના path_cxt *pc = ખાલી;
	અક્ષર *id = ખાલી;
	પૂર્ણાંક rc = 0;

	pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	જો (!pc)
		જાઓ done;
	ul_path_refer_vfs(pc, vfs);
	જો (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		જાઓ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	જો (strncmp(id, "LVM-", 4) == 0) {
		અક્ષર *p = strrchr(id + 4, '-');

		જો (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	જો (uuid)
		*uuid = id;
	નહીંતર
		મુક્ત(id);
	પાછા rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
પૂર્ણાંક sysfs_devno_is_dm_private(dev_t devno, અક્ષર **uuid,
			      અચલ સંરચના ul_vfs_ops *vfs)
{
	સંરચના path_cxt *pc = ખાલી;
	અક્ષર *id = ખાલી;
	પૂર્ણાંક rc = 0;

	pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	જો (!pc)
		જાઓ done;
	ul_path_refer_vfs(pc, vfs);
	જો (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		જાઓ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	જો (strncmp(id, "LVM-", 4) == 0) {
		અક્ષર *p = strrchr(id + 4, '-');

		જો (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} નહીંતર જો (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	જો (uuid)
		*uuid = id;
	નહીંતર
		મુક્ત(id);
	પાછા rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
પૂર્ણાંક sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	જો (sysfs_devno_to_wholedisk(devno, ખાલી, 0, &disk) != 0)
		પાછા -1;

	પાછા devno == disk;
}


પૂર્ણાંક sysfs_blkdev_scsi_get_hctl(સંરચના path_cxt *pc, પૂર્ણાંક *h, પૂર્ણાંક *c, પૂર્ણાંક *t, પૂર્ણાંક *l)
{
	અક્ષર buf[PATH_MAX], *hctl;
	સંરચના sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	જો (!blk || blk->hctl_error)
		પાછા -EINVAL;
	જો (blk->has_hctl)
		જાઓ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, કદ(buf), "device");
	જો (len < 0)
		પાછા len;

	hctl = strrchr(buf, '/');
	જો (!hctl)
		પાછા -1;
	hctl++;

	જો (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		પાછા -1;

	blk->has_hctl = 1;
done:
	જો (h)
		*h = blk->scsi_host;
	જો (c)
		*c = blk->scsi_channel;
	જો (t)
		*t = blk->scsi_target;
	જો (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	પાછા 0;
}


સ્થિર અક્ષર *scsi_host_attribute_path(
			સંરચના path_cxt *pc,
			અચલ અક્ષર *type,
			અક્ષર *buf,
			size_t bufsz,
			અચલ અક્ષર *attr)
{
	પૂર્ણાંક len;
	પૂર્ણાંક host;
	અચલ અક્ષર *prefix;

	જો (sysfs_blkdev_scsi_get_hctl(pc, &host, ખાલી, ખાલી, ખાલી))
		પાછા ખાલી;

	prefix = ul_path_get_prefix(pc);
	જો (!prefix)
		prefix = "";

	જો (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	નહીંતર
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	પાછા (len < 0 || (size_t) len >= bufsz) ? ખાલી : buf;
}

અક્ષર *sysfs_blkdev_scsi_host_strdup_attribute(સંરચના path_cxt *pc,
			અચલ અક્ષર *type, અચલ અક્ષર *attr)
{
	અક્ષર buf[1024];
	પૂર્ણાંક rc;
	FILE *f;

	જો (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, કદ(buf), attr))
		પાછા ખાલી;

	જો (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		પાછા ખાલી;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	પાછા rc == 1 ? strdup(buf) : ખાલી;
}

પૂર્ણાંક sysfs_blkdev_scsi_host_is(સંરચના path_cxt *pc, અચલ અક્ષર *type)
{
	અક્ષર buf[PATH_MAX];
	સંરચના stat st;

	જો (!type || !scsi_host_attribute_path(pc, type,
				buf, કદ(buf), ખાલી))
		પાછા 0;

	પાછા stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

સ્થિર અક્ષર *scsi_attribute_path(સંરચના path_cxt *pc,
		અક્ષર *buf, size_t bufsz, અચલ અક્ષર *attr)
{
	પૂર્ણાંક len, h, c, t, l;
	અચલ અક્ષર *prefix;

	જો (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		પાછા ખાલી;

	prefix = ul_path_get_prefix(pc);
	જો (!prefix)
		prefix = "";

	જો (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	નહીંતર
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	પાછા (len < 0 || (size_t) len >= bufsz) ? ખાલી : buf;
}

પૂર્ણાંક sysfs_blkdev_scsi_has_attribute(સંરચના path_cxt *pc, અચલ અક્ષર *attr)
{
	અક્ષર path[PATH_MAX];
	સંરચના stat st;

	જો (!scsi_attribute_path(pc, path, કદ(path), attr))
		પાછા 0;

	પાછા stat(path, &st) == 0;
}

પૂર્ણાંક sysfs_blkdev_scsi_path_contains(સંરચના path_cxt *pc, અચલ અક્ષર *pattern)
{
	અક્ષર path[PATH_MAX], linkc[PATH_MAX];
	સંરચના stat st;
	ssize_t len;

	જો (!scsi_attribute_path(pc, path, કદ(path), ખાલી))
		પાછા 0;

	જો (stat(path, &st) != 0)
		પાછા 0;

	len = readlink(path, linkc, કદ(linkc) - 1);
	જો (len < 0)
		પાછા 0;

	linkc[len] = '\0';
	પાછા strstr(linkc, pattern) != ખાલી;
}

સ્થિર dev_t read_devno(અચલ અક્ષર *path)
{
	FILE *f;
	પૂર્ણાંક maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	જો (!f)
		પાછા 0;

	જો (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	પાછા dev;
}

પૂર્ણાંક sysfs_devname_is_hidden(અચલ અક્ષર *prefix, અચલ અક્ષર *name)
{
	અક્ષર buf[PATH_MAX];
	પૂર્ણાંક rc = 0, hidden = 0, len;
	FILE *f;

	જો (strncmp("/dev/", name, 5) == 0)
		પાછા 0;

	જો (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, કદ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	જો (len < 0 || (size_t) len + 1 > કદ(buf))
		પાછા 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	જો (!f)
		પાછા 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	પાછા rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(અચલ અક્ષર *prefix, અચલ અક્ષર *name, અચલ અક્ષર *parent)
{
	અક્ષર buf[PATH_MAX];
	અક્ષર *_name = ખાલી, *_parent = ખાલી;	/* name as encoded in sysfs */
	dev_t dev = 0;
	પૂર્ણાંક len;

	જો (!prefix)
		prefix = "";

	પ્રતિજ્ઞા(name);

	જો (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		સંરચના stat st;

		જો (stat(name, &st) == 0) {
			dev = st.st_rdev;
			જાઓ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	જો (!_name)
		જાઓ done;
	sysfs_devname_dev_to_sys(_name);

	જો (parent) {
		_parent = strdup(parent);
		જો (!_parent)
			જાઓ done;
	}

	જો (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, કદ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		જો (len < 0 || (size_t) len >= કદ(buf))
			જાઓ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		જાઓ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, કદ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	જો (len < 0 || (size_t) len >= કદ(buf))
		જાઓ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	જો (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, કદ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		જો (len < 0 || (size_t) len >= કદ(buf))
			જાઓ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	જો (!dev) {
		len = snprintf(buf, કદ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		જો (len < 0 || (size_t) len >= કદ(buf))
			જાઓ done;
		dev = read_devno(buf);
	}
done:
	મુક્ત(_name);
	મુક્ત(_parent);
	પાછા dev;
}

dev_t sysfs_devname_to_devno(અચલ અક્ષર *name)
{
	પાછા __sysfs_devname_to_devno(ખાલી, name, ખાલી);
}

અક્ષર *sysfs_blkdev_get_path(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsiz)
{
	અચલ અક્ષર *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	અક્ષર *res = ખાલી;
	size_t sz;
	સંરચના stat st;

	જો (!name)
		જાઓ done;

	sz = strlen(name);
	જો (sz + કદ("/dev/") > bufsiz)
		જાઓ done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	જો (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	પાછા res;
}

dev_t sysfs_blkdev_get_devno(સંરચના path_cxt *pc)
{
	પાછા ((સંરચના sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
અક્ષર *sysfs_devno_to_devpath(dev_t devno, અક્ષર *buf, size_t bufsiz)
{
	સંરચના path_cxt *pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	અક્ષર *res = ખાલી;

	જો (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	પાછા res;
}

અક્ષર *sysfs_devno_to_devname(dev_t devno, અક્ષર *buf, size_t bufsiz)
{
	સંરચના path_cxt *pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	અક્ષર *res = ખાલી;

	જો (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	પાછા res;
}

પૂર્ણાંક sysfs_devno_ગણતરી_partitions(dev_t devno)
{
	સંરચના path_cxt *pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	પૂર્ણાંક n = 0;

	જો (pc) {
		અક્ષર buf[PATH_MAX + 1];
		અક્ષર *name = sysfs_blkdev_get_name(pc, buf, કદ(buf));

		n = sysfs_blkdev_ગણતરી_partitions(pc, name);
		ul_unref_path(pc);
	}
	પાછા n;
}

અક્ષર *sysfs_chrdev_devno_to_devname(dev_t devno, અક્ષર *buf, size_t bufsiz)
{
	અક્ષર link[PATH_MAX];
	સંરચના path_cxt *pc;
	અક્ષર *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	જો (!pc)
		પાછા ખાલી;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, કદ(link), ખાલી);
	ul_unref_path(pc);

	જો (sz < 0)
		પાછા ખાલી;

	name = strrchr(link, '/');
	જો (!name)
		પાછા ખાલી;

	name++;
	sz = strlen(name);
	જો ((size_t) sz + 1 > bufsiz)
		પાછા ખાલી;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	પાછા buf;

}

ગણના sysfs_byteorder sysfs_get_byteorder(સંરચના path_cxt *pc)
{
	પૂર્ણાંક rc;
	અક્ષર buf[BUFSIZ];
	ગણના sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, કદ(buf), _PATH_SYS_CPU_BYTEORDER);
	જો (rc < 0)
		જાઓ unknown;

	જો (strncmp(buf, "little", કદ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		જાઓ out;
	} નહીંતર જો (strncmp(buf, "big", કદ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		જાઓ out;
	}

unknown:
#જો __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#અથવાજો __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#નહીંતર
#error Unknown byte order
#endif

out:
	પાછા ret;
}

પૂર્ણાંક sysfs_get_address_bits(સંરચના path_cxt *pc)
{
	પૂર્ણાંક rc;
	પૂર્ણાંક address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	જો (rc < 0)
		પાછા rc;
	જો (address_bits < 0)
		પાછા -EINVAL;
	પાછા address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdlib.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના path_cxt *pc;
	અક્ષર *devname;
	dev_t devno, disk_devno;
	અક્ષર path[PATH_MAX], *sub, *chain;
	અક્ષર diskname[32];
	પૂર્ણાંક i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	જો (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	જો (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned પૂર્ણાંક) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, કદ(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, કદ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, કદ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned પૂર્ણાંક) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, ખાલી, ખાલી);
	જો (!pc)
		જાઓ done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned પૂર્ણાંક) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, કદ(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, કદ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, કદ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned પૂર્ણાંક) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	જો (is_part && disk_devno) {
		સંરચના path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, ખાલી, ખાલી);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_ગણતરી_dirents(pc, "slaves"));

	જો (!is_part) {
		printf("First 5 partitions:\n");
		માટે (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			જો (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	જો (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	નહીંતર
		printf(" SIZE: %"PRIu64"\n", u64);

	જો (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	નહીંતર
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, કદ(path));
	printf(" SUBSYSTEMS:\n");

	જ્યારે (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		મુક્ત(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	પાછા rc;
}
#endif /* TEST_PROGRAM_SYSFS */
