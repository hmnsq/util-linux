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

#ifdef HAVE_LINUX_FD_H
#شامل <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#شامل <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#شامل <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#شامل "all-io.h"
#شامل "blkdev.h"
#شامل "c.h"
#شامل "cctype.h"
#شامل "linux_version.h"
#شامل "fileutils.h"
#شامل "nls.h"

ایستا long blkdev_valid_offset(صحیح fd, off_t offset)
{
	نویسه ch;

	اگر (lseek (fd, offset, 0) < 0)
		بازگشت 0;
	اگر (ul_read_all (fd, &ch, 1) < 1)
		بازگشت 0;
	بازگشت 1;
}

صحیح is_blkdev(صحیح fd)
{
	ساختار stat st;
	بازگشت (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (صحیح fd)
{
	off_t high, کم = 0;

	برای (high = 1024; blkdev_valid_offset(fd, high); ) {
		اگر (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			بازگشت -1;
		}

		کم = high;

		اگر (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		وگرنه
			high *= 2;
	}

	وقتی (کم < high - 1) {
		off_t mid = (کم + high) / 2;

		اگر (blkdev_valid_offset (fd, mid))
			کم = mid;
		وگرنه
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	بازگشت (کم + 1);
}

/* get size in bytes */
صحیح blkdev_get_size(صحیح fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	اگر (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		بازگشت 0;
	}
#endif

#ifdef BLKGETSIZE64
	اگر (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		بازگشت 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		اگر (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			بازگشت 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	اگر (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		بازگشت 0;
#endif

#ifdef FDGETPRM
	{
		ساختار floppy_struct this_floppy;

		اگر (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			بازگشت 0;
		}
	}
#endif /* FDGETPRM */

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
		ساختار disklabel lab;
		ساختار partition *pp;
		ساختار stat st;

		اگر ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		اگر (part >= 0 && (ioctl(fd, DIOCGDINFO, (نویسه *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			اگر (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 بازگشت 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ساختار stat st;

		اگر (fstat(fd, &st) != 0)
			بازگشت -1;
		اگر (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			بازگشت 0;
		}
		اگر (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			بازگشت -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		اگر (size < 0)
			بازگشت -1;

		*bytes = (uint64_t) size;
	}

	بازگشت 0;
}

/* get 512-byte sector count */
صحیح blkdev_get_sectors(صحیح fd, uint64_t *sectors)
{
	uint64_t bytes;

	اگر (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		بازگشت 0;
	}

	بازگشت -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
صحیح blkdev_get_sector_size(صحیح fd, صحیح *sector_size)
{
	اگر (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		بازگشت 0;
	بازگشت -1;
}
#وگرنه
صحیح blkdev_get_sector_size(صحیح fd __attribute__((__unused__)), صحیح *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	بازگشت 0;
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
صحیح blkdev_get_physector_size(صحیح fd, صحیح *sector_size)
{
	اگر (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		بازگشت 0;
	بازگشت -1;
}
#وگرنه
صحیح blkdev_get_physector_size(صحیح fd __attribute__((__unused__)), صحیح *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	بازگشت 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
صحیح blkdev_is_misaligned(صحیح fd)
{
	صحیح aligned;

	اگر (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		بازگشت 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	بازگشت aligned != 0 ? 1 : 0;
}
#وگرنه
صحیح blkdev_is_misaligned(صحیح fd __attribute__((__unused__)))
{
	بازگشت 0;
}
#endif

صحیح open_blkdev_or_file(ثابت ساختار stat *st, ثابت نویسه *name, ثابت صحیح oflag)
{
	صحیح fd;

	اگر (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} وگرنه
		fd = open(name, oflag);
	اگر (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		بازگشت -1;
	}
	اگر (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	بازگشت fd;
}

#ifdef CDROM_GET_CAPABILITY
صحیح blkdev_is_cdrom(صحیح fd)
{
	صحیح ret = ioctl(fd, CDROM_GET_CAPABILITY, پوچ);
	بازگشت ret < 0 ? 0 : ret;
}
#وگرنه
صحیح blkdev_is_cdrom(صحیح fd __attribute__((__unused__)))
{
	بازگشت 0;
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
صحیح blkdev_get_geometry(صحیح fd, unsigned صحیح *h, unsigned صحیح *s)
{
	ساختار hd_geometry geometry;

	اگر (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		بازگشت 0;
	}
#وگرنه
صحیح blkdev_get_geometry(صحیح fd __attribute__((__unused__)),
		unsigned صحیح *h, unsigned صحیح *s)
{
	*h = 0;
	*s = 0;
#endif
	بازگشت -1;
}

/*
 * Convert scsi type to human readable string.
 */
ثابت نویسه *blkdev_scsi_type_to_name(صحیح type)
{
	انتخاب (type) {
	حالت SCSI_TYPE_DISK:
		بازگشت "disk";
	حالت SCSI_TYPE_TAPE:
		بازگشت "tape";
	حالت SCSI_TYPE_PRINTER:
		بازگشت "printer";
	حالت SCSI_TYPE_PROCESSOR:
		بازگشت "processor";
	حالت SCSI_TYPE_WORM:
		بازگشت "worm";
	حالت SCSI_TYPE_ROM:
		بازگشت "rom";
	حالت SCSI_TYPE_SCANNER:
		بازگشت "scanner";
	حالت SCSI_TYPE_MOD:
		بازگشت "mo-disk";
	حالت SCSI_TYPE_MEDIUM_CHANGER:
		بازگشت "changer";
	حالت SCSI_TYPE_COMM:
		بازگشت "comm";
	حالت SCSI_TYPE_RAID:
		بازگشت "raid";
	حالت SCSI_TYPE_ENCLOSURE:
		بازگشت "enclosure";
	حالت SCSI_TYPE_RBC:
		بازگشت "rbc";
	حالت SCSI_TYPE_OSD:
		بازگشت "osd";
	حالت SCSI_TYPE_NO_LUN:
		بازگشت "no-lun";
	پیشفرض:
		بشکن;
	}
	بازگشت پوچ;
}

/* return 0 on success */
صحیح blkdev_lock(صحیح fd, ثابت نویسه *devname, ثابت نویسه *lockmode)
{
	صحیح oper, rc, پیام = 0;

	اگر (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	اگر (!lockmode)
		بازگشت 0;

	اگر (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	وگرنه اگر (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	وگرنه اگر (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		بازگشت 0;
	وگرنه {
		warnx(_("unsupported lock mode: %s"), lockmode);
		بازگشت -EINVAL;
	}

	اگر (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		اگر (rc == 0)
			بازگشت 0;
		اگر (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			پیام = 1;
		}
	}
	rc = flock(fd, oper);
	اگر (rc != 0) {
		انتخاب (errno) {
		حالت EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			بشکن;
		پیشفرض:
			warn(_("%s: failed to get lock"), devname);
		}
	} وگرنه اگر (پیام)
		fprintf(stderr, _("OK\n"));
	بازگشت rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
ساختار blk_zone_report *blkdev_get_zonereport(صحیح fd, uint64_t sector, uint32_t nzones)
{
	ساختار blk_zone_report *rep;
	size_t rep_size;
	صحیح ret;

	rep_size = اندازه(ساختار blk_zone_report) + اندازه(ساختار blk_zone) * nzones;
	rep = calloc(1, rep_size);
	اگر (!rep)
		بازگشت پوچ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	اگر (ret || rep->nr_zones != nzones) {
		آزاد(rep);
		بازگشت پوچ;
	}

	بازگشت rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <fcntl.h>

صحیح اصلی(صحیح argc, نویسه **argv)
{
	uint64_t bytes, sectors;
	صحیح sector_size, phy_sector_size;
	صحیح fd;

	اگر (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	اگر ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	اگر (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	اگر (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	اگر (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	اگر (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
