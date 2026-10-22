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

static long blkdev_valid_offset(số_nguyên fd, off_t offset)
{
	char ch;

	nếu (lseek (fd, offset, 0) < 0)
		trả_về 0;
	nếu (ul_read_all (fd, &ch, 1) < 1)
		trả_về 0;
	trả_về 1;
}

số_nguyên is_blkdev(số_nguyên fd)
{
	struct stat st;
	trả_về (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (số_nguyên fd)
{
	off_t high, low = 0;

	cho (high = 1024; blkdev_valid_offset(fd, high); ) {
		nếu (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			trả_về -1;
		}

		low = high;

		nếu (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		không_thì
			high *= 2;
	}

	trong_khi (low < high - 1) {
		off_t mid = (low + high) / 2;

		nếu (blkdev_valid_offset (fd, mid))
			low = mid;
		không_thì
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	trả_về (low + 1);
}

/* get size in bytes */
số_nguyên blkdev_get_size(số_nguyên fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	nếu (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		trả_về 0;
	}
#endif

#ifdef BLKGETSIZE64
	nếu (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		trả_về 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		nếu (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			trả_về 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	nếu (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		trả_về 0;
#endif

#ifdef FDGETPRM
	{
		struct floppy_struct this_floppy;

		nếu (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			trả_về 0;
		}
	}
#endif /* FDGETPRM */

#nếu defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		số_nguyên part = -1;
		struct disklabel lab;
		struct partition *pp;
		struct stat st;

		nếu ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		nếu (part >= 0 && (ioctl(fd, DIOCGDINFO, (char *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			nếu (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 trả_về 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		struct stat st;

		nếu (fstat(fd, &st) != 0)
			trả_về -1;
		nếu (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			trả_về 0;
		}
		nếu (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			trả_về -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		nếu (size < 0)
			trả_về -1;

		*bytes = (uint64_t) size;
	}

	trả_về 0;
}

/* get 512-byte sector count */
số_nguyên blkdev_get_sectors(số_nguyên fd, uint64_t *sectors)
{
	uint64_t bytes;

	nếu (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		trả_về 0;
	}

	trả_về -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
số_nguyên blkdev_get_sector_size(số_nguyên fd, số_nguyên *sector_size)
{
	nếu (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		trả_về 0;
	trả_về -1;
}
#không_thì
số_nguyên blkdev_get_sector_size(số_nguyên fd __attribute__((__unused__)), số_nguyên *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	trả_về 0;
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
số_nguyên blkdev_get_physector_size(số_nguyên fd, số_nguyên *sector_size)
{
	nếu (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		trả_về 0;
	trả_về -1;
}
#không_thì
số_nguyên blkdev_get_physector_size(số_nguyên fd __attribute__((__unused__)), số_nguyên *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	trả_về 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
số_nguyên blkdev_is_misaligned(số_nguyên fd)
{
	số_nguyên aligned;

	nếu (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		trả_về 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	trả_về aligned != 0 ? 1 : 0;
}
#không_thì
số_nguyên blkdev_is_misaligned(số_nguyên fd __attribute__((__unused__)))
{
	trả_về 0;
}
#endif

số_nguyên open_blkdev_or_file(const struct stat *st, const char *name, const số_nguyên oflag)
{
	số_nguyên fd;

	nếu (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} không_thì
		fd = open(name, oflag);
	nếu (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		trả_về -1;
	}
	nếu (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	trả_về fd;
}

#ifdef CDROM_GET_CAPABILITY
số_nguyên blkdev_is_cdrom(số_nguyên fd)
{
	số_nguyên ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	trả_về ret < 0 ? 0 : ret;
}
#không_thì
số_nguyên blkdev_is_cdrom(số_nguyên fd __attribute__((__unused__)))
{
	trả_về 0;
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
số_nguyên blkdev_get_geometry(số_nguyên fd, unsigned số_nguyên *h, unsigned số_nguyên *s)
{
	struct hd_geometry geometry;

	nếu (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		trả_về 0;
	}
#không_thì
số_nguyên blkdev_get_geometry(số_nguyên fd __attribute__((__unused__)),
		unsigned số_nguyên *h, unsigned số_nguyên *s)
{
	*h = 0;
	*s = 0;
#endif
	trả_về -1;
}

/*
 * Convert scsi type to human readable string.
 */
const char *blkdev_scsi_type_to_name(số_nguyên type)
{
	switch (type) {
	case SCSI_TYPE_DISK:
		trả_về "disk";
	case SCSI_TYPE_TAPE:
		trả_về "tape";
	case SCSI_TYPE_PRINTER:
		trả_về "printer";
	case SCSI_TYPE_PROCESSOR:
		trả_về "processor";
	case SCSI_TYPE_WORM:
		trả_về "worm";
	case SCSI_TYPE_ROM:
		trả_về "rom";
	case SCSI_TYPE_SCANNER:
		trả_về "scanner";
	case SCSI_TYPE_MOD:
		trả_về "mo-disk";
	case SCSI_TYPE_MEDIUM_CHANGER:
		trả_về "changer";
	case SCSI_TYPE_COMM:
		trả_về "comm";
	case SCSI_TYPE_RAID:
		trả_về "raid";
	case SCSI_TYPE_ENCLOSURE:
		trả_về "enclosure";
	case SCSI_TYPE_RBC:
		trả_về "rbc";
	case SCSI_TYPE_OSD:
		trả_về "osd";
	case SCSI_TYPE_NO_LUN:
		trả_về "no-lun";
	default:
		break;
	}
	trả_về NULL;
}

/* return 0 on success */
số_nguyên blkdev_lock(số_nguyên fd, const char *devname, const char *lockmode)
{
	số_nguyên oper, rc, msg = 0;

	nếu (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	nếu (!lockmode)
		trả_về 0;

	nếu (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	không_thì nếu (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	không_thì nếu (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		trả_về 0;
	không_thì {
		warnx(_("unsupported lock mode: %s"), lockmode);
		trả_về -EINVAL;
	}

	nếu (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		nếu (rc == 0)
			trả_về 0;
		nếu (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	nếu (rc != 0) {
		switch (errno) {
		case EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			break;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} không_thì nếu (msg)
		fprintf(stderr, _("OK\n"));
	trả_về rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
struct blk_zone_report *blkdev_get_zonereport(số_nguyên fd, uint64_t sector, uint32_t nzones)
{
	struct blk_zone_report *rep;
	size_t rep_size;
	số_nguyên ret;

	rep_size = sizeof(struct blk_zone_report) + sizeof(struct blk_zone) * nzones;
	rep = calloc(1, rep_size);
	nếu (!rep)
		trả_về NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	nếu (ret || rep->nr_zones != nzones) {
		free(rep);
		trả_về NULL;
	}

	trả_về rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

số_nguyên main(số_nguyên argc, char **argv)
{
	uint64_t bytes, sectors;
	số_nguyên sector_size, phy_sector_size;
	số_nguyên fd;

	nếu (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nếu ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	nếu (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	nếu (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	nếu (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	nếu (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
