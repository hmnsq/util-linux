/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#અન્તર્ભાવ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#અન્તર્ભાવ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#અન્તર્ભાવ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "blkdev.h"
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "linux_version.h"
#અન્તર્ભાવ "fileutils.h"
#અન્તર્ભાવ "nls.h"

સ્થિર long blkdev_valid_offset(પૂર્ણાંક fd, off_t offset)
{
	અક્ષર ch;

	જો (lseek (fd, offset, 0) < 0)
		પાછા 0;
	જો (ul_read_all (fd, &ch, 1) < 1)
		પાછા 0;
	પાછા 1;
}

પૂર્ણાંક is_blkdev(પૂર્ણાંક fd)
{
	સંરચના stat st;
	પાછા (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (પૂર્ણાંક fd)
{
	off_t high, નીચલું = 0;

	માટે (high = 1024; blkdev_valid_offset(fd, high); ) {
		જો (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			પાછા -1;
		}

		નીચલું = high;

		જો (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		નહીંતર
			high *= 2;
	}

	જ્યારે (નીચલું < high - 1) {
		off_t mid = (નીચલું + high) / 2;

		જો (blkdev_valid_offset (fd, mid))
			નીચલું = mid;
		નહીંતર
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	પાછા (નીચલું + 1);
}

/* get size in bytes */
પૂર્ણાંક blkdev_get_size(પૂર્ણાંક fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	જો (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		પાછા 0;
	}
#endif

#ifdef BLKGETSIZE64
	જો (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		પાછા 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		જો (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			પાછા 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	જો (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		પાછા 0;
#endif

#ifdef FDGETPRM
	{
		સંરચના floppy_struct this_floppy;

		જો (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			પાછા 0;
		}
	}
#endif /* FDGETPRM */

#જો defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		પૂર્ણાંક part = -1;
		સંરચના disklabel lab;
		સંરચના partition *pp;
		સંરચના stat st;

		જો ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		જો (part >= 0 && (ioctl(fd, DIOCGDINFO, (અક્ષર *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			જો (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 પાછા 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		સંરચના stat st;

		જો (fstat(fd, &st) != 0)
			પાછા -1;
		જો (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			પાછા 0;
		}
		જો (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			પાછા -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		જો (size < 0)
			પાછા -1;

		*bytes = (uint64_t) size;
	}

	પાછા 0;
}

/* get 512-byte sector count */
પૂર્ણાંક blkdev_get_sectors(પૂર્ણાંક fd, uint64_t *sectors)
{
	uint64_t bytes;

	જો (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		પાછા 0;
	}

	પાછા -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
પૂર્ણાંક blkdev_get_sector_size(પૂર્ણાંક fd, પૂર્ણાંક *sector_size)
{
	જો (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		પાછા 0;
	પાછા -1;
}
#નહીંતર
પૂર્ણાંક blkdev_get_sector_size(પૂર્ણાંક fd __attribute__((__unused__)), પૂર્ણાંક *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	પાછા 0;
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
પૂર્ણાંક blkdev_get_physector_size(પૂર્ણાંક fd, પૂર્ણાંક *sector_size)
{
	જો (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		પાછા 0;
	પાછા -1;
}
#નહીંતર
પૂર્ણાંક blkdev_get_physector_size(પૂર્ણાંક fd __attribute__((__unused__)), પૂર્ણાંક *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	પાછા 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
પૂર્ણાંક blkdev_is_misaligned(પૂર્ણાંક fd)
{
	પૂર્ણાંક aligned;

	જો (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		પાછા 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	પાછા aligned != 0 ? 1 : 0;
}
#નહીંતર
પૂર્ણાંક blkdev_is_misaligned(પૂર્ણાંક fd __attribute__((__unused__)))
{
	પાછા 0;
}
#endif

પૂર્ણાંક open_blkdev_or_file(અચલ સંરચના stat *st, અચલ અક્ષર *name, અચલ પૂર્ણાંક oflag)
{
	પૂર્ણાંક fd;

	જો (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} નહીંતર
		fd = open(name, oflag);
	જો (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		પાછા -1;
	}
	જો (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	પાછા fd;
}

#ifdef CDROM_GET_CAPABILITY
પૂર્ણાંક blkdev_is_cdrom(પૂર્ણાંક fd)
{
	પૂર્ણાંક ret = ioctl(fd, CDROM_GET_CAPABILITY, ખાલી);
	પાછા ret < 0 ? 0 : ret;
}
#નહીંતર
પૂર્ણાંક blkdev_is_cdrom(પૂર્ણાંક fd __attribute__((__unused__)))
{
	પાછા 0;
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
પૂર્ણાંક blkdev_get_geometry(પૂર્ણાંક fd, unsigned પૂર્ણાંક *h, unsigned પૂર્ણાંક *s)
{
	સંરચના hd_geometry geometry;

	જો (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		પાછા 0;
	}
#નહીંતર
પૂર્ણાંક blkdev_get_geometry(પૂર્ણાંક fd __attribute__((__unused__)),
		unsigned પૂર્ણાંક *h, unsigned પૂર્ણાંક *s)
{
	*h = 0;
	*s = 0;
#endif
	પાછા -1;
}

/*
 * Convert scsi type to human readable string.
 */
અચલ અક્ષર *blkdev_scsi_type_to_name(પૂર્ણાંક type)
{
	પસંદગી (type) {
	સ્થિતિ SCSI_TYPE_DISK:
		પાછા "disk";
	સ્થિતિ SCSI_TYPE_TAPE:
		પાછા "tape";
	સ્થિતિ SCSI_TYPE_PRINTER:
		પાછા "printer";
	સ્થિતિ SCSI_TYPE_PROCESSOR:
		પાછા "processor";
	સ્થિતિ SCSI_TYPE_WORM:
		પાછા "worm";
	સ્થિતિ SCSI_TYPE_ROM:
		પાછા "rom";
	સ્થિતિ SCSI_TYPE_SCANNER:
		પાછા "scanner";
	સ્થિતિ SCSI_TYPE_MOD:
		પાછા "mo-disk";
	સ્થિતિ SCSI_TYPE_MEDIUM_CHANGER:
		પાછા "changer";
	સ્થિતિ SCSI_TYPE_COMM:
		પાછા "comm";
	સ્થિતિ SCSI_TYPE_RAID:
		પાછા "raid";
	સ્થિતિ SCSI_TYPE_ENCLOSURE:
		પાછા "enclosure";
	સ્થિતિ SCSI_TYPE_RBC:
		પાછા "rbc";
	સ્થિતિ SCSI_TYPE_OSD:
		પાછા "osd";
	સ્થિતિ SCSI_TYPE_NO_LUN:
		પાછા "no-lun";
	મૂળભૂત:
		તોડો;
	}
	પાછા ખાલી;
}

/* return 0 on success */
પૂર્ણાંક blkdev_lock(પૂર્ણાંક fd, અચલ અક્ષર *devname, અચલ અક્ષર *lockmode)
{
	પૂર્ણાંક oper, rc, સંદેશ = 0;

	જો (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	જો (!lockmode)
		પાછા 0;

	જો (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	નહીંતર જો (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	નહીંતર જો (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		પાછા 0;
	નહીંતર {
		warnx(_("unsupported lock mode: %s"), lockmode);
		પાછા -EINVAL;
	}

	જો (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		જો (rc == 0)
			પાછા 0;
		જો (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			સંદેશ = 1;
		}
	}
	rc = flock(fd, oper);
	જો (rc != 0) {
		પસંદગી (errno) {
		સ્થિતિ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			તોડો;
		મૂળભૂત:
			warn(_("%s: failed to get lock"), devname);
		}
	} નહીંતર જો (સંદેશ)
		fprintf(stderr, _("OK\n"));
	પાછા rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
સંરચના blk_zone_report *blkdev_get_zonereport(પૂર્ણાંક fd, uint64_t sector, uint32_t nzones)
{
	સંરચના blk_zone_report *rep;
	size_t rep_size;
	પૂર્ણાંક ret;

	rep_size = કદ(સંરચના blk_zone_report) + કદ(સંરચના blk_zone) * nzones;
	rep = calloc(1, rep_size);
	જો (!rep)
		પાછા ખાલી;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	જો (ret || rep->nr_zones != nzones) {
		મુક્ત(rep);
		પાછા ખાલી;
	}

	પાછા rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <fcntl.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	uint64_t bytes, sectors;
	પૂર્ણાંક sector_size, phy_sector_size;
	પૂર્ણાંક fd;

	જો (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	જો ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	જો (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	જો (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	જો (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	જો (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
