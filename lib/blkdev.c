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

স্থির long blkdev_valid_offset(পূর্ণসংখ্যা fd, off_t offset)
{
	অক্ষর ch;

	যদি (lseek (fd, offset, 0) < 0)
		ফেরত 0;
	যদি (ul_read_all (fd, &ch, 1) < 1)
		ফেরত 0;
	ফেরত 1;
}

পূর্ণসংখ্যা is_blkdev(পূর্ণসংখ্যা fd)
{
	কাঠামো stat st;
	ফেরত (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_আকার (পূর্ণসংখ্যা fd)
{
	off_t high, নিচের = 0;

	জন্য (high = 1024; blkdev_valid_offset(fd, high); ) {
		যদি (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ফেরত -1;
		}

		নিচের = high;

		যদি (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		অন্যথা
			high *= 2;
	}

	যখন (নিচের < high - 1) {
		off_t mid = (নিচের + high) / 2;

		যদি (blkdev_valid_offset (fd, mid))
			নিচের = mid;
		অন্যথা
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ফেরত (নিচের + 1);
}

/* get size in bytes */
পূর্ণসংখ্যা blkdev_get_আকার(পূর্ণসংখ্যা fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	যদি (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ফেরত 0;
	}
#endif

#ifdef BLKGETSIZE64
	যদি (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ফেরত 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long আকার_2;

		যদি (ioctl(fd, BLKGETSIZE, &আকার_2) >= 0) {
			*bytes = ((uint64_t) আকার_2) << 9;
			ফেরত 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	যদি (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ফেরত 0;
#endif

#ifdef FDGETPRM
	{
		কাঠামো floppy_struct this_floppy;

		যদি (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.আকার_2) << 9;
			ফেরত 0;
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
		পূর্ণসংখ্যা part = -1;
		কাঠামো disklabel lab;
		কাঠামো partition *pp;
		কাঠামো stat st;

		যদি ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		যদি (part >= 0 && (ioctl(fd, DIOCGDINFO, (অক্ষর *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			যদি (pp->p_আকার) {
				 *bytes = ((uint64_t) pp->p_আকার) << 9;
				 ফেরত 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		কাঠামো stat st;

		যদি (fstat(fd, &st) != 0)
			ফেরত -1;
		যদি (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ফেরত 0;
		}
		যদি (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ফেরত -1;
		}
	}

	{
		off_t আকার_2 = blkdev_find_আকার(fd);

		যদি (আকার_2 < 0)
			ফেরত -1;

		*bytes = (uint64_t) আকার_2;
	}

	ফেরত 0;
}

/* get 512-byte sector count */
পূর্ণসংখ্যা blkdev_get_sectors(পূর্ণসংখ্যা fd, uint64_t *sectors)
{
	uint64_t bytes;

	যদি (blkdev_get_আকার(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ফেরত 0;
	}

	ফেরত -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
পূর্ণসংখ্যা blkdev_get_sector_আকার(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা *sector_আকার)
{
	যদি (ioctl(fd, BLKSSZGET, sector_আকার) >= 0)
		ফেরত 0;
	ফেরত -1;
}
#অন্যথা
পূর্ণসংখ্যা blkdev_get_sector_আকার(পূর্ণসংখ্যা fd __attribute__((__unused__)), পূর্ণসংখ্যা *sector_আকার)
{
	*sector_আকার = DEFAULT_SECTOR_আকার;
	ফেরত 0;
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
পূর্ণসংখ্যা blkdev_get_physector_আকার(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা *sector_আকার)
{
	যদি (ioctl(fd, BLKPBSZGET, sector_আকার) >= 0)
		ফেরত 0;
	ফেরত -1;
}
#অন্যথা
পূর্ণসংখ্যা blkdev_get_physector_আকার(পূর্ণসংখ্যা fd __attribute__((__unused__)), পূর্ণসংখ্যা *sector_আকার)
{
	*sector_আকার = DEFAULT_SECTOR_আকার;
	ফেরত 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
পূর্ণসংখ্যা blkdev_is_misaligned(পূর্ণসংখ্যা fd)
{
	পূর্ণসংখ্যা aligned;

	যদি (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ফেরত 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ফেরত aligned != 0 ? 1 : 0;
}
#অন্যথা
পূর্ণসংখ্যা blkdev_is_misaligned(পূর্ণসংখ্যা fd __attribute__((__unused__)))
{
	ফেরত 0;
}
#endif

পূর্ণসংখ্যা open_blkdev_or_file(ধ্রুবক কাঠামো stat *st, ধ্রুবক অক্ষর *name, ধ্রুবক পূর্ণসংখ্যা oflag)
{
	পূর্ণসংখ্যা fd;

	যদি (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} অন্যথা
		fd = open(name, oflag);
	যদি (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ফেরত -1;
	}
	যদি (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ফেরত fd;
}

#ifdef CDROM_GET_CAPABILITY
পূর্ণসংখ্যা blkdev_is_cdrom(পূর্ণসংখ্যা fd)
{
	পূর্ণসংখ্যা ret = ioctl(fd, CDROM_GET_CAPABILITY, ফাঁকা);
	ফেরত ret < 0 ? 0 : ret;
}
#অন্যথা
পূর্ণসংখ্যা blkdev_is_cdrom(পূর্ণসংখ্যা fd __attribute__((__unused__)))
{
	ফেরত 0;
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
পূর্ণসংখ্যা blkdev_get_geometry(পূর্ণসংখ্যা fd, unsigned পূর্ণসংখ্যা *h, unsigned পূর্ণসংখ্যা *s)
{
	কাঠামো hd_geometry geometry;

	যদি (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ফেরত 0;
	}
#অন্যথা
পূর্ণসংখ্যা blkdev_get_geometry(পূর্ণসংখ্যা fd __attribute__((__unused__)),
		unsigned পূর্ণসংখ্যা *h, unsigned পূর্ণসংখ্যা *s)
{
	*h = 0;
	*s = 0;
#endif
	ফেরত -1;
}

/*
 * Convert scsi type to human readable string.
 */
ধ্রুবক অক্ষর *blkdev_scsi_type_to_name(পূর্ণসংখ্যা type)
{
	নির্বাচন (type) {
	ক্ষেত্র SCSI_TYPE_DISK:
		ফেরত "disk";
	ক্ষেত্র SCSI_TYPE_TAPE:
		ফেরত "tape";
	ক্ষেত্র SCSI_TYPE_PRINTER:
		ফেরত "printer";
	ক্ষেত্র SCSI_TYPE_PROCESSOR:
		ফেরত "processor";
	ক্ষেত্র SCSI_TYPE_WORM:
		ফেরত "worm";
	ক্ষেত্র SCSI_TYPE_ROM:
		ফেরত "rom";
	ক্ষেত্র SCSI_TYPE_SCANNER:
		ফেরত "scanner";
	ক্ষেত্র SCSI_TYPE_MOD:
		ফেরত "mo-disk";
	ক্ষেত্র SCSI_TYPE_MEDIUM_CHANGER:
		ফেরত "changer";
	ক্ষেত্র SCSI_TYPE_COMM:
		ফেরত "comm";
	ক্ষেত্র SCSI_TYPE_RAID:
		ফেরত "raid";
	ক্ষেত্র SCSI_TYPE_ENCLOSURE:
		ফেরত "enclosure";
	ক্ষেত্র SCSI_TYPE_RBC:
		ফেরত "rbc";
	ক্ষেত্র SCSI_TYPE_OSD:
		ফেরত "osd";
	ক্ষেত্র SCSI_TYPE_NO_LUN:
		ফেরত "no-lun";
	পূর্বনির্ধারিত:
		বিরতি;
	}
	ফেরত ফাঁকা;
}

/* return 0 on success */
পূর্ণসংখ্যা blkdev_lock(পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *devname, ধ্রুবক অক্ষর *lockmode)
{
	পূর্ণসংখ্যা oper, rc, বার্তা = 0;

	যদি (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	যদি (!lockmode)
		ফেরত 0;

	যদি (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	অন্যথা যদি (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	অন্যথা যদি (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ফেরত 0;
	অন্যথা {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ফেরত -EINVAL;
	}

	যদি (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		যদি (rc == 0)
			ফেরত 0;
		যদি (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			বার্তা = 1;
		}
	}
	rc = flock(fd, oper);
	যদি (rc != 0) {
		নির্বাচন (errno) {
		ক্ষেত্র EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			বিরতি;
		পূর্বনির্ধারিত:
			warn(_("%s: failed to get lock"), devname);
		}
	} অন্যথা যদি (বার্তা)
		fprintf(stderr, _("OK\n"));
	ফেরত rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
কাঠামো blk_zone_report *blkdev_get_zonereport(পূর্ণসংখ্যা fd, uint64_t sector, uint32_t nzones)
{
	কাঠামো blk_zone_report *rep;
	size_t rep_আকার;
	পূর্ণসংখ্যা ret;

	rep_আকার = আকার(কাঠামো blk_zone_report) + আকার(কাঠামো blk_zone) * nzones;
	rep = calloc(1, rep_আকার);
	যদি (!rep)
		ফেরত ফাঁকা;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	যদি (ret || rep->nr_zones != nzones) {
		মুক্ত(rep);
		ফেরত ফাঁকা;
	}

	ফেরত rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <fcntl.h>

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	uint64_t bytes, sectors;
	পূর্ণসংখ্যা sector_আকার, phy_sector_আকার;
	পূর্ণসংখ্যা fd;

	যদি (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	যদি ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	যদি (blkdev_get_আকার(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	যদি (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	যদি (blkdev_get_sector_আকার(fd, &sector_আকার) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	যদি (blkdev_get_physector_আকার(fd, &phy_sector_আকার) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_আকার);
	printf("phy-sector size: %d\n", phy_sector_আকার);

	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
