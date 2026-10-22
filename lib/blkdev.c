/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/file.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdint.h>

#ifdef HAVE_LINUX_FD_H
#অন্তর্ভাব <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#অন্তর্ভাব <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#অন্তর্ভাব <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "blkdev.h"
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "linux_version.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "nls.h"

স্থির long blkdev_valid_offset(পূর্ণাঙ্ক fd, off_t offset)
{
	বর্ণ ch;

	যদি (lseek (fd, offset, 0) < 0)
		প্রত্যাগম 0;
	যদি (ul_read_all (fd, &ch, 1) < 1)
		প্রত্যাগম 0;
	প্রত্যাগম 1;
}

পূর্ণাঙ্ক is_blkdev(পূর্ণাঙ্ক fd)
{
	সংরচনা stat st;
	প্রত্যাগম (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (পূর্ণাঙ্ক fd)
{
	off_t high, নিচের = 0;

	প্রতি (high = 1024; blkdev_valid_offset(fd, high); ) {
		যদি (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			প্রত্যাগম -1;
		}

		নিচের = high;

		যদি (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		অন্যথা
			high *= 2;
	}

	যাবত্ (নিচের < high - 1) {
		off_t mid = (নিচের + high) / 2;

		যদি (blkdev_valid_offset (fd, mid))
			নিচের = mid;
		অন্যথা
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	প্রত্যাগম (নিচের + 1);
}

/* get size in bytes */
পূর্ণাঙ্ক blkdev_get_size(পূর্ণাঙ্ক fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	যদি (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		প্রত্যাগম 0;
	}
#endif

#ifdef BLKGETSIZE64
	যদি (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		প্রত্যাগম 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		যদি (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			প্রত্যাগম 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	যদি (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		প্রত্যাগম 0;
#endif

#ifdef FDGETPRM
	{
		সংরচনা floppy_struct this_floppy;

		যদি (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			প্রত্যাগম 0;
		}
	}
#endif /* FDGETPRM */

#যদি defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
	{
		/*
		 * This code works for FreeBSD 4.11 i386, except for the full device
		 * (such as /dev/ad0). It doesn't work properly for newer FreeBSD
		 * though. FreeBSD >= 5.0 should be covered by the DIOCGMEDIASIZE
		 * above however.
		 *
		 * Note that FreeBSD >= 4.0 has disk devices as unbuffered (raw,
		 * character) devices, so we need to check for S_ISCHR, too.
		 */
		পূর্ণাঙ্ক part = -1;
		সংরচনা disklabel lab;
		সংরচনা partition *pp;
		সংরচনা stat st;

		যদি ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		যদি (part >= 0 && (ioctl(fd, DIOCGDINFO, (বর্ণ *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			যদি (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 প্রত্যাগম 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		সংরচনা stat st;

		যদি (fstat(fd, &st) != 0)
			প্রত্যাগম -1;
		যদি (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			প্রত্যাগম 0;
		}
		যদি (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			প্রত্যাগম -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		যদি (size < 0)
			প্রত্যাগম -1;

		*bytes = (uint64_t) size;
	}

	প্রত্যাগম 0;
}

/* get 512-byte sector count */
পূর্ণাঙ্ক blkdev_get_sectors(পূর্ণাঙ্ক fd, uint64_t *sectors)
{
	uint64_t bytes;

	যদি (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		প্রত্যাগম 0;
	}

	প্রত্যাগম -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
পূর্ণাঙ্ক blkdev_get_sector_size(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক *sector_size)
{
	যদি (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		প্রত্যাগম 0;
	প্রত্যাগম -1;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_sector_size(পূর্ণাঙ্ক fd __attribute__((__unused__)), পূর্ণাঙ্ক *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	প্রত্যাগম 0;
}
#endif

/*
 * Get physical block device size. The BLKPBSZGET is supported since Linux
 * 2.6.32. For old kernels is probably the best to assume that physical sector
 * size is the same as logical sector size.
 *
 * Example:
 *
 * rc = blkdev_get_physector_size(fd, &physec);
 * if (rc || physec == 0) {
 *	rc = blkdev_get_sector_size(fd, &physec);
 *	if (rc)
 *		physec = DEFAULT_SECTOR_SIZE;
 * }
 */
#ifdef BLKPBSZGET
পূর্ণাঙ্ক blkdev_get_physector_size(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক *sector_size)
{
	যদি (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		প্রত্যাগম 0;
	প্রত্যাগম -1;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_physector_size(পূর্ণাঙ্ক fd __attribute__((__unused__)), পূর্ণাঙ্ক *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	প্রত্যাগম 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
পূর্ণাঙ্ক blkdev_is_misaligned(পূর্ণাঙ্ক fd)
{
	পূর্ণাঙ্ক aligned;

	যদি (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		প্রত্যাগম 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	প্রত্যাগম aligned != 0 ? 1 : 0;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_is_misaligned(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	প্রত্যাগম 0;
}
#endif

পূর্ণাঙ্ক open_blkdev_or_file(ধ্রুব সংরচনা stat *st, ধ্রুব বর্ণ *name, ধ্রুব পূর্ণাঙ্ক oflag)
{
	পূর্ণাঙ্ক fd;

	যদি (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} অন্যথা
		fd = open(name, oflag);
	যদি (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		প্রত্যাগম -1;
	}
	যদি (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	প্রত্যাগম fd;
}

#ifdef CDROM_GET_CAPABILITY
পূর্ণাঙ্ক blkdev_is_cdrom(পূর্ণাঙ্ক fd)
{
	পূর্ণাঙ্ক ret = ioctl(fd, CDROM_GET_CAPABILITY, রিক্ত);
	প্রত্যাগম ret < 0 ? 0 : ret;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_is_cdrom(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	প্রত্যাগম 0;
}
#endif

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#ifdef HDIO_GETGEO
পূর্ণাঙ্ক blkdev_get_geometry(পূর্ণাঙ্ক fd, unsigned পূর্ণাঙ্ক *h, unsigned পূর্ণাঙ্ক *s)
{
	সংরচনা hd_geometry geometry;

	যদি (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		প্রত্যাগম 0;
	}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_geometry(পূর্ণাঙ্ক fd __attribute__((__unused__)),
		unsigned পূর্ণাঙ্ক *h, unsigned পূর্ণাঙ্ক *s)
{
	*h = 0;
	*s = 0;
#endif
	প্রত্যাগম -1;
}

/*
 * Convert scsi type to human readable string.
 */
ধ্রুব বর্ণ *blkdev_scsi_type_to_name(পূর্ণাঙ্ক type)
{
	বিকল্প (type) {
	পক্ষ SCSI_TYPE_DISK:
		প্রত্যাগম "disk";
	পক্ষ SCSI_TYPE_TAPE:
		প্রত্যাগম "tape";
	পক্ষ SCSI_TYPE_PRINTER:
		প্রত্যাগম "printer";
	পক্ষ SCSI_TYPE_PROCESSOR:
		প্রত্যাগম "processor";
	পক্ষ SCSI_TYPE_WORM:
		প্রত্যাগম "worm";
	পক্ষ SCSI_TYPE_ROM:
		প্রত্যাগম "rom";
	পক্ষ SCSI_TYPE_SCANNER:
		প্রত্যাগম "scanner";
	পক্ষ SCSI_TYPE_MOD:
		প্রত্যাগম "mo-disk";
	পক্ষ SCSI_TYPE_MEDIUM_CHANGER:
		প্রত্যাগম "changer";
	পক্ষ SCSI_TYPE_COMM:
		প্রত্যাগম "comm";
	পক্ষ SCSI_TYPE_RAID:
		প্রত্যাগম "raid";
	পক্ষ SCSI_TYPE_ENCLOSURE:
		প্রত্যাগম "enclosure";
	পক্ষ SCSI_TYPE_RBC:
		প্রত্যাগম "rbc";
	পক্ষ SCSI_TYPE_OSD:
		প্রত্যাগম "osd";
	পক্ষ SCSI_TYPE_NO_LUN:
		প্রত্যাগম "no-lun";
	স্বভাব:
		ভঙ্গ;
	}
	প্রত্যাগম রিক্ত;
}

/* return 0 on success */
পূর্ণাঙ্ক blkdev_lock(পূর্ণাঙ্ক fd, ধ্রুব বর্ণ *devname, ধ্রুব বর্ণ *lockmode)
{
	পূর্ণাঙ্ক oper, rc, বার্তা = 0;

	যদি (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	যদি (!lockmode)
		প্রত্যাগম 0;

	যদি (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	অন্যথা যদি (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	অন্যথা যদি (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		প্রত্যাগম 0;
	অন্যথা {
		warnx(_("unsupported lock mode: %s"), lockmode);
		প্রত্যাগম -EINVAL;
	}

	যদি (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		যদি (rc == 0)
			প্রত্যাগম 0;
		যদি (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			বার্তা = 1;
		}
	}
	rc = flock(fd, oper);
	যদি (rc != 0) {
		বিকল্প (errno) {
		পক্ষ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ভঙ্গ;
		স্বভাব:
			warn(_("%s: failed to get lock"), devname);
		}
	} অন্যথা যদি (বার্তা)
		fprintf(stderr, _("OK\n"));
	প্রত্যাগম rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
সংরচনা blk_zone_report *blkdev_get_zonereport(পূর্ণাঙ্ক fd, uint64_t sector, uint32_t nzones)
{
	সংরচনা blk_zone_report *rep;
	size_t rep_size;
	পূর্ণাঙ্ক ret;

	rep_size = পরিমাণ(সংরচনা blk_zone_report) + পরিমাণ(সংরচনা blk_zone) * nzones;
	rep = calloc(1, rep_size);
	যদি (!rep)
		প্রত্যাগম রিক্ত;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	যদি (ret || rep->nr_zones != nzones) {
		মুক্ত(rep);
		প্রত্যাগম রিক্ত;
	}

	প্রত্যাগম rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <fcntl.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	uint64_t bytes, sectors;
	পূর্ণাঙ্ক sector_size, phy_sector_size;
	পূর্ণাঙ্ক fd;

	যদি (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	যদি ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	যদি (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	যদি (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	যদি (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	যদি (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	প্রত্যাগম EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
