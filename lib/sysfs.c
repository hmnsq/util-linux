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
	ᮚᮦᮔ᮪ (ulsysfs_debug_mask)
		ᮝᮜᮤ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_sysfs_path(dev_t devno, sa_nracanaa path_cxt *parent, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ᮚᮦᮔ᮪ (!pc)
		ᮝᮜᮤ NULL;
	ᮚᮦᮔ᮪ (prefix)
		ul_path_set_prefix(pc, prefix);

	ᮚᮦᮔ᮪ (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ᮝᮜᮤ NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ᮝᮜᮤ pc;
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
	ᮚᮦᮔ᮪ (rc)
		ᮝᮜᮤ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ᮚᮦᮔ᮪ (rc < 0)
		ᮝᮜᮤ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	ᮚᮦᮔ᮪ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, parimaa_na(sa_nracanaa sysfs_blkdev));
		ᮚᮦᮔ᮪ (!blk)
			ᮝᮜᮤ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ᮝᮜᮤ 0;
}

static void sysfs_blkdev_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk;

	ᮚᮦᮔ᮪ (!pc)
		ᮝᮜᮤ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	ᮚᮦᮔ᮪ (!blk)
		ᮝᮜᮤ;

	ul_unref_path(blk->parent);
	free(blk);

	ul_path_set_dialect(pc, NULL, NULL);
}

puur_naa_nka sysfs_blkdev_set_parent(sa_nracanaa path_cxt *pc, sa_nracanaa path_cxt *parent)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ᮚᮦᮔ᮪ (!pc || !blk)
		ᮝᮜᮤ -EINVAL;

	ᮚᮦᮔ᮪ (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = NULL;
	}

	ᮚᮦᮔ᮪ (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} ᮜᮤᮚ
		blk->parent = NULL;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ᮝᮜᮤ 0;
}

sa_nracanaa path_cxt *sysfs_blkdev_get_parent(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ᮝᮜᮤ blk ? blk->parent : NULL;
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

	ᮚᮦᮔ᮪ (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		ᮚᮦᮔ᮪ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ᮝᮜᮤ 0;
		}
	}
	ᮝᮜᮤ 1;	/* no redirect */
}

var_na *sysfs_blkdev_get_name(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	var_na *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ᮚᮦᮔ᮪ (sz < 0)
		ᮝᮜᮤ NULL;

	name = strrchr(link, '/');
	ᮚᮦᮔ᮪ (!name)
		ᮝᮜᮤ NULL;

	name++;
	sz = strlen(name);
	ᮚᮦᮔ᮪ ((size_t) sz + 1 > bufsiz)
		ᮝᮜᮤ NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ᮝᮜᮤ buf;
}

puur_naa_nka sysfs_blkdev_is_partition_dirent(DIR *dir, sa_nracanaa dirent *d, const var_na *parent_name)
{
	var_na path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	ᮚᮦᮔ᮪ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ᮝᮜᮤ 0;
#endif
	size_t len = 0;

	ᮚᮦᮔ᮪ (parent_name) {
		const var_na *p = parent_name;

		/* /dev/sda --> "sda" */
		ᮚᮦᮔ᮪ (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			ᮚᮦᮔ᮪ (!p)
				ᮝᮜᮤ 0;
			p++;
		}

		len = strlen(p);
		ᮚᮦᮔ᮪ ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	ᮚᮦᮔ᮪ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ᮝᮜᮤ ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, parimaa_na(path), "%s/start", d->d_name);

	ᮝᮜᮤ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

puur_naa_nka sysfs_blkdev_count_partitions(sa_nracanaa path_cxt *pc, const var_na *devname)
{
	DIR *dir;
	sa_nracanaa dirent *d;
	puur_naa_nka r = 0;

	dir = ul_path_opendir(pc, NULL);
	ᮚᮦᮔ᮪ (!dir)
		ᮝᮜᮤ 0;

	ᮞᮨᮓᮨᮜ ((d = xreaddir(dir))) {
		ᮚᮦᮔ᮪ (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ᮝᮜᮤ r;
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
	ᮚᮦᮔ᮪ (!dir)
		ᮝᮜᮤ 0;

	ᮞᮨᮓᮨᮜ ((d = xreaddir(dir))) {
		puur_naa_nka n;

		ᮚᮦᮔ᮪ (!sysfs_blkdev_is_partition_dirent(dir, d, NULL))
			continue;

		ᮚᮦᮔ᮪ (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			continue;

		ᮚᮦᮔ᮪ (n == partno) {
			ᮚᮦᮔ᮪ (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				bha_nga;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (puur_naa_nka) partno, (puur_naa_nka) devno));
	ᮝᮜᮤ devno;
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
	ᮚᮦᮔ᮪ (!dir)
		ᮝᮜᮤ NULL;

	ᮞᮨᮓᮨᮜ ((d = xreaddir(dir))) {
		ᮚᮦᮔ᮪ (name)
			goto err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ᮝᮜᮤ name;
err:
	free(name);
	closedir(dir);
	ᮝᮜᮤ NULL;
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

	ᮚᮦᮔ᮪ (!chain || !*chain)
		ᮝᮜᮤ NULL;

	len = strlen(chain);
	ᮚᮦᮔ᮪ (len + parimaa_na(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ᮝᮜᮤ NULL;

	do {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, parimaa_na(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ᮚᮦᮔ᮪ (p) {
			*p = '\0';
			len = p - chain;
		}

		ᮚᮦᮔ᮪ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ᮝᮜᮤ basename(buf);
		}

	} ᮞᮨᮓᮨᮜ (p);

	ᮝᮜᮤ NULL;
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
	ᮚᮦᮔ᮪ (ssz <= 0)
		ᮝᮜᮤ NULL;

	ᮚᮦᮔ᮪ ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, NULL);
	ᮚᮦᮔ᮪ (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ᮝᮜᮤ res;
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

	ᮚᮦᮔ᮪ (!subsys || !devchain)
		ᮝᮜᮤ -EINVAL;

	*subsys = NULL;

	ᮞᮨᮓᮨᮜ ((sub = get_subsystem(devchain, subbuf, parimaa_na(subbuf)))) {
		*subsys = strdup(sub);
		ᮚᮦᮔ᮪ (!*subsys)
			ᮝᮜᮤ -ENOMEM;
		ᮝᮜᮤ 0;
	}

	ᮝᮜᮤ 1;
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

	ᮚᮦᮔ᮪ (!chain || !*chain)
		ᮝᮜᮤ 0;

	len = strlen(chain);
	ᮚᮦᮔ᮪ (len + parimaa_na(REMOVABLE_FILENAME) > PATH_MAX)
		ᮝᮜᮤ 0;

	do {
		puur_naa_nka fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, parimaa_na(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ᮚᮦᮔ᮪ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			bha_nga;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ᮚᮦᮔ᮪ (fd != -1) {
			rc = ul_read_all(fd, buf, parimaa_na(buf));
			close(fd);

			ᮚᮦᮔ᮪ (rc > 0) {
				ᮚᮦᮔ᮪ (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ᮝᮜᮤ 0;
				} ᮜᮤᮚ ᮚᮦᮔ᮪ (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ᮝᮜᮤ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ᮚᮦᮔ᮪ (p) {
			*p = '\0';
			len = p - chain;
		}

	} ᮞᮨᮓᮨᮜ (p);

	ᮝᮜᮤ 0;
}

puur_naa_nka sysfs_blkdev_is_hotpluggable(sa_nracanaa path_cxt *pc)
{
	var_na buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, parimaa_na(buf));
	ᮝᮜᮤ sysfs_devchain_is_removable(chain);
}

puur_naa_nka sysfs_blkdev_is_removable(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ᮚᮦᮔ᮪ (ul_path_read_s32(pc, &rc, "removable") == 0)
		ᮝᮜᮤ rc;

	ᮝᮜᮤ 0;
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
	ᮚᮦᮔ᮪ (!name)
		ᮝᮜᮤ -1;

	ᮚᮦᮔ᮪ (diskname && len)
		xstrncpy(diskname, name, len);

	ᮚᮦᮔ᮪ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
		ᮚᮦᮔ᮪ (!*diskdevno)
			rc = -1;
	}

	free(name);
	ᮝᮜᮤ rc;
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

	ᮚᮦᮔ᮪ (!pc)
		ᮝᮜᮤ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	ᮚᮦᮔ᮪ (!is_part) {
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

		ᮚᮦᮔ᮪ (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		free(uuid);

		ᮚᮦᮔ᮪ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			goto done;

		is_part = 0;
	}

	ᮚᮦᮔ᮪ (!is_part) {
		/*
		 * unpartitioned device
		 */
		ᮚᮦᮔ᮪ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			goto err;
		ᮚᮦᮔ᮪ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ᮜᮤᮚ {
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
		ᮚᮦᮔ᮪ (linklen < 0)
			goto err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		ᮚᮦᮔ᮪ (!name)
			goto err;

		sysfs_devname_sys_to_dev(name);
		ᮚᮦᮔ᮪ (diskname && len)
			xstrncpy(diskname, name, len);

		ᮚᮦᮔ᮪ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, NULL);
			ᮚᮦᮔ᮪ (!*diskdevno)
				goto err;
		}
	}

done:
	ᮝᮜᮤ 0;
err:
	ᮝᮜᮤ -1;
}

puur_naa_nka sysfs_devno_to_wholedisk(dev_t devno, var_na *diskname,
		             size_t len, dev_t *diskdevno)
{
	sa_nracanaa path_cxt *pc;
	puur_naa_nka rc = 0;

	ᮚᮦᮔ᮪ (!devno)
		ᮝᮜᮤ -EINVAL;
	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ᮚᮦᮔ᮪ (!pc)
		ᮝᮜᮤ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ᮝᮜᮤ rc;
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
	ᮚᮦᮔ᮪ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	ᮚᮦᮔ᮪ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ᮚᮦᮔ᮪ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		ᮚᮦᮔ᮪ (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	ᮚᮦᮔ᮪ (uuid)
		*uuid = id;
	ᮜᮤᮚ
		free(id);
	ᮝᮜᮤ rc;
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
	ᮚᮦᮔ᮪ (!pc)
		goto done;
	ul_path_refer_vfs(pc, vfs);
	ᮚᮦᮔ᮪ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		goto done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ᮚᮦᮔ᮪ (strncmp(id, "LVM-", 4) == 0) {
		var_na *p = strrchr(id + 4, '-');

		ᮚᮦᮔ᮪ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	ᮚᮦᮔ᮪ (uuid)
		*uuid = id;
	ᮜᮤᮚ
		free(id);
	ᮝᮜᮤ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
puur_naa_nka sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ᮚᮦᮔ᮪ (sysfs_devno_to_wholedisk(devno, NULL, 0, &disk) != 0)
		ᮝᮜᮤ -1;

	ᮝᮜᮤ devno == disk;
}


puur_naa_nka sysfs_blkdev_scsi_get_hctl(sa_nracanaa path_cxt *pc, puur_naa_nka *h, puur_naa_nka *c, puur_naa_nka *t, puur_naa_nka *l)
{
	var_na buf[PATH_MAX], *hctl;
	sa_nracanaa sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	ᮚᮦᮔ᮪ (!blk || blk->hctl_error)
		ᮝᮜᮤ -EINVAL;
	ᮚᮦᮔ᮪ (blk->has_hctl)
		goto done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, parimaa_na(buf), "device");
	ᮚᮦᮔ᮪ (len < 0)
		ᮝᮜᮤ len;

	hctl = strrchr(buf, '/');
	ᮚᮦᮔ᮪ (!hctl)
		ᮝᮜᮤ -1;
	hctl++;

	ᮚᮦᮔ᮪ (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ᮝᮜᮤ -1;

	blk->has_hctl = 1;
done:
	ᮚᮦᮔ᮪ (h)
		*h = blk->scsi_host;
	ᮚᮦᮔ᮪ (c)
		*c = blk->scsi_channel;
	ᮚᮦᮔ᮪ (t)
		*t = blk->scsi_target;
	ᮚᮦᮔ᮪ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ᮝᮜᮤ 0;
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

	ᮚᮦᮔ᮪ (sysfs_blkdev_scsi_get_hctl(pc, &host, NULL, NULL, NULL))
		ᮝᮜᮤ NULL;

	prefix = ul_path_get_prefix(pc);
	ᮚᮦᮔ᮪ (!prefix)
		prefix = "";

	ᮚᮦᮔ᮪ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	ᮜᮤᮚ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ᮝᮜᮤ (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

var_na *sysfs_blkdev_scsi_host_strdup_attribute(sa_nracanaa path_cxt *pc,
			const var_na *type, const var_na *attr)
{
	var_na buf[1024];
	puur_naa_nka rc;
	FILE *f;

	ᮚᮦᮔ᮪ (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, parimaa_na(buf), attr))
		ᮝᮜᮤ NULL;

	ᮚᮦᮔ᮪ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ᮝᮜᮤ NULL;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ᮝᮜᮤ rc == 1 ? strdup(buf) : NULL;
}

puur_naa_nka sysfs_blkdev_scsi_host_is(sa_nracanaa path_cxt *pc, const var_na *type)
{
	var_na buf[PATH_MAX];
	sa_nracanaa stat st;

	ᮚᮦᮔ᮪ (!type || !scsi_host_attribute_path(pc, type,
				buf, parimaa_na(buf), NULL))
		ᮝᮜᮤ 0;

	ᮝᮜᮤ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

static var_na *scsi_attribute_path(sa_nracanaa path_cxt *pc,
		var_na *buf, size_t bufsz, const var_na *attr)
{
	puur_naa_nka len, h, c, t, l;
	const var_na *prefix;

	ᮚᮦᮔ᮪ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ᮝᮜᮤ NULL;

	prefix = ul_path_get_prefix(pc);
	ᮚᮦᮔ᮪ (!prefix)
		prefix = "";

	ᮚᮦᮔ᮪ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	ᮜᮤᮚ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ᮝᮜᮤ (len < 0 || (size_t) len >= bufsz) ? NULL : buf;
}

puur_naa_nka sysfs_blkdev_scsi_has_attribute(sa_nracanaa path_cxt *pc, const var_na *attr)
{
	var_na path[PATH_MAX];
	sa_nracanaa stat st;

	ᮚᮦᮔ᮪ (!scsi_attribute_path(pc, path, parimaa_na(path), attr))
		ᮝᮜᮤ 0;

	ᮝᮜᮤ stat(path, &st) == 0;
}

puur_naa_nka sysfs_blkdev_scsi_path_contains(sa_nracanaa path_cxt *pc, const var_na *pattern)
{
	var_na path[PATH_MAX], linkc[PATH_MAX];
	sa_nracanaa stat st;
	ssize_t len;

	ᮚᮦᮔ᮪ (!scsi_attribute_path(pc, path, parimaa_na(path), NULL))
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (stat(path, &st) != 0)
		ᮝᮜᮤ 0;

	len = readlink(path, linkc, parimaa_na(linkc) - 1);
	ᮚᮦᮔ᮪ (len < 0)
		ᮝᮜᮤ 0;

	linkc[len] = '\0';
	ᮝᮜᮤ strstr(linkc, pattern) != NULL;
}

static dev_t read_devno(const var_na *path)
{
	FILE *f;
	puur_naa_nka maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	ᮚᮦᮔ᮪ (!f)
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ᮝᮜᮤ dev;
}

puur_naa_nka sysfs_devname_is_hidden(const var_na *prefix, const var_na *name)
{
	var_na buf[PATH_MAX];
	puur_naa_nka rc = 0, hidden = 0, len;
	FILE *f;

	ᮚᮦᮔ᮪ (strncmp("/dev/", name, 5) == 0)
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, parimaa_na(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	ᮚᮦᮔ᮪ (len < 0 || (size_t) len + 1 > parimaa_na(buf))
		ᮝᮜᮤ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ᮚᮦᮔ᮪ (!f)
		ᮝᮜᮤ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ᮝᮜᮤ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(const var_na *prefix, const var_na *name, const var_na *parent)
{
	var_na buf[PATH_MAX];
	var_na *_name = NULL, *_parent = NULL;	/* name as encoded in sysfs */
	dev_t dev = 0;
	puur_naa_nka len;

	ᮚᮦᮔ᮪ (!prefix)
		prefix = "";

	assert(name);

	ᮚᮦᮔ᮪ (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		sa_nracanaa stat st;

		ᮚᮦᮔ᮪ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			goto done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ᮚᮦᮔ᮪ (!_name)
		goto done;
	sysfs_devname_dev_to_sys(_name);

	ᮚᮦᮔ᮪ (parent) {
		_parent = strdup(parent);
		ᮚᮦᮔ᮪ (!_parent)
			goto done;
	}

	ᮚᮦᮔ᮪ (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		ᮚᮦᮔ᮪ (len < 0 || (size_t) len >= parimaa_na(buf))
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
	ᮚᮦᮔ᮪ (len < 0 || (size_t) len >= parimaa_na(buf))
		goto done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ᮚᮦᮔ᮪ (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		ᮚᮦᮔ᮪ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ᮚᮦᮔ᮪ (!dev) {
		len = snprintf(buf, parimaa_na(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		ᮚᮦᮔ᮪ (len < 0 || (size_t) len >= parimaa_na(buf))
			goto done;
		dev = read_devno(buf);
	}
done:
	free(_name);
	free(_parent);
	ᮝᮜᮤ dev;
}

dev_t sysfs_devname_to_devno(const var_na *name)
{
	ᮝᮜᮤ __sysfs_devname_to_devno(NULL, name, NULL);
}

var_na *sysfs_blkdev_get_path(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz)
{
	const var_na *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	var_na *res = NULL;
	size_t sz;
	sa_nracanaa stat st;

	ᮚᮦᮔ᮪ (!name)
		goto done;

	sz = strlen(name);
	ᮚᮦᮔ᮪ (sz + parimaa_na("/dev/") > bufsiz)
		goto done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	ᮚᮦᮔ᮪ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ᮝᮜᮤ res;
}

dev_t sysfs_blkdev_get_devno(sa_nracanaa path_cxt *pc)
{
	ᮝᮜᮤ ((sa_nracanaa sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
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

	ᮚᮦᮔ᮪ (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ᮝᮜᮤ res;
}

var_na *sysfs_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	var_na *res = NULL;

	ᮚᮦᮔ᮪ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ᮝᮜᮤ res;
}

puur_naa_nka sysfs_devno_count_partitions(dev_t devno)
{
	sa_nracanaa path_cxt *pc = ul_new_sysfs_path(devno, NULL, NULL);
	puur_naa_nka n = 0;

	ᮚᮦᮔ᮪ (pc) {
		var_na buf[PATH_MAX + 1];
		var_na *name = sysfs_blkdev_get_name(pc, buf, parimaa_na(buf));

		n = sysfs_blkdev_count_partitions(pc, name);
		ul_unref_path(pc);
	}
	ᮝᮜᮤ n;
}

var_na *sysfs_chrdev_devno_to_devname(dev_t devno, var_na *buf, size_t bufsiz)
{
	var_na link[PATH_MAX];
	sa_nracanaa path_cxt *pc;
	var_na *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ᮚᮦᮔ᮪ (!pc)
		ᮝᮜᮤ NULL;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, parimaa_na(link), NULL);
	ul_unref_path(pc);

	ᮚᮦᮔ᮪ (sz < 0)
		ᮝᮜᮤ NULL;

	name = strrchr(link, '/');
	ᮚᮦᮔ᮪ (!name)
		ᮝᮜᮤ NULL;

	name++;
	sz = strlen(name);
	ᮚᮦᮔ᮪ ((size_t) sz + 1 > bufsiz)
		ᮝᮜᮤ NULL;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ᮝᮜᮤ buf;

}

ga_nanaa sysfs_byteorder sysfs_get_byteorder(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc;
	var_na buf[BUFSIZ];
	ga_nanaa sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), _PATH_SYS_CPU_BYTEORDER);
	ᮚᮦᮔ᮪ (rc < 0)
		goto unknown;

	ᮚᮦᮔ᮪ (strncmp(buf, "little", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		goto out;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (strncmp(buf, "big", parimaa_na(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		goto out;
	}

unknown:
#ᮚᮦᮔ᮪ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ᮜᮤᮚ
#error Unknown byte order
#endif

out:
	ᮝᮜᮤ ret;
}

puur_naa_nka sysfs_get_address_bits(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc;
	puur_naa_nka address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	ᮚᮦᮔ᮪ (rc < 0)
		ᮝᮜᮤ rc;
	ᮚᮦᮔ᮪ (address_bits < 0)
		ᮝᮜᮤ -EINVAL;
	ᮝᮜᮤ address_bits;
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

	ᮚᮦᮔ᮪ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	ᮚᮦᮔ᮪ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned puur_naa_nka) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, parimaa_na(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, parimaa_na(path)));

	sysfs_devno_to_wholedisk(devno, diskname, parimaa_na(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned puur_naa_nka) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, NULL, NULL);
	ᮚᮦᮔ᮪ (!pc)
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

	ᮚᮦᮔ᮪ (is_part && disk_devno) {
		sa_nracanaa path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, NULL, NULL);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_count_dirents(pc, "slaves"));

	ᮚᮦᮔ᮪ (!is_part) {
		printf("First 5 partitions:\n");
		for (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			ᮚᮦᮔ᮪ (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ᮚᮦᮔ᮪ (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	ᮜᮤᮚ
		printf(" SIZE: %"PRIu64"\n", u64);

	ᮚᮦᮔ᮪ (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	ᮜᮤᮚ
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, parimaa_na(path));
	printf(" SUBSYSTEMS:\n");

	ᮞᮨᮓᮨᮜ (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		free(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ᮝᮜᮤ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
