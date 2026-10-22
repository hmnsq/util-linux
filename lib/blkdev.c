/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#ಅನ್ತರ್ಭಾವ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#ಅನ್ತರ್ಭಾವ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#ಅನ್ತರ್ಭಾವ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "blkdev.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "linux_version.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "nls.h"

ಸ್ಥಿರ long blkdev_valid_offset(ಪೂರ್ಣಾಙ್ಕ fd, off_t offset)
{
	ವರ್ಣ ch;

	ಯದಿ (lseek (fd, offset, 0) < 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (ul_read_all (fd, &ch, 1) < 1)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ is_blkdev(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಸಂರಚನಾ stat st;
	ಪ್ರತ್ಯಾಗಮ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ಪೂರ್ಣಾಙ್ಕ fd)
{
	off_t high, ಕೆಳಗಿನ = 0;

	ಪ್ರತಿ (high = 1024; blkdev_valid_offset(fd, high); ) {
		ಯದಿ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ಪ್ರತ್ಯಾಗಮ -1;
		}

		ಕೆಳಗಿನ = high;

		ಯದಿ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ಅನ್ಯಥಾ
			high *= 2;
	}

	ಯಾವತ್ (ಕೆಳಗಿನ < high - 1) {
		off_t mid = (ಕೆಳಗಿನ + high) / 2;

		ಯದಿ (blkdev_valid_offset (fd, mid))
			ಕೆಳಗಿನ = mid;
		ಅನ್ಯಥಾ
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ಪ್ರತ್ಯಾಗಮ (ಕೆಳಗಿನ + 1);
}

/* get size in bytes */
ಪೂರ್ಣಾಙ್ಕ blkdev_get_size(ಪೂರ್ಣಾಙ್ಕ fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ಯದಿ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#endif

#ifdef BLKGETSIZE64
	ಯದಿ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ಯದಿ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ಯದಿ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif

#ifdef FDGETPRM
	{
		ಸಂರಚನಾ floppy_struct this_floppy;

		ಯದಿ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}
#endif /* FDGETPRM */

#ಯದಿ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ಪೂರ್ಣಾಙ್ಕ part = -1;
		ಸಂರಚನಾ disklabel lab;
		ಸಂರಚನಾ partition *pp;
		ಸಂರಚನಾ stat st;

		ಯದಿ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ಯದಿ (part >= 0 && (ioctl(fd, DIOCGDINFO, (ವರ್ಣ *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ಯದಿ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ಪ್ರತ್ಯಾಗಮ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ಸಂರಚನಾ stat st;

		ಯದಿ (fstat(fd, &st) != 0)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
		ಯದಿ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ಪ್ರತ್ಯಾಗಮ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ಯದಿ (size < 0)
			ಪ್ರತ್ಯಾಗಮ -1;

		*bytes = (uint64_t) size;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* get 512-byte sector count */
ಪೂರ್ಣಾಙ್ಕ blkdev_get_sectors(ಪೂರ್ಣಾಙ್ಕ fd, uint64_t *sectors)
{
	uint64_t bytes;

	ಯದಿ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ಪೂರ್ಣಾಙ್ಕ blkdev_get_sector_size(ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ *sector_size)
{
	ಯದಿ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ -1;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ blkdev_get_sector_size(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)), ಪೂರ್ಣಾಙ್ಕ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ blkdev_get_physector_size(ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ *sector_size)
{
	ಯದಿ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ -1;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ blkdev_get_physector_size(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)), ಪೂರ್ಣಾಙ್ಕ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ಪೂರ್ಣಾಙ್ಕ blkdev_is_misaligned(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಪೂರ್ಣಾಙ್ಕ aligned;

	ಯದಿ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ಪ್ರತ್ಯಾಗಮ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ಪ್ರತ್ಯಾಗಮ aligned != 0 ? 1 : 0;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ blkdev_is_misaligned(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)))
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಪೂರ್ಣಾಙ್ಕ open_blkdev_or_file(ಧ್ರುವ ಸಂರಚನಾ stat *st, ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ oflag)
{
	ಪೂರ್ಣಾಙ್ಕ fd;

	ಯದಿ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ಅನ್ಯಥಾ
		fd = open(name, oflag);
	ಯದಿ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ಪ್ರತ್ಯಾಗಮ fd;
}

#ifdef CDROM_GET_CAPABILITY
ಪೂರ್ಣಾಙ್ಕ blkdev_is_cdrom(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಪೂರ್ಣಾಙ್ಕ ret = ioctl(fd, CDROM_GET_CAPABILITY, ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ ret < 0 ? 0 : ret;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ blkdev_is_cdrom(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)))
{
	ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ blkdev_get_geometry(ಪೂರ್ಣಾಙ್ಕ fd, unsigned ಪೂರ್ಣಾಙ್ಕ *h, unsigned ಪೂರ್ಣಾಙ್ಕ *s)
{
	ಸಂರಚನಾ hd_geometry geometry;

	ಯದಿ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ blkdev_get_geometry(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)),
		unsigned ಪೂರ್ಣಾಙ್ಕ *h, unsigned ಪೂರ್ಣಾಙ್ಕ *s)
{
	*h = 0;
	*s = 0;
#endif
	ಪ್ರತ್ಯಾಗಮ -1;
}

/*
 * Convert scsi type to human readable string.
 */
ಧ್ರುವ ವರ್ಣ *blkdev_scsi_type_to_name(ಪೂರ್ಣಾಙ್ಕ type)
{
	ವಿಕಲ್ಪ (type) {
	ಪಕ್ಷ SCSI_TYPE_DISK:
		ಪ್ರತ್ಯಾಗಮ "disk";
	ಪಕ್ಷ SCSI_TYPE_TAPE:
		ಪ್ರತ್ಯಾಗಮ "tape";
	ಪಕ್ಷ SCSI_TYPE_PRINTER:
		ಪ್ರತ್ಯಾಗಮ "printer";
	ಪಕ್ಷ SCSI_TYPE_PROCESSOR:
		ಪ್ರತ್ಯಾಗಮ "processor";
	ಪಕ್ಷ SCSI_TYPE_WORM:
		ಪ್ರತ್ಯಾಗಮ "worm";
	ಪಕ್ಷ SCSI_TYPE_ROM:
		ಪ್ರತ್ಯಾಗಮ "rom";
	ಪಕ್ಷ SCSI_TYPE_SCANNER:
		ಪ್ರತ್ಯಾಗಮ "scanner";
	ಪಕ್ಷ SCSI_TYPE_MOD:
		ಪ್ರತ್ಯಾಗಮ "mo-disk";
	ಪಕ್ಷ SCSI_TYPE_MEDIUM_CHANGER:
		ಪ್ರತ್ಯಾಗಮ "changer";
	ಪಕ್ಷ SCSI_TYPE_COMM:
		ಪ್ರತ್ಯಾಗಮ "comm";
	ಪಕ್ಷ SCSI_TYPE_RAID:
		ಪ್ರತ್ಯಾಗಮ "raid";
	ಪಕ್ಷ SCSI_TYPE_ENCLOSURE:
		ಪ್ರತ್ಯಾಗಮ "enclosure";
	ಪಕ್ಷ SCSI_TYPE_RBC:
		ಪ್ರತ್ಯಾಗಮ "rbc";
	ಪಕ್ಷ SCSI_TYPE_OSD:
		ಪ್ರತ್ಯಾಗಮ "osd";
	ಪಕ್ಷ SCSI_TYPE_NO_LUN:
		ಪ್ರತ್ಯಾಗಮ "no-lun";
	ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* return 0 on success */
ಪೂರ್ಣಾಙ್ಕ blkdev_lock(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *devname, ಧ್ರುವ ವರ್ಣ *lockmode)
{
	ಪೂರ್ಣಾಙ್ಕ oper, rc, ಸಂದೇಶ = 0;

	ಯದಿ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ಯದಿ (!lockmode)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ಅನ್ಯಥಾ ಯದಿ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ಅನ್ಯಥಾ ಯದಿ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಅನ್ಯಥಾ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಯದಿ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ಯದಿ (rc == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
		ಯದಿ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			ಸಂದೇಶ = 1;
		}
	}
	rc = flock(fd, oper);
	ಯದಿ (rc != 0) {
		ವಿಕಲ್ಪ (errno) {
		ಪಕ್ಷ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			warn(_("%s: failed to get lock"), devname);
		}
	} ಅನ್ಯಥಾ ಯದಿ (ಸಂದೇಶ)
		fprintf(stderr, _("OK\n"));
	ಪ್ರತ್ಯಾಗಮ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
ಸಂರಚನಾ blk_zone_report *blkdev_get_zonereport(ಪೂರ್ಣಾಙ್ಕ fd, uint64_t sector, uint32_t nzones)
{
	ಸಂರಚನಾ blk_zone_report *rep;
	size_t rep_size;
	ಪೂರ್ಣಾಙ್ಕ ret;

	rep_size = ಪರಿಮಾಣ(ಸಂರಚನಾ blk_zone_report) + ಪರಿಮಾಣ(ಸಂರಚನಾ blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ಯದಿ (!rep)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ಯದಿ (ret || rep->nr_zones != nzones) {
		ಮುಕ್ತ(rep);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	uint64_t bytes, sectors;
	ಪೂರ್ಣಾಙ್ಕ sector_size, phy_sector_size;
	ಪೂರ್ಣಾಙ್ಕ fd;

	ಯದಿ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ಯದಿ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ಯದಿ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ಯದಿ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ಯದಿ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ಯದಿ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
