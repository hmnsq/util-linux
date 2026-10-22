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
स्थिर पूर्णांक  sysfs_blkdev_enoent_redirect(संरचना path_cxt *pc, स्थिरांक अक्षर *path, पूर्णांक *dirfd);

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
	जर (ulsysfs_debug_mask)
		परत;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

संरचना path_cxt *ul_new_sysfs_path(dev_t devno, संरचना path_cxt *parent, स्थिरांक अक्षर *prefix)
{
	संरचना path_cxt *pc = ul_new_path(रिक्त);

	जर (!pc)
		परत रिक्त;
	जर (prefix)
		ul_path_set_prefix(pc, prefix);

	जर (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		परत रिक्त;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	परत pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णांक sysfs_blkdev_init_path(संरचना path_cxt *pc, dev_t devno, संरचना path_cxt *parent)
{
	संरचना sysfs_blkdev *blk;
	पूर्णांक rc;
	अक्षर buf[आकार(_PATH_SYS_DEVBLOCK)
		 + आकार(stringify_मूल्य(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, आकार(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	जर (rc)
		परत rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	जर (rc < 0)
		परत rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	जर (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = सुस्मृति(1, आकार(संरचना sysfs_blkdev));
		जर (!blk)
			परत -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	परत 0;
}

स्थिर शून्य sysfs_blkdev_deinit_path(संरचना path_cxt *pc)
{
	संरचना sysfs_blkdev *blk;

	जर (!pc)
		परत;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	जर (!blk)
		परत;

	ul_unref_path(blk->parent);
	मुक्त(blk);

	ul_path_set_dialect(pc, रिक्त, रिक्त);
}

पूर्णांक sysfs_blkdev_set_parent(संरचना path_cxt *pc, संरचना path_cxt *parent)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);

	जर (!pc || !blk)
		परत -EINVAL;

	जर (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = रिक्त;
	}

	जर (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} अन्यथा
		blk->parent = रिक्त;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	परत 0;
}

संरचना path_cxt *sysfs_blkdev_get_parent(संरचना path_cxt *pc)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);
	परत blk ? blk->parent : रिक्त;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
स्थिर पूर्णांक sysfs_blkdev_enoent_redirect(संरचना path_cxt *pc, स्थिरांक अक्षर *path, पूर्णांक *dirfd)
{
	संरचना sysfs_blkdev *blk = ul_path_get_dialect(pc);

	जर (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		जर (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			परत 0;
		}
	}
	परत 1;	/* no redirect */
}

अक्षर *sysfs_blkdev_get_name(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz)
{
	अक्षर link[PATH_MAX];
	अक्षर *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, आकार(link), रिक्त);
	जर (sz < 0)
		परत रिक्त;

	name = म_खोजप(link, '/');
	जर (!name)
		परत रिक्त;

	name++;
	sz = म_माप(name);
	जर ((माप_प्रकार) sz + 1 > bufsiz)
		परत रिक्त;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	परत buf;
}

पूर्णांक sysfs_blkdev_is_partition_dirent(सूची *dir, संरचना dirent *d, स्थिरांक अक्षर *parent_name)
{
	अक्षर path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	जर (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		परत 0;
#endif
	माप_प्रकार len = 0;

	जर (parent_name) {
		स्थिरांक अक्षर *p = parent_name;

		/* /dev/sda --> "sda" */
		जर (*parent_name == '/') {
			p = म_खोजप(parent_name, '/');
			जर (!p)
				परत 0;
			p++;
		}

		len = म_माप(p);
		जर ((म_माप(d->d_name) <= len) || (म_भेदन(p, d->d_name, len) != 0))
			len = 0;
	}

	जर (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		परत ((*(d->d_name + len) == 'p' && है_अंक(*(d->d_name + len + 1)))
			|| है_अंक(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, आकार(path), "%s/start", d->d_name);

	परत faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

पूर्णांक sysfs_blkdev_मोजणी_partitions(संरचना path_cxt *pc, स्थिरांक अक्षर *devname)
{
	सूची *dir;
	संरचना dirent *d;
	पूर्णांक r = 0;

	dir = ul_path_opendir(pc, रिक्त);
	जर (!dir)
		परत 0;

	पर्यंत ((d = xreaddir(dir))) {
		जर (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	बंद_सूची(dir);
	परत r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(संरचना path_cxt *pc, पूर्णांक partno)
{
	सूची *dir;
	संरचना dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, रिक्त);
	जर (!dir)
		परत 0;

	पर्यंत ((d = xreaddir(dir))) {
		पूर्णांक n;

		जर (!sysfs_blkdev_is_partition_dirent(dir, d, रिक्त))
			सुरूठेवा;

		जर (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			सुरूठेवा;

		जर (n == partno) {
			जर (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				थांबा;
		}
	}

	बंद_सूची(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (पूर्णांक) partno, (पूर्णांक) devno));
	परत devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
अक्षर *sysfs_blkdev_get_slave(संरचना path_cxt *pc)
{
	सूची *dir;
	संरचना dirent *d;
	अक्षर *name = रिक्त;

	dir = ul_path_opendir(pc, "slaves");
	जर (!dir)
		परत रिक्त;

	पर्यंत ((d = xreaddir(dir))) {
		जर (name)
			जा err;	/* more slaves */
		name = strdup(d->d_name);
	}

	बंद_सूची(dir);
	परत name;
err:
	मुक्त(name);
	बंद_सूची(dir);
	परत रिक्त;
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
स्थिर अक्षर *get_subsystem(अक्षर *chain, अक्षर *buf, माप_प्रकार bufsz)
{
	माप_प्रकार len;
	अक्षर *p;

	जर (!chain || !*chain)
		परत रिक्त;

	len = म_माप(chain);
	जर (len + आकार(SUBSYSTEM_LINKNAME) > PATH_MAX)
		परत रिक्त;

	करा {
		ssize_t sz;

		/* append "/subsystem" to the path */
		स_नकल(chain + len, SUBSYSTEM_LINKNAME, आकार(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		जर (p) {
			*p = '\0';
			len = p - chain;
		}

		जर (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			परत basename(buf);
		}

	} पर्यंत (p);

	परत रिक्त;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
अक्षर *sysfs_blkdev_get_devchain(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	माप_प्रकार sz = 0;
	संरचना ul_buffer tmp = UL_INIT_BUFFER;
	स्थिरांक अक्षर *p;
	अक्षर *res = रिक्त;

	ssz = ul_path_readlink(pc, buf, bufsz, रिक्त);
	जर (ssz <= 0)
		परत रिक्त;

	जर ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, रिक्त);
	जर (p && sz <= bufsz) {
		स_नकल(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	परत res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
पूर्णांक sysfs_blkdev_next_subsystem(संरचना path_cxt *pc __attribute__((unused)),
			 अक्षर *devchain, अक्षर **subsys)
{
	अक्षर subbuf[PATH_MAX];
	अक्षर *sub;

	जर (!subsys || !devchain)
		परत -EINVAL;

	*subsys = रिक्त;

	पर्यंत ((sub = get_subsystem(devchain, subbuf, आकार(subbuf)))) {
		*subsys = strdup(sub);
		जर (!*subsys)
			परत -ENOMEM;
		परत 0;
	}

	परत 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
स्थिर पूर्णांक sysfs_devchain_is_removable(अक्षर *chain)
{
	माप_प्रकार len;
	अक्षर buf[20];
	अक्षर *p;

	जर (!chain || !*chain)
		परत 0;

	len = म_माप(chain);
	जर (len + आकार(REMOVABLE_FILENAME) > PATH_MAX)
		परत 0;

	करा {
		पूर्णांक fd, rc;

		/* append "/removable" to the path */
		स_नकल(chain + len, REMOVABLE_FILENAME, आकार(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		जर (म_भेद(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			थांबा;

		/* try to read it */
		fd = खोलो(chain, O_RDONLY | O_CLOEXEC);
		जर (fd != -1) {
			rc = ul_read_all(fd, buf, आकार(buf));
			बंद(fd);

			जर (rc > 0) {
				जर (म_भेदन(buf, "fixed", min(rc, 5)) == 0) {
					परत 0;
				} अन्यथा जर (म_भेदन(buf, "removable", min(rc, 9)) == 0) {
					परत 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		जर (p) {
			*p = '\0';
			len = p - chain;
		}

	} पर्यंत (p);

	परत 0;
}

पूर्णांक sysfs_blkdev_is_hotpluggable(संरचना path_cxt *pc)
{
	अक्षर buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, आकार(buf));
	परत sysfs_devchain_is_removable(chain);
}

पूर्णांक sysfs_blkdev_is_removable(संरचना path_cxt *pc)
{
	पूर्णांक rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	जर (ul_path_read_s32(pc, &rc, "removable") == 0)
		परत rc;

	परत 0;
}

स्थिर पूर्णांक get_dm_wholedisk(संरचना path_cxt *pc, अक्षर *diskname,
                            माप_प्रकार len, dev_t *diskdevno)
{
	पूर्णांक rc = 0;
	अक्षर *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	जर (!name)
		परत -1;

	जर (diskname && len)
		xstrncpy(diskname, name, len);

	जर (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, रिक्त);
		जर (!*diskdevno)
			rc = -1;
	}

	मुक्त(name);
	परत rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
पूर्णांक sysfs_blkdev_get_wholedisk(	संरचना path_cxt *pc,
				अक्षर *diskname,
				माप_प्रकार len,
				dev_t *diskdevno)
{
	पूर्णांक is_part = 0;

	जर (!pc)
		परत -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	जर (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		अक्षर *uuid = रिक्त, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : रिक्त;

		जर (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		मुक्त(uuid);

		जर (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			जा done;

		is_part = 0;
	}

	जर (!is_part) {
		/*
		 * unpartitioned device
		 */
		जर (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			जा err;
		जर (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} अन्यथा {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		अक्षर linkpath[PATH_MAX];
		अक्षर *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, आकार(linkpath), रिक्त);
		जर (linklen < 0)
			जा err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		जर (!name)
			जा err;

		sysfs_devname_sys_to_dev(name);
		जर (diskname && len)
			xstrncpy(diskname, name, len);

		जर (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, रिक्त);
			जर (!*diskdevno)
				जा err;
		}
	}

done:
	परत 0;
err:
	परत -1;
}

पूर्णांक sysfs_devno_to_wholedisk(dev_t devno, अक्षर *diskname,
		             माप_प्रकार len, dev_t *diskdevno)
{
	संरचना path_cxt *pc;
	पूर्णांक rc = 0;

	जर (!devno)
		परत -EINVAL;
	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	जर (!pc)
		परत -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	परत rc;
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
पूर्णांक sysfs_devno_is_dm_hidden(dev_t devno, अक्षर **uuid,
			     स्थिरांक संरचना ul_vfs_ops *vfs)
{
	संरचना path_cxt *pc = रिक्त;
	अक्षर *id = रिक्त;
	पूर्णांक rc = 0;

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	जर (!pc)
		जा done;
	ul_path_refer_vfs(pc, vfs);
	जर (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		जा done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	जर (म_भेदन(id, "LVM-", 4) == 0) {
		अक्षर *p = म_खोजप(id + 4, '-');

		जर (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	जर (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	परत rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
पूर्णांक sysfs_devno_is_dm_private(dev_t devno, अक्षर **uuid,
			      स्थिरांक संरचना ul_vfs_ops *vfs)
{
	संरचना path_cxt *pc = रिक्त;
	अक्षर *id = रिक्त;
	पूर्णांक rc = 0;

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	जर (!pc)
		जा done;
	ul_path_refer_vfs(pc, vfs);
	जर (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		जा done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	जर (म_भेदन(id, "LVM-", 4) == 0) {
		अक्षर *p = म_खोजप(id + 4, '-');

		जर (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} अन्यथा जर (म_भेदन(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	जर (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	परत rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
पूर्णांक sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	जर (sysfs_devno_to_wholedisk(devno, रिक्त, 0, &disk) != 0)
		परत -1;

	परत devno == disk;
}


पूर्णांक sysfs_blkdev_scsi_get_hctl(संरचना path_cxt *pc, पूर्णांक *h, पूर्णांक *c, पूर्णांक *t, पूर्णांक *l)
{
	अक्षर buf[PATH_MAX], *hctl;
	संरचना sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	जर (!blk || blk->hctl_error)
		परत -EINVAL;
	जर (blk->has_hctl)
		जा done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, आकार(buf), "device");
	जर (len < 0)
		परत len;

	hctl = म_खोजप(buf, '/');
	जर (!hctl)
		परत -1;
	hctl++;

	जर (माला_पूछो(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		परत -1;

	blk->has_hctl = 1;
done:
	जर (h)
		*h = blk->scsi_host;
	जर (c)
		*c = blk->scsi_channel;
	जर (t)
		*t = blk->scsi_target;
	जर (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	परत 0;
}


स्थिर अक्षर *scsi_host_attribute_path(
			संरचना path_cxt *pc,
			स्थिरांक अक्षर *type,
			अक्षर *buf,
			माप_प्रकार bufsz,
			स्थिरांक अक्षर *attr)
{
	पूर्णांक len;
	पूर्णांक host;
	स्थिरांक अक्षर *prefix;

	जर (sysfs_blkdev_scsi_get_hctl(pc, &host, रिक्त, रिक्त, रिक्त))
		परत रिक्त;

	prefix = ul_path_get_prefix(pc);
	जर (!prefix)
		prefix = "";

	जर (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	परत (len < 0 || (माप_प्रकार) len >= bufsz) ? रिक्त : buf;
}

अक्षर *sysfs_blkdev_scsi_host_strdup_attribute(संरचना path_cxt *pc,
			स्थिरांक अक्षर *type, स्थिरांक अक्षर *attr)
{
	अक्षर buf[1024];
	पूर्णांक rc;
	खाता *f;

	जर (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, आकार(buf), attr))
		परत रिक्त;

	जर (!(f = ख_खोलो(buf, "r" UL_CLOEXECSTR)))
		परत रिक्त;

	rc = ख_पूछो(f, "%1023[^\n]", buf);
	ख_बंद(f);

	परत rc == 1 ? strdup(buf) : रिक्त;
}

पूर्णांक sysfs_blkdev_scsi_host_is(संरचना path_cxt *pc, स्थिरांक अक्षर *type)
{
	अक्षर buf[PATH_MAX];
	संरचना stat st;

	जर (!type || !scsi_host_attribute_path(pc, type,
				buf, आकार(buf), रिक्त))
		परत 0;

	परत stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

स्थिर अक्षर *scsi_attribute_path(संरचना path_cxt *pc,
		अक्षर *buf, माप_प्रकार bufsz, स्थिरांक अक्षर *attr)
{
	पूर्णांक len, h, c, t, l;
	स्थिरांक अक्षर *prefix;

	जर (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		परत रिक्त;

	prefix = ul_path_get_prefix(pc);
	जर (!prefix)
		prefix = "";

	जर (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	परत (len < 0 || (माप_प्रकार) len >= bufsz) ? रिक्त : buf;
}

पूर्णांक sysfs_blkdev_scsi_has_attribute(संरचना path_cxt *pc, स्थिरांक अक्षर *attr)
{
	अक्षर path[PATH_MAX];
	संरचना stat st;

	जर (!scsi_attribute_path(pc, path, आकार(path), attr))
		परत 0;

	परत stat(path, &st) == 0;
}

पूर्णांक sysfs_blkdev_scsi_path_contains(संरचना path_cxt *pc, स्थिरांक अक्षर *pattern)
{
	अक्षर path[PATH_MAX], linkc[PATH_MAX];
	संरचना stat st;
	ssize_t len;

	जर (!scsi_attribute_path(pc, path, आकार(path), रिक्त))
		परत 0;

	जर (stat(path, &st) != 0)
		परत 0;

	len = readlink(path, linkc, आकार(linkc) - 1);
	जर (len < 0)
		परत 0;

	linkc[len] = '\0';
	परत म_माला(linkc, pattern) != रिक्त;
}

स्थिर dev_t read_devno(स्थिरांक अक्षर *path)
{
	खाता *f;
	पूर्णांक maj = 0, min = 0;
	dev_t dev = 0;

	f = ख_खोलो(path, "r" UL_CLOEXECSTR);
	जर (!f)
		परत 0;

	जर (ख_पूछो(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	ख_बंद(f);
	परत dev;
}

पूर्णांक sysfs_devname_is_hidden(स्थिरांक अक्षर *prefix, स्थिरांक अक्षर *name)
{
	अक्षर buf[PATH_MAX];
	पूर्णांक rc = 0, hidden = 0, len;
	खाता *f;

	जर (म_भेदन("/dev/", name, 5) == 0)
		परत 0;

	जर (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, आकार(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	जर (len < 0 || (माप_प्रकार) len + 1 > आकार(buf))
		परत 0;

	f = ख_खोलो(buf, "r" UL_CLOEXECSTR);
	जर (!f)
		परत 0;

	rc = ख_पूछो(f, "%d", &hidden);
	ख_बंद(f);

	परत rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(स्थिरांक अक्षर *prefix, स्थिरांक अक्षर *name, स्थिरांक अक्षर *parent)
{
	अक्षर buf[PATH_MAX];
	अक्षर *_name = रिक्त, *_parent = रिक्त;	/* name as encoded in sysfs */
	dev_t dev = 0;
	पूर्णांक len;

	जर (!prefix)
		prefix = "";

	प्रतिज्ञा(name);

	जर (म_भेदन("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		संरचना stat st;

		जर (stat(name, &st) == 0) {
			dev = st.st_rdev;
			जा done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	जर (!_name)
		जा done;
	sysfs_devname_dev_to_sys(_name);

	जर (parent) {
		_parent = strdup(parent);
		जर (!_parent)
			जा done;
	}

	जर (parent && म_भेदन("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, आकार(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
			जा done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		जा done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, आकार(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
		जा done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	जर (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, आकार(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
			जा done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	जर (!dev) {
		len = snprintf(buf, आकार(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
			जा done;
		dev = read_devno(buf);
	}
done:
	मुक्त(_name);
	मुक्त(_parent);
	परत dev;
}

dev_t sysfs_devname_to_devno(स्थिरांक अक्षर *name)
{
	परत __sysfs_devname_to_devno(रिक्त, name, रिक्त);
}

अक्षर *sysfs_blkdev_get_path(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz)
{
	स्थिरांक अक्षर *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	अक्षर *res = रिक्त;
	माप_प्रकार sz;
	संरचना stat st;

	जर (!name)
		जा done;

	sz = म_माप(name);
	जर (sz + आकार("/dev/") > bufsiz)
		जा done;

	/* create the final "/dev/<name>" string */
	स_हटाओ(buf + 5, name, sz + 1);
	स_नकल(buf, "/dev/", 5);

	जर (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	परत res;
}

dev_t sysfs_blkdev_get_devno(संरचना path_cxt *pc)
{
	परत ((संरचना sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
अक्षर *sysfs_devno_to_devpath(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	अक्षर *res = रिक्त;

	जर (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	परत res;
}

अक्षर *sysfs_devno_to_devname(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	अक्षर *res = रिक्त;

	जर (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	परत res;
}

पूर्णांक sysfs_devno_मोजणी_partitions(dev_t devno)
{
	संरचना path_cxt *pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	पूर्णांक n = 0;

	जर (pc) {
		अक्षर buf[PATH_MAX + 1];
		अक्षर *name = sysfs_blkdev_get_name(pc, buf, आकार(buf));

		n = sysfs_blkdev_मोजणी_partitions(pc, name);
		ul_unref_path(pc);
	}
	परत n;
}

अक्षर *sysfs_chrdev_devno_to_devname(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	अक्षर link[PATH_MAX];
	संरचना path_cxt *pc;
	अक्षर *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	जर (!pc)
		परत रिक्त;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, आकार(link), रिक्त);
	ul_unref_path(pc);

	जर (sz < 0)
		परत रिक्त;

	name = म_खोजप(link, '/');
	जर (!name)
		परत रिक्त;

	name++;
	sz = म_माप(name);
	जर ((माप_प्रकार) sz + 1 > bufsiz)
		परत रिक्त;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	परत buf;

}

गणना sysfs_byteorder sysfs_get_byteorder(संरचना path_cxt *pc)
{
	पूर्णांक rc;
	अक्षर buf[बफ_मान];
	गणना sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, आकार(buf), _PATH_SYS_CPU_BYTEORDER);
	जर (rc < 0)
		जा unknown;

	जर (म_भेदन(buf, "little", आकार(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		जा out;
	} अन्यथा जर (म_भेदन(buf, "big", आकार(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		जा out;
	}

unknown:
#जर __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#किंवाजर __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#अन्यथा
#error Unknown byte order
#endif

out:
	परत ret;
}

पूर्णांक sysfs_get_address_bits(संरचना path_cxt *pc)
{
	पूर्णांक rc;
	पूर्णांक address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	जर (rc < 0)
		परत rc;
	जर (address_bits < 0)
		परत -EINVAL;
	परत address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdlib.h>

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	संरचना path_cxt *pc;
	अक्षर *devname;
	dev_t devno, disk_devno;
	अक्षर path[PATH_MAX], *sub, *chain;
	अक्षर diskname[32];
	पूर्णांक i, is_part, rc = निकास_सफल;
	uint64_t u64;

	जर (argc != 2)
		errx(निकास_त्रुटि, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	जर (!devno)
		err(निकास_त्रुटि, "failed to read devno");

	म_लिखो("non-context:\n");
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, आकार(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, आकार(path)));

	sysfs_devno_to_wholedisk(devno, diskname, आकार(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, रिक्त, रिक्त);
	जर (!pc)
		जा done;

	म_लिखो("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, आकार(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, आकार(path)));

	sysfs_devno_to_wholedisk(devno, diskname, आकार(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (अचिन्हित पूर्णांक) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	म_लिखो(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	जर (is_part && disk_devno) {
		संरचना path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, रिक्त, रिक्त);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	म_लिखो(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	म_लिखो(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	म_लिखो(" SLAVES: %d\n", ul_path_मोजणी_dirents(pc, "slaves"));

	जर (!is_part) {
		म_लिखो("First 5 partitions:\n");
		साठी (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			जर (dev)
				म_लिखो("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	जर (ul_path_read_u64(pc, &u64, "size") != 0)
		म_लिखो(" (!) read SIZE failed\n");
	अन्यथा
		म_लिखो(" SIZE: %"PRIu64"\n", u64);

	जर (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		म_लिखो(" (!) read SECTOR failed\n");
	अन्यथा
		म_लिखो(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, आकार(path));
	म_लिखो(" SUBSYSTEMS:\n");

	पर्यंत (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		म_लिखो("\t%s\n", sub);
		मुक्त(sub);
	}

	rc = निकास_सफल;
done:
	ul_unref_path(pc);
	परत rc;
}
#endif /* TEST_PROGRAM_SYSFS */
