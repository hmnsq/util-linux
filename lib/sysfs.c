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

ساكن فراغ sysfs_blkdev_deinit_رستو(بنية رستو_cxt *pc);
ساكن صحيح  sysfs_blkdev_enoent_redirect(بنية رستو_cxt *pc, ثابت حرف *رستو, صحيح *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_نقاب(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_خالي_جايون_MASKNAMES;

#وصف_ڪريو ULSYSFS_DEBUG_INIT	(1 << 1)
#وصف_ڪريو ULSYSFS_DEBUG_CXT	(1 << 2)

#وصف_ڪريو DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#وصف_ڪريو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#وصف_ڪريو ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

فراغ ul_sysfs_init_debug(فراغ)
{
	إذا (ulsysfs_debug_نقاب)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

بنية رستو_cxt *ul_new_sysfs_رستو(dev_t devno, بنية رستو_cxt *والد, ثابت حرف *اڳياڙي)
{
	بنية رستو_cxt *pc = ul_new_رستو(عدم);

	إذا (!pc)
		أرجع عدم;
	إذا (اڳياڙي)
		ul_رستو_سيٽ_اڳياڙي(pc, اڳياڙي);

	إذا (sysfs_blkdev_init_رستو(pc, devno, والد) != 0) {
		ul_unref_رستو(pc);
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
صحيح sysfs_blkdev_init_رستو(بنية رستو_cxt *pc, dev_t devno, بنية رستو_cxt *والد)
{
	بنية sysfs_blkdev *blk;
	صحيح rc;
	حرف buf[حجم(رستو_SYS_DEVBLOCK)
		 + حجم(stringify_قدر(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, حجم(buf), رستو_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_رستو_سيٽ_dir(pc, buf);
	إذا (rc)
		أرجع rc;

	/* make sure path exists */
	rc = ul_رستو_get_dirfd(pc);
	إذا (rc < 0)
		أرجع rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_رستو_get_dialect(pc);
	إذا (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = صفر_يادگيري_ڏيو(1, حجم(بنية sysfs_blkdev));
		إذا (!blk)
			أرجع -ENOMEM;

		ul_رستو_سيٽ_dialect(pc, blk, sysfs_blkdev_deinit_رستو);
		ul_رستو_سيٽ_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_سيٽ_والد(pc, والد);

	أرجع 0;
}

ساكن فراغ sysfs_blkdev_deinit_رستو(بنية رستو_cxt *pc)
{
	بنية sysfs_blkdev *blk;

	إذا (!pc)
		أرجع;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_رستو_get_dialect(pc);
	إذا (!blk)
		أرجع;

	ul_unref_رستو(blk->والد);
	حرر(blk);

	ul_رستو_سيٽ_dialect(pc, عدم, عدم);
}

صحيح sysfs_blkdev_سيٽ_والد(بنية رستو_cxt *pc, بنية رستو_cxt *والد)
{
	بنية sysfs_blkdev *blk = ul_رستو_get_dialect(pc);

	إذا (!pc || !blk)
		أرجع -EINVAL;

	إذا (blk->والد) {
		ul_unref_رستو(blk->والد);
		blk->والد = عدم;
	}

	إذا (والد) {
		ul_ref_رستو(والد);
		blk->والد = والد;
	} وإلا
		blk->والد = عدم;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	أرجع 0;
}

بنية رستو_cxt *sysfs_blkdev_get_والد(بنية رستو_cxt *pc)
{
	بنية sysfs_blkdev *blk = ul_رستو_get_dialect(pc);
	أرجع blk ? blk->والد : عدم;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ساكن صحيح sysfs_blkdev_enoent_redirect(بنية رستو_cxt *pc, ثابت حرف *رستو, صحيح *dirfd)
{
	بنية sysfs_blkdev *blk = ul_رستو_get_dialect(pc);

	إذا (blk && blk->والد && رستو) {
		*dirfd = ul_رستو_get_dirfd(blk->والد);
		إذا (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", رستو));
			أرجع 0;
		}
	}
	أرجع 1;	/* no redirect */
}

حرف *sysfs_blkdev_get_name(بنية رستو_cxt *pc, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	حرف *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_رستو_readlink(pc, link, حجم(link), عدم);
	إذا (sz < 0)
		أرجع عدم;

	name = strrchr(link, '/');
	إذا (!name)
		أرجع عدم;

	name++;
	sz = لڙي_ڊيگهه(name);
	إذا ((size_t) sz + 1 > bufsiz)
		أرجع عدم;

	يادگيري_نقل(buf, name, sz + 1);
	sysfs_devname_sys_منزل_dev(buf);
	أرجع buf;
}

صحيح sysfs_blkdev_is_حصا_ڪريو_dirent(DIR *dir, بنية dirent *d, ثابت حرف *والد_name)
{
	حرف رستو[NAME_MAX + 6 + 1];

#جيڪڏهن_وصف_ٿيل _DIRENT_HAVE_D_TYPE
	إذا (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		أرجع 0;
#جيڪڏهن_پورو
	size_t len = 0;

	إذا (والد_name) {
		ثابت حرف *p = والد_name;

		/* /dev/sda --> "sda" */
		إذا (*والد_name == '/') {
			p = strrchr(والد_name, '/');
			إذا (!p)
				أرجع 0;
			p++;
		}

		len = لڙي_ڊيگهه(p);
		إذا ((لڙي_ڊيگهه(d->d_name) <= len) || (لڙي_ڀيٽ(p, d->d_name, len) != 0))
			len = 0;
	}

	إذا (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		أرجع ((*(d->d_name + len) == 'p' && انگ_آهي(*(d->d_name + len + 1)))
			|| انگ_آهي(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(رستو, حجم(رستو), "%s/start", d->d_name);

	أرجع faccessat(dirfd(dir), رستو, R_OK, 0) == 0;
}

صحيح sysfs_blkdev_ڳڻپ_partitions(بنية رستو_cxt *pc, ثابت حرف *devname)
{
	DIR *dir;
	بنية dirent *d;
	صحيح r = 0;

	dir = ul_رستو_opendir(pc, عدم);
	إذا (!dir)
		أرجع 0;

	طالما ((d = xreaddir(dir))) {
		إذا (sysfs_blkdev_is_حصا_ڪريو_dirent(dir, d, devname))
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
dev_t sysfs_blkdev_partno_منزل_devno(بنية رستو_cxt *pc, صحيح partno)
{
	DIR *dir;
	بنية dirent *d;
	dev_t devno = 0;

	dir = ul_رستو_opendir(pc, عدم);
	إذا (!dir)
		أرجع 0;

	طالما ((d = xreaddir(dir))) {
		صحيح n;

		إذا (!sysfs_blkdev_is_حصا_ڪريو_dirent(dir, d, عدم))
			استمر;

		إذا (ul_رستو_readf_s32(pc, &n, "%s/partition", d->d_name))
			استمر;

		إذا (n == partno) {
			إذا (ul_رستو_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
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
حرف *sysfs_blkdev_get_slave(بنية رستو_cxt *pc)
{
	DIR *dir;
	بنية dirent *d;
	حرف *name = عدم;

	dir = ul_رستو_opendir(pc, "slaves");
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


#وصف_ڪريو SUBSYSTEM_LINKNAME	"/subsystem"

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

	len = لڙي_ڊيگهه(chain);
	إذا (len + حجم(SUBSYSTEM_LINKNAME) > PATH_MAX)
		أرجع عدم;

	افعل {
		ssize_t sz;

		/* append "/subsystem" to the path */
		يادگيري_نقل(chain + len, SUBSYSTEM_LINKNAME, حجم(SUBSYSTEM_LINKNAME));

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
حرف *sysfs_blkdev_get_devchain(بنية رستو_cxt *pc, حرف *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	بنية ul_بفر tmp = UL_INIT_بفر;
	ثابت حرف *p;
	حرف *res = عدم;

	ssz = ul_رستو_readlink(pc, buf, bufsz, عدم);
	إذا (ssz <= 0)
		أرجع عدم;

	إذا ((p = ul_رستو_get_اڳياڙي(pc)))
		ul_بفر_append_string(&tmp, p);

	ul_بفر_append_string(&tmp, رستو_SYS_DEVBLOCK "/");
	ul_بفر_append_معلومات(&tmp, buf, ssz);

	p = ul_بفر_get_string(&tmp, &sz, عدم);
	إذا (p && sz <= bufsz) {
		يادگيري_نقل(buf, p, sz);
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
صحيح sysfs_blkdev_ايندڙ_subsystem(بنية رستو_cxt *pc __attribute__((اڻ_استعمال_ٿيل)),
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

#وصف_ڪريو REMOVABLE_FILENAME	"/removable"

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

	len = لڙي_ڊيگهه(chain);
	إذا (len + حجم(REMOVABLE_FILENAME) > PATH_MAX)
		أرجع 0;

	افعل {
		صحيح fd, rc;

		/* append "/removable" to the path */
		يادگيري_نقل(chain + len, REMOVABLE_FILENAME, حجم(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		إذا (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			اكسر;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		إذا (fd != -1) {
			rc = ul_read_all(fd, buf, حجم(buf));
			close(fd);

			إذا (rc > 0) {
				إذا (لڙي_ڀيٽ(buf, "fixed", min(rc, 5)) == 0) {
					أرجع 0;
				} وإلا إذا (لڙي_ڀيٽ(buf, "removable", min(rc, 9)) == 0) {
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

صحيح sysfs_blkdev_is_hotpluggable(بنية رستو_cxt *pc)
{
	حرف buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, حجم(buf));
	أرجع sysfs_devchain_is_removable(chain);
}

صحيح sysfs_blkdev_is_removable(بنية رستو_cxt *pc)
{
	صحيح rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	إذا (ul_رستو_read_s32(pc, &rc, "removable") == 0)
		أرجع rc;

	أرجع 0;
}

ساكن صحيح get_dm_wholedisk(بنية رستو_cxt *pc, حرف *diskname,
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
		*diskdevno = __sysfs_devname_to_devno(ul_رستو_get_اڳياڙي(pc), name, عدم);
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
صحيح sysfs_blkdev_get_wholedisk(	بنية رستو_cxt *pc,
				حرف *diskname,
				size_t len,
				dev_t *diskdevno)
{
	صحيح is_part = 0;

	إذا (!pc)
		أرجع -1;

	is_part = ul_رستو_access(pc, F_OK, "partition") == 0;
	إذا (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		حرف *uuid = عدم, *tmp, *اڳياڙي;

		ul_رستو_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		اڳياڙي = uuid ? strsep(&tmp, "-") : عدم;

		إذا (اڳياڙي && c_strncasecmp(اڳياڙي, "part", 4) == 0)
			is_part = 1;
		حرر(uuid);

		إذا (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			اذهب پورو;

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

		linklen = ul_رستو_readlink(pc, linkpath, حجم(linkpath), عدم);
		إذا (linklen < 0)
			اذهب err;

		stripoff_آخري_component(linkpath);	  /* dirname */
		name = stripoff_آخري_component(linkpath);   /* basename */
		إذا (!name)
			اذهب err;

		sysfs_devname_sys_منزل_dev(name);
		إذا (diskname && len)
			xstrncpy(diskname, name, len);

		إذا (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_رستو_get_اڳياڙي(pc), name, عدم);
			إذا (!*diskdevno)
				اذهب err;
		}
	}

پورو:
	أرجع 0;
err:
	أرجع -1;
}

صحيح sysfs_devno_منزل_wholedisk(dev_t devno, حرف *diskname,
		             size_t len, dev_t *diskdevno)
{
	بنية رستو_cxt *pc;
	صحيح rc = 0;

	إذا (!devno)
		أرجع -EINVAL;
	pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	إذا (!pc)
		أرجع -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_رستو(pc);
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
	بنية رستو_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	إذا (!pc)
		اذهب پورو;
	ul_رستو_refer_vfs(pc, vfs);
	إذا (ul_رستو_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب پورو;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	إذا (لڙي_ڀيٽ(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		إذا (p && *(p + 1))
			rc = 1;
	}
پورو:
	ul_unref_رستو(pc);
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
صحيح sysfs_devno_is_dm_ذاتي(dev_t devno, حرف **uuid,
			      ثابت بنية ul_vfs_ops *vfs)
{
	بنية رستو_cxt *pc = عدم;
	حرف *id = عدم;
	صحيح rc = 0;

	pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	إذا (!pc)
		اذهب پورو;
	ul_رستو_refer_vfs(pc, vfs);
	إذا (ul_رستو_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		اذهب پورو;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	إذا (لڙي_ڀيٽ(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		إذا (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} وإلا إذا (لڙي_ڀيٽ(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
پورو:
	ul_unref_رستو(pc);
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


صحيح sysfs_blkdev_scsi_get_hctl(بنية رستو_cxt *pc, صحيح *h, صحيح *c, صحيح *t, صحيح *l)
{
	حرف buf[PATH_MAX], *hctl;
	بنية sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_رستو_get_dialect(pc);

	إذا (!blk || blk->hctl_error)
		أرجع -EINVAL;
	إذا (blk->has_hctl)
		اذهب پورو;

	blk->hctl_error = 1;
	len = ul_رستو_readlink(pc, buf, حجم(buf), "device");
	إذا (len < 0)
		أرجع len;

	hctl = strrchr(buf, '/');
	إذا (!hctl)
		أرجع -1;
	hctl++;

	إذا (لڙي_مان_پڙهو(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_هدف, &blk->scsi_lun) != 4)
		أرجع -1;

	blk->has_hctl = 1;
پورو:
	إذا (h)
		*h = blk->scsi_host;
	إذا (c)
		*c = blk->scsi_channel;
	إذا (t)
		*t = blk->scsi_هدف;
	إذا (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	أرجع 0;
}


ساكن حرف *scsi_host_attribute_رستو(
			بنية رستو_cxt *pc,
			ثابت حرف *type,
			حرف *buf,
			size_t bufsz,
			ثابت حرف *attr)
{
	صحيح len;
	صحيح host;
	ثابت حرف *اڳياڙي;

	إذا (sysfs_blkdev_scsi_get_hctl(pc, &host, عدم, عدم, عدم))
		أرجع عدم;

	اڳياڙي = ul_رستو_get_اڳياڙي(pc);
	إذا (!اڳياڙي)
		اڳياڙي = "";

	إذا (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				اڳياڙي, رستو_SYS_CLASS, type, host, attr);
	وإلا
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				اڳياڙي, رستو_SYS_CLASS, type, host);

	أرجع (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

حرف *sysfs_blkdev_scsi_host_strdup_attribute(بنية رستو_cxt *pc,
			ثابت حرف *type, ثابت حرف *attr)
{
	حرف buf[1024];
	صحيح rc;
	FILE *f;

	إذا (!attr || !type ||
	    !scsi_host_attribute_رستو(pc, type, buf, حجم(buf), attr))
		أرجع عدم;

	إذا (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		أرجع عدم;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	أرجع rc == 1 ? strdup(buf) : عدم;
}

صحيح sysfs_blkdev_scsi_host_is(بنية رستو_cxt *pc, ثابت حرف *type)
{
	حرف buf[PATH_MAX];
	بنية stat st;

	إذا (!type || !scsi_host_attribute_رستو(pc, type,
				buf, حجم(buf), عدم))
		أرجع 0;

	أرجع stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ساكن حرف *scsi_attribute_رستو(بنية رستو_cxt *pc,
		حرف *buf, size_t bufsz, ثابت حرف *attr)
{
	صحيح len, h, c, t, l;
	ثابت حرف *اڳياڙي;

	إذا (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		أرجع عدم;

	اڳياڙي = ul_رستو_get_اڳياڙي(pc);
	إذا (!اڳياڙي)
		اڳياڙي = "";

	إذا (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				اڳياڙي, رستو_SYS_SCSI,
				h,c,t,l, attr);
	وإلا
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				اڳياڙي, رستو_SYS_SCSI,
				h,c,t,l);
	أرجع (len < 0 || (size_t) len >= bufsz) ? عدم : buf;
}

صحيح sysfs_blkdev_scsi_has_attribute(بنية رستو_cxt *pc, ثابت حرف *attr)
{
	حرف رستو[PATH_MAX];
	بنية stat st;

	إذا (!scsi_attribute_رستو(pc, رستو, حجم(رستو), attr))
		أرجع 0;

	أرجع stat(رستو, &st) == 0;
}

صحيح sysfs_blkdev_scsi_رستو_شامل_آهي(بنية رستو_cxt *pc, ثابت حرف *نمونو)
{
	حرف رستو[PATH_MAX], linkc[PATH_MAX];
	بنية stat st;
	ssize_t len;

	إذا (!scsi_attribute_رستو(pc, رستو, حجم(رستو), عدم))
		أرجع 0;

	إذا (stat(رستو, &st) != 0)
		أرجع 0;

	len = readlink(رستو, linkc, حجم(linkc) - 1);
	إذا (len < 0)
		أرجع 0;

	linkc[len] = '\0';
	أرجع لڙي_ڳولا(linkc, نمونو) != عدم;
}

ساكن dev_t read_devno(ثابت حرف *رستو)
{
	FILE *f;
	صحيح maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(رستو, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع 0;

	إذا (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	أرجع dev;
}

صحيح sysfs_devname_is_hidden(ثابت حرف *اڳياڙي, ثابت حرف *name)
{
	حرف buf[PATH_MAX];
	صحيح rc = 0, hidden = 0, len;
	FILE *f;

	إذا (لڙي_ڀيٽ("/dev/", name, 5) == 0)
		أرجع 0;

	إذا (!اڳياڙي)
		اڳياڙي = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, حجم(buf),
			"%s" رستو_SYS_بلاڪ "/%s/hidden",
			اڳياڙي, name);

	إذا (len < 0 || (size_t) len + 1 > حجم(buf))
		أرجع 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	إذا (!f)
		أرجع 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	أرجع rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ثابت حرف *اڳياڙي, ثابت حرف *name, ثابت حرف *والد)
{
	حرف buf[PATH_MAX];
	حرف *_name = عدم, *والد_2 = عدم;	/* name as encoded in sysfs */
	dev_t dev = 0;
	صحيح len;

	إذا (!اڳياڙي)
		اڳياڙي = "";

	أكد(name);

	إذا (لڙي_ڀيٽ("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		بنية stat st;

		إذا (stat(name, &st) == 0) {
			dev = st.st_rdev;
			اذهب پورو;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	إذا (!_name)
		اذهب پورو;
	sysfs_devname_dev_منزل_sys(_name);

	إذا (والد) {
		والد_2 = strdup(والد);
		إذا (!والد_2)
			اذهب پورو;
	}

	إذا (والد && لڙي_ڀيٽ("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_منزل_sys(والد_2);
		len = snprintf(buf, حجم(buf),
				"%s" رستو_SYS_بلاڪ "/%s/%s/dev",
				اڳياڙي,	والد_2, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب پورو;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		اذهب پورو;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, حجم(buf),
			"%s" رستو_SYS_بلاڪ "/%s/dev",
			اڳياڙي, _name);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		اذهب پورو;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	إذا (!dev && والد && ul_startswith(name, والد)) {
		len = snprintf(buf, حجم(buf),
				"%s" رستو_SYS_بلاڪ "/%s/%s/dev",
				اڳياڙي, والد_2, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب پورو;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	إذا (!dev) {
		len = snprintf(buf, حجم(buf),
				"%s" رستو_SYS_بلاڪ "/%s/device/dev",
				اڳياڙي, _name);
		إذا (len < 0 || (size_t) len >= حجم(buf))
			اذهب پورو;
		dev = read_devno(buf);
	}
پورو:
	حرر(_name);
	حرر(والد_2);
	أرجع dev;
}

dev_t sysfs_devname_منزل_devno(ثابت حرف *name)
{
	أرجع __sysfs_devname_to_devno(عدم, name, عدم);
}

حرف *sysfs_blkdev_get_رستو(بنية رستو_cxt *pc, حرف *buf, size_t bufsiz)
{
	ثابت حرف *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	حرف *res = عدم;
	size_t sz;
	بنية stat st;

	إذا (!name)
		اذهب پورو;

	sz = لڙي_ڊيگهه(name);
	إذا (sz + حجم("/dev/") > bufsiz)
		اذهب پورو;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	يادگيري_نقل(buf, "/dev/", 5);

	إذا (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
پورو:
	أرجع res;
}

dev_t sysfs_blkdev_get_devno(بنية رستو_cxt *pc)
{
	أرجع ((بنية sysfs_blkdev *) ul_رستو_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
حرف *sysfs_devno_منزل_devpath(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية رستو_cxt *pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	حرف *res = عدم;

	إذا (pc) {
		res = sysfs_blkdev_get_رستو(pc, buf, bufsiz);
		ul_unref_رستو(pc);
	}
	أرجع res;
}

حرف *sysfs_devno_منزل_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	بنية رستو_cxt *pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	حرف *res = عدم;

	إذا (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_رستو(pc);
	}
	أرجع res;
}

صحيح sysfs_devno_ڳڻپ_partitions(dev_t devno)
{
	بنية رستو_cxt *pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	صحيح n = 0;

	إذا (pc) {
		حرف buf[PATH_MAX + 1];
		حرف *name = sysfs_blkdev_get_name(pc, buf, حجم(buf));

		n = sysfs_blkdev_ڳڻپ_partitions(pc, name);
		ul_unref_رستو(pc);
	}
	أرجع n;
}

حرف *sysfs_chrdev_devno_منزل_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	بنية رستو_cxt *pc;
	حرف *name;
	ssize_t	sz;

	pc = ul_new_رستو(رستو_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	إذا (!pc)
		أرجع عدم;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_رستو_readlink(pc, link, حجم(link), عدم);
	ul_unref_رستو(pc);

	إذا (sz < 0)
		أرجع عدم;

	name = strrchr(link, '/');
	إذا (!name)
		أرجع عدم;

	name++;
	sz = لڙي_ڊيگهه(name);
	إذا ((size_t) sz + 1 > bufsiz)
		أرجع عدم;

	يادگيري_نقل(buf, name, sz + 1);
	sysfs_devname_sys_منزل_dev(buf);
	أرجع buf;

}

تعداد sysfs_byteorder sysfs_get_byteorder(بنية رستو_cxt *pc)
{
	صحيح rc;
	حرف buf[BUFSIZ];
	تعداد sysfs_byteorder ret;

	rc = ul_رستو_read_بفر(pc, buf, حجم(buf), رستو_SYS_CPU_BYTEORDER);
	إذا (rc < 0)
		اذهب unknown;

	إذا (لڙي_ڀيٽ(buf, "little", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		اذهب ٻاهريون;
	} وإلا إذا (لڙي_ڀيٽ(buf, "big", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		اذهب ٻاهريون;
	}

unknown:
#إذا __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#وإلاإذا __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#وإلا
#غلطي Unknown byte ترتيب
#جيڪڏهن_پورو

ٻاهريون:
	أرجع ret;
}

صحيح sysfs_get_پتو_بٽ(بنية رستو_cxt *pc)
{
	صحيح rc;
	صحيح پتو_بٽ;

	rc = ul_رستو_scanf(pc, رستو_SYS_پتو_بٽ, "%d", &پتو_بٽ);
	إذا (rc < 0)
		أرجع rc;
	إذا (پتو_بٽ < 0)
		أرجع -EINVAL;
	أرجع پتو_بٽ;
}


#جيڪڏهن_وصف_ٿيل TEST_پروگرام_SYSFS
#ضمن <errno.h>
#ضمن <stdlib.h>

صحيح مکيه(صحيح argc, حرف *argv[])
{
	بنية رستو_cxt *pc;
	حرف *devname;
	dev_t devno, disk_devno;
	حرف رستو[PATH_MAX], *sub, *chain;
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

	لکو("non-context:\n");
	لکو(" DEVNO:   %u (%u:%u)\n", (بنانشان صحيح) devno, major(devno), minor(devno));
	لکو(" DEVNAME: %s\n", sysfs_devno_منزل_devname(devno, رستو, حجم(رستو)));
	لکو(" DEVPATH: %s\n", sysfs_devno_منزل_devpath(devno, رستو, حجم(رستو)));

	sysfs_devno_منزل_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	لکو(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (بنانشان صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	لکو(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_رستو(devno, عدم, عدم);
	إذا (!pc)
		اذهب پورو;

	لکو("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	لکو(" DEVNO:   %u (%u:%u)\n", (بنانشان صحيح) devno, major(devno), minor(devno));
	لکو(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, رستو, حجم(رستو)));
	لکو(" DEVPATH: %s\n", sysfs_blkdev_get_رستو(pc, رستو, حجم(رستو)));

	sysfs_devno_منزل_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	لکو(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (بنانشان صحيح) disk_devno, major(disk_devno), minor(disk_devno));
	لکو(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_رستو_access(pc, F_OK, "partition") == 0;
	لکو(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	إذا (is_part && disk_devno) {
		بنية رستو_cxt *disk_pc =  ul_new_sysfs_رستو(disk_devno, عدم, عدم);
		sysfs_blkdev_سيٽ_والد(pc, disk_pc);

		ul_unref_رستو(disk_pc);
	}

	لکو(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	لکو(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	لکو(" SLAVES: %d\n", ul_رستو_ڳڻپ_dirents(pc, "slaves"));

	إذا (!is_part) {
		لکو("First 5 partitions:\n");
		لكل (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_منزل_devno(pc, i);
			إذا (dev)
				لکو("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	إذا (ul_رستو_read_u64(pc, &u64, "size") != 0)
		لکو(" (!) read SIZE failed\n");
	وإلا
		لکو(" SIZE: %"PRIu64"\n", u64);

	إذا (ul_رستو_read_s32(pc, &i, "queue/hw_sector_size"))
		لکو(" (!) read SECTOR failed\n");
	وإلا
		لکو(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, رستو, حجم(رستو));
	لکو(" SUBSYSTEMS:\n");

	طالما (chain && sysfs_blkdev_ايندڙ_subsystem(pc, chain, &sub) == 0) {
		لکو("\t%s\n", sub);
		حرر(sub);
	}

	rc = EXIT_SUCCESS;
پورو:
	ul_unref_رستو(pc);
	أرجع rc;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_SYSFS */
