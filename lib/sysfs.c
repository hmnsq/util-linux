/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2011]
 */
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <libgen.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "sysfs.h"
#అన్తర్భావ "fileutils.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "debug.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "buffer.h"

స్థిర శూన్య sysfs_blkdev_deinit_మార్గం(నిర్మాణం మార్గం_cxt *pc);
స్థిర పూర్ణాంకం  sysfs_blkdev_enoent_redirect(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం *dirfd);

/*
 * Debug stuff (based on include/debug.h)
 */
స్థిర UL_DEBUG_DEFINE_ముసుగు(ulsysfs);
UL_DEBUG_DEFINE_MASKNAMES(ulsysfs) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు ULSYSFS_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULSYSFS_DEBUG_CXT	(1 << 2)

#నిర్వచించు DBG(m, x)		__UL_DBG(ulsysfs, ULSYSFS_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulsysfs, ULSYSFS_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(ulsysfs, ULSYSFS_DEBUG_, m, x)

శూన్య ul_sysfs_init_debug(శూన్య)
{
	ఒకవేళ (ulsysfs_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(ulsysfs, ULSYSFS_DEBUG_, 0, ULSYSFS_DEBUG);
}

నిర్మాణం మార్గం_cxt *ul_new_sysfs_మార్గం(dev_t devno, నిర్మాణం మార్గం_cxt *మాతృ, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	నిర్మాణం మార్గం_cxt *pc = ul_new_మార్గం(ఖాళీ);

	ఒకవేళ (!pc)
		తిరిగి ఖాళీ;
	ఒకవేళ (ఉపసర్గ)
		ul_మార్గం_సమితి_ఉపసర్గ(pc, ఉపసర్గ);

	ఒకవేళ (sysfs_blkdev_init_మార్గం(pc, devno, మాతృ) != 0) {
		ul_unref_మార్గం(pc);
		తిరిగి ఖాళీ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	తిరిగి pc;
}

/*
 * sysfs_blkdev_* is sysfs extension to ul_path_* API for block devices.
 *
 * The function is possible to call in loop and without sysfs_blkdev_deinit_path().
 * The sysfs_blkdev_deinit_path() is automatically called by ul_unref_path().
 *
 */
పూర్ణాంకం sysfs_blkdev_init_మార్గం(నిర్మాణం మార్గం_cxt *pc, dev_t devno, నిర్మాణం మార్గం_cxt *మాతృ)
{
	నిర్మాణం sysfs_blkdev *blk;
	పూర్ణాంకం rc;
	అక్షరం buf[పరిమాణం(మార్గం_SYS_DEVBLOCK)
		 + పరిమాణం(stringify_విలువ(UINT32_MAX)) * 2
		 + 3];

	/* define path to devno stuff */
	snprintf(buf, పరిమాణం(buf), మార్గం_SYS_DEVBLOCK "/%u:%u", major(devno), minor(devno));
	rc = ul_మార్గం_సమితి_dir(pc, buf);
	ఒకవేళ (rc)
		తిరిగి rc;

	/* make sure path exists */
	rc = ul_మార్గం_get_dirfd(pc);
	ఒకవేళ (rc < 0)
		తిరిగి rc;

	/* initialize sysfs blkdev specific stuff */
	blk = ul_మార్గం_get_dialect(pc);
	ఒకవేళ (!blk) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new sysfs handler"));
		blk = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం sysfs_blkdev));
		ఒకవేళ (!blk)
			తిరిగి -ENOMEM;

		ul_మార్గం_సమితి_dialect(pc, blk, sysfs_blkdev_deinit_మార్గం);
		ul_మార్గం_సమితి_enoent_redirect(pc, sysfs_blkdev_enoent_redirect);
	}

	DBG_OBJ(CXT, pc, ul_debug("init sysfs stuff"));

	blk->devno = devno;
	sysfs_blkdev_సమితి_మాతృ(pc, మాతృ);

	తిరిగి 0;
}

స్థిర శూన్య sysfs_blkdev_deinit_మార్గం(నిర్మాణం మార్గం_cxt *pc)
{
	నిర్మాణం sysfs_blkdev *blk;

	ఒకవేళ (!pc)
		తిరిగి;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	blk = ul_మార్గం_get_dialect(pc);
	ఒకవేళ (!blk)
		తిరిగి;

	ul_unref_మార్గం(blk->మాతృ);
	విడుదల(blk);

	ul_మార్గం_సమితి_dialect(pc, ఖాళీ, ఖాళీ);
}

పూర్ణాంకం sysfs_blkdev_సమితి_మాతృ(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం మార్గం_cxt *మాతృ)
{
	నిర్మాణం sysfs_blkdev *blk = ul_మార్గం_get_dialect(pc);

	ఒకవేళ (!pc || !blk)
		తిరిగి -EINVAL;

	ఒకవేళ (blk->మాతృ) {
		ul_unref_మార్గం(blk->మాతృ);
		blk->మాతృ = ఖాళీ;
	}

	ఒకవేళ (మాతృ) {
		ul_ref_మార్గం(మాతృ);
		blk->మాతృ = మాతృ;
	} లేకపోతే
		blk->మాతృ = ఖాళీ;

	DBG_OBJ(CXT, pc, ul_debug("new parent"));
	తిరిగి 0;
}

నిర్మాణం మార్గం_cxt *sysfs_blkdev_get_మాతృ(నిర్మాణం మార్గం_cxt *pc)
{
	నిర్మాణం sysfs_blkdev *blk = ul_మార్గం_get_dialect(pc);
	తిరిగి blk ? blk->మాతృ : ఖాళీ;
}

/*
 * Redirects ENOENT errors to the parent.
 * For example
 *
 *	/sys/dev/block/8:1/queue/logical_block_size redirects to
 *	/sys/dev/block/8:0/queue/logical_block_size
 */
స్థిర పూర్ణాంకం sysfs_blkdev_enoent_redirect(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, పూర్ణాంకం *dirfd)
{
	నిర్మాణం sysfs_blkdev *blk = ul_మార్గం_get_dialect(pc);

	ఒకవేళ (blk && blk->మాతృ && మార్గం) {
		*dirfd = ul_మార్గం_get_dirfd(blk->మాతృ);
		ఒకవేళ (*dirfd >= 0) {
			DBG_OBJ(CXT, pc, ul_debug("%s redirected to parent", మార్గం));
			తిరిగి 0;
		}
	}
	తిరిగి 1;	/* no redirect */
}

అక్షరం *sysfs_blkdev_get_name(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsiz)
{
	అక్షరం link[PATH_MAX];
	అక్షరం *name;
	ssize_t	sz;

	/* read /sys/dev/block/<maj:min> link */
	sz = ul_మార్గం_readlink(pc, link, పరిమాణం(link), ఖాళీ);
	ఒకవేళ (sz < 0)
		తిరిగి ఖాళీ;

	name = strrchr(link, '/');
	ఒకవేళ (!name)
		తిరిగి ఖాళీ;

	name++;
	sz = అక్షరశ్రేణి_పొడవు(name);
	ఒకవేళ ((size_t) sz + 1 > bufsiz)
		తిరిగి ఖాళీ;

	జ్ఞాపకం_నకలు(buf, name, sz + 1);
	sysfs_devname_sys_గమ్యం_dev(buf);
	తిరిగి buf;
}

పూర్ణాంకం sysfs_blkdev_is_విభజన_dirent(DIR *dir, నిర్మాణం dirent *d, స్థిరరాశి అక్షరం *మాతృ_name)
{
	అక్షరం మార్గం[NAME_MAX + 6 + 1];

#నిర్వచితమైతే _DIRENT_HAVE_D_TYPE
	ఒకవేళ (d->d_type != DT_DIR &&
	    d->d_type != DT_LNK &&
	    d->d_type != DT_UNKNOWN)
		తిరిగి 0;
#అయితే_ముగింపు
	size_t len = 0;

	ఒకవేళ (మాతృ_name) {
		స్థిరరాశి అక్షరం *p = మాతృ_name;

		/* /dev/sda --> "sda" */
		ఒకవేళ (*మాతృ_name == '/') {
			p = strrchr(మాతృ_name, '/');
			ఒకవేళ (!p)
				తిరిగి 0;
			p++;
		}

		len = అక్షరశ్రేణి_పొడవు(p);
		ఒకవేళ ((అక్షరశ్రేణి_పొడవు(d->d_name) <= len) || (అక్షరశ్రేణి_పోలిక(p, d->d_name, len) != 0))
			len = 0;
	}

	ఒకవేళ (len > 0) {
		/* partitions subdir name is
		 *	"<parent>[:digit:]" or "<parent>p[:digit:]"
		 */
		తిరిగి ((*(d->d_name + len) == 'p' && అంకెనా(*(d->d_name + len + 1)))
			|| అంకెనా(*(d->d_name + len)));
	}

	/* Cannot use /partition file, not supported on old sysfs */
	snprintf(మార్గం, పరిమాణం(మార్గం), "%s/start", d->d_name);

	తిరిగి faccessat(dirfd(dir), మార్గం, R_OK, 0) == 0;
}

పూర్ణాంకం sysfs_blkdev_లెక్క_partitions(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *devname)
{
	DIR *dir;
	నిర్మాణం dirent *d;
	పూర్ణాంకం r = 0;

	dir = ul_మార్గం_opendir(pc, ఖాళీ);
	ఒకవేళ (!dir)
		తిరిగి 0;

	అయితే ((d = xreaddir(dir))) {
		ఒకవేళ (sysfs_blkdev_is_విభజన_dirent(dir, d, devname))
			r++;
	}

	closedir(dir);
	తిరిగి r;
}

/*
 * Converts @partno (partition number) to devno of the partition.
 * The @pc handles wholedisk device.
 *
 * Note that this code does not expect any special format of the
 * partitions devnames.
 */
dev_t sysfs_blkdev_partno_గమ్యం_devno(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం partno)
{
	DIR *dir;
	నిర్మాణం dirent *d;
	dev_t devno = 0;

	dir = ul_మార్గం_opendir(pc, ఖాళీ);
	ఒకవేళ (!dir)
		తిరిగి 0;

	అయితే ((d = xreaddir(dir))) {
		పూర్ణాంకం n;

		ఒకవేళ (!sysfs_blkdev_is_విభజన_dirent(dir, d, ఖాళీ))
			కొనసాగు;

		ఒకవేళ (ul_మార్గం_readf_s32(pc, &n, "%s/partition", d->d_name))
			కొనసాగు;

		ఒకవేళ (n == partno) {
			ఒకవేళ (ul_మార్గం_readf_majmin(pc, &devno, "%s/dev", d->d_name) == 0)
				ఆపు;
		}
	}

	closedir(dir);
	DBG_OBJ(CXT, pc, ul_debug("partno (%d) -> devno (%d)", (పూర్ణాంకం) partno, (పూర్ణాంకం) devno));
	తిరిగి devno;
}


/*
 * Returns slave name if there is only one slave, otherwise returns NULL.
 * The result should be deallocated by free().
 */
అక్షరం *sysfs_blkdev_get_slave(నిర్మాణం మార్గం_cxt *pc)
{
	DIR *dir;
	నిర్మాణం dirent *d;
	అక్షరం *name = ఖాళీ;

	dir = ul_మార్గం_opendir(pc, "slaves");
	ఒకవేళ (!dir)
		తిరిగి ఖాళీ;

	అయితే ((d = xreaddir(dir))) {
		ఒకవేళ (name)
			వెళ్ళు err;	/* more slaves */
		name = strdup(d->d_name);
	}

	closedir(dir);
	తిరిగి name;
err:
	విడుదల(name);
	closedir(dir);
	తిరిగి ఖాళీ;
}


#నిర్వచించు SUBSYSTEM_LINKNAME	"/subsystem"

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
స్థిర అక్షరం *get_subsystem(అక్షరం *chain, అక్షరం *buf, size_t bufsz)
{
	size_t len;
	అక్షరం *p;

	ఒకవేళ (!chain || !*chain)
		తిరిగి ఖాళీ;

	len = అక్షరశ్రేణి_పొడవు(chain);
	ఒకవేళ (len + పరిమాణం(SUBSYSTEM_LINKNAME) > PATH_MAX)
		తిరిగి ఖాళీ;

	చేయి {
		ssize_t sz;

		/* append "/subsystem" to the path */
		జ్ఞాపకం_నకలు(chain + len, SUBSYSTEM_LINKNAME, పరిమాణం(SUBSYSTEM_LINKNAME));

		/* try if subsystem symlink exists */
		sz = readlink(chain, buf, bufsz - 1);

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ఒకవేళ (p) {
			*p = '\0';
			len = p - chain;
		}

		ఒకవేళ (sz > 0) {
			/* we found symlink to subsystem, return basename */
			buf[sz] = '\0';
			తిరిగి basename(buf);
		}

	} అయితే (p);

	తిరిగి ఖాళీ;
}

/*
 * Returns complete path to the device, the path contains all subsystems
 * used for the device.
 */
అక్షరం *sysfs_blkdev_get_devchain(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz)
{
	/* read /sys/dev/block/<maj>:<min> symlink */
	ssize_t ssz;
	size_t sz = 0;
	నిర్మాణం ul_బఫరు tmp = UL_INIT_బఫరు;
	స్థిరరాశి అక్షరం *p;
	అక్షరం *res = ఖాళీ;

	ssz = ul_మార్గం_readlink(pc, buf, bufsz, ఖాళీ);
	ఒకవేళ (ssz <= 0)
		తిరిగి ఖాళీ;

	ఒకవేళ ((p = ul_మార్గం_get_ఉపసర్గ(pc)))
		ul_బఫరు_append_string(&tmp, p);

	ul_బఫరు_append_string(&tmp, మార్గం_SYS_DEVBLOCK "/");
	ul_బఫరు_append_దత్తాంశం(&tmp, buf, ssz);

	p = ul_బఫరు_get_string(&tmp, &sz, ఖాళీ);
	ఒకవేళ (p && sz <= bufsz) {
		జ్ఞాపకం_నకలు(buf, p, sz);
		res = buf;
	}
	ul_బఫరు_free_దత్తాంశం(&tmp);
	తిరిగి res;
}

/*
 * The @subsys returns the next subsystem in the chain. Function modifies
 * @devchain string.
 *
 * Returns: 0 in success, <0 on error, 1 on end of chain
 */
పూర్ణాంకం sysfs_blkdev_తదుపరి_subsystem(నిర్మాణం మార్గం_cxt *pc __attribute__((వాడని)),
			 అక్షరం *devchain, అక్షరం **subsys)
{
	అక్షరం subbuf[PATH_MAX];
	అక్షరం *sub;

	ఒకవేళ (!subsys || !devchain)
		తిరిగి -EINVAL;

	*subsys = ఖాళీ;

	అయితే ((sub = get_subsystem(devchain, subbuf, పరిమాణం(subbuf)))) {
		*subsys = strdup(sub);
		ఒకవేళ (!*subsys)
			తిరిగి -ENOMEM;
		తిరిగి 0;
	}

	తిరిగి 1;
}

#నిర్వచించు REMOVABLE_FILENAME	"/removable"

/*
 * For example:
 *
 * chain: /sys/dev/block/../../devices/pci0000:00/0000:00:1a.0/usb1/1-1/1-1.2/ \
 *                           1-1.2:1.0/host65/target65:0:0/65:0:0:0/block/sdb
 */
స్థిర పూర్ణాంకం sysfs_devchain_is_removable(అక్షరం *chain)
{
	size_t len;
	అక్షరం buf[20];
	అక్షరం *p;

	ఒకవేళ (!chain || !*chain)
		తిరిగి 0;

	len = అక్షరశ్రేణి_పొడవు(chain);
	ఒకవేళ (len + పరిమాణం(REMOVABLE_FILENAME) > PATH_MAX)
		తిరిగి 0;

	చేయి {
		పూర్ణాంకం fd, rc;

		/* append "/removable" to the path */
		జ్ఞాపకం_నకలు(chain + len, REMOVABLE_FILENAME, పరిమాణం(REMOVABLE_FILENAME));

		/* root of device hierarchy */
		ఒకవేళ (strcmp(chain, "/sys/dev/block" REMOVABLE_FILENAME) == 0)
			ఆపు;

		/* try to read it */
		fd = open(chain, O_RDONLY | O_CLOEXEC);
		ఒకవేళ (fd != -1) {
			rc = ul_read_all(fd, buf, పరిమాణం(buf));
			close(fd);

			ఒకవేళ (rc > 0) {
				ఒకవేళ (అక్షరశ్రేణి_పోలిక(buf, "fixed", min(rc, 5)) == 0) {
					తిరిగి 0;
				} లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(buf, "removable", min(rc, 9)) == 0) {
					తిరిగి 1;
				}
			}
		}

		/* remove last subsystem from chain */
		chain[len] = '\0';
		p = strrchr(chain, '/');
		ఒకవేళ (p) {
			*p = '\0';
			len = p - chain;
		}

	} అయితే (p);

	తిరిగి 0;
}

పూర్ణాంకం sysfs_blkdev_is_hotpluggable(నిర్మాణం మార్గం_cxt *pc)
{
	అక్షరం buf[PATH_MAX], *chain;

	chain = sysfs_blkdev_get_devchain(pc, buf, పరిమాణం(buf));
	తిరిగి sysfs_devchain_is_removable(chain);
}

పూర్ణాంకం sysfs_blkdev_is_removable(నిర్మాణం మార్గం_cxt *pc)
{
	పూర్ణాంకం rc = 0;

	// FIXME usb is not actually removable

	/* check /sys/dev/block/<maj>:<min>/removable attribute */
	ఒకవేళ (ul_మార్గం_read_s32(pc, &rc, "removable") == 0)
		తిరిగి rc;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం get_dm_wholedisk(నిర్మాణం మార్గం_cxt *pc, అక్షరం *diskname,
                            size_t len, dev_t *diskdevno)
{
	పూర్ణాంకం rc = 0;
	అక్షరం *name;

	/* Note: sysfs_blkdev_get_slave() returns the first slave only;
	 * if there are more slaves, it returns NULL.
	 */
	name = sysfs_blkdev_get_slave(pc);
	ఒకవేళ (!name)
		తిరిగి -1;

	ఒకవేళ (diskname && len)
		xstrncpy(diskname, name, len);

	ఒకవేళ (diskdevno) {
		*diskdevno = __sysfs_devname_to_devno(ul_మార్గం_get_ఉపసర్గ(pc), name, ఖాళీ);
		ఒకవేళ (!*diskdevno)
			rc = -1;
	}

	విడుదల(name);
	తిరిగి rc;
}

/*
 * Returns by @diskdevno whole disk device devno and (optionally) by
 * @diskname the whole disk device name.
 */
పూర్ణాంకం sysfs_blkdev_get_wholedisk(	నిర్మాణం మార్గం_cxt *pc,
				అక్షరం *diskname,
				size_t len,
				dev_t *diskdevno)
{
	పూర్ణాంకం is_part = 0;

	ఒకవేళ (!pc)
		తిరిగి -1;

	is_part = ul_మార్గం_access(pc, F_OK, "partition") == 0;
	ఒకవేళ (!is_part) {
		/*
		 * Extra case for partitions mapped by device-mapper.
		 *
		 * All regular partitions (added by BLKPG ioctl or kernel PT
		 * parser) have the /sys/.../partition file. The partitions
		 * mapped by DM don't have such file, but they have "part"
		 * prefix in DM UUID.
		 */
		అక్షరం *uuid = ఖాళీ, *tmp, *ఉపసర్గ;

		ul_మార్గం_read_string(pc, &uuid, "dm/uuid");
		tmp = uuid;
		ఉపసర్గ = uuid ? strsep(&tmp, "-") : ఖాళీ;

		ఒకవేళ (ఉపసర్గ && c_strncasecmp(ఉపసర్గ, "part", 4) == 0)
			is_part = 1;
		విడుదల(uuid);

		ఒకవేళ (is_part &&
			get_dm_wholedisk(pc, diskname, len, diskdevno) == 0)
			/*
			 * partitioned device, mapped by DM
			 */
			వెళ్ళు పూర్తి;

		is_part = 0;
	}

	ఒకవేళ (!is_part) {
		/*
		 * unpartitioned device
		 */
		ఒకవేళ (diskname && !sysfs_blkdev_get_name(pc, diskname, len))
			వెళ్ళు err;
		ఒకవేళ (diskdevno)
			*diskdevno = sysfs_blkdev_get_devno(pc);
	} లేకపోతే {
		/*
		 * partitioned device
		 *  - readlink /sys/dev/block/8:1   = ../../block/sda/sda1
		 *  - dirname  ../../block/sda/sda1 = ../../block/sda
		 *  - basename ../../block/sda	  = sda
		 */
		అక్షరం linkpath[PATH_MAX];
		అక్షరం *name;
		ssize_t	linklen;

		linklen = ul_మార్గం_readlink(pc, linkpath, పరిమాణం(linkpath), ఖాళీ);
		ఒకవేళ (linklen < 0)
			వెళ్ళు err;

		stripoff_చివరి_component(linkpath);	  /* dirname */
		name = stripoff_చివరి_component(linkpath);   /* basename */
		ఒకవేళ (!name)
			వెళ్ళు err;

		sysfs_devname_sys_గమ్యం_dev(name);
		ఒకవేళ (diskname && len)
			xstrncpy(diskname, name, len);

		ఒకవేళ (diskdevno) {
			*diskdevno = __sysfs_devname_to_devno(ul_మార్గం_get_ఉపసర్గ(pc), name, ఖాళీ);
			ఒకవేళ (!*diskdevno)
				వెళ్ళు err;
		}
	}

పూర్తి:
	తిరిగి 0;
err:
	తిరిగి -1;
}

పూర్ణాంకం sysfs_devno_గమ్యం_wholedisk(dev_t devno, అక్షరం *diskname,
		             size_t len, dev_t *diskdevno)
{
	నిర్మాణం మార్గం_cxt *pc;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (!devno)
		తిరిగి -EINVAL;
	pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	ఒకవేళ (!pc)
		తిరిగి -ENOMEM;

	rc = sysfs_blkdev_get_wholedisk(pc, diskname, len, diskdevno);
	ul_unref_మార్గం(pc);
	తిరిగి rc;
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
పూర్ణాంకం sysfs_devno_is_dm_hidden(dev_t devno, అక్షరం **uuid,
			     స్థిరరాశి నిర్మాణం ul_vfs_ops *vfs)
{
	నిర్మాణం మార్గం_cxt *pc = ఖాళీ;
	అక్షరం *id = ఖాళీ;
	పూర్ణాంకం rc = 0;

	pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	ఒకవేళ (!pc)
		వెళ్ళు పూర్తి;
	ul_మార్గం_refer_vfs(pc, vfs);
	ఒకవేళ (ul_మార్గం_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		వెళ్ళు పూర్తి;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(id, "LVM-", 4) == 0) {
		అక్షరం *p = strrchr(id + 4, '-');

		ఒకవేళ (p && *(p + 1))
			rc = 1;
	}
పూర్తి:
	ul_unref_మార్గం(pc);
	ఒకవేళ (uuid)
		*uuid = id;
	లేకపోతే
		విడుదల(id);
	తిరిగి rc;
}

/*
 * Returns 1 if the device is private device mapper device. The @uuid
 * (if not NULL) returns DM device UUID, use free() to deallocate.
 */
పూర్ణాంకం sysfs_devno_is_dm_వ్యక్తిగత(dev_t devno, అక్షరం **uuid,
			      స్థిరరాశి నిర్మాణం ul_vfs_ops *vfs)
{
	నిర్మాణం మార్గం_cxt *pc = ఖాళీ;
	అక్షరం *id = ఖాళీ;
	పూర్ణాంకం rc = 0;

	pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	ఒకవేళ (!pc)
		వెళ్ళు పూర్తి;
	ul_మార్గం_refer_vfs(pc, vfs);
	ఒకవేళ (ul_మార్గం_read_string(pc, &id, "dm/uuid") <= 0 || !id)
		వెళ్ళు పూర్తి;

	/* Private LVM devices use "LVM-<uuid>-<name>" uuid format (important
	 * is the "LVM" prefix and "-<name>" postfix).
	 */
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(id, "LVM-", 4) == 0) {
		అక్షరం *p = strrchr(id + 4, '-');

		ఒకవేళ (p && *(p + 1))
			rc = 1;

	/* Private Stratis devices prefix the UUID with "stratis-1-private"
	 */
	} లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(id, "stratis-1-private", 17) == 0) {
		rc = 1;
	}
పూర్తి:
	ul_unref_మార్గం(pc);
	ఒకవేళ (uuid)
		*uuid = id;
	లేకపోతే
		విడుదల(id);
	తిరిగి rc;
}

/*
 * Return 0 or 1, or < 0 in case of error
 */
పూర్ణాంకం sysfs_devno_is_wholedisk(dev_t devno)
{
	dev_t disk;

	ఒకవేళ (sysfs_devno_గమ్యం_wholedisk(devno, ఖాళీ, 0, &disk) != 0)
		తిరిగి -1;

	తిరిగి devno == disk;
}


పూర్ణాంకం sysfs_blkdev_scsi_get_hctl(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం *h, పూర్ణాంకం *c, పూర్ణాంకం *t, పూర్ణాంకం *l)
{
	అక్షరం buf[PATH_MAX], *hctl;
	నిర్మాణం sysfs_blkdev *blk;
	ssize_t len;

	blk = ul_మార్గం_get_dialect(pc);

	ఒకవేళ (!blk || blk->hctl_error)
		తిరిగి -EINVAL;
	ఒకవేళ (blk->has_hctl)
		వెళ్ళు పూర్తి;

	blk->hctl_error = 1;
	len = ul_మార్గం_readlink(pc, buf, పరిమాణం(buf), "device");
	ఒకవేళ (len < 0)
		తిరిగి len;

	hctl = strrchr(buf, '/');
	ఒకవేళ (!hctl)
		తిరిగి -1;
	hctl++;

	ఒకవేళ (అక్షరశ్రేణి_చదువు(hctl, "%u:%u:%u:%u",	&blk->scsi_host, &blk->scsi_channel,
				&blk->scsi_లక్ష్యం, &blk->scsi_lun) != 4)
		తిరిగి -1;

	blk->has_hctl = 1;
పూర్తి:
	ఒకవేళ (h)
		*h = blk->scsi_host;
	ఒకవేళ (c)
		*c = blk->scsi_channel;
	ఒకవేళ (t)
		*t = blk->scsi_లక్ష్యం;
	ఒకవేళ (l)
		*l = blk->scsi_lun;

	blk->hctl_error = 0;
	తిరిగి 0;
}


స్థిర అక్షరం *scsi_host_attribute_మార్గం(
			నిర్మాణం మార్గం_cxt *pc,
			స్థిరరాశి అక్షరం *type,
			అక్షరం *buf,
			size_t bufsz,
			స్థిరరాశి అక్షరం *attr)
{
	పూర్ణాంకం len;
	పూర్ణాంకం host;
	స్థిరరాశి అక్షరం *ఉపసర్గ;

	ఒకవేళ (sysfs_blkdev_scsi_get_hctl(pc, &host, ఖాళీ, ఖాళీ, ఖాళీ))
		తిరిగి ఖాళీ;

	ఉపసర్గ = ul_మార్గం_get_ఉపసర్గ(pc);
	ఒకవేళ (!ఉపసర్గ)
		ఉపసర్గ = "";

	ఒకవేళ (attr)
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d/%s",
				ఉపసర్గ, మార్గం_SYS_CLASS, type, host, attr);
	లేకపోతే
		len = snprintf(buf, bufsz, "%s%s/%s_host/host%d",
				ఉపసర్గ, మార్గం_SYS_CLASS, type, host);

	తిరిగి (len < 0 || (size_t) len >= bufsz) ? ఖాళీ : buf;
}

అక్షరం *sysfs_blkdev_scsi_host_strdup_attribute(నిర్మాణం మార్గం_cxt *pc,
			స్థిరరాశి అక్షరం *type, స్థిరరాశి అక్షరం *attr)
{
	అక్షరం buf[1024];
	పూర్ణాంకం rc;
	FILE *f;

	ఒకవేళ (!attr || !type ||
	    !scsi_host_attribute_మార్గం(pc, type, buf, పరిమాణం(buf), attr))
		తిరిగి ఖాళీ;

	ఒకవేళ (!(f = fopen(buf, "r" UL_CLOEXECSTR)))
		తిరిగి ఖాళీ;

	rc = fscanf(f, "%1023[^\n]", buf);
	fclose(f);

	తిరిగి rc == 1 ? strdup(buf) : ఖాళీ;
}

పూర్ణాంకం sysfs_blkdev_scsi_host_is(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *type)
{
	అక్షరం buf[PATH_MAX];
	నిర్మాణం stat st;

	ఒకవేళ (!type || !scsi_host_attribute_మార్గం(pc, type,
				buf, పరిమాణం(buf), ఖాళీ))
		తిరిగి 0;

	తిరిగి stat(buf, &st) == 0 && S_ISDIR(st.st_mode);
}

స్థిర అక్షరం *scsi_attribute_మార్గం(నిర్మాణం మార్గం_cxt *pc,
		అక్షరం *buf, size_t bufsz, స్థిరరాశి అక్షరం *attr)
{
	పూర్ణాంకం len, h, c, t, l;
	స్థిరరాశి అక్షరం *ఉపసర్గ;

	ఒకవేళ (sysfs_blkdev_scsi_get_hctl(pc, &h, &c, &t, &l) != 0)
		తిరిగి ఖాళీ;

	ఉపసర్గ = ul_మార్గం_get_ఉపసర్గ(pc);
	ఒకవేళ (!ఉపసర్గ)
		ఉపసర్గ = "";

	ఒకవేళ (attr)
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d/%s",
				ఉపసర్గ, మార్గం_SYS_SCSI,
				h,c,t,l, attr);
	లేకపోతే
		len = snprintf(buf, bufsz, "%s%s/devices/%d:%d:%d:%d",
				ఉపసర్గ, మార్గం_SYS_SCSI,
				h,c,t,l);
	తిరిగి (len < 0 || (size_t) len >= bufsz) ? ఖాళీ : buf;
}

పూర్ణాంకం sysfs_blkdev_scsi_has_attribute(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *attr)
{
	అక్షరం మార్గం[PATH_MAX];
	నిర్మాణం stat st;

	ఒకవేళ (!scsi_attribute_మార్గం(pc, మార్గం, పరిమాణం(మార్గం), attr))
		తిరిగి 0;

	తిరిగి stat(మార్గం, &st) == 0;
}

పూర్ణాంకం sysfs_blkdev_scsi_మార్గం_కలిగియుంది(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *నమూనా)
{
	అక్షరం మార్గం[PATH_MAX], linkc[PATH_MAX];
	నిర్మాణం stat st;
	ssize_t len;

	ఒకవేళ (!scsi_attribute_మార్గం(pc, మార్గం, పరిమాణం(మార్గం), ఖాళీ))
		తిరిగి 0;

	ఒకవేళ (stat(మార్గం, &st) != 0)
		తిరిగి 0;

	len = readlink(మార్గం, linkc, పరిమాణం(linkc) - 1);
	ఒకవేళ (len < 0)
		తిరిగి 0;

	linkc[len] = '\0';
	తిరిగి అక్షరశ్రేణి_శోధన(linkc, నమూనా) != ఖాళీ;
}

స్థిర dev_t read_devno(స్థిరరాశి అక్షరం *మార్గం)
{
	FILE *f;
	పూర్ణాంకం maj = 0, min = 0;
	dev_t dev = 0;

	f = fopen(మార్గం, "r" UL_CLOEXECSTR);
	ఒకవేళ (!f)
		తిరిగి 0;

	ఒకవేళ (fscanf(f, "%d:%d", &maj, &min) == 2)
		dev = makedev(maj, min);
	fclose(f);
	తిరిగి dev;
}

పూర్ణాంకం sysfs_devname_is_hidden(స్థిరరాశి అక్షరం *ఉపసర్గ, స్థిరరాశి అక్షరం *name)
{
	అక్షరం buf[PATH_MAX];
	పూర్ణాంకం rc = 0, hidden = 0, len;
	FILE *f;

	ఒకవేళ (అక్షరశ్రేణి_పోలిక("/dev/", name, 5) == 0)
		తిరిగి 0;

	ఒకవేళ (!ఉపసర్గ)
		ఉపసర్గ = "";
	/*
	 * Create path to /sys/block/<name>/hidden
	 */
	len = snprintf(buf, పరిమాణం(buf),
			"%s" మార్గం_SYS_ఖండం "/%s/hidden",
			ఉపసర్గ, name);

	ఒకవేళ (len < 0 || (size_t) len + 1 > పరిమాణం(buf))
		తిరిగి 0;

	f = fopen(buf, "r" UL_CLOEXECSTR);
	ఒకవేళ (!f)
		తిరిగి 0;

	rc = fscanf(f, "%d", &hidden);
	fclose(f);

	తిరిగి rc == 1 ? hidden : 0;
}


dev_t __sysfs_devname_to_devno(స్థిరరాశి అక్షరం *ఉపసర్గ, స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *మాతృ)
{
	అక్షరం buf[PATH_MAX];
	అక్షరం *_name = ఖాళీ, *మాతృ_2 = ఖాళీ;	/* name as encoded in sysfs */
	dev_t dev = 0;
	పూర్ణాంకం len;

	ఒకవేళ (!ఉపసర్గ)
		ఉపసర్గ = "";

	ప్రతిజ్ఞా(name);

	ఒకవేళ (అక్షరశ్రేణి_పోలిక("/dev/", name, 5) == 0) {
		/*
		 * Read from /dev
		 */
		నిర్మాణం stat st;

		ఒకవేళ (stat(name, &st) == 0) {
			dev = st.st_rdev;
			వెళ్ళు పూర్తి;
		}
		name += 5;	/* unaccessible, or not node in /dev */
	}

	_name = strdup(name);
	ఒకవేళ (!_name)
		వెళ్ళు పూర్తి;
	sysfs_devname_dev_గమ్యం_sys(_name);

	ఒకవేళ (మాతృ) {
		మాతృ_2 = strdup(మాతృ);
		ఒకవేళ (!మాతృ_2)
			వెళ్ళు పూర్తి;
	}

	ఒకవేళ (మాతృ && అక్షరశ్రేణి_పోలిక("dm-", name, 3) != 0) {
		/*
		 * Create path to /sys/block/<parent>/<name>/dev
		 */
		sysfs_devname_dev_గమ్యం_sys(మాతృ_2);
		len = snprintf(buf, పరిమాణం(buf),
				"%s" మార్గం_SYS_ఖండం "/%s/%s/dev",
				ఉపసర్గ,	మాతృ_2, _name);
		ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
			వెళ్ళు పూర్తి;

		/* don't try anything else for dm-* */
		dev = read_devno(buf);
		వెళ్ళు పూర్తి;
	}

	/*
	 * Read from /sys/block/<sysname>/dev
	 */
	len = snprintf(buf, పరిమాణం(buf),
			"%s" మార్గం_SYS_ఖండం "/%s/dev",
			ఉపసర్గ, _name);
	ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
		వెళ్ళు పూర్తి;
	dev = read_devno(buf);

	/*
	 * Read from /sys/block/<parent>/<partition>/dev
	 */
	ఒకవేళ (!dev && మాతృ && ul_startswith(name, మాతృ)) {
		len = snprintf(buf, పరిమాణం(buf),
				"%s" మార్గం_SYS_ఖండం "/%s/%s/dev",
				ఉపసర్గ, మాతృ_2, _name);
		ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
			వెళ్ళు పూర్తి;
		dev = read_devno(buf);
	}

	/*
	 * Read from /sys/block/<sysname>/device/dev
	 */
	ఒకవేళ (!dev) {
		len = snprintf(buf, పరిమాణం(buf),
				"%s" మార్గం_SYS_ఖండం "/%s/device/dev",
				ఉపసర్గ, _name);
		ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
			వెళ్ళు పూర్తి;
		dev = read_devno(buf);
	}
పూర్తి:
	విడుదల(_name);
	విడుదల(మాతృ_2);
	తిరిగి dev;
}

dev_t sysfs_devname_గమ్యం_devno(స్థిరరాశి అక్షరం *name)
{
	తిరిగి __sysfs_devname_to_devno(ఖాళీ, name, ఖాళీ);
}

అక్షరం *sysfs_blkdev_get_మార్గం(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsiz)
{
	స్థిరరాశి అక్షరం *name = sysfs_blkdev_get_name(pc, buf, bufsiz);
	అక్షరం *res = ఖాళీ;
	size_t sz;
	నిర్మాణం stat st;

	ఒకవేళ (!name)
		వెళ్ళు పూర్తి;

	sz = అక్షరశ్రేణి_పొడవు(name);
	ఒకవేళ (sz + పరిమాణం("/dev/") > bufsiz)
		వెళ్ళు పూర్తి;

	/* create the final "/dev/<name>" string */
	memmove(buf + 5, name, sz + 1);
	జ్ఞాపకం_నకలు(buf, "/dev/", 5);

	ఒకవేళ (!stat(buf, &st) && S_ISBLK(st.st_mode) && st.st_rdev == sysfs_blkdev_get_devno(pc))
		res = buf;
పూర్తి:
	తిరిగి res;
}

dev_t sysfs_blkdev_get_devno(నిర్మాణం మార్గం_cxt *pc)
{
	తిరిగి ((నిర్మాణం sysfs_blkdev *) ul_మార్గం_get_dialect(pc))->devno;
}

/*
 * Returns devname (e.g. "/dev/sda1") for the given devno.
 *
 * Please, use more robust blkid_devno_to_devname() in your applications.
 */
అక్షరం *sysfs_devno_గమ్యం_devpath(dev_t devno, అక్షరం *buf, size_t bufsiz)
{
	నిర్మాణం మార్గం_cxt *pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (pc) {
		res = sysfs_blkdev_get_మార్గం(pc, buf, bufsiz);
		ul_unref_మార్గం(pc);
	}
	తిరిగి res;
}

అక్షరం *sysfs_devno_గమ్యం_devname(dev_t devno, అక్షరం *buf, size_t bufsiz)
{
	నిర్మాణం మార్గం_cxt *pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	అక్షరం *res = ఖాళీ;

	ఒకవేళ (pc) {
		res = sysfs_blkdev_get_name(pc, buf, bufsiz);
		ul_unref_మార్గం(pc);
	}
	తిరిగి res;
}

పూర్ణాంకం sysfs_devno_లెక్క_partitions(dev_t devno)
{
	నిర్మాణం మార్గం_cxt *pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	పూర్ణాంకం n = 0;

	ఒకవేళ (pc) {
		అక్షరం buf[PATH_MAX + 1];
		అక్షరం *name = sysfs_blkdev_get_name(pc, buf, పరిమాణం(buf));

		n = sysfs_blkdev_లెక్క_partitions(pc, name);
		ul_unref_మార్గం(pc);
	}
	తిరిగి n;
}

అక్షరం *sysfs_chrdev_devno_గమ్యం_devname(dev_t devno, అక్షరం *buf, size_t bufsiz)
{
	అక్షరం link[PATH_MAX];
	నిర్మాణం మార్గం_cxt *pc;
	అక్షరం *name;
	ssize_t	sz;

	pc = ul_new_మార్గం(మార్గం_SYS_DEVCHAR "/%u:%u", major(devno), minor(devno));
	ఒకవేళ (!pc)
		తిరిగి ఖాళీ;

	/* read /sys/dev/char/<maj:min> link */
	sz = ul_మార్గం_readlink(pc, link, పరిమాణం(link), ఖాళీ);
	ul_unref_మార్గం(pc);

	ఒకవేళ (sz < 0)
		తిరిగి ఖాళీ;

	name = strrchr(link, '/');
	ఒకవేళ (!name)
		తిరిగి ఖాళీ;

	name++;
	sz = అక్షరశ్రేణి_పొడవు(name);
	ఒకవేళ ((size_t) sz + 1 > bufsiz)
		తిరిగి ఖాళీ;

	జ్ఞాపకం_నకలు(buf, name, sz + 1);
	sysfs_devname_sys_గమ్యం_dev(buf);
	తిరిగి buf;

}

గణన sysfs_byteorder sysfs_get_byteorder(నిర్మాణం మార్గం_cxt *pc)
{
	పూర్ణాంకం rc;
	అక్షరం buf[BUFSIZ];
	గణన sysfs_byteorder ret;

	rc = ul_మార్గం_read_బఫరు(pc, buf, పరిమాణం(buf), మార్గం_SYS_CPU_BYTEORDER);
	ఒకవేళ (rc < 0)
		వెళ్ళు unknown;

	ఒకవేళ (అక్షరశ్రేణి_పోలిక(buf, "little", పరిమాణం(buf)) == 0) {
		ret = SYSFS_BYTEORDER_LITTLE;
		వెళ్ళు బహిర్గతం;
	} లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(buf, "big", పరిమాణం(buf)) == 0) {
		ret = SYSFS_BYTEORDER_BIG;
		వెళ్ళు బహిర్గతం;
	}

unknown:
#ఒకవేళ __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	ret = SYSFS_BYTEORDER_LITTLE;
#లేదావేళ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	ret = SYSFS_BYTEORDER_BIG;
#లేకపోతే
#దోషం Unknown byte క్రమం
#అయితే_ముగింపు

బహిర్గతం:
	తిరిగి ret;
}

పూర్ణాంకం sysfs_get_చిరునామా_బిట్లు(నిర్మాణం మార్గం_cxt *pc)
{
	పూర్ణాంకం rc;
	పూర్ణాంకం చిరునామా_బిట్లు;

	rc = ul_మార్గం_scanf(pc, మార్గం_SYS_చిరునామా_బిట్లు, "%d", &చిరునామా_బిట్లు);
	ఒకవేళ (rc < 0)
		తిరిగి rc;
	ఒకవేళ (చిరునామా_బిట్లు < 0)
		తిరిగి -EINVAL;
	తిరిగి చిరునామా_బిట్లు;
}


#నిర్వచితమైతే TEST_కార్యక్రమం_SYSFS
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdlib.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం మార్గం_cxt *pc;
	అక్షరం *devname;
	dev_t devno, disk_devno;
	అక్షరం మార్గం[PATH_MAX], *sub, *chain;
	అక్షరం diskname[32];
	పూర్ణాంకం i, is_part, rc = EXIT_SUCCESS;
	uint64_t u64;

	ఒకవేళ (argc != 2)
		errx(EXIT_FAILURE, "usage: %s <devname>", argv[0]);

	ul_sysfs_init_debug();

	devname = argv[1];
	devno = sysfs_devname_గమ్యం_devno(devname);

	ఒకవేళ (!devno)
		err(EXIT_FAILURE, "failed to read devno");

	రాయి("non-context:\n");
	రాయి(" DEVNO:   %u (%u:%u)\n", (గుర్తులేని పూర్ణాంకం) devno, major(devno), minor(devno));
	రాయి(" DEVNAME: %s\n", sysfs_devno_గమ్యం_devname(devno, మార్గం, పరిమాణం(మార్గం)));
	రాయి(" DEVPATH: %s\n", sysfs_devno_గమ్యం_devpath(devno, మార్గం, పరిమాణం(మార్గం)));

	sysfs_devno_గమ్యం_wholedisk(devno, diskname, పరిమాణం(diskname), &disk_devno);
	రాయి(" WHOLEDISK-DEVNO:   %u (%u:%u)\n", (గుర్తులేని పూర్ణాంకం) disk_devno, major(disk_devno), minor(disk_devno));
	రాయి(" WHOLEDISK-DEVNAME: %s\n", diskname);

	pc = ul_new_sysfs_మార్గం(devno, ఖాళీ, ఖాళీ);
	ఒకవేళ (!pc)
		వెళ్ళు పూర్తి;

	రాయి("context based:\n");
	devno = sysfs_blkdev_get_devno(pc);
	రాయి(" DEVNO:   %u (%u:%u)\n", (గుర్తులేని పూర్ణాంకం) devno, major(devno), minor(devno));
	రాయి(" DEVNAME: %s\n", sysfs_blkdev_get_name(pc, మార్గం, పరిమాణం(మార్గం)));
	రాయి(" DEVPATH: %s\n", sysfs_blkdev_get_మార్గం(pc, మార్గం, పరిమాణం(మార్గం)));

	sysfs_devno_గమ్యం_wholedisk(devno, diskname, పరిమాణం(diskname), &disk_devno);
	రాయి(" WHOLEDISK-DEVNO: %u (%u:%u)\n", (గుర్తులేని పూర్ణాంకం) disk_devno, major(disk_devno), minor(disk_devno));
	రాయి(" WHOLEDISK-DEVNAME: %s\n", diskname);

	is_part = ul_మార్గం_access(pc, F_OK, "partition") == 0;
	రాయి(" PARTITION: %s\n", is_part ? "YES" : "NOT");

	ఒకవేళ (is_part && disk_devno) {
		నిర్మాణం మార్గం_cxt *disk_pc =  ul_new_sysfs_మార్గం(disk_devno, ఖాళీ, ఖాళీ);
		sysfs_blkdev_సమితి_మాతృ(pc, disk_pc);

		ul_unref_మార్గం(disk_pc);
	}

	రాయి(" HOTPLUG: %s\n", sysfs_blkdev_is_hotpluggable(pc) ? "yes" : "no");
	రాయి(" REMOVABLE: %s\n", sysfs_blkdev_is_removable(pc) ? "yes" : "no");
	రాయి(" SLAVES: %d\n", ul_మార్గం_లెక్క_dirents(pc, "slaves"));

	ఒకవేళ (!is_part) {
		రాయి("First 5 partitions:\n");
		కొరకు (i = 1; i <= 5; i++) {
			dev_t dev = sysfs_blkdev_partno_గమ్యం_devno(pc, i);
			ఒకవేళ (dev)
				రాయి("\t#%d %u:%u\n", i, major(dev), minor(dev));
		}
	}

	ఒకవేళ (ul_మార్గం_read_u64(pc, &u64, "size") != 0)
		రాయి(" (!) read SIZE failed\n");
	లేకపోతే
		రాయి(" SIZE: %"PRIu64"\n", u64);

	ఒకవేళ (ul_మార్గం_read_s32(pc, &i, "queue/hw_sector_size"))
		రాయి(" (!) read SECTOR failed\n");
	లేకపోతే
		రాయి(" SECTOR: %d\n", i);


	chain = sysfs_blkdev_get_devchain(pc, మార్గం, పరిమాణం(మార్గం));
	రాయి(" SUBSYSTEMS:\n");

	అయితే (chain && sysfs_blkdev_తదుపరి_subsystem(pc, chain, &sub) == 0) {
		రాయి("\t%s\n", sub);
		విడుదల(sub);
	}

	rc = EXIT_SUCCESS;
పూర్తి:
	ul_unref_మార్గం(pc);
	తిరిగి rc;
}
#అయితే_ముగింపు /* TEST_PROGRAM_SYSFS */
