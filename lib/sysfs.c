/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <libgen.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "sysfs.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "buffer.h"

ସ୍ଥିର ଶୂନ୍ୟ sysfs_blkdev_deinit_path(ସଂରଚନା path_cxt *pc);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା  sysfs_blkdev_enoent_redirect(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
ସ୍ଥିର UL_DEBUG_DEFINE_MASK(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULSYSFS_DEBUG_INIT	(1 << 1)
#define ULSYSFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

ଶୂନ୍ୟ ul_sysfs_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (ulsysfs_debug_mask)
		ଫେରନ୍ତୁ;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

ସଂରଚନା path_cxt *ul_new_sysfs_path(dev_t devno, ସଂରଚନା path_cxt *parent, ଅଚଳ ଅକ୍ଷର *prefix)
{
	ସଂରଚନା path_cxt *pc = ul_new_path(ଖାଲି);

	ଯଦି (!pc)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (prefix)
		ul_path_set_prefix(pc, prefix);

	ଯଦି (sysfs_blkdev_init_path(pc, devno, parent) != 0) {
		ul_unref_path(pc);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ଫେରନ୍ତୁ pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_init_path(ସଂରଚନା path_cxt *pc, dev_t devno, ସଂରଚନା path_cxt *parent)
{
	ସଂରଚନା sysfs_blkdev *blk;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅକ୍ଷର buf[ଆକାର(_PATH_SYS_DEVBLOCK)
		 + ଆକାର(stringify_ମୂଲ୍ୟ(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, ଆକାର(buf), _PATH_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_path_set_dir(pc, buf);
	ଯଦି (rc)
		ଫେରନ୍ତୁ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_path_get_dialect(pc);
	ଯଦି (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = calloc(1, ଆକାର(ସଂରଚନା sysfs_blkdev));
		ଯଦି (!blk)
			ଫେରନ୍ତୁ -ENOMEM;

		ul_path_set_dialect(pc, blk, sysfs_blkdev_deinit_path);
		ul_path_set_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_set_parent(pc, parent);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ sysfs_blkdev_deinit_path(ସଂରଚନା path_cxt *pc)
{
	ସଂରଚନା sysfs_blkdev *blk;

	ଯଦି (!pc)
		ଫେରନ୍ତୁ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_path_get_dialect(pc);
	ଯଦି (!blk)
		ଫେରନ୍ତୁ;

	ul_unref_path(blk->parent);
	ମୁକ୍ତ(blk);

	ul_path_set_dialect(pc, ଖାଲି, ଖାଲି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_set_parent(ସଂରଚନା path_cxt *pc, ସଂରଚନା path_cxt *parent)
{
	ସଂରଚନା sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ଯଦି (!pc || !blk)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (blk->parent) {
		ul_unref_path(blk->parent);
		blk->parent = ଖାଲି;
	}

	ଯଦି (parent) {
		ul_ref_path(parent);
		blk->parent = parent;
	} ଅନ୍ୟଥା
		blk->parent = ଖାଲି;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	ଫେରନ୍ତୁ 0;
}

ସଂରଚନା path_cxt *sysfs_blkdev_get_parent(ସଂରଚନା path_cxt *pc)
{
	ସଂରଚନା sysfs_blkdev *blk = ul_path_get_dialect(pc);
	ଫେରନ୍ତୁ blk ? blk->parent : ଖାଲି;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_enoent_redirect(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dirfd)
{
	ସଂରଚନା sysfs_blkdev *blk = ul_path_get_dialect(pc);

	ଯଦି (blk && blk->parent && path) {
		*dirfd = ul_path_get_dirfd(blk->parent);
		ଯଦି (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", path));
			ଫେରନ୍ତୁ 0;
		}
	}
	ଫେରନ୍ତୁ 1;	/* no redirect */
}

ଅକ୍ଷର *sysfs_blkdev_get_name(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ଅକ୍ଷର link[PATH_MAX];
	ଅକ୍ଷର *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_path_readlink(pc, link, ଆକାର(link), ଖାଲି);
	ଯଦି (sz < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	name = strrchr(link, '/');
	ଯଦି (!name)
		ଫେରନ୍ତୁ ଖାଲି;

	name++;
	sz = strlen(name);
	ଯଦି ((size_t) sz + 1 > bufsiz)
		ଫେରନ୍ତୁ ଖାଲି;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ଫେରନ୍ତୁ buf;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_is_partition_dirent(DIR *dir, ସଂରଚନା dirent *d, ଅଚଳ ଅକ୍ଷର *parent_name)
{
	ଅକ୍ଷର path[NAME_MAX + 6 + 1];

#ifdef _DIRENT_HAVE_D_TYPE
	ଯଦି (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		ଫେରନ୍ତୁ 0;
#endif
	size_t len = 0;

	ଯଦି (parent_name) {
		ଅଚଳ ଅକ୍ଷର *p = parent_name;

		/* /dev/sda --> "sda" */
		ଯଦି (*parent_name == '/') {
			p = strrchr(parent_name, '/');
			ଯଦି (!p)
				ଫେରନ୍ତୁ 0;
			p++;
		}

		len = strlen(p);
		ଯଦି ((strlen(d->d_name) <= len) || (strncmp(p, d->d_name, len) != 0))
			len = 0;
	}

	ଯଦି (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		ଫେରନ୍ତୁ ((*(d->d_name + len) == 'p' && isdigit(*(d->d_name + len + 1)))
			|| isdigit(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(path, ଆକାର(path), "%s/start", d->d_name);

	ଫେରନ୍ତୁ faccessat(dirfd(dir), path, R_OK, 0) == 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_ଗଣନା_partitions(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *devname)
{
	DIR *dir;
	ସଂରଚନା dirent *d;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r = 0;

	dir = ul_path_opendir(pc, ଖାଲି);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ 0;

	ଯେତେବେଳେ ((d = xreaddir(dir))) {
		ଯଦି (sysfs_blkdev_is_partition_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	ଫେରନ୍ତୁ r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_to_devno(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା partno)
{
	DIR *dir;
	ସଂରଚନା dirent *d;
	dev_t devno = 0;

	dir = ul_path_opendir(pc, ଖାଲି);
	ଯଦି (!dir)
		ଫେରନ୍ତୁ 0;

	ଯେତେବେଳେ ((d = xreaddir(dir))) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

		ଯଦି (!sysfs_blkdev_is_partition_dirent(dir, d, ଖାଲି))
			ଜାରି;

		ଯଦି (ul_path_readf_s32(pc, &n, "%s/partition", d->d_name))
			ଜାରି;

		ଯଦି (n == partno) {
			ଯଦି (ul_path_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ଭାଙ୍ଗ;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) partno, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) devno));
	ଫେରନ୍ତୁ devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
ଅକ୍ଷର *sysfs_blkdev_get_slave(ସଂରଚନା path_cxt *pc)
{
	DIR *dir;
	ସଂରଚନା dirent *d;
	ଅକ୍ଷର *name = ଖାଲି;

	dir = ul_path_opendir(pc, "slaves");
	ଯଦି (!dir)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯେତେବେଳେ ((d = xreaddir(dir))) {
		ଯଦି (name)
			ଯାଅ err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	ଫେରନ୍ତୁ name;
err:
	ମୁକ୍ତ(name);
	closedir(dir);
	ଫେରନ୍ତୁ ଖାଲି;
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
ସ୍ଥିର ଅକ୍ଷର *get_subsystem(ଅକ୍ଷର *chain, ଅକ୍ଷର *buf, size_t bufsz)
{
	size_t len;
	ଅକ୍ଷର *p;

	ଯଦି (!chain || !*chain)
		ଫେରନ୍ତୁ ଖାଲି;

	len = strlen(chain);
	ଯଦି (len + ଆକାର(SUBSYSTEM_LINKNAME) > PATH_MAX)
		ଫେରନ୍ତୁ ଖାଲି;

	କର {
		ssize_t sz;

		/* append "/subsystem" to the path */
		memcpy(chain + len, SUBSYSTEM_LINKNAME, ଆକାର(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ଯଦି (p) {
			*p = '\0';
			len = p - chain;
		}

		ଯଦି (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			ଫେରନ୍ତୁ basename(buf);
		}

	} ଯେତେବେଳେ (p);

	ଫେରନ୍ତୁ ଖାଲି;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
ଅକ୍ଷର *sysfs_blkdev_get_devchain(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	ସଂରଚନା ul_buffer tmp = UL_INIT_BUFFER;
	ଅଚଳ ଅକ୍ଷର *p;
	ଅକ୍ଷର *res = ଖାଲି;

	ssz = ul_path_readlink(pc, buf, bufsz, ଖାଲି);
	ଯଦି (ssz <= 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି ((p = ul_path_get_prefix(pc)))
		ul_buffer_append_string(&tmp, p);

	ul_buffer_append_string(&tmp, _PATH_SYS_DEVBLOCK "/");
	ul_buffer_append_data(&tmp, buf, ssz);

	p = ul_buffer_get_string(&tmp, &sz, ଖାଲି);
	ଯଦି (p && sz <= bufsz) {
		memcpy(buf, p, sz);
		res = buf;
	}
	ul_buffer_free_data(&tmp);
	ଫେରନ୍ତୁ res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_next_subsystem(ସଂରଚନା path_cxt *pc __attribute__((unused)),
			 ଅକ୍ଷର *devchain, ଅକ୍ଷର **subsys)
{
	ଅକ୍ଷର subbuf[PATH_MAX];
	ଅକ୍ଷର *sub;

	ଯଦି (!subsys || !devchain)
		ଫେରନ୍ତୁ -EINVAL;

	*subsys = ଖାଲି;

	ଯେତେବେଳେ ((sub = get_subsystem(devchain, subbuf, ଆକାର(subbuf)))) {
		*subsys = strdup(sub);
		ଯଦି (!*subsys)
			ଫେରନ୍ତୁ -ENOMEM;
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ 1;
}

#define REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devchain_is_removable(ଅକ୍ଷର *chain)
{
	size_t len;
	ଅକ୍ଷର buf[20];
	ଅକ୍ଷର *p;

	ଯଦି (!chain || !*chain)
		ଫେରନ୍ତୁ 0;

	len = strlen(chain);
	ଯଦି (len + ଆକାର(REMOVABLE_FILENAME) > PATH_MAX)
		ଫେରନ୍ତୁ 0;

	କର {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, rc;

		/* append "/removable" to the path */
		memcpy(chain + len, REMOVABLE_FILENAME, ଆକାର(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ଯଦି (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ଭାଙ୍ଗ;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ଯଦି (fd != -1) {
			rc = ul_read_all(fd, buf, ଆକାର(buf));
			close(fd);

			ଯଦି (rc > 0) {
				ଯଦି (strncmp(buf, "fixed", min(rc, 5)) == 0) {
					ଫେରନ୍ତୁ 0;
				} ଅନ୍ୟଥା ଯଦି (strncmp(buf, "removable", min(rc, 9)) == 0) {
					ଫେରନ୍ତୁ 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ଯଦି (p) {
			*p = '\0';
			len = p - chain;
		}

	} ଯେତେବେଳେ (p);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_is_hotpluggable(ସଂରଚନା path_cxt *pc)
{
	ଅକ୍ଷର buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, ଆକାର(buf));
	ଫେରନ୍ତୁ sysfs_devchain_is_removable(chain);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_is_removable(ସଂରଚନା path_cxt *pc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ଯଦି (ul_path_read_s32(pc, &rc, "removable") == 0)
		ଫେରନ୍ତୁ rc;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_dm_wholedisk(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *diskname,
                            size_t len, dev_t *diskdevno)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
	ଅକ୍ଷର *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ଯଦି (!name)
		ଫେରନ୍ତୁ -1;

	ଯଦି (diskname && len)
		xstrncpy(diskname, name, len);

	ଯଦି (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ଖାଲି);
		ଯଦି (!*diskdevno)
			rc = -1;
	}

	ମୁକ୍ତ(name);
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_get_wholedisk(	ସଂରଚନା path_cxt *pc,
				ଅକ୍ଷର *diskname,
				size_t len,
				dev_t *diskdevno)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_part = 0;

	ଯଦି (!pc)
		ଫେରନ୍ତୁ -1;

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	ଯଦି (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		ଅକ୍ଷର *uuid = ଖାଲି, *tmp, *prefix;

		ul_path_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		prefix = uuid ? strsep(&tmp, "-") : ଖାଲି;

		ଯଦି (prefix && c_strncasecmp(prefix, "part", 4) == 0)
			is_part = 1;
		ମୁକ୍ତ(uuid);

		ଯଦି (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			ଯାଅ done;

		is_part = 0;
	}

	ଯଦି (!is_part) {
		/*
		 * unpartitioned device
		 */
		ଯଦି (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			ଯାଅ err;
		ଯଦି (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} ଅନ୍ୟଥା {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		ଅକ୍ଷର linkpath[PATH_MAX];
		ଅକ୍ଷର *name;
		ssize_t	linklen;

		linklen = ul_path_readlink(pc, linkpath, ଆକାର(linkpath), ଖାଲି);
		ଯଦି (linklen < 0)
			ଯାଅ err;

		stripoff_last_component(linkpath);	  /* dirname */
		name = stripoff_last_component(linkpath);   /* basename */
		ଯଦି (!name)
			ଯାଅ err;

		sysfs_devname_sys_to_dev(name);
		ଯଦି (diskname && len)
			xstrncpy(diskname, name, len);

		ଯଦି (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_path_get_prefix(pc), name, ଖାଲି);
			ଯଦି (!*diskdevno)
				ଯାଅ err;
		}
	}

done:
	ଫେରନ୍ତୁ 0;
err:
	ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devno_to_wholedisk(dev_t devno, ଅକ୍ଷର *diskname,
		             size_t len, dev_t *diskdevno)
{
	ସଂରଚନା path_cxt *pc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (!devno)
		ଫେରନ୍ତୁ -EINVAL;
	pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଯଦି (!pc)
		ଫେରନ୍ତୁ -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_path(pc);
	ଫେରନ୍ତୁ rc;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devno_is_dm_hidden(dev_t devno, ଅକ୍ଷର **uuid,
			     ଅଚଳ ସଂରଚନା ul_vfs_ops *vfs)
{
	ସଂରଚନା path_cxt *pc = ଖାଲି;
	ଅକ୍ଷର *id = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଯଦି (!pc)
		ଯାଅ done;
	ul_path_refer_vfs(pc, vfs);
	ଯଦି (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ଯାଅ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ଯଦି (strncmp(id, "LVM-", 4) == 0) {
		ଅକ୍ଷର *p = strrchr(id + 4, '-');

		ଯଦି (p && *(p + 1))
			rc = 1;
	}
done:
	ul_unref_path(pc);
	ଯଦି (uuid)
		*uuid = id;
	ଅନ୍ୟଥା
		ମୁକ୍ତ(id);
	ଫେରନ୍ତୁ rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devno_is_dm_private(dev_t devno, ଅକ୍ଷର **uuid,
			      ଅଚଳ ସଂରଚନା ul_vfs_ops *vfs)
{
	ସଂରଚନା path_cxt *pc = ଖାଲି;
	ଅକ୍ଷର *id = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଯଦି (!pc)
		ଯାଅ done;
	ul_path_refer_vfs(pc, vfs);
	ଯଦି (ul_path_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		ଯାଅ done;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ଯଦି (strncmp(id, "LVM-", 4) == 0) {
		ଅକ୍ଷର *p = strrchr(id + 4, '-');

		ଯଦି (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} ଅନ୍ୟଥା ଯଦି (strncmp(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
done:
	ul_unref_path(pc);
	ଯଦି (uuid)
		*uuid = id;
	ଅନ୍ୟଥା
		ମୁକ୍ତ(id);
	ଫେରନ୍ତୁ rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ଯଦି (sysfs_devno_to_wholedisk(devno, ଖାଲି, 0, &disk) != 0)
		ଫେରନ୍ତୁ -1;

	ଫେରନ୍ତୁ devno == disk;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_scsi_get_hctl(ସଂରଚନା path_cxt *pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *h, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *l)
{
	ଅକ୍ଷର buf[PATH_MAX], *hctl;
	ସଂରଚନା sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_path_get_dialect(pc);

	ଯଦି (!blk || blk->hctl_error)
		ଫେରନ୍ତୁ -EINVAL;
	ଯଦି (blk->has_hctl)
		ଯାଅ done;

	blk->hctl_error = 1;
	len = ul_path_readlink(pc, buf, ଆକାର(buf), "device");
	ଯଦି (len < 0)
		ଫେରନ୍ତୁ len;

	hctl = strrchr(buf, '/');
	ଯଦି (!hctl)
		ଫେରନ୍ତୁ -1;
	hctl++;

	ଯଦି (sscanf(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_target, &blk->scsi_lun) != 4)
		ଫେରନ୍ତୁ -1;

	blk->has_hctl = 1;
done:
	ଯଦି (h)
		*h = blk->scsi_host;
	ଯଦି (c)
		*c = blk->scsi_channel;
	ଯଦି (t)
		*t = blk->scsi_target;
	ଯଦି (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ଅକ୍ଷର *scsi_host_attribute_path(
			ସଂରଚନା path_cxt *pc,
			ଅଚଳ ଅକ୍ଷର *type,
			ଅକ୍ଷର *buf,
			size_t bufsz,
			ଅଚଳ ଅକ୍ଷର *attr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା host;
	ଅଚଳ ଅକ୍ଷର *prefix;

	ଯଦି (sysfs_blkdev_scsi_get_hctl(pc, &host, ଖାଲି, ଖାଲି, ଖାଲି))
		ଫେରନ୍ତୁ ଖାଲି;

	prefix = ul_path_get_prefix(pc);
	ଯଦି (!prefix)
		prefix = "";

	ଯଦି (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				prefix, _PATH_SYS_CLASS, type, host, attr);
	ଅନ୍ୟଥା
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				prefix, _PATH_SYS_CLASS, type, host);

	ଫେରନ୍ତୁ (len < 0 || (size_t) len >= bufsz) ? ଖାଲି : buf;
}

ଅକ୍ଷର *sysfs_blkdev_scsi_host_strdup_attribute(ସଂରଚନା path_cxt *pc,
			ଅଚଳ ଅକ୍ଷର *type, ଅଚଳ ଅକ୍ଷର *attr)
{
	ଅକ୍ଷର buf[1024];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	FILE *f;

	ଯଦି (!attr || !type ||
	    !scsi_host_attribute_path(pc, type, buf, ଆକାର(buf), attr))
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		ଫେରନ୍ତୁ ଖାଲି;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	ଫେରନ୍ତୁ rc == 1 ? strdup(buf) : ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_scsi_host_is(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *type)
{
	ଅକ୍ଷର buf[PATH_MAX];
	ସଂରଚନା stat st;

	ଯଦି (!type || !scsi_host_attribute_path(pc, type,
				buf, ଆକାର(buf), ଖାଲି))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

ସ୍ଥିର ଅକ୍ଷର *scsi_attribute_path(ସଂରଚନା path_cxt *pc,
		ଅକ୍ଷର *buf, size_t bufsz, ଅଚଳ ଅକ୍ଷର *attr)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, h, c, t, l;
	ଅଚଳ ଅକ୍ଷର *prefix;

	ଯଦି (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		ଫେରନ୍ତୁ ଖାଲି;

	prefix = ul_path_get_prefix(pc);
	ଯଦି (!prefix)
		prefix = "";

	ଯଦି (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l, attr);
	ଅନ୍ୟଥା
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				prefix, _PATH_SYS_SCSI,
				h,c,t,l);
	ଫେରନ୍ତୁ (len < 0 || (size_t) len >= bufsz) ? ଖାଲି : buf;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_scsi_has_attribute(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *attr)
{
	ଅକ୍ଷର path[PATH_MAX];
	ସଂରଚନା stat st;

	ଯଦି (!scsi_attribute_path(pc, path, ଆକାର(path), attr))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ stat(path, &st) == 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_blkdev_scsi_path_contains(ସଂରଚନା path_cxt *pc, ଅଚଳ ଅକ୍ଷର *pattern)
{
	ଅକ୍ଷର path[PATH_MAX], linkc[PATH_MAX];
	ସଂରଚନା stat st;
	ssize_t len;

	ଯଦି (!scsi_attribute_path(pc, path, ଆକାର(path), ଖାଲି))
		ଫେରନ୍ତୁ 0;

	ଯଦି (stat(path, &st) != 0)
		ଫେରନ୍ତୁ 0;

	len = readlink(path, linkc, ଆକାର(linkc) - 1);
	ଯଦି (len < 0)
		ଫେରନ୍ତୁ 0;

	linkc[len] = '\0';
	ଫେରନ୍ତୁ strstr(linkc, pattern) != ଖାଲି;
}

ସ୍ଥିର dev_t read_devno(ଅଚଳ ଅକ୍ଷର *path)
{
	FILE *f;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(path, "r" UL_CLOEXECSTR);
	ଯଦି (!f)
		ଫେରନ୍ତୁ 0;

	ଯଦି (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	ଫେରନ୍ତୁ dev;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devname_is_hidden(ଅଚଳ ଅକ୍ଷର *prefix, ଅଚଳ ଅକ୍ଷର *name)
{
	ଅକ୍ଷର buf[PATH_MAX];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0, hidden = 0, len;
	FILE *f;

	ଯଦି (strncmp("/dev/", name, 5) == 0)
		ଫେରନ୍ତୁ 0;

	ଯଦି (!prefix)
		prefix = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, ଆକାର(buf),
			"%s" _PATH_SYS_BLOCK "/%s/hidden",
			prefix, name);

	ଯଦି (len < 0 || (size_t) len + 1 > ଆକାର(buf))
		ଫେରନ୍ତୁ 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ଯଦି (!f)
		ଫେରନ୍ତୁ 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	ଫେରନ୍ତୁ rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(ଅଚଳ ଅକ୍ଷର *prefix, ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *parent)
{
	ଅକ୍ଷର buf[PATH_MAX];
	ଅକ୍ଷର *_name = ଖାଲି, *_parent = ଖାଲି;	/* name as encoded in sysfs */
	dev_t dev = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	ଯଦି (!prefix)
		prefix = "";

	ପ୍ରତିଜ୍ଞା(name);

	ଯଦି (strncmp("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		ସଂରଚନା stat st;

		ଯଦି (stat(name, &st) == 0) {
			dev = st.st_rdev;
			ଯାଅ done;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ଯଦି (!_name)
		ଯାଅ done;
	sysfs_devname_dev_to_sys(_name);

	ଯଦି (parent) {
		_parent = strdup(parent);
		ଯଦି (!_parent)
			ଯାଅ done;
	}

	ଯଦି (parent && strncmp("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_to_sys(_parent);
		len = snprintf(buf, ଆକାର(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix,	_parent, _name);
		ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
			ଯାଅ done;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		ଯାଅ done;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, ଆକାର(buf),
			"%s" _PATH_SYS_BLOCK "/%s/dev",
			prefix, _name);
	ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
		ଯାଅ done;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ଯଦି (!dev && parent && ul_startswith(name, parent)) {
		len = snprintf(buf, ଆକାର(buf),
				"%s" _PATH_SYS_BLOCK "/%s/%s/dev",
				prefix, _parent, _name);
		ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
			ଯାଅ done;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ଯଦି (!dev) {
		len = snprintf(buf, ଆକାର(buf),
				"%s" _PATH_SYS_BLOCK "/%s/device/dev",
				prefix, _name);
		ଯଦି (len < 0 || (size_t) len >= ଆକାର(buf))
			ଯାଅ done;
		dev = read_devno(buf);
	}
done:
	ମୁକ୍ତ(_name);
	ମୁକ୍ତ(_parent);
	ଫେରନ୍ତୁ dev;
}

dev_t sysfs_devname_to_devno(ଅଚଳ ଅକ୍ଷର *name)
{
	ଫେରନ୍ତୁ __sysfs_devname_to_devno(ଖାଲି, name, ଖାଲି);
}

ଅକ୍ଷର *sysfs_blkdev_get_path(ସଂରଚନା path_cxt *pc, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ଅଚଳ ଅକ୍ଷର *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	ଅକ୍ଷର *res = ଖାଲି;
	size_t sz;
	ସଂରଚନା stat st;

	ଯଦି (!name)
		ଯାଅ done;

	sz = strlen(name);
	ଯଦି (sz + ଆକାର("/dev/") > bufsiz)
		ଯାଅ done;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	memcpy(buf, "/dev/", 5);

	ଯଦି (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
done:
	ଫେରନ୍ତୁ res;
}

dev_t sysfs_blkdev_get_devno(ସଂରଚନା path_cxt *pc)
{
	ଫେରନ୍ତୁ ((ସଂରଚନା sysfs_blkdev *) ul_path_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
ଅକ୍ଷର *sysfs_devno_to_devpath(dev_t devno, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ସଂରଚନା path_cxt *pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (pc) {
		res = sysfs_blkdev_get_path(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ଫେରନ୍ତୁ res;
}

ଅକ୍ଷର *sysfs_devno_to_devname(dev_t devno, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ସଂରଚନା path_cxt *pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଅକ୍ଷର *res = ଖାଲି;

	ଯଦି (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_path(pc);
	}
	ଫେରନ୍ତୁ res;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_devno_ଗଣନା_partitions(dev_t devno)
{
	ସଂରଚନା path_cxt *pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;

	ଯଦି (pc) {
		ଅକ୍ଷର buf[PATH_MAX + 1];
		ଅକ୍ଷର *name = sysfs_blkdev_get_name(pc, buf, ଆକାର(buf));

		n = sysfs_blkdev_ଗଣନା_partitions(pc, name);
		ul_unref_path(pc);
	}
	ଫେରନ୍ତୁ n;
}

ଅକ୍ଷର *sysfs_chrdev_devno_to_devname(dev_t devno, ଅକ୍ଷର *buf, size_t bufsiz)
{
	ଅକ୍ଷର link[PATH_MAX];
	ସଂରଚନା path_cxt *pc;
	ଅକ୍ଷର *name;
	ssize_t	sz;

	pc = ul_new_path(_PATH_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ଯଦି (!pc)
		ଫେରନ୍ତୁ ଖାଲି;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_path_readlink(pc, link, ଆକାର(link), ଖାଲି);
	ul_unref_path(pc);

	ଯଦି (sz < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	name = strrchr(link, '/');
	ଯଦି (!name)
		ଫେରନ୍ତୁ ଖାଲି;

	name++;
	sz = strlen(name);
	ଯଦି ((size_t) sz + 1 > bufsiz)
		ଫେରନ୍ତୁ ଖାଲି;

	memcpy(buf, name, sz + 1);
	sysfs_devname_sys_to_dev(buf);
	ଫେରନ୍ତୁ buf;

}

ଗଣନା sysfs_byteorder sysfs_get_byteorder(ସଂରଚନା path_cxt *pc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅକ୍ଷର buf[BUFSIZ];
	ଗଣନା sysfs_byteorder ret;

	rc = ul_path_read_buffer(pc, buf, ଆକାର(buf), _PATH_SYS_CPU_BYTEORDER);
	ଯଦି (rc < 0)
		ଯାଅ unknown;

	ଯଦି (strncmp(buf, "little", ଆକାର(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		ଯାଅ out;
	} ଅନ୍ୟଥା ଯଦି (strncmp(buf, "big", ଆକାର(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		ଯାଅ out;
	}

unknown:
#ଯଦି __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#ନଚେତଯଦି __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#ଅନ୍ୟଥା
#error Unknown byte order
#endif

out:
	ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sysfs_get_address_bits(ସଂରଚନା path_cxt *pc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା address_bits;

	rc = ul_path_scanf(pc, _PATH_SYS_ADDRESS_BITS, "%d", &address_bits);
	ଯଦି (rc < 0)
		ଫେରନ୍ତୁ rc;
	ଯଦି (address_bits < 0)
		ଫେରନ୍ତୁ -EINVAL;
	ଫେରନ୍ତୁ address_bits;
}


#ifdef TEST_PROGRAM_SYSFS
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା path_cxt *pc;
	ଅକ୍ଷର *devname;
	dev_t devno, disk_devno;
	ଅକ୍ଷର path[PATH_MAX], *sub, *chain;
	ଅକ୍ଷର diskname[32];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ଯଦି (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_to_devno(devname);

	ଯଦି (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	printf("non-context:\n");
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_devno_to_devname(devno, path, ଆକାର(path)));
	printf(" DEVPATH: %s\n", sysfs_devno_to_devpath(devno, path, ଆକାର(path)));

	sysfs_devno_to_wholedisk(devno, diskname, ଆକାର(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_path(devno, ଖାଲି, ଖାଲି);
	ଯଦି (!pc)
		ଯାଅ done;

	printf("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	printf(" DEVNO:   %u (%u:%u)\n", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) devno, major(devno), minor(devno));
	printf(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, path, ଆକାର(path)));
	printf(" DEVPATH: %s\n", sysfs_blkdev_get_path(pc, path, ଆକାର(path)));

	sysfs_devno_to_wholedisk(devno, diskname, ଆକାର(diskname), &disk_devno);
	printf(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) disk_devno, major(disk_devno), minor(disk_devno));
	printf(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_path_access(pc, F_OK, "partition") == 0;
	printf(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ଯଦି (is_part && disk_devno) {
		ସଂରଚନା path_cxt *disk_pc =  ul_new_sysfs_path(disk_devno, ଖାଲି, ଖାଲି);
		sysfs_blkdev_set_parent(pc, disk_pc);

		ul_unref_path(disk_pc);
	}

	printf(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	printf(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	printf(" SLAVES: %d\n", ul_path_ଗଣନା_dirents(pc, "slaves"));

	ଯଦି (!is_part) {
		printf("First 5 partitions:\n");
		ପାଇଁ (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_to_devno(pc, i);
			ଯଦି (dev)
				printf("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ଯଦି (ul_path_read_u64(pc, &u64, "size") != 0)
		printf(" (!) read SIZE failed\n");
	ଅନ୍ୟଥା
		printf(" SIZE: %"PRIu64"\n", u64);

	ଯଦି (ul_path_read_s32(pc, &i, "queue/hw_sector_size"))
		printf(" (!) read SECTOR failed\n");
	ଅନ୍ୟଥା
		printf(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, path, ଆକାର(path));
	printf(" SUBSYSTEMS:\n");

	ଯେତେବେଳେ (chain && sysfs_blkdev_next_subsystem(pc, chain, &sub) == 0) {
		printf("\t%s\n", sub);
		ମୁକ୍ତ(sub);
	}

	rc = EXIT_SUCCESS;
done:
	ul_unref_path(pc);
	ଫେରନ୍ତୁ rc;
}
#endif /* TEST_PROGRAM_SYSFS */
