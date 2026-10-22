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

static long blkdev_valid_offset(puur_naa_nka fd, off_t offset)
{
	var_na ch;

	ᯀᯮᯝ (lseek (fd, offset, 0) < 0)
		ᯀᯮᯞ 0;
	ᯀᯮᯝ (ul_read_all (fd, &ch, 1) < 1)
		ᯀᯮᯞ 0;
	ᯀᯮᯞ 1;
}

puur_naa_nka is_blkdev(puur_naa_nka fd)
{
	sa_nracanaa stat st;
	ᯀᯮᯞ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (puur_naa_nka fd)
{
	off_t high, low = 0;

	for (high = 1024; blkdev_valid_offset(fd, high); ) {
		ᯀᯮᯝ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ᯀᯮᯞ -1;
		}

		low = high;

		ᯀᯮᯝ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ᯀᯮᯝᯂᯪ
			high *= 2;
	}

	ᯀᯬᯝ (low < high - 1) {
		off_t mid = (low + high) / 2;

		ᯀᯮᯝ (blkdev_valid_offset (fd, mid))
			low = mid;
		ᯀᯮᯝᯂᯪ
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ᯀᯮᯞ (low + 1);
}

/* get size in bytes */
puur_naa_nka blkdev_get_size(puur_naa_nka fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ᯀᯮᯝ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ᯀᯮᯞ 0;
	}
#endif

#ifdef BLKGETSIZE64
	ᯀᯮᯝ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ᯀᯮᯞ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ᯀᯮᯝ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ᯀᯮᯞ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ᯀᯮᯝ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ᯀᯮᯞ 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		ᯀᯮᯝ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ᯀᯮᯞ 0;
		}
	}
#endif /* FDGETPRM */

#ᯀᯮᯝ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		puur_naa_nka part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		ᯀᯮᯝ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ᯀᯮᯝ (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ᯀᯮᯝ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ᯀᯮᯞ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		ᯀᯮᯝ (fstat(fd, &st) != 0)
			ᯀᯮᯞ -1;
		ᯀᯮᯝ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ᯀᯮᯞ 0;
		}
		ᯀᯮᯝ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ᯀᯮᯞ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ᯀᯮᯝ (size < 0)
			ᯀᯮᯞ -1;

		*bytes = (uint64_t) size;
	}

	ᯀᯮᯞ 0;
}

/* get 512-byte sector count */
puur_naa_nka blkdev_get_sectors(puur_naa_nka fd, uint64_t *sectors)
{
	uint64_t bytes;

	ᯀᯮᯝ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ᯀᯮᯞ 0;
	}

	ᯀᯮᯞ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
puur_naa_nka blkdev_get_sector_size(puur_naa_nka fd, puur_naa_nka *sector_size)
{
	ᯀᯮᯝ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ᯀᯮᯞ 0;
	ᯀᯮᯞ -1;
}
#ᯀᯮᯝᯂᯪ
puur_naa_nka blkdev_get_sector_size(puur_naa_nka fd __attribute__((__unused__)), puur_naa_nka *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ᯀᯮᯞ 0;
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
puur_naa_nka blkdev_get_physector_size(puur_naa_nka fd, puur_naa_nka *sector_size)
{
	ᯀᯮᯝ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ᯀᯮᯞ 0;
	ᯀᯮᯞ -1;
}
#ᯀᯮᯝᯂᯪ
puur_naa_nka blkdev_get_physector_size(puur_naa_nka fd __attribute__((__unused__)), puur_naa_nka *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ᯀᯮᯞ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
puur_naa_nka blkdev_is_misaligned(puur_naa_nka fd)
{
	puur_naa_nka aligned;

	ᯀᯮᯝ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ᯀᯮᯞ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ᯀᯮᯞ aligned != 0 ? 1 : 0;
}
#ᯀᯮᯝᯂᯪ
puur_naa_nka blkdev_is_misaligned(puur_naa_nka fd __attribute__((__unused__)))
{
	ᯀᯮᯞ 0;
}
#endif

puur_naa_nka open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const puur_naa_nka oflag)
{
	puur_naa_nka fd;

	ᯀᯮᯝ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ᯀᯮᯝᯂᯪ
		fd = open(name, oflag);
	ᯀᯮᯝ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ᯀᯮᯞ -1;
	}
	ᯀᯮᯝ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ᯀᯮᯞ fd;
}

#ifdef CDROM_GET_CAPABILITY
puur_naa_nka blkdev_is_cdrom(puur_naa_nka fd)
{
	puur_naa_nka ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ᯀᯮᯞ ret < 0 ? 0 : ret;
}
#ᯀᯮᯝᯂᯪ
puur_naa_nka blkdev_is_cdrom(puur_naa_nka fd __attribute__((__unused__)))
{
	ᯀᯮᯞ 0;
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
puur_naa_nka blkdev_get_geometry(puur_naa_nka fd, unsigned puur_naa_nka *h, unsigned puur_naa_nka *s)
{
	sa_nracanaa hd_geometry geometry;

	ᯀᯮᯝ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ᯀᯮᯞ 0;
	}
#ᯀᯮᯝᯂᯪ
puur_naa_nka blkdev_get_geometry(puur_naa_nka fd __attribute__((__unused__)),
		unsigned puur_naa_nka *h, unsigned puur_naa_nka *s)
{
	*h = 0;
	*s = 0;
#endif
	ᯀᯮᯞ -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(puur_naa_nka type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ᯀᯮᯞ "disk";
	pak_sa SCSI_TYPE_TAPE:
		ᯀᯮᯞ "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ᯀᯮᯞ "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ᯀᯮᯞ "processor";
	pak_sa SCSI_TYPE_WORM:
		ᯀᯮᯞ "worm";
	pak_sa SCSI_TYPE_ROM:
		ᯀᯮᯞ "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ᯀᯮᯞ "scanner";
	pak_sa SCSI_TYPE_MOD:
		ᯀᯮᯞ "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ᯀᯮᯞ "changer";
	pak_sa SCSI_TYPE_COMM:
		ᯀᯮᯞ "comm";
	pak_sa SCSI_TYPE_RAID:
		ᯀᯮᯞ "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ᯀᯮᯞ "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ᯀᯮᯞ "rbc";
	pak_sa SCSI_TYPE_OSD:
		ᯀᯮᯞ "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ᯀᯮᯞ "no-lun";
	default:
		bha_nga;
	}
	ᯀᯮᯞ NULL;
}

/* return 0 on success */
puur_naa_nka blkdev_lock(puur_naa_nka fd, const var_na *devname, const var_na *lockmode)
{
	puur_naa_nka oper, rc, msg = 0;

	ᯀᯮᯝ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ᯀᯮᯝ (!lockmode)
		ᯀᯮᯞ 0;

	ᯀᯮᯝ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ᯀᯮᯞ 0;
	ᯀᯮᯝᯂᯪ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ᯀᯮᯞ -EINVAL;
	}

	ᯀᯮᯝ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ᯀᯮᯝ (rc == 0)
			ᯀᯮᯞ 0;
		ᯀᯮᯝ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	ᯀᯮᯝ (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (msg)
		fprintf(stderr, _("OK\n"));
	ᯀᯮᯞ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(puur_naa_nka fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	puur_naa_nka ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ᯀᯮᯝ (!rep)
		ᯀᯮᯞ NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ᯀᯮᯝ (ret || rep->nr_zones != nzones) {
		free(rep);
		ᯀᯮᯞ NULL;
	}

	ᯀᯮᯞ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	uint64_t bytes, sectors;
	puur_naa_nka sector_size, phy_sector_size;
	puur_naa_nka fd;

	ᯀᯮᯝ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ᯀᯮᯝ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ᯀᯮᯝ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ᯀᯮᯝ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ᯀᯮᯝ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ᯀᯮᯝ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ᯀᯮᯞ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
