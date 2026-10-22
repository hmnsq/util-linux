/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <sys/file.h>
#ضمن <sys/ioctl.h>
#ضمن <errno.h>
#ضمن <unistd.h>
#ضمن <stdint.h>

#ifdef HAVE_LINUX_FD_H
#ضمن <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#ضمن <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#ضمن <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#ضمن "all-io.h"
#ضمن "blkdev.h"
#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "linux_version.h"
#ضمن "fileutils.h"
#ضمن "nls.h"

ساكن long blkdev_valid_offset(صحيح fd, off_t offset)
{
	حرف ch;

	إذا (lseek (fd, offset, 0) < 0)
		أرجع 0;
	إذا (ul_read_all (fd, &ch, 1) < 1)
		أرجع 0;
	أرجع 1;
}

صحيح is_blkdev(صحيح fd)
{
	بنية stat st;
	أرجع (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (صحيح fd)
{
	off_t high, رنده = 0;

	لكل (high = 1024; blkdev_valid_offset(fd, high); ) {
		إذا (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			أرجع -1;
		}

		رنده = high;

		إذا (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		وإلا
			high *= 2;
	}

	طالما (رنده < high - 1) {
		off_t mid = (رنده + high) / 2;

		إذا (blkdev_valid_offset (fd, mid))
			رنده = mid;
		وإلا
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	أرجع (رنده + 1);
}

/* get size in bytes */
صحيح blkdev_get_size(صحيح fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	إذا (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		أرجع 0;
	}
#endif

#ifdef BLKGETSIZE64
	إذا (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		أرجع 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		إذا (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			أرجع 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	إذا (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		أرجع 0;
#endif

#ifdef FDGETPRM
	{
		بنية floppy_struct this_floppy;

		إذا (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			أرجع 0;
		}
	}
#endif /* FDGETPRM */

#إذا defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		صحيح part = -1;
		بنية disklabel lab;
		بنية partition *pp;
		بنية stat st;

		إذا ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		إذا (part >= 0 && (ioctl(fd, DIOCGDINFO, (حرف *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			إذا (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 أرجع 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		بنية stat st;

		إذا (fstat(fd, &st) != 0)
			أرجع -1;
		إذا (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			أرجع 0;
		}
		إذا (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			أرجع -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		إذا (size < 0)
			أرجع -1;

		*bytes = (uint64_t) size;
	}

	أرجع 0;
}

/* get 512-byte sector count */
صحيح blkdev_get_sectors(صحيح fd, uint64_t *sectors)
{
	uint64_t bytes;

	إذا (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		أرجع 0;
	}

	أرجع -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
صحيح blkdev_get_sector_size(صحيح fd, صحيح *sector_size)
{
	إذا (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_sector_size(صحيح fd __attribute__((__unused__)), صحيح *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	أرجع 0;
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
صحيح blkdev_get_physector_size(صحيح fd, صحيح *sector_size)
{
	إذا (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_physector_size(صحيح fd __attribute__((__unused__)), صحيح *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	أرجع 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
صحيح blkdev_is_misaligned(صحيح fd)
{
	صحيح aligned;

	إذا (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		أرجع 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	أرجع aligned != 0 ? 1 : 0;
}
#وإلا
صحيح blkdev_is_misaligned(صحيح fd __attribute__((__unused__)))
{
	أرجع 0;
}
#endif

صحيح open_blkdev_or_file(ثابت بنية stat *st, ثابت حرف *name, ثابت صحيح oflag)
{
	صحيح fd;

	إذا (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} وإلا
		fd = open(name, oflag);
	إذا (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		أرجع -1;
	}
	إذا (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	أرجع fd;
}

#ifdef CDROM_GET_CAPABILITY
صحيح blkdev_is_cdrom(صحيح fd)
{
	صحيح ret = ioctl(fd, CDROM_GET_CAPABILITY, عدم);
	أرجع ret < 0 ? 0 : ret;
}
#وإلا
صحيح blkdev_is_cdrom(صحيح fd __attribute__((__unused__)))
{
	أرجع 0;
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
صحيح blkdev_get_geometry(صحيح fd, unsigned صحيح *h, unsigned صحيح *s)
{
	بنية hd_geometry geometry;

	إذا (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		أرجع 0;
	}
#وإلا
صحيح blkdev_get_geometry(صحيح fd __attribute__((__unused__)),
		unsigned صحيح *h, unsigned صحيح *s)
{
	*h = 0;
	*s = 0;
#endif
	أرجع -1;
}

/*
 * Convert scsi type to human readable string.
 */
ثابت حرف *blkdev_scsi_type_to_name(صحيح type)
{
	بدل (type) {
	حالة SCSI_TYPE_DISK:
		أرجع "disk";
	حالة SCSI_TYPE_TAPE:
		أرجع "tape";
	حالة SCSI_TYPE_PRINTER:
		أرجع "printer";
	حالة SCSI_TYPE_PROCESSOR:
		أرجع "processor";
	حالة SCSI_TYPE_WORM:
		أرجع "worm";
	حالة SCSI_TYPE_ROM:
		أرجع "rom";
	حالة SCSI_TYPE_SCANNER:
		أرجع "scanner";
	حالة SCSI_TYPE_MOD:
		أرجع "mo-disk";
	حالة SCSI_TYPE_MEDIUM_CHANGER:
		أرجع "changer";
	حالة SCSI_TYPE_COMM:
		أرجع "comm";
	حالة SCSI_TYPE_RAID:
		أرجع "raid";
	حالة SCSI_TYPE_ENCLOSURE:
		أرجع "enclosure";
	حالة SCSI_TYPE_RBC:
		أرجع "rbc";
	حالة SCSI_TYPE_OSD:
		أرجع "osd";
	حالة SCSI_TYPE_NO_LUN:
		أرجع "no-lun";
	افتراضي:
		اكسر;
	}
	أرجع عدم;
}

/* return 0 on success */
صحيح blkdev_lock(صحيح fd, ثابت حرف *devname, ثابت حرف *lockmode)
{
	صحيح oper, rc, ڤسنن = 0;

	إذا (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	إذا (!lockmode)
		أرجع 0;

	إذا (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	وإلا إذا (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	وإلا إذا (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		أرجع 0;
	وإلا {
		warnx(_("unsupported lock mode: %s"), lockmode);
		أرجع -EINVAL;
	}

	إذا (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		إذا (rc == 0)
			أرجع 0;
		إذا (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			ڤسنن = 1;
		}
	}
	rc = flock(fd, oper);
	إذا (rc != 0) {
		بدل (errno) {
		حالة EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			اكسر;
		افتراضي:
			warn(_("%s: failed to get lock"), devname);
		}
	} وإلا إذا (ڤسنن)
		fprintf(stderr, _("OK\n"));
	أرجع rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
بنية blk_zone_report *blkdev_get_zonereport(صحيح fd, uint64_t sector, uint32_t nzones)
{
	بنية blk_zone_report *rep;
	size_t rep_size;
	صحيح ret;

	rep_size = حجم(بنية blk_zone_report) + حجم(بنية blk_zone) * nzones;
	rep = calloc(1, rep_size);
	إذا (!rep)
		أرجع عدم;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	إذا (ret || rep->nr_zones != nzones) {
		حرر(rep);
		أرجع عدم;
	}

	أرجع rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <fcntl.h>

صحيح main(صحيح argc, حرف **argv)
{
	uint64_t bytes, sectors;
	صحيح sector_size, phy_sector_size;
	صحيح fd;

	إذا (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	إذا ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	إذا (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	إذا (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	إذا (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	إذا (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	أرجع EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
