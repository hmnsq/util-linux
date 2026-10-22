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
static පූර්ණ_සංඛ්‍යා  sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, පූර්ණ_සංඛ්‍යා *dirfd);

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
	නම් (ulsysfs_debug_mask)
		ආපසු;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_sysfs_path(dev_t devno, sa_nracanaa path_cxt *parent, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	නම් (!pc)
		ආපසු NULL;
	නම් (prefix)
		ul_path_set_prefix(pc, prefix);

	නම් (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ආපසු NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ආපසු pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
පූර්ණ_සංඛ්‍යා sysfs_blkdev_init_path(sa_nracanaa path_cxt *pc, dev_t devno, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk;
	පූර්ණ_සංඛ්‍යා rc;
	var_na buf[parimaa_na(_PATH_SYS_DEVBLOCK)
		 + parimaa_na(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, parimaa_na(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	නම් (rc)
		ආපසු rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	නම් (rc < 0)
		ආපසු rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	නම් (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, parimaa_na(sa_nracanaa sysfs_blkdev));
		නම් (!blk)
			ආපසු -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ආපසු 0;
}

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk;

	නම් (!pc)
		ආපසු;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	නම් (!blk)
		ආපසු;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_set_parent(sa_nracanaa path_cxt *pc, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	නම් (!pc || !blk)
		ආපසු -EINVAL;

	නම් (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	නම් (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} නැතහොත්
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ආපසු 0;
}

sa_nracanaa path_cxt *sysfs_blkdev_get_parent(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ආපසු blk ? blk->parent : NULL;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
static පූර්ණ_සංඛ්‍යා sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, පූර්ණ_සංඛ්‍යා *dirfd)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	නම් (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		නම් (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ආපසු 0;
		}
	}
	ආපසු 1;	/* no redirect */
}

var_na *sysfs_blkdev_get_name(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	var_na *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	නම් (sz < 0)
		ආපසු NULL;

	name = strrchr(link, '/');
	නම් (!name)
		ආපසු NULL;

	name++;
	sz = strlen(name);
	නම් ((size_t) sz + 1 > bufsiz)
		ආපසු NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ආපසු buf;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_is_partition_dirent(DIR *dir, sa_nracanaa dirent *d, const var_na *parent_name)
{
	var_na path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	නම් (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ආපසු 0;
#endif
	size_t len = 0;

	නම් (parent_name) {
		const var_na *p = parent_name;

		/* /dev/sda --> "sda" */
		නම් (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			නම් (!p)
				ආපසු 0;
			p++;
		}

		len = strlen(p);
		නම් ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	නම් (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ආපසු ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, parimaa_na(path), "%s/start", d->d_name);

	ආපසු faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_count_partitions(sa_nracanaa path_cxt *pc, const var_na *devname)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	පූර්ණ_සංඛ්‍යා r = 0;

	dir = ul_path_opendir(pc, NULL);
	නම් (!dir)
		ආපසු 0;

	අතරතුර ((d = xreaddir(dir))) {
		නම් (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ආපසු r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(sa_nracanaa path_cxt *pc, පූර්ණ_සංඛ්‍යා partno)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, NULL);
	නම් (!dir)
		ආපසු 0;

	අතරතුර ((d = xreaddir(dir))) {
		පූර්ණ_සංඛ්‍යා n;

		නම් (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		නම් (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		නම් (n == partno) {
			නම් (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				bha_nga;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (පූර්ණ_සංඛ්‍යා) partno, (පූර්ණ_සංඛ්‍යා) devno));
	ආපසු devno;
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
	නම් (!dir)
		ආපසු NULL;

	අතරතුර ((d = xreaddir(dir))) {
		නම් (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ආපසු name;
err:
	free(name);
	closedir(dir);
	ආපසු NULL;
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

	නම් (!chain || !*chain)
		ආපසු NULL;

	len = strlen(chain);
	නම් (len + parimaa_na(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ආපසු NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, parimaa_na(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		නම් (p) {
			*p = '\0';
			len = p - chain;
		}

		නම් (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ආපසු basename(buf);
		}

	} අතරතුර (p);

	ආපසු NULL;
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
	නම් (ssz <= 0)
		ආපසු NULL;

	නම් ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	නම් (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ආපසු res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
පූර්ණ_සංඛ්‍යා sysfs_blkdev_next_subsystem(sa_nracanaa path_cxt *pc __attribute__((unused)),
			 var_na *devchain, var_na **subsys)
{
	var_na subbuf[PATH_MAX];
	var_na *sub;

	නම් (!subsys || !devchain)
		ආපසු -EINVAL;

	*subsys = NULL;

	අතරතුර ((sub = get_subsystem(devchain, subbuf, parimaa_na(subbuf)))) {
		*subsys = strdup(sub);
		නම් (!*subsys)
			ආපසු -ENOMEM;
		ආපසු 0;
	}

	ආපසු 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
static පූර්ණ_සංඛ්‍යා sysfs_devchain_is_removable(var_na *chain)
{
	size_t len;
	var_na buf[20];
	var_na *p;

	නම් (!chain || !*chain)
		ආපසු 0;

	len = strlen(chain);
	නම් (len + parimaa_na(REMOVABLE_FILENAME) > PATH_MAX)
		ආපසු 0;

	do {
		පූර්ණ_සංඛ්‍යා fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, parimaa_na(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		නම් (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			bha_nga;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		නම් (fd != -1) {
			rc = ul_read_all(fd, buf, parimaa_na(buf));
			close(fd);

			නම් (rc > 0) {
				නම් (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ආපසු 0;
				} නැතහොත් නම් (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ආපසු 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		නම් (p) {
			*p = '\0';
			len = p - chain;
		}

	} අතරතුර (p);

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_is_hotpluggable(sa_nracanaa path_cxt *pc)
{
	var_na buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, parimaa_na(buf));
	ආපසු sysfs_devchain_is_removable(chain);
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_is_removable(sa_nracanaa path_cxt *pc)
{
	පූර්ණ_සංඛ්‍යා rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	නම් (ul_path_read_s32(pc, &rc, "removable") == 0)
		ආපසු rc;

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා get_dm_wholedisk(sa_nracanaa path_cxt *pc, var_na *diskname,
                            size_t len, dev_t *diskdevno)
{
	පූර්ණ_සංඛ්‍යා rc = 0;
	var_na *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	නම් (!name)
		ආපසු -1;

	නම් (diskname && len)
		xstrncpy(diskname, name, len);

	නම් (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		නම් (!*diskdevno)
			rc = -1;
	}

	free(name);
	ආපසු rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
පූර්ණ_සංඛ්‍යා sysfs_blkdev_get_wholedisk(	sa_nracanaa path_cxt *pc,
				var_na *diskname,
				size_t len,
				dev_t *diskdevno)
{
	පූර්ණ_සංඛ්‍යා is_part = 0;

	නම් (!pc)
		ආපසු -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	නම් (!is_part) {
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

		නම් (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		නම් (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	නම් (!is_part) {
		/*
		 * unpartitioned device
		 */
		නම් (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		නම් (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} නැතහොත් {
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
		නම් (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		නම් (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		නම් (diskname && len)
			xstrncpy(diskname, name, len);

		නම් (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			නම් (!*diskdevno)
				goto err;
		}
	}

done:
	ආපසු 0;
err:
	ආපසු -1;
}

පූර්ණ_සංඛ්‍යා sysfs_devno_to_wholedisk(dev_t devno, var_na *diskname,
		             size_t len, dev_t *diskdevno)
{
	sa_nracanaa path_cxt *pc;
	පූර්ණ_සංඛ්‍යා rc = 0;

	නම් (!devno)
		ආපසු -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	නම් (!pc)
		ආපසු -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ආපසු rc;
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
පූර්ණ_සංඛ්‍යා sysfs_devno_is_dm_hidden(dev_t devno, var_na **uuid,
			     const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	පූර්ණ_සංඛ්‍යා rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	නම් (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	නම් (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	නම් (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		නම් (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	නම් (uuid)
		*uuid = id;
	නැතහොත්
		free(id);
	ආපසු rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
පූර්ණ_සංඛ්‍යා sysfs_devno_is_dm_private(dev_t devno, var_na **uuid,
			      const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	පූර්ණ_සංඛ්‍යා rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	නම් (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	නම් (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	නම් (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		නම් (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} නැතහොත් නම් (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	නම් (uuid)
		*uuid = id;
	නැතහොත්
		free(id);
	ආපසු rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
පූර්ණ_සංඛ්‍යා sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	නම් (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		ආපසු -1;

	ආපසු devno == disk;
}


පූර්ණ_සංඛ්‍යා sysfs_blkdev_scsi_get_hctl(sa_nracanaa path_cxt *pc, පූර්ණ_සංඛ්‍යා *h, පූර්ණ_සංඛ්‍යා *c, පූර්ණ_සංඛ්‍යා *t, පූර්ණ_සංඛ්‍යා *l)
{
	var_na buf[PATH_MAX], *hctl;
	sa_nracanaa sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	නම් (!blk || blk->hctl_error)
		ආපසු -EINVAL;
	නම් (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, parimaa_na(buf), "device");
	නම් (len < 0)
		ආපසු len;

	hctl = strrchr(buf, '/');
	නම් (!hctl)
		ආපසු -1;
	hctl++;

	නම් (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ආපසු -1;

	blk->has_hctl = 1;
done:
	නම් (h)
		*h = blk->scsi_host;
	නම් (c)
		*c = blk->scsi_channel;
	නම් (t)
		*t = blk->scsi_target;
	නම් (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ආපසු 0;
}


static var_na *scsi_host_attribute_path(
			sa_nracanaa path_cxt *pc,
			const var_na *type,
			var_na *buf,
			size_t bufsz,
			const var_na *attr)
{
	පූර්ණ_සංඛ්‍යා len;
	පූර්ණ_සංඛ්‍යා host;
	const var_na *prefix;

	නම් (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		ආපසු NULL;

	prefix = ul_path_get_prefix(pc);
	නම් (!prefix)
		prefix = "";

	නම් (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	නැතහොත්
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ආපසු (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

var_na *sysfs_blkdev_scsi_host_strdup_attribute(sa_nracanaa path_cxt *pc,
			const var_na *type, const var_na *attr)
{
	var_na buf[1024];
	පූර්ණ_සංඛ්‍යා rc;
	FILE *f;

	නම් (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, parimaa_na(buf), attr))
		ආපසු NULL;

	නම් (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ආපසු NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ආපසු rc == 1 ? strdup(buf) : NULL;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_scsi_host_is(sa_nracanaa path_cxt *pc, const var_na *type)
{
	var_na buf[PATH_MAX];
	sa_nracanaa stat st;

	නම් (!type || !scsi_host_attribute_path(pc, type,
				buf, parimaa_na(buf), NULL))
		ආපසු 0;

	ආපසු stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static var_na *scsi_attribute_path(sa_nracanaa path_cxt *pc,
		var_na *buf, size_t bufsz, const var_na *attr)
{
	පූර්ණ_සංඛ්‍යා len, h, c, t, l;
	const var_na *prefix;

	නම් (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ආපසු NULL;

	prefix = ul_path_get_prefix(pc);
	නම් (!prefix)
		prefix = "";

	නම් (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	නැතහොත්
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ආපසු (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_scsi_has_attribute(sa_nracanaa path_cxt *pc, const var_na *attr)
{
	var_na path[PATH_MAX];
	sa_nracanaa stat st;

	නම් (!scsi_attribute_path(pc, path, parimaa_na(path), attr))
		ආපසු 0;

	ආපසු stat(path, &st) == 0;
}

පූර්ණ_සංඛ්‍යා sysfs_blkdev_scsi_path_contains(sa_nracanaa path_cxt *pc, const var_na *pattern)
{
	var_na path[PATH_MAX], linkc[PATH_MAX];
	sa_nracanaa stat st;
	ssize_t len;

	නම් (!scsi_attribute_path(pc, path, parimaa_na(path), NULL))
		ආපසු 0;

	නම් (stat(path, &st) != 0)
		ආපසු 0;

	len = readlink(path, linkc, parimaa_na(linkc) - 1);
	නම් (len < 0)
		ආපසු 0;

	linkc[len] = '\0';
	ආපසු strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const var_na *path)
{
	FILE *f;
	පූර්ණ_සංඛ්‍යා maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	නම් (!f)
		ආපසු 0;

	නම් (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ආපසු dev;
}

පූර්ණ_සංඛ්‍යා sysfs_devname_is_hidden(const var_na *prefix, const var_na *name)
{
	var_na buf[PATH_MAX];
	පූර්ණ_සංඛ්‍යා rc = 0, hidden = 0, len;
	FILE *f;

	නම් (strncmp("/dev/", name, 5) == 0)
		ආපසු 0;

	නම් (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	නම් (len < 0 || (size_t) len + 1 > parimaa_na(buf))
		ආපසු 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	නම් (!f)
		ආපසු 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ආපසු rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const var_na *prefix, const var_na *name, const var_na *parent)
{
	var_na buf[PATH_MAX];
	var_na *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	පූර්ණ_සංඛ්‍යා len;

	නම් (!prefix)
		prefix = "";

	assert(name);

	නම් (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		sa_nracanaa stat st;

		නම් (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	නම් (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	නම් (parent) {
		_parent = strdup(parent);
		නම් (!_parent)
			goto done;
	}

	නම් (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		නම් (len < 0 || (size_t) len >= parimaa_na(buf))
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
	නම් (len < 0 || (size_t) len >= parimaa_na(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	නම් (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		නම් (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	නම් (!dev) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		නම් (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	ආපසු dev;
}

dev_t sysfs_devname_to_devno(const var_na *name)
{
	ආපසු __sysfs_devname_to_devno(NULL, name, NULL);
}

var_na *sysfs_blkdev_get_path(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	const var_na *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	var_na *res = NULL;
	size_t sz;
	sa_nracanaa stat st;

	නම් (!name)
		goto done;

	sz = strlen(name);
	නම් (sz + parimaa_na("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	නම් (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ආපසු res;
}

dev_t sysfs_blkdev_get_devno(sa_nracanaa path_cxt *pc)
{
	ආපසු ((sa_nracanaa sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
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

	නම් (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ආපසු res;
}

var_na *sysfs_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	නම් (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ආපසු res;
}

පූර්ණ_සංඛ්‍යා sysfs_devno_count_partitions(dev_t devno)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	පූර්ණ_සංඛ්‍යා n = 0;

	නම් (pc) {
		var_na buf[PATH_MAX + 1];
		var_na *name = sysfs_blkdev_get_name(pc, buf, parimaa_na(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	ආපසු n;
}

var_na *sysfs_chrdev_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	sa_nracanaa path_cxt *pc;
	var_na *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	නම් (!pc)
		ආපසු NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ul_unref_path(pc);

	නම් (sz < 0)
		ආපසු NULL;

	name = strrchr(link, '/');
	නම් (!name)
		ආපසු NULL;

	name++;
	sz = strlen(name);
	නම් ((size_t) sz + 1 > bufsiz)
		ආපසු NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ආපසු buf;

}

ga_nanaa sysfs_byteorder sysfs_get_byteorder(sa_nracanaa path_cxt *pc)
{
	පූර්ණ_සංඛ්‍යා rc;
	var_na buf[BUFSIZ];
	ga_nanaa sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), _PATH_SYS_CPU_BYTEORDER);
	නම් (rc < 0)
		goto unknown;

	නම් (strncmp(buf, "little", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} නැතහොත් නම් (strncmp(buf, "big", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#නම් __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#නැතහොත්
#error Unknown byte order
#endif

out:
	ආපසු ret;
}

පූර්ණ_සංඛ්‍යා sysfs_get_address_bits(sa_nracanaa path_cxt *pc)
{
	පූර්ණ_සංඛ්‍යා rc;
	පූර්ණ_සංඛ්‍යා address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	නම් (rc < 0)
		ආපසු rc;
	නම් (address_bits < 0)
		ආපසු -EINVAL;
	ආපසු address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#include <errno.h>
#include <stdlib.h>

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	sa_nracanaa path_cxt *pc;
	var_na *devname;
	dev_t devno, disk_devno;
	var_na path[PATH_MAX], *sub, *chain;
	var_na diskname[32];
	පූර්ණ_සංඛ්‍යා i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	නම් (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	නම් (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned පූර්ණ_සංඛ්‍යා) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned පූර්ණ_සංඛ්‍යා) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	නම් (!pc)
		goto done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned පූර්ණ_සංඛ්‍යා) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned පූර්ණ_සංඛ්‍යා) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	නම් (is_part && disk_devno) {
		sa_nracanaa path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	නම් (!is_part) {
		printf("First 5 partitions:\n");
		සඳහා (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			නම් (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	නම් (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	නැතහොත්
		printf(" SIZE: %"PRIu64"\n", u64);

	නම් (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	නැතහොත්
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, parimaa_na(path));
	printf(" SUBSYSTEMS:\n");

	අතරතුර (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ආපසු rc;
}
#endif /* TEST_PROGRAM_SYSFS */
