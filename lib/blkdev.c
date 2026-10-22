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

static long blkdev_valid_offset(целое fd, off_t offset)
{
	char ch;

	если (lseek (fd, offset, 0) < 0)
		вернуть 0;
	если (ul_read_all (fd, &ch, 1) < 1)
		вернуть 0;
	вернуть 1;
}

целое is_blkdev(целое fd)
{
	struct stat st;
	вернуть (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (целое fd)
{
	off_t high, low = 0;

	для (high = 1024; blkdev_valid_offset(fd, high); ) {
		если (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			вернуть -1;
		}

		low = high;

		если (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		иначе
			high *= 2;
	}

	пока (low < high - 1) {
		off_t mid = (low + high) / 2;

		если (blkdev_valid_offset (fd, mid))
			low = mid;
		иначе
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	вернуть (low + 1);
}

/* get size in bytes */
целое blkdev_get_size(целое fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	если (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		вернуть 0;
	}
#endif

#ifdef BLKGETSIZE64
	если (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		вернуть 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		если (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			вернуть 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	если (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		вернуть 0;
#endif

#ifdef FDGETPRM
	{
		struct floppy_struct this_floppy;

		если (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			вернуть 0;
		}
	}
#endif /* FDGETPRM */

#если defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		целое part = -1;
		struct disklabel lab;
		struct partition *pp;
		struct stat st;

		если ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		если (part >= 0 && (ioctl(fd, DIOCGDINFO, (char *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			если (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 вернуть 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		struct stat st;

		если (fstat(fd, &st) != 0)
			вернуть -1;
		если (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			вернуть 0;
		}
		если (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			вернуть -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		если (size < 0)
			вернуть -1;

		*bytes = (uint64_t) size;
	}

	вернуть 0;
}

/* get 512-byte sector count */
целое blkdev_get_sectors(целое fd, uint64_t *sectors)
{
	uint64_t bytes;

	если (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		вернуть 0;
	}

	вернуть -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
целое blkdev_get_sector_size(целое fd, целое *sector_size)
{
	если (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		вернуть 0;
	вернуть -1;
}
#иначе
целое blkdev_get_sector_size(целое fd __attribute__((__unused__)), целое *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	вернуть 0;
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
целое blkdev_get_physector_size(целое fd, целое *sector_size)
{
	если (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		вернуть 0;
	вернуть -1;
}
#иначе
целое blkdev_get_physector_size(целое fd __attribute__((__unused__)), целое *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	вернуть 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
целое blkdev_is_misaligned(целое fd)
{
	целое aligned;

	если (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		вернуть 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	вернуть aligned != 0 ? 1 : 0;
}
#иначе
целое blkdev_is_misaligned(целое fd __attribute__((__unused__)))
{
	вернуть 0;
}
#endif

целое open_blkdev_or_file(const struct stat *st, const char *name, const целое oflag)
{
	целое fd;

	если (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} иначе
		fd = open(name, oflag);
	если (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		вернуть -1;
	}
	если (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	вернуть fd;
}

#ifdef CDROM_GET_CAPABILITY
целое blkdev_is_cdrom(целое fd)
{
	целое ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	вернуть ret < 0 ? 0 : ret;
}
#иначе
целое blkdev_is_cdrom(целое fd __attribute__((__unused__)))
{
	вернуть 0;
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
целое blkdev_get_geometry(целое fd, unsigned целое *h, unsigned целое *s)
{
	struct hd_geometry geometry;

	если (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		вернуть 0;
	}
#иначе
целое blkdev_get_geometry(целое fd __attribute__((__unused__)),
		unsigned целое *h, unsigned целое *s)
{
	*h = 0;
	*s = 0;
#endif
	вернуть -1;
}

/*
 * Convert scsi type to human readable string.
 */
const char *blkdev_scsi_type_to_name(целое type)
{
	switch (type) {
	case SCSI_TYPE_DISK:
		вернуть "disk";
	case SCSI_TYPE_TAPE:
		вернуть "tape";
	case SCSI_TYPE_PRINTER:
		вернуть "printer";
	case SCSI_TYPE_PROCESSOR:
		вернуть "processor";
	case SCSI_TYPE_WORM:
		вернуть "worm";
	case SCSI_TYPE_ROM:
		вернуть "rom";
	case SCSI_TYPE_SCANNER:
		вернуть "scanner";
	case SCSI_TYPE_MOD:
		вернуть "mo-disk";
	case SCSI_TYPE_MEDIUM_CHANGER:
		вернуть "changer";
	case SCSI_TYPE_COMM:
		вернуть "comm";
	case SCSI_TYPE_RAID:
		вернуть "raid";
	case SCSI_TYPE_ENCLOSURE:
		вернуть "enclosure";
	case SCSI_TYPE_RBC:
		вернуть "rbc";
	case SCSI_TYPE_OSD:
		вернуть "osd";
	case SCSI_TYPE_NO_LUN:
		вернуть "no-lun";
	default:
		break;
	}
	вернуть NULL;
}

/* return 0 on success */
целое blkdev_lock(целое fd, const char *devname, const char *lockmode)
{
	целое oper, rc, msg = 0;

	если (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	если (!lockmode)
		вернуть 0;

	если (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	иначе если (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	иначе если (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		вернуть 0;
	иначе {
		warnx(_("unsupported lock mode: %s"), lockmode);
		вернуть -EINVAL;
	}

	если (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		если (rc == 0)
			вернуть 0;
		если (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	если (rc != 0) {
		switch (errno) {
		case EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			break;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} иначе если (msg)
		fprintf(stderr, _("OK\n"));
	вернуть rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
struct blk_zone_report *blkdev_get_zonereport(целое fd, uint64_t sector, uint32_t nzones)
{
	struct blk_zone_report *rep;
	size_t rep_size;
	целое ret;

	rep_size = sizeof(struct blk_zone_report) + sizeof(struct blk_zone) * nzones;
	rep = calloc(1, rep_size);
	если (!rep)
		вернуть NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	если (ret || rep->nr_zones != nzones) {
		free(rep);
		вернуть NULL;
	}

	вернуть rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

целое основной(целое argc, char **argv)
{
	uint64_t bytes, sectors;
	целое sector_size, phy_sector_size;
	целое fd;

	если (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	если ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	если (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	если (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	если (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	если (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	вернуть EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
