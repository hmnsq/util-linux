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
static ཧྲིལ  sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, ཧྲིལ *dirfd);

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
	གལ་ཏེ (ulsysfs_debug_mask)
		ཕྱིར;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_sysfs_path(dev_t devno, sa_nracanaa path_cxt *parent, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	གལ་ཏེ (!pc)
		ཕྱིར NULL;
	གལ་ཏེ (prefix)
		ul_path_set_prefix(pc, prefix);

	གལ་ཏེ (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ཕྱིར NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ཕྱིར pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ཧྲིལ sysfs_blkdev_init_path(sa_nracanaa path_cxt *pc, dev_t devno, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk;
	ཧྲིལ rc;
	var_na buf[parimaa_na(_PATH_SYS_DEVBLOCK)
		 + parimaa_na(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, parimaa_na(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	གལ་ཏེ (rc)
		ཕྱིར rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	གལ་ཏེ (rc < 0)
		ཕྱིར rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	གལ་ཏེ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, parimaa_na(sa_nracanaa sysfs_blkdev));
		གལ་ཏེ (!blk)
			ཕྱིར -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ཕྱིར 0;
}

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk;

	གལ་ཏེ (!pc)
		ཕྱིར;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	གལ་ཏེ (!blk)
		ཕྱིར;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

ཧྲིལ sysfs_blkdev_set_parent(sa_nracanaa path_cxt *pc, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	གལ་ཏེ (!pc || !blk)
		ཕྱིར -EINVAL;

	གལ་ཏེ (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	གལ་ཏེ (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} དེ་མིན
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ཕྱིར 0;
}

sa_nracanaa path_cxt *sysfs_blkdev_get_parent(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ཕྱིར blk ? blk->parent : NULL;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
static ཧྲིལ sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, ཧྲིལ *dirfd)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	གལ་ཏེ (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		གལ་ཏེ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ཕྱིར 0;
		}
	}
	ཕྱིར 1;	/* no redirect */
}

var_na *sysfs_blkdev_get_name(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	var_na *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	གལ་ཏེ (sz < 0)
		ཕྱིར NULL;

	name = strrchr(link, '/');
	གལ་ཏེ (!name)
		ཕྱིར NULL;

	name++;
	sz = strlen(name);
	གལ་ཏེ ((size_t) sz + 1 > bufsiz)
		ཕྱིར NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ཕྱིར buf;
}

ཧྲིལ sysfs_blkdev_is_partition_dirent(DIR *dir, sa_nracanaa dirent *d, const var_na *parent_name)
{
	var_na path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	གལ་ཏེ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ཕྱིར 0;
#endif
	size_t len = 0;

	གལ་ཏེ (parent_name) {
		const var_na *p = parent_name;

		/* /dev/sda --> "sda" */
		གལ་ཏེ (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			གལ་ཏེ (!p)
				ཕྱིར 0;
			p++;
		}

		len = strlen(p);
		གལ་ཏེ ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	གལ་ཏེ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ཕྱིར ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, parimaa_na(path), "%s/start", d->d_name);

	ཕྱིར faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

ཧྲིལ sysfs_blkdev_count_partitions(sa_nracanaa path_cxt *pc, const var_na *devname)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	ཧྲིལ r = 0;

	dir = ul_path_opendir(pc, NULL);
	གལ་ཏེ (!dir)
		ཕྱིར 0;

	སྐབས ((d = xreaddir(dir))) {
		གལ་ཏེ (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ཕྱིར r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(sa_nracanaa path_cxt *pc, ཧྲིལ partno)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, NULL);
	གལ་ཏེ (!dir)
		ཕྱིར 0;

	སྐབས ((d = xreaddir(dir))) {
		ཧྲིལ n;

		གལ་ཏེ (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		གལ་ཏེ (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		གལ་ཏེ (n == partno) {
			གལ་ཏེ (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				bha_nga;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ཧྲིལ) partno, (ཧྲིལ) devno));
	ཕྱིར devno;
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
	གལ་ཏེ (!dir)
		ཕྱིར NULL;

	སྐབས ((d = xreaddir(dir))) {
		གལ་ཏེ (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ཕྱིར name;
err:
	free(name);
	closedir(dir);
	ཕྱིར NULL;
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

	གལ་ཏེ (!chain || !*chain)
		ཕྱིར NULL;

	len = strlen(chain);
	གལ་ཏེ (len + parimaa_na(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ཕྱིར NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, parimaa_na(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		གལ་ཏེ (p) {
			*p = '\0';
			len = p - chain;
		}

		གལ་ཏེ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ཕྱིར basename(buf);
		}

	} སྐབས (p);

	ཕྱིར NULL;
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
	གལ་ཏེ (ssz <= 0)
		ཕྱིར NULL;

	གལ་ཏེ ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	གལ་ཏེ (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ཕྱིར res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ཧྲིལ sysfs_blkdev_next_subsystem(sa_nracanaa path_cxt *pc __attribute__((unused)),
			 var_na *devchain, var_na **subsys)
{
	var_na subbuf[PATH_MAX];
	var_na *sub;

	གལ་ཏེ (!subsys || !devchain)
		ཕྱིར -EINVAL;

	*subsys = NULL;

	སྐབས ((sub = get_subsystem(devchain, subbuf, parimaa_na(subbuf)))) {
		*subsys = strdup(sub);
		གལ་ཏེ (!*subsys)
			ཕྱིར -ENOMEM;
		ཕྱིར 0;
	}

	ཕྱིར 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
static ཧྲིལ sysfs_devchain_is_removable(var_na *chain)
{
	size_t len;
	var_na buf[20];
	var_na *p;

	གལ་ཏེ (!chain || !*chain)
		ཕྱིར 0;

	len = strlen(chain);
	གལ་ཏེ (len + parimaa_na(REMOVABLE_FILENAME) > PATH_MAX)
		ཕྱིར 0;

	do {
		ཧྲིལ fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, parimaa_na(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		གལ་ཏེ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			bha_nga;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		གལ་ཏེ (fd != -1) {
			rc = ul_read_all(fd, buf, parimaa_na(buf));
			close(fd);

			གལ་ཏེ (rc > 0) {
				གལ་ཏེ (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ཕྱིར 0;
				} དེ་མིན གལ་ཏེ (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ཕྱིར 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		གལ་ཏེ (p) {
			*p = '\0';
			len = p - chain;
		}

	} སྐབས (p);

	ཕྱིར 0;
}

ཧྲིལ sysfs_blkdev_is_hotpluggable(sa_nracanaa path_cxt *pc)
{
	var_na buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, parimaa_na(buf));
	ཕྱིར sysfs_devchain_is_removable(chain);
}

ཧྲིལ sysfs_blkdev_is_removable(sa_nracanaa path_cxt *pc)
{
	ཧྲིལ rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	གལ་ཏེ (ul_path_read_s32(pc, &rc, "removable") == 0)
		ཕྱིར rc;

	ཕྱིར 0;
}

static ཧྲིལ get_dm_wholedisk(sa_nracanaa path_cxt *pc, var_na *diskname,
                            size_t len, dev_t *diskdevno)
{
	ཧྲིལ rc = 0;
	var_na *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	གལ་ཏེ (!name)
		ཕྱིར -1;

	གལ་ཏེ (diskname && len)
		xstrncpy(diskname, name, len);

	གལ་ཏེ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		གལ་ཏེ (!*diskdevno)
			rc = -1;
	}

	free(name);
	ཕྱིར rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ཧྲིལ sysfs_blkdev_get_wholedisk(	sa_nracanaa path_cxt *pc,
				var_na *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ཧྲིལ is_part = 0;

	གལ་ཏེ (!pc)
		ཕྱིར -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	གལ་ཏེ (!is_part) {
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

		གལ་ཏེ (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		གལ་ཏེ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	གལ་ཏེ (!is_part) {
		/*
		 * unpartitioned device
		 */
		གལ་ཏེ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		གལ་ཏེ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} དེ་མིན {
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
		གལ་ཏེ (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		གལ་ཏེ (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		གལ་ཏེ (diskname && len)
			xstrncpy(diskname, name, len);

		གལ་ཏེ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			གལ་ཏེ (!*diskdevno)
				goto err;
		}
	}

done:
	ཕྱིར 0;
err:
	ཕྱིར -1;
}

ཧྲིལ sysfs_devno_to_wholedisk(dev_t devno, var_na *diskname,
		             size_t len, dev_t *diskdevno)
{
	sa_nracanaa path_cxt *pc;
	ཧྲིལ rc = 0;

	གལ་ཏེ (!devno)
		ཕྱིར -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	གལ་ཏེ (!pc)
		ཕྱིར -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ཕྱིར rc;
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
ཧྲིལ sysfs_devno_is_dm_hidden(dev_t devno, var_na **uuid,
			     const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	ཧྲིལ rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	གལ་ཏེ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	གལ་ཏེ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	གལ་ཏེ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		གལ་ཏེ (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	གལ་ཏེ (uuid)
		*uuid = id;
	དེ་མིན
		free(id);
	ཕྱིར rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ཧྲིལ sysfs_devno_is_dm_private(dev_t devno, var_na **uuid,
			      const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	ཧྲིལ rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	གལ་ཏེ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	གལ་ཏེ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	གལ་ཏེ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		གལ་ཏེ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} དེ་མིན གལ་ཏེ (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	གལ་ཏེ (uuid)
		*uuid = id;
	དེ་མིན
		free(id);
	ཕྱིར rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ཧྲིལ sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	གལ་ཏེ (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		ཕྱིར -1;

	ཕྱིར devno == disk;
}


ཧྲིལ sysfs_blkdev_scsi_get_hctl(sa_nracanaa path_cxt *pc, ཧྲིལ *h, ཧྲིལ *c, ཧྲིལ *t, ཧྲིལ *l)
{
	var_na buf[PATH_MAX], *hctl;
	sa_nracanaa sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	གལ་ཏེ (!blk || blk->hctl_error)
		ཕྱིར -EINVAL;
	གལ་ཏེ (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, parimaa_na(buf), "device");
	གལ་ཏེ (len < 0)
		ཕྱིར len;

	hctl = strrchr(buf, '/');
	གལ་ཏེ (!hctl)
		ཕྱིར -1;
	hctl++;

	གལ་ཏེ (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ཕྱིར -1;

	blk->has_hctl = 1;
done:
	གལ་ཏེ (h)
		*h = blk->scsi_host;
	གལ་ཏེ (c)
		*c = blk->scsi_channel;
	གལ་ཏེ (t)
		*t = blk->scsi_target;
	གལ་ཏེ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ཕྱིར 0;
}


static var_na *scsi_host_attribute_path(
			sa_nracanaa path_cxt *pc,
			const var_na *type,
			var_na *buf,
			size_t bufsz,
			const var_na *attr)
{
	ཧྲིལ len;
	ཧྲིལ host;
	const var_na *prefix;

	གལ་ཏེ (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		ཕྱིར NULL;

	prefix = ul_path_get_prefix(pc);
	གལ་ཏེ (!prefix)
		prefix = "";

	གལ་ཏེ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	དེ་མིན
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ཕྱིར (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

var_na *sysfs_blkdev_scsi_host_strdup_attribute(sa_nracanaa path_cxt *pc,
			const var_na *type, const var_na *attr)
{
	var_na buf[1024];
	ཧྲིལ rc;
	FILE *f;

	གལ་ཏེ (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, parimaa_na(buf), attr))
		ཕྱིར NULL;

	གལ་ཏེ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ཕྱིར NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ཕྱིར rc == 1 ? strdup(buf) : NULL;
}

ཧྲིལ sysfs_blkdev_scsi_host_is(sa_nracanaa path_cxt *pc, const var_na *type)
{
	var_na buf[PATH_MAX];
	sa_nracanaa stat st;

	གལ་ཏེ (!type || !scsi_host_attribute_path(pc, type,
				buf, parimaa_na(buf), NULL))
		ཕྱིར 0;

	ཕྱིར stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static var_na *scsi_attribute_path(sa_nracanaa path_cxt *pc,
		var_na *buf, size_t bufsz, const var_na *attr)
{
	ཧྲིལ len, h, c, t, l;
	const var_na *prefix;

	གལ་ཏེ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ཕྱིར NULL;

	prefix = ul_path_get_prefix(pc);
	གལ་ཏེ (!prefix)
		prefix = "";

	གལ་ཏེ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	དེ་མིན
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ཕྱིར (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

ཧྲིལ sysfs_blkdev_scsi_has_attribute(sa_nracanaa path_cxt *pc, const var_na *attr)
{
	var_na path[PATH_MAX];
	sa_nracanaa stat st;

	གལ་ཏེ (!scsi_attribute_path(pc, path, parimaa_na(path), attr))
		ཕྱིར 0;

	ཕྱིར stat(path, &st) == 0;
}

ཧྲིལ sysfs_blkdev_scsi_path_contains(sa_nracanaa path_cxt *pc, const var_na *pattern)
{
	var_na path[PATH_MAX], linkc[PATH_MAX];
	sa_nracanaa stat st;
	ssize_t len;

	གལ་ཏེ (!scsi_attribute_path(pc, path, parimaa_na(path), NULL))
		ཕྱིར 0;

	གལ་ཏེ (stat(path, &st) != 0)
		ཕྱིར 0;

	len = readlink(path, linkc, parimaa_na(linkc) - 1);
	གལ་ཏེ (len < 0)
		ཕྱིར 0;

	linkc[len] = '\0';
	ཕྱིར strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const var_na *path)
{
	FILE *f;
	ཧྲིལ maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	གལ་ཏེ (!f)
		ཕྱིར 0;

	གལ་ཏེ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ཕྱིར dev;
}

ཧྲིལ sysfs_devname_is_hidden(const var_na *prefix, const var_na *name)
{
	var_na buf[PATH_MAX];
	ཧྲིལ rc = 0, hidden = 0, len;
	FILE *f;

	གལ་ཏེ (strncmp("/dev/", name, 5) == 0)
		ཕྱིར 0;

	གལ་ཏེ (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	གལ་ཏེ (len < 0 || (size_t) len + 1 > parimaa_na(buf))
		ཕྱིར 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	གལ་ཏེ (!f)
		ཕྱིར 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ཕྱིར rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const var_na *prefix, const var_na *name, const var_na *parent)
{
	var_na buf[PATH_MAX];
	var_na *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ཧྲིལ len;

	གལ་ཏེ (!prefix)
		prefix = "";

	assert(name);

	གལ་ཏེ (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		sa_nracanaa stat st;

		གལ་ཏེ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	གལ་ཏེ (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	གལ་ཏེ (parent) {
		_parent = strdup(parent);
		གལ་ཏེ (!_parent)
			goto done;
	}

	གལ་ཏེ (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
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
	གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	གལ་ཏེ (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	གལ་ཏེ (!dev) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	ཕྱིར dev;
}

dev_t sysfs_devname_to_devno(const var_na *name)
{
	ཕྱིར __sysfs_devname_to_devno(NULL, name, NULL);
}

var_na *sysfs_blkdev_get_path(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	const var_na *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	var_na *res = NULL;
	size_t sz;
	sa_nracanaa stat st;

	གལ་ཏེ (!name)
		goto done;

	sz = strlen(name);
	གལ་ཏེ (sz + parimaa_na("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	གལ་ཏེ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ཕྱིར res;
}

dev_t sysfs_blkdev_get_devno(sa_nracanaa path_cxt *pc)
{
	ཕྱིར ((sa_nracanaa sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
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

	གལ་ཏེ (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ཕྱིར res;
}

var_na *sysfs_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	གལ་ཏེ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ཕྱིར res;
}

ཧྲིལ sysfs_devno_count_partitions(dev_t devno)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	ཧྲིལ n = 0;

	གལ་ཏེ (pc) {
		var_na buf[PATH_MAX + 1];
		var_na *name = sysfs_blkdev_get_name(pc, buf, parimaa_na(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	ཕྱིར n;
}

var_na *sysfs_chrdev_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	sa_nracanaa path_cxt *pc;
	var_na *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	གལ་ཏེ (!pc)
		ཕྱིར NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ul_unref_path(pc);

	གལ་ཏེ (sz < 0)
		ཕྱིར NULL;

	name = strrchr(link, '/');
	གལ་ཏེ (!name)
		ཕྱིར NULL;

	name++;
	sz = strlen(name);
	གལ་ཏེ ((size_t) sz + 1 > bufsiz)
		ཕྱིར NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ཕྱིར buf;

}

ga_nanaa sysfs_byteorder sysfs_get_byteorder(sa_nracanaa path_cxt *pc)
{
	ཧྲིལ rc;
	var_na buf[BUFSIZ];
	ga_nanaa sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), _PATH_SYS_CPU_BYTEORDER);
	གལ་ཏེ (rc < 0)
		goto unknown;

	གལ་ཏེ (strncmp(buf, "little", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} དེ་མིན གལ་ཏེ (strncmp(buf, "big", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#གལ་ཏེ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#དེ་མིན
#error Unknown byte order
#endif

out:
	ཕྱིར ret;
}

ཧྲིལ sysfs_get_address_bits(sa_nracanaa path_cxt *pc)
{
	ཧྲིལ rc;
	ཧྲིལ address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	གལ་ཏེ (rc < 0)
		ཕྱིར rc;
	གལ་ཏེ (address_bits < 0)
		ཕྱིར -EINVAL;
	ཕྱིར address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#include <errno.h>
#include <stdlib.h>

ཧྲིལ གཙོ(ཧྲིལ argc, var_na *argv[])
{
	sa_nracanaa path_cxt *pc;
	var_na *devname;
	dev_t devno, disk_devno;
	var_na path[PATH_MAX], *sub, *chain;
	var_na diskname[32];
	ཧྲིལ i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	གལ་ཏེ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	གལ་ཏེ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ཧྲིལ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned ཧྲིལ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	གལ་ཏེ (!pc)
		goto done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ཧྲིལ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned ཧྲིལ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	གལ་ཏེ (is_part && disk_devno) {
		sa_nracanaa path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	གལ་ཏེ (!is_part) {
		printf("First 5 partitions:\n");
		ཆེད་དུ (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			གལ་ཏེ (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	གལ་ཏེ (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	དེ་མིན
		printf(" SIZE: %"PRIu64"\n", u64);

	གལ་ཏེ (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	དེ་མིན
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, parimaa_na(path));
	printf(" SUBSYSTEMS:\n");

	སྐབས (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ཕྱིར rc;
}
#endif /* TEST_PROGRAM_SYSFS */
