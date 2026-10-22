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

#جيڪڏهن_وصف_ٿيل HAVE_LINUX_FD_H
#ضمن <linux/fd.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل HAVE_LINUX_BLKZONED_H
#ضمن <linux/blkzoned.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل HAVE_SYS_DISKLABEL_H
#ضمن <sys/disklabel.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل HAVE_SYS_DISK_H
# include <sys/disk.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#جيڪڏهن_پورو

#ضمن "all-io.h"
#ضمن "blkdev.h"
#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "linux_version.h"
#ضمن "fileutils.h"
#ضمن "nls.h"

ساكن ڊگهو blkdev_valid_وٿي(صحيح fd, off_t وٿي)
{
	حرف ch;

	إذا (lseek (fd, وٿي, 0) < 0)
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

off_t blkdev_ڳوليو_سائيز (صحيح fd)
{
	off_t high, هيٺيون = 0;

	لكل (high = 1024; blkdev_valid_وٿي(fd, high); ) {
		إذا (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			أرجع -1;
		}

		هيٺيون = high;

		إذا (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		وإلا
			high *= 2;
	}

	طالما (هيٺيون < high - 1) {
		off_t mid = (هيٺيون + high) / 2;

		إذا (blkdev_valid_وٿي (fd, mid))
			هيٺيون = mid;
		وإلا
			high = mid;
	}
	blkdev_valid_وٿي (fd, 0);
	أرجع (هيٺيون + 1);
}

/* get size in bytes */
صحيح blkdev_get_سائيز(صحيح fd, uint64_t *بائيٽون)
{
#جيڪڏهن_وصف_ٿيل DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	إذا (ioctl(fd, DKIOCGETBLOCKCOUNT, بائيٽون) >= 0) {
		*بائيٽون <<= 9;
		أرجع 0;
	}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل BLKGETSIZE64
	إذا (ioctl(fd, BLKGETSIZE64, بائيٽون) >= 0)
		أرجع 0;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل BLKGETSIZE
	{
		بنانشان ڊگهو سائيز;

		إذا (ioctl(fd, BLKGETSIZE, &سائيز) >= 0) {
			*بائيٽون = ((uint64_t) سائيز) << 9;
			أرجع 0;
		}
	}

#جيڪڏهن_پورو /* BLKGETSIZE */

#جيڪڏهن_وصف_ٿيل DIOCGMEDIASIZE
	/* FreeBSD */
	إذا (ioctl(fd, DIOCGMEDIASIZE, بائيٽون) >= 0)
		أرجع 0;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FDGETPRM
	{
		بنية floppy_struct this_floppy;

		إذا (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*بائيٽون = ((uint64_t) this_floppy.سائيز) << 9;
			أرجع 0;
		}
	}
#جيڪڏهن_پورو /* FDGETPRM */

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
		بنية حصا_ڪريو *pp;
		بنية stat st;

		إذا ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		إذا (part >= 0 && (ioctl(fd, DIOCGDINFO, (حرف *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			إذا (pp->p_سائيز) {
				 *بائيٽون = ((uint64_t) pp->p_سائيز) << 9;
				 أرجع 0;
			}
		}
	}
#جيڪڏهن_پورو /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		بنية stat st;

		إذا (fstat(fd, &st) != 0)
			أرجع -1;
		إذا (S_ISREG(st.st_mode)) {
			*بائيٽون = (uint64_t) st.st_size;
			أرجع 0;
		}
		إذا (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			أرجع -1;
		}
	}

	{
		off_t سائيز = blkdev_ڳوليو_سائيز(fd);

		إذا (سائيز < 0)
			أرجع -1;

		*بائيٽون = (uint64_t) سائيز;
	}

	أرجع 0;
}

/* get 512-byte sector count */
صحيح blkdev_get_sectors(صحيح fd, uint64_t *sectors)
{
	uint64_t بائيٽون;

	إذا (blkdev_get_سائيز(fd, &بائيٽون) == 0) {
		*sectors = (بائيٽون >> 9);
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
#جيڪڏهن_وصف_ٿيل BLKSSZGET
صحيح blkdev_get_sector_سائيز(صحيح fd, صحيح *sector_سائيز)
{
	إذا (ioctl(fd, BLKSSZGET, sector_سائيز) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_sector_سائيز(صحيح fd __attribute__((__unused__)), صحيح *sector_سائيز)
{
	*sector_سائيز = DEFAULT_SECTOR_سائيز;
	أرجع 0;
}
#جيڪڏهن_پورو

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
#جيڪڏهن_وصف_ٿيل BLKPBSZGET
صحيح blkdev_get_physector_سائيز(صحيح fd, صحيح *sector_سائيز)
{
	إذا (ioctl(fd, BLKPBSZGET, sector_سائيز) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_physector_سائيز(صحيح fd __attribute__((__unused__)), صحيح *sector_سائيز)
{
	*sector_سائيز = DEFAULT_SECTOR_سائيز;
	أرجع 0;
}
#جيڪڏهن_پورو

/*
 * Return the alignment status of a device
 */
#جيڪڏهن_وصف_ٿيل BLKALIGNOFF
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
#جيڪڏهن_پورو

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

#جيڪڏهن_وصف_ٿيل CDROM_GET_CAPABILITY
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
#جيڪڏهن_پورو

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#جيڪڏهن_وصف_ٿيل HDIO_GETGEO
صحيح blkdev_get_geometry(صحيح fd, بنانشان صحيح *h, بنانشان صحيح *s)
{
	بنية hd_geometry geometry;

	إذا (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		أرجع 0;
	}
#وإلا
صحيح blkdev_get_geometry(صحيح fd __attribute__((__unused__)),
		بنانشان صحيح *h, بنانشان صحيح *s)
{
	*h = 0;
	*s = 0;
#جيڪڏهن_پورو
	أرجع -1;
}

/*
 * Convert scsi type to human readable string.
 */
ثابت حرف *blkdev_scsi_type_منزل_name(صحيح type)
{
	بدل (type) {
	حالة SCSI_TYPE_DISK:
		أرجع "disk";
	حالة SCSI_TYPE_پٽي:
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
صحيح blkdev_تالو(صحيح fd, ثابت حرف *devname, ثابت حرف *lockmode)
{
	صحيح oper, rc, نياپو = 0;

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
					پروگرام_invocation_short_name, devname);
			نياپو = 1;
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
	} وإلا إذا (نياپو)
		fprintf(stderr, _("OK\n"));
	أرجع rc;
}

#جيڪڏهن_وصف_ٿيل HAVE_LINUX_BLKZONED_H
بنية blk_zone_report *blkdev_get_zonereport(صحيح fd, uint64_t sector, uint32_t nzones)
{
	بنية blk_zone_report *rep;
	size_t rep_سائيز;
	صحيح ret;

	rep_سائيز = حجم(بنية blk_zone_report) + حجم(بنية blk_zone) * nzones;
	rep = صفر_يادگيري_ڏيو(1, rep_سائيز);
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
#جيڪڏهن_پورو


#جيڪڏهن_وصف_ٿيل TEST_پروگرام_BLKDEV
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <fcntl.h>

صحيح مکيه(صحيح argc, حرف **argv)
{
	uint64_t بائيٽون, sectors;
	صحيح sector_سائيز, phy_sector_سائيز;
	صحيح fd;

	إذا (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		ٻاهر_نڪرو(EXIT_FAILURE);
	}

	إذا ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	إذا (blkdev_get_سائيز(fd, &بائيٽون) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	إذا (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	إذا (blkdev_get_sector_سائيز(fd, &sector_سائيز) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	إذا (blkdev_get_physector_سائيز(fd, &phy_sector_سائيز) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	لکو("          bytes: %" PRIu64 "\n", بائيٽون);
	لکو("        sectors: %" PRIu64 "\n", sectors);
	لکو("    sector size: %d\n", sector_سائيز);
	لکو("phy-sector size: %d\n", phy_sector_سائيز);

	أرجع EXIT_SUCCESS;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_BLKDEV */
