/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <sys/file.h>
#شامل <sys/ioctl.h>
#شامل <errno.h>
#شامل <unistd.h>
#شامل <stdint.h>

#اگر_تعریف_شدہ HAVE_LINUX_FD_H
#شامل <linux/fd.h>
#اگر_ختم

#اگر_تعریف_شدہ HAVE_LINUX_BLKZONED_H
#شامل <linux/blkzoned.h>
#اگر_ختم

#اگر_تعریف_شدہ HAVE_SYS_DISKLABEL_H
#شامل <sys/disklabel.h>
#اگر_ختم

#اگر_تعریف_شدہ HAVE_SYS_DISK_H
# include <sys/disk.h>
#اگر_ختم

#اگر_غیر_تعریف_شدہ EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#اگر_ختم

#شامل "all-io.h"
#شامل "blkdev.h"
#شامل "c.h"
#شامل "cctype.h"
#شامل "linux_version.h"
#شامل "fileutils.h"
#شامل "nls.h"

ساکن طویل blkdev_valid_انحراف(صحیح fd, off_t انحراف)
{
	حرف ch;

	اگر (lseek (fd, انحراف, 0) < 0)
		واپس 0;
	اگر (ul_read_all (fd, &ch, 1) < 1)
		واپس 0;
	واپس 1;
}

صحیح is_blkdev(صحیح fd)
{
	ساخت stat st;
	واپس (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_ڈھونڈو_سائز (صحیح fd)
{
	off_t high, کم = 0;

	برائے (high = 1024; blkdev_valid_انحراف(fd, high); ) {
		اگر (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			واپس -1;
		}

		کم = high;

		اگر (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ورنہ
			high *= 2;
	}

	جب (کم < high - 1) {
		off_t mid = (کم + high) / 2;

		اگر (blkdev_valid_انحراف (fd, mid))
			کم = mid;
		ورنہ
			high = mid;
	}
	blkdev_valid_انحراف (fd, 0);
	واپس (کم + 1);
}

/* get size in bytes */
صحیح blkdev_get_سائز(صحیح fd, uint64_t *بائٹیں)
{
#اگر_تعریف_شدہ DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	اگر (ioctl(fd, DKIOCGETBLOCKCOUNT, بائٹیں) >= 0) {
		*بائٹیں <<= 9;
		واپس 0;
	}
#اگر_ختم

#اگر_تعریف_شدہ BLKGETSIZE64
	اگر (ioctl(fd, BLKGETSIZE64, بائٹیں) >= 0)
		واپس 0;
#اگر_ختم

#اگر_تعریف_شدہ BLKGETSIZE
	{
		غیرعلامتی طویل سائز;

		اگر (ioctl(fd, BLKGETSIZE, &سائز) >= 0) {
			*بائٹیں = ((uint64_t) سائز) << 9;
			واپس 0;
		}
	}

#اگر_ختم /* BLKGETSIZE */

#اگر_تعریف_شدہ DIOCGMEDIASIZE
	/* FreeBSD */
	اگر (ioctl(fd, DIOCGMEDIASIZE, بائٹیں) >= 0)
		واپس 0;
#اگر_ختم

#اگر_تعریف_شدہ FDGETPRM
	{
		ساخت floppy_struct this_floppy;

		اگر (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*بائٹیں = ((uint64_t) this_floppy.سائز) << 9;
			واپس 0;
		}
	}
#اگر_ختم /* FDGETPRM */

#اگر defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		صحیح part = -1;
		ساخت disklabel lab;
		ساخت تقسیم *pp;
		ساخت stat st;

		اگر ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		اگر (part >= 0 && (ioctl(fd, DIOCGDINFO, (حرف *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			اگر (pp->p_سائز) {
				 *بائٹیں = ((uint64_t) pp->p_سائز) << 9;
				 واپس 0;
			}
		}
	}
#اگر_ختم /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ساخت stat st;

		اگر (fstat(fd, &st) != 0)
			واپس -1;
		اگر (S_ISREG(st.st_mode)) {
			*بائٹیں = (uint64_t) st.st_size;
			واپس 0;
		}
		اگر (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			واپس -1;
		}
	}

	{
		off_t سائز = blkdev_ڈھونڈو_سائز(fd);

		اگر (سائز < 0)
			واپس -1;

		*بائٹیں = (uint64_t) سائز;
	}

	واپس 0;
}

/* get 512-byte sector count */
صحیح blkdev_get_sectors(صحیح fd, uint64_t *sectors)
{
	uint64_t بائٹیں;

	اگر (blkdev_get_سائز(fd, &بائٹیں) == 0) {
		*sectors = (بائٹیں >> 9);
		واپس 0;
	}

	واپس -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#اگر_تعریف_شدہ BLKSSZGET
صحیح blkdev_get_sector_سائز(صحیح fd, صحیح *sector_سائز)
{
	اگر (ioctl(fd, BLKSSZGET, sector_سائز) >= 0)
		واپس 0;
	واپس -1;
}
#ورنہ
صحیح blkdev_get_sector_سائز(صحیح fd __attribute__((__unused__)), صحیح *sector_سائز)
{
	*sector_سائز = DEFAULT_SECTOR_سائز;
	واپس 0;
}
#اگر_ختم

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
#اگر_تعریف_شدہ BLKPBSZGET
صحیح blkdev_get_physector_سائز(صحیح fd, صحیح *sector_سائز)
{
	اگر (ioctl(fd, BLKPBSZGET, sector_سائز) >= 0)
		واپس 0;
	واپس -1;
}
#ورنہ
صحیح blkdev_get_physector_سائز(صحیح fd __attribute__((__unused__)), صحیح *sector_سائز)
{
	*sector_سائز = DEFAULT_SECTOR_سائز;
	واپس 0;
}
#اگر_ختم

/*
 * Return the alignment status of a device
 */
#اگر_تعریف_شدہ BLKALIGNOFF
صحیح blkdev_is_misaligned(صحیح fd)
{
	صحیح aligned;

	اگر (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		واپس 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	واپس aligned != 0 ? 1 : 0;
}
#ورنہ
صحیح blkdev_is_misaligned(صحیح fd __attribute__((__unused__)))
{
	واپس 0;
}
#اگر_ختم

صحیح open_blkdev_or_file(ثابت ساخت stat *st, ثابت حرف *name, ثابت صحیح oflag)
{
	صحیح fd;

	اگر (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ورنہ
		fd = open(name, oflag);
	اگر (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		واپس -1;
	}
	اگر (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	واپس fd;
}

#اگر_تعریف_شدہ CDROM_GET_CAPABILITY
صحیح blkdev_is_cdrom(صحیح fd)
{
	صحیح ret = ioctl(fd, CDROM_GET_CAPABILITY, تہی);
	واپس ret < 0 ? 0 : ret;
}
#ورنہ
صحیح blkdev_is_cdrom(صحیح fd __attribute__((__unused__)))
{
	واپس 0;
}
#اگر_ختم

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#اگر_تعریف_شدہ HDIO_GETGEO
صحیح blkdev_get_geometry(صحیح fd, غیرعلامتی صحیح *h, غیرعلامتی صحیح *s)
{
	ساخت hd_geometry geometry;

	اگر (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		واپس 0;
	}
#ورنہ
صحیح blkdev_get_geometry(صحیح fd __attribute__((__unused__)),
		غیرعلامتی صحیح *h, غیرعلامتی صحیح *s)
{
	*h = 0;
	*s = 0;
#اگر_ختم
	واپس -1;
}

/*
 * Convert scsi type to human readable string.
 */
ثابت حرف *blkdev_scsi_type_مقصد_name(صحیح type)
{
	انتخاب (type) {
	حالت SCSI_TYPE_DISK:
		واپس "disk";
	حالت SCSI_TYPE_فیتہ:
		واپس "tape";
	حالت SCSI_TYPE_PRINTER:
		واپس "printer";
	حالت SCSI_TYPE_PROCESSOR:
		واپس "processor";
	حالت SCSI_TYPE_WORM:
		واپس "worm";
	حالت SCSI_TYPE_ROM:
		واپس "rom";
	حالت SCSI_TYPE_SCANNER:
		واپس "scanner";
	حالت SCSI_TYPE_MOD:
		واپس "mo-disk";
	حالت SCSI_TYPE_MEDIUM_CHANGER:
		واپس "changer";
	حالت SCSI_TYPE_COMM:
		واپس "comm";
	حالت SCSI_TYPE_RAID:
		واپس "raid";
	حالت SCSI_TYPE_ENCLOSURE:
		واپس "enclosure";
	حالت SCSI_TYPE_RBC:
		واپس "rbc";
	حالت SCSI_TYPE_OSD:
		واپس "osd";
	حالت SCSI_TYPE_NO_LUN:
		واپس "no-lun";
	طےشدہ:
		توڑو;
	}
	واپس تہی;
}

/* return 0 on success */
صحیح blkdev_تالا(صحیح fd, ثابت حرف *devname, ثابت حرف *lockmode)
{
	صحیح oper, rc, پیغام = 0;

	اگر (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	اگر (!lockmode)
		واپس 0;

	اگر (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ورنہ اگر (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ورنہ اگر (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		واپس 0;
	ورنہ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		واپس -EINVAL;
	}

	اگر (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		اگر (rc == 0)
			واپس 0;
		اگر (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					پروگرام_invocation_short_name, devname);
			پیغام = 1;
		}
	}
	rc = flock(fd, oper);
	اگر (rc != 0) {
		انتخاب (errno) {
		حالت EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			توڑو;
		طےشدہ:
			warn(_("%s: failed to get lock"), devname);
		}
	} ورنہ اگر (پیغام)
		fprintf(stderr, _("OK\n"));
	واپس rc;
}

#اگر_تعریف_شدہ HAVE_LINUX_BLKZONED_H
ساخت blk_zone_report *blkdev_get_zonereport(صحیح fd, uint64_t sector, uint32_t nzones)
{
	ساخت blk_zone_report *rep;
	size_t rep_سائز;
	صحیح ret;

	rep_سائز = حجم(ساخت blk_zone_report) + حجم(ساخت blk_zone) * nzones;
	rep = صفر_حافظہ_دو(1, rep_سائز);
	اگر (!rep)
		واپس تہی;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	اگر (ret || rep->nr_zones != nzones) {
		آزاد(rep);
		واپس تہی;
	}

	واپس rep;
}
#اگر_ختم


#اگر_تعریف_شدہ TEST_پروگرام_BLKDEV
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <fcntl.h>

صحیح مین(صحیح argc, حرف **argv)
{
	uint64_t بائٹیں, sectors;
	صحیح sector_سائز, phy_sector_سائز;
	صحیح fd;

	اگر (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		باہر_نکلو(EXIT_FAILURE);
	}

	اگر ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	اگر (blkdev_get_سائز(fd, &بائٹیں) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	اگر (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	اگر (blkdev_get_sector_سائز(fd, &sector_سائز) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	اگر (blkdev_get_physector_سائز(fd, &phy_sector_سائز) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	لکھو("          bytes: %" PRIu64 "\n", بائٹیں);
	لکھو("        sectors: %" PRIu64 "\n", sectors);
	لکھو("    sector size: %d\n", sector_سائز);
	لکھو("phy-sector size: %d\n", phy_sector_سائز);

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_BLKDEV */
