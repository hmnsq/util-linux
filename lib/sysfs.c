/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <libgen.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "sysfs.h"
#അന്തര്ഭാവ "fileutils.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "debug.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "buffer.h"

സ്ഥിതം ശൂന്യം sysfs_blkdev_deinit_path(ഘടന path_cxt *pc);
സ്ഥിതം പൂർണ്ണസംഖ്യ  sysfs_blkdev_enoent_redirect(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
സ്ഥിതം UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

ശൂന്യം ul_sysfs_init_debug(ശൂന്യം)
{
	എങ്കിൽ (ulsysfs_debug_mask)
		തിരികെ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ഘടന path_cxt *ul_new_sysfs_path(dev_t devno, ഘടന path_cxt *parent, സ്ഥിരം അക്ഷരം *prefix)
{
	ഘടന path_cxt *pc = ul_new_path(ശൂന്യ);

	എങ്കിൽ (!pc)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (prefix)
		ul_path_set_prefix(pc, prefix);

	എങ്കിൽ (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		തിരികെ ശൂന്യ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	തിരികെ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
പൂർണ്ണസംഖ്യ sysfs_blkdev_init_path(ഘടന path_cxt *pc, dev_t devno, ഘടന path_cxt *parent)
{
	ഘടന sysfs_blkdev *blk;
	പൂർണ്ണസംഖ്യ rc;
	അക്ഷരം buf[വലുപ്പം(_PATH_SYS_DEVBLOCK)
		 + വലുപ്പം(stringify_മൂല്യം(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, വലുപ്പം(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	എങ്കിൽ (rc)
		തിരികെ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	എങ്കിൽ (rc < 0)
		തിരികെ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	എങ്കിൽ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, വലുപ്പം(ഘടന sysfs_blkdev));
		എങ്കിൽ (!blk)
			തിരികെ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം sysfs_blkdev_deinit_path(ഘടന path_cxt *pc)
{
	ഘടന sysfs_blkdev *blk;

	എങ്കിൽ (!pc)
		തിരികെ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	എങ്കിൽ (!blk)
		തിരികെ;

	ul_unref_path(blk->parent);
	സ്വതന്ത്രം(blk);

	ul_path_set_dialect(pc, ശൂന്യ, ശൂന്യ);
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_set_parent(ഘടന path_cxt *pc, ഘടന path_cxt *parent)
{
	ഘടന sysfs_blkdev *blk = ul_path_get_dialect(pc);

	എങ്കിൽ (!pc || !blk)
		തിരികെ -EINVAL;

	എങ്കിൽ (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = ശൂന്യ;
	}

	എങ്കിൽ (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} അല്ലെങ്കിൽ
		blk->parent = ശൂന്യ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	തിരികെ 0;
}

ഘടന path_cxt *sysfs_blkdev_get_parent(ഘടന path_cxt *pc)
{
	ഘടന sysfs_blkdev *blk = ul_path_get_dialect(pc);
	തിരികെ blk ? blk->parent : ശൂന്യ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ sysfs_blkdev_enoent_redirect(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *path, പൂർണ്ണസംഖ്യ *dirfd)
{
	ഘടന sysfs_blkdev *blk = ul_path_get_dialect(pc);

	എങ്കിൽ (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		എങ്കിൽ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			തിരികെ 0;
		}
	}
	തിരികെ 1;	/* no redirect */
}

അക്ഷരം *sysfs_blkdev_get_name(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsiz)
{
	അക്ഷരം link[PATH_MAX];
	അക്ഷരം *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, വലുപ്പം(link), ശൂന്യ);
	എങ്കിൽ (sz < 0)
		തിരികെ ശൂന്യ;

	name = strrchr(link, '/');
	എങ്കിൽ (!name)
		തിരികെ ശൂന്യ;

	name++;
	sz = strlen(name);
	എങ്കിൽ ((size_t) sz + 1 > bufsiz)
		തിരികെ ശൂന്യ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	തിരികെ buf;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_is_partition_dirent(DIR *dir, ഘടന dirent *d, സ്ഥിരം അക്ഷരം *parent_name)
{
	അക്ഷരം path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	എങ്കിൽ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		തിരികെ 0;
#endif
	size_t len = 0;

	എങ്കിൽ (parent_name) {
		സ്ഥിരം അക്ഷരം *p = parent_name;

		/* /dev/sda --> "sda" */
		എങ്കിൽ (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			എങ്കിൽ (!p)
				തിരികെ 0;
			p++;
		}

		len = strlen(p);
		എങ്കിൽ ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	എങ്കിൽ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		തിരികെ ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, വലുപ്പം(path), "%s/start", d->d_name);

	തിരികെ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_എണ്ണം_partitions(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *devname)
{
	DIR *dir;
	ഘടന dirent *d;
	പൂർണ്ണസംഖ്യ r = 0;

	dir = ul_path_opendir(pc, ശൂന്യ);
	എങ്കിൽ (!dir)
		തിരികെ 0;

	ആകുമ്പോൾ ((d = xreaddir(dir))) {
		എങ്കിൽ (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	തിരികെ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ partno)
{
	DIR *dir;
	ഘടന dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, ശൂന്യ);
	എങ്കിൽ (!dir)
		തിരികെ 0;

	ആകുമ്പോൾ ((d = xreaddir(dir))) {
		പൂർണ്ണസംഖ്യ n;

		എങ്കിൽ (!sysfs_blkdev_is_partition_dirent(dir, d, ശൂന്യ))
			തുടരുക;

		എങ്കിൽ (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			തുടരുക;

		എങ്കിൽ (n == partno) {
			എങ്കിൽ (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				നിർത്തുക;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (പൂർണ്ണസംഖ്യ) partno, (പൂർണ്ണസംഖ്യ) devno));
	തിരികെ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
അക്ഷരം *sysfs_blkdev_get_slave(ഘടന path_cxt *pc)
{
	DIR *dir;
	ഘടന dirent *d;
	അക്ഷരം *name = ശൂന്യ;

	dir = ul_path_opendir(pc, "slaves");
	എങ്കിൽ (!dir)
		തിരികെ ശൂന്യ;

	ആകുമ്പോൾ ((d = xreaddir(dir))) {
		എങ്കിൽ (name)
			പോകുക err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	തിരികെ name;
err:
	സ്വതന്ത്രം(name);
	closedir(dir);
	തിരികെ ശൂന്യ;
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
സ്ഥിതം അക്ഷരം *get_subsystem(അക്ഷരം *chain, അക്ഷരം *buf, size_t bufsz)
{
	size_t len;
	അക്ഷരം *p;

	എങ്കിൽ (!chain || !*chain)
		തിരികെ ശൂന്യ;

	len = strlen(chain);
	എങ്കിൽ (len + വലുപ്പം(SUBSYSTEM_LINKNAME) > PATH_MAX)
		തിരികെ ശൂന്യ;

	ചെയ്യുക {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, വലുപ്പം(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		എങ്കിൽ (p) {
			*p = '\0';
			len = p - chain;
		}

		എങ്കിൽ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			തിരികെ basename(buf);
		}

	} ആകുമ്പോൾ (p);

	തിരികെ ശൂന്യ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
അക്ഷരം *sysfs_blkdev_get_devchain(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ഘടന ul_buffer tmp = UL_INIT_BUFFER;
	സ്ഥിരം അക്ഷരം *p;
	അക്ഷരം *res = ശൂന്യ;

	ssz = ul_path_readlink(pc, buf, bufsz, ശൂന്യ);
	എങ്കിൽ (ssz <= 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, ശൂന്യ);
	എങ്കിൽ (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	തിരികെ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
പൂർണ്ണസംഖ്യ sysfs_blkdev_next_subsystem(ഘടന path_cxt *pc __attribute__((unused)),
			 അക്ഷരം *devchain, അക്ഷരം **subsys)
{
	അക്ഷരം subbuf[PATH_MAX];
	അക്ഷരം *sub;

	എങ്കിൽ (!subsys || !devchain)
		തിരികെ -EINVAL;

	*subsys = ശൂന്യ;

	ആകുമ്പോൾ ((sub = get_subsystem(devchain, subbuf, വലുപ്പം(subbuf)))) {
		*subsys = strdup(sub);
		എങ്കിൽ (!*subsys)
			തിരികെ -ENOMEM;
		തിരികെ 0;
	}

	തിരികെ 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ sysfs_devchain_is_removable(അക്ഷരം *chain)
{
	size_t len;
	അക്ഷരം buf[20];
	അക്ഷരം *p;

	എങ്കിൽ (!chain || !*chain)
		തിരികെ 0;

	len = strlen(chain);
	എങ്കിൽ (len + വലുപ്പം(REMOVABLE_FILENAME) > PATH_MAX)
		തിരികെ 0;

	ചെയ്യുക {
		പൂർണ്ണസംഖ്യ fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, വലുപ്പം(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		എങ്കിൽ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			നിർത്തുക;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		എങ്കിൽ (fd != -1) {
			rc = ul_read_all(fd, buf, വലുപ്പം(buf));
			close(fd);

			എങ്കിൽ (rc > 0) {
				എങ്കിൽ (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					തിരികെ 0;
				} അല്ലെങ്കിൽ എങ്കിൽ (strncmp(buf, "removable", min(rc, 9)) == 0) {
					തിരികെ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		എങ്കിൽ (p) {
			*p = '\0';
			len = p - chain;
		}

	} ആകുമ്പോൾ (p);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_is_hotpluggable(ഘടന path_cxt *pc)
{
	അക്ഷരം buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, വലുപ്പം(buf));
	തിരികെ sysfs_devchain_is_removable(chain);
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_is_removable(ഘടന path_cxt *pc)
{
	പൂർണ്ണസംഖ്യ rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	എങ്കിൽ (ul_path_read_s32(pc, &rc, "removable") == 0)
		തിരികെ rc;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ get_dm_wholedisk(ഘടന path_cxt *pc, അക്ഷരം *diskname,
                            size_t len, dev_t *diskdevno)
{
	പൂർണ്ണസംഖ്യ rc = 0;
	അക്ഷരം *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	എങ്കിൽ (!name)
		തിരികെ -1;

	എങ്കിൽ (diskname && len)
		xstrncpy(diskname, name, len);

	എങ്കിൽ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ശൂന്യ);
		എങ്കിൽ (!*diskdevno)
			rc = -1;
	}

	സ്വതന്ത്രം(name);
	തിരികെ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
പൂർണ്ണസംഖ്യ sysfs_blkdev_get_wholedisk(	ഘടന path_cxt *pc,
				അക്ഷരം *diskname,
				size_t len,
				dev_t *diskdevno)
{
	പൂർണ്ണസംഖ്യ is_part = 0;

	എങ്കിൽ (!pc)
		തിരികെ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	എങ്കിൽ (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		അക്ഷരം *uuid = ശൂന്യ, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : ശൂന്യ;

		എങ്കിൽ (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		സ്വതന്ത്രം(uuid);

		എങ്കിൽ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			പോകുക done;

		is_part = 0;
	}

	എങ്കിൽ (!is_part) {
		/*
		 * unpartitioned device
		 */
		എങ്കിൽ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			പോകുക err;
		എങ്കിൽ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} അല്ലെങ്കിൽ {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		അക്ഷരം linkpath[PATH_MAX];
		അക്ഷരം *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, വലുപ്പം(linkpath), ശൂന്യ);
		എങ്കിൽ (linklen < 0)
			പോകുക err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		എങ്കിൽ (!name)
			പോകുക err;

		sysfs_devname_sys_to_dev(name);
		എങ്കിൽ (diskname && len)
			xstrncpy(diskname, name, len);

		എങ്കിൽ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ശൂന്യ);
			എങ്കിൽ (!*diskdevno)
				പോകുക err;
		}
	}

done:
	തിരികെ 0;
err:
	തിരികെ -1;
}

പൂർണ്ണസംഖ്യ sysfs_devno_to_wholedisk(dev_t devno, അക്ഷരം *diskname,
		             size_t len, dev_t *diskdevno)
{
	ഘടന path_cxt *pc;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (!devno)
		തിരികെ -EINVAL;
	pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (!pc)
		തിരികെ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	തിരികെ rc;
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
പൂർണ്ണസംഖ്യ sysfs_devno_is_dm_hidden(dev_t devno, അക്ഷരം **uuid,
			     സ്ഥിരം ഘടന ul_vfs_ops *vfs)
{
	ഘടന path_cxt *pc = ശൂന്യ;
	അക്ഷരം *id = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc = 0;

	pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (!pc)
		പോകുക done;
	ul_path_refer_vfs(pc, vfs);
	എങ്കിൽ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		പോകുക done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	എങ്കിൽ (strncmp(id, "LVM-", 4) == 0) {
		അക്ഷരം *p = strrchr(id + 4, '-');

		എങ്കിൽ (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	എങ്കിൽ (uuid)
		*uuid = id;
	അല്ലെങ്കിൽ
		സ്വതന്ത്രം(id);
	തിരികെ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
പൂർണ്ണസംഖ്യ sysfs_devno_is_dm_private(dev_t devno, അക്ഷരം **uuid,
			      സ്ഥിരം ഘടന ul_vfs_ops *vfs)
{
	ഘടന path_cxt *pc = ശൂന്യ;
	അക്ഷരം *id = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc = 0;

	pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (!pc)
		പോകുക done;
	ul_path_refer_vfs(pc, vfs);
	എങ്കിൽ (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		പോകുക done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	എങ്കിൽ (strncmp(id, "LVM-", 4) == 0) {
		അക്ഷരം *p = strrchr(id + 4, '-');

		എങ്കിൽ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} അല്ലെങ്കിൽ എങ്കിൽ (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	എങ്കിൽ (uuid)
		*uuid = id;
	അല്ലെങ്കിൽ
		സ്വതന്ത്രം(id);
	തിരികെ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
പൂർണ്ണസംഖ്യ sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	എങ്കിൽ (sysfs_devno_to_wholedisk(devno, ശൂന്യ, 0, &disk) != 0)
		തിരികെ -1;

	തിരികെ devno == disk;
}


പൂർണ്ണസംഖ്യ sysfs_blkdev_scsi_get_hctl(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ *h, പൂർണ്ണസംഖ്യ *c, പൂർണ്ണസംഖ്യ *t, പൂർണ്ണസംഖ്യ *l)
{
	അക്ഷരം buf[PATH_MAX], *hctl;
	ഘടന sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	എങ്കിൽ (!blk || blk->hctl_error)
		തിരികെ -EINVAL;
	എങ്കിൽ (blk->has_hctl)
		പോകുക done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, വലുപ്പം(buf), "device");
	എങ്കിൽ (len < 0)
		തിരികെ len;

	hctl = strrchr(buf, '/');
	എങ്കിൽ (!hctl)
		തിരികെ -1;
	hctl++;

	എങ്കിൽ (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		തിരികെ -1;

	blk->has_hctl = 1;
done:
	എങ്കിൽ (h)
		*h = blk->scsi_host;
	എങ്കിൽ (c)
		*c = blk->scsi_channel;
	എങ്കിൽ (t)
		*t = blk->scsi_target;
	എങ്കിൽ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	തിരികെ 0;
}


സ്ഥിതം അക്ഷരം *scsi_host_attribute_path(
			ഘടന path_cxt *pc,
			സ്ഥിരം അക്ഷരം *type,
			അക്ഷരം *buf,
			size_t bufsz,
			സ്ഥിരം അക്ഷരം *attr)
{
	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ host;
	സ്ഥിരം അക്ഷരം *prefix;

	എങ്കിൽ (sysfs_blkdev_scsi_get_hctl(pc, &host, ശൂന്യ, ശൂന്യ, ശൂന്യ))
		തിരികെ ശൂന്യ;

	prefix = ul_path_get_prefix(pc);
	എങ്കിൽ (!prefix)
		prefix = "";

	എങ്കിൽ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	അല്ലെങ്കിൽ
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	തിരികെ (len < 0 || (size_t) len >= bufsz) ? ശൂന്യ : buf;
}

അക്ഷരം *sysfs_blkdev_scsi_host_strdup_attribute(ഘടന path_cxt *pc,
			സ്ഥിരം അക്ഷരം *type, സ്ഥിരം അക്ഷരം *attr)
{
	അക്ഷരം buf[1024];
	പൂർണ്ണസംഖ്യ rc;
	FILE *f;

	എങ്കിൽ (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, വലുപ്പം(buf), attr))
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		തിരികെ ശൂന്യ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	തിരികെ rc == 1 ? strdup(buf) : ശൂന്യ;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_scsi_host_is(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *type)
{
	അക്ഷരം buf[PATH_MAX];
	ഘടന stat st;

	എങ്കിൽ (!type || !scsi_host_attribute_path(pc, type,
				buf, വലുപ്പം(buf), ശൂന്യ))
		തിരികെ 0;

	തിരികെ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

സ്ഥിതം അക്ഷരം *scsi_attribute_path(ഘടന path_cxt *pc,
		അക്ഷരം *buf, size_t bufsz, സ്ഥിരം അക്ഷരം *attr)
{
	പൂർണ്ണസംഖ്യ len, h, c, t, l;
	സ്ഥിരം അക്ഷരം *prefix;

	എങ്കിൽ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		തിരികെ ശൂന്യ;

	prefix = ul_path_get_prefix(pc);
	എങ്കിൽ (!prefix)
		prefix = "";

	എങ്കിൽ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	അല്ലെങ്കിൽ
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	തിരികെ (len < 0 || (size_t) len >= bufsz) ? ശൂന്യ : buf;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_scsi_has_attribute(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *attr)
{
	അക്ഷരം path[PATH_MAX];
	ഘടന stat st;

	എങ്കിൽ (!scsi_attribute_path(pc, path, വലുപ്പം(path), attr))
		തിരികെ 0;

	തിരികെ stat(path, &st) == 0;
}

പൂർണ്ണസംഖ്യ sysfs_blkdev_scsi_path_contains(ഘടന path_cxt *pc, സ്ഥിരം അക്ഷരം *pattern)
{
	അക്ഷരം path[PATH_MAX], linkc[PATH_MAX];
	ഘടന stat st;
	ssize_t len;

	എങ്കിൽ (!scsi_attribute_path(pc, path, വലുപ്പം(path), ശൂന്യ))
		തിരികെ 0;

	എങ്കിൽ (stat(path, &st) != 0)
		തിരികെ 0;

	len = readlink(path, linkc, വലുപ്പം(linkc) - 1);
	എങ്കിൽ (len < 0)
		തിരികെ 0;

	linkc[len] = '\0';
	തിരികെ strstr(linkc, pattern) != ശൂന്യ;
}

സ്ഥിതം dev_t read_devno(സ്ഥിരം അക്ഷരം *path)
{
	FILE *f;
	പൂർണ്ണസംഖ്യ maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	എങ്കിൽ (!f)
		തിരികെ 0;

	എങ്കിൽ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	തിരികെ dev;
}

പൂർണ്ണസംഖ്യ sysfs_devname_is_hidden(സ്ഥിരം അക്ഷരം *prefix, സ്ഥിരം അക്ഷരം *name)
{
	അക്ഷരം buf[PATH_MAX];
	പൂർണ്ണസംഖ്യ rc = 0, hidden = 0, len;
	FILE *f;

	എങ്കിൽ (strncmp("/dev/", name, 5) == 0)
		തിരികെ 0;

	എങ്കിൽ (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, വലുപ്പം(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	എങ്കിൽ (len < 0 || (size_t) len + 1 > വലുപ്പം(buf))
		തിരികെ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	എങ്കിൽ (!f)
		തിരികെ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	തിരികെ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(സ്ഥിരം അക്ഷരം *prefix, സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *parent)
{
	അക്ഷരം buf[PATH_MAX];
	അക്ഷരം *_name = ശൂന്യ, *_parent = ശൂന്യ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	പൂർണ്ണസംഖ്യ len;

	എങ്കിൽ (!prefix)
		prefix = "";

	പ്രതിജ്ഞാ(name);

	എങ്കിൽ (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ഘടന stat st;

		എങ്കിൽ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			പോകുക done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	എങ്കിൽ (!_name)
		പോകുക done;
	sysfs_devname_dev_to_sys(_name);

	എങ്കിൽ (parent) {
		_parent = strdup(parent);
		എങ്കിൽ (!_parent)
			പോകുക done;
	}

	എങ്കിൽ (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, വലുപ്പം(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
			പോകുക done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		പോകുക done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, വലുപ്പം(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
		പോകുക done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	എങ്കിൽ (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, വലുപ്പം(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
			പോകുക done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	എങ്കിൽ (!dev) {
		len = snprintf(buf, വലുപ്പം(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		എങ്കിൽ (len < 0 || (size_t) len >= വലുപ്പം(buf))
			പോകുക done;
		dev = read_devno(buf);
	}
done:
	സ്വതന്ത്രം(_name);
	സ്വതന്ത്രം(_parent);
	തിരികെ dev;
}

dev_t sysfs_devname_to_devno(സ്ഥിരം അക്ഷരം *name)
{
	തിരികെ __sysfs_devname_to_devno(ശൂന്യ, name, ശൂന്യ);
}

അക്ഷരം *sysfs_blkdev_get_path(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsiz)
{
	സ്ഥിരം അക്ഷരം *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	അക്ഷരം *res = ശൂന്യ;
	size_t sz;
	ഘടന stat st;

	എങ്കിൽ (!name)
		പോകുക done;

	sz = strlen(name);
	എങ്കിൽ (sz + വലുപ്പം("/dev/") > bufsiz)
		പോകുക done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	എങ്കിൽ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	തിരികെ res;
}

dev_t sysfs_blkdev_get_devno(ഘടന path_cxt *pc)
{
	തിരികെ ((ഘടന sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
അക്ഷരം *sysfs_devno_to_devpath(dev_t devno, അക്ഷരം *buf, size_t bufsiz)
{
	ഘടന path_cxt *pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	തിരികെ res;
}

അക്ഷരം *sysfs_devno_to_devname(dev_t devno, അക്ഷരം *buf, size_t bufsiz)
{
	ഘടന path_cxt *pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	അക്ഷരം *res = ശൂന്യ;

	എങ്കിൽ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	തിരികെ res;
}

പൂർണ്ണസംഖ്യ sysfs_devno_എണ്ണം_partitions(dev_t devno)
{
	ഘടന path_cxt *pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	പൂർണ്ണസംഖ്യ n = 0;

	എങ്കിൽ (pc) {
		അക്ഷരം buf[PATH_MAX + 1];
		അക്ഷരം *name = sysfs_blkdev_get_name(pc, buf, വലുപ്പം(buf));

		n = sysfs_blkdev_എണ്ണം_partitions(pc, name);
		ul_unref_path(pc);
	}
	തിരികെ n;
}

അക്ഷരം *sysfs_chrdev_devno_to_devname(dev_t devno, അക്ഷരം *buf, size_t bufsiz)
{
	അക്ഷരം link[PATH_MAX];
	ഘടന path_cxt *pc;
	അക്ഷരം *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	എങ്കിൽ (!pc)
		തിരികെ ശൂന്യ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, വലുപ്പം(link), ശൂന്യ);
	ul_unref_path(pc);

	എങ്കിൽ (sz < 0)
		തിരികെ ശൂന്യ;

	name = strrchr(link, '/');
	എങ്കിൽ (!name)
		തിരികെ ശൂന്യ;

	name++;
	sz = strlen(name);
	എങ്കിൽ ((size_t) sz + 1 > bufsiz)
		തിരികെ ശൂന്യ;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	തിരികെ buf;

}

എണ്ണൽ sysfs_byteorder sysfs_get_byteorder(ഘടന path_cxt *pc)
{
	പൂർണ്ണസംഖ്യ rc;
	അക്ഷരം buf[BUFSIZ];
	എണ്ണൽ sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, വലുപ്പം(buf), _PATH_SYS_CPU_BYTEORDER);
	എങ്കിൽ (rc < 0)
		പോകുക unknown;

	എങ്കിൽ (strncmp(buf, "little", വലുപ്പം(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		പോകുക out;
	} അല്ലെങ്കിൽ എങ്കിൽ (strncmp(buf, "big", വലുപ്പം(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		പോകുക out;
	}

unknown:
#എങ്കിൽ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#അല്ലെങ്കിലെങ്കിൽ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#അല്ലെങ്കിൽ
#error Unknown byte order
#endif

out:
	തിരികെ ret;
}

പൂർണ്ണസംഖ്യ sysfs_get_address_bits(ഘടന path_cxt *pc)
{
	പൂർണ്ണസംഖ്യ rc;
	പൂർണ്ണസംഖ്യ address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	എങ്കിൽ (rc < 0)
		തിരികെ rc;
	എങ്കിൽ (address_bits < 0)
		തിരികെ -EINVAL;
	തിരികെ address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdlib.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന path_cxt *pc;
	അക്ഷരം *devname;
	dev_t devno, disk_devno;
	അക്ഷരം path[PATH_MAX], *sub, *chain;
	അക്ഷരം diskname[32];
	പൂർണ്ണസംഖ്യ i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	എങ്കിൽ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	എങ്കിൽ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned പൂർണ്ണസംഖ്യ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, വലുപ്പം(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, വലുപ്പം(path)));

	sysfs_devno_to_wholedisk(devno, diskname, വലുപ്പം(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned പൂർണ്ണസംഖ്യ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (!pc)
		പോകുക done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned പൂർണ്ണസംഖ്യ) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, വലുപ്പം(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, വലുപ്പം(path)));

	sysfs_devno_to_wholedisk(devno, diskname, വലുപ്പം(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned പൂർണ്ണസംഖ്യ) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	എങ്കിൽ (is_part && disk_devno) {
		ഘടന path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, ശൂന്യ, ശൂന്യ);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_എണ്ണം_dirents(pc, "slaves"));

	എങ്കിൽ (!is_part) {
		printf("First 5 partitions:\n");
		വേണ്ടി (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			എങ്കിൽ (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	എങ്കിൽ (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	അല്ലെങ്കിൽ
		printf(" SIZE: %"PRIu64"\n", u64);

	എങ്കിൽ (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	അല്ലെങ്കിൽ
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, വലുപ്പം(path));
	printf(" SUBSYSTEMS:\n");

	ആകുമ്പോൾ (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		സ്വതന്ത്രം(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	തിരികെ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
