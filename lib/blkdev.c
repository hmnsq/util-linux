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

static long blkdev_valid_offset(정수 fd, off_t offset)
{
	char ch;

	만약 (lseek (fd, offset, 0) < 0)
		반환 0;
	만약 (ul_read_all (fd, &ch, 1) < 1)
		반환 0;
	반환 1;
}

정수 is_blkdev(정수 fd)
{
	struct stat st;
	반환 (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (정수 fd)
{
	off_t high, low = 0;

	위해 (high = 1024; blkdev_valid_offset(fd, high); ) {
		만약 (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			반환 -1;
		}

		low = high;

		만약 (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		아니면
			high *= 2;
	}

	동안 (low < high - 1) {
		off_t mid = (low + high) / 2;

		만약 (blkdev_valid_offset (fd, mid))
			low = mid;
		아니면
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	반환 (low + 1);
}

/* get size in bytes */
정수 blkdev_get_size(정수 fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	만약 (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		반환 0;
	}
#endif

#ifdef BLKGETSIZE64
	만약 (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		반환 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		만약 (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			반환 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	만약 (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		반환 0;
#endif

#ifdef FDGETPRM
	{
		struct floppy_struct this_floppy;

		만약 (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			반환 0;
		}
	}
#endif /* FDGETPRM */

#만약 defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		정수 part = -1;
		struct disklabel lab;
		struct partition *pp;
		struct stat st;

		만약 ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		만약 (part >= 0 && (ioctl(fd, DIOCGDINFO, (char *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			만약 (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 반환 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		struct stat st;

		만약 (fstat(fd, &st) != 0)
			반환 -1;
		만약 (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			반환 0;
		}
		만약 (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			반환 -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		만약 (size < 0)
			반환 -1;

		*bytes = (uint64_t) size;
	}

	반환 0;
}

/* get 512-byte sector count */
정수 blkdev_get_sectors(정수 fd, uint64_t *sectors)
{
	uint64_t bytes;

	만약 (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		반환 0;
	}

	반환 -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
정수 blkdev_get_sector_size(정수 fd, 정수 *sector_size)
{
	만약 (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		반환 0;
	반환 -1;
}
#아니면
정수 blkdev_get_sector_size(정수 fd __attribute__((__unused__)), 정수 *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	반환 0;
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
정수 blkdev_get_physector_size(정수 fd, 정수 *sector_size)
{
	만약 (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		반환 0;
	반환 -1;
}
#아니면
정수 blkdev_get_physector_size(정수 fd __attribute__((__unused__)), 정수 *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	반환 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
정수 blkdev_is_misaligned(정수 fd)
{
	정수 aligned;

	만약 (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		반환 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	반환 aligned != 0 ? 1 : 0;
}
#아니면
정수 blkdev_is_misaligned(정수 fd __attribute__((__unused__)))
{
	반환 0;
}
#endif

정수 open_blkdev_or_file(const struct stat *st, const char *name, const 정수 oflag)
{
	정수 fd;

	만약 (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} 아니면
		fd = open(name, oflag);
	만약 (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		반환 -1;
	}
	만약 (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	반환 fd;
}

#ifdef CDROM_GET_CAPABILITY
정수 blkdev_is_cdrom(정수 fd)
{
	정수 ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	반환 ret < 0 ? 0 : ret;
}
#아니면
정수 blkdev_is_cdrom(정수 fd __attribute__((__unused__)))
{
	반환 0;
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
정수 blkdev_get_geometry(정수 fd, unsigned 정수 *h, unsigned 정수 *s)
{
	struct hd_geometry geometry;

	만약 (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		반환 0;
	}
#아니면
정수 blkdev_get_geometry(정수 fd __attribute__((__unused__)),
		unsigned 정수 *h, unsigned 정수 *s)
{
	*h = 0;
	*s = 0;
#endif
	반환 -1;
}

/*
 * Convert scsi type to human readable string.
 */
const char *blkdev_scsi_type_to_name(정수 type)
{
	switch (type) {
	case SCSI_TYPE_DISK:
		반환 "disk";
	case SCSI_TYPE_TAPE:
		반환 "tape";
	case SCSI_TYPE_PRINTER:
		반환 "printer";
	case SCSI_TYPE_PROCESSOR:
		반환 "processor";
	case SCSI_TYPE_WORM:
		반환 "worm";
	case SCSI_TYPE_ROM:
		반환 "rom";
	case SCSI_TYPE_SCANNER:
		반환 "scanner";
	case SCSI_TYPE_MOD:
		반환 "mo-disk";
	case SCSI_TYPE_MEDIUM_CHANGER:
		반환 "changer";
	case SCSI_TYPE_COMM:
		반환 "comm";
	case SCSI_TYPE_RAID:
		반환 "raid";
	case SCSI_TYPE_ENCLOSURE:
		반환 "enclosure";
	case SCSI_TYPE_RBC:
		반환 "rbc";
	case SCSI_TYPE_OSD:
		반환 "osd";
	case SCSI_TYPE_NO_LUN:
		반환 "no-lun";
	default:
		break;
	}
	반환 NULL;
}

/* return 0 on success */
정수 blkdev_lock(정수 fd, const char *devname, const char *lockmode)
{
	정수 oper, rc, msg = 0;

	만약 (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	만약 (!lockmode)
		반환 0;

	만약 (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	아니면 만약 (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	아니면 만약 (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		반환 0;
	아니면 {
		warnx(_("unsupported lock mode: %s"), lockmode);
		반환 -EINVAL;
	}

	만약 (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		만약 (rc == 0)
			반환 0;
		만약 (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	만약 (rc != 0) {
		switch (errno) {
		case EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			break;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} 아니면 만약 (msg)
		fprintf(stderr, _("OK\n"));
	반환 rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
struct blk_zone_report *blkdev_get_zonereport(정수 fd, uint64_t sector, uint32_t nzones)
{
	struct blk_zone_report *rep;
	size_t rep_size;
	정수 ret;

	rep_size = sizeof(struct blk_zone_report) + sizeof(struct blk_zone) * nzones;
	rep = calloc(1, rep_size);
	만약 (!rep)
		반환 NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	만약 (ret || rep->nr_zones != nzones) {
		free(rep);
		반환 NULL;
	}

	반환 rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

정수 main(정수 argc, char **argv)
{
	uint64_t bytes, sectors;
	정수 sector_size, phy_sector_size;
	정수 fd;

	만약 (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	만약 ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	만약 (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	만약 (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	만약 (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	만약 (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	반환 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
