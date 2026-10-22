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

static long blkdev_valid_offset(цео fd, off_t offset)
{
	char ch;

	ако (lseek (fd, offset, 0) < 0)
		врати 0;
	ако (ul_read_all (fd, &ch, 1) < 1)
		врати 0;
	врати 1;
}

цео is_blkdev(цео fd)
{
	struct stat st;
	врати (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (цео fd)
{
	off_t high, low = 0;

	за (high = 1024; blkdev_valid_offset(fd, high); ) {
		ако (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			врати -1;
		}

		low = high;

		ако (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		инако
			high *= 2;
	}

	док (low < high - 1) {
		off_t mid = (low + high) / 2;

		ако (blkdev_valid_offset (fd, mid))
			low = mid;
		инако
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	врати (low + 1);
}

/* get size in bytes */
цео blkdev_get_size(цео fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ако (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		врати 0;
	}
#endif

#ifdef BLKGETSIZE64
	ако (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		врати 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ако (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			врати 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ако (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		врати 0;
#endif

#ifdef FDGETPRM
	{
		struct floppy_struct this_floppy;

		ако (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			врати 0;
		}
	}
#endif /* FDGETPRM */

#ако defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		цео part = -1;
		struct disklabel lab;
		struct partition *pp;
		struct stat st;

		ако ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ако (part >= 0 && (ioctl(fd, DIOCGDINFO, (char *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ако (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 врати 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		struct stat st;

		ако (fstat(fd, &st) != 0)
			врати -1;
		ако (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			врати 0;
		}
		ако (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			врати -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ако (size < 0)
			врати -1;

		*bytes = (uint64_t) size;
	}

	врати 0;
}

/* get 512-byte sector count */
цео blkdev_get_sectors(цео fd, uint64_t *sectors)
{
	uint64_t bytes;

	ако (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		врати 0;
	}

	врати -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
цео blkdev_get_sector_size(цео fd, цео *sector_size)
{
	ако (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		врати 0;
	врати -1;
}
#инако
цео blkdev_get_sector_size(цео fd __attribute__((__unused__)), цео *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	врати 0;
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
цео blkdev_get_physector_size(цео fd, цео *sector_size)
{
	ако (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		врати 0;
	врати -1;
}
#инако
цео blkdev_get_physector_size(цео fd __attribute__((__unused__)), цео *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	врати 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
цео blkdev_is_misaligned(цео fd)
{
	цео aligned;

	ако (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		врати 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	врати aligned != 0 ? 1 : 0;
}
#инако
цео blkdev_is_misaligned(цео fd __attribute__((__unused__)))
{
	врати 0;
}
#endif

цео open_blkdev_or_file(const struct stat *st, const char *name, const цео oflag)
{
	цео fd;

	ако (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} инако
		fd = open(name, oflag);
	ако (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		врати -1;
	}
	ако (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	врати fd;
}

#ifdef CDROM_GET_CAPABILITY
цео blkdev_is_cdrom(цео fd)
{
	цео ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	врати ret < 0 ? 0 : ret;
}
#инако
цео blkdev_is_cdrom(цео fd __attribute__((__unused__)))
{
	врати 0;
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
цео blkdev_get_geometry(цео fd, unsigned цео *h, unsigned цео *s)
{
	struct hd_geometry geometry;

	ако (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		врати 0;
	}
#инако
цео blkdev_get_geometry(цео fd __attribute__((__unused__)),
		unsigned цео *h, unsigned цео *s)
{
	*h = 0;
	*s = 0;
#endif
	врати -1;
}

/*
 * Convert scsi type to human readable string.
 */
const char *blkdev_scsi_type_to_name(цео type)
{
	switch (type) {
	case SCSI_TYPE_DISK:
		врати "disk";
	case SCSI_TYPE_TAPE:
		врати "tape";
	case SCSI_TYPE_PRINTER:
		врати "printer";
	case SCSI_TYPE_PROCESSOR:
		врати "processor";
	case SCSI_TYPE_WORM:
		врати "worm";
	case SCSI_TYPE_ROM:
		врати "rom";
	case SCSI_TYPE_SCANNER:
		врати "scanner";
	case SCSI_TYPE_MOD:
		врати "mo-disk";
	case SCSI_TYPE_MEDIUM_CHANGER:
		врати "changer";
	case SCSI_TYPE_COMM:
		врати "comm";
	case SCSI_TYPE_RAID:
		врати "raid";
	case SCSI_TYPE_ENCLOSURE:
		врати "enclosure";
	case SCSI_TYPE_RBC:
		врати "rbc";
	case SCSI_TYPE_OSD:
		врати "osd";
	case SCSI_TYPE_NO_LUN:
		врати "no-lun";
	default:
		break;
	}
	врати NULL;
}

/* return 0 on success */
цео blkdev_lock(цео fd, const char *devname, const char *lockmode)
{
	цео oper, rc, msg = 0;

	ако (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ако (!lockmode)
		врати 0;

	ако (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	инако ако (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	инако ако (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		врати 0;
	инако {
		warnx(_("unsupported lock mode: %s"), lockmode);
		врати -EINVAL;
	}

	ако (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ако (rc == 0)
			врати 0;
		ако (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	ако (rc != 0) {
		switch (errno) {
		case EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			break;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} инако ако (msg)
		fprintf(stderr, _("OK\n"));
	врати rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
struct blk_zone_report *blkdev_get_zonereport(цео fd, uint64_t sector, uint32_t nzones)
{
	struct blk_zone_report *rep;
	size_t rep_size;
	цео ret;

	rep_size = sizeof(struct blk_zone_report) + sizeof(struct blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ако (!rep)
		врати NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ако (ret || rep->nr_zones != nzones) {
		free(rep);
		врати NULL;
	}

	врати rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

цео main(цео argc, char **argv)
{
	uint64_t bytes, sectors;
	цео sector_size, phy_sector_size;
	цео fd;

	ако (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ако ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ако (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ако (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ако (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ако (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	врати EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
