/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <libgen.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "sysfs.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "buffer.h"

स्थिर शून्य sysfs_blkdev_deinit_path(संरचना path_cxt *pc);
स्थिर पूर्णाङ्क  sysfs_blkdev_enoent_redirect(संरचना path_cxt *pc, अचर वर्ण *path, पूर्णाङ्क *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

शून्य ul_sysfs_init_debug(शून्य)
{
	यदि (ulsysfs_debug_mask)
		फर्काऊ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

संरचना path_cxt *ul_new_sysfs_path(dev_t devno, संरचना path_cxt *parent, अचर वर्ण *prefix)
{
	संरचना path_cxt *pc = ul_new_path(रिक्त);

	यदि (!pc)
		फर्काऊ रिक्त;
	यदि (prefix)
		ul_path_set_prefix(pc, prefix);

	यदि (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		फर्काऊ रिक्त;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	फर्काऊ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णाङ्क sysfs_blkdev_init_path(संरचना path_cxt *pc, dev_t devno, संरचना path_cxt *parent)
{
	संरचना sysfs_blkdev *blk;
	पूर्णाङ्क rc;
	वर्ण buf[परिमाण(_PATH_SYS_DEVBLOCK)
		 + परिमाण(stringify_मान(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, परिमाण(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	यदि (rc)
		फर्काऊ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	यदि (rc < 0)
		फर्काऊ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	यदि (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = सुस्मृति(1, परिमाण(संरचना sysfs_blkdev));
		यदि (!blk)
			फर्काऊ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	फर्काऊ 0;
}

स्थिर शून्य sysfs_blkdev_deinit_path(संरचना path_cxt *pc)
{
	संरचना sysfs_blkdev *blk;

	यदि (!pc)
		फर्काऊ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	यदि (!blk)
		फर्काऊ;

	ul_unref_path(blk->parent);
	मुक्त(blk);

	ul_path_set_dialect(pc, रिक्त, रिक्त);
}

पूर्णाङ्क sysfs_blkdev_set_parent(संरचना path_cxt *pc, संरचना path_cxt *parent)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);

	यदि (!pc || !blk)
		फर्काऊ -EINVAL;

	यदि (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = रिक्त;
	}

	यदि (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} अन्यथा
		blk->parent = रिक्त;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	फर्काऊ 0;
}

संरचना path_cxt *sysfs_blkdev_get_parent(संरचना path_cxt *pc)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);
	फर्काऊ blk ? blk->parent : रिक्त;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
स्थिर पूर्णाङ्क sysfs_blkdev_enoent_redirect(संरचना path_cxt *pc, अचर वर्ण *path, पूर्णाङ्क *dirfd)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);

	यदि (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		यदि (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			फर्काऊ 0;
		}
	}
	फर्काऊ 1;	/* no redirect */
}

वर्ण *sysfs_blkdev_get_name(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsiz)
{
	वर्ण link[PATH_MAX];
	वर्ण *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, परिमाण(link), रिक्त);
	यदि (sz < 0)
		फर्काऊ रिक्त;

	name = म_खोजप(link, '/');
	यदि (!name)
		फर्काऊ रिक्त;

	name++;
	sz = म_माप(name);
	यदि ((माप_प्रकार) sz + 1 > bufsiz)
		फर्काऊ रिक्त;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	फर्काऊ buf;
}

पूर्णाङ्क sysfs_blkdev_is_partition_dirent(सूची *dir, संरचना dirent *d, अचर वर्ण *parent_name)
{
	वर्ण path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		फर्काऊ 0;
#endif
	माप_प्रकार len = 0;

	यदि (parent_name) {
		अचर वर्ण *p = parent_name;

		/* /dev/sda --> "sda" */
		यदि (*parent_name == '/') {
			p = म_खोजप(parent_name, '/');
			यदि (!p)
				फर्काऊ 0;
			p++;
		}

		len = म_माप(p);
		यदि ((म_माप(d->d_name) <= len) || (म_भेदन(p, d->d_name, len) != 0))
			len = 0;
	}

	यदि (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		फर्काऊ ((*(d->d_name + len) == 'p' && है_अंक(*(d->d_name + len + 1)))
			|| है_अंक(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, परिमाण(path), "%s/start", d->d_name);

	फर्काऊ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

पूर्णाङ्क sysfs_blkdev_गन्ती_partitions(संरचना path_cxt *pc, अचर वर्ण *devname)
{
	सूची *dir;
	संरचना dirent *d;
	पूर्णाङ्क r = 0;

	dir = ul_path_opendir(pc, रिक्त);
	यदि (!dir)
		फर्काऊ 0;

	जबसम्म ((d = xreaddir(dir))) {
		यदि (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	बंद_सूची(dir);
	फर्काऊ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(संरचना path_cxt *pc, पूर्णाङ्क partno)
{
	सूची *dir;
	संरचना dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, रिक्त);
	यदि (!dir)
		फर्काऊ 0;

	जबसम्म ((d = xreaddir(dir))) {
		पूर्णाङ्क n;

		यदि (!sysfs_blkdev_is_partition_dirent(dir, d, रिक्त))
			जारी;

		यदि (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			जारी;

		यदि (n == partno) {
			यदि (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				रोक;
		}
	}

	बंद_सूची(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (पूर्णाङ्क) partno, (पूर्णाङ्क) devno));
	फर्काऊ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
वर्ण *sysfs_blkdev_get_slave(संरचना path_cxt *pc)
{
	सूची *dir;
	संरचना dirent *d;
	वर्ण *name = रिक्त;

	dir = ul_path_opendir(pc, "slaves");
	यदि (!dir)
		फर्काऊ रिक्त;

	जबसम्म ((d = xreaddir(dir))) {
		यदि (name)
			गच्छ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	बंद_सूची(dir);
	फर्काऊ name;
err:
	मुक्त(name);
	बंद_सूची(dir);
	फर्काऊ रिक्त;
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
स्थिर वर्ण *get_subsystem(वर्ण *chain, वर्ण *buf, माप_प्रकार bufsz)
{
	माप_प्रकार len;
	वर्ण *p;

	यदि (!chain || !*chain)
		फर्काऊ रिक्त;

	len = म_माप(chain);
	यदि (len + परिमाण(SUBSYSTEM_LINKNAME) > PATH_MAX)
		फर्काऊ रिक्त;

	गर {
		ssize_t sz;

		/* append "/subsystem" to the path */
		स_नकल(chain + len, SUBSYSTEM_LINKNAME, परिमाण(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		यदि (p) {
			*p = '\0';
			len = p - chain;
		}

		यदि (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			फर्काऊ basename(buf);
		}

	} जबसम्म (p);

	फर्काऊ रिक्त;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
वर्ण *sysfs_blkdev_get_devchain(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	माप_प्रकार sz = 0;
	संरचना ul_buffer tmp = UL_INIT_BUFFER;
	अचर वर्ण *p;
	वर्ण *res = रिक्त;

	ssz = ul_path_readlink(pc, buf, bufsz, रिक्त);
	यदि (ssz <= 0)
		फर्काऊ रिक्त;

	यदि ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, रिक्त);
	यदि (p && sz <= bufsz) {
		स_नकल(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	फर्काऊ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
पूर्णाङ्क sysfs_blkdev_next_subsystem(संरचना path_cxt *pc __attribute__((unused)),
			 वर्ण *devchain, वर्ण **subsys)
{
	वर्ण subbuf[PATH_MAX];
	वर्ण *sub;

	यदि (!subsys || !devchain)
		फर्काऊ -EINVAL;

	*subsys = रिक्त;

	जबसम्म ((sub = get_subsystem(devchain, subbuf, परिमाण(subbuf)))) {
		*subsys = strdup(sub);
		यदि (!*subsys)
			फर्काऊ -ENOMEM;
		फर्काऊ 0;
	}

	फर्काऊ 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
स्थिर पूर्णाङ्क sysfs_devchain_is_removable(वर्ण *chain)
{
	माप_प्रकार len;
	वर्ण buf[20];
	वर्ण *p;

	यदि (!chain || !*chain)
		फर्काऊ 0;

	len = म_माप(chain);
	यदि (len + परिमाण(REMOVABLE_FILENAME) > PATH_MAX)
		फर्काऊ 0;

	गर {
		पूर्णाङ्क fd, rc;

		/* append "/removable" to the path */
		स_नकल(chain + len, REMOVABLE_FILENAME, परिमाण(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		यदि (म_भेद(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			रोक;

		/* try to read it */
		fd = खोलो(chain, O_RDONLY | O_CLOEXEC);
		यदि (fd != -1) {
			rc = ul_read_all(fd, buf, परिमाण(buf));
			बंद(fd);

			यदि (rc > 0) {
				यदि (म_भेदन(buf, "fixed", min(rc, 5)) == 0) {
					फर्काऊ 0;
				} अन्यथा यदि (म_भेदन(buf, "removable", min(rc, 9)) == 0) {
					फर्काऊ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		यदि (p) {
			*p = '\0';
			len = p - chain;
		}

	} जबसम्म (p);

	फर्काऊ 0;
}

पूर्णाङ्क sysfs_blkdev_is_hotpluggable(संरचना path_cxt *pc)
{
	वर्ण buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, परिमाण(buf));
	फर्काऊ sysfs_devchain_is_removable(chain);
}

पूर्णाङ्क sysfs_blkdev_is_removable(संरचना path_cxt *pc)
{
	पूर्णाङ्क rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	यदि (ul_path_read_s32(pc, &rc, "removable") == 0)
		फर्काऊ rc;

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क get_dm_wholedisk(संरचना path_cxt *pc, वर्ण *diskname,
                            माप_प्रकार len, dev_t *diskdevno)
{
	पूर्णाङ्क rc = 0;
	वर्ण *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	यदि (!name)
		फर्काऊ -1;

	यदि (diskname && len)
		xstrncpy(diskname, name, len);

	यदि (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, रिक्त);
		यदि (!*diskdevno)
			rc = -1;
	}

	मुक्त(name);
	फर्काऊ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
पूर्णाङ्क sysfs_blkdev_get_wholedisk(	संरचना path_cxt *pc,
				वर्ण *diskname,
				माप_प्रकार len,
				dev_t *diskdevno)
{
	पूर्णाङ्क is_part = 0;

	यदि (!pc)
		फर्काऊ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	यदि (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		वर्ण *uuid = रिक्त, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : रिक्त;

		यदि (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		मुक्त(uuid);

		यदि (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			गच्छ done;

		is_part = 0;
	}

	यदि (!is_part) {
		/*
		 * unpartitioned device
		 */
		यदि (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			गच्छ err;
		यदि (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} अन्यथा {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		वर्ण linkpath[PATH_MAX];
		वर्ण *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, परिमाण(linkpath), रिक्त);
		यदि (linklen < 0)
			गच्छ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		यदि (!name)
			गच्छ err;

		sysfs_devname_sys_to_dev(name);
		यदि (diskname && len)
			xstrncpy(diskname, name, len);

		यदि (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, रिक्त);
			यदि (!*diskdevno)
				गच्छ err;
		}
	}

done:
	फर्काऊ 0;
err:
	फर्काऊ -1;
}

पूर्णाङ्क sysfs_devno_to_wholedisk(dev_t devno, वर्ण *diskname,
		             माप_प्रकार len, dev_t *diskdevno)
{
	संरचना path_cxt *pc;
	पूर्णाङ्क rc = 0;

	यदि (!devno)
		फर्काऊ -EINVAL;
	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	यदि (!pc)
		फर्काऊ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	फर्काऊ rc;
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
पूर्णाङ्क sysfs_devno_is_dm_hidden(dev_t devno, वर्ण **uuid,
			     अचर संरचना ul_vfs_ops *vfs)
{
	संरचना path_cxt *pc = रिक्त;
	वर्ण *id = रिक्त;
	पूर्णाङ्क rc = 0;

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	यदि (!pc)
		गच्छ done;
	ul_path_refer_vfs(pc, vfs);
	यदि (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		गच्छ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	यदि (म_भेदन(id, "LVM-", 4) == 0) {
		वर्ण *p = म_खोजप(id + 4, '-');

		यदि (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	यदि (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	फर्काऊ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
पूर्णाङ्क sysfs_devno_is_dm_private(dev_t devno, वर्ण **uuid,
			      अचर संरचना ul_vfs_ops *vfs)
{
	संरचना path_cxt *pc = रिक्त;
	वर्ण *id = रिक्त;
	पूर्णाङ्क rc = 0;

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	यदि (!pc)
		गच्छ done;
	ul_path_refer_vfs(pc, vfs);
	यदि (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		गच्छ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	यदि (म_भेदन(id, "LVM-", 4) == 0) {
		वर्ण *p = म_खोजप(id + 4, '-');

		यदि (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} अन्यथा यदि (म_भेदन(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	यदि (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	फर्काऊ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
पूर्णाङ्क sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	यदि (sysfs_devno_to_wholedisk(devno, रिक्त, 0, &disk) != 0)
		फर्काऊ -1;

	फर्काऊ devno == disk;
}


पूर्णाङ्क sysfs_blkdev_scsi_get_hctl(संरचना path_cxt *pc, पूर्णाङ्क *h, पूर्णाङ्क *c, पूर्णाङ्क *t, पूर्णाङ्क *l)
{
	वर्ण buf[PATH_MAX], *hctl;
	संरचना sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	यदि (!blk || blk->hctl_error)
		फर्काऊ -EINVAL;
	यदि (blk->has_hctl)
		गच्छ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, परिमाण(buf), "device");
	यदि (len < 0)
		फर्काऊ len;

	hctl = म_खोजप(buf, '/');
	यदि (!hctl)
		फर्काऊ -1;
	hctl++;

	यदि (माला_पूछो(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		फर्काऊ -1;

	blk->has_hctl = 1;
done:
	यदि (h)
		*h = blk->scsi_host;
	यदि (c)
		*c = blk->scsi_channel;
	यदि (t)
		*t = blk->scsi_target;
	यदि (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	फर्काऊ 0;
}


स्थिर वर्ण *scsi_host_attribute_path(
			संरचना path_cxt *pc,
			अचर वर्ण *type,
			वर्ण *buf,
			माप_प्रकार bufsz,
			अचर वर्ण *attr)
{
	पूर्णाङ्क len;
	पूर्णाङ्क host;
	अचर वर्ण *prefix;

	यदि (sysfs_blkdev_scsi_get_hctl(pc, &host, रिक्त, रिक्त, रिक्त))
		फर्काऊ रिक्त;

	prefix = ul_path_get_prefix(pc);
	यदि (!prefix)
		prefix = "";

	यदि (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	फर्काऊ (len < 0 || (माप_प्रकार) len >= bufsz) ? रिक्त : buf;
}

वर्ण *sysfs_blkdev_scsi_host_strdup_attribute(संरचना path_cxt *pc,
			अचर वर्ण *type, अचर वर्ण *attr)
{
	वर्ण buf[1024];
	पूर्णाङ्क rc;
	खाता *f;

	यदि (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, परिमाण(buf), attr))
		फर्काऊ रिक्त;

	यदि (!(f = ख_खोलो(buf, "r" UL_CLOEXECSTR)))
		फर्काऊ रिक्त;

	rc = ख_पूछो(f, "%1023[^\n]", buf);
	ख_बंद(f);

	फर्काऊ rc == 1 ? strdup(buf) : रिक्त;
}

पूर्णाङ्क sysfs_blkdev_scsi_host_is(संरचना path_cxt *pc, अचर वर्ण *type)
{
	वर्ण buf[PATH_MAX];
	संरचना stat st;

	यदि (!type || !scsi_host_attribute_path(pc, type,
				buf, परिमाण(buf), रिक्त))
		फर्काऊ 0;

	फर्काऊ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

स्थिर वर्ण *scsi_attribute_path(संरचना path_cxt *pc,
		वर्ण *buf, माप_प्रकार bufsz, अचर वर्ण *attr)
{
	पूर्णाङ्क len, h, c, t, l;
	अचर वर्ण *prefix;

	यदि (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		फर्काऊ रिक्त;

	prefix = ul_path_get_prefix(pc);
	यदि (!prefix)
		prefix = "";

	यदि (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	फर्काऊ (len < 0 || (माप_प्रकार) len >= bufsz) ? रिक्त : buf;
}

पूर्णाङ्क sysfs_blkdev_scsi_has_attribute(संरचना path_cxt *pc, अचर वर्ण *attr)
{
	वर्ण path[PATH_MAX];
	संरचना stat st;

	यदि (!scsi_attribute_path(pc, path, परिमाण(path), attr))
		फर्काऊ 0;

	फर्काऊ stat(path, &st) == 0;
}

पूर्णाङ्क sysfs_blkdev_scsi_path_contains(संरचना path_cxt *pc, अचर वर्ण *pattern)
{
	वर्ण path[PATH_MAX], linkc[PATH_MAX];
	संरचना stat st;
	ssize_t len;

	यदि (!scsi_attribute_path(pc, path, परिमाण(path), रिक्त))
		फर्काऊ 0;

	यदि (stat(path, &st) != 0)
		फर्काऊ 0;

	len = readlink(path, linkc, परिमाण(linkc) - 1);
	यदि (len < 0)
		फर्काऊ 0;

	linkc[len] = '\0';
	फर्काऊ म_माला(linkc, pattern) != रिक्त;
}

स्थिर dev_t read_devno(अचर वर्ण *path)
{
	खाता *f;
	पूर्णाङ्क maj = 0, min = 0;
	dev_t dev = 0;

	f = ख_खोलो(path, "r" UL_CLOEXECSTR);
	यदि (!f)
		फर्काऊ 0;

	यदि (ख_पूछो(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	ख_बंद(f);
	फर्काऊ dev;
}

पूर्णाङ्क sysfs_devname_is_hidden(अचर वर्ण *prefix, अचर वर्ण *name)
{
	वर्ण buf[PATH_MAX];
	पूर्णाङ्क rc = 0, hidden = 0, len;
	खाता *f;

	यदि (म_भेदन("/dev/", name, 5) == 0)
		फर्काऊ 0;

	यदि (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, परिमाण(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	यदि (len < 0 || (माप_प्रकार) len + 1 > परिमाण(buf))
		फर्काऊ 0;

	f = ख_खोलो(buf, "r" UL_CLOEXECSTR);
	यदि (!f)
		फर्काऊ 0;

	rc = ख_पूछो(f, "%d", &hidden);
	ख_बंद(f);

	फर्काऊ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(अचर वर्ण *prefix, अचर वर्ण *name, अचर वर्ण *parent)
{
	वर्ण buf[PATH_MAX];
	वर्ण *_name = रिक्त, *_parent = रिक्त;	/* name as encoded in sysfs */
	dev_t dev = 0;
	पूर्णाङ्क len;

	यदि (!prefix)
		prefix = "";

	प्रतिज्ञा(name);

	यदि (म_भेदन("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		संरचना stat st;

		यदि (stat(name, &st) == 0) {
			dev = st.st_rdev;
			गच्छ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	यदि (!_name)
		गच्छ done;
	sysfs_devname_dev_to_sys(_name);

	यदि (parent) {
		_parent = strdup(parent);
		यदि (!_parent)
			गच्छ done;
	}

	यदि (parent && म_भेदन("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, परिमाण(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		यदि (len < 0 || (माप_प्रकार) len >= परिमाण(buf))
			गच्छ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		गच्छ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, परिमाण(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	यदि (len < 0 || (माप_प्रकार) len >= परिमाण(buf))
		गच्छ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	यदि (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, परिमाण(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		यदि (len < 0 || (माप_प्रकार) len >= परिमाण(buf))
			गच्छ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	यदि (!dev) {
		len = snprintf(buf, परिमाण(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		यदि (len < 0 || (माप_प्रकार) len >= परिमाण(buf))
			गच्छ done;
		dev = read_devno(buf);
	}
done:
	मुक्त(_name);
	मुक्त(_parent);
	फर्काऊ dev;
}

dev_t sysfs_devname_to_devno(अचर वर्ण *name)
{
	फर्काऊ __sysfs_devname_to_devno(रिक्त, name, रिक्त);
}

वर्ण *sysfs_blkdev_get_path(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsiz)
{
	अचर वर्ण *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	वर्ण *res = रिक्त;
	माप_प्रकार sz;
	संरचना stat st;

	यदि (!name)
		गच्छ done;

	sz = म_माप(name);
	यदि (sz + परिमाण("/dev/") > bufsiz)
		गच्छ done;

	/* create the final "/dev/<name>" string */
	स_हटाओ(buf + 5, name, sz + 1);
	स_नकल(buf, "/dev/", 5);

	यदि (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	फर्काऊ res;
}

dev_t sysfs_blkdev_get_devno(संरचना path_cxt *pc)
{
	फर्काऊ ((संरचना sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
वर्ण *sysfs_devno_to_devpath(dev_t devno, वर्ण *buf, माप_प्रकार bufsiz)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	वर्ण *res = रिक्त;

	यदि (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	फर्काऊ res;
}

वर्ण *sysfs_devno_to_devname(dev_t devno, वर्ण *buf, माप_प्रकार bufsiz)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	वर्ण *res = रिक्त;

	यदि (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	फर्काऊ res;
}

पूर्णाङ्क sysfs_devno_गन्ती_partitions(dev_t devno)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	पूर्णाङ्क n = 0;

	यदि (pc) {
		वर्ण buf[PATH_MAX + 1];
		वर्ण *name = sysfs_blkdev_get_name(pc, buf, परिमाण(buf));

		n = sysfs_blkdev_गन्ती_partitions(pc, name);
		ul_unref_path(pc);
	}
	फर्काऊ n;
}

वर्ण *sysfs_chrdev_devno_to_devname(dev_t devno, वर्ण *buf, माप_प्रकार bufsiz)
{
	वर्ण link[PATH_MAX];
	संरचना path_cxt *pc;
	वर्ण *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	यदि (!pc)
		फर्काऊ रिक्त;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, परिमाण(link), रिक्त);
	ul_unref_path(pc);

	यदि (sz < 0)
		फर्काऊ रिक्त;

	name = म_खोजप(link, '/');
	यदि (!name)
		फर्काऊ रिक्त;

	name++;
	sz = म_माप(name);
	यदि ((माप_प्रकार) sz + 1 > bufsiz)
		फर्काऊ रिक्त;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	फर्काऊ buf;

}

गणना sysfs_byteorder sysfs_get_byteorder(संरचना path_cxt *pc)
{
	पूर्णाङ्क rc;
	वर्ण buf[बफ_मान];
	गणना sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, परिमाण(buf), _PATH_SYS_CPU_BYTEORDER);
	यदि (rc < 0)
		गच्छ unknown;

	यदि (म_भेदन(buf, "little", परिमाण(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		गच्छ out;
	} अन्यथा यदि (म_भेदन(buf, "big", परिमाण(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		गच्छ out;
	}

unknown:
#यदि __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#अथवायदि __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#अन्यथा
#error Unknown byte order
#endif

out:
	फर्काऊ ret;
}

पूर्णाङ्क sysfs_get_address_bits(संरचना path_cxt *pc)
{
	पूर्णाङ्क rc;
	पूर्णाङ्क address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	यदि (rc < 0)
		फर्काऊ rc;
	यदि (address_bits < 0)
		फर्काऊ -EINVAL;
	फर्काऊ address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdlib.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना path_cxt *pc;
	वर्ण *devname;
	dev_t devno, disk_devno;
	वर्ण path[PATH_MAX], *sub, *chain;
	वर्ण diskname[32];
	पूर्णाङ्क i, is_part, rc = निकास_सफल;
	uint64_t u64;

	यदि (argc != 2)
		errx(निकास_त्रुटि, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	यदि (!devno)
		err(निकास_त्रुटि, "failed to read devno");

	म_लिखो("non-context:\n");
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णाङ्क) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, परिमाण(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, परिमाण(path)));

	sysfs_devno_to_wholedisk(devno, diskname, परिमाण(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णाङ्क) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	यदि (!pc)
		गच्छ done;

	म_लिखो("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णाङ्क) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, परिमाण(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, परिमाण(path)));

	sysfs_devno_to_wholedisk(devno, diskname, परिमाण(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (अचिन्हित पूर्णाङ्क) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	म_लिखो(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	यदि (is_part && disk_devno) {
		संरचना path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, रिक्त, रिक्त);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	म_लिखो(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	म_लिखो(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	म_लिखो(" SLAVES: %d\n", ul_path_गन्ती_dirents(pc, "slaves"));

	यदि (!is_part) {
		म_लिखो("First 5 partitions:\n");
		लागि (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			यदि (dev)
				म_लिखो("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	यदि (ul_path_read_u64(pc, &u64, "size") != 0)
		म_लिखो(" (!) read SIZE failed\n");
	अन्यथा
		म_लिखो(" SIZE: %"PRIu64"\n", u64);

	यदि (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		म_लिखो(" (!) read SECTOR failed\n");
	अन्यथा
		म_लिखो(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, परिमाण(path));
	म_लिखो(" SUBSYSTEMS:\n");

	जबसम्म (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		म_लिखो("\t%s\n", sub);
		मुक्त(sub);
	}

	rc = निकास_सफल;
done:
	ul_unref_path(pc);
	फर्काऊ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
