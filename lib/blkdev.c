/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LINUX_FD_H
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/fd.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LINUX_BLKZONED_H
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/blkzoned.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_DISKLABEL_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/disklabel.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_DISK_H
# include <sys/disk.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "blkdev.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "linux_version.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"

ਸਥਿਰ ਲੰਮਾ blkdev_valid_ਵਿੱਥ(ਪੂਰਨ fd, off_t ਵਿੱਥ)
{
	ਅੱਖਰ ch;

	ਜੇ (lseek (fd, ਵਿੱਥ, 0) < 0)
		ਵਾਪਸ 0;
	ਜੇ (ul_read_all (fd, &ch, 1) < 1)
		ਵਾਪਸ 0;
	ਵਾਪਸ 1;
}

ਪੂਰਨ is_blkdev(ਪੂਰਨ fd)
{
	ਬਣਤਰ stat st;
	ਵਾਪਸ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_ਲੱਭੋ_ਮਾਪ (ਪੂਰਨ fd)
{
	off_t high, ਨੀਵਾਂ = 0;

	ਲਈ (high = 1024; blkdev_valid_ਵਿੱਥ(fd, high); ) {
		ਜੇ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ਵਾਪਸ -1;
		}

		ਨੀਵਾਂ = high;

		ਜੇ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ਨਹੀਂਤਾਂ
			high *= 2;
	}

	ਜਦੋਂ (ਨੀਵਾਂ < high - 1) {
		off_t mid = (ਨੀਵਾਂ + high) / 2;

		ਜੇ (blkdev_valid_ਵਿੱਥ (fd, mid))
			ਨੀਵਾਂ = mid;
		ਨਹੀਂਤਾਂ
			high = mid;
	}
	blkdev_valid_ਵਿੱਥ (fd, 0);
	ਵਾਪਸ (ਨੀਵਾਂ + 1);
}

/* get size in bytes */
ਪੂਰਨ blkdev_get_ਮਾਪ(ਪੂਰਨ fd, uint64_t *ਬਾਈਟਾਂ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ਜੇ (ioctl(fd, DKIOCGETBLOCKCOUNT, ਬਾਈਟਾਂ) >= 0) {
		*ਬਾਈਟਾਂ <<= 9;
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BLKGETSIZE64
	ਜੇ (ioctl(fd, BLKGETSIZE64, ਬਾਈਟਾਂ) >= 0)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BLKGETSIZE
	{
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ;

		ਜੇ (ioctl(fd, BLKGETSIZE, &ਮਾਪ) >= 0) {
			*ਬਾਈਟਾਂ = ((uint64_t) ਮਾਪ) << 9;
			ਵਾਪਸ 0;
		}
	}

#ਜੇ_ਅੰਤ /* BLKGETSIZE */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DIOCGMEDIASIZE
	/* FreeBSD */
	ਜੇ (ioctl(fd, DIOCGMEDIASIZE, ਬਾਈਟਾਂ) >= 0)
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FDGETPRM
	{
		ਬਣਤਰ floppy_struct this_floppy;

		ਜੇ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*ਬਾਈਟਾਂ = ((uint64_t) this_floppy.ਮਾਪ) << 9;
			ਵਾਪਸ 0;
		}
	}
#ਜੇ_ਅੰਤ /* FDGETPRM */

#ਜੇ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ਪੂਰਨ part = -1;
		ਬਣਤਰ disklabel lab;
		ਬਣਤਰ ਹਿੱਸੇ_ਕਰੋ *pp;
		ਬਣਤਰ stat st;

		ਜੇ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ਜੇ (part >= 0 && (ioctl(fd, DIOCGDINFO, (ਅੱਖਰ *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ਜੇ (pp->p_ਮਾਪ) {
				 *ਬਾਈਟਾਂ = ((uint64_t) pp->p_ਮਾਪ) << 9;
				 ਵਾਪਸ 0;
			}
		}
	}
#ਜੇ_ਅੰਤ /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ਬਣਤਰ stat st;

		ਜੇ (fstat(fd, &st) != 0)
			ਵਾਪਸ -1;
		ਜੇ (S_ISREG(st.st_mode)) {
			*ਬਾਈਟਾਂ = (uint64_t) st.st_size;
			ਵਾਪਸ 0;
		}
		ਜੇ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ਵਾਪਸ -1;
		}
	}

	{
		off_t ਮਾਪ = blkdev_ਲੱਭੋ_ਮਾਪ(fd);

		ਜੇ (ਮਾਪ < 0)
			ਵਾਪਸ -1;

		*ਬਾਈਟਾਂ = (uint64_t) ਮਾਪ;
	}

	ਵਾਪਸ 0;
}

/* get 512-byte sector count */
ਪੂਰਨ blkdev_get_sectors(ਪੂਰਨ fd, uint64_t *sectors)
{
	uint64_t ਬਾਈਟਾਂ;

	ਜੇ (blkdev_get_ਮਾਪ(fd, &ਬਾਈਟਾਂ) == 0) {
		*sectors = (ਬਾਈਟਾਂ >> 9);
		ਵਾਪਸ 0;
	}

	ਵਾਪਸ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BLKSSZGET
ਪੂਰਨ blkdev_get_sector_ਮਾਪ(ਪੂਰਨ fd, ਪੂਰਨ *sector_ਮਾਪ)
{
	ਜੇ (ioctl(fd, BLKSSZGET, sector_ਮਾਪ) >= 0)
		ਵਾਪਸ 0;
	ਵਾਪਸ -1;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ blkdev_get_sector_ਮਾਪ(ਪੂਰਨ fd __attribute__((__unused__)), ਪੂਰਨ *sector_ਮਾਪ)
{
	*sector_ਮਾਪ = DEFAULT_SECTOR_ਮਾਪ;
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BLKPBSZGET
ਪੂਰਨ blkdev_get_physector_ਮਾਪ(ਪੂਰਨ fd, ਪੂਰਨ *sector_ਮਾਪ)
{
	ਜੇ (ioctl(fd, BLKPBSZGET, sector_ਮਾਪ) >= 0)
		ਵਾਪਸ 0;
	ਵਾਪਸ -1;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ blkdev_get_physector_ਮਾਪ(ਪੂਰਨ fd __attribute__((__unused__)), ਪੂਰਨ *sector_ਮਾਪ)
{
	*sector_ਮਾਪ = DEFAULT_SECTOR_ਮਾਪ;
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/*
 * Return the alignment status of a device
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BLKALIGNOFF
ਪੂਰਨ blkdev_is_misaligned(ਪੂਰਨ fd)
{
	ਪੂਰਨ aligned;

	ਜੇ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ਵਾਪਸ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ਵਾਪਸ aligned != 0 ? 1 : 0;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ blkdev_is_misaligned(ਪੂਰਨ fd __attribute__((__unused__)))
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ open_blkdev_or_file(ਅਚਲ ਬਣਤਰ stat *st, ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਪੂਰਨ oflag)
{
	ਪੂਰਨ fd;

	ਜੇ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ਨਹੀਂਤਾਂ
		fd = open(name, oflag);
	ਜੇ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ਵਾਪਸ -1;
	}
	ਜੇ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ਵਾਪਸ fd;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CDROM_GET_CAPABILITY
ਪੂਰਨ blkdev_is_cdrom(ਪੂਰਨ fd)
{
	ਪੂਰਨ ret = ioctl(fd, CDROM_GET_CAPABILITY, ਨਿਰਾ);
	ਵਾਪਸ ret < 0 ? 0 : ret;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ blkdev_is_cdrom(ਪੂਰਨ fd __attribute__((__unused__)))
{
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HDIO_GETGEO
ਪੂਰਨ blkdev_get_geometry(ਪੂਰਨ fd, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *h, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *s)
{
	ਬਣਤਰ hd_geometry geometry;

	ਜੇ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ਵਾਪਸ 0;
	}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ blkdev_get_geometry(ਪੂਰਨ fd __attribute__((__unused__)),
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *h, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *s)
{
	*h = 0;
	*s = 0;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ -1;
}

/*
 * Convert scsi type to human readable string.
 */
ਅਚਲ ਅੱਖਰ *blkdev_scsi_type_ਮੰਜ਼ਿਲ_name(ਪੂਰਨ type)
{
	ਚੋਣ (type) {
	ਹਾਲਤ SCSI_TYPE_DISK:
		ਵਾਪਸ "disk";
	ਹਾਲਤ SCSI_TYPE_ਫੀਤਾ:
		ਵਾਪਸ "tape";
	ਹਾਲਤ SCSI_TYPE_PRINTER:
		ਵਾਪਸ "printer";
	ਹਾਲਤ SCSI_TYPE_PROCESSOR:
		ਵਾਪਸ "processor";
	ਹਾਲਤ SCSI_TYPE_WORM:
		ਵਾਪਸ "worm";
	ਹਾਲਤ SCSI_TYPE_ROM:
		ਵਾਪਸ "rom";
	ਹਾਲਤ SCSI_TYPE_SCANNER:
		ਵਾਪਸ "scanner";
	ਹਾਲਤ SCSI_TYPE_MOD:
		ਵਾਪਸ "mo-disk";
	ਹਾਲਤ SCSI_TYPE_MEDIUM_CHANGER:
		ਵਾਪਸ "changer";
	ਹਾਲਤ SCSI_TYPE_COMM:
		ਵਾਪਸ "comm";
	ਹਾਲਤ SCSI_TYPE_RAID:
		ਵਾਪਸ "raid";
	ਹਾਲਤ SCSI_TYPE_ENCLOSURE:
		ਵਾਪਸ "enclosure";
	ਹਾਲਤ SCSI_TYPE_RBC:
		ਵਾਪਸ "rbc";
	ਹਾਲਤ SCSI_TYPE_OSD:
		ਵਾਪਸ "osd";
	ਹਾਲਤ SCSI_TYPE_NO_LUN:
		ਵਾਪਸ "no-lun";
	ਮੂਲ:
		ਰੋਕੋ;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

/* return 0 on success */
ਪੂਰਨ blkdev_ਤਾਲਾ(ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *devname, ਅਚਲ ਅੱਖਰ *lockmode)
{
	ਪੂਰਨ oper, rc, ਸੁਨੇਹਾ = 0;

	ਜੇ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ਜੇ (!lockmode)
		ਵਾਪਸ 0;

	ਜੇ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ਨਹੀਂਤਾਂ ਜੇ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ਨਹੀਂਤਾਂ ਜੇ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ਵਾਪਸ 0;
	ਨਹੀਂਤਾਂ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ਵਾਪਸ -EINVAL;
	}

	ਜੇ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ਜੇ (rc == 0)
			ਵਾਪਸ 0;
		ਜੇ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					ਪ੍ਰੋਗਰਾਮ_invocation_short_name, devname);
			ਸੁਨੇਹਾ = 1;
		}
	}
	rc = flock(fd, oper);
	ਜੇ (rc != 0) {
		ਚੋਣ (errno) {
		ਹਾਲਤ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ਰੋਕੋ;
		ਮੂਲ:
			warn(_("%s: failed to get lock"), devname);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (ਸੁਨੇਹਾ)
		fprintf(stderr, _("OK\n"));
	ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LINUX_BLKZONED_H
ਬਣਤਰ blk_zone_report *blkdev_get_zonereport(ਪੂਰਨ fd, uint64_t sector, uint32_t nzones)
{
	ਬਣਤਰ blk_zone_report *rep;
	size_t rep_ਮਾਪ;
	ਪੂਰਨ ret;

	rep_ਮਾਪ = ਆਕਾਰ(ਬਣਤਰ blk_zone_report) + ਆਕਾਰ(ਬਣਤਰ blk_zone) * nzones;
	rep = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, rep_ਮਾਪ);
	ਜੇ (!rep)
		ਵਾਪਸ ਨਿਰਾ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ਜੇ (ret || rep->nr_zones != nzones) {
		ਮੁਕਤ(rep);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ rep;
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_BLKDEV
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	uint64_t ਬਾਈਟਾਂ, sectors;
	ਪੂਰਨ sector_ਮਾਪ, phy_sector_ਮਾਪ;
	ਪੂਰਨ fd;

	ਜੇ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	ਜੇ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ਜੇ (blkdev_get_ਮਾਪ(fd, &ਬਾਈਟਾਂ) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ਜੇ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ਜੇ (blkdev_get_sector_ਮਾਪ(fd, &sector_ਮਾਪ) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ਜੇ (blkdev_get_physector_ਮਾਪ(fd, &phy_sector_ਮਾਪ) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	ਲਿਖੋ("          bytes: %" PRIu64 "\n", ਬਾਈਟਾਂ);
	ਲਿਖੋ("        sectors: %" PRIu64 "\n", sectors);
	ਲਿਖੋ("    sector size: %d\n", sector_ਮਾਪ);
	ਲਿਖੋ("phy-sector size: %d\n", phy_sector_ਮਾਪ);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_BLKDEV */
