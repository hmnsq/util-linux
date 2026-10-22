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

#اگر_تعریف_شده HAVE_LINUX_FD_H
#ضمن <linux/fd.h>
#پایان_اگر

#اگر_تعریف_شده HAVE_LINUX_BLKZONED_H
#ضمن <linux/blkzoned.h>
#پایان_اگر

#اگر_تعریف_شده HAVE_SYS_DISKLABEL_H
#ضمن <sys/disklabel.h>
#پایان_اگر

#اگر_تعریف_شده HAVE_SYS_DISK_H
# include <sys/disk.h>
#پایان_اگر

#اگر_تعریف_نشده EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#پایان_اگر

#ضمن "all-io.h"
#ضمن "blkdev.h"
#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "linux_version.h"
#ضمن "fileutils.h"
#ضمن "nls.h"

ساكن دراز blkdev_valid_جابه_جایی(صحيح fd, off_t جابه_جایی)
{
	حرف ch;

	إذا (lseek (fd, جابه_جایی, 0) < 0)
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

off_t blkdev_پیدا_کن_اندازه (صحيح fd)
{
	off_t high, کم = 0;

	لكل (high = 1024; blkdev_valid_جابه_جایی(fd, high); ) {
		إذا (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			أرجع -1;
		}

		کم = high;

		إذا (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		وإلا
			high *= 2;
	}

	طالما (کم < high - 1) {
		off_t mid = (کم + high) / 2;

		إذا (blkdev_valid_جابه_جایی (fd, mid))
			کم = mid;
		وإلا
			high = mid;
	}
	blkdev_valid_جابه_جایی (fd, 0);
	أرجع (کم + 1);
}

/* get size in bytes */
صحيح blkdev_get_اندازه(صحيح fd, uint64_t *بایتها)
{
#اگر_تعریف_شده DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	إذا (ioctl(fd, DKIOCGETBLOCKCOUNT, بایتها) >= 0) {
		*بایتها <<= 9;
		أرجع 0;
	}
#پایان_اگر

#اگر_تعریف_شده BLKGETSIZE64
	إذا (ioctl(fd, BLKGETSIZE64, بایتها) >= 0)
		أرجع 0;
#پایان_اگر

#اگر_تعریف_شده BLKGETSIZE
	{
		بیعلامت دراز اندازه;

		إذا (ioctl(fd, BLKGETSIZE, &اندازه) >= 0) {
			*بایتها = ((uint64_t) اندازه) << 9;
			أرجع 0;
		}
	}

#پایان_اگر /* BLKGETSIZE */

#اگر_تعریف_شده DIOCGMEDIASIZE
	/* FreeBSD */
	إذا (ioctl(fd, DIOCGMEDIASIZE, بایتها) >= 0)
		أرجع 0;
#پایان_اگر

#اگر_تعریف_شده FDGETPRM
	{
		بنية floppy_struct this_floppy;

		إذا (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*بایتها = ((uint64_t) this_floppy.اندازه) << 9;
			أرجع 0;
		}
	}
#پایان_اگر /* FDGETPRM */

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
		بنية افراز *pp;
		بنية stat st;

		إذا ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		إذا (part >= 0 && (ioctl(fd, DIOCGDINFO, (حرف *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			إذا (pp->p_اندازه) {
				 *بایتها = ((uint64_t) pp->p_اندازه) << 9;
				 أرجع 0;
			}
		}
	}
#پایان_اگر /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		بنية stat st;

		إذا (fstat(fd, &st) != 0)
			أرجع -1;
		إذا (S_ISREG(st.st_mode)) {
			*بایتها = (uint64_t) st.st_size;
			أرجع 0;
		}
		إذا (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			أرجع -1;
		}
	}

	{
		off_t اندازه = blkdev_پیدا_کن_اندازه(fd);

		إذا (اندازه < 0)
			أرجع -1;

		*بایتها = (uint64_t) اندازه;
	}

	أرجع 0;
}

/* get 512-byte sector count */
صحيح blkdev_get_sectors(صحيح fd, uint64_t *sectors)
{
	uint64_t بایتها;

	إذا (blkdev_get_اندازه(fd, &بایتها) == 0) {
		*sectors = (بایتها >> 9);
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
#اگر_تعریف_شده BLKSSZGET
صحيح blkdev_get_sector_اندازه(صحيح fd, صحيح *sector_اندازه)
{
	إذا (ioctl(fd, BLKSSZGET, sector_اندازه) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_sector_اندازه(صحيح fd __attribute__((__unused__)), صحيح *sector_اندازه)
{
	*sector_اندازه = DEFAULT_SECTOR_اندازه;
	أرجع 0;
}
#پایان_اگر

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
#اگر_تعریف_شده BLKPBSZGET
صحيح blkdev_get_physector_اندازه(صحيح fd, صحيح *sector_اندازه)
{
	إذا (ioctl(fd, BLKPBSZGET, sector_اندازه) >= 0)
		أرجع 0;
	أرجع -1;
}
#وإلا
صحيح blkdev_get_physector_اندازه(صحيح fd __attribute__((__unused__)), صحيح *sector_اندازه)
{
	*sector_اندازه = DEFAULT_SECTOR_اندازه;
	أرجع 0;
}
#پایان_اگر

/*
 * Return the alignment status of a device
 */
#اگر_تعریف_شده BLKALIGNOFF
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
#پایان_اگر

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

#اگر_تعریف_شده CDROM_GET_CAPABILITY
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
#پایان_اگر

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#اگر_تعریف_شده HDIO_GETGEO
صحيح blkdev_get_geometry(صحيح fd, بیعلامت صحيح *h, بیعلامت صحيح *s)
{
	بنية hd_geometry geometry;

	إذا (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		أرجع 0;
	}
#وإلا
صحيح blkdev_get_geometry(صحيح fd __attribute__((__unused__)),
		بیعلامت صحيح *h, بیعلامت صحيح *s)
{
	*h = 0;
	*s = 0;
#پایان_اگر
	أرجع -1;
}

/*
 * Convert scsi type to human readable string.
 */
ثابت حرف *blkdev_scsi_type_مقصد_name(صحيح type)
{
	بدل (type) {
	حالة SCSI_TYPE_DISK:
		أرجع "disk";
	حالة SCSI_TYPE_نوار:
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
صحيح blkdev_قفل(صحيح fd, ثابت حرف *devname, ثابت حرف *lockmode)
{
	صحيح oper, rc, پیام = 0;

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
					برنامه_invocation_short_name, devname);
			پیام = 1;
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
	} وإلا إذا (پیام)
		fprintf(stderr, _("OK\n"));
	أرجع rc;
}

#اگر_تعریف_شده HAVE_LINUX_BLKZONED_H
بنية blk_zone_report *blkdev_get_zonereport(صحيح fd, uint64_t sector, uint32_t nzones)
{
	بنية blk_zone_report *rep;
	size_t rep_اندازه;
	صحيح ret;

	rep_اندازه = حجم(بنية blk_zone_report) + حجم(بنية blk_zone) * nzones;
	rep = حافظه_صفر_بده(1, rep_اندازه);
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
#پایان_اگر


#اگر_تعریف_شده TEST_برنامه_BLKDEV
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <fcntl.h>

صحيح اصلی(صحيح argc, حرف **argv)
{
	uint64_t بایتها, sectors;
	صحيح sector_اندازه, phy_sector_اندازه;
	صحيح fd;

	إذا (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		خروج(EXIT_FAILURE);
	}

	إذا ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	إذا (blkdev_get_اندازه(fd, &بایتها) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	إذا (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	إذا (blkdev_get_sector_اندازه(fd, &sector_اندازه) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	إذا (blkdev_get_physector_اندازه(fd, &phy_sector_اندازه) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	چاپ_کن("          bytes: %" PRIu64 "\n", بایتها);
	چاپ_کن("        sectors: %" PRIu64 "\n", sectors);
	چاپ_کن("    sector size: %d\n", sector_اندازه);
	چاپ_کن("phy-sector size: %d\n", phy_sector_اندازه);

	أرجع EXIT_SUCCESS;
}
#پایان_اگر /* TEST_PROGRAM_BLKDEV */
