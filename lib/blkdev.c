/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/file.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#அந்தர்பாவ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#அந்தர்பாவ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#அந்தர்பாவ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "blkdev.h"
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "linux_version.h"
#அந்தர்பாவ "fileutils.h"
#அந்தர்பாவ "nls.h"

நிலையான long blkdev_valid_offset(முழு fd, off_t offset)
{
	எழுத்து ch;

	எனில் (lseek (fd, offset, 0) < 0)
		திரும்பு 0;
	எனில் (ul_read_all (fd, &ch, 1) < 1)
		திரும்பு 0;
	திரும்பு 1;
}

முழு is_blkdev(முழு fd)
{
	கட்டமைப்பு stat st;
	திரும்பு (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (முழு fd)
{
	off_t high, கீழ் = 0;

	க்கு (high = 1024; blkdev_valid_offset(fd, high); ) {
		எனில் (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			திரும்பு -1;
		}

		கீழ் = high;

		எனில் (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		இல்லையெனில்
			high *= 2;
	}

	போது (கீழ் < high - 1) {
		off_t mid = (கீழ் + high) / 2;

		எனில் (blkdev_valid_offset (fd, mid))
			கீழ் = mid;
		இல்லையெனில்
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	திரும்பு (கீழ் + 1);
}

/* get size in bytes */
முழு blkdev_get_size(முழு fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	எனில் (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		திரும்பு 0;
	}
#endif

#ifdef BLKGETSIZE64
	எனில் (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		திரும்பு 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		எனில் (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			திரும்பு 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	எனில் (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		திரும்பு 0;
#endif

#ifdef FDGETPRM
	{
		கட்டமைப்பு floppy_struct this_floppy;

		எனில் (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			திரும்பு 0;
		}
	}
#endif /* FDGETPRM */

#எனில் defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		முழு part = -1;
		கட்டமைப்பு disklabel lab;
		கட்டமைப்பு partition *pp;
		கட்டமைப்பு stat st;

		எனில் ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		எனில் (part >= 0 && (ioctl(fd, DIOCGDINFO, (எழுத்து *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			எனில் (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 திரும்பு 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		கட்டமைப்பு stat st;

		எனில் (fstat(fd, &st) != 0)
			திரும்பு -1;
		எனில் (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			திரும்பு 0;
		}
		எனில் (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			திரும்பு -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		எனில் (size < 0)
			திரும்பு -1;

		*bytes = (uint64_t) size;
	}

	திரும்பு 0;
}

/* get 512-byte sector count */
முழு blkdev_get_sectors(முழு fd, uint64_t *sectors)
{
	uint64_t bytes;

	எனில் (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		திரும்பு 0;
	}

	திரும்பு -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
முழு blkdev_get_sector_size(முழு fd, முழு *sector_size)
{
	எனில் (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		திரும்பு 0;
	திரும்பு -1;
}
#இல்லையெனில்
முழு blkdev_get_sector_size(முழு fd __attribute__((__unused__)), முழு *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	திரும்பு 0;
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
முழு blkdev_get_physector_size(முழு fd, முழு *sector_size)
{
	எனில் (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		திரும்பு 0;
	திரும்பு -1;
}
#இல்லையெனில்
முழு blkdev_get_physector_size(முழு fd __attribute__((__unused__)), முழு *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	திரும்பு 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
முழு blkdev_is_misaligned(முழு fd)
{
	முழு aligned;

	எனில் (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		திரும்பு 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	திரும்பு aligned != 0 ? 1 : 0;
}
#இல்லையெனில்
முழு blkdev_is_misaligned(முழு fd __attribute__((__unused__)))
{
	திரும்பு 0;
}
#endif

முழு open_blkdev_or_file(மாறிலி கட்டமைப்பு stat *st, மாறிலி எழுத்து *name, மாறிலி முழு oflag)
{
	முழு fd;

	எனில் (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} இல்லையெனில்
		fd = open(name, oflag);
	எனில் (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		திரும்பு -1;
	}
	எனில் (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	திரும்பு fd;
}

#ifdef CDROM_GET_CAPABILITY
முழு blkdev_is_cdrom(முழு fd)
{
	முழு ret = ioctl(fd, CDROM_GET_CAPABILITY, வெற்று);
	திரும்பு ret < 0 ? 0 : ret;
}
#இல்லையெனில்
முழு blkdev_is_cdrom(முழு fd __attribute__((__unused__)))
{
	திரும்பு 0;
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
முழு blkdev_get_geometry(முழு fd, unsigned முழு *h, unsigned முழு *s)
{
	கட்டமைப்பு hd_geometry geometry;

	எனில் (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		திரும்பு 0;
	}
#இல்லையெனில்
முழு blkdev_get_geometry(முழு fd __attribute__((__unused__)),
		unsigned முழு *h, unsigned முழு *s)
{
	*h = 0;
	*s = 0;
#endif
	திரும்பு -1;
}

/*
 * Convert scsi type to human readable string.
 */
மாறிலி எழுத்து *blkdev_scsi_type_to_name(முழு type)
{
	தேர்வு (type) {
	நிலை SCSI_TYPE_DISK:
		திரும்பு "disk";
	நிலை SCSI_TYPE_TAPE:
		திரும்பு "tape";
	நிலை SCSI_TYPE_PRINTER:
		திரும்பு "printer";
	நிலை SCSI_TYPE_PROCESSOR:
		திரும்பு "processor";
	நிலை SCSI_TYPE_WORM:
		திரும்பு "worm";
	நிலை SCSI_TYPE_ROM:
		திரும்பு "rom";
	நிலை SCSI_TYPE_SCANNER:
		திரும்பு "scanner";
	நிலை SCSI_TYPE_MOD:
		திரும்பு "mo-disk";
	நிலை SCSI_TYPE_MEDIUM_CHANGER:
		திரும்பு "changer";
	நிலை SCSI_TYPE_COMM:
		திரும்பு "comm";
	நிலை SCSI_TYPE_RAID:
		திரும்பு "raid";
	நிலை SCSI_TYPE_ENCLOSURE:
		திரும்பு "enclosure";
	நிலை SCSI_TYPE_RBC:
		திரும்பு "rbc";
	நிலை SCSI_TYPE_OSD:
		திரும்பு "osd";
	நிலை SCSI_TYPE_NO_LUN:
		திரும்பு "no-lun";
	இயல்பு:
		நிறுத்து;
	}
	திரும்பு வெற்று;
}

/* return 0 on success */
முழு blkdev_lock(முழு fd, மாறிலி எழுத்து *devname, மாறிலி எழுத்து *lockmode)
{
	முழு oper, rc, செய்தி = 0;

	எனில் (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	எனில் (!lockmode)
		திரும்பு 0;

	எனில் (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	இல்லையெனில் எனில் (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	இல்லையெனில் எனில் (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		திரும்பு 0;
	இல்லையெனில் {
		warnx(_("unsupported lock mode: %s"), lockmode);
		திரும்பு -EINVAL;
	}

	எனில் (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		எனில் (rc == 0)
			திரும்பு 0;
		எனில் (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			செய்தி = 1;
		}
	}
	rc = flock(fd, oper);
	எனில் (rc != 0) {
		தேர்வு (errno) {
		நிலை EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			நிறுத்து;
		இயல்பு:
			warn(_("%s: failed to get lock"), devname);
		}
	} இல்லையெனில் எனில் (செய்தி)
		fprintf(stderr, _("OK\n"));
	திரும்பு rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
கட்டமைப்பு blk_zone_report *blkdev_get_zonereport(முழு fd, uint64_t sector, uint32_t nzones)
{
	கட்டமைப்பு blk_zone_report *rep;
	size_t rep_size;
	முழு ret;

	rep_size = அளவு(கட்டமைப்பு blk_zone_report) + அளவு(கட்டமைப்பு blk_zone) * nzones;
	rep = calloc(1, rep_size);
	எனில் (!rep)
		திரும்பு வெற்று;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	எனில் (ret || rep->nr_zones != nzones) {
		விடுவி(rep);
		திரும்பு வெற்று;
	}

	திரும்பு rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <fcntl.h>

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	uint64_t bytes, sectors;
	முழு sector_size, phy_sector_size;
	முழு fd;

	எனில் (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	எனில் ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	எனில் (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	எனில் (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	எனில் (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	எனில் (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
