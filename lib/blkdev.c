/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/file.h>
#অন্তর্ভাৱ <sys/ioctl.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#অন্তর্ভাৱ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#অন্তর্ভাৱ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#অন্তর্ভাৱ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "blkdev.h"
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "cctype.h"
#অন্তর্ভাৱ "linux_version.h"
#অন্তর্ভাৱ "fileutils.h"
#অন্তর্ভাৱ "nls.h"

স্থির long blkdev_valid_offset(পূর্ণাঙ্ক fd, off_t offset)
{
	ৱর্ণ ch;

	যদি (lseek (fd, offset, 0) < 0)
		ঘূৰাই 0;
	যদি (ul_read_all (fd, &ch, 1) < 1)
		ঘূৰাই 0;
	ঘূৰাই 1;
}

পূর্ণাঙ্ক is_blkdev(পূর্ণাঙ্ক fd)
{
	সংরচনা stat st;
	ঘূৰাই (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (পূর্ণাঙ্ক fd)
{
	off_t high, নিচেৰ = 0;

	বাবে (high = 1024; blkdev_valid_offset(fd, high); ) {
		যদি (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ঘূৰাই -1;
		}

		নিচেৰ = high;

		যদি (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		অন্যথা
			high *= 2;
	}

	যেতিয়ালৈ (নিচেৰ < high - 1) {
		off_t mid = (নিচেৰ + high) / 2;

		যদি (blkdev_valid_offset (fd, mid))
			নিচেৰ = mid;
		অন্যথা
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ঘূৰাই (নিচেৰ + 1);
}

/* get size in bytes */
পূর্ণাঙ্ক blkdev_get_size(পূর্ণাঙ্ক fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	যদি (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ঘূৰাই 0;
	}
#endif

#ifdef BLKGETSIZE64
	যদি (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ঘূৰাই 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		যদি (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ঘূৰাই 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	যদি (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ঘূৰাই 0;
#endif

#ifdef FDGETPRM
	{
		সংরচনা floppy_struct this_floppy;

		যদি (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ঘূৰাই 0;
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

		যদি (part >= 0 && (ioctl(fd, DIOCGDINFO, (ৱর্ণ *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			যদি (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ঘূৰাই 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		সংরচনা stat st;

		যদি (fstat(fd, &st) != 0)
			ঘূৰাই -1;
		যদি (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ঘূৰাই 0;
		}
		যদি (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ঘূৰাই -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		যদি (size < 0)
			ঘূৰাই -1;

		*bytes = (uint64_t) size;
	}

	ঘূৰাই 0;
}

/* get 512-byte sector count */
পূর্ণাঙ্ক blkdev_get_sectors(পূর্ণাঙ্ক fd, uint64_t *sectors)
{
	uint64_t bytes;

	যদি (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ঘূৰাই 0;
	}

	ঘূৰাই -1;
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
		ঘূৰাই 0;
	ঘূৰাই -1;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_sector_size(পূর্ণাঙ্ক fd __attribute__((__unused__)), পূর্ণাঙ্ক *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ঘূৰাই 0;
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
		ঘূৰাই 0;
	ঘূৰাই -1;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_physector_size(পূর্ণাঙ্ক fd __attribute__((__unused__)), পূর্ণাঙ্ক *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ঘূৰাই 0;
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
		ঘূৰাই 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ঘূৰাই aligned != 0 ? 1 : 0;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_is_misaligned(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	ঘূৰাই 0;
}
#endif

পূর্ণাঙ্ক open_blkdev_or_file(ধ্রুৱ সংরচনা stat *st, ধ্রুৱ ৱর্ণ *name, ধ্রুৱ পূর্ণাঙ্ক oflag)
{
	পূর্ণাঙ্ক fd;

	যদি (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} অন্যথা
		fd = open(name, oflag);
	যদি (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ঘূৰাই -1;
	}
	যদি (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ঘূৰাই fd;
}

#ifdef CDROM_GET_CAPABILITY
পূর্ণাঙ্ক blkdev_is_cdrom(পূর্ণাঙ্ক fd)
{
	পূর্ণাঙ্ক ret = ioctl(fd, CDROM_GET_CAPABILITY, রিক্ত);
	ঘূৰাই ret < 0 ? 0 : ret;
}
#অন্যথা
পূর্ণাঙ্ক blkdev_is_cdrom(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	ঘূৰাই 0;
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
		ঘূৰাই 0;
	}
#অন্যথা
পূর্ণাঙ্ক blkdev_get_geometry(পূর্ণাঙ্ক fd __attribute__((__unused__)),
		unsigned পূর্ণাঙ্ক *h, unsigned পূর্ণাঙ্ক *s)
{
	*h = 0;
	*s = 0;
#endif
	ঘূৰাই -1;
}

/*
 * Convert scsi type to human readable string.
 */
ধ্রুৱ ৱর্ণ *blkdev_scsi_type_to_name(পূর্ণাঙ্ক type)
{
	ৱিকল্প (type) {
	পক্ষ SCSI_TYPE_DISK:
		ঘূৰাই "disk";
	পক্ষ SCSI_TYPE_TAPE:
		ঘূৰাই "tape";
	পক্ষ SCSI_TYPE_PRINTER:
		ঘূৰাই "printer";
	পক্ষ SCSI_TYPE_PROCESSOR:
		ঘূৰাই "processor";
	পক্ষ SCSI_TYPE_WORM:
		ঘূৰাই "worm";
	পক্ষ SCSI_TYPE_ROM:
		ঘূৰাই "rom";
	পক্ষ SCSI_TYPE_SCANNER:
		ঘূৰাই "scanner";
	পক্ষ SCSI_TYPE_MOD:
		ঘূৰাই "mo-disk";
	পক্ষ SCSI_TYPE_MEDIUM_CHANGER:
		ঘূৰাই "changer";
	পক্ষ SCSI_TYPE_COMM:
		ঘূৰাই "comm";
	পক্ষ SCSI_TYPE_RAID:
		ঘূৰাই "raid";
	পক্ষ SCSI_TYPE_ENCLOSURE:
		ঘূৰাই "enclosure";
	পক্ষ SCSI_TYPE_RBC:
		ঘূৰাই "rbc";
	পক্ষ SCSI_TYPE_OSD:
		ঘূৰাই "osd";
	পক্ষ SCSI_TYPE_NO_LUN:
		ঘূৰাই "no-lun";
	স্ৱভাৱ:
		ভঙ্গ;
	}
	ঘূৰাই রিক্ত;
}

/* return 0 on success */
পূর্ণাঙ্ক blkdev_lock(পূর্ণাঙ্ক fd, ধ্রুৱ ৱর্ণ *devname, ধ্রুৱ ৱর্ণ *lockmode)
{
	পূর্ণাঙ্ক oper, rc, বাৰ্তা = 0;

	যদি (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	যদি (!lockmode)
		ঘূৰাই 0;

	যদি (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	অন্যথা যদি (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	অন্যথা যদি (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ঘূৰাই 0;
	অন্যথা {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ঘূৰাই -EINVAL;
	}

	যদি (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		যদি (rc == 0)
			ঘূৰাই 0;
		যদি (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			বাৰ্তা = 1;
		}
	}
	rc = flock(fd, oper);
	যদি (rc != 0) {
		ৱিকল্প (errno) {
		পক্ষ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ভঙ্গ;
		স্ৱভাৱ:
			warn(_("%s: failed to get lock"), devname);
		}
	} অন্যথা যদি (বাৰ্তা)
		fprintf(stderr, _("OK\n"));
	ঘূৰাই rc;
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
		ঘূৰাই রিক্ত;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	যদি (ret || rep->nr_zones != nzones) {
		মুক্ত(rep);
		ঘূৰাই রিক্ত;
	}

	ঘূৰাই rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <fcntl.h>

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
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

	ঘূৰাই EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
