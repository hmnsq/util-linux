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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LINUX_FD_H
#ಅನ್ತರ್ಭಾವ <linux/fd.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LINUX_BLKZONED_H
#ಅನ್ತರ್ಭಾವ <linux/blkzoned.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_DISKLABEL_H
#ಅನ್ತರ್ಭಾವ <sys/disklabel.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SYS_DISK_H
# include <sys/disk.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "blkdev.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "linux_version.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "nls.h"

ಸ್ಥಿರ ದೀರ್ಘ blkdev_valid_ಸ್ಥಾನಾಂತರ(ಪೂರ್ಣಾಂಕ fd, off_t ಸ್ಥಾನಾಂತರ)
{
	ಅಕ್ಷರ ch;

	ಆದರೆ (lseek (fd, ಸ್ಥಾನಾಂತರ, 0) < 0)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (ul_read_all (fd, &ch, 1) < 1)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ is_blkdev(ಪೂರ್ಣಾಂಕ fd)
{
	ರಚನೆ stat st;
	ಹಿಂತಿರುಗಿ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_ಕಂಡುಹಿಡಿ_ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ fd)
{
	off_t high, ಕೆಳಗಿನ = 0;

	ಗಾಗಿ (high = 1024; blkdev_valid_ಸ್ಥಾನಾಂತರ(fd, high); ) {
		ಆದರೆ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ಹಿಂತಿರುಗಿ -1;
		}

		ಕೆಳಗಿನ = high;

		ಆದರೆ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ಇಲ್ಲದಿದ್ದರೆ
			high *= 2;
	}

	ಆಗ (ಕೆಳಗಿನ < high - 1) {
		off_t mid = (ಕೆಳಗಿನ + high) / 2;

		ಆದರೆ (blkdev_valid_ಸ್ಥಾನಾಂತರ (fd, mid))
			ಕೆಳಗಿನ = mid;
		ಇಲ್ಲದಿದ್ದರೆ
			high = mid;
	}
	blkdev_valid_ಸ್ಥಾನಾಂತರ (fd, 0);
	ಹಿಂತಿರುಗಿ (ಕೆಳಗಿನ + 1);
}

/* get size in bytes */
ಪೂರ್ಣಾಂಕ blkdev_get_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fd, uint64_t *ಬೈಟುಗಳು)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ಆದರೆ (ioctl(fd, DKIOCGETBLOCKCOUNT, ಬೈಟುಗಳು) >= 0) {
		*ಬೈಟುಗಳು <<= 9;
		ಹಿಂತಿರುಗಿ 0;
	}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BLKGETSIZE64
	ಆದರೆ (ioctl(fd, BLKGETSIZE64, ಬೈಟುಗಳು) >= 0)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BLKGETSIZE
	{
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಗಾತ್ರ_2;

		ಆದರೆ (ioctl(fd, BLKGETSIZE, &ಗಾತ್ರ_2) >= 0) {
			*ಬೈಟುಗಳು = ((uint64_t) ಗಾತ್ರ_2) << 9;
			ಹಿಂತಿರುಗಿ 0;
		}
	}

#ಆದರೆ_ಅಂತ್ಯ /* BLKGETSIZE */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DIOCGMEDIASIZE
	/* FreeBSD */
	ಆದರೆ (ioctl(fd, DIOCGMEDIASIZE, ಬೈಟುಗಳು) >= 0)
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FDGETPRM
	{
		ರಚನೆ floppy_struct this_floppy;

		ಆದರೆ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*ಬೈಟುಗಳು = ((uint64_t) this_floppy.ಗಾತ್ರ_2) << 9;
			ಹಿಂತಿರುಗಿ 0;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* FDGETPRM */

#ಆದರೆ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ಪೂರ್ಣಾಂಕ part = -1;
		ರಚನೆ disklabel lab;
		ರಚನೆ ವಿಭಜನೆ *pp;
		ರಚನೆ stat st;

		ಆದರೆ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ಆದರೆ (part >= 0 && (ioctl(fd, DIOCGDINFO, (ಅಕ್ಷರ *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ಆದರೆ (pp->p_ಗಾತ್ರ) {
				 *ಬೈಟುಗಳು = ((uint64_t) pp->p_ಗಾತ್ರ) << 9;
				 ಹಿಂತಿರುಗಿ 0;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ರಚನೆ stat st;

		ಆದರೆ (fstat(fd, &st) != 0)
			ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (S_ISREG(st.st_mode)) {
			*ಬೈಟುಗಳು = (uint64_t) st.st_size;
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ಹಿಂತಿರುಗಿ -1;
		}
	}

	{
		off_t ಗಾತ್ರ_2 = blkdev_ಕಂಡುಹಿಡಿ_ಗಾತ್ರ(fd);

		ಆದರೆ (ಗಾತ್ರ_2 < 0)
			ಹಿಂತಿರುಗಿ -1;

		*ಬೈಟುಗಳು = (uint64_t) ಗಾತ್ರ_2;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/* get 512-byte sector count */
ಪೂರ್ಣಾಂಕ blkdev_get_sectors(ಪೂರ್ಣಾಂಕ fd, uint64_t *sectors)
{
	uint64_t ಬೈಟುಗಳು;

	ಆದರೆ (blkdev_get_ಗಾತ್ರ(fd, &ಬೈಟುಗಳು) == 0) {
		*sectors = (ಬೈಟುಗಳು >> 9);
		ಹಿಂತಿರುಗಿ 0;
	}

	ಹಿಂತಿರುಗಿ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BLKSSZGET
ಪೂರ್ಣಾಂಕ blkdev_get_sector_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ *sector_ಗಾತ್ರ)
{
	ಆದರೆ (ioctl(fd, BLKSSZGET, sector_ಗಾತ್ರ) >= 0)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ -1;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ blkdev_get_sector_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)), ಪೂರ್ಣಾಂಕ *sector_ಗಾತ್ರ)
{
	*sector_ಗಾತ್ರ = DEFAULT_SECTOR_ಗಾತ್ರ;
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BLKPBSZGET
ಪೂರ್ಣಾಂಕ blkdev_get_physector_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ *sector_ಗಾತ್ರ)
{
	ಆದರೆ (ioctl(fd, BLKPBSZGET, sector_ಗಾತ್ರ) >= 0)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ -1;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ blkdev_get_physector_ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)), ಪೂರ್ಣಾಂಕ *sector_ಗಾತ್ರ)
{
	*sector_ಗಾತ್ರ = DEFAULT_SECTOR_ಗಾತ್ರ;
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Return the alignment status of a device
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BLKALIGNOFF
ಪೂರ್ಣಾಂಕ blkdev_is_misaligned(ಪೂರ್ಣಾಂಕ fd)
{
	ಪೂರ್ಣಾಂಕ aligned;

	ಆದರೆ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ಹಿಂತಿರುಗಿ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ಹಿಂತಿರುಗಿ aligned != 0 ? 1 : 0;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ blkdev_is_misaligned(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)))
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ open_blkdev_or_file(ಸ್ಥಿರಾಂಕ ರಚನೆ stat *st, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ oflag)
{
	ಪೂರ್ಣಾಂಕ fd;

	ಆದರೆ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ಇಲ್ಲದಿದ್ದರೆ
		fd = open(name, oflag);
	ಆದರೆ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ಹಿಂತಿರುಗಿ fd;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CDROM_GET_CAPABILITY
ಪೂರ್ಣಾಂಕ blkdev_is_cdrom(ಪೂರ್ಣಾಂಕ fd)
{
	ಪೂರ್ಣಾಂಕ ret = ioctl(fd, CDROM_GET_CAPABILITY, ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ ret < 0 ? 0 : ret;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ blkdev_is_cdrom(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)))
{
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HDIO_GETGEO
ಪೂರ್ಣಾಂಕ blkdev_get_geometry(ಪೂರ್ಣಾಂಕ fd, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *h, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *s)
{
	ರಚನೆ hd_geometry geometry;

	ಆದರೆ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ಹಿಂತಿರುಗಿ 0;
	}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ blkdev_get_geometry(ಪೂರ್ಣಾಂಕ fd __attribute__((__unused__)),
		ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *h, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *s)
{
	*h = 0;
	*s = 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ -1;
}

/*
 * Convert scsi type to human readable string.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *blkdev_scsi_type_ಗಮ್ಯ_name(ಪೂರ್ಣಾಂಕ type)
{
	ಆಯ್ಕೆ (type) {
	ಸ್ಥಿತಿ SCSI_TYPE_DISK:
		ಹಿಂತಿರುಗಿ "disk";
	ಸ್ಥಿತಿ SCSI_TYPE_ಪಟ್ಟಿ:
		ಹಿಂತಿರುಗಿ "tape";
	ಸ್ಥಿತಿ SCSI_TYPE_PRINTER:
		ಹಿಂತಿರುಗಿ "printer";
	ಸ್ಥಿತಿ SCSI_TYPE_PROCESSOR:
		ಹಿಂತಿರುಗಿ "processor";
	ಸ್ಥಿತಿ SCSI_TYPE_WORM:
		ಹಿಂತಿರುಗಿ "worm";
	ಸ್ಥಿತಿ SCSI_TYPE_ROM:
		ಹಿಂತಿರುಗಿ "rom";
	ಸ್ಥಿತಿ SCSI_TYPE_SCANNER:
		ಹಿಂತಿರುಗಿ "scanner";
	ಸ್ಥಿತಿ SCSI_TYPE_MOD:
		ಹಿಂತಿರುಗಿ "mo-disk";
	ಸ್ಥಿತಿ SCSI_TYPE_MEDIUM_CHANGER:
		ಹಿಂತಿರುಗಿ "changer";
	ಸ್ಥಿತಿ SCSI_TYPE_COMM:
		ಹಿಂತಿರುಗಿ "comm";
	ಸ್ಥಿತಿ SCSI_TYPE_RAID:
		ಹಿಂತಿರುಗಿ "raid";
	ಸ್ಥಿತಿ SCSI_TYPE_ENCLOSURE:
		ಹಿಂತಿರುಗಿ "enclosure";
	ಸ್ಥಿತಿ SCSI_TYPE_RBC:
		ಹಿಂತಿರುಗಿ "rbc";
	ಸ್ಥಿತಿ SCSI_TYPE_OSD:
		ಹಿಂತಿರುಗಿ "osd";
	ಸ್ಥಿತಿ SCSI_TYPE_NO_LUN:
		ಹಿಂತಿರುಗಿ "no-lun";
	ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* return 0 on success */
ಪೂರ್ಣಾಂಕ blkdev_ಬೀಗ(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lockmode)
{
	ಪೂರ್ಣಾಂಕ oper, rc, ಸಂದೇಶ = 0;

	ಆದರೆ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ಆದರೆ (!lockmode)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ಹಿಂತಿರುಗಿ 0;
	ಇಲ್ಲದಿದ್ದರೆ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಆದರೆ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ಆದರೆ (rc == 0)
			ಹಿಂತಿರುಗಿ 0;
		ಆದರೆ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					ಕಾರ್ಯಕ್ರಮ_invocation_short_name, devname);
			ಸಂದೇಶ = 1;
		}
	}
	rc = flock(fd, oper);
	ಆದರೆ (rc != 0) {
		ಆಯ್ಕೆ (errno) {
		ಸ್ಥಿತಿ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			warn(_("%s: failed to get lock"), devname);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಸಂದೇಶ)
		fprintf(stderr, _("OK\n"));
	ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LINUX_BLKZONED_H
ರಚನೆ blk_zone_report *blkdev_get_zonereport(ಪೂರ್ಣಾಂಕ fd, uint64_t sector, uint32_t nzones)
{
	ರಚನೆ blk_zone_report *rep;
	size_t rep_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ ret;

	rep_ಗಾತ್ರ = ಗಾತ್ರ(ರಚನೆ blk_zone_report) + ಗಾತ್ರ(ರಚನೆ blk_zone) * nzones;
	rep = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, rep_ಗಾತ್ರ);
	ಆದರೆ (!rep)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ಆದರೆ (ret || rep->nr_zones != nzones) {
		ಬಿಡುಗಡೆ(rep);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ rep;
}
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_BLKDEV
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	uint64_t ಬೈಟುಗಳು, sectors;
	ಪೂರ್ಣಾಂಕ sector_ಗಾತ್ರ, phy_sector_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ fd;

	ಆದರೆ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	ಆದರೆ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ಆದರೆ (blkdev_get_ಗಾತ್ರ(fd, &ಬೈಟುಗಳು) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ಆದರೆ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ಆದರೆ (blkdev_get_sector_ಗಾತ್ರ(fd, &sector_ಗಾತ್ರ) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ಆದರೆ (blkdev_get_physector_ಗಾತ್ರ(fd, &phy_sector_ಗಾತ್ರ) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	ಬರೆ("          bytes: %" PRIu64 "\n", ಬೈಟುಗಳು);
	ಬರೆ("        sectors: %" PRIu64 "\n", sectors);
	ಬರೆ("    sector size: %d\n", sector_ಗಾತ್ರ);
	ಬರೆ("phy-sector size: %d\n", phy_sector_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_BLKDEV */
