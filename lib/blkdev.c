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

static long blkdev_valid_offset(පූර්ණ_සංඛ්‍යා fd, off_t offset)
{
	var_na ch;

	නම් (lseek (fd, offset, 0) < 0)
		ආපසු 0;
	නම් (ul_read_all (fd, &ch, 1) < 1)
		ආපසු 0;
	ආපසු 1;
}

පූර්ණ_සංඛ්‍යා is_blkdev(පූර්ණ_සංඛ්‍යා fd)
{
	sa_nracanaa stat st;
	ආපසු (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (පූර්ණ_සංඛ්‍යා fd)
{
	off_t high, low = 0;

	සඳහා (high = 1024; blkdev_valid_offset(fd, high); ) {
		නම් (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ආපසු -1;
		}

		low = high;

		නම් (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		නැතහොත්
			high *= 2;
	}

	අතරතුර (low < high - 1) {
		off_t mid = (low + high) / 2;

		නම් (blkdev_valid_offset (fd, mid))
			low = mid;
		නැතහොත්
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ආපසු (low + 1);
}

/* get size in bytes */
පූර්ණ_සංඛ්‍යා blkdev_get_size(පූර්ණ_සංඛ්‍යා fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	නම් (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ආපසු 0;
	}
#endif

#ifdef BLKGETSIZE64
	නම් (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ආපසු 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		නම් (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ආපසු 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	නම් (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ආපසු 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		නම් (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ආපසු 0;
		}
	}
#endif /* FDGETPRM */

#නම් defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		පූර්ණ_සංඛ්‍යා part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		නම් ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		නම් (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			නම් (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ආපසු 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		නම් (fstat(fd, &st) != 0)
			ආපසු -1;
		නම් (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ආපසු 0;
		}
		නම් (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ආපසු -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		නම් (size < 0)
			ආපසු -1;

		*bytes = (uint64_t) size;
	}

	ආපසු 0;
}

/* get 512-byte sector count */
පූර්ණ_සංඛ්‍යා blkdev_get_sectors(පූර්ණ_සංඛ්‍යා fd, uint64_t *sectors)
{
	uint64_t bytes;

	නම් (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ආපසු 0;
	}

	ආපසු -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
පූර්ණ_සංඛ්‍යා blkdev_get_sector_size(පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා *sector_size)
{
	නම් (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ආපසු 0;
	ආපසු -1;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා blkdev_get_sector_size(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)), පූර්ණ_සංඛ්‍යා *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ආපසු 0;
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
පූර්ණ_සංඛ්‍යා blkdev_get_physector_size(පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා *sector_size)
{
	නම් (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ආපසු 0;
	ආපසු -1;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා blkdev_get_physector_size(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)), පූර්ණ_සංඛ්‍යා *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ආපසු 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
පූර්ණ_සංඛ්‍යා blkdev_is_misaligned(පූර්ණ_සංඛ්‍යා fd)
{
	පූර්ණ_සංඛ්‍යා aligned;

	නම් (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ආපසු 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ආපසු aligned != 0 ? 1 : 0;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා blkdev_is_misaligned(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)))
{
	ආපසු 0;
}
#endif

පූර්ණ_සංඛ්‍යා open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const පූර්ණ_සංඛ්‍යා oflag)
{
	පූර්ණ_සංඛ්‍යා fd;

	නම් (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} නැතහොත්
		fd = open(name, oflag);
	නම් (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ආපසු -1;
	}
	නම් (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ආපසු fd;
}

#ifdef CDROM_GET_CAPABILITY
පූර්ණ_සංඛ්‍යා blkdev_is_cdrom(පූර්ණ_සංඛ්‍යා fd)
{
	පූර්ණ_සංඛ්‍යා ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ආපසු ret < 0 ? 0 : ret;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා blkdev_is_cdrom(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)))
{
	ආපසු 0;
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
පූර්ණ_සංඛ්‍යා blkdev_get_geometry(පූර්ණ_සංඛ්‍යා fd, unsigned පූර්ණ_සංඛ්‍යා *h, unsigned පූර්ණ_සංඛ්‍යා *s)
{
	sa_nracanaa hd_geometry geometry;

	නම් (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ආපසු 0;
	}
#නැතහොත්
පූර්ණ_සංඛ්‍යා blkdev_get_geometry(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)),
		unsigned පූර්ණ_සංඛ්‍යා *h, unsigned පූර්ණ_සංඛ්‍යා *s)
{
	*h = 0;
	*s = 0;
#endif
	ආපසු -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(පූර්ණ_සංඛ්‍යා type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ආපසු "disk";
	pak_sa SCSI_TYPE_TAPE:
		ආපසු "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ආපසු "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ආපසු "processor";
	pak_sa SCSI_TYPE_WORM:
		ආපසු "worm";
	pak_sa SCSI_TYPE_ROM:
		ආපසු "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ආපසු "scanner";
	pak_sa SCSI_TYPE_MOD:
		ආපසු "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ආපසු "changer";
	pak_sa SCSI_TYPE_COMM:
		ආපසු "comm";
	pak_sa SCSI_TYPE_RAID:
		ආපසු "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ආපසු "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ආපසු "rbc";
	pak_sa SCSI_TYPE_OSD:
		ආපසු "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ආපසු "no-lun";
	default:
		bha_nga;
	}
	ආපසු NULL;
}

/* return 0 on success */
පූර්ණ_සංඛ්‍යා blkdev_lock(පූර්ණ_සංඛ්‍යා fd, const var_na *devname, const var_na *lockmode)
{
	පූර්ණ_සංඛ්‍යා oper, rc, msg = 0;

	නම් (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	නම් (!lockmode)
		ආපසු 0;

	නම් (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	නැතහොත් නම් (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	නැතහොත් නම් (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ආපසු 0;
	නැතහොත් {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ආපසු -EINVAL;
	}

	නම් (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		නම් (rc == 0)
			ආපසු 0;
		නම් (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	නම් (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} නැතහොත් නම් (msg)
		fprintf(stderr, _("OK\n"));
	ආපසු rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(පූර්ණ_සංඛ්‍යා fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	පූර්ණ_සංඛ්‍යා ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	නම් (!rep)
		ආපසු NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	නම් (ret || rep->nr_zones != nzones) {
		free(rep);
		ආපසු NULL;
	}

	ආපසු rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	uint64_t bytes, sectors;
	පූර්ණ_සංඛ්‍යා sector_size, phy_sector_size;
	පූර්ණ_සංඛ්‍යා fd;

	නම් (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	නම් ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	නම් (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	නම් (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	නම් (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	නම් (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ආපසු EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
