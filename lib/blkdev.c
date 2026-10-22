/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdint.h>

#ifdef HAVE_LINUX_FD_H
#अन्तर्भाव <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#अन्तर्भाव <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#अन्तर्भाव <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#अन्तर्भाव "all-io.h"
#अन्तर्भाव "blkdev.h"
#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "linux_version.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "nls.h"

स्थिर long blkdev_valid_offset(पूर्णाङ्क fd, off_t offset)
{
	वर्ण ch;

	जइ (lseek (fd, offset, 0) < 0)
		प्रत्यागम 0;
	जइ (ul_read_all (fd, &ch, 1) < 1)
		प्रत्यागम 0;
	प्रत्यागम 1;
}

पूर्णाङ्क is_blkdev(पूर्णाङ्क fd)
{
	संरचना stat st;
	प्रत्यागम (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (पूर्णाङ्क fd)
{
	off_t high, नीचः = 0;

	प्रति (high = 1024; blkdev_valid_offset(fd, high); ) {
		जइ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			प्रत्यागम -1;
		}

		नीचः = high;

		जइ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अण्णहा
			high *= 2;
	}

	जाव (नीचः < high - 1) {
		off_t mid = (नीचः + high) / 2;

		जइ (blkdev_valid_offset (fd, mid))
			नीचः = mid;
		अण्णहा
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	प्रत्यागम (नीचः + 1);
}

/* get size in bytes */
पूर्णाङ्क blkdev_get_size(पूर्णाङ्क fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	जइ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		प्रत्यागम 0;
	}
#endif

#ifdef BLKGETSIZE64
	जइ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		प्रत्यागम 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		जइ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			प्रत्यागम 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	जइ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		प्रत्यागम 0;
#endif

#ifdef FDGETPRM
	{
		संरचना floppy_struct this_floppy;

		जइ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			प्रत्यागम 0;
		}
	}
#endif /* FDGETPRM */

#जइ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		पूर्णाङ्क part = -1;
		संरचना disklabel lab;
		संरचना partition *pp;
		संरचना stat st;

		जइ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		जइ (part >= 0 && (ioctl(fd, DIOCGDINFO, (वर्ण *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			जइ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 प्रत्यागम 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		संरचना stat st;

		जइ (fstat(fd, &st) != 0)
			प्रत्यागम -1;
		जइ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			प्रत्यागम 0;
		}
		जइ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			प्रत्यागम -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		जइ (size < 0)
			प्रत्यागम -1;

		*bytes = (uint64_t) size;
	}

	प्रत्यागम 0;
}

/* get 512-byte sector count */
पूर्णाङ्क blkdev_get_sectors(पूर्णाङ्क fd, uint64_t *sectors)
{
	uint64_t bytes;

	जइ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		प्रत्यागम 0;
	}

	प्रत्यागम -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
पूर्णाङ्क blkdev_get_sector_size(पूर्णाङ्क fd, पूर्णाङ्क *sector_size)
{
	जइ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		प्रत्यागम 0;
	प्रत्यागम -1;
}
#अण्णहा
पूर्णाङ्क blkdev_get_sector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	प्रत्यागम 0;
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
पूर्णाङ्क blkdev_get_physector_size(पूर्णाङ्क fd, पूर्णाङ्क *sector_size)
{
	जइ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		प्रत्यागम 0;
	प्रत्यागम -1;
}
#अण्णहा
पूर्णाङ्क blkdev_get_physector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	प्रत्यागम 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd)
{
	पूर्णाङ्क aligned;

	जइ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		प्रत्यागम 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	प्रत्यागम aligned != 0 ? 1 : 0;
}
#अण्णहा
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
}
#endif

पूर्णाङ्क open_blkdev_or_file(ध्रुव संरचना stat *st, ध्रुव वर्ण *name, ध्रुव पूर्णाङ्क oflag)
{
	पूर्णाङ्क fd;

	जइ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} अण्णहा
		fd = open(name, oflag);
	जइ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		प्रत्यागम -1;
	}
	जइ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	प्रत्यागम fd;
}

#ifdef CDROM_GET_CAPABILITY
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd)
{
	पूर्णाङ्क ret = ioctl(fd, CDROM_GET_CAPABILITY, रिक्त);
	प्रत्यागम ret < 0 ? 0 : ret;
}
#अण्णहा
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
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
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd, unsigned पूर्णाङ्क *h, unsigned पूर्णाङ्क *s)
{
	संरचना hd_geometry geometry;

	जइ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		प्रत्यागम 0;
	}
#अण्णहा
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd __attribute__((__unused__)),
		unsigned पूर्णाङ्क *h, unsigned पूर्णाङ्क *s)
{
	*h = 0;
	*s = 0;
#endif
	प्रत्यागम -1;
}

/*
 * Convert scsi type to human readable string.
 */
ध्रुव वर्ण *blkdev_scsi_type_to_name(पूर्णाङ्क type)
{
	विकल्प (type) {
	पक्ष SCSI_TYPE_DISK:
		प्रत्यागम "disk";
	पक्ष SCSI_TYPE_TAPE:
		प्रत्यागम "tape";
	पक्ष SCSI_TYPE_PRINTER:
		प्रत्यागम "printer";
	पक्ष SCSI_TYPE_PROCESSOR:
		प्रत्यागम "processor";
	पक्ष SCSI_TYPE_WORM:
		प्रत्यागम "worm";
	पक्ष SCSI_TYPE_ROM:
		प्रत्यागम "rom";
	पक्ष SCSI_TYPE_SCANNER:
		प्रत्यागम "scanner";
	पक्ष SCSI_TYPE_MOD:
		प्रत्यागम "mo-disk";
	पक्ष SCSI_TYPE_MEDIUM_CHANGER:
		प्रत्यागम "changer";
	पक्ष SCSI_TYPE_COMM:
		प्रत्यागम "comm";
	पक्ष SCSI_TYPE_RAID:
		प्रत्यागम "raid";
	पक्ष SCSI_TYPE_ENCLOSURE:
		प्रत्यागम "enclosure";
	पक्ष SCSI_TYPE_RBC:
		प्रत्यागम "rbc";
	पक्ष SCSI_TYPE_OSD:
		प्रत्यागम "osd";
	पक्ष SCSI_TYPE_NO_LUN:
		प्रत्यागम "no-lun";
	स्वभाव:
		भङ्ग;
	}
	प्रत्यागम रिक्त;
}

/* return 0 on success */
पूर्णाङ्क blkdev_lock(पूर्णाङ्क fd, ध्रुव वर्ण *devname, ध्रुव वर्ण *lockmode)
{
	पूर्णाङ्क oper, rc, सन्देशः = 0;

	जइ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	जइ (!lockmode)
		प्रत्यागम 0;

	जइ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	अण्णहा जइ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अण्णहा जइ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		प्रत्यागम 0;
	अण्णहा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		प्रत्यागम -EINVAL;
	}

	जइ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		जइ (rc == 0)
			प्रत्यागम 0;
		जइ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			सन्देशः = 1;
		}
	}
	rc = flock(fd, oper);
	जइ (rc != 0) {
		विकल्प (errno) {
		पक्ष EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			भङ्ग;
		स्वभाव:
			warn(_("%s: failed to get lock"), devname);
		}
	} अण्णहा जइ (सन्देशः)
		fprintf(stderr, _("OK\n"));
	प्रत्यागम rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
संरचना blk_zone_report *blkdev_get_zonereport(पूर्णाङ्क fd, uint64_t sector, uint32_t nzones)
{
	संरचना blk_zone_report *rep;
	size_t rep_size;
	पूर्णाङ्क ret;

	rep_size = परिमाण(संरचना blk_zone_report) + परिमाण(संरचना blk_zone) * nzones;
	rep = calloc(1, rep_size);
	जइ (!rep)
		प्रत्यागम रिक्त;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	जइ (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <fcntl.h>

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	uint64_t bytes, sectors;
	पूर्णाङ्क sector_size, phy_sector_size;
	पूर्णाङ्क fd;

	जइ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	जइ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	जइ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	जइ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	जइ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	जइ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
