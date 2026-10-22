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

static long blkdev_valid_offset(ཧྲིལ fd, off_t offset)
{
	var_na ch;

	གལ་ཏེ (lseek (fd, offset, 0) < 0)
		ཕྱིར 0;
	གལ་ཏེ (ul_read_all (fd, &ch, 1) < 1)
		ཕྱིར 0;
	ཕྱིར 1;
}

ཧྲིལ is_blkdev(ཧྲིལ fd)
{
	sa_nracanaa stat st;
	ཕྱིར (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ཧྲིལ fd)
{
	off_t high, low = 0;

	ཆེད་དུ (high = 1024; blkdev_valid_offset(fd, high); ) {
		གལ་ཏེ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ཕྱིར -1;
		}

		low = high;

		གལ་ཏེ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		དེ་མིན
			high *= 2;
	}

	སྐབས (low < high - 1) {
		off_t mid = (low + high) / 2;

		གལ་ཏེ (blkdev_valid_offset (fd, mid))
			low = mid;
		དེ་མིན
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ཕྱིར (low + 1);
}

/* get size in bytes */
ཧྲིལ blkdev_get_size(ཧྲིལ fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	གལ་ཏེ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ཕྱིར 0;
	}
#endif

#ifdef BLKGETSIZE64
	གལ་ཏེ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ཕྱིར 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		གལ་ཏེ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ཕྱིར 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	གལ་ཏེ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ཕྱིར 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		གལ་ཏེ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ཕྱིར 0;
		}
	}
#endif /* FDGETPRM */

#གལ་ཏེ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ཧྲིལ part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		གལ་ཏེ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		གལ་ཏེ (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			གལ་ཏེ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ཕྱིར 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		གལ་ཏེ (fstat(fd, &st) != 0)
			ཕྱིར -1;
		གལ་ཏེ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ཕྱིར 0;
		}
		གལ་ཏེ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ཕྱིར -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		གལ་ཏེ (size < 0)
			ཕྱིར -1;

		*bytes = (uint64_t) size;
	}

	ཕྱིར 0;
}

/* get 512-byte sector count */
ཧྲིལ blkdev_get_sectors(ཧྲིལ fd, uint64_t *sectors)
{
	uint64_t bytes;

	གལ་ཏེ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ཕྱིར 0;
	}

	ཕྱིར -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ཧྲིལ blkdev_get_sector_size(ཧྲིལ fd, ཧྲིལ *sector_size)
{
	གལ་ཏེ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ཕྱིར 0;
	ཕྱིར -1;
}
#དེ་མིན
ཧྲིལ blkdev_get_sector_size(ཧྲིལ fd __attribute__((__unused__)), ཧྲིལ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ཕྱིར 0;
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
ཧྲིལ blkdev_get_physector_size(ཧྲིལ fd, ཧྲིལ *sector_size)
{
	གལ་ཏེ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ཕྱིར 0;
	ཕྱིར -1;
}
#དེ་མིན
ཧྲིལ blkdev_get_physector_size(ཧྲིལ fd __attribute__((__unused__)), ཧྲིལ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ཕྱིར 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ཧྲིལ blkdev_is_misaligned(ཧྲིལ fd)
{
	ཧྲིལ aligned;

	གལ་ཏེ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ཕྱིར 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ཕྱིར aligned != 0 ? 1 : 0;
}
#དེ་མིན
ཧྲིལ blkdev_is_misaligned(ཧྲིལ fd __attribute__((__unused__)))
{
	ཕྱིར 0;
}
#endif

ཧྲིལ open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const ཧྲིལ oflag)
{
	ཧྲིལ fd;

	གལ་ཏེ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} དེ་མིན
		fd = open(name, oflag);
	གལ་ཏེ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ཕྱིར -1;
	}
	གལ་ཏེ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ཕྱིར fd;
}

#ifdef CDROM_GET_CAPABILITY
ཧྲིལ blkdev_is_cdrom(ཧྲིལ fd)
{
	ཧྲིལ ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ཕྱིར ret < 0 ? 0 : ret;
}
#དེ་མིན
ཧྲིལ blkdev_is_cdrom(ཧྲིལ fd __attribute__((__unused__)))
{
	ཕྱིར 0;
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
ཧྲིལ blkdev_get_geometry(ཧྲིལ fd, unsigned ཧྲིལ *h, unsigned ཧྲིལ *s)
{
	sa_nracanaa hd_geometry geometry;

	གལ་ཏེ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ཕྱིར 0;
	}
#དེ་མིན
ཧྲིལ blkdev_get_geometry(ཧྲིལ fd __attribute__((__unused__)),
		unsigned ཧྲིལ *h, unsigned ཧྲིལ *s)
{
	*h = 0;
	*s = 0;
#endif
	ཕྱིར -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(ཧྲིལ type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ཕྱིར "disk";
	pak_sa SCSI_TYPE_TAPE:
		ཕྱིར "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ཕྱིར "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ཕྱིར "processor";
	pak_sa SCSI_TYPE_WORM:
		ཕྱིར "worm";
	pak_sa SCSI_TYPE_ROM:
		ཕྱིར "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ཕྱིར "scanner";
	pak_sa SCSI_TYPE_MOD:
		ཕྱིར "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ཕྱིར "changer";
	pak_sa SCSI_TYPE_COMM:
		ཕྱིར "comm";
	pak_sa SCSI_TYPE_RAID:
		ཕྱིར "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ཕྱིར "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ཕྱིར "rbc";
	pak_sa SCSI_TYPE_OSD:
		ཕྱིར "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ཕྱིར "no-lun";
	default:
		bha_nga;
	}
	ཕྱིར NULL;
}

/* return 0 on success */
ཧྲིལ blkdev_lock(ཧྲིལ fd, const var_na *devname, const var_na *lockmode)
{
	ཧྲིལ oper, rc, msg = 0;

	གལ་ཏེ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	གལ་ཏེ (!lockmode)
		ཕྱིར 0;

	གལ་ཏེ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	དེ་མིན གལ་ཏེ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	དེ་མིན གལ་ཏེ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ཕྱིར 0;
	དེ་མིན {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ཕྱིར -EINVAL;
	}

	གལ་ཏེ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		གལ་ཏེ (rc == 0)
			ཕྱིར 0;
		གལ་ཏེ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	གལ་ཏེ (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} དེ་མིན གལ་ཏེ (msg)
		fprintf(stderr, _("OK\n"));
	ཕྱིར rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(ཧྲིལ fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	ཧྲིལ ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	གལ་ཏེ (!rep)
		ཕྱིར NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	གལ་ཏེ (ret || rep->nr_zones != nzones) {
		free(rep);
		ཕྱིར NULL;
	}

	ཕྱིར rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	uint64_t bytes, sectors;
	ཧྲིལ sector_size, phy_sector_size;
	ཧྲིལ fd;

	གལ་ཏེ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	གལ་ཏེ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	གལ་ཏེ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	གལ་ཏེ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	གལ་ཏེ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	གལ་ཏེ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ཕྱིར EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
