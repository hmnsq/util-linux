/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#ifdef HAVE_LINUX_FD_H
#include <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#include <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#include <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#include "all-io.h"
#include "blkdev.h"
#include "c.h"
#include "cctype.h"
#include "linux_version.h"
#include "fileutils.h"
#include "nls.h"

static long blkdev_valid_offset(ціле fd, off_t offset)
{
	char ch;

	якщо (lseek (fd, offset, 0) < 0)
		повернути 0;
	якщо (ul_read_all (fd, &ch, 1) < 1)
		повернути 0;
	повернути 1;
}

ціле is_blkdev(ціле fd)
{
	struct stat st;
	повернути (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ціле fd)
{
	off_t high, low = 0;

	для (high = 1024; blkdev_valid_offset(fd, high); ) {
		якщо (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			повернути -1;
		}

		low = high;

		якщо (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		інакше
			high *= 2;
	}

	поки (low < high - 1) {
		off_t mid = (low + high) / 2;

		якщо (blkdev_valid_offset (fd, mid))
			low = mid;
		інакше
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	повернути (low + 1);
}

/* get size in bytes */
ціле blkdev_get_size(ціле fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	якщо (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		повернути 0;
	}
#endif

#ifdef BLKGETSIZE64
	якщо (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		повернути 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		якщо (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			повернути 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	якщо (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		повернути 0;
#endif

#ifdef FDGETPRM
	{
		struct floppy_struct this_floppy;

		якщо (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			повернути 0;
		}
	}
#endif /* FDGETPRM */

#якщо defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ціле part = -1;
		struct disklabel lab;
		struct partition *pp;
		struct stat st;

		якщо ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		якщо (part >= 0 && (ioctl(fd, DIOCGDINFO, (char *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			якщо (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 повернути 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		struct stat st;

		якщо (fstat(fd, &st) != 0)
			повернути -1;
		якщо (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			повернути 0;
		}
		якщо (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			повернути -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		якщо (size < 0)
			повернути -1;

		*bytes = (uint64_t) size;
	}

	повернути 0;
}

/* get 512-byte sector count */
ціле blkdev_get_sectors(ціле fd, uint64_t *sectors)
{
	uint64_t bytes;

	якщо (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		повернути 0;
	}

	повернути -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ціле blkdev_get_sector_size(ціле fd, ціле *sector_size)
{
	якщо (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		повернути 0;
	повернути -1;
}
#інакше
ціле blkdev_get_sector_size(ціле fd __attribute__((__unused__)), ціле *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	повернути 0;
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
ціле blkdev_get_physector_size(ціле fd, ціле *sector_size)
{
	якщо (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		повернути 0;
	повернути -1;
}
#інакше
ціле blkdev_get_physector_size(ціле fd __attribute__((__unused__)), ціле *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	повернути 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ціле blkdev_is_misaligned(ціле fd)
{
	ціле aligned;

	якщо (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		повернути 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	повернути aligned != 0 ? 1 : 0;
}
#інакше
ціле blkdev_is_misaligned(ціле fd __attribute__((__unused__)))
{
	повернути 0;
}
#endif

ціле open_blkdev_or_file(const struct stat *st, const char *name, const ціле oflag)
{
	ціле fd;

	якщо (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} інакше
		fd = open(name, oflag);
	якщо (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		повернути -1;
	}
	якщо (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	повернути fd;
}

#ifdef CDROM_GET_CAPABILITY
ціле blkdev_is_cdrom(ціле fd)
{
	ціле ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	повернути ret < 0 ? 0 : ret;
}
#інакше
ціле blkdev_is_cdrom(ціле fd __attribute__((__unused__)))
{
	повернути 0;
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
ціле blkdev_get_geometry(ціле fd, unsigned ціле *h, unsigned ціле *s)
{
	struct hd_geometry geometry;

	якщо (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		повернути 0;
	}
#інакше
ціле blkdev_get_geometry(ціле fd __attribute__((__unused__)),
		unsigned ціле *h, unsigned ціле *s)
{
	*h = 0;
	*s = 0;
#endif
	повернути -1;
}

/*
 * Convert scsi type to human readable string.
 */
const char *blkdev_scsi_type_to_name(ціле type)
{
	switch (type) {
	case SCSI_TYPE_DISK:
		повернути "disk";
	case SCSI_TYPE_TAPE:
		повернути "tape";
	case SCSI_TYPE_PRINTER:
		повернути "printer";
	case SCSI_TYPE_PROCESSOR:
		повернути "processor";
	case SCSI_TYPE_WORM:
		повернути "worm";
	case SCSI_TYPE_ROM:
		повернути "rom";
	case SCSI_TYPE_SCANNER:
		повернути "scanner";
	case SCSI_TYPE_MOD:
		повернути "mo-disk";
	case SCSI_TYPE_MEDIUM_CHANGER:
		повернути "changer";
	case SCSI_TYPE_COMM:
		повернути "comm";
	case SCSI_TYPE_RAID:
		повернути "raid";
	case SCSI_TYPE_ENCLOSURE:
		повернути "enclosure";
	case SCSI_TYPE_RBC:
		повернути "rbc";
	case SCSI_TYPE_OSD:
		повернути "osd";
	case SCSI_TYPE_NO_LUN:
		повернути "no-lun";
	default:
		break;
	}
	повернути NULL;
}

/* return 0 on success */
ціле blkdev_lock(ціле fd, const char *devname, const char *lockmode)
{
	ціле oper, rc, msg = 0;

	якщо (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	якщо (!lockmode)
		повернути 0;

	якщо (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	інакше якщо (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	інакше якщо (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		повернути 0;
	інакше {
		warnx(_("unsupported lock mode: %s"), lockmode);
		повернути -EINVAL;
	}

	якщо (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		якщо (rc == 0)
			повернути 0;
		якщо (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	якщо (rc != 0) {
		switch (errno) {
		case EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			break;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} інакше якщо (msg)
		fprintf(stderr, _("OK\n"));
	повернути rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
struct blk_zone_report *blkdev_get_zonereport(ціле fd, uint64_t sector, uint32_t nzones)
{
	struct blk_zone_report *rep;
	size_t rep_size;
	ціле ret;

	rep_size = sizeof(struct blk_zone_report) + sizeof(struct blk_zone) * nzones;
	rep = calloc(1, rep_size);
	якщо (!rep)
		повернути NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	якщо (ret || rep->nr_zones != nzones) {
		free(rep);
		повернути NULL;
	}

	повернути rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

ціле main(ціле argc, char **argv)
{
	uint64_t bytes, sectors;
	ціле sector_size, phy_sector_size;
	ціле fd;

	якщо (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	якщо ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	якщо (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	якщо (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	якщо (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	якщо (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	повернути EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
