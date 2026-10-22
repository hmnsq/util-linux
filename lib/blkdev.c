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

static long blkdev_valid_offset(จำนวนเต็ม fd, off_t offset)
{
	var_na ch;

	ถ้า (lseek (fd, offset, 0) < 0)
		คืน 0;
	ถ้า (ul_read_all (fd, &ch, 1) < 1)
		คืน 0;
	คืน 1;
}

จำนวนเต็ม is_blkdev(จำนวนเต็ม fd)
{
	sa_nracanaa stat st;
	คืน (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (จำนวนเต็ม fd)
{
	off_t high, low = 0;

	สำหรับ (high = 1024; blkdev_valid_offset(fd, high); ) {
		ถ้า (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			คืน -1;
		}

		low = high;

		ถ้า (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		มิฉะนั้น
			high *= 2;
	}

	ขณะ (low < high - 1) {
		off_t mid = (low + high) / 2;

		ถ้า (blkdev_valid_offset (fd, mid))
			low = mid;
		มิฉะนั้น
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	คืน (low + 1);
}

/* get size in bytes */
จำนวนเต็ม blkdev_get_size(จำนวนเต็ม fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ถ้า (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		คืน 0;
	}
#endif

#ifdef BLKGETSIZE64
	ถ้า (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		คืน 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ถ้า (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			คืน 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ถ้า (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		คืน 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		ถ้า (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			คืน 0;
		}
	}
#endif /* FDGETPRM */

#ถ้า defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		จำนวนเต็ม part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		ถ้า ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ถ้า (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ถ้า (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 คืน 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		ถ้า (fstat(fd, &st) != 0)
			คืน -1;
		ถ้า (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			คืน 0;
		}
		ถ้า (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			คืน -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ถ้า (size < 0)
			คืน -1;

		*bytes = (uint64_t) size;
	}

	คืน 0;
}

/* get 512-byte sector count */
จำนวนเต็ม blkdev_get_sectors(จำนวนเต็ม fd, uint64_t *sectors)
{
	uint64_t bytes;

	ถ้า (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		คืน 0;
	}

	คืน -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
จำนวนเต็ม blkdev_get_sector_size(จำนวนเต็ม fd, จำนวนเต็ม *sector_size)
{
	ถ้า (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		คืน 0;
	คืน -1;
}
#มิฉะนั้น
จำนวนเต็ม blkdev_get_sector_size(จำนวนเต็ม fd __attribute__((__unused__)), จำนวนเต็ม *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	คืน 0;
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
จำนวนเต็ม blkdev_get_physector_size(จำนวนเต็ม fd, จำนวนเต็ม *sector_size)
{
	ถ้า (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		คืน 0;
	คืน -1;
}
#มิฉะนั้น
จำนวนเต็ม blkdev_get_physector_size(จำนวนเต็ม fd __attribute__((__unused__)), จำนวนเต็ม *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	คืน 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
จำนวนเต็ม blkdev_is_misaligned(จำนวนเต็ม fd)
{
	จำนวนเต็ม aligned;

	ถ้า (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		คืน 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	คืน aligned != 0 ? 1 : 0;
}
#มิฉะนั้น
จำนวนเต็ม blkdev_is_misaligned(จำนวนเต็ม fd __attribute__((__unused__)))
{
	คืน 0;
}
#endif

จำนวนเต็ม open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const จำนวนเต็ม oflag)
{
	จำนวนเต็ม fd;

	ถ้า (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} มิฉะนั้น
		fd = open(name, oflag);
	ถ้า (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		คืน -1;
	}
	ถ้า (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	คืน fd;
}

#ifdef CDROM_GET_CAPABILITY
จำนวนเต็ม blkdev_is_cdrom(จำนวนเต็ม fd)
{
	จำนวนเต็ม ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	คืน ret < 0 ? 0 : ret;
}
#มิฉะนั้น
จำนวนเต็ม blkdev_is_cdrom(จำนวนเต็ม fd __attribute__((__unused__)))
{
	คืน 0;
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
จำนวนเต็ม blkdev_get_geometry(จำนวนเต็ม fd, unsigned จำนวนเต็ม *h, unsigned จำนวนเต็ม *s)
{
	sa_nracanaa hd_geometry geometry;

	ถ้า (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		คืน 0;
	}
#มิฉะนั้น
จำนวนเต็ม blkdev_get_geometry(จำนวนเต็ม fd __attribute__((__unused__)),
		unsigned จำนวนเต็ม *h, unsigned จำนวนเต็ม *s)
{
	*h = 0;
	*s = 0;
#endif
	คืน -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(จำนวนเต็ม type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		คืน "disk";
	pak_sa SCSI_TYPE_TAPE:
		คืน "tape";
	pak_sa SCSI_TYPE_PRINTER:
		คืน "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		คืน "processor";
	pak_sa SCSI_TYPE_WORM:
		คืน "worm";
	pak_sa SCSI_TYPE_ROM:
		คืน "rom";
	pak_sa SCSI_TYPE_SCANNER:
		คืน "scanner";
	pak_sa SCSI_TYPE_MOD:
		คืน "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		คืน "changer";
	pak_sa SCSI_TYPE_COMM:
		คืน "comm";
	pak_sa SCSI_TYPE_RAID:
		คืน "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		คืน "enclosure";
	pak_sa SCSI_TYPE_RBC:
		คืน "rbc";
	pak_sa SCSI_TYPE_OSD:
		คืน "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		คืน "no-lun";
	default:
		bha_nga;
	}
	คืน NULL;
}

/* return 0 on success */
จำนวนเต็ม blkdev_lock(จำนวนเต็ม fd, const var_na *devname, const var_na *lockmode)
{
	จำนวนเต็ม oper, rc, msg = 0;

	ถ้า (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ถ้า (!lockmode)
		คืน 0;

	ถ้า (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	มิฉะนั้น ถ้า (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	มิฉะนั้น ถ้า (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		คืน 0;
	มิฉะนั้น {
		warnx(_("unsupported lock mode: %s"), lockmode);
		คืน -EINVAL;
	}

	ถ้า (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ถ้า (rc == 0)
			คืน 0;
		ถ้า (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	ถ้า (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} มิฉะนั้น ถ้า (msg)
		fprintf(stderr, _("OK\n"));
	คืน rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(จำนวนเต็ม fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	จำนวนเต็ม ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ถ้า (!rep)
		คืน NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ถ้า (ret || rep->nr_zones != nzones) {
		free(rep);
		คืน NULL;
	}

	คืน rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
	uint64_t bytes, sectors;
	จำนวนเต็ม sector_size, phy_sector_size;
	จำนวนเต็ม fd;

	ถ้า (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ถ้า ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ถ้า (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ถ้า (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ถ้า (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ถ้า (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	คืน EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
