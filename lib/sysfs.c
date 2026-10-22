/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#ضمن <ctype.h>
#ضمن <inttypes.h>
#ضمن <libgen.h>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "pathnames.h"
#ضمن "sysfs.h"
#ضمن "fileutils.h"
#ضمن "all-io.h"
#ضمن "debug.h"
#ضمن "strutils.h"
#ضمن "buffer.h"

ساكن فراغ sysfs_blkdev_deinit_لار(بنية لار_cxt *pc);
ساكن صحيح  sysfs_blkdev_enoent_redirect(بنية لار_cxt *pc, ثابت حرف *لار, صحيح *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_ماسک(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_تشه_MASKNAMES;

#تعریف_کړه ULSYSFS_DEBUG_INIT	(1 << 1)
#تعریف_کړه ULSYSFS_DEBUG_CXT	(1 << 2)

#تعریف_کړه DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#تعریف_کړه DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#تعریف_کړه ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

فراغ ul_sysfs_init_debug(فراغ)
{
	إذا (ulsysfs_debug_ماسک)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

بنية لار_cxt *ul_new_sysfs_لار(dev_t devno, بنية لار_cxt *پلار, ثابت حرف *مختاړی)
{
	بنية لار_cxt *pc = ul_new_لار(عدم);

	إذا (!pc)
		أرجع عدم;
	إذا (مختاړی)
		ul_لار_سېټ_مختاړی(pc, مختاړی);

	إذا (sysfs_blkdev_init_لار(pc, devno, پلار) != 0) {
		ul_unref_لار(pc);
		أرجع عدم;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	أرجع pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحيح sysfs_blkdev_init_لار(بنية لار_cxt *pc, dev_t devno, بنية لار_cxt *پلار)
{
	بنية sysfs_blkdev *blk;
	صحيح rc;
	حرف buf[حجم(لار_SYS_DEVBLOCK)
		 + حجم(stringify_ارزښت(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, حجم(buf), لار_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_لار_سېټ_dir(pc, buf);
	إذا (rc)
		أرجع rc;

	/* make sure path exists */
	rc = ul_لار_get_dirfd(pc);
	إذا (rc < 0)
		أرجع rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_لار_get_dialect(pc);
	إذا (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = حافظه_صفر_ورکړه(1, حجم(بنية sysfs_blkdev));
		إذا (!blk)
			أرجع -ENOMEM;

		ul_لار_سېټ_dialect(pc, blk, sysfs_blkdev_deinit_لار);
		ul_لار_سېټ_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_سېټ_پلار(pc, پلار);

	أرجع 0;
}

ساكن فراغ sysfs_blkdev_deinit_لار(بنية لار_cxt *pc)
{
	بنية sysfs_blkdev *blk;

	إذا (!pc)
		أرجع;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_لار_get_dialect(pc);
	إذا (!blk)
		أرجع;

	ul_unref_لار(blk->پلار);
	حرر(blk);

	ul_لار_سېټ_dialect(pc, عدم, عدم);
}

صحيح sysfs_blkdev_سېټ_پلار(بنية لار_cxt *pc, بنية لار_cxt *پلار)
{
	بنية sysfs_blkdev *blk = ul_لار_get_dialect(pc);

	إذا (!pc || !blk)
		أرجع -EINVAL;

	إذا (blk->پلار) {
		ul_unref_لار(blk->پلار);
		blk->پلار = عدم;
	}

	إذا (پلار) {
		ul_ref_لار(پلار);
		blk->پلار = پلار;
	} وإلا
		blk->پلار = عدم;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	أرجع 0;
}

بنية لار_cxt *sysfs_blkdev_get_پلار(بنية لار_cxt *pc)
{
	بنية sysfs_blkdev *blk = ul_لار_get_dialect(pc);
	أرجع blk ? blk->پلار : عدم;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ساكن صحيح sysfs_blkdev_enoent_redirect(بنية لار_cxt *pc, ثابت حرف *لار, صحيح *dirfd)
{
	بنية sysfs_blkdev *blk = ul_لار_get_dialect(pc);

	إذا (blk && blk->پلار && لار) {
		*dirfd = ul_لار_get_dirfd(blk->پلار);
		إذا (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", لار));
			أرجع 0;
		}
	}
	أرجع 1;	/* no redirect */
}

حرف *sysfs_blkdev_get_name(بنية لار_cxt *pc, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	حرف *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_لار_readlink(pc, link, حجم(link), عدم);
	إذا (sz < 0)
		أرجع عدم;

	name = strrchr(link, '/');
	إذا (!name)
		أرجع عدم;

	name++;
	sz = مزي_اوږدوالی(name);
	إذا ((size_t) sz + 1 > bufsiz)
		أرجع عدم;

	حافظه_نقل(buf, name, sz + 1);
	sysfs_devname_sys_منزل_dev(buf);
	أرجع buf;
}

صحيح sysfs_blkdev_is_برخه_کول_dirent(DIR *dir, بنية dirent *d, ثابت حرف *پلار_name)
{
	حرف لار[NAME_MAX + 6 + 1];

#که_تعریف_شوی _DIRENT_HAVE_D_TYPE
	إذا (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		أرجع 0;
#د_که_پای
	size_t len = 0;

	إذا (پلار_name) {
		ثابت حرف *p = پلار_name;

		/* /dev/sda --> "sda" */
		إذا (*پلار_name == '/') {
			p = strrchr(پلار_name, '/');
			إذا (!p)
				أرجع 0;
			p++;
		}

		len = مزي_اوږدوالی(p);
		إذا ((مزي_اوږدوالی(d->d_name) <= len) || (مزي_پرتله(p, d->d_name, len) != 0))
			len = 0;
	}

	إذا (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		أرجع ((*(d->d_name + len) == 'p' && که_عدد(*(d->d_name + len + 1)))
			|| که_عدد(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(لار, حجم(لار), "%s/start", d->d_name);

	أرجع faccessat(dirfd(dir), لار, R_OK, 0) == 0;
}

صحيح sysfs_blkdev_شمېر_partitions(بنية لار_cxt *pc, ثابت حرف *devname)
{
	DIR *dir;
	بنية dirent *d;
	صحيح r = 0;

	dir = ul_لار_opendir(pc, عدم);
	إذا (!dir)
		أرجع 0;

	طالما ((d = xreaddir(dir))) {
		إذا (sysfs_blkdev_is_برخه_کول_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	أرجع r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_منزل_devno(بنية لار_cxt *pc, صحيح partno)
{
	DIR *dir;
	بنية dirent *d;
	dev_t devno = 0;

	dir = ul_لار_opendir(pc, عدم);
	إذا (!dir)
		أرجع 0;

	طالما ((d = xreaddir(dir))) {
		صحيح n;

		إذا (!sysfs_blkdev_is_برخه_کول_dirent(dir, d, عدم))
			استمر;

		إذا (ul_لار_readf_s32(pc, &n, "%s/partition", d->d_name))
			استمر;

		إذا (n == partno) {
			إذا (ul_لار_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				اكسر;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (صحيح) partno, (صحيح) devno));
	أرجع devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
حرف *sysfs_blkdev_get_slave(بنية لار_cxt *pc)
{
	DIR *dir;
	بنية dirent *d;
	حرف *name = عدم;

	dir = ul_لار_opendir(pc, "slaves");
	إذا (!dir)
		أرجع عدم;

	طالما ((d = xreaddir(dir))) {
		إذا (name)
			اذهب err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	أرجع name;
err:
	حرر(name);
	closedir(dir);
	أرجع عدم;
}


#تعریف_کړه SUBSYSTEM_LINKNAME	"/subsystem"

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
ساكن حرف *get_subsystem(حرف *chain, حرف *buf, size_t bufsz)
{
	size_t len;
	حرف *p;

	إذا (!chain || !*chain)
		أرجع عدم;

	len = مزي_اوږدوالی(chain);
	إذا (len + حجم(SUBSYSTEM_LINKNAME) > PATH_MAX)
		أرجع عدم;

	افعل {
		ssize_t sz;

		/* append "/subsystem" to the path */
		حافظه_نقل(chain + len, SUBSYSTEM_LINKNAME, حجم(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		إذا (p) {
			*p = '\0';
			len = p - chain;
		}

		إذا (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			أرجع basename(buf);
		}

	} طالما (p);

	أرجع عدم;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
حرف *sysfs_blkdev_get_devchain(بنية لار_cxt *pc, حرف *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	بنية ul_بفر tmp = UL_INIT_بفر;
	ثابت حرف *p;
	حرف *res = عدم;

	ssz = ul_لار_readlink(pc, buf, bufsz, عدم);
	إذا (ssz <= 0)
		أرجع عدم;

	إذا ((p = ul_لار_get_مختاړی(pc)))
		ul_بفر_append_string(&tmp, p);

	ul_بفر_append_string(&tmp, لار_SYS_DEVBLOCK "/");
	ul_بفر_append_معلومات(&tmp, buf, ssz);

	p = ul_بفر_get_string(&tmp, &sz, عدم);
	إذا (p && sz <= bufsz) {
		حافظه_نقل(buf, p, sz);
		res = buf;
	}
	ul_بفر_free_معلومات(&tmp);
	أرجع res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
صحيح sysfs_blkdev_بل_subsystem(بنية لار_cxt *pc __attribute__((ناکارول_شوی)),
			 حرف *devchain, حرف **subsys)
{
	حرف subbuf[PATH_MAX];
	حرف *sub;

	إذا (!subsys || !devchain)
		أرجع -EINVAL;

	*subsys = عدم;

	طالما ((sub = get_subsystem(devchain, subbuf, حجم(subbuf)))) {
		*subsys = strdup(sub);
		إذا (!*subsys)
			أرجع -ENOMEM;
		أرجع 0;
	}

	أرجع 1;
}

#تعریف_کړه REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ساكن صحيح sysfs_devchain_is_removable(حرف *chain)
{
	size_t len;
	حرف buf[20];
	حرف *p;

	إذا (!chain || !*chain)
		أرجع 0;

	len = مزي_اوږدوالی(chain);
	إذا (len + حجم(REMOVABLE_FILENAME) > PATH_MAX)
		أرجع 0;

	افعل {
		صحيح fd, rc;

		/* append "/removable" to the path */
		حافظه_نقل(chain + len, REMOVABLE_FILENAME, حجم(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		إذا (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			اكسر;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		إذا (fd != -1) {
			rc = ul_read_all(fd, buf, حجم(buf));
			close(fd);

			إذا (rc > 0) {
				إذا (مزي_پرتله(buf, "fixed", min(rc, 5)) == 0) {
					أرجع 0;
				} وإلا إذا (مزي_پرتله(buf, "removable", min(rc, 9)) == 0) {
					أرجع 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		إذا (p) {
			*p = '\0';
			len = p - chain;
		}

	} طالما (p);

	أرجع 0;
}

صحيح sysfs_blkdev_is_hotpluggable(بنية لار_cxt *pc)
{
	حرف buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, حجم(buf));
	أرجع sysfs_devchain_is_removable(chain);
}

صحيح sysfs_blkdev_is_removable(بنية لار_cxt *pc)
{
	صحيح rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	إذا (ul_لار_read_s32(pc, &rc, "removable") == 0)
		أرجع rc;

	أرجع 0;
}

ساكن صحيح get_dm_wholedisk(بنية لار_cxt *pc, حرف *diskname,
                            size_t len, dev_t *diskdevno)
{
	صحيح rc = 0;
	حرف *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	إذا (!name)
		أرجع -1;

	إذا (diskname && len)
		xstrncpy(diskname, name, len);

	إذا (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_لار_get_مختاړی(pc), name, عدم);
		إذا (!*diskdevno)
			rc = -1;
	}

	حرر(name);
	أرجع rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
صحيح sysfs_blkdev_get_wholedisk(	بنية لار_cxt *pc,
				حرف *diskname,
				size_t len,
				dev_t *diskdevno)
{
	صحيح is_part = 0;

	إذا (!pc)
		أرجع -1;

	is_part = ul_لار_access(pc, F_OK, "partition") == 0;
	إذا (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		حرف *uuid = عدم, *tmp, *مختاړی;

		ul_لار_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		مختاړی = uuid ? strsep(&tmp, "-") : عدم;

		إذا (مختاړی && c_strncasecmp(مختاړی, "part", 4) == 0)
			is_part = 1;
		حرر(uuid);

		إذا (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			اذهب بشپړ;

		is_part = 0;
	}

	إذا (!is_part) {
		/*
		 * unpartitioned device
		 */
		إذا (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			اذهب err;
		إذا (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} وإلا {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		حرف linkpath[PATH_MAX];
		حرف *name;
		ssize_t	linklen;

		linklen = ul_لار_readlink(pc, linkpath, حجم(linkpath), عدم);
		إذا (linklen < 0)
			اذهب err;

		stripoff_وروستی_component(linkpath);	  /* dirname */
		name = stripoff_وروستی_component(linkpath);   /* basename */
		إذا (!name)
			اذهب err;

		sysfs_devname_sys_منزل_dev(name);
		إذا (diskname && len)
			xstrncpy(diskname, name, len);

		إذا (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_لار_get_مختاړی(pc), name, عدم);
			إذا (!*diskdevno)
				اذهب err;
		}
	}

بشپړ:
	أرجع 0;
err:
	أرجع -1;
}

صحيح sysfs_devno_منزل_wholedisk(dev_t devno, حرف *diskname,
		             size_t len, dev_t *diskdevno)
{
	بنية لار_cxt *pc;
	صحيح rc = 0;

	إذا (!devno)
		أرجع -EINVAL;
	pc = ul_new_sysfs_لار(devno, عدم, عدم);
	إذا (!pc)
		أرجع -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_لار(pc);
	أرجع rc;
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
صحيح sysfs_devno_is_dm_hidden(dev_t devno, حرف **uuid,
			     ثابت بنية ul_vfs_ops *vfs)
{
	بنية لار_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_لار(devno, عدم, عدم);
	إذا (!pc)
		اذهب بشپړ;
	ul_لار_refer_vfs(pc, vfs);
	إذا (ul_لار_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب بشپړ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	إذا (مزي_پرتله(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		إذا (p && *(p + 1))
			rc = 1;
	}
بشپړ:
	ul_unref_لار(pc);
	إذا (uuid)
		*uuid = id;
	وإلا
		حرر(id);
	أرجع rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
صحيح sysfs_devno_is_dm_شخصي(dev_t devno, حرف **uuid,
			      ثابت بنية ul_vfs_ops *vfs)
{
	بنية لار_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_لار(devno, عدم, عدم);
	إذا (!pc)
		اذهب بشپړ;
	ul_لار_refer_vfs(pc, vfs);
	إذا (ul_لار_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب بشپړ;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	إذا (مزي_پرتله(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		إذا (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} وإلا إذا (مزي_پرتله(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
بشپړ:
	ul_unref_لار(pc);
	إذا (uuid)
		*uuid = id;
	وإلا
		حرر(id);
	أرجع rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
صحيح sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	إذا (sysfs_devno_منزل_wholedisk(devno, عدم, 0, &disk) != 0)
		أرجع -1;

	أرجع devno == disk;
}


صحيح sysfs_blkdev_scsi_get_hctl(بنية لار_cxt *pc, صحيح *h, صحيح *c, صحيح *t, صحيح *l)
{
	حرف buf[PATH_MAX], *hctl;
	بنية sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_لار_get_dialect(pc);

	إذا (!blk || blk->hctl_error)
		أرجع -EINVAL;
	إذا (blk->has_hctl)
		اذهب بشپړ;

	blk->hctl_error = 1;
	len = ul_لار_readlink(pc, buf, حجم(buf), "device");
	إذا (len < 0)
		أرجع len;

	hctl = strrchr(buf, '/');
	إذا (!hctl)
		أرجع -1;
	hctl++;

	إذا (له_مزي_ولوله(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_موخه, &blk->scsi_lun) != 4)
		أرجع -1;

	blk->has_hctl = 1;
بشپړ:
	إذا (h)
		*h = blk->scsi_host;
	إذا (c)
		*c = blk->scsi_channel;
	إذا (t)
		*t = blk->scsi_موخه;
	إذا (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	أرجع 0;
}


ساكن حرف *scsi_host_attribute_لار(
			بنية لار_cxt *pc,
			ثابت حرف *type,
			حرف *buf,
			size_t bufsz,
			ثابت حرف *attr)
{
	صحيح len;
	صحيح host;
	ثابت حرف *مختاړی;

	إذا (sysfs_blkdev_scsi_get_hctl(pc, &host, عدم, عدم, عدم))
		أرجع عدم;

	مختاړی = ul_لار_get_مختاړی(pc);
	إذا (!مختاړی)
		مختاړی = "";

	إذا (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				مختاړی, لار_SYS_CLASS, type, host, attr);
	وإلا
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				مختاړی, لار_SYS_CLASS, type, host);

	أرجع (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

حرف *sysfs_blkdev_scsi_host_strdup_attribute(بنية لار_cxt *pc,
			ثابت حرف *type, ثابت حرف *attr)
{
	حرف buf[1024];
	صحيح rc;
	FILE *f;

	إذا (!attr || !type ||
	    !scsi_host_attribute_لار(pc, type, buf, حجم(buf), attr))
		أرجع عدم;

	إذا (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		أرجع عدم;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	أرجع rc == 1 ? strdup(buf) : عدم;
}

صحيح sysfs_blkdev_scsi_host_is(بنية لار_cxt *pc, ثابت حرف *type)
{
	حرف buf[PATH_MAX];
	بنية stat st;

	إذا (!type || !scsi_host_attribute_لار(pc, type,
				buf, حجم(buf), عدم))
		أرجع 0;

	أرجع stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ساكن حرف *scsi_attribute_لار(بنية لار_cxt *pc,
		حرف *buf, size_t bufsz, ثابت حرف *attr)
{
	صحيح len, h, c, t, l;
	ثابت حرف *مختاړی;

	إذا (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		أرجع عدم;

	مختاړی = ul_لار_get_مختاړی(pc);
	إذا (!مختاړی)
		مختاړی = "";

	إذا (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				مختاړی, لار_SYS_SCSI,
				h,c,t,l, attr);
	وإلا
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				مختاړی, لار_SYS_SCSI,
				h,c,t,l);
	أرجع (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

صحيح sysfs_blkdev_scsi_has_attribute(بنية لار_cxt *pc, ثابت حرف *attr)
{
	حرف لار[PATH_MAX];
	بنية stat st;

	إذا (!scsi_attribute_لار(pc, لار, حجم(لار), attr))
		أرجع 0;

	أرجع stat(لار, &st) == 0;
}

صحيح sysfs_blkdev_scsi_لار_لري(بنية لار_cxt *pc, ثابت حرف *نمونه)
{
	حرف لار[PATH_MAX], linkc[PATH_MAX];
	بنية stat st;
	ssize_t len;

	إذا (!scsi_attribute_لار(pc, لار, حجم(لار), عدم))
		أرجع 0;

	إذا (stat(لار, &st) != 0)
		أرجع 0;

	len = readlink(لار, linkc, حجم(linkc) - 1);
	إذا (len < 0)
		أرجع 0;

	linkc[len] = '\0';
	أرجع مزي_پلټنه(linkc, نمونه) != عدم;
}

ساكن dev_t read_devno(ثابت حرف *لار)
{
	FILE *f;
	صحيح maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(لار, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع 0;

	إذا (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	أرجع dev;
}

صحيح sysfs_devname_is_hidden(ثابت حرف *مختاړی, ثابت حرف *name)
{
	حرف buf[PATH_MAX];
	صحيح rc = 0, hidden = 0, len;
	FILE *f;

	إذا (مزي_پرتله("/dev/", name, 5) == 0)
		أرجع 0;

	إذا (!مختاړی)
		مختاړی = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, حجم(buf),
			"%s" لار_SYS_بلاک "/%s/hidden",
			مختاړی, name);

	إذا (len < 0 || (size_t) len + 1 > حجم(buf))
		أرجع 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	أرجع rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ثابت حرف *مختاړی, ثابت حرف *name, ثابت حرف *پلار)
{
	حرف buf[PATH_MAX];
	حرف *_name = عدم, *پلار_2 = عدم;	/* name as encoded in sysfs */
	dev_t dev = 0;
	صحيح len;

	إذا (!مختاړی)
		مختاړی = "";

	أكد(name);

	إذا (مزي_پرتله("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		بنية stat st;

		إذا (stat(name, &st) == 0) {
			dev = st.st_rdev;
			اذهب بشپړ;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	إذا (!_name)
		اذهب بشپړ;
	sysfs_devname_dev_منزل_sys(_name);

	إذا (پلار) {
		پلار_2 = strdup(پلار);
		إذا (!پلار_2)
			اذهب بشپړ;
	}

	إذا (پلار && مزي_پرتله("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_منزل_sys(پلار_2);
		len = snprintf(buf, حجم(buf),
				"%s" لار_SYS_بلاک "/%s/%s/dev",
				مختاړی,	پلار_2, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب بشپړ;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		اذهب بشپړ;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, حجم(buf),
			"%s" لار_SYS_بلاک "/%s/dev",
			مختاړی, _name);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		اذهب بشپړ;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	إذا (!dev && پلار && ul_startswith(name, پلار)) {
		len = snprintf(buf, حجم(buf),
				"%s" لار_SYS_بلاک "/%s/%s/dev",
				مختاړی, پلار_2, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب بشپړ;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	إذا (!dev) {
		len = snprintf(buf, حجم(buf),
				"%s" لار_SYS_بلاک "/%s/device/dev",
				مختاړی, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب بشپړ;
		dev = read_devno(buf);
	}
بشپړ:
	حرر(_name);
	حرر(پلار_2);
	أرجع dev;
}

dev_t sysfs_devname_منزل_devno(ثابت حرف *name)
{
	أرجع __sysfs_devname_to_devno(عدم, name, عدم);
}

حرف *sysfs_blkdev_get_لار(بنية لار_cxt *pc, حرف *buf, size_t bufsiz)
{
	ثابت حرف *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	حرف *res = عدم;
	size_t sz;
	بنية stat st;

	إذا (!name)
		اذهب بشپړ;

	sz = مزي_اوږدوالی(name);
	إذا (sz + حجم("/dev/") > bufsiz)
		اذهب بشپړ;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	حافظه_نقل(buf, "/dev/", 5);

	إذا (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
بشپړ:
	أرجع res;
}

dev_t sysfs_blkdev_get_devno(بنية لار_cxt *pc)
{
	أرجع ((بنية sysfs_blkdev *) ul_لار_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
حرف *sysfs_devno_منزل_devpath(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية لار_cxt *pc = ul_new_sysfs_لار(devno, عدم, عدم);
	حرف *res = عدم;

	إذا (pc) {
		res = sysfs_blkdev_get_لار(pc, buf, bufsiz);
		ul_unref_لار(pc);
	}
	أرجع res;
}

حرف *sysfs_devno_منزل_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية لار_cxt *pc = ul_new_sysfs_لار(devno, عدم, عدم);
	حرف *res = عدم;

	إذا (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_لار(pc);
	}
	أرجع res;
}

صحيح sysfs_devno_شمېر_partitions(dev_t devno)
{
	بنية لار_cxt *pc = ul_new_sysfs_لار(devno, عدم, عدم);
	صحيح n = 0;

	إذا (pc) {
		حرف buf[PATH_MAX + 1];
		حرف *name = sysfs_blkdev_get_name(pc, buf, حجم(buf));

		n = sysfs_blkdev_شمېر_partitions(pc, name);
		ul_unref_لار(pc);
	}
	أرجع n;
}

حرف *sysfs_chrdev_devno_منزل_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	بنية لار_cxt *pc;
	حرف *name;
	ssize_t	sz;

	pc = ul_new_لار(لار_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	إذا (!pc)
		أرجع عدم;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_لار_readlink(pc, link, حجم(link), عدم);
	ul_unref_لار(pc);

	إذا (sz < 0)
		أرجع عدم;

	name = strrchr(link, '/');
	إذا (!name)
		أرجع عدم;

	name++;
	sz = مزي_اوږدوالی(name);
	إذا ((size_t) sz + 1 > bufsiz)
		أرجع عدم;

	حافظه_نقل(buf, name, sz + 1);
	sysfs_devname_sys_منزل_dev(buf);
	أرجع buf;

}

تعداد sysfs_byteorder sysfs_get_byteorder(بنية لار_cxt *pc)
{
	صحيح rc;
	حرف buf[BUFSIZ];
	تعداد sysfs_byteorder ret;

	rc = ul_لار_read_بفر(pc, buf, حجم(buf), لار_SYS_CPU_BYTEORDER);
	إذا (rc < 0)
		اذهب unknown;

	إذا (مزي_پرتله(buf, "little", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		اذهب بهر;
	} وإلا إذا (مزي_پرتله(buf, "big", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		اذهب بهر;
	}

unknown:
#إذا __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#وإلاإذا __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#وإلا
#تېروتنه Unknown byte ترتیب
#د_که_پای

بهر:
	أرجع ret;
}

صحيح sysfs_get_پته_بټونه(بنية لار_cxt *pc)
{
	صحيح rc;
	صحيح پته_بټونه;

	rc = ul_لار_scanf(pc, لار_SYS_پته_بټونه, "%d", &پته_بټونه);
	إذا (rc < 0)
		أرجع rc;
	إذا (پته_بټونه < 0)
		أرجع -EINVAL;
	أرجع پته_بټونه;
}


#که_تعریف_شوی TEST_پروګرام_SYSFS
#ضمن <errno.h>
#ضمن <stdlib.h>

صحيح اصلي(صحيح argc, حرف *argv[])
{
	بنية لار_cxt *pc;
	حرف *devname;
	dev_t devno, disk_devno;
	حرف لار[PATH_MAX], *sub, *chain;
	حرف diskname[32];
	صحيح i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	إذا (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_منزل_devno(devname);

	إذا (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	چاپ_کړه("non-context:\n");
	چاپ_کړه(" DEVNO:   %u (%u:%u)\n", (بینښه صحيح) devno, major(devno), minor(devno));
	چاپ_کړه(" DEVNAME: %s\n", sysfs_devno_منزل_devname(devno, لار, حجم(لار)));
	چاپ_کړه(" DEVPATH: %s\n", sysfs_devno_منزل_devpath(devno, لار, حجم(لار)));

	sysfs_devno_منزل_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	چاپ_کړه(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (بینښه صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	چاپ_کړه(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_لار(devno, عدم, عدم);
	إذا (!pc)
		اذهب بشپړ;

	چاپ_کړه("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	چاپ_کړه(" DEVNO:   %u (%u:%u)\n", (بینښه صحيح) devno, major(devno), minor(devno));
	چاپ_کړه(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, لار, حجم(لار)));
	چاپ_کړه(" DEVPATH: %s\n", sysfs_blkdev_get_لار(pc, لار, حجم(لار)));

	sysfs_devno_منزل_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	چاپ_کړه(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (بینښه صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	چاپ_کړه(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_لار_access(pc, F_OK, "partition") == 0;
	چاپ_کړه(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	إذا (is_part && disk_devno) {
		بنية لار_cxt *disk_pc =  ul_new_sysfs_لار(disk_devno, عدم, عدم);
		sysfs_blkdev_سېټ_پلار(pc, disk_pc);

		ul_unref_لار(disk_pc);
	}

	چاپ_کړه(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	چاپ_کړه(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	چاپ_کړه(" SLAVES: %d\n", ul_لار_شمېر_dirents(pc, "slaves"));

	إذا (!is_part) {
		چاپ_کړه("First 5 partitions:\n");
		لكل (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_منزل_devno(pc, i);
			إذا (dev)
				چاپ_کړه("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	إذا (ul_لار_read_u64(pc, &u64, "size") != 0)
		چاپ_کړه(" (!) read SIZE failed\n");
	وإلا
		چاپ_کړه(" SIZE: %"PRIu64"\n", u64);

	إذا (ul_لار_read_s32(pc, &i, "queue/hw_sector_size"))
		چاپ_کړه(" (!) read SECTOR failed\n");
	وإلا
		چاپ_کړه(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, لار, حجم(لار));
	چاپ_کړه(" SUBSYSTEMS:\n");

	طالما (chain && sysfs_blkdev_بل_subsystem(pc, chain, &sub) == 0) {
		چاپ_کړه("\t%s\n", sub);
		حرر(sub);
	}

	rc = EXIT_SUCCESS;
بشپړ:
	ul_unref_لار(pc);
	أرجع rc;
}
#د_که_پای /* TEST_PROGRAM_SYSFS */
