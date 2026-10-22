/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#شامل <ctype.h>
#شامل <inttypes.h>
#شامل <libgen.h>
#شامل <fcntl.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "pathnames.h"
#شامل "sysfs.h"
#شامل "fileutils.h"
#شامل "all-io.h"
#شامل "debug.h"
#شامل "strutils.h"
#شامل "buffer.h"

ساکن خالی sysfs_blkdev_deinit_راستہ(ساخت راستہ_cxt *pc);
ساکن صحیح  sysfs_blkdev_enoent_redirect(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, صحیح *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ساکن UL_DEBUG_DEFINE_نقاب(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو ULSYSFS_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULSYSFS_DEBUG_CXT	(1 << 2)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

خالی ul_sysfs_init_debug(خالی)
{
	اگر (ulsysfs_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ساخت راستہ_cxt *ul_new_sysfs_راستہ(dev_t devno, ساخت راستہ_cxt *والد, ثابت حرف *سابقہ)
{
	ساخت راستہ_cxt *pc = ul_new_راستہ(تہی);

	اگر (!pc)
		واپس تہی;
	اگر (سابقہ)
		ul_راستہ_سیٹ_سابقہ(pc, سابقہ);

	اگر (sysfs_blkdev_init_راستہ(pc, devno, والد) != 0) {
		ul_unref_راستہ(pc);
		واپس تہی;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	واپس pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
صحیح sysfs_blkdev_init_راستہ(ساخت راستہ_cxt *pc, dev_t devno, ساخت راستہ_cxt *والد)
{
	ساخت sysfs_blkdev *blk;
	صحیح rc;
	حرف buf[حجم(راستہ_SYS_DEVBLOCK)
		 + حجم(stringify_قدر(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, حجم(buf), راستہ_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_راستہ_سیٹ_dir(pc, buf);
	اگر (rc)
		واپس rc;

	/* make sure path exists */
	rc = ul_راستہ_get_dirfd(pc);
	اگر (rc < 0)
		واپس rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_راستہ_get_dialect(pc);
	اگر (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = صفر_حافظہ_دو(1, حجم(ساخت sysfs_blkdev));
		اگر (!blk)
			واپس -ENOMEM;

		ul_راستہ_سیٹ_dialect(pc, blk, sysfs_blkdev_deinit_راستہ);
		ul_راستہ_سیٹ_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_سیٹ_والد(pc, والد);

	واپس 0;
}

ساکن خالی sysfs_blkdev_deinit_راستہ(ساخت راستہ_cxt *pc)
{
	ساخت sysfs_blkdev *blk;

	اگر (!pc)
		واپس;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_راستہ_get_dialect(pc);
	اگر (!blk)
		واپس;

	ul_unref_راستہ(blk->والد);
	آزاد(blk);

	ul_راستہ_سیٹ_dialect(pc, تہی, تہی);
}

صحیح sysfs_blkdev_سیٹ_والد(ساخت راستہ_cxt *pc, ساخت راستہ_cxt *والد)
{
	ساخت sysfs_blkdev *blk = ul_راستہ_get_dialect(pc);

	اگر (!pc || !blk)
		واپس -EINVAL;

	اگر (blk->والد) {
		ul_unref_راستہ(blk->والد);
		blk->والد = تہی;
	}

	اگر (والد) {
		ul_ref_راستہ(والد);
		blk->والد = والد;
	} ورنہ
		blk->والد = تہی;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	واپس 0;
}

ساخت راستہ_cxt *sysfs_blkdev_get_والد(ساخت راستہ_cxt *pc)
{
	ساخت sysfs_blkdev *blk = ul_راستہ_get_dialect(pc);
	واپس blk ? blk->والد : تہی;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ساکن صحیح sysfs_blkdev_enoent_redirect(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, صحیح *dirfd)
{
	ساخت sysfs_blkdev *blk = ul_راستہ_get_dialect(pc);

	اگر (blk && blk->والد && راستہ) {
		*dirfd = ul_راستہ_get_dirfd(blk->والد);
		اگر (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", راستہ));
			واپس 0;
		}
	}
	واپس 1;	/* no redirect */
}

حرف *sysfs_blkdev_get_name(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	حرف *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_راستہ_readlink(pc, link, حجم(link), تہی);
	اگر (sz < 0)
		واپس تہی;

	name = strrchr(link, '/');
	اگر (!name)
		واپس تہی;

	name++;
	sz = لڑی_لمبائی(name);
	اگر ((size_t) sz + 1 > bufsiz)
		واپس تہی;

	حافظہ_نقل(buf, name, sz + 1);
	sysfs_devname_sys_مقصد_dev(buf);
	واپس buf;
}

صحیح sysfs_blkdev_is_تقسیم_dirent(DIR *dir, ساخت dirent *d, ثابت حرف *والد_name)
{
	حرف راستہ[NAME_MAX + 6 + 1];

#اگر_تعریف_شدہ _DIRENT_HAVE_D_TYPE
	اگر (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		واپس 0;
#اگر_ختم
	size_t len = 0;

	اگر (والد_name) {
		ثابت حرف *p = والد_name;

		/* /dev/sda --> "sda" */
		اگر (*والد_name == '/') {
			p = strrchr(والد_name, '/');
			اگر (!p)
				واپس 0;
			p++;
		}

		len = لڑی_لمبائی(p);
		اگر ((لڑی_لمبائی(d->d_name) <= len) || (لڑی_موازنہ(p, d->d_name, len) != 0))
			len = 0;
	}

	اگر (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		واپس ((*(d->d_name + len) == 'p' && ہندسہ_ہے(*(d->d_name + len + 1)))
			|| ہندسہ_ہے(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(راستہ, حجم(راستہ), "%s/start", d->d_name);

	واپس faccessat(dirfd(dir), راستہ, R_OK, 0) == 0;
}

صحیح sysfs_blkdev_گنتی_partitions(ساخت راستہ_cxt *pc, ثابت حرف *devname)
{
	DIR *dir;
	ساخت dirent *d;
	صحیح r = 0;

	dir = ul_راستہ_opendir(pc, تہی);
	اگر (!dir)
		واپس 0;

	جب ((d = xreaddir(dir))) {
		اگر (sysfs_blkdev_is_تقسیم_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	واپس r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_مقصد_devno(ساخت راستہ_cxt *pc, صحیح partno)
{
	DIR *dir;
	ساخت dirent *d;
	dev_t devno = 0;

	dir = ul_راستہ_opendir(pc, تہی);
	اگر (!dir)
		واپس 0;

	جب ((d = xreaddir(dir))) {
		صحیح n;

		اگر (!sysfs_blkdev_is_تقسیم_dirent(dir, d, تہی))
			جاری;

		اگر (ul_راستہ_readf_s32(pc, &n, "%s/partition", d->d_name))
			جاری;

		اگر (n == partno) {
			اگر (ul_راستہ_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				توڑو;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (صحیح) partno, (صحیح) devno));
	واپس devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
حرف *sysfs_blkdev_get_slave(ساخت راستہ_cxt *pc)
{
	DIR *dir;
	ساخت dirent *d;
	حرف *name = تہی;

	dir = ul_راستہ_opendir(pc, "slaves");
	اگر (!dir)
		واپس تہی;

	جب ((d = xreaddir(dir))) {
		اگر (name)
			جاؤ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	واپس name;
err:
	آزاد(name);
	closedir(dir);
	واپس تہی;
}


#تعریف_کرو SUBSYSTEM_LINKNAME	"/subsystem"

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
ساکن حرف *get_subsystem(حرف *chain, حرف *buf, size_t bufsz)
{
	size_t len;
	حرف *p;

	اگر (!chain || !*chain)
		واپس تہی;

	len = لڑی_لمبائی(chain);
	اگر (len + حجم(SUBSYSTEM_LINKNAME) > PATH_MAX)
		واپس تہی;

	کرو {
		ssize_t sz;

		/* append "/subsystem" to the path */
		حافظہ_نقل(chain + len, SUBSYSTEM_LINKNAME, حجم(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		اگر (p) {
			*p = '\0';
			len = p - chain;
		}

		اگر (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			واپس basename(buf);
		}

	} جب (p);

	واپس تہی;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
حرف *sysfs_blkdev_get_devchain(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ساخت ul_بفر tmp = UL_INIT_بفر;
	ثابت حرف *p;
	حرف *res = تہی;

	ssz = ul_راستہ_readlink(pc, buf, bufsz, تہی);
	اگر (ssz <= 0)
		واپس تہی;

	اگر ((p = ul_راستہ_get_سابقہ(pc)))
		ul_بفر_append_string(&tmp, p);

	ul_بفر_append_string(&tmp, راستہ_SYS_DEVBLOCK "/");
	ul_بفر_append_ڈیٹا(&tmp, buf, ssz);

	p = ul_بفر_get_string(&tmp, &sz, تہی);
	اگر (p && sz <= bufsz) {
		حافظہ_نقل(buf, p, sz);
		res = buf;
	}
	ul_بفر_free_ڈیٹا(&tmp);
	واپس res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
صحیح sysfs_blkdev_اگلا_subsystem(ساخت راستہ_cxt *pc __attribute__((غیر_مستعمل)),
			 حرف *devchain, حرف **subsys)
{
	حرف subbuf[PATH_MAX];
	حرف *sub;

	اگر (!subsys || !devchain)
		واپس -EINVAL;

	*subsys = تہی;

	جب ((sub = get_subsystem(devchain, subbuf, حجم(subbuf)))) {
		*subsys = strdup(sub);
		اگر (!*subsys)
			واپس -ENOMEM;
		واپس 0;
	}

	واپس 1;
}

#تعریف_کرو REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ساکن صحیح sysfs_devchain_is_removable(حرف *chain)
{
	size_t len;
	حرف buf[20];
	حرف *p;

	اگر (!chain || !*chain)
		واپس 0;

	len = لڑی_لمبائی(chain);
	اگر (len + حجم(REMOVABLE_FILENAME) > PATH_MAX)
		واپس 0;

	کرو {
		صحیح fd, rc;

		/* append "/removable" to the path */
		حافظہ_نقل(chain + len, REMOVABLE_FILENAME, حجم(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		اگر (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			توڑو;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		اگر (fd != -1) {
			rc = ul_read_all(fd, buf, حجم(buf));
			close(fd);

			اگر (rc > 0) {
				اگر (لڑی_موازنہ(buf, "fixed", min(rc, 5)) == 0) {
					واپس 0;
				} ورنہ اگر (لڑی_موازنہ(buf, "removable", min(rc, 9)) == 0) {
					واپس 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		اگر (p) {
			*p = '\0';
			len = p - chain;
		}

	} جب (p);

	واپس 0;
}

صحیح sysfs_blkdev_is_hotpluggable(ساخت راستہ_cxt *pc)
{
	حرف buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, حجم(buf));
	واپس sysfs_devchain_is_removable(chain);
}

صحیح sysfs_blkdev_is_removable(ساخت راستہ_cxt *pc)
{
	صحیح rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	اگر (ul_راستہ_read_s32(pc, &rc, "removable") == 0)
		واپس rc;

	واپس 0;
}

ساکن صحیح get_dm_wholedisk(ساخت راستہ_cxt *pc, حرف *diskname,
                            size_t len, dev_t *diskdevno)
{
	صحیح rc = 0;
	حرف *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	اگر (!name)
		واپس -1;

	اگر (diskname && len)
		xstrncpy(diskname, name, len);

	اگر (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_راستہ_get_سابقہ(pc), name, تہی);
		اگر (!*diskdevno)
			rc = -1;
	}

	آزاد(name);
	واپس rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
صحیح sysfs_blkdev_get_wholedisk(	ساخت راستہ_cxt *pc,
				حرف *diskname,
				size_t len,
				dev_t *diskdevno)
{
	صحیح is_part = 0;

	اگر (!pc)
		واپس -1;

	is_part = ul_راستہ_access(pc, F_OK, "partition") == 0;
	اگر (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		حرف *uuid = تہی, *tmp, *سابقہ;

		ul_راستہ_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		سابقہ = uuid ? strsep(&tmp, "-") : تہی;

		اگر (سابقہ && c_strncasecmp(سابقہ, "part", 4) == 0)
			is_part = 1;
		آزاد(uuid);

		اگر (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			جاؤ مکمل;

		is_part = 0;
	}

	اگر (!is_part) {
		/*
		 * unpartitioned device
		 */
		اگر (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			جاؤ err;
		اگر (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ورنہ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		حرف linkpath[PATH_MAX];
		حرف *name;
		ssize_t	linklen;

		linklen = ul_راستہ_readlink(pc, linkpath, حجم(linkpath), تہی);
		اگر (linklen < 0)
			جاؤ err;

		stripoff_آخری_component(linkpath);	  /* dirname */
		name = stripoff_آخری_component(linkpath);   /* basename */
		اگر (!name)
			جاؤ err;

		sysfs_devname_sys_مقصد_dev(name);
		اگر (diskname && len)
			xstrncpy(diskname, name, len);

		اگر (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_راستہ_get_سابقہ(pc), name, تہی);
			اگر (!*diskdevno)
				جاؤ err;
		}
	}

مکمل:
	واپس 0;
err:
	واپس -1;
}

صحیح sysfs_devno_مقصد_wholedisk(dev_t devno, حرف *diskname,
		             size_t len, dev_t *diskdevno)
{
	ساخت راستہ_cxt *pc;
	صحیح rc = 0;

	اگر (!devno)
		واپس -EINVAL;
	pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	اگر (!pc)
		واپس -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_راستہ(pc);
	واپس rc;
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
صحیح sysfs_devno_is_dm_hidden(dev_t devno, حرف **uuid,
			     ثابت ساخت ul_vfs_ops *vfs)
{
	ساخت راستہ_cxt *pc = تہی;
	حرف *id = تہی;
	صحیح rc = 0;

	pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	اگر (!pc)
		جاؤ مکمل;
	ul_راستہ_refer_vfs(pc, vfs);
	اگر (ul_راستہ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		جاؤ مکمل;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	اگر (لڑی_موازنہ(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		اگر (p && *(p + 1))
			rc = 1;
	}
مکمل:
	ul_unref_راستہ(pc);
	اگر (uuid)
		*uuid = id;
	ورنہ
		آزاد(id);
	واپس rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
صحیح sysfs_devno_is_dm_نجی(dev_t devno, حرف **uuid,
			      ثابت ساخت ul_vfs_ops *vfs)
{
	ساخت راستہ_cxt *pc = تہی;
	حرف *id = تہی;
	صحیح rc = 0;

	pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	اگر (!pc)
		جاؤ مکمل;
	ul_راستہ_refer_vfs(pc, vfs);
	اگر (ul_راستہ_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		جاؤ مکمل;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	اگر (لڑی_موازنہ(id, "LVM-", 4) == 0) {
		حرف *p = strrchr(id + 4, '-');

		اگر (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ورنہ اگر (لڑی_موازنہ(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
مکمل:
	ul_unref_راستہ(pc);
	اگر (uuid)
		*uuid = id;
	ورنہ
		آزاد(id);
	واپس rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
صحیح sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	اگر (sysfs_devno_مقصد_wholedisk(devno, تہی, 0, &disk) != 0)
		واپس -1;

	واپس devno == disk;
}


صحیح sysfs_blkdev_scsi_get_hctl(ساخت راستہ_cxt *pc, صحیح *h, صحیح *c, صحیح *t, صحیح *l)
{
	حرف buf[PATH_MAX], *hctl;
	ساخت sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_راستہ_get_dialect(pc);

	اگر (!blk || blk->hctl_error)
		واپس -EINVAL;
	اگر (blk->has_hctl)
		جاؤ مکمل;

	blk->hctl_error = 1;
	len = ul_راستہ_readlink(pc, buf, حجم(buf), "device");
	اگر (len < 0)
		واپس len;

	hctl = strrchr(buf, '/');
	اگر (!hctl)
		واپس -1;
	hctl++;

	اگر (لڑی_سے_پڑھو(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_ہدف, &blk->scsi_lun) != 4)
		واپس -1;

	blk->has_hctl = 1;
مکمل:
	اگر (h)
		*h = blk->scsi_host;
	اگر (c)
		*c = blk->scsi_channel;
	اگر (t)
		*t = blk->scsi_ہدف;
	اگر (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	واپس 0;
}


ساکن حرف *scsi_host_attribute_راستہ(
			ساخت راستہ_cxt *pc,
			ثابت حرف *type,
			حرف *buf,
			size_t bufsz,
			ثابت حرف *attr)
{
	صحیح len;
	صحیح host;
	ثابت حرف *سابقہ;

	اگر (sysfs_blkdev_scsi_get_hctl(pc, &host, تہی, تہی, تہی))
		واپس تہی;

	سابقہ = ul_راستہ_get_سابقہ(pc);
	اگر (!سابقہ)
		سابقہ = "";

	اگر (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				سابقہ, راستہ_SYS_CLASS, type, host, attr);
	ورنہ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				سابقہ, راستہ_SYS_CLASS, type, host);

	واپس (len < 0 || (size_t) len >= bufsz) ? تہی : buf;
}

حرف *sysfs_blkdev_scsi_host_strdup_attribute(ساخت راستہ_cxt *pc,
			ثابت حرف *type, ثابت حرف *attr)
{
	حرف buf[1024];
	صحیح rc;
	FILE *f;

	اگر (!attr || !type ||
	    !scsi_host_attribute_راستہ(pc, type, buf, حجم(buf), attr))
		واپس تہی;

	اگر (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		واپس تہی;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	واپس rc == 1 ? strdup(buf) : تہی;
}

صحیح sysfs_blkdev_scsi_host_is(ساخت راستہ_cxt *pc, ثابت حرف *type)
{
	حرف buf[PATH_MAX];
	ساخت stat st;

	اگر (!type || !scsi_host_attribute_راستہ(pc, type,
				buf, حجم(buf), تہی))
		واپس 0;

	واپس stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ساکن حرف *scsi_attribute_راستہ(ساخت راستہ_cxt *pc,
		حرف *buf, size_t bufsz, ثابت حرف *attr)
{
	صحیح len, h, c, t, l;
	ثابت حرف *سابقہ;

	اگر (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		واپس تہی;

	سابقہ = ul_راستہ_get_سابقہ(pc);
	اگر (!سابقہ)
		سابقہ = "";

	اگر (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				سابقہ, راستہ_SYS_SCSI,
				h,c,t,l, attr);
	ورنہ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				سابقہ, راستہ_SYS_SCSI,
				h,c,t,l);
	واپس (len < 0 || (size_t) len >= bufsz) ? تہی : buf;
}

صحیح sysfs_blkdev_scsi_has_attribute(ساخت راستہ_cxt *pc, ثابت حرف *attr)
{
	حرف راستہ[PATH_MAX];
	ساخت stat st;

	اگر (!scsi_attribute_راستہ(pc, راستہ, حجم(راستہ), attr))
		واپس 0;

	واپس stat(راستہ, &st) == 0;
}

صحیح sysfs_blkdev_scsi_راستہ_شامل_ہے(ساخت راستہ_cxt *pc, ثابت حرف *نمونہ)
{
	حرف راستہ[PATH_MAX], linkc[PATH_MAX];
	ساخت stat st;
	ssize_t len;

	اگر (!scsi_attribute_راستہ(pc, راستہ, حجم(راستہ), تہی))
		واپس 0;

	اگر (stat(راستہ, &st) != 0)
		واپس 0;

	len = readlink(راستہ, linkc, حجم(linkc) - 1);
	اگر (len < 0)
		واپس 0;

	linkc[len] = '\0';
	واپس لڑی_تلاش(linkc, نمونہ) != تہی;
}

ساکن dev_t read_devno(ثابت حرف *راستہ)
{
	FILE *f;
	صحیح maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(راستہ, "r" UL_CLOEXECSTR);
	اگر (!f)
		واپس 0;

	اگر (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	واپس dev;
}

صحیح sysfs_devname_is_hidden(ثابت حرف *سابقہ, ثابت حرف *name)
{
	حرف buf[PATH_MAX];
	صحیح rc = 0, hidden = 0, len;
	FILE *f;

	اگر (لڑی_موازنہ("/dev/", name, 5) == 0)
		واپس 0;

	اگر (!سابقہ)
		سابقہ = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, حجم(buf),
			"%s" راستہ_SYS_بلاک "/%s/hidden",
			سابقہ, name);

	اگر (len < 0 || (size_t) len + 1 > حجم(buf))
		واپس 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	اگر (!f)
		واپس 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	واپس rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ثابت حرف *سابقہ, ثابت حرف *name, ثابت حرف *والد)
{
	حرف buf[PATH_MAX];
	حرف *_name = تہی, *والد_2 = تہی;	/* name as encoded in sysfs */
	dev_t dev = 0;
	صحیح len;

	اگر (!سابقہ)
		سابقہ = "";

	أكد(name);

	اگر (لڑی_موازنہ("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ساخت stat st;

		اگر (stat(name, &st) == 0) {
			dev = st.st_rdev;
			جاؤ مکمل;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	اگر (!_name)
		جاؤ مکمل;
	sysfs_devname_dev_مقصد_sys(_name);

	اگر (والد) {
		والد_2 = strdup(والد);
		اگر (!والد_2)
			جاؤ مکمل;
	}

	اگر (والد && لڑی_موازنہ("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_مقصد_sys(والد_2);
		len = snprintf(buf, حجم(buf),
				"%s" راستہ_SYS_بلاک "/%s/%s/dev",
				سابقہ,	والد_2, _name);
		اگر (len < 0 || (size_t) len >= حجم(buf))
			جاؤ مکمل;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		جاؤ مکمل;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, حجم(buf),
			"%s" راستہ_SYS_بلاک "/%s/dev",
			سابقہ, _name);
	اگر (len < 0 || (size_t) len >= حجم(buf))
		جاؤ مکمل;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	اگر (!dev && والد && ul_startswith(name, والد)) {
		len = snprintf(buf, حجم(buf),
				"%s" راستہ_SYS_بلاک "/%s/%s/dev",
				سابقہ, والد_2, _name);
		اگر (len < 0 || (size_t) len >= حجم(buf))
			جاؤ مکمل;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	اگر (!dev) {
		len = snprintf(buf, حجم(buf),
				"%s" راستہ_SYS_بلاک "/%s/device/dev",
				سابقہ, _name);
		اگر (len < 0 || (size_t) len >= حجم(buf))
			جاؤ مکمل;
		dev = read_devno(buf);
	}
مکمل:
	آزاد(_name);
	آزاد(والد_2);
	واپس dev;
}

dev_t sysfs_devname_مقصد_devno(ثابت حرف *name)
{
	واپس __sysfs_devname_to_devno(تہی, name, تہی);
}

حرف *sysfs_blkdev_get_راستہ(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsiz)
{
	ثابت حرف *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	حرف *res = تہی;
	size_t sz;
	ساخت stat st;

	اگر (!name)
		جاؤ مکمل;

	sz = لڑی_لمبائی(name);
	اگر (sz + حجم("/dev/") > bufsiz)
		جاؤ مکمل;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	حافظہ_نقل(buf, "/dev/", 5);

	اگر (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
مکمل:
	واپس res;
}

dev_t sysfs_blkdev_get_devno(ساخت راستہ_cxt *pc)
{
	واپس ((ساخت sysfs_blkdev *) ul_راستہ_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
حرف *sysfs_devno_مقصد_devpath(dev_t devno, حرف *buf, size_t bufsiz)
{
	ساخت راستہ_cxt *pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	حرف *res = تہی;

	اگر (pc) {
		res = sysfs_blkdev_get_راستہ(pc, buf, bufsiz);
		ul_unref_راستہ(pc);
	}
	واپس res;
}

حرف *sysfs_devno_مقصد_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	ساخت راستہ_cxt *pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	حرف *res = تہی;

	اگر (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_راستہ(pc);
	}
	واپس res;
}

صحیح sysfs_devno_گنتی_partitions(dev_t devno)
{
	ساخت راستہ_cxt *pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	صحیح n = 0;

	اگر (pc) {
		حرف buf[PATH_MAX + 1];
		حرف *name = sysfs_blkdev_get_name(pc, buf, حجم(buf));

		n = sysfs_blkdev_گنتی_partitions(pc, name);
		ul_unref_راستہ(pc);
	}
	واپس n;
}

حرف *sysfs_chrdev_devno_مقصد_devname(dev_t devno, حرف *buf, size_t bufsiz)
{
	حرف link[PATH_MAX];
	ساخت راستہ_cxt *pc;
	حرف *name;
	ssize_t	sz;

	pc = ul_new_راستہ(راستہ_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	اگر (!pc)
		واپس تہی;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_راستہ_readlink(pc, link, حجم(link), تہی);
	ul_unref_راستہ(pc);

	اگر (sz < 0)
		واپس تہی;

	name = strrchr(link, '/');
	اگر (!name)
		واپس تہی;

	name++;
	sz = لڑی_لمبائی(name);
	اگر ((size_t) sz + 1 > bufsiz)
		واپس تہی;

	حافظہ_نقل(buf, name, sz + 1);
	sysfs_devname_sys_مقصد_dev(buf);
	واپس buf;

}

شمار sysfs_byteorder sysfs_get_byteorder(ساخت راستہ_cxt *pc)
{
	صحیح rc;
	حرف buf[BUFSIZ];
	شمار sysfs_byteorder ret;

	rc = ul_راستہ_read_بفر(pc, buf, حجم(buf), راستہ_SYS_CPU_BYTEORDER);
	اگر (rc < 0)
		جاؤ unknown;

	اگر (لڑی_موازنہ(buf, "little", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		جاؤ برآمد;
	} ورنہ اگر (لڑی_موازنہ(buf, "big", حجم(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		جاؤ برآمد;
	}

unknown:
#اگر __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#وگرنہ_اگر __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ورنہ
#غلطی Unknown byte ترتیب
#اگر_ختم

برآمد:
	واپس ret;
}

صحیح sysfs_get_پتہ_بٹ(ساخت راستہ_cxt *pc)
{
	صحیح rc;
	صحیح پتہ_بٹ;

	rc = ul_راستہ_scanf(pc, راستہ_SYS_پتہ_بٹ, "%d", &پتہ_بٹ);
	اگر (rc < 0)
		واپس rc;
	اگر (پتہ_بٹ < 0)
		واپس -EINVAL;
	واپس پتہ_بٹ;
}


#اگر_تعریف_شدہ TEST_پروگرام_SYSFS
#شامل <errno.h>
#شامل <stdlib.h>

صحیح مین(صحیح argc, حرف *argv[])
{
	ساخت راستہ_cxt *pc;
	حرف *devname;
	dev_t devno, disk_devno;
	حرف راستہ[PATH_MAX], *sub, *chain;
	حرف diskname[32];
	صحیح i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	اگر (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_مقصد_devno(devname);

	اگر (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	لکھو("non-context:\n");
	لکھو(" DEVNO:   %u (%u:%u)\n", (غیرعلامتی صحیح) devno, major(devno), minor(devno));
	لکھو(" DEVNAME: %s\n", sysfs_devno_مقصد_devname(devno, راستہ, حجم(راستہ)));
	لکھو(" DEVPATH: %s\n", sysfs_devno_مقصد_devpath(devno, راستہ, حجم(راستہ)));

	sysfs_devno_مقصد_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	لکھو(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (غیرعلامتی صحیح) disk_devno, major(disk_devno), minor(disk_devno));
	لکھو(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_راستہ(devno, تہی, تہی);
	اگر (!pc)
		جاؤ مکمل;

	لکھو("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	لکھو(" DEVNO:   %u (%u:%u)\n", (غیرعلامتی صحیح) devno, major(devno), minor(devno));
	لکھو(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, راستہ, حجم(راستہ)));
	لکھو(" DEVPATH: %s\n", sysfs_blkdev_get_راستہ(pc, راستہ, حجم(راستہ)));

	sysfs_devno_مقصد_wholedisk(devno, diskname, حجم(diskname), &disk_devno);
	لکھو(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (غیرعلامتی صحیح) disk_devno, major(disk_devno), minor(disk_devno));
	لکھو(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_راستہ_access(pc, F_OK, "partition") == 0;
	لکھو(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	اگر (is_part && disk_devno) {
		ساخت راستہ_cxt *disk_pc =  ul_new_sysfs_راستہ(disk_devno, تہی, تہی);
		sysfs_blkdev_سیٹ_والد(pc, disk_pc);

		ul_unref_راستہ(disk_pc);
	}

	لکھو(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	لکھو(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	لکھو(" SLAVES: %d\n", ul_راستہ_گنتی_dirents(pc, "slaves"));

	اگر (!is_part) {
		لکھو("First 5 partitions:\n");
		برائے (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_مقصد_devno(pc, i);
			اگر (dev)
				لکھو("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	اگر (ul_راستہ_read_u64(pc, &u64, "size") != 0)
		لکھو(" (!) read SIZE failed\n");
	ورنہ
		لکھو(" SIZE: %"PRIu64"\n", u64);

	اگر (ul_راستہ_read_s32(pc, &i, "queue/hw_sector_size"))
		لکھو(" (!) read SECTOR failed\n");
	ورنہ
		لکھو(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, راستہ, حجم(راستہ));
	لکھو(" SUBSYSTEMS:\n");

	جب (chain && sysfs_blkdev_اگلا_subsystem(pc, chain, &sub) == 0) {
		لکھو("\t%s\n", sub);
		آزاد(sub);
	}

	rc = EXIT_SUCCESS;
مکمل:
	ul_unref_راستہ(pc);
	واپس rc;
}
#اگر_ختم /* TEST_PROGRAM_SYSFS */
