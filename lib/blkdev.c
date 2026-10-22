/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/file.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#ଅନ୍ତର୍ଭାଵ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#ଅନ୍ତର୍ଭାଵ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#ଅନ୍ତର୍ଭାଵ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "blkdev.h"
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "linux_version.h"
#ଅନ୍ତର୍ଭାଵ "fileutils.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"

ସ୍ଥିର long blkdev_valid_offset(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t offset)
{
	ଅକ୍ଷର ch;

	ଯଦି (lseek (fd, offset, 0) < 0)
		ଫେରନ୍ତୁ 0;
	ଯଦି (ul_read_all (fd, &ch, 1) < 1)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_blkdev(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ସଂରଚନା stat st;
	ଫେରନ୍ତୁ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	off_t high, ତଳ = 0;

	ପାଇଁ (high = 1024; blkdev_valid_offset(fd, high); ) {
		ଯଦି (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			ଫେରନ୍ତୁ -1;
		}

		ତଳ = high;

		ଯଦି (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		ଅନ୍ୟଥା
			high *= 2;
	}

	ଯେତେବେଳେ (ତଳ < high - 1) {
		off_t mid = (ତଳ + high) / 2;

		ଯଦି (blkdev_valid_offset (fd, mid))
			ତଳ = mid;
		ଅନ୍ୟଥା
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	ଫେରନ୍ତୁ (ତଳ + 1);
}

/* get size in bytes */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ଯଦି (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		ଫେରନ୍ତୁ 0;
	}
#endif

#ifdef BLKGETSIZE64
	ଯଦି (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		ଫେରନ୍ତୁ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		ଯଦି (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			ଫେରନ୍ତୁ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	ଯଦି (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		ଫେରନ୍ତୁ 0;
#endif

#ifdef FDGETPRM
	{
		ସଂରଚନା floppy_struct this_floppy;

		ଯଦି (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			ଫେରନ୍ତୁ 0;
		}
	}
#endif /* FDGETPRM */

#ଯଦି defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା part = -1;
		ସଂରଚନା disklabel lab;
		ସଂରଚନା partition *pp;
		ସଂରଚନା stat st;

		ଯଦି ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ଯଦି (part >= 0 && (ioctl(fd, DIOCGDINFO, (ଅକ୍ଷର *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ଯଦି (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 ଫେରନ୍ତୁ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ସଂରଚନା stat st;

		ଯଦି (fstat(fd, &st) != 0)
			ଫେରନ୍ତୁ -1;
		ଯଦି (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			ଫେରନ୍ତୁ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		ଯଦି (size < 0)
			ଫେରନ୍ତୁ -1;

		*bytes = (uint64_t) size;
	}

	ଫେରନ୍ତୁ 0;
}

/* get 512-byte sector count */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_sectors(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, uint64_t *sectors)
{
	uint64_t bytes;

	ଯଦି (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_sector_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sector_size)
{
	ଯଦି (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ -1;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_sector_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)), ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_physector_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sector_size)
{
	ଯଦି (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ -1;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_physector_size(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)), ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	ଫେରନ୍ତୁ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_is_misaligned(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା aligned;

	ଯଦି (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		ଫେରନ୍ତୁ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	ଫେରନ୍ତୁ aligned != 0 ? 1 : 0;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_is_misaligned(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)))
{
	ଫେରନ୍ତୁ 0;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_blkdev_or_file(ଅଚଳ ସଂରଚନା stat *st, ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା oflag)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	ଯଦି (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} ଅନ୍ୟଥା
		fd = open(name, oflag);
	ଯଦି (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	ଫେରନ୍ତୁ fd;
}

#ifdef CDROM_GET_CAPABILITY
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_is_cdrom(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = ioctl(fd, CDROM_GET_CAPABILITY, ଖାଲି);
	ଫେରନ୍ତୁ ret < 0 ? 0 : ret;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_is_cdrom(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)))
{
	ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_geometry(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *h, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *s)
{
	ସଂରଚନା hd_geometry geometry;

	ଯଦି (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		ଫେରନ୍ତୁ 0;
	}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_get_geometry(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd __attribute__((__unused__)),
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *h, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *s)
{
	*h = 0;
	*s = 0;
#endif
	ଫେରନ୍ତୁ -1;
}

/*
 * Convert scsi type to human readable string.
 */
ଅଚଳ ଅକ୍ଷର *blkdev_scsi_type_to_name(ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
	ବାଛ (type) {
	ସ୍ଥିତି SCSI_TYPE_DISK:
		ଫେରନ୍ତୁ "disk";
	ସ୍ଥିତି SCSI_TYPE_TAPE:
		ଫେରନ୍ତୁ "tape";
	ସ୍ଥିତି SCSI_TYPE_PRINTER:
		ଫେରନ୍ତୁ "printer";
	ସ୍ଥିତି SCSI_TYPE_PROCESSOR:
		ଫେରନ୍ତୁ "processor";
	ସ୍ଥିତି SCSI_TYPE_WORM:
		ଫେରନ୍ତୁ "worm";
	ସ୍ଥିତି SCSI_TYPE_ROM:
		ଫେରନ୍ତୁ "rom";
	ସ୍ଥିତି SCSI_TYPE_SCANNER:
		ଫେରନ୍ତୁ "scanner";
	ସ୍ଥିତି SCSI_TYPE_MOD:
		ଫେରନ୍ତୁ "mo-disk";
	ସ୍ଥିତି SCSI_TYPE_MEDIUM_CHANGER:
		ଫେରନ୍ତୁ "changer";
	ସ୍ଥିତି SCSI_TYPE_COMM:
		ଫେରନ୍ତୁ "comm";
	ସ୍ଥିତି SCSI_TYPE_RAID:
		ଫେରନ୍ତୁ "raid";
	ସ୍ଥିତି SCSI_TYPE_ENCLOSURE:
		ଫେରନ୍ତୁ "enclosure";
	ସ୍ଥିତି SCSI_TYPE_RBC:
		ଫେରନ୍ତୁ "rbc";
	ସ୍ଥିତି SCSI_TYPE_OSD:
		ଫେରନ୍ତୁ "osd";
	ସ୍ଥିତି SCSI_TYPE_NO_LUN:
		ଫେରନ୍ତୁ "no-lun";
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

/* return 0 on success */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା blkdev_lock(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *devname, ଅଚଳ ଅକ୍ଷର *lockmode)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା oper, rc, ସନ୍ଦେଶ = 0;

	ଯଦି (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ଯଦି (!lockmode)
		ଫେରନ୍ତୁ 0;

	ଯଦି (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	ଅନ୍ୟଥା ଯଦି (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	ଅନ୍ୟଥା ଯଦି (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		ଫେରନ୍ତୁ 0;
	ଅନ୍ୟଥା {
		warnx(_("unsupported lock mode: %s"), lockmode);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଯଦି (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ଯଦି (rc == 0)
			ଫେରନ୍ତୁ 0;
		ଯଦି (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			ସନ୍ଦେଶ = 1;
		}
	}
	rc = flock(fd, oper);
	ଯଦି (rc != 0) {
		ବାଛ (errno) {
		ସ୍ଥିତି EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			warn(_("%s: failed to get lock"), devname);
		}
	} ଅନ୍ୟଥା ଯଦି (ସନ୍ଦେଶ)
		fprintf(stderr, _("OK\n"));
	ଫେରନ୍ତୁ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
ସଂରଚନା blk_zone_report *blkdev_get_zonereport(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, uint64_t sector, uint32_t nzones)
{
	ସଂରଚନା blk_zone_report *rep;
	size_t rep_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	rep_size = ଆକାର(ସଂରଚନା blk_zone_report) + ଆକାର(ସଂରଚନା blk_zone) * nzones;
	rep = calloc(1, rep_size);
	ଯଦି (!rep)
		ଫେରନ୍ତୁ ଖାଲି;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ଯଦି (ret || rep->nr_zones != nzones) {
		ମୁକ୍ତ(rep);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଫେରନ୍ତୁ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	uint64_t bytes, sectors;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sector_size, phy_sector_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	ଯଦି (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ଯଦି ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ଯଦି (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ଯଦି (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ଯଦି (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ଯଦି (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
