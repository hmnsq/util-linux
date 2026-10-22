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

static long blkdev_valid_offset(ចំនួនគត់ fd, off_t offset)
{
	var_na ch;

	ប្រសិនបើ (lseek (fd, offset, 0) < 0)
		ត្រឡប់ 0;
	ប្រសិនបើ (ul_read_all (fd, &ch, 1) < 1)
		ត្រឡប់ 0;
	ត្រឡប់ 1;
}

ចំនួនគត់ is_blkdev(ចំនួនគត់ fd)
{
	sa_nracanaa stat st;
	ត្រឡប់ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ចំនួនគត់ fd)
{
	off_t high, low = 0;

	សម្រាប់ (high = 1024; blkdev_valid_offset(fd, high); ) {
		ប្រសិនបើ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ត្រឡប់ -1;
		}

		low = high;

		ប្រសិនបើ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		បើមិនដូច្នេះ
			high *= 2;
	}

	ខណៈពេល (low < high - 1) {
		off_t mid = (low + high) / 2;

		ប្រសិនបើ (blkdev_valid_offset (fd, mid))
			low = mid;
		បើមិនដូច្នេះ
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ត្រឡប់ (low + 1);
}

/* get size in bytes */
ចំនួនគត់ blkdev_get_size(ចំនួនគត់ fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ប្រសិនបើ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ត្រឡប់ 0;
	}
#endif

#ifdef BLKGETSIZE64
	ប្រសិនបើ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ត្រឡប់ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ប្រសិនបើ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ត្រឡប់ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ប្រសិនបើ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ត្រឡប់ 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		ប្រសិនបើ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ត្រឡប់ 0;
		}
	}
#endif /* FDGETPRM */

#ប្រសិនបើ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ចំនួនគត់ part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		ប្រសិនបើ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ប្រសិនបើ (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ប្រសិនបើ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ត្រឡប់ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		ប្រសិនបើ (fstat(fd, &st) != 0)
			ត្រឡប់ -1;
		ប្រសិនបើ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ត្រឡប់ 0;
		}
		ប្រសិនបើ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ត្រឡប់ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ប្រសិនបើ (size < 0)
			ត្រឡប់ -1;

		*bytes = (uint64_t) size;
	}

	ត្រឡប់ 0;
}

/* get 512-byte sector count */
ចំនួនគត់ blkdev_get_sectors(ចំនួនគត់ fd, uint64_t *sectors)
{
	uint64_t bytes;

	ប្រសិនបើ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ត្រឡប់ 0;
	}

	ត្រឡប់ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ចំនួនគត់ blkdev_get_sector_size(ចំនួនគត់ fd, ចំនួនគត់ *sector_size)
{
	ប្រសិនបើ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ត្រឡប់ 0;
	ត្រឡប់ -1;
}
#បើមិនដូច្នេះ
ចំនួនគត់ blkdev_get_sector_size(ចំនួនគត់ fd __attribute__((__unused__)), ចំនួនគត់ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ត្រឡប់ 0;
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
ចំនួនគត់ blkdev_get_physector_size(ចំនួនគត់ fd, ចំនួនគត់ *sector_size)
{
	ប្រសិនបើ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ត្រឡប់ 0;
	ត្រឡប់ -1;
}
#បើមិនដូច្នេះ
ចំនួនគត់ blkdev_get_physector_size(ចំនួនគត់ fd __attribute__((__unused__)), ចំនួនគត់ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ត្រឡប់ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ចំនួនគត់ blkdev_is_misaligned(ចំនួនគត់ fd)
{
	ចំនួនគត់ aligned;

	ប្រសិនបើ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ត្រឡប់ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ត្រឡប់ aligned != 0 ? 1 : 0;
}
#បើមិនដូច្នេះ
ចំនួនគត់ blkdev_is_misaligned(ចំនួនគត់ fd __attribute__((__unused__)))
{
	ត្រឡប់ 0;
}
#endif

ចំនួនគត់ open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const ចំនួនគត់ oflag)
{
	ចំនួនគត់ fd;

	ប្រសិនបើ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} បើមិនដូច្នេះ
		fd = open(name, oflag);
	ប្រសិនបើ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ត្រឡប់ -1;
	}
	ប្រសិនបើ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ត្រឡប់ fd;
}

#ifdef CDROM_GET_CAPABILITY
ចំនួនគត់ blkdev_is_cdrom(ចំនួនគត់ fd)
{
	ចំនួនគត់ ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ត្រឡប់ ret < 0 ? 0 : ret;
}
#បើមិនដូច្នេះ
ចំនួនគត់ blkdev_is_cdrom(ចំនួនគត់ fd __attribute__((__unused__)))
{
	ត្រឡប់ 0;
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
ចំនួនគត់ blkdev_get_geometry(ចំនួនគត់ fd, unsigned ចំនួនគត់ *h, unsigned ចំនួនគត់ *s)
{
	sa_nracanaa hd_geometry geometry;

	ប្រសិនបើ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ត្រឡប់ 0;
	}
#បើមិនដូច្នេះ
ចំនួនគត់ blkdev_get_geometry(ចំនួនគត់ fd __attribute__((__unused__)),
		unsigned ចំនួនគត់ *h, unsigned ចំនួនគត់ *s)
{
	*h = 0;
	*s = 0;
#endif
	ត្រឡប់ -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(ចំនួនគត់ type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ត្រឡប់ "disk";
	pak_sa SCSI_TYPE_TAPE:
		ត្រឡប់ "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ត្រឡប់ "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ត្រឡប់ "processor";
	pak_sa SCSI_TYPE_WORM:
		ត្រឡប់ "worm";
	pak_sa SCSI_TYPE_ROM:
		ត្រឡប់ "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ត្រឡប់ "scanner";
	pak_sa SCSI_TYPE_MOD:
		ត្រឡប់ "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ត្រឡប់ "changer";
	pak_sa SCSI_TYPE_COMM:
		ត្រឡប់ "comm";
	pak_sa SCSI_TYPE_RAID:
		ត្រឡប់ "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ត្រឡប់ "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ត្រឡប់ "rbc";
	pak_sa SCSI_TYPE_OSD:
		ត្រឡប់ "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ត្រឡប់ "no-lun";
	default:
		bha_nga;
	}
	ត្រឡប់ NULL;
}

/* return 0 on success */
ចំនួនគត់ blkdev_lock(ចំនួនគត់ fd, const var_na *devname, const var_na *lockmode)
{
	ចំនួនគត់ oper, rc, msg = 0;

	ប្រសិនបើ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ប្រសិនបើ (!lockmode)
		ត្រឡប់ 0;

	ប្រសិនបើ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	បើមិនដូច្នេះ ប្រសិនបើ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	បើមិនដូច្នេះ ប្រសិនបើ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ត្រឡប់ 0;
	បើមិនដូច្នេះ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ត្រឡប់ -EINVAL;
	}

	ប្រសិនបើ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ប្រសិនបើ (rc == 0)
			ត្រឡប់ 0;
		ប្រសិនបើ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	ប្រសិនបើ (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} បើមិនដូច្នេះ ប្រសិនបើ (msg)
		fprintf(stderr, _("OK\n"));
	ត្រឡប់ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(ចំនួនគត់ fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	ចំនួនគត់ ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ប្រសិនបើ (!rep)
		ត្រឡប់ NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ប្រសិនបើ (ret || rep->nr_zones != nzones) {
		free(rep);
		ត្រឡប់ NULL;
	}

	ត្រឡប់ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

ចំនួនគត់ main(ចំនួនគត់ argc, var_na **argv)
{
	uint64_t bytes, sectors;
	ចំនួនគត់ sector_size, phy_sector_size;
	ចំនួនគត់ fd;

	ប្រសិនបើ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ប្រសិនបើ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ប្រសិនបើ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ប្រសិនបើ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ប្រសិនបើ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ប្រសិនបើ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ត្រឡប់ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
