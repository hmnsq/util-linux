/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#समावेश <ctype.h>
#समावेश <inttypes.h>
#समावेश <libgen.h>
#समावेश <fcntl.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "pathnames.h"
#समावेश "sysfs.h"
#समावेश "fileutils.h"
#समावेश "all-io.h"
#समावेश "debug.h"
#समावेश "strutils.h"
#समावेश "buffer.h"

अटल व्योम sysfs_blkdev_deinit_path(काष्ठा path_cxt *pc);
अटल पूर्णांक  sysfs_blkdev_enoent_redirect(काष्ठा path_cxt *pc, स्थिर अक्षर *path, पूर्णांक *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
अटल UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा ULSYSFS_DEBUG_INIT	(1 << 1)
#घोषणा ULSYSFS_DEBUG_CXT	(1 << 2)

#घोषणा DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

व्योम ul_sysfs_init_debug(व्योम)
{
	अगर (ulsysfs_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

काष्ठा path_cxt *ul_new_sysfs_path(dev_t devno, काष्ठा path_cxt *parent, स्थिर अक्षर *prefix)
{
	काष्ठा path_cxt *pc = ul_new_path(शून्य);

	अगर (!pc)
		वापस शून्य;
	अगर (prefix)
		ul_path_set_prefix(pc, prefix);

	अगर (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		वापस शून्य;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	वापस pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णांक sysfs_blkdev_init_path(काष्ठा path_cxt *pc, dev_t devno, काष्ठा path_cxt *parent)
{
	काष्ठा sysfs_blkdev *blk;
	पूर्णांक rc;
	अक्षर buf[माप(_PATH_SYS_DEVBLOCK)
		 + माप(stringify_मूल्य(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, माप(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	अगर (rc)
		वापस rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	अगर (rc < 0)
		वापस rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	अगर (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = सुस्मृति(1, माप(काष्ठा sysfs_blkdev));
		अगर (!blk)
			वापस -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	वापस 0;
}

अटल व्योम sysfs_blkdev_deinit_path(काष्ठा path_cxt *pc)
{
	काष्ठा sysfs_blkdev *blk;

	अगर (!pc)
		वापस;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	अगर (!blk)
		वापस;

	ul_unref_path(blk->parent);
	मुक्त(blk);

	ul_path_set_dialect(pc, शून्य, शून्य);
}

पूर्णांक sysfs_blkdev_set_parent(काष्ठा path_cxt *pc, काष्ठा path_cxt *parent)
{
	काष्ठा sysfs_blkdev *blk = ul_path_get_dialect(pc);

	अगर (!pc || !blk)
		वापस -EINVAL;

	अगर (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = शून्य;
	}

	अगर (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} अन्यथा
		blk->parent = शून्य;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	वापस 0;
}

काष्ठा path_cxt *sysfs_blkdev_get_parent(काष्ठा path_cxt *pc)
{
	काष्ठा sysfs_blkdev *blk = ul_path_get_dialect(pc);
	वापस blk ? blk->parent : शून्य;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
अटल पूर्णांक sysfs_blkdev_enoent_redirect(काष्ठा path_cxt *pc, स्थिर अक्षर *path, पूर्णांक *dirfd)
{
	काष्ठा sysfs_blkdev *blk = ul_path_get_dialect(pc);

	अगर (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		अगर (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			वापस 0;
		}
	}
	वापस 1;	/* no redirect */
}

अक्षर *sysfs_blkdev_get_name(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz)
{
	अक्षर link[PATH_MAX];
	अक्षर *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, माप(link), शून्य);
	अगर (sz < 0)
		वापस शून्य;

	name = म_खोजप(link, '/');
	अगर (!name)
		वापस शून्य;

	name++;
	sz = म_माप(name);
	अगर ((माप_प्रकार) sz + 1 > bufsiz)
		वापस शून्य;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	वापस buf;
}

पूर्णांक sysfs_blkdev_is_partition_dirent(सूची *dir, काष्ठा dirent *d, स्थिर अक्षर *parent_name)
{
	अक्षर path[NAME_MAX + 6 + 1];

#अगर_घोषित _DIRENT_HAVE_D_TYPE
	अगर (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		वापस 0;
#पूर्ण_अगर
	माप_प्रकार len = 0;

	अगर (parent_name) {
		स्थिर अक्षर *p = parent_name;

		/* /dev/sda --> "sda" */
		अगर (*parent_name == '/') {
			p = म_खोजप(parent_name, '/');
			अगर (!p)
				वापस 0;
			p++;
		}

		len = म_माप(p);
		अगर ((म_माप(d->d_name) <= len) || (म_भेदन(p, d->d_name, len) != 0))
			len = 0;
	}

	अगर (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		वापस ((*(d->d_name + len) == 'p' && है_अंक(*(d->d_name + len + 1)))
			|| है_अंक(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, माप(path), "%s/start", d->d_name);

	वापस faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

पूर्णांक sysfs_blkdev_गिनती_partitions(काष्ठा path_cxt *pc, स्थिर अक्षर *devname)
{
	सूची *dir;
	काष्ठा dirent *d;
	पूर्णांक r = 0;

	dir = ul_path_opendir(pc, शून्य);
	अगर (!dir)
		वापस 0;

	जबतक ((d = xreaddir(dir))) {
		अगर (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	बंद_सूची(dir);
	वापस r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(काष्ठा path_cxt *pc, पूर्णांक partno)
{
	सूची *dir;
	काष्ठा dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, शून्य);
	अगर (!dir)
		वापस 0;

	जबतक ((d = xreaddir(dir))) {
		पूर्णांक n;

		अगर (!sysfs_blkdev_is_partition_dirent(dir, d, शून्य))
			जारी;

		अगर (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			जारी;

		अगर (n == partno) {
			अगर (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				अवरोध;
		}
	}

	बंद_सूची(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (पूर्णांक) partno, (पूर्णांक) devno));
	वापस devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
अक्षर *sysfs_blkdev_get_slave(काष्ठा path_cxt *pc)
{
	सूची *dir;
	काष्ठा dirent *d;
	अक्षर *name = शून्य;

	dir = ul_path_opendir(pc, "slaves");
	अगर (!dir)
		वापस शून्य;

	जबतक ((d = xreaddir(dir))) {
		अगर (name)
			जाओ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	बंद_सूची(dir);
	वापस name;
err:
	मुक्त(name);
	बंद_सूची(dir);
	वापस शून्य;
}


#घोषणा SUBSYSTEM_LINKNAME	"/subsystem"

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
अटल अक्षर *get_subsystem(अक्षर *chain, अक्षर *buf, माप_प्रकार bufsz)
{
	माप_प्रकार len;
	अक्षर *p;

	अगर (!chain || !*chain)
		वापस शून्य;

	len = म_माप(chain);
	अगर (len + माप(SUBSYSTEM_LINKNAME) > PATH_MAX)
		वापस शून्य;

	करो {
		ssize_t sz;

		/* append "/subsystem" to the path */
		स_नकल(chain + len, SUBSYSTEM_LINKNAME, माप(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		अगर (p) {
			*p = '\0';
			len = p - chain;
		}

		अगर (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			वापस basename(buf);
		}

	} जबतक (p);

	वापस शून्य;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
अक्षर *sysfs_blkdev_get_devchain(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	माप_प्रकार sz = 0;
	काष्ठा ul_बफर tmp = UL_INIT_बफर;
	स्थिर अक्षर *p;
	अक्षर *res = शून्य;

	ssz = ul_path_readlink(pc, buf, bufsz, शून्य);
	अगर (ssz <= 0)
		वापस शून्य;

	अगर ((p = ul_path_get_prefix(pc)))
		ul_बफर_append_string(&tmp, p);

	ul_बफर_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_बफर_append_तथ्य(&tmp, buf, ssz);

	p = ul_बफर_get_string(&tmp, &sz, शून्य);
	अगर (p && sz <= bufsz) {
		स_नकल(buf, p, sz);
		res = buf;
	}
	ul_बफर_free_तथ्य(&tmp);
	वापस res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
पूर्णांक sysfs_blkdev_अग्र_subsystem(काष्ठा path_cxt *pc __attribute__((unused)),
			 अक्षर *devchain, अक्षर **subsys)
{
	अक्षर subbuf[PATH_MAX];
	अक्षर *sub;

	अगर (!subsys || !devchain)
		वापस -EINVAL;

	*subsys = शून्य;

	जबतक ((sub = get_subsystem(devchain, subbuf, माप(subbuf)))) {
		*subsys = strdup(sub);
		अगर (!*subsys)
			वापस -ENOMEM;
		वापस 0;
	}

	वापस 1;
}

#घोषणा REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
अटल पूर्णांक sysfs_devchain_is_removable(अक्षर *chain)
{
	माप_प्रकार len;
	अक्षर buf[20];
	अक्षर *p;

	अगर (!chain || !*chain)
		वापस 0;

	len = म_माप(chain);
	अगर (len + माप(REMOVABLE_FILENAME) > PATH_MAX)
		वापस 0;

	करो {
		पूर्णांक fd, rc;

		/* append "/removable" to the path */
		स_नकल(chain + len, REMOVABLE_FILENAME, माप(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		अगर (म_भेद(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			अवरोध;

		/* try to read it */
		fd = खोलो(chain, O_RDONLY | O_CLOEXEC);
		अगर (fd != -1) {
			rc = ul_read_all(fd, buf, माप(buf));
			बंद(fd);

			अगर (rc > 0) {
				अगर (म_भेदन(buf, "fixed", min(rc, 5)) == 0) {
					वापस 0;
				} अन्यथा अगर (म_भेदन(buf, "removable", min(rc, 9)) == 0) {
					वापस 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = म_खोजप(chain, '/');
		अगर (p) {
			*p = '\0';
			len = p - chain;
		}

	} जबतक (p);

	वापस 0;
}

पूर्णांक sysfs_blkdev_is_hotpluggable(काष्ठा path_cxt *pc)
{
	अक्षर buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, माप(buf));
	वापस sysfs_devchain_is_removable(chain);
}

पूर्णांक sysfs_blkdev_is_removable(काष्ठा path_cxt *pc)
{
	पूर्णांक rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	अगर (ul_path_read_s32(pc, &rc, "removable") == 0)
		वापस rc;

	वापस 0;
}

अटल पूर्णांक get_dm_wholedisk(काष्ठा path_cxt *pc, अक्षर *diskname,
                            माप_प्रकार len, dev_t *diskdevno)
{
	पूर्णांक rc = 0;
	अक्षर *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	अगर (!name)
		वापस -1;

	अगर (diskname && len)
		xstrncpy(diskname, name, len);

	अगर (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, शून्य);
		अगर (!*diskdevno)
			rc = -1;
	}

	मुक्त(name);
	वापस rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
पूर्णांक sysfs_blkdev_get_wholedisk(	काष्ठा path_cxt *pc,
				अक्षर *diskname,
				माप_प्रकार len,
				dev_t *diskdevno)
{
	पूर्णांक is_part = 0;

	अगर (!pc)
		वापस -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	अगर (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		अक्षर *uuid = शून्य, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : शून्य;

		अगर (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		मुक्त(uuid);

		अगर (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			जाओ done;

		is_part = 0;
	}

	अगर (!is_part) {
		/*
		 * unpartitioned device
		 */
		अगर (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			जाओ err;
		अगर (diskdevno)
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

		linklen = ul_path_readlink(pc, linkpath, माप(linkpath), शून्य);
		अगर (linklen < 0)
			जाओ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		अगर (!name)
			जाओ err;

		sysfs_devname_sys_to_dev(name);
		अगर (diskname && len)
			xstrncpy(diskname, name, len);

		अगर (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, शून्य);
			अगर (!*diskdevno)
				जाओ err;
		}
	}

done:
	वापस 0;
err:
	वापस -1;
}

पूर्णांक sysfs_devno_to_wholedisk(dev_t devno, अक्षर *diskname,
		             माप_प्रकार len, dev_t *diskdevno)
{
	काष्ठा path_cxt *pc;
	पूर्णांक rc = 0;

	अगर (!devno)
		वापस -EINVAL;
	pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अगर (!pc)
		वापस -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	वापस rc;
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
			     स्थिर काष्ठा ul_vfs_ops *vfs)
{
	काष्ठा path_cxt *pc = शून्य;
	अक्षर *id = शून्य;
	पूर्णांक rc = 0;

	pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अगर (!pc)
		जाओ done;
	ul_path_refer_vfs(pc, vfs);
	अगर (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		जाओ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	अगर (म_भेदन(id, "LVM-", 4) == 0) {
		अक्षर *p = म_खोजप(id + 4, '-');

		अगर (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	अगर (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	वापस rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
पूर्णांक sysfs_devno_is_dm_private(dev_t devno, अक्षर **uuid,
			      स्थिर काष्ठा ul_vfs_ops *vfs)
{
	काष्ठा path_cxt *pc = शून्य;
	अक्षर *id = शून्य;
	पूर्णांक rc = 0;

	pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अगर (!pc)
		जाओ done;
	ul_path_refer_vfs(pc, vfs);
	अगर (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		जाओ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	अगर (म_भेदन(id, "LVM-", 4) == 0) {
		अक्षर *p = म_खोजप(id + 4, '-');

		अगर (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} अन्यथा अगर (म_भेदन(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	अगर (uuid)
		*uuid = id;
	अन्यथा
		मुक्त(id);
	वापस rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
पूर्णांक sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	अगर (sysfs_devno_to_wholedisk(devno, शून्य, 0, &disk) != 0)
		वापस -1;

	वापस devno == disk;
}


पूर्णांक sysfs_blkdev_scsi_get_hctl(काष्ठा path_cxt *pc, पूर्णांक *h, पूर्णांक *c, पूर्णांक *t, पूर्णांक *l)
{
	अक्षर buf[PATH_MAX], *hctl;
	काष्ठा sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	अगर (!blk || blk->hctl_error)
		वापस -EINVAL;
	अगर (blk->has_hctl)
		जाओ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, माप(buf), "device");
	अगर (len < 0)
		वापस len;

	hctl = म_खोजप(buf, '/');
	अगर (!hctl)
		वापस -1;
	hctl++;

	अगर (माला_पूछो(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		वापस -1;

	blk->has_hctl = 1;
done:
	अगर (h)
		*h = blk->scsi_host;
	अगर (c)
		*c = blk->scsi_channel;
	अगर (t)
		*t = blk->scsi_target;
	अगर (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	वापस 0;
}


अटल अक्षर *scsi_host_attribute_path(
			काष्ठा path_cxt *pc,
			स्थिर अक्षर *type,
			अक्षर *buf,
			माप_प्रकार bufsz,
			स्थिर अक्षर *attr)
{
	पूर्णांक len;
	पूर्णांक host;
	स्थिर अक्षर *prefix;

	अगर (sysfs_blkdev_scsi_get_hctl(pc, &host, शून्य, शून्य, शून्य))
		वापस शून्य;

	prefix = ul_path_get_prefix(pc);
	अगर (!prefix)
		prefix = "";

	अगर (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	वापस (len < 0 || (माप_प्रकार) len >= bufsz) ? शून्य : buf;
}

अक्षर *sysfs_blkdev_scsi_host_strdup_attribute(काष्ठा path_cxt *pc,
			स्थिर अक्षर *type, स्थिर अक्षर *attr)
{
	अक्षर buf[1024];
	पूर्णांक rc;
	खाता *f;

	अगर (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, माप(buf), attr))
		वापस शून्य;

	अगर (!(f = ख_खोलो(buf, "r" UL_CLOEXECSTR)))
		वापस शून्य;

	rc = ख_पूछो(f, "%1023[^\n]", buf);
	ख_बंद(f);

	वापस rc == 1 ? strdup(buf) : शून्य;
}

पूर्णांक sysfs_blkdev_scsi_host_is(काष्ठा path_cxt *pc, स्थिर अक्षर *type)
{
	अक्षर buf[PATH_MAX];
	काष्ठा stat st;

	अगर (!type || !scsi_host_attribute_path(pc, type,
				buf, माप(buf), शून्य))
		वापस 0;

	वापस stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

अटल अक्षर *scsi_attribute_path(काष्ठा path_cxt *pc,
		अक्षर *buf, माप_प्रकार bufsz, स्थिर अक्षर *attr)
{
	पूर्णांक len, h, c, t, l;
	स्थिर अक्षर *prefix;

	अगर (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		वापस शून्य;

	prefix = ul_path_get_prefix(pc);
	अगर (!prefix)
		prefix = "";

	अगर (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	अन्यथा
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	वापस (len < 0 || (माप_प्रकार) len >= bufsz) ? शून्य : buf;
}

पूर्णांक sysfs_blkdev_scsi_has_attribute(काष्ठा path_cxt *pc, स्थिर अक्षर *attr)
{
	अक्षर path[PATH_MAX];
	काष्ठा stat st;

	अगर (!scsi_attribute_path(pc, path, माप(path), attr))
		वापस 0;

	वापस stat(path, &st) == 0;
}

पूर्णांक sysfs_blkdev_scsi_path_contains(काष्ठा path_cxt *pc, स्थिर अक्षर *pattern)
{
	अक्षर path[PATH_MAX], linkc[PATH_MAX];
	काष्ठा stat st;
	ssize_t len;

	अगर (!scsi_attribute_path(pc, path, माप(path), शून्य))
		वापस 0;

	अगर (stat(path, &st) != 0)
		वापस 0;

	len = readlink(path, linkc, माप(linkc) - 1);
	अगर (len < 0)
		वापस 0;

	linkc[len] = '\0';
	वापस म_माला(linkc, pattern) != शून्य;
}

अटल dev_t read_devno(स्थिर अक्षर *path)
{
	खाता *f;
	पूर्णांक maj = 0, min = 0;
	dev_t dev = 0;

	f = ख_खोलो(path, "r" UL_CLOEXECSTR);
	अगर (!f)
		वापस 0;

	अगर (ख_पूछो(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	ख_बंद(f);
	वापस dev;
}

पूर्णांक sysfs_devname_is_hidden(स्थिर अक्षर *prefix, स्थिर अक्षर *name)
{
	अक्षर buf[PATH_MAX];
	पूर्णांक rc = 0, hidden = 0, len;
	खाता *f;

	अगर (म_भेदन("/dev/", name, 5) == 0)
		वापस 0;

	अगर (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, माप(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	अगर (len < 0 || (माप_प्रकार) len + 1 > माप(buf))
		वापस 0;

	f = ख_खोलो(buf, "r" UL_CLOEXECSTR);
	अगर (!f)
		वापस 0;

	rc = ख_पूछो(f, "%d", &hidden);
	ख_बंद(f);

	वापस rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(स्थिर अक्षर *prefix, स्थिर अक्षर *name, स्थिर अक्षर *parent)
{
	अक्षर buf[PATH_MAX];
	अक्षर *_name = शून्य, *_parent = शून्य;	/* name as encoded in sysfs */
	dev_t dev = 0;
	पूर्णांक len;

	अगर (!prefix)
		prefix = "";

	निश्चित(name);

	अगर (म_भेदन("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		काष्ठा stat st;

		अगर (stat(name, &st) == 0) {
			dev = st.st_rdev;
			जाओ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	अगर (!_name)
		जाओ done;
	sysfs_devname_dev_to_sys(_name);

	अगर (parent) {
		_parent = strdup(parent);
		अगर (!_parent)
			जाओ done;
	}

	अगर (parent && म_भेदन("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, माप(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
			जाओ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		जाओ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, माप(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
		जाओ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	अगर (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, माप(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
			जाओ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	अगर (!dev) {
		len = snprintf(buf, माप(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
			जाओ done;
		dev = read_devno(buf);
	}
done:
	मुक्त(_name);
	मुक्त(_parent);
	वापस dev;
}

dev_t sysfs_devname_to_devno(स्थिर अक्षर *name)
{
	वापस __sysfs_devname_to_devno(शून्य, name, शून्य);
}

अक्षर *sysfs_blkdev_get_path(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz)
{
	स्थिर अक्षर *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	अक्षर *res = शून्य;
	माप_प्रकार sz;
	काष्ठा stat st;

	अगर (!name)
		जाओ done;

	sz = म_माप(name);
	अगर (sz + माप("/dev/") > bufsiz)
		जाओ done;

	/* create the final "/dev/<name>" string */
	स_हटाओ(buf + 5, name, sz + 1);
	स_नकल(buf, "/dev/", 5);

	अगर (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	वापस res;
}

dev_t sysfs_blkdev_get_devno(काष्ठा path_cxt *pc)
{
	वापस ((काष्ठा sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
अक्षर *sysfs_devno_to_devpath(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	काष्ठा path_cxt *pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अक्षर *res = शून्य;

	अगर (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	वापस res;
}

अक्षर *sysfs_devno_to_devname(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	काष्ठा path_cxt *pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अक्षर *res = शून्य;

	अगर (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	वापस res;
}

पूर्णांक sysfs_devno_गिनती_partitions(dev_t devno)
{
	काष्ठा path_cxt *pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	पूर्णांक n = 0;

	अगर (pc) {
		अक्षर buf[PATH_MAX + 1];
		अक्षर *name = sysfs_blkdev_get_name(pc, buf, माप(buf));

		n = sysfs_blkdev_गिनती_partitions(pc, name);
		ul_unref_path(pc);
	}
	वापस n;
}

अक्षर *sysfs_chrdev_devno_to_devname(dev_t devno, अक्षर *buf, माप_प्रकार bufsiz)
{
	अक्षर link[PATH_MAX];
	काष्ठा path_cxt *pc;
	अक्षर *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	अगर (!pc)
		वापस शून्य;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, माप(link), शून्य);
	ul_unref_path(pc);

	अगर (sz < 0)
		वापस शून्य;

	name = म_खोजप(link, '/');
	अगर (!name)
		वापस शून्य;

	name++;
	sz = म_माप(name);
	अगर ((माप_प्रकार) sz + 1 > bufsiz)
		वापस शून्य;

	स_नकल(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	वापस buf;

}

क्रमागत sysfs_byteorder sysfs_get_byteorder(काष्ठा path_cxt *pc)
{
	पूर्णांक rc;
	अक्षर buf[बफ_मान];
	क्रमागत sysfs_byteorder ret;

	rc = ul_path_read_बफर(pc, buf, माप(buf), _PATH_SYS_CPU_BYTEORDER);
	अगर (rc < 0)
		जाओ unknown;

	अगर (म_भेदन(buf, "little", माप(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		जाओ out;
	} अन्यथा अगर (म_भेदन(buf, "big", माप(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		जाओ out;
	}

unknown:
#अगर __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#या_अगर __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#अन्यथा
#त्रुटि Unknown byte order
#पूर्ण_अगर

out:
	वापस ret;
}

पूर्णांक sysfs_get_address_bits(काष्ठा path_cxt *pc)
{
	पूर्णांक rc;
	पूर्णांक address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	अगर (rc < 0)
		वापस rc;
	अगर (address_bits < 0)
		वापस -EINVAL;
	वापस address_bits;
}


#अगर_घोषित TEST_PROGRAM_SYSFS
#समावेश <errno.h>
#समावेश <stdlib.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा path_cxt *pc;
	अक्षर *devname;
	dev_t devno, disk_devno;
	अक्षर path[PATH_MAX], *sub, *chain;
	अक्षर diskname[32];
	पूर्णांक i, is_part, rc = निकास_सफल;
	uint64_t u64;

	अगर (argc != 2)
		errx(निकास_त्रुटि, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	अगर (!devno)
		err(निकास_त्रुटि, "failed to read devno");

	म_लिखो("non-context:\n");
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, माप(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, माप(path)));

	sysfs_devno_to_wholedisk(devno, diskname, माप(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, शून्य, शून्य);
	अगर (!pc)
		जाओ done;

	म_लिखो("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	म_लिखो(" DEVNO:   %u (%u:%u)\n", (अचिन्हित पूर्णांक) devno, major(devno), minor(devno));
	म_लिखो(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, माप(path)));
	म_लिखो(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, माप(path)));

	sysfs_devno_to_wholedisk(devno, diskname, माप(diskname), &disk_devno);
	म_लिखो(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (अचिन्हित पूर्णांक) disk_devno, major(disk_devno), minor(disk_devno));
	म_लिखो(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	म_लिखो(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	अगर (is_part && disk_devno) {
		काष्ठा path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, शून्य, शून्य);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	म_लिखो(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	म_लिखो(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	म_लिखो(" SLAVES: %d\n", ul_path_गिनती_dirents(pc, "slaves"));

	अगर (!is_part) {
		म_लिखो("First 5 partitions:\n");
		क्रम (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			अगर (dev)
				म_लिखो("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	अगर (ul_path_read_u64(pc, &u64, "size") != 0)
		म_लिखो(" (!) read SIZE failed\n");
	अन्यथा
		म_लिखो(" SIZE: %"PRIu64"\n", u64);

	अगर (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		म_लिखो(" (!) read SECTOR failed\n");
	अन्यथा
		म_लिखो(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, माप(path));
	म_लिखो(" SUBSYSTEMS:\n");

	जबतक (chain && sysfs_blkdev_अग्र_subsystem(pc, chain, &sub) == 0) {
		म_लिखो("\t%s\n", sub);
		मुक्त(sub);
	}

	rc = निकास_सफल;
done:
	ul_unref_path(pc);
	वापस rc;
}
#पूर्ण_अगर /* TEST_PROGRAM_SYSFS */
