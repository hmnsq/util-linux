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
static ကိန်းပြည့်  sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, ကိန်းပြည့် *dirfd);

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
	အကယ်၍ (ulsysfs_debug_mask)
		ပြန်ပေး;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_sysfs_path(dev_t devno, sa_nracanaa path_cxt *parent, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	အကယ်၍ (!pc)
		ပြန်ပေး NULL;
	အကယ်၍ (prefix)
		ul_path_set_prefix(pc, prefix);

	အကယ်၍ (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ပြန်ပေး NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ပြန်ပေး pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ကိန်းပြည့် sysfs_blkdev_init_path(sa_nracanaa path_cxt *pc, dev_t devno, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk;
	ကိန်းပြည့် rc;
	var_na buf[parimaa_na(_PATH_SYS_DEVBLOCK)
		 + parimaa_na(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, parimaa_na(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	အကယ်၍ (rc)
		ပြန်ပေး rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	အကယ်၍ (rc < 0)
		ပြန်ပေး rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	အကယ်၍ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, parimaa_na(sa_nracanaa sysfs_blkdev));
		အကယ်၍ (!blk)
			ပြန်ပေး -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ပြန်ပေး 0;
}

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk;

	အကယ်၍ (!pc)
		ပြန်ပေး;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	အကယ်၍ (!blk)
		ပြန်ပေး;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

ကိန်းပြည့် sysfs_blkdev_set_parent(sa_nracanaa path_cxt *pc, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	အကယ်၍ (!pc || !blk)
		ပြန်ပေး -EINVAL;

	အကယ်၍ (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	အကယ်၍ (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} မဟုတ်ပါက
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ပြန်ပေး 0;
}

sa_nracanaa path_cxt *sysfs_blkdev_get_parent(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ပြန်ပေး blk ? blk->parent : NULL;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
static ကိန်းပြည့် sysfs_blkdev_enoent_redirect(sa_nracanaa path_cxt *pc, const var_na *path, ကိန်းပြည့် *dirfd)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	အကယ်၍ (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		အကယ်၍ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ပြန်ပေး 0;
		}
	}
	ပြန်ပေး 1;	/* no redirect */
}

var_na *sysfs_blkdev_get_name(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	var_na *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	အကယ်၍ (sz < 0)
		ပြန်ပေး NULL;

	name = strrchr(link, '/');
	အကယ်၍ (!name)
		ပြန်ပေး NULL;

	name++;
	sz = strlen(name);
	အကယ်၍ ((size_t) sz + 1 > bufsiz)
		ပြန်ပေး NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ပြန်ပေး buf;
}

ကိန်းပြည့် sysfs_blkdev_is_partition_dirent(DIR *dir, sa_nracanaa dirent *d, const var_na *parent_name)
{
	var_na path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	အကယ်၍ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ပြန်ပေး 0;
#endif
	size_t len = 0;

	အကယ်၍ (parent_name) {
		const var_na *p = parent_name;

		/* /dev/sda --> "sda" */
		အကယ်၍ (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			အကယ်၍ (!p)
				ပြန်ပေး 0;
			p++;
		}

		len = strlen(p);
		အကယ်၍ ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	အကယ်၍ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ပြန်ပေး ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, parimaa_na(path), "%s/start", d->d_name);

	ပြန်ပေး faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

ကိန်းပြည့် sysfs_blkdev_count_partitions(sa_nracanaa path_cxt *pc, const var_na *devname)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	ကိန်းပြည့် r = 0;

	dir = ul_path_opendir(pc, NULL);
	အကယ်၍ (!dir)
		ပြန်ပေး 0;

	နေစဉ် ((d = xreaddir(dir))) {
		အကယ်၍ (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ပြန်ပေး r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(sa_nracanaa path_cxt *pc, ကိန်းပြည့် partno)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, NULL);
	အကယ်၍ (!dir)
		ပြန်ပေး 0;

	နေစဉ် ((d = xreaddir(dir))) {
		ကိန်းပြည့် n;

		အကယ်၍ (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		အကယ်၍ (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		အကယ်၍ (n == partno) {
			အကယ်၍ (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				bha_nga;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ကိန်းပြည့်) partno, (ကိန်းပြည့်) devno));
	ပြန်ပေး devno;
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
	အကယ်၍ (!dir)
		ပြန်ပေး NULL;

	နေစဉ် ((d = xreaddir(dir))) {
		အကယ်၍ (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ပြန်ပေး name;
err:
	free(name);
	closedir(dir);
	ပြန်ပေး NULL;
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

	အကယ်၍ (!chain || !*chain)
		ပြန်ပေး NULL;

	len = strlen(chain);
	အကယ်၍ (len + parimaa_na(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ပြန်ပေး NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, parimaa_na(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		အကယ်၍ (p) {
			*p = '\0';
			len = p - chain;
		}

		အကယ်၍ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ပြန်ပေး basename(buf);
		}

	} နေစဉ် (p);

	ပြန်ပေး NULL;
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
	အကယ်၍ (ssz <= 0)
		ပြန်ပေး NULL;

	အကယ်၍ ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	အကယ်၍ (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ပြန်ပေး res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ကိန်းပြည့် sysfs_blkdev_next_subsystem(sa_nracanaa path_cxt *pc __attribute__((unused)),
			 var_na *devchain, var_na **subsys)
{
	var_na subbuf[PATH_MAX];
	var_na *sub;

	အကယ်၍ (!subsys || !devchain)
		ပြန်ပေး -EINVAL;

	*subsys = NULL;

	နေစဉ် ((sub = get_subsystem(devchain, subbuf, parimaa_na(subbuf)))) {
		*subsys = strdup(sub);
		အကယ်၍ (!*subsys)
			ပြန်ပေး -ENOMEM;
		ပြန်ပေး 0;
	}

	ပြန်ပေး 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
static ကိန်းပြည့် sysfs_devchain_is_removable(var_na *chain)
{
	size_t len;
	var_na buf[20];
	var_na *p;

	အကယ်၍ (!chain || !*chain)
		ပြန်ပေး 0;

	len = strlen(chain);
	အကယ်၍ (len + parimaa_na(REMOVABLE_FILENAME) > PATH_MAX)
		ပြန်ပေး 0;

	do {
		ကိန်းပြည့် fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, parimaa_na(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		အကယ်၍ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			bha_nga;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		အကယ်၍ (fd != -1) {
			rc = ul_read_all(fd, buf, parimaa_na(buf));
			close(fd);

			အကယ်၍ (rc > 0) {
				အကယ်၍ (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ပြန်ပေး 0;
				} မဟုတ်ပါက အကယ်၍ (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ပြန်ပေး 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		အကယ်၍ (p) {
			*p = '\0';
			len = p - chain;
		}

	} နေစဉ် (p);

	ပြန်ပေး 0;
}

ကိန်းပြည့် sysfs_blkdev_is_hotpluggable(sa_nracanaa path_cxt *pc)
{
	var_na buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, parimaa_na(buf));
	ပြန်ပေး sysfs_devchain_is_removable(chain);
}

ကိန်းပြည့် sysfs_blkdev_is_removable(sa_nracanaa path_cxt *pc)
{
	ကိန်းပြည့် rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	အကယ်၍ (ul_path_read_s32(pc, &rc, "removable") == 0)
		ပြန်ပေး rc;

	ပြန်ပေး 0;
}

static ကိန်းပြည့် get_dm_wholedisk(sa_nracanaa path_cxt *pc, var_na *diskname,
                            size_t len, dev_t *diskdevno)
{
	ကိန်းပြည့် rc = 0;
	var_na *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	အကယ်၍ (!name)
		ပြန်ပေး -1;

	အကယ်၍ (diskname && len)
		xstrncpy(diskname, name, len);

	အကယ်၍ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		အကယ်၍ (!*diskdevno)
			rc = -1;
	}

	free(name);
	ပြန်ပေး rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ကိန်းပြည့် sysfs_blkdev_get_wholedisk(	sa_nracanaa path_cxt *pc,
				var_na *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ကိန်းပြည့် is_part = 0;

	အကယ်၍ (!pc)
		ပြန်ပေး -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	အကယ်၍ (!is_part) {
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

		အကယ်၍ (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		အကယ်၍ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	အကယ်၍ (!is_part) {
		/*
		 * unpartitioned device
		 */
		အကယ်၍ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		အကယ်၍ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} မဟုတ်ပါက {
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
		အကယ်၍ (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		အကယ်၍ (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		အကယ်၍ (diskname && len)
			xstrncpy(diskname, name, len);

		အကယ်၍ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			အကယ်၍ (!*diskdevno)
				goto err;
		}
	}

done:
	ပြန်ပေး 0;
err:
	ပြန်ပေး -1;
}

ကိန်းပြည့် sysfs_devno_to_wholedisk(dev_t devno, var_na *diskname,
		             size_t len, dev_t *diskdevno)
{
	sa_nracanaa path_cxt *pc;
	ကိန်းပြည့် rc = 0;

	အကယ်၍ (!devno)
		ပြန်ပေး -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	အကယ်၍ (!pc)
		ပြန်ပေး -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ပြန်ပေး rc;
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
ကိန်းပြည့် sysfs_devno_is_dm_hidden(dev_t devno, var_na **uuid,
			     const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	ကိန်းပြည့် rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	အကယ်၍ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	အကယ်၍ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	အကယ်၍ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		အကယ်၍ (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	အကယ်၍ (uuid)
		*uuid = id;
	မဟုတ်ပါက
		free(id);
	ပြန်ပေး rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ကိန်းပြည့် sysfs_devno_is_dm_private(dev_t devno, var_na **uuid,
			      const sa_nracanaa ul_vfs_ops *vfs)
{
	sa_nracanaa path_cxt *pc = NULL;
	var_na *id = NULL;
	ကိန်းပြည့် rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	အကယ်၍ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	အကယ်၍ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	အကယ်၍ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		အကယ်၍ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} မဟုတ်ပါက အကယ်၍ (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	အကယ်၍ (uuid)
		*uuid = id;
	မဟုတ်ပါက
		free(id);
	ပြန်ပေး rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ကိန်းပြည့် sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	အကယ်၍ (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		ပြန်ပေး -1;

	ပြန်ပေး devno == disk;
}


ကိန်းပြည့် sysfs_blkdev_scsi_get_hctl(sa_nracanaa path_cxt *pc, ကိန်းပြည့် *h, ကိန်းပြည့် *c, ကိန်းပြည့် *t, ကိန်းပြည့် *l)
{
	var_na buf[PATH_MAX], *hctl;
	sa_nracanaa sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	အကယ်၍ (!blk || blk->hctl_error)
		ပြန်ပေး -EINVAL;
	အကယ်၍ (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, parimaa_na(buf), "device");
	အကယ်၍ (len < 0)
		ပြန်ပေး len;

	hctl = strrchr(buf, '/');
	အကယ်၍ (!hctl)
		ပြန်ပေး -1;
	hctl++;

	အကယ်၍ (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ပြန်ပေး -1;

	blk->has_hctl = 1;
done:
	အကယ်၍ (h)
		*h = blk->scsi_host;
	အကယ်၍ (c)
		*c = blk->scsi_channel;
	အကယ်၍ (t)
		*t = blk->scsi_target;
	အကယ်၍ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ပြန်ပေး 0;
}


static var_na *scsi_host_attribute_path(
			sa_nracanaa path_cxt *pc,
			const var_na *type,
			var_na *buf,
			size_t bufsz,
			const var_na *attr)
{
	ကိန်းပြည့် len;
	ကိန်းပြည့် host;
	const var_na *prefix;

	အကယ်၍ (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		ပြန်ပေး NULL;

	prefix = ul_path_get_prefix(pc);
	အကယ်၍ (!prefix)
		prefix = "";

	အကယ်၍ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	မဟုတ်ပါက
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ပြန်ပေး (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

var_na *sysfs_blkdev_scsi_host_strdup_attribute(sa_nracanaa path_cxt *pc,
			const var_na *type, const var_na *attr)
{
	var_na buf[1024];
	ကိန်းပြည့် rc;
	FILE *f;

	အကယ်၍ (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, parimaa_na(buf), attr))
		ပြန်ပေး NULL;

	အကယ်၍ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ပြန်ပေး NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ပြန်ပေး rc == 1 ? strdup(buf) : NULL;
}

ကိန်းပြည့် sysfs_blkdev_scsi_host_is(sa_nracanaa path_cxt *pc, const var_na *type)
{
	var_na buf[PATH_MAX];
	sa_nracanaa stat st;

	အကယ်၍ (!type || !scsi_host_attribute_path(pc, type,
				buf, parimaa_na(buf), NULL))
		ပြန်ပေး 0;

	ပြန်ပေး stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static var_na *scsi_attribute_path(sa_nracanaa path_cxt *pc,
		var_na *buf, size_t bufsz, const var_na *attr)
{
	ကိန်းပြည့် len, h, c, t, l;
	const var_na *prefix;

	အကယ်၍ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ပြန်ပေး NULL;

	prefix = ul_path_get_prefix(pc);
	အကယ်၍ (!prefix)
		prefix = "";

	အကယ်၍ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	မဟုတ်ပါက
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ပြန်ပေး (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

ကိန်းပြည့် sysfs_blkdev_scsi_has_attribute(sa_nracanaa path_cxt *pc, const var_na *attr)
{
	var_na path[PATH_MAX];
	sa_nracanaa stat st;

	အကယ်၍ (!scsi_attribute_path(pc, path, parimaa_na(path), attr))
		ပြန်ပေး 0;

	ပြန်ပေး stat(path, &st) == 0;
}

ကိန်းပြည့် sysfs_blkdev_scsi_path_contains(sa_nracanaa path_cxt *pc, const var_na *pattern)
{
	var_na path[PATH_MAX], linkc[PATH_MAX];
	sa_nracanaa stat st;
	ssize_t len;

	အကယ်၍ (!scsi_attribute_path(pc, path, parimaa_na(path), NULL))
		ပြန်ပေး 0;

	အကယ်၍ (stat(path, &st) != 0)
		ပြန်ပေး 0;

	len = readlink(path, linkc, parimaa_na(linkc) - 1);
	အကယ်၍ (len < 0)
		ပြန်ပေး 0;

	linkc[len] = '\0';
	ပြန်ပေး strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const var_na *path)
{
	FILE *f;
	ကိန်းပြည့် maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	အကယ်၍ (!f)
		ပြန်ပေး 0;

	အကယ်၍ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ပြန်ပေး dev;
}

ကိန်းပြည့် sysfs_devname_is_hidden(const var_na *prefix, const var_na *name)
{
	var_na buf[PATH_MAX];
	ကိန်းပြည့် rc = 0, hidden = 0, len;
	FILE *f;

	အကယ်၍ (strncmp("/dev/", name, 5) == 0)
		ပြန်ပေး 0;

	အကယ်၍ (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	အကယ်၍ (len < 0 || (size_t) len + 1 > parimaa_na(buf))
		ပြန်ပေး 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	အကယ်၍ (!f)
		ပြန်ပေး 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ပြန်ပေး rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const var_na *prefix, const var_na *name, const var_na *parent)
{
	var_na buf[PATH_MAX];
	var_na *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ကိန်းပြည့် len;

	အကယ်၍ (!prefix)
		prefix = "";

	assert(name);

	အကယ်၍ (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		sa_nracanaa stat st;

		အကယ်၍ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	အကယ်၍ (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	အကယ်၍ (parent) {
		_parent = strdup(parent);
		အကယ်၍ (!_parent)
			goto done;
	}

	အကယ်၍ (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
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
	အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	အကယ်၍ (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	အကယ်၍ (!dev) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	ပြန်ပေး dev;
}

dev_t sysfs_devname_to_devno(const var_na *name)
{
	ပြန်ပေး __sysfs_devname_to_devno(NULL, name, NULL);
}

var_na *sysfs_blkdev_get_path(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	const var_na *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	var_na *res = NULL;
	size_t sz;
	sa_nracanaa stat st;

	အကယ်၍ (!name)
		goto done;

	sz = strlen(name);
	အကယ်၍ (sz + parimaa_na("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	အကယ်၍ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ပြန်ပေး res;
}

dev_t sysfs_blkdev_get_devno(sa_nracanaa path_cxt *pc)
{
	ပြန်ပေး ((sa_nracanaa sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
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

	အကယ်၍ (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ပြန်ပေး res;
}

var_na *sysfs_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	အကယ်၍ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ပြန်ပေး res;
}

ကိန်းပြည့် sysfs_devno_count_partitions(dev_t devno)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	ကိန်းပြည့် n = 0;

	အကယ်၍ (pc) {
		var_na buf[PATH_MAX + 1];
		var_na *name = sysfs_blkdev_get_name(pc, buf, parimaa_na(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	ပြန်ပေး n;
}

var_na *sysfs_chrdev_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	sa_nracanaa path_cxt *pc;
	var_na *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	အကယ်၍ (!pc)
		ပြန်ပေး NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ul_unref_path(pc);

	အကယ်၍ (sz < 0)
		ပြန်ပေး NULL;

	name = strrchr(link, '/');
	အကယ်၍ (!name)
		ပြန်ပေး NULL;

	name++;
	sz = strlen(name);
	အကယ်၍ ((size_t) sz + 1 > bufsiz)
		ပြန်ပေး NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ပြန်ပေး buf;

}

ga_nanaa sysfs_byteorder sysfs_get_byteorder(sa_nracanaa path_cxt *pc)
{
	ကိန်းပြည့် rc;
	var_na buf[BUFSIZ];
	ga_nanaa sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), _PATH_SYS_CPU_BYTEORDER);
	အကယ်၍ (rc < 0)
		goto unknown;

	အကယ်၍ (strncmp(buf, "little", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} မဟုတ်ပါက အကယ်၍ (strncmp(buf, "big", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#အကယ်၍ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#မဟုတ်ပါက
#error Unknown byte order
#endif

out:
	ပြန်ပေး ret;
}

ကိန်းပြည့် sysfs_get_address_bits(sa_nracanaa path_cxt *pc)
{
	ကိန်းပြည့် rc;
	ကိန်းပြည့် address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	အကယ်၍ (rc < 0)
		ပြန်ပေး rc;
	အကယ်၍ (address_bits < 0)
		ပြန်ပေး -EINVAL;
	ပြန်ပေး address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#include <errno.h>
#include <stdlib.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	sa_nracanaa path_cxt *pc;
	var_na *devname;
	dev_t devno, disk_devno;
	var_na path[PATH_MAX], *sub, *chain;
	var_na diskname[32];
	ကိန်းပြည့် i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	အကယ်၍ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	အကယ်၍ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ကိန်းပြည့်) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned ကိန်းပြည့်) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	အကယ်၍ (!pc)
		goto done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ကိန်းပြည့်) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned ကိန်းပြည့်) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	အကယ်၍ (is_part && disk_devno) {
		sa_nracanaa path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	အကယ်၍ (!is_part) {
		printf("First 5 partitions:\n");
		အတွက် (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			အကယ်၍ (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	အကယ်၍ (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	မဟုတ်ပါက
		printf(" SIZE: %"PRIu64"\n", u64);

	အကယ်၍ (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	မဟုတ်ပါက
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, parimaa_na(path));
	printf(" SUBSYSTEMS:\n");

	နေစဉ် (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ပြန်ပေး rc;
}
#endif /* TEST_PROGRAM_SYSFS */
