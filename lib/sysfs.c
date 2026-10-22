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

static void sysfs_blkdev_deinit_path(struct path_cxt *pc);
static số_nguyên  sysfs_blkdev_enoent_redirect(struct path_cxt *pc, const char *path, số_nguyên *dirfd);

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
	nếu (ulsysfs_debug_mask)
		trả_về;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

struct path_cxt *ul_new_sysfs_path(dev_t devno, struct path_cxt *parent, const char *prefix)
{
	struct path_cxt *pc = ul_new_path(NULL);

	nếu (!pc)
		trả_về NULL;
	nếu (prefix)
		ul_path_set_prefix(pc, prefix);

	nếu (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		trả_về NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	trả_về pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
số_nguyên sysfs_blkdev_init_path(struct path_cxt *pc, dev_t devno, struct path_cxt *parent)
{
	struct sysfs_blkdev *blk;
	số_nguyên rc;
	char buf[sizeof(_PATH_SYS_DEVBLOCK)
		 + sizeof(stringify_value(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, sizeof(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	nếu (rc)
		trả_về rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	nếu (rc < 0)
		trả_về rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	nếu (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, sizeof(struct sysfs_blkdev));
		nếu (!blk)
			trả_về -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	trả_về 0;
}

static void sysfs_blkdev_deinit_path(struct path_cxt *pc)
{
	struct sysfs_blkdev *blk;

	nếu (!pc)
		trả_về;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	nếu (!blk)
		trả_về;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

số_nguyên sysfs_blkdev_set_parent(struct path_cxt *pc, struct path_cxt *parent)
{
	struct sysfs_blkdev *blk = ul_path_get_dialect(pc);

	nếu (!pc || !blk)
		trả_về -EINVAL;

	nếu (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	nếu (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} không_thì
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	trả_về 0;
}

struct path_cxt *sysfs_blkdev_get_parent(struct path_cxt *pc)
{
	struct sysfs_blkdev *blk = ul_path_get_dialect(pc);
	trả_về blk ? blk->parent : NULL;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
static số_nguyên sysfs_blkdev_enoent_redirect(struct path_cxt *pc, const char *path, số_nguyên *dirfd)
{
	struct sysfs_blkdev *blk = ul_path_get_dialect(pc);

	nếu (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		nếu (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			trả_về 0;
		}
	}
	trả_về 1;	/* no redirect */
}

char *sysfs_blkdev_get_name(struct path_cxt *pc, char *buf, size_t bufsiz)
{
	char link[PATH_MAX];
	char *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, sizeof(link), NULL);
	nếu (sz < 0)
		trả_về NULL;

	name = strrchr(link, '/');
	nếu (!name)
		trả_về NULL;

	name++;
	sz = strlen(name);
	nếu ((size_t) sz + 1 > bufsiz)
		trả_về NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	trả_về buf;
}

số_nguyên sysfs_blkdev_is_partition_dirent(DIR *dir, struct dirent *d, const char *parent_name)
{
	char path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	nếu (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		trả_về 0;
#endif
	size_t len = 0;

	nếu (parent_name) {
		const char *p = parent_name;

		/* /dev/sda --> "sda" */
		nếu (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			nếu (!p)
				trả_về 0;
			p++;
		}

		len = strlen(p);
		nếu ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	nếu (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		trả_về ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, sizeof(path), "%s/start", d->d_name);

	trả_về faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

số_nguyên sysfs_blkdev_count_partitions(struct path_cxt *pc, const char *devname)
{
	DIR *dir;
	struct dirent *d;
	số_nguyên r = 0;

	dir = ul_path_opendir(pc, NULL);
	nếu (!dir)
		trả_về 0;

	trong_khi ((d = xreaddir(dir))) {
		nếu (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	trả_về r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(struct path_cxt *pc, số_nguyên partno)
{
	DIR *dir;
	struct dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, NULL);
	nếu (!dir)
		trả_về 0;

	trong_khi ((d = xreaddir(dir))) {
		số_nguyên n;

		nếu (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		nếu (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		nếu (n == partno) {
			nếu (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				break;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (số_nguyên) partno, (số_nguyên) devno));
	trả_về devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
char *sysfs_blkdev_get_slave(struct path_cxt *pc)
{
	DIR *dir;
	struct dirent *d;
	char *name = NULL;

	dir = ul_path_opendir(pc, "slaves");
	nếu (!dir)
		trả_về NULL;

	trong_khi ((d = xreaddir(dir))) {
		nếu (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	trả_về name;
err:
	free(name);
	closedir(dir);
	trả_về NULL;
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
static char *get_subsystem(char *chain, char *buf, size_t bufsz)
{
	size_t len;
	char *p;

	nếu (!chain || !*chain)
		trả_về NULL;

	len = strlen(chain);
	nếu (len + sizeof(SUBSYSTEM_LINKNAME) > PATH_MAX)
		trả_về NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, sizeof(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		nếu (p) {
			*p = '\0';
			len = p - chain;
		}

		nếu (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			trả_về basename(buf);
		}

	} trong_khi (p);

	trả_về NULL;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
char *sysfs_blkdev_get_devchain(struct path_cxt *pc, char *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	struct ul_buffer tmp = UL_INIT_BUFFER;
	const char *p;
	char *res = NULL;

	ssz = ul_path_readlink(pc, buf, bufsz, NULL);
	nếu (ssz <= 0)
		trả_về NULL;

	nếu ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	nếu (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	trả_về res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
số_nguyên sysfs_blkdev_next_subsystem(struct path_cxt *pc __attribute__((unused)),
			 char *devchain, char **subsys)
{
	char subbuf[PATH_MAX];
	char *sub;

	nếu (!subsys || !devchain)
		trả_về -EINVAL;

	*subsys = NULL;

	trong_khi ((sub = get_subsystem(devchain, subbuf, sizeof(subbuf)))) {
		*subsys = strdup(sub);
		nếu (!*subsys)
			trả_về -ENOMEM;
		trả_về 0;
	}

	trả_về 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
static số_nguyên sysfs_devchain_is_removable(char *chain)
{
	size_t len;
	char buf[20];
	char *p;

	nếu (!chain || !*chain)
		trả_về 0;

	len = strlen(chain);
	nếu (len + sizeof(REMOVABLE_FILENAME) > PATH_MAX)
		trả_về 0;

	do {
		số_nguyên fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, sizeof(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		nếu (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			break;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		nếu (fd != -1) {
			rc = ul_read_all(fd, buf, sizeof(buf));
			close(fd);

			nếu (rc > 0) {
				nếu (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					trả_về 0;
				} không_thì nếu (strncmp(buf, "removable", min(rc, 9)) == 0) {
					trả_về 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		nếu (p) {
			*p = '\0';
			len = p - chain;
		}

	} trong_khi (p);

	trả_về 0;
}

số_nguyên sysfs_blkdev_is_hotpluggable(struct path_cxt *pc)
{
	char buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, sizeof(buf));
	trả_về sysfs_devchain_is_removable(chain);
}

số_nguyên sysfs_blkdev_is_removable(struct path_cxt *pc)
{
	số_nguyên rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	nếu (ul_path_read_s32(pc, &rc, "removable") == 0)
		trả_về rc;

	trả_về 0;
}

static số_nguyên get_dm_wholedisk(struct path_cxt *pc, char *diskname,
                            size_t len, dev_t *diskdevno)
{
	số_nguyên rc = 0;
	char *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	nếu (!name)
		trả_về -1;

	nếu (diskname && len)
		xstrncpy(diskname, name, len);

	nếu (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		nếu (!*diskdevno)
			rc = -1;
	}

	free(name);
	trả_về rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
số_nguyên sysfs_blkdev_get_wholedisk(	struct path_cxt *pc,
				char *diskname,
				size_t len,
				dev_t *diskdevno)
{
	số_nguyên is_part = 0;

	nếu (!pc)
		trả_về -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	nếu (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		char *uuid = NULL, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : NULL;

		nếu (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		nếu (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	nếu (!is_part) {
		/*
		 * unpartitioned device
		 */
		nếu (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		nếu (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} không_thì {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		char linkpath[PATH_MAX];
		char *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, sizeof(linkpath), NULL);
		nếu (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		nếu (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		nếu (diskname && len)
			xstrncpy(diskname, name, len);

		nếu (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			nếu (!*diskdevno)
				goto err;
		}
	}

done:
	trả_về 0;
err:
	trả_về -1;
}

số_nguyên sysfs_devno_to_wholedisk(dev_t devno, char *diskname,
		             size_t len, dev_t *diskdevno)
{
	struct path_cxt *pc;
	số_nguyên rc = 0;

	nếu (!devno)
		trả_về -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	nếu (!pc)
		trả_về -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	trả_về rc;
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
số_nguyên sysfs_devno_is_dm_hidden(dev_t devno, char **uuid,
			     const struct ul_vfs_ops *vfs)
{
	struct path_cxt *pc = NULL;
	char *id = NULL;
	số_nguyên rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	nếu (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	nếu (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	nếu (strncmp(id, "LVM-", 4) == 0) {
		char *p = strrchr(id + 4, '-');

		nếu (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	nếu (uuid)
		*uuid = id;
	không_thì
		free(id);
	trả_về rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
số_nguyên sysfs_devno_is_dm_private(dev_t devno, char **uuid,
			      const struct ul_vfs_ops *vfs)
{
	struct path_cxt *pc = NULL;
	char *id = NULL;
	số_nguyên rc = 0;

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	nếu (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	nếu (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	nếu (strncmp(id, "LVM-", 4) == 0) {
		char *p = strrchr(id + 4, '-');

		nếu (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} không_thì nếu (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	nếu (uuid)
		*uuid = id;
	không_thì
		free(id);
	trả_về rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
số_nguyên sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	nếu (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		trả_về -1;

	trả_về devno == disk;
}


số_nguyên sysfs_blkdev_scsi_get_hctl(struct path_cxt *pc, số_nguyên *h, số_nguyên *c, số_nguyên *t, số_nguyên *l)
{
	char buf[PATH_MAX], *hctl;
	struct sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	nếu (!blk || blk->hctl_error)
		trả_về -EINVAL;
	nếu (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, sizeof(buf), "device");
	nếu (len < 0)
		trả_về len;

	hctl = strrchr(buf, '/');
	nếu (!hctl)
		trả_về -1;
	hctl++;

	nếu (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		trả_về -1;

	blk->has_hctl = 1;
done:
	nếu (h)
		*h = blk->scsi_host;
	nếu (c)
		*c = blk->scsi_channel;
	nếu (t)
		*t = blk->scsi_target;
	nếu (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	trả_về 0;
}


static char *scsi_host_attribute_path(
			struct path_cxt *pc,
			const char *type,
			char *buf,
			size_t bufsz,
			const char *attr)
{
	số_nguyên len;
	số_nguyên host;
	const char *prefix;

	nếu (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		trả_về NULL;

	prefix = ul_path_get_prefix(pc);
	nếu (!prefix)
		prefix = "";

	nếu (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	không_thì
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	trả_về (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

char *sysfs_blkdev_scsi_host_strdup_attribute(struct path_cxt *pc,
			const char *type, const char *attr)
{
	char buf[1024];
	số_nguyên rc;
	FILE *f;

	nếu (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, sizeof(buf), attr))
		trả_về NULL;

	nếu (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		trả_về NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	trả_về rc == 1 ? strdup(buf) : NULL;
}

số_nguyên sysfs_blkdev_scsi_host_is(struct path_cxt *pc, const char *type)
{
	char buf[PATH_MAX];
	struct stat st;

	nếu (!type || !scsi_host_attribute_path(pc, type,
				buf, sizeof(buf), NULL))
		trả_về 0;

	trả_về stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static char *scsi_attribute_path(struct path_cxt *pc,
		char *buf, size_t bufsz, const char *attr)
{
	số_nguyên len, h, c, t, l;
	const char *prefix;

	nếu (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		trả_về NULL;

	prefix = ul_path_get_prefix(pc);
	nếu (!prefix)
		prefix = "";

	nếu (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	không_thì
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	trả_về (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

số_nguyên sysfs_blkdev_scsi_has_attribute(struct path_cxt *pc, const char *attr)
{
	char path[PATH_MAX];
	struct stat st;

	nếu (!scsi_attribute_path(pc, path, sizeof(path), attr))
		trả_về 0;

	trả_về stat(path, &st) == 0;
}

số_nguyên sysfs_blkdev_scsi_path_contains(struct path_cxt *pc, const char *pattern)
{
	char path[PATH_MAX], linkc[PATH_MAX];
	struct stat st;
	ssize_t len;

	nếu (!scsi_attribute_path(pc, path, sizeof(path), NULL))
		trả_về 0;

	nếu (stat(path, &st) != 0)
		trả_về 0;

	len = readlink(path, linkc, sizeof(linkc) - 1);
	nếu (len < 0)
		trả_về 0;

	linkc[len] = '\0';
	trả_về strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const char *path)
{
	FILE *f;
	số_nguyên maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	nếu (!f)
		trả_về 0;

	nếu (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	trả_về dev;
}

số_nguyên sysfs_devname_is_hidden(const char *prefix, const char *name)
{
	char buf[PATH_MAX];
	số_nguyên rc = 0, hidden = 0, len;
	FILE *f;

	nếu (strncmp("/dev/", name, 5) == 0)
		trả_về 0;

	nếu (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, sizeof(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	nếu (len < 0 || (size_t) len + 1 > sizeof(buf))
		trả_về 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	nếu (!f)
		trả_về 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	trả_về rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const char *prefix, const char *name, const char *parent)
{
	char buf[PATH_MAX];
	char *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	số_nguyên len;

	nếu (!prefix)
		prefix = "";

	assert(name);

	nếu (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		struct stat st;

		nếu (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	nếu (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	nếu (parent) {
		_parent = strdup(parent);
		nếu (!_parent)
			goto done;
	}

	nếu (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, sizeof(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		nếu (len < 0 || (size_t) len >= sizeof(buf))
			goto done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		goto done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, sizeof(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	nếu (len < 0 || (size_t) len >= sizeof(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	nếu (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, sizeof(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		nếu (len < 0 || (size_t) len >= sizeof(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	nếu (!dev) {
		len = snprintf(buf, sizeof(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		nếu (len < 0 || (size_t) len >= sizeof(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	trả_về dev;
}

dev_t sysfs_devname_to_devno(const char *name)
{
	trả_về __sysfs_devname_to_devno(NULL, name, NULL);
}

char *sysfs_blkdev_get_path(struct path_cxt *pc, char *buf, size_t bufsiz)
{
	const char *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	char *res = NULL;
	size_t sz;
	struct stat st;

	nếu (!name)
		goto done;

	sz = strlen(name);
	nếu (sz + sizeof("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	nếu (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	trả_về res;
}

dev_t sysfs_blkdev_get_devno(struct path_cxt *pc)
{
	trả_về ((struct sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
char *sysfs_devno_to_devpath(dev_t devno, char *buf, size_t bufsiz)
{
	struct path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	char *res = NULL;

	nếu (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	trả_về res;
}

char *sysfs_devno_to_devname(dev_t devno, char *buf, size_t bufsiz)
{
	struct path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	char *res = NULL;

	nếu (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	trả_về res;
}

số_nguyên sysfs_devno_count_partitions(dev_t devno)
{
	struct path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	số_nguyên n = 0;

	nếu (pc) {
		char buf[PATH_MAX + 1];
		char *name = sysfs_blkdev_get_name(pc, buf, sizeof(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	trả_về n;
}

char *sysfs_chrdev_devno_to_devname(dev_t devno, char *buf, size_t bufsiz)
{
	char link[PATH_MAX];
	struct path_cxt *pc;
	char *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	nếu (!pc)
		trả_về NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, sizeof(link), NULL);
	ul_unref_path(pc);

	nếu (sz < 0)
		trả_về NULL;

	name = strrchr(link, '/');
	nếu (!name)
		trả_về NULL;

	name++;
	sz = strlen(name);
	nếu ((size_t) sz + 1 > bufsiz)
		trả_về NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	trả_về buf;

}

enum sysfs_byteorder sysfs_get_byteorder(struct path_cxt *pc)
{
	số_nguyên rc;
	char buf[BUFSIZ];
	enum sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, sizeof(buf), _PATH_SYS_CPU_BYTEORDER);
	nếu (rc < 0)
		goto unknown;

	nếu (strncmp(buf, "little", sizeof(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} không_thì nếu (strncmp(buf, "big", sizeof(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#nếu __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#không_thì
#error Unknown byte order
#endif

out:
	trả_về ret;
}

số_nguyên sysfs_get_address_bits(struct path_cxt *pc)
{
	số_nguyên rc;
	số_nguyên address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	nếu (rc < 0)
		trả_về rc;
	nếu (address_bits < 0)
		trả_về -EINVAL;
	trả_về address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#include <errno.h>
#include <stdlib.h>

số_nguyên main(số_nguyên argc, char *argv[])
{
	struct path_cxt *pc;
	char *devname;
	dev_t devno, disk_devno;
	char path[PATH_MAX], *sub, *chain;
	char diskname[32];
	số_nguyên i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	nếu (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	nếu (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned số_nguyên) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, sizeof(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, sizeof(path)));

	sysfs_devno_to_wholedisk(devno, diskname, sizeof(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned số_nguyên) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	nếu (!pc)
		goto done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned số_nguyên) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, sizeof(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, sizeof(path)));

	sysfs_devno_to_wholedisk(devno, diskname, sizeof(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned số_nguyên) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	nếu (is_part && disk_devno) {
		struct path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	nếu (!is_part) {
		printf("First 5 partitions:\n");
		cho (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			nếu (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	nếu (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	không_thì
		printf(" SIZE: %"PRIu64"\n", u64);

	nếu (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	không_thì
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, sizeof(path));
	printf(" SUBSYSTEMS:\n");

	trong_khi (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	trả_về rc;
}
#endif /* TEST_PROGRAM_SYSFS */
