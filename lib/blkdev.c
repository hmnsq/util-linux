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

static long blkdev_valid_offset(ကိန်းပြည့် fd, off_t offset)
{
	var_na ch;

	အကယ်၍ (lseek (fd, offset, 0) < 0)
		ပြန်ပေး 0;
	အကယ်၍ (ul_read_all (fd, &ch, 1) < 1)
		ပြန်ပေး 0;
	ပြန်ပေး 1;
}

ကိန်းပြည့် is_blkdev(ကိန်းပြည့် fd)
{
	sa_nracanaa stat st;
	ပြန်ပေး (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ကိန်းပြည့် fd)
{
	off_t high, low = 0;

	အတွက် (high = 1024; blkdev_valid_offset(fd, high); ) {
		အကယ်၍ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ပြန်ပေး -1;
		}

		low = high;

		အကယ်၍ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		မဟုတ်ပါက
			high *= 2;
	}

	နေစဉ် (low < high - 1) {
		off_t mid = (low + high) / 2;

		အကယ်၍ (blkdev_valid_offset (fd, mid))
			low = mid;
		မဟုတ်ပါက
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ပြန်ပေး (low + 1);
}

/* get size in bytes */
ကိန်းပြည့် blkdev_get_size(ကိန်းပြည့် fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	အကယ်၍ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ပြန်ပေး 0;
	}
#endif

#ifdef BLKGETSIZE64
	အကယ်၍ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ပြန်ပေး 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		အကယ်၍ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ပြန်ပေး 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	အကယ်၍ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ပြန်ပေး 0;
#endif

#ifdef FDGETPRM
	{
		sa_nracanaa floppy_struct this_floppy;

		အကယ်၍ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ပြန်ပေး 0;
		}
	}
#endif /* FDGETPRM */

#အကယ်၍ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ကိန်းပြည့် part = -1;
		sa_nracanaa disklabel lab;
		sa_nracanaa partition *pp;
		sa_nracanaa stat st;

		အကယ်၍ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		အကယ်၍ (part >= 0 && (ioctl(fd, DIOCGDINFO, (var_na *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			အကယ်၍ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ပြန်ပေး 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		sa_nracanaa stat st;

		အကယ်၍ (fstat(fd, &st) != 0)
			ပြန်ပေး -1;
		အကယ်၍ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ပြန်ပေး 0;
		}
		အကယ်၍ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ပြန်ပေး -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		အကယ်၍ (size < 0)
			ပြန်ပေး -1;

		*bytes = (uint64_t) size;
	}

	ပြန်ပေး 0;
}

/* get 512-byte sector count */
ကိန်းပြည့် blkdev_get_sectors(ကိန်းပြည့် fd, uint64_t *sectors)
{
	uint64_t bytes;

	အကယ်၍ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ပြန်ပေး 0;
	}

	ပြန်ပေး -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ကိန်းပြည့် blkdev_get_sector_size(ကိန်းပြည့် fd, ကိန်းပြည့် *sector_size)
{
	အကယ်၍ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ပြန်ပေး 0;
	ပြန်ပေး -1;
}
#မဟုတ်ပါက
ကိန်းပြည့် blkdev_get_sector_size(ကိန်းပြည့် fd __attribute__((__unused__)), ကိန်းပြည့် *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ပြန်ပေး 0;
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
ကိန်းပြည့် blkdev_get_physector_size(ကိန်းပြည့် fd, ကိန်းပြည့် *sector_size)
{
	အကယ်၍ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ပြန်ပေး 0;
	ပြန်ပေး -1;
}
#မဟုတ်ပါက
ကိန်းပြည့် blkdev_get_physector_size(ကိန်းပြည့် fd __attribute__((__unused__)), ကိန်းပြည့် *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ပြန်ပေး 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ကိန်းပြည့် blkdev_is_misaligned(ကိန်းပြည့် fd)
{
	ကိန်းပြည့် aligned;

	အကယ်၍ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ပြန်ပေး 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ပြန်ပေး aligned != 0 ? 1 : 0;
}
#မဟုတ်ပါက
ကိန်းပြည့် blkdev_is_misaligned(ကိန်းပြည့် fd __attribute__((__unused__)))
{
	ပြန်ပေး 0;
}
#endif

ကိန်းပြည့် open_blkdev_or_file(const sa_nracanaa stat *st, const var_na *name, const ကိန်းပြည့် oflag)
{
	ကိန်းပြည့် fd;

	အကယ်၍ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} မဟုတ်ပါက
		fd = open(name, oflag);
	အကယ်၍ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ပြန်ပေး -1;
	}
	အကယ်၍ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ပြန်ပေး fd;
}

#ifdef CDROM_GET_CAPABILITY
ကိန်းပြည့် blkdev_is_cdrom(ကိန်းပြည့် fd)
{
	ကိန်းပြည့် ret = ioctl(fd, CDROM_GET_CAPABILITY, NULL);
	ပြန်ပေး ret < 0 ? 0 : ret;
}
#မဟုတ်ပါက
ကိန်းပြည့် blkdev_is_cdrom(ကိန်းပြည့် fd __attribute__((__unused__)))
{
	ပြန်ပေး 0;
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
ကိန်းပြည့် blkdev_get_geometry(ကိန်းပြည့် fd, unsigned ကိန်းပြည့် *h, unsigned ကိန်းပြည့် *s)
{
	sa_nracanaa hd_geometry geometry;

	အကယ်၍ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ပြန်ပေး 0;
	}
#မဟုတ်ပါက
ကိန်းပြည့် blkdev_get_geometry(ကိန်းပြည့် fd __attribute__((__unused__)),
		unsigned ကိန်းပြည့် *h, unsigned ကိန်းပြည့် *s)
{
	*h = 0;
	*s = 0;
#endif
	ပြန်ပေး -1;
}

/*
 * Convert scsi type to human readable string.
 */
const var_na *blkdev_scsi_type_to_name(ကိန်းပြည့် type)
{
	switch (type) {
	pak_sa SCSI_TYPE_DISK:
		ပြန်ပေး "disk";
	pak_sa SCSI_TYPE_TAPE:
		ပြန်ပေး "tape";
	pak_sa SCSI_TYPE_PRINTER:
		ပြန်ပေး "printer";
	pak_sa SCSI_TYPE_PROCESSOR:
		ပြန်ပေး "processor";
	pak_sa SCSI_TYPE_WORM:
		ပြန်ပေး "worm";
	pak_sa SCSI_TYPE_ROM:
		ပြန်ပေး "rom";
	pak_sa SCSI_TYPE_SCANNER:
		ပြန်ပေး "scanner";
	pak_sa SCSI_TYPE_MOD:
		ပြန်ပေး "mo-disk";
	pak_sa SCSI_TYPE_MEDIUM_CHANGER:
		ပြန်ပေး "changer";
	pak_sa SCSI_TYPE_COMM:
		ပြန်ပေး "comm";
	pak_sa SCSI_TYPE_RAID:
		ပြန်ပေး "raid";
	pak_sa SCSI_TYPE_ENCLOSURE:
		ပြန်ပေး "enclosure";
	pak_sa SCSI_TYPE_RBC:
		ပြန်ပေး "rbc";
	pak_sa SCSI_TYPE_OSD:
		ပြန်ပေး "osd";
	pak_sa SCSI_TYPE_NO_LUN:
		ပြန်ပေး "no-lun";
	default:
		bha_nga;
	}
	ပြန်ပေး NULL;
}

/* return 0 on success */
ကိန်းပြည့် blkdev_lock(ကိန်းပြည့် fd, const var_na *devname, const var_na *lockmode)
{
	ကိန်းပြည့် oper, rc, msg = 0;

	အကယ်၍ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	အကယ်၍ (!lockmode)
		ပြန်ပေး 0;

	အကယ်၍ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	မဟုတ်ပါက အကယ်၍ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	မဟုတ်ပါက အကယ်၍ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ပြန်ပေး 0;
	မဟုတ်ပါက {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ပြန်ပေး -EINVAL;
	}

	အကယ်၍ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		အကယ်၍ (rc == 0)
			ပြန်ပေး 0;
		အကယ်၍ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			msg = 1;
		}
	}
	rc = flock(fd, oper);
	အကယ်၍ (rc != 0) {
		switch (errno) {
		pak_sa EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			bha_nga;
		default:
			warn(_("%s: failed to get lock"), devname);
		}
	} မဟုတ်ပါက အကယ်၍ (msg)
		fprintf(stderr, _("OK\n"));
	ပြန်ပေး rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
sa_nracanaa blk_zone_report *blkdev_get_zonereport(ကိန်းပြည့် fd, uint64_t sector, uint32_t nzones)
{
	sa_nracanaa blk_zone_report *rep;
	size_t rep_size;
	ကိန်းပြည့် ret;

	rep_size = parimaa_na(sa_nracanaa blk_zone_report) + parimaa_na(sa_nracanaa blk_zone) * nzones;
	rep = calloc(1, rep_size);
	အကယ်၍ (!rep)
		ပြန်ပေး NULL;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	အကယ်၍ (ret || rep->nr_zones != nzones) {
		free(rep);
		ပြန်ပေး NULL;
	}

	ပြန်ပေး rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	uint64_t bytes, sectors;
	ကိန်းပြည့် sector_size, phy_sector_size;
	ကိန်းပြည့် fd;

	အကယ်၍ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	အကယ်၍ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	အကယ်၍ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	အကယ်၍ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	အကယ်၍ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	အကယ်၍ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ပြန်ပေး EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
