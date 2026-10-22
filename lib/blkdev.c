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

static long blkdev_valid_offset(ຈຳນວນເຕັມ fd, off_t offset)
{
	var_na ch;

	ຖ້າ (lseek (fd, offset, 0) < 0)
		ສົ່ງຄືນ 0;
	ຖ້າ (ul_read_all (fd, &ch, 1) < 1)
		ສົ່ງຄືນ 0;
	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ is_blkdev(ຈຳນວນເຕັມ fd)
{
	sa_nracanaa stat st;
	ສົ່ງຄືນ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ຈຳນວນເຕັມ fd)
{
	off_t high, low = 0;

	ສຳລັບ (high = 1024; blkdev_valid_offset(fd, high); ) {
		ຖ້າ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ສົ່ງຄືນ -1;
		}

		low = high;

		ຖ້າ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ບໍ່ດັ່ງນັ້ນ
			high *= 2;
	}

	ໃນຂະນະທີ່ (low < high - 1) {
		off_t mid = (low + high) / 2;

		ຖ້າ (blkdev_valid_offset (fd, mid))
			low = mid;
		ບໍ່ດັ່ງນັ້ນ
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ສົ່ງຄືນ (low + 1);
}

/* get size in bytes */
ຈຳນວນເຕັມ blkdev_get_size(ຈຳນວນເຕັມ fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ຖ້າ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ສົ່ງຄືນ 0;
	}
#endif

#ifdef BLKGETSIZE64
	ຖ້າ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ສົ່ງຄືນ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ຖ້າ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ສົ່ງຄືນ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ຖ້າ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ສົ່ງຄືນ 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		ຖ້າ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ສົ່ງຄືນ 0;
		}
	}
#endif /* FDGETPRM */

#ຖ້າ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ຈຳນວນເຕັມ part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		ຖ້າ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ຖ້າ (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ຖ້າ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ສົ່ງຄືນ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		ຖ້າ (fstat(fd, &st) != 0)
			ສົ່ງຄືນ -1;
		ຖ້າ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ສົ່ງຄືນ 0;
		}
		ຖ້າ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ສົ່ງຄືນ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ຖ້າ (size < 0)
			ສົ່ງຄືນ -1;

		*bytes = (uint64_t) size;
	}

	ສົ່ງຄືນ 0;
}

/* get 512-byte sector count */
ຈຳນວນເຕັມ blkdev_get_sectors(ຈຳນວນເຕັມ fd, uint64_t *sectors)
{
	uint64_t bytes;

	ຖ້າ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ສົ່ງຄືນ 0;
	}

	ສົ່ງຄືນ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ຈຳນວນເຕັມ blkdev_get_sector_size(ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ *sector_size)
{
	ຖ້າ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ສົ່ງຄືນ 0;
	ສົ່ງຄືນ -1;
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ blkdev_get_sector_size(ຈຳນວນເຕັມ fd __attribute__((__unused__)), ຈຳນວນເຕັມ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ສົ່ງຄືນ 0;
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
ຈຳນວນເຕັມ blkdev_get_physector_size(ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ *sector_size)
{
	ຖ້າ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ສົ່ງຄືນ 0;
	ສົ່ງຄືນ -1;
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ blkdev_get_physector_size(ຈຳນວນເຕັມ fd __attribute__((__unused__)), ຈຳນວນເຕັມ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ສົ່ງຄືນ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ຈຳນວນເຕັມ blkdev_is_misaligned(ຈຳນວນເຕັມ fd)
{
	ຈຳນວນເຕັມ aligned;

	ຖ້າ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ສົ່ງຄືນ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ສົ່ງຄືນ aligned != 0 ? 1 : 0;
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ blkdev_is_misaligned(ຈຳນວນເຕັມ fd __attribute__((__unused__)))
{
	ສົ່ງຄືນ 0;
}
#endif

ຈຳນວນເຕັມ open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const ຈຳນວນເຕັມ oflag)
{
	ຈຳນວນເຕັມ fd;

	ຖ້າ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ບໍ່ດັ່ງນັ້ນ
		fd = open(name, oflag);
	ຖ້າ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ສົ່ງຄືນ -1;
	}
	ຖ້າ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ສົ່ງຄືນ fd;
}

#ifdef CDROM_GET_CAPABILITY
ຈຳນວນເຕັມ blkdev_is_cdrom(ຈຳນວນເຕັມ fd)
{
	ຈຳນວນເຕັມ ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ສົ່ງຄືນ ret < 0 ? 0 : ret;
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ blkdev_is_cdrom(ຈຳນວນເຕັມ fd __attribute__((__unused__)))
{
	ສົ່ງຄືນ 0;
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
ຈຳນວນເຕັມ blkdev_get_geometry(ຈຳນວນເຕັມ fd, unsigned ຈຳນວນເຕັມ *h, unsigned ຈຳນວນເຕັມ *s)
{
	sa_nracanaa hd_geometry geometry;

	ຖ້າ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ສົ່ງຄືນ 0;
	}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ blkdev_get_geometry(ຈຳນວນເຕັມ fd __attribute__((__unused__)),
		unsigned ຈຳນວນເຕັມ *h, unsigned ຈຳນວນເຕັມ *s)
{
	*h = 0;
	*s = 0;
#endif
	ສົ່ງຄືນ -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(ຈຳນວນເຕັມ type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ສົ່ງຄືນ "disk";
	pak_sa SCSI_TYPE_TAPE:
		ສົ່ງຄືນ "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ສົ່ງຄືນ "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ສົ່ງຄືນ "processor";
	pak_sa SCSI_TYPE_WORM:
		ສົ່ງຄືນ "worm";
	pak_sa SCSI_TYPE_ROM:
		ສົ່ງຄືນ "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ສົ່ງຄືນ "scanner";
	pak_sa SCSI_TYPE_MOD:
		ສົ່ງຄືນ "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ສົ່ງຄືນ "changer";
	pak_sa SCSI_TYPE_COMM:
		ສົ່ງຄືນ "comm";
	pak_sa SCSI_TYPE_RAID:
		ສົ່ງຄືນ "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ສົ່ງຄືນ "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ສົ່ງຄືນ "rbc";
	pak_sa SCSI_TYPE_OSD:
		ສົ່ງຄືນ "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ສົ່ງຄືນ "no-lun";
	default:
		bha_nga;
	}
	ສົ່ງຄືນ NULL;
}

/* return 0 on success */
ຈຳນວນເຕັມ blkdev_lock(ຈຳນວນເຕັມ fd, const var_na *devname, const var_na *lockmode)
{
	ຈຳນວນເຕັມ oper, rc, msg = 0;

	ຖ້າ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ຖ້າ (!lockmode)
		ສົ່ງຄືນ 0;

	ຖ້າ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ບໍ່ດັ່ງນັ້ນ ຖ້າ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ບໍ່ດັ່ງນັ້ນ ຖ້າ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ສົ່ງຄືນ 0;
	ບໍ່ດັ່ງນັ້ນ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ສົ່ງຄືນ -EINVAL;
	}

	ຖ້າ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ຖ້າ (rc == 0)
			ສົ່ງຄືນ 0;
		ຖ້າ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	ຖ້າ (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (msg)
		fprintf(stderr, _("OK\n"));
	ສົ່ງຄືນ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(ຈຳນວນເຕັມ fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	ຈຳນວນເຕັມ ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ຖ້າ (!rep)
		ສົ່ງຄືນ NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ຖ້າ (ret || rep->nr_zones != nzones) {
		free(rep);
		ສົ່ງຄືນ NULL;
	}

	ສົ່ງຄືນ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	uint64_t bytes, sectors;
	ຈຳນວນເຕັມ sector_size, phy_sector_size;
	ຈຳນວນເຕັມ fd;

	ຖ້າ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ຖ້າ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ຖ້າ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ຖ້າ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ຖ້າ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ຖ້າ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ສົ່ງຄືນ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
