/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <libgen.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "sysfs.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "buffer.h"

স্থির শূন্য sysfs_blkdev_deinit_path(সংরচনা path_cxt *pc);
স্থির পূর্ণাঙ্ক  sysfs_blkdev_enoent_redirect(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *path, পূর্ণাঙ্ক *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

শূন্য ul_sysfs_init_debug(শূন্য)
{
	যদি (ulsysfs_debug_mask)
		প্রত্যাগম;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

সংরচনা path_cxt *ul_new_sysfs_path(dev_t devno, সংরচনা path_cxt *parent, ধ্রুব বর্ণ *prefix)
{
	সংরচনা path_cxt *pc = ul_new_path(রিক্ত);

	যদি (!pc)
		প্রত্যাগম রিক্ত;
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		প্রত্যাগম রিক্ত;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	প্রত্যাগম pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
পূর্ণাঙ্ক sysfs_blkdev_init_path(সংরচনা path_cxt *pc, dev_t devno, সংরচনা path_cxt *parent)
{
	সংরচনা sysfs_blkdev *blk;
	পূর্ণাঙ্ক rc;
	বর্ণ buf[পরিমাণ(_PATH_SYS_DEVBLOCK)
		 + পরিমাণ(stringify_মান(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, পরিমাণ(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	যদি (rc)
		প্রত্যাগম rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	যদি (rc < 0)
		প্রত্যাগম rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	যদি (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, পরিমাণ(সংরচনা sysfs_blkdev));
		যদি (!blk)
			প্রত্যাগম -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	প্রত্যাগম 0;
}

স্থির শূন্য sysfs_blkdev_deinit_path(সংরচনা path_cxt *pc)
{
	সংরচনা sysfs_blkdev *blk;

	যদি (!pc)
		প্রত্যাগম;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	যদি (!blk)
		প্রত্যাগম;

	ul_unref_path(blk->parent);
	মুক্ত(blk);

	ul_path_set_dialect(pc, রিক্ত, রিক্ত);
}

পূর্ণাঙ্ক sysfs_blkdev_set_parent(সংরচনা path_cxt *pc, সংরচনা path_cxt *parent)
{
	সংরচনা sysfs_blkdev *blk = ul_path_get_dialect(pc);

	যদি (!pc || !blk)
		প্রত্যাগম -EINVAL;

	যদি (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = রিক্ত;
	}

	যদি (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} অন্যথা
		blk->parent = রিক্ত;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	প্রত্যাগম 0;
}

সংরচনা path_cxt *sysfs_blkdev_get_parent(সংরচনা path_cxt *pc)
{
	সংরচনা sysfs_blkdev *blk = ul_path_get_dialect(pc);
	প্রত্যাগম blk ? blk->parent : রিক্ত;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
স্থির পূর্ণাঙ্ক sysfs_blkdev_enoent_redirect(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *path, পূর্ণাঙ্ক *dirfd)
{
	সংরচনা sysfs_blkdev *blk = ul_path_get_dialect(pc);

	যদি (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		যদি (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			প্রত্যাগম 0;
		}
	}
	প্রত্যাগম 1;	/* no redirect */
}

বর্ণ *sysfs_blkdev_get_name(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsiz)
{
	বর্ণ link[PATH_MAX];
	বর্ণ *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, পরিমাণ(link), রিক্ত);
	যদি (sz < 0)
		প্রত্যাগম রিক্ত;

	name = strrchr(link, '/');
	যদি (!name)
		প্রত্যাগম রিক্ত;

	name++;
	sz = strlen(name);
	যদি ((size_t) sz + 1 > bufsiz)
		প্রত্যাগম রিক্ত;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	প্রত্যাগম buf;
}

পূর্ণাঙ্ক sysfs_blkdev_is_partition_dirent(DIR *dir, সংরচনা dirent *d, ধ্রুব বর্ণ *parent_name)
{
	বর্ণ path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		প্রত্যাগম 0;
#endif
	size_t len = 0;

	যদি (parent_name) {
		ধ্রুব বর্ণ *p = parent_name;

		/* /dev/sda --> "sda" */
		যদি (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			যদি (!p)
				প্রত্যাগম 0;
			p++;
		}

		len = strlen(p);
		যদি ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	যদি (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		প্রত্যাগম ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, পরিমাণ(path), "%s/start", d->d_name);

	প্রত্যাগম faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

পূর্ণাঙ্ক sysfs_blkdev_গুনতি_partitions(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *devname)
{
	DIR *dir;
	সংরচনা dirent *d;
	পূর্ণাঙ্ক r = 0;

	dir = ul_path_opendir(pc, রিক্ত);
	যদি (!dir)
		প্রত্যাগম 0;

	যাবত্ ((d = xreaddir(dir))) {
		যদি (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	প্রত্যাগম r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(সংরচনা path_cxt *pc, পূর্ণাঙ্ক partno)
{
	DIR *dir;
	সংরচনা dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, রিক্ত);
	যদি (!dir)
		প্রত্যাগম 0;

	যাবত্ ((d = xreaddir(dir))) {
		পূর্ণাঙ্ক n;

		যদি (!sysfs_blkdev_is_partition_dirent(dir, d, রিক্ত))
			অনুবর্ত;

		যদি (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			অনুবর্ত;

		যদি (n == partno) {
			যদি (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ভঙ্গ;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (পূর্ণাঙ্ক) partno, (পূর্ণাঙ্ক) devno));
	প্রত্যাগম devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
বর্ণ *sysfs_blkdev_get_slave(সংরচনা path_cxt *pc)
{
	DIR *dir;
	সংরচনা dirent *d;
	বর্ণ *name = রিক্ত;

	dir = ul_path_opendir(pc, "slaves");
	যদি (!dir)
		প্রত্যাগম রিক্ত;

	যাবত্ ((d = xreaddir(dir))) {
		যদি (name)
			গচ্ছ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	প্রত্যাগম name;
err:
	মুক্ত(name);
	closedir(dir);
	প্রত্যাগম রিক্ত;
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
স্থির বর্ণ *get_subsystem(বর্ণ *chain, বর্ণ *buf, size_t bufsz)
{
	size_t len;
	বর্ণ *p;

	যদি (!chain || !*chain)
		প্রত্যাগম রিক্ত;

	len = strlen(chain);
	যদি (len + পরিমাণ(SUBSYSTEM_LINKNAME) > PATH_MAX)
		প্রত্যাগম রিক্ত;

	কুরু {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, পরিমাণ(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		যদি (p) {
			*p = '\0';
			len = p - chain;
		}

		যদি (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			প্রত্যাগম basename(buf);
		}

	} যাবত্ (p);

	প্রত্যাগম রিক্ত;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
বর্ণ *sysfs_blkdev_get_devchain(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	সংরচনা ul_buffer tmp = UL_INIT_BUFFER;
	ধ্রুব বর্ণ *p;
	বর্ণ *res = রিক্ত;

	ssz = ul_path_readlink(pc, buf, bufsz, রিক্ত);
	যদি (ssz <= 0)
		প্রত্যাগম রিক্ত;

	যদি ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, রিক্ত);
	যদি (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	প্রত্যাগম res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
পূর্ণাঙ্ক sysfs_blkdev_next_subsystem(সংরচনা path_cxt *pc __attribute__((unused)),
			 বর্ণ *devchain, বর্ণ **subsys)
{
	বর্ণ subbuf[PATH_MAX];
	বর্ণ *sub;

	যদি (!subsys || !devchain)
		প্রত্যাগম -EINVAL;

	*subsys = রিক্ত;

	যাবত্ ((sub = get_subsystem(devchain, subbuf, পরিমাণ(subbuf)))) {
		*subsys = strdup(sub);
		যদি (!*subsys)
			প্রত্যাগম -ENOMEM;
		প্রত্যাগম 0;
	}

	প্রত্যাগম 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
স্থির পূর্ণাঙ্ক sysfs_devchain_is_removable(বর্ণ *chain)
{
	size_t len;
	বর্ণ buf[20];
	বর্ণ *p;

	যদি (!chain || !*chain)
		প্রত্যাগম 0;

	len = strlen(chain);
	যদি (len + পরিমাণ(REMOVABLE_FILENAME) > PATH_MAX)
		প্রত্যাগম 0;

	কুরু {
		পূর্ণাঙ্ক fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, পরিমাণ(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		যদি (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ভঙ্গ;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		যদি (fd != -1) {
			rc = ul_read_all(fd, buf, পরিমাণ(buf));
			close(fd);

			যদি (rc > 0) {
				যদি (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					প্রত্যাগম 0;
				} অন্যথা যদি (strncmp(buf, "removable", min(rc, 9)) == 0) {
					প্রত্যাগম 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		যদি (p) {
			*p = '\0';
			len = p - chain;
		}

	} যাবত্ (p);

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক sysfs_blkdev_is_hotpluggable(সংরচনা path_cxt *pc)
{
	বর্ণ buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, পরিমাণ(buf));
	প্রত্যাগম sysfs_devchain_is_removable(chain);
}

পূর্ণাঙ্ক sysfs_blkdev_is_removable(সংরচনা path_cxt *pc)
{
	পূর্ণাঙ্ক rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	যদি (ul_path_read_s32(pc, &rc, "removable") == 0)
		প্রত্যাগম rc;

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক get_dm_wholedisk(সংরচনা path_cxt *pc, বর্ণ *diskname,
                            size_t len, dev_t *diskdevno)
{
	পূর্ণাঙ্ক rc = 0;
	বর্ণ *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	যদি (!name)
		প্রত্যাগম -1;

	যদি (diskname && len)
		xstrncpy(diskname, name, len);

	যদি (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, রিক্ত);
		যদি (!*diskdevno)
			rc = -1;
	}

	মুক্ত(name);
	প্রত্যাগম rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
পূর্ণাঙ্ক sysfs_blkdev_get_wholedisk(	সংরচনা path_cxt *pc,
				বর্ণ *diskname,
				size_t len,
				dev_t *diskdevno)
{
	পূর্ণাঙ্ক is_part = 0;

	যদি (!pc)
		প্রত্যাগম -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	যদি (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		বর্ণ *uuid = রিক্ত, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : রিক্ত;

		যদি (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		মুক্ত(uuid);

		যদি (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			গচ্ছ done;

		is_part = 0;
	}

	যদি (!is_part) {
		/*
		 * unpartitioned device
		 */
		যদি (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			গচ্ছ err;
		যদি (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} অন্যথা {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		বর্ণ linkpath[PATH_MAX];
		বর্ণ *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, পরিমাণ(linkpath), রিক্ত);
		যদি (linklen < 0)
			গচ্ছ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		যদি (!name)
			গচ্ছ err;

		sysfs_devname_sys_to_dev(name);
		যদি (diskname && len)
			xstrncpy(diskname, name, len);

		যদি (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, রিক্ত);
			যদি (!*diskdevno)
				গচ্ছ err;
		}
	}

done:
	প্রত্যাগম 0;
err:
	প্রত্যাগম -1;
}

পূর্ণাঙ্ক sysfs_devno_to_wholedisk(dev_t devno, বর্ণ *diskname,
		             size_t len, dev_t *diskdevno)
{
	সংরচনা path_cxt *pc;
	পূর্ণাঙ্ক rc = 0;

	যদি (!devno)
		প্রত্যাগম -EINVAL;
	pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	যদি (!pc)
		প্রত্যাগম -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	প্রত্যাগম rc;
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
পূর্ণাঙ্ক sysfs_devno_is_dm_hidden(dev_t devno, বর্ণ **uuid,
			     ধ্রুব সংরচনা ul_vfs_ops *vfs)
{
	সংরচনা path_cxt *pc = রিক্ত;
	বর্ণ *id = রিক্ত;
	পূর্ণাঙ্ক rc = 0;

	pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	যদি (!pc)
		গচ্ছ done;
	ul_path_refer_vfs(pc, vfs);
	যদি (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		গচ্ছ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	যদি (strncmp(id, "LVM-", 4) == 0) {
		বর্ণ *p = strrchr(id + 4, '-');

		যদি (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	যদি (uuid)
		*uuid = id;
	অন্যথা
		মুক্ত(id);
	প্রত্যাগম rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
পূর্ণাঙ্ক sysfs_devno_is_dm_private(dev_t devno, বর্ণ **uuid,
			      ধ্রুব সংরচনা ul_vfs_ops *vfs)
{
	সংরচনা path_cxt *pc = রিক্ত;
	বর্ণ *id = রিক্ত;
	পূর্ণাঙ্ক rc = 0;

	pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	যদি (!pc)
		গচ্ছ done;
	ul_path_refer_vfs(pc, vfs);
	যদি (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		গচ্ছ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	যদি (strncmp(id, "LVM-", 4) == 0) {
		বর্ণ *p = strrchr(id + 4, '-');

		যদি (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} অন্যথা যদি (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	যদি (uuid)
		*uuid = id;
	অন্যথা
		মুক্ত(id);
	প্রত্যাগম rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
পূর্ণাঙ্ক sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	যদি (sysfs_devno_to_wholedisk(devno, রিক্ত, 0, &disk) != 0)
		প্রত্যাগম -1;

	প্রত্যাগম devno == disk;
}


পূর্ণাঙ্ক sysfs_blkdev_scsi_get_hctl(সংরচনা path_cxt *pc, পূর্ণাঙ্ক *h, পূর্ণাঙ্ক *c, পূর্ণাঙ্ক *t, পূর্ণাঙ্ক *l)
{
	বর্ণ buf[PATH_MAX], *hctl;
	সংরচনা sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	যদি (!blk || blk->hctl_error)
		প্রত্যাগম -EINVAL;
	যদি (blk->has_hctl)
		গচ্ছ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, পরিমাণ(buf), "device");
	যদি (len < 0)
		প্রত্যাগম len;

	hctl = strrchr(buf, '/');
	যদি (!hctl)
		প্রত্যাগম -1;
	hctl++;

	যদি (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		প্রত্যাগম -1;

	blk->has_hctl = 1;
done:
	যদি (h)
		*h = blk->scsi_host;
	যদি (c)
		*c = blk->scsi_channel;
	যদি (t)
		*t = blk->scsi_target;
	যদি (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	প্রত্যাগম 0;
}


স্থির বর্ণ *scsi_host_attribute_path(
			সংরচনা path_cxt *pc,
			ধ্রুব বর্ণ *type,
			বর্ণ *buf,
			size_t bufsz,
			ধ্রুব বর্ণ *attr)
{
	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক host;
	ধ্রুব বর্ণ *prefix;

	যদি (sysfs_blkdev_scsi_get_hctl(pc, &host, রিক্ত, রিক্ত, রিক্ত))
		প্রত্যাগম রিক্ত;

	prefix = ul_path_get_prefix(pc);
	যদি (!prefix)
		prefix = "";

	যদি (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	অন্যথা
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	প্রত্যাগম (len < 0 || (size_t) len >= bufsz) ? রিক্ত : buf;
}

বর্ণ *sysfs_blkdev_scsi_host_strdup_attribute(সংরচনা path_cxt *pc,
			ধ্রুব বর্ণ *type, ধ্রুব বর্ণ *attr)
{
	বর্ণ buf[1024];
	পূর্ণাঙ্ক rc;
	FILE *f;

	যদি (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, পরিমাণ(buf), attr))
		প্রত্যাগম রিক্ত;

	যদি (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		প্রত্যাগম রিক্ত;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	প্রত্যাগম rc == 1 ? strdup(buf) : রিক্ত;
}

পূর্ণাঙ্ক sysfs_blkdev_scsi_host_is(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *type)
{
	বর্ণ buf[PATH_MAX];
	সংরচনা stat st;

	যদি (!type || !scsi_host_attribute_path(pc, type,
				buf, পরিমাণ(buf), রিক্ত))
		প্রত্যাগম 0;

	প্রত্যাগম stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

স্থির বর্ণ *scsi_attribute_path(সংরচনা path_cxt *pc,
		বর্ণ *buf, size_t bufsz, ধ্রুব বর্ণ *attr)
{
	পূর্ণাঙ্ক len, h, c, t, l;
	ধ্রুব বর্ণ *prefix;

	যদি (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		প্রত্যাগম রিক্ত;

	prefix = ul_path_get_prefix(pc);
	যদি (!prefix)
		prefix = "";

	যদি (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	অন্যথা
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	প্রত্যাগম (len < 0 || (size_t) len >= bufsz) ? রিক্ত : buf;
}

পূর্ণাঙ্ক sysfs_blkdev_scsi_has_attribute(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *attr)
{
	বর্ণ path[PATH_MAX];
	সংরচনা stat st;

	যদি (!scsi_attribute_path(pc, path, পরিমাণ(path), attr))
		প্রত্যাগম 0;

	প্রত্যাগম stat(path, &st) == 0;
}

পূর্ণাঙ্ক sysfs_blkdev_scsi_path_contains(সংরচনা path_cxt *pc, ধ্রুব বর্ণ *pattern)
{
	বর্ণ path[PATH_MAX], linkc[PATH_MAX];
	সংরচনা stat st;
	ssize_t len;

	যদি (!scsi_attribute_path(pc, path, পরিমাণ(path), রিক্ত))
		প্রত্যাগম 0;

	যদি (stat(path, &st) != 0)
		প্রত্যাগম 0;

	len = readlink(path, linkc, পরিমাণ(linkc) - 1);
	যদি (len < 0)
		প্রত্যাগম 0;

	linkc[len] = '\0';
	প্রত্যাগম strstr(linkc, pattern) != রিক্ত;
}

স্থির dev_t read_devno(ধ্রুব বর্ণ *path)
{
	FILE *f;
	পূর্ণাঙ্ক maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	যদি (!f)
		প্রত্যাগম 0;

	যদি (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	প্রত্যাগম dev;
}

পূর্ণাঙ্ক sysfs_devname_is_hidden(ধ্রুব বর্ণ *prefix, ধ্রুব বর্ণ *name)
{
	বর্ণ buf[PATH_MAX];
	পূর্ণাঙ্ক rc = 0, hidden = 0, len;
	FILE *f;

	যদি (strncmp("/dev/", name, 5) == 0)
		প্রত্যাগম 0;

	যদি (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, পরিমাণ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	যদি (len < 0 || (size_t) len + 1 > পরিমাণ(buf))
		প্রত্যাগম 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	যদি (!f)
		প্রত্যাগম 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	প্রত্যাগম rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ধ্রুব বর্ণ *prefix, ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *parent)
{
	বর্ণ buf[PATH_MAX];
	বর্ণ *_name = রিক্ত, *_parent = রিক্ত;	/* name as encoded in sysfs */
	dev_t dev = 0;
	পূর্ণাঙ্ক len;

	যদি (!prefix)
		prefix = "";

	প্রতিজ্ঞা(name);

	যদি (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		সংরচনা stat st;

		যদি (stat(name, &st) == 0) {
			dev = st.st_rdev;
			গচ্ছ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	যদি (!_name)
		গচ্ছ done;
	sysfs_devname_dev_to_sys(_name);

	যদি (parent) {
		_parent = strdup(parent);
		যদি (!_parent)
			গচ্ছ done;
	}

	যদি (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, পরিমাণ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
			গচ্ছ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		গচ্ছ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, পরিমাণ(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
		গচ্ছ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	যদি (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, পরিমাণ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
			গচ্ছ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	যদি (!dev) {
		len = snprintf(buf, পরিমাণ(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		যদি (len < 0 || (size_t) len >= পরিমাণ(buf))
			গচ্ছ done;
		dev = read_devno(buf);
	}
done:
	মুক্ত(_name);
	মুক্ত(_parent);
	প্রত্যাগম dev;
}

dev_t sysfs_devname_to_devno(ধ্রুব বর্ণ *name)
{
	প্রত্যাগম __sysfs_devname_to_devno(রিক্ত, name, রিক্ত);
}

বর্ণ *sysfs_blkdev_get_path(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsiz)
{
	ধ্রুব বর্ণ *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	বর্ণ *res = রিক্ত;
	size_t sz;
	সংরচনা stat st;

	যদি (!name)
		গচ্ছ done;

	sz = strlen(name);
	যদি (sz + পরিমাণ("/dev/") > bufsiz)
		গচ্ছ done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	যদি (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	প্রত্যাগম res;
}

dev_t sysfs_blkdev_get_devno(সংরচনা path_cxt *pc)
{
	প্রত্যাগম ((সংরচনা sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
বর্ণ *sysfs_devno_to_devpath(dev_t devno, বর্ণ *buf, size_t bufsiz)
{
	সংরচনা path_cxt *pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	বর্ণ *res = রিক্ত;

	যদি (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	প্রত্যাগম res;
}

বর্ণ *sysfs_devno_to_devname(dev_t devno, বর্ণ *buf, size_t bufsiz)
{
	সংরচনা path_cxt *pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	বর্ণ *res = রিক্ত;

	যদি (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	প্রত্যাগম res;
}

পূর্ণাঙ্ক sysfs_devno_গুনতি_partitions(dev_t devno)
{
	সংরচনা path_cxt *pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	পূর্ণাঙ্ক n = 0;

	যদি (pc) {
		বর্ণ buf[PATH_MAX + 1];
		বর্ণ *name = sysfs_blkdev_get_name(pc, buf, পরিমাণ(buf));

		n = sysfs_blkdev_গুনতি_partitions(pc, name);
		ul_unref_path(pc);
	}
	প্রত্যাগম n;
}

বর্ণ *sysfs_chrdev_devno_to_devname(dev_t devno, বর্ণ *buf, size_t bufsiz)
{
	বর্ণ link[PATH_MAX];
	সংরচনা path_cxt *pc;
	বর্ণ *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	যদি (!pc)
		প্রত্যাগম রিক্ত;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, পরিমাণ(link), রিক্ত);
	ul_unref_path(pc);

	যদি (sz < 0)
		প্রত্যাগম রিক্ত;

	name = strrchr(link, '/');
	যদি (!name)
		প্রত্যাগম রিক্ত;

	name++;
	sz = strlen(name);
	যদি ((size_t) sz + 1 > bufsiz)
		প্রত্যাগম রিক্ত;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	প্রত্যাগম buf;

}

গণনা sysfs_byteorder sysfs_get_byteorder(সংরচনা path_cxt *pc)
{
	পূর্ণাঙ্ক rc;
	বর্ণ buf[BUFSIZ];
	গণনা sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, পরিমাণ(buf), _PATH_SYS_CPU_BYTEORDER);
	যদি (rc < 0)
		গচ্ছ unknown;

	যদি (strncmp(buf, "little", পরিমাণ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		গচ্ছ out;
	} অন্যথা যদি (strncmp(buf, "big", পরিমাণ(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		গচ্ছ out;
	}

unknown:
#যদি __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#নোচেদ্যদি __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#অন্যথা
#error Unknown byte order
#endif

out:
	প্রত্যাগম ret;
}

পূর্ণাঙ্ক sysfs_get_address_bits(সংরচনা path_cxt *pc)
{
	পূর্ণাঙ্ক rc;
	পূর্ণাঙ্ক address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	যদি (rc < 0)
		প্রত্যাগম rc;
	যদি (address_bits < 0)
		প্রত্যাগম -EINVAL;
	প্রত্যাগম address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdlib.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	সংরচনা path_cxt *pc;
	বর্ণ *devname;
	dev_t devno, disk_devno;
	বর্ণ path[PATH_MAX], *sub, *chain;
	বর্ণ diskname[32];
	পূর্ণাঙ্ক i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	যদি (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	যদি (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণাঙ্ক) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, পরিমাণ(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, পরিমাণ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, পরিমাণ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণাঙ্ক) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, রিক্ত, রিক্ত);
	যদি (!pc)
		গচ্ছ done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned পূর্ণাঙ্ক) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, পরিমাণ(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, পরিমাণ(path)));

	sysfs_devno_to_wholedisk(devno, diskname, পরিমাণ(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned পূর্ণাঙ্ক) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	যদি (is_part && disk_devno) {
		সংরচনা path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, রিক্ত, রিক্ত);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_গুনতি_dirents(pc, "slaves"));

	যদি (!is_part) {
		printf("First 5 partitions:\n");
		প্রতি (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			যদি (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	যদি (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	অন্যথা
		printf(" SIZE: %"PRIu64"\n", u64);

	যদি (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	অন্যথা
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, পরিমাণ(path));
	printf(" SUBSYSTEMS:\n");

	যাবত্ (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		মুক্ত(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	প্রত্যাগম rc;
}
#endif /* TEST_PROGRAM_SYSFS */
