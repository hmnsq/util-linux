/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#include <ctype.h>
#include <inttypes.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "c.h"
#include "cctype.h"
#include "pathnames.h"
#include "sysfs.h"
#include "fileutils.h"
#include "all-io.h"
#include "debug.h"
#include "strutils.h"
#include "buffer.h"

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc);
static puur_naa_nka  sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, puur_naa_nka *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
static UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

void ul_sysfs_init_debug(void)
{
	ပေႃး (ulsysfs_debug_mask)
		ပွၵ်ႈ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_sysfs_path(dev_t devno, sa_nracanaa path_cxt *parent, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ပေႃး (!pc)
		ပွၵ်ႈ NULL;
	ပေႃး (prefix)
		ul_path_set_prefix(pc, prefix);

	ပေႃး (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ပွၵ်ႈ NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ပွၵ်ႈ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
puur_naa_nka sysfs_blkdev_init_path(sa_nracanaa path_cxt *pc, dev_t devno, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk;
	puur_naa_nka rc;
	var_na buf[parimaa_na(_PATH_SYS_DEVBLOCK)
		 + parimaa_na(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, parimaa_na(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	ပေႃး (rc)
		ပွၵ်ႈ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ပေႃး (rc < 0)
		ပွၵ်ႈ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	ပေႃး (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, parimaa_na(sa_nracanaa sysfs_blkdev));
		ပေႃး (!blk)
			ပွၵ်ႈ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ပွၵ်ႈ 0;
}

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk;

	ပေႃး (!pc)
		ပွၵ်ႈ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	ပေႃး (!blk)
		ပွၵ်ႈ;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

puur_naa_nka sysfs_blkdev_set_parent(sa_nracanaa path_cxt *pc, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ပေႃး (!pc || !blk)
		ပွၵ်ႈ -EINVAL;

	ပေႃး (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	ပေႃး (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} ပေႃးဢမ်ႇ
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ပွၵ်ႈ 0;
}

sa_nracanaa path_cxt *sysfs_blkdev_get_parent(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ပွၵ်ႈ blk ? blk->parent : NULL;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
static puur_naa_nka sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, puur_naa_nka *dirfd)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ပေႃး (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		ပေႃး (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ပွၵ်ႈ 0;
		}
	}
	ပွၵ်ႈ 1;	/* no redirect */
}

var_na *sysfs_blkdev_get_name(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	var_na *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ပေႃး (sz < 0)
		ပွၵ်ႈ NULL;

	name = strrchr(link, '/');
	ပေႃး (!name)
		ပွၵ်ႈ NULL;

	name++;
	sz = strlen(name);
	ပေႃး ((size_t) sz + 1 > bufsiz)
		ပွၵ်ႈ NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ပွၵ်ႈ buf;
}

puur_naa_nka sysfs_blkdev_is_partition_dirent(DIR *dir, sa_nracanaa dirent *d, const var_na *parent_name)
{
	var_na path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	ပေႃး (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ပွၵ်ႈ 0;
#endif
	size_t len = 0;

	ပေႃး (parent_name) {
		const var_na *p = parent_name;

		/* /dev/sda --> "sda" */
		ပေႃး (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			ပေႃး (!p)
				ပွၵ်ႈ 0;
			p++;
		}

		len = strlen(p);
		ပေႃး ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	ပေႃး (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ပွၵ်ႈ ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, parimaa_na(path), "%s/start", d->d_name);

	ပွၵ်ႈ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

puur_naa_nka sysfs_blkdev_count_partitions(sa_nracanaa path_cxt *pc, const var_na *devname)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	puur_naa_nka r = 0;

	dir = ul_path_opendir(pc, NULL);
	ပေႃး (!dir)
		ပွၵ်ႈ 0;

	ၶၢဝ်း ((d = xreaddir(dir))) {
		ပေႃး (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ပွၵ်ႈ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(sa_nracanaa path_cxt *pc, puur_naa_nka partno)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, NULL);
	ပေႃး (!dir)
		ပွၵ်ႈ 0;

	ၶၢဝ်း ((d = xreaddir(dir))) {
		puur_naa_nka n;

		ပေႃး (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		ပေႃး (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		ပေႃး (n == partno) {
			ပေႃး (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				bha_nga;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (puur_naa_nka) partno, (puur_naa_nka) devno));
	ပွၵ်ႈ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
var_na *sysfs_blkdev_get_slave(sa_nracanaa path_cxt *pc)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;

	dir = ul_path_opendir(pc, "slaves");
	ပေႃး (!dir)
		ပွၵ်ႈ NULL;

	ၶၢဝ်း ((d = xreaddir(dir))) {
		ပေႃး (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ပွၵ်ႈ name;
err:
	free(name);
	closedir(dir);
	ပွၵ်ႈ NULL;
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
static var_na *get_subsystem(var_na *chain, var_na *buf, size_t bufsz)
{
	size_t len;
	var_na *p;

	ပေႃး (!chain || !*chain)
		ပွၵ်ႈ NULL;

	len = strlen(chain);
	ပေႃး (len + parimaa_na(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ပွၵ်ႈ NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, parimaa_na(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ပေႃး (p) {
			*p = '\0';
			len = p - chain;
		}

		ပေႃး (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ပွၵ်ႈ basename(buf);
		}

	} ၶၢဝ်း (p);

	ပွၵ်ႈ NULL;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
var_na *sysfs_blkdev_get_devchain(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	sa_nracanaa ul_buffer tmp = UL_INIT_BUFFER;
	const var_na *p;
	var_na *res = NULL;

	ssz = ul_path_readlink(pc, buf, bufsz, NULL);
	ပေႃး (ssz <= 0)
		ပွၵ်ႈ NULL;

	ပေႃး ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	ပေႃး (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ပွၵ်ႈ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
puur_naa_nka sysfs_blkdev_next_subsystem(sa_nracanaa path_cxt *pc __attribute__((unused)),
			 var_na *devchain, var_na **subsys)
{
	var_na subbuf[PATH_MAX];
	var_na *sub;

	ပေႃး (!subsys || !devchain)
		ပွၵ်ႈ -EINVAL;

	*subsys = NULL;

	ၶၢဝ်း ((sub = get_subsystem(devchain, subbuf, parimaa_na(subbuf)))) {
		*subsys = strdup(sub);
		ပေႃး (!*subsys)
			ပွၵ်ႈ -ENOMEM;
		ပွၵ်ႈ 0;
	}

	ပွၵ်ႈ 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
static puur_naa_nka sysfs_devchain_is_removable(var_na *chain)
{
	size_t len;
	var_na buf[20];
	var_na *p;

	ပေႃး (!chain || !*chain)
		ပွၵ်ႈ 0;

	len = strlen(chain);
	ပေႃး (len + parimaa_na(REMOVABLE_FILENAME) > PATH_MAX)
		ပွၵ်ႈ 0;

	do {
		puur_naa_nka fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, parimaa_na(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ပေႃး (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			bha_nga;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ပေႃး (fd != -1) {
			rc = ul_read_all(fd, buf, parimaa_na(buf));
			close(fd);

			ပေႃး (rc > 0) {
				ပေႃး (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ပွၵ်ႈ 0;
				} ပေႃးဢမ်ႇ ပေႃး (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ပွၵ်ႈ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ပေႃး (p) {
			*p = '\0';
			len = p - chain;
		}

	} ၶၢဝ်း (p);

	ပွၵ်ႈ 0;
}

puur_naa_nka sysfs_blkdev_is_hotpluggable(sa_nracanaa path_cxt *pc)
{
	var_na buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, parimaa_na(buf));
	ပွၵ်ႈ sysfs_devchain_is_removable(chain);
}

puur_naa_nka sysfs_blkdev_is_removable(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ပေႃး (ul_path_read_s32(pc, &rc, "removable") == 0)
		ပွၵ်ႈ rc;

	ပွၵ်ႈ 0;
}

static puur_naa_nka get_dm_wholedisk(sa_nracanaa path_cxt *pc, var_na *diskname,
                            size_t len, dev_t *diskdevno)
{
	puur_naa_nka rc = 0;
	var_na *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ပေႃး (!name)
		ပွၵ်ႈ -1;

	ပေႃး (diskname && len)
		xstrncpy(diskname, name, len);

	ပေႃး (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		ပေႃး (!*diskdevno)
			rc = -1;
	}

	free(name);
	ပွၵ်ႈ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
puur_naa_nka sysfs_blkdev_get_wholedisk(	sa_nracanaa path_cxt *pc,
				var_na *diskname,
				size_t len,
				dev_t *diskdevno)
{
	puur_naa_nka is_part = 0;

	ပေႃး (!pc)
		ပွၵ်ႈ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	ပေႃး (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		var_na *uuid = NULL, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : NULL;

		ပေႃး (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		ပေႃး (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	ပေႃး (!is_part) {
		/*
		 * unpartitioned device
		 */
		ပေႃး (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		ပေႃး (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ပေႃးဢမ်ႇ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		var_na linkpath[PATH_MAX];
		var_na *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, parimaa_na(linkpath), NULL);
		ပေႃး (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		ပေႃး (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		ပေႃး (diskname && len)
			xstrncpy(diskname, name, len);

		ပေႃး (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			ပေႃး (!*diskdevno)
				goto err;
		}
	}

done:
	ပွၵ်ႈ 0;
err:
	ပွၵ်ႈ -1;
}

puur_naa_nka sysfs_devno_to_wholedisk(dev_t devno, var_na *diskname,
		             size_t len, dev_t *diskdevno)
{
	sa_nracanaa path_cxt *pc;
	puur_naa_nka rc = 0;

	ပေႃး (!devno)
		ပွၵ်ႈ -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ပေႃး (!pc)
		ပွၵ်ႈ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ပွၵ်ႈ rc;
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
puur_naa_nka sysfs_devno_is_dm_hidden(dev_t devno, var_na **uuid,
			     const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	puur_naa_nka rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ပေႃး (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	ပေႃး (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ပေႃး (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		ပေႃး (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	ပေႃး (uuid)
		*uuid = id;
	ပေႃးဢမ်ႇ
		free(id);
	ပွၵ်ႈ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
puur_naa_nka sysfs_devno_is_dm_private(dev_t devno, var_na **uuid,
			      const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	puur_naa_nka rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ပေႃး (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	ပေႃး (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ပေႃး (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		ပေႃး (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ပေႃးဢမ်ႇ ပေႃး (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	ပေႃး (uuid)
		*uuid = id;
	ပေႃးဢမ်ႇ
		free(id);
	ပွၵ်ႈ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
puur_naa_nka sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ပေႃး (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		ပွၵ်ႈ -1;

	ပွၵ်ႈ devno == disk;
}


puur_naa_nka sysfs_blkdev_scsi_get_hctl(sa_nracanaa path_cxt *pc, puur_naa_nka *h, puur_naa_nka *c, puur_naa_nka *t, puur_naa_nka *l)
{
	var_na buf[PATH_MAX], *hctl;
	sa_nracanaa sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	ပေႃး (!blk || blk->hctl_error)
		ပွၵ်ႈ -EINVAL;
	ပေႃး (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, parimaa_na(buf), "device");
	ပေႃး (len < 0)
		ပွၵ်ႈ len;

	hctl = strrchr(buf, '/');
	ပေႃး (!hctl)
		ပွၵ်ႈ -1;
	hctl++;

	ပေႃး (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ပွၵ်ႈ -1;

	blk->has_hctl = 1;
done:
	ပေႃး (h)
		*h = blk->scsi_host;
	ပေႃး (c)
		*c = blk->scsi_channel;
	ပေႃး (t)
		*t = blk->scsi_target;
	ပေႃး (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ပွၵ်ႈ 0;
}


static var_na *scsi_host_attribute_path(
			sa_nracanaa path_cxt *pc,
			const var_na *type,
			var_na *buf,
			size_t bufsz,
			const var_na *attr)
{
	puur_naa_nka len;
	puur_naa_nka host;
	const var_na *prefix;

	ပေႃး (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		ပွၵ်ႈ NULL;

	prefix = ul_path_get_prefix(pc);
	ပေႃး (!prefix)
		prefix = "";

	ပေႃး (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	ပေႃးဢမ်ႇ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ပွၵ်ႈ (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

var_na *sysfs_blkdev_scsi_host_strdup_attribute(sa_nracanaa path_cxt *pc,
			const var_na *type, const var_na *attr)
{
	var_na buf[1024];
	puur_naa_nka rc;
	FILE *f;

	ပေႃး (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, parimaa_na(buf), attr))
		ပွၵ်ႈ NULL;

	ပေႃး (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ပွၵ်ႈ NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ပွၵ်ႈ rc == 1 ? strdup(buf) : NULL;
}

puur_naa_nka sysfs_blkdev_scsi_host_is(sa_nracanaa path_cxt *pc, const var_na *type)
{
	var_na buf[PATH_MAX];
	sa_nracanaa stat st;

	ပေႃး (!type || !scsi_host_attribute_path(pc, type,
				buf, parimaa_na(buf), NULL))
		ပွၵ်ႈ 0;

	ပွၵ်ႈ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static var_na *scsi_attribute_path(sa_nracanaa path_cxt *pc,
		var_na *buf, size_t bufsz, const var_na *attr)
{
	puur_naa_nka len, h, c, t, l;
	const var_na *prefix;

	ပေႃး (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ပွၵ်ႈ NULL;

	prefix = ul_path_get_prefix(pc);
	ပေႃး (!prefix)
		prefix = "";

	ပေႃး (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	ပေႃးဢမ်ႇ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ပွၵ်ႈ (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

puur_naa_nka sysfs_blkdev_scsi_has_attribute(sa_nracanaa path_cxt *pc, const var_na *attr)
{
	var_na path[PATH_MAX];
	sa_nracanaa stat st;

	ပေႃး (!scsi_attribute_path(pc, path, parimaa_na(path), attr))
		ပွၵ်ႈ 0;

	ပွၵ်ႈ stat(path, &st) == 0;
}

puur_naa_nka sysfs_blkdev_scsi_path_contains(sa_nracanaa path_cxt *pc, const var_na *pattern)
{
	var_na path[PATH_MAX], linkc[PATH_MAX];
	sa_nracanaa stat st;
	ssize_t len;

	ပေႃး (!scsi_attribute_path(pc, path, parimaa_na(path), NULL))
		ပွၵ်ႈ 0;

	ပေႃး (stat(path, &st) != 0)
		ပွၵ်ႈ 0;

	len = readlink(path, linkc, parimaa_na(linkc) - 1);
	ပေႃး (len < 0)
		ပွၵ်ႈ 0;

	linkc[len] = '\0';
	ပွၵ်ႈ strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const var_na *path)
{
	FILE *f;
	puur_naa_nka maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	ပေႃး (!f)
		ပွၵ်ႈ 0;

	ပေႃး (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ပွၵ်ႈ dev;
}

puur_naa_nka sysfs_devname_is_hidden(const var_na *prefix, const var_na *name)
{
	var_na buf[PATH_MAX];
	puur_naa_nka rc = 0, hidden = 0, len;
	FILE *f;

	ပေႃး (strncmp("/dev/", name, 5) == 0)
		ပွၵ်ႈ 0;

	ပေႃး (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	ပေႃး (len < 0 || (size_t) len + 1 > parimaa_na(buf))
		ပွၵ်ႈ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ပေႃး (!f)
		ပွၵ်ႈ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ပွၵ်ႈ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const var_na *prefix, const var_na *name, const var_na *parent)
{
	var_na buf[PATH_MAX];
	var_na *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	puur_naa_nka len;

	ပေႃး (!prefix)
		prefix = "";

	assert(name);

	ပေႃး (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		sa_nracanaa stat st;

		ပေႃး (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ပေႃး (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	ပေႃး (parent) {
		_parent = strdup(parent);
		ပေႃး (!_parent)
			goto done;
	}

	ပေႃး (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		ပေႃး (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		goto done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	ပေႃး (len < 0 || (size_t) len >= parimaa_na(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ပေႃး (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		ပေႃး (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ပေႃး (!dev) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		ပေႃး (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	ပွၵ်ႈ dev;
}

dev_t sysfs_devname_to_devno(const var_na *name)
{
	ပွၵ်ႈ __sysfs_devname_to_devno(NULL, name, NULL);
}

var_na *sysfs_blkdev_get_path(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	const var_na *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	var_na *res = NULL;
	size_t sz;
	sa_nracanaa stat st;

	ပေႃး (!name)
		goto done;

	sz = strlen(name);
	ပေႃး (sz + parimaa_na("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	ပေႃး (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ပွၵ်ႈ res;
}

dev_t sysfs_blkdev_get_devno(sa_nracanaa path_cxt *pc)
{
	ပွၵ်ႈ ((sa_nracanaa sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
var_na *sysfs_devno_to_devpath(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	ပေႃး (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ပွၵ်ႈ res;
}

var_na *sysfs_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	ပေႃး (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ပွၵ်ႈ res;
}

puur_naa_nka sysfs_devno_count_partitions(dev_t devno)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	puur_naa_nka n = 0;

	ပေႃး (pc) {
		var_na buf[PATH_MAX + 1];
		var_na *name = sysfs_blkdev_get_name(pc, buf, parimaa_na(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	ပွၵ်ႈ n;
}

var_na *sysfs_chrdev_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	sa_nracanaa path_cxt *pc;
	var_na *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ပေႃး (!pc)
		ပွၵ်ႈ NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ul_unref_path(pc);

	ပေႃး (sz < 0)
		ပွၵ်ႈ NULL;

	name = strrchr(link, '/');
	ပေႃး (!name)
		ပွၵ်ႈ NULL;

	name++;
	sz = strlen(name);
	ပေႃး ((size_t) sz + 1 > bufsiz)
		ပွၵ်ႈ NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ပွၵ်ႈ buf;

}

ga_nanaa sysfs_byteorder sysfs_get_byteorder(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc;
	var_na buf[BUFSIZ];
	ga_nanaa sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), _PATH_SYS_CPU_BYTEORDER);
	ပေႃး (rc < 0)
		goto unknown;

	ပေႃး (strncmp(buf, "little", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} ပေႃးဢမ်ႇ ပေႃး (strncmp(buf, "big", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#ပေႃး __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ပေႃးဢမ်ႇ
#error Unknown byte order
#endif

out:
	ပွၵ်ႈ ret;
}

puur_naa_nka sysfs_get_address_bits(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc;
	puur_naa_nka address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	ပေႃး (rc < 0)
		ပွၵ်ႈ rc;
	ပေႃး (address_bits < 0)
		ပွၵ်ႈ -EINVAL;
	ပွၵ်ႈ address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#include <errno.h>
#include <stdlib.h>

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	sa_nracanaa path_cxt *pc;
	var_na *devname;
	dev_t devno, disk_devno;
	var_na path[PATH_MAX], *sub, *chain;
	var_na diskname[32];
	puur_naa_nka i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ပေႃး (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	ပေႃး (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned puur_naa_nka) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned puur_naa_nka) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ပေႃး (!pc)
		goto done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned puur_naa_nka) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned puur_naa_nka) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ပေႃး (is_part && disk_devno) {
		sa_nracanaa path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	ပေႃး (!is_part) {
		printf("First 5 partitions:\n");
		for (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			ပေႃး (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ပေႃး (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	ပေႃးဢမ်ႇ
		printf(" SIZE: %"PRIu64"\n", u64);

	ပေႃး (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	ပေႃးဢမ်ႇ
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, parimaa_na(path));
	printf(" SUBSYSTEMS:\n");

	ၶၢဝ်း (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ပွၵ်ႈ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
