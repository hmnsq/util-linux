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

	सचे (lseek (fd, offset, 0) < 0)
		पटिनिवत्त 0;
	सचे (ul_read_all (fd, &ch, 1) < 1)
		पटिनिवत्त 0;
	पटिनिवत्त 1;
}

पूर्णाङ्क is_blkdev(पूर्णाङ्क fd)
{
	संरचना stat st;
	पटिनिवत्त (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (पूर्णाङ्क fd)
{
	off_t high, नीचः = 0;

	अत्थाय (high = 1024; blkdev_valid_offset(fd, high); ) {
		सचे (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			पटिनिवत्त -1;
		}

		नीचः = high;

		सचे (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अञ्ञथा
			high *= 2;
	}

	याव (नीचः < high - 1) {
		off_t mid = (नीचः + high) / 2;

		सचे (blkdev_valid_offset (fd, mid))
			नीचः = mid;
		अञ्ञथा
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	पटिनिवत्त (नीचः + 1);
}

/* get size in bytes */
पूर्णाङ्क blkdev_get_size(पूर्णाङ्क fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	सचे (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		पटिनिवत्त 0;
	}
#endif

#ifdef BLKGETSIZE64
	सचे (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		पटिनिवत्त 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		सचे (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			पटिनिवत्त 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	सचे (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		पटिनिवत्त 0;
#endif

#ifdef FDGETPRM
	{
		संरचना floppy_struct this_floppy;

		सचे (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			पटिनिवत्त 0;
		}
	}
#endif /* FDGETPRM */

#सचे defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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

		सचे ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		सचे (part >= 0 && (ioctl(fd, DIOCGDINFO, (वर्ण *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			सचे (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 पटिनिवत्त 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		संरचना stat st;

		सचे (fstat(fd, &st) != 0)
			पटिनिवत्त -1;
		सचे (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			पटिनिवत्त 0;
		}
		सचे (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			पटिनिवत्त -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		सचे (size < 0)
			पटिनिवत्त -1;

		*bytes = (uint64_t) size;
	}

	पटिनिवत्त 0;
}

/* get 512-byte sector count */
पूर्णाङ्क blkdev_get_sectors(पूर्णाङ्क fd, uint64_t *sectors)
{
	uint64_t bytes;

	सचे (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		पटिनिवत्त 0;
	}

	पटिनिवत्त -1;
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
	सचे (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		पटिनिवत्त 0;
	पटिनिवत्त -1;
}
#अञ्ञथा
पूर्णाङ्क blkdev_get_sector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	पटिनिवत्त 0;
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
	सचे (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		पटिनिवत्त 0;
	पटिनिवत्त -1;
}
#अञ्ञथा
पूर्णाङ्क blkdev_get_physector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	पटिनिवत्त 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd)
{
	पूर्णाङ्क aligned;

	सचे (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		पटिनिवत्त 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	पटिनिवत्त aligned != 0 ? 1 : 0;
}
#अञ्ञथा
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd __attribute__((__unused__)))
{
	पटिनिवत्त 0;
}
#endif

पूर्णाङ्क open_blkdev_or_file(ध्रुव संरचना stat *st, ध्रुव वर्ण *name, ध्रुव पूर्णाङ्क oflag)
{
	पूर्णाङ्क fd;

	सचे (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} अञ्ञथा
		fd = open(name, oflag);
	सचे (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		पटिनिवत्त -1;
	}
	सचे (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	पटिनिवत्त fd;
}

#ifdef CDROM_GET_CAPABILITY
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd)
{
	पूर्णाङ्क ret = ioctl(fd, CDROM_GET_CAPABILITY, रिक्त);
	पटिनिवत्त ret < 0 ? 0 : ret;
}
#अञ्ञथा
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd __attribute__((__unused__)))
{
	पटिनिवत्त 0;
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

	सचे (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		पटिनिवत्त 0;
	}
#अञ्ञथा
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd __attribute__((__unused__)),
		unsigned पूर्णाङ्क *h, unsigned पूर्णाङ्क *s)
{
	*h = 0;
	*s = 0;
#endif
	पटिनिवत्त -1;
}

/*
 * Convert scsi type to human readable string.
 */
ध्रुव वर्ण *blkdev_scsi_type_to_name(पूर्णाङ्क type)
{
	विकल्प (type) {
	पक्ष SCSI_TYPE_DISK:
		पटिनिवत्त "disk";
	पक्ष SCSI_TYPE_TAPE:
		पटिनिवत्त "tape";
	पक्ष SCSI_TYPE_PRINTER:
		पटिनिवत्त "printer";
	पक्ष SCSI_TYPE_PROCESSOR:
		पटिनिवत्त "processor";
	पक्ष SCSI_TYPE_WORM:
		पटिनिवत्त "worm";
	पक्ष SCSI_TYPE_ROM:
		पटिनिवत्त "rom";
	पक्ष SCSI_TYPE_SCANNER:
		पटिनिवत्त "scanner";
	पक्ष SCSI_TYPE_MOD:
		पटिनिवत्त "mo-disk";
	पक्ष SCSI_TYPE_MEDIUM_CHANGER:
		पटिनिवत्त "changer";
	पक्ष SCSI_TYPE_COMM:
		पटिनिवत्त "comm";
	पक्ष SCSI_TYPE_RAID:
		पटिनिवत्त "raid";
	पक्ष SCSI_TYPE_ENCLOSURE:
		पटिनिवत्त "enclosure";
	पक्ष SCSI_TYPE_RBC:
		पटिनिवत्त "rbc";
	पक्ष SCSI_TYPE_OSD:
		पटिनिवत्त "osd";
	पक्ष SCSI_TYPE_NO_LUN:
		पटिनिवत्त "no-lun";
	स्वभाव:
		भङ्ग;
	}
	पटिनिवत्त रिक्त;
}

/* return 0 on success */
पूर्णाङ्क blkdev_lock(पूर्णाङ्क fd, ध्रुव वर्ण *devname, ध्रुव वर्ण *lockmode)
{
	पूर्णाङ्क oper, rc, सन्देशः = 0;

	सचे (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	सचे (!lockmode)
		पटिनिवत्त 0;

	सचे (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	अञ्ञथा सचे (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अञ्ञथा सचे (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		पटिनिवत्त 0;
	अञ्ञथा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		पटिनिवत्त -EINVAL;
	}

	सचे (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		सचे (rc == 0)
			पटिनिवत्त 0;
		सचे (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			सन्देशः = 1;
		}
	}
	rc = flock(fd, oper);
	सचे (rc != 0) {
		विकल्प (errno) {
		पक्ष EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			भङ्ग;
		स्वभाव:
			warn(_("%s: failed to get lock"), devname);
		}
	} अञ्ञथा सचे (सन्देशः)
		fprintf(stderr, _("OK\n"));
	पटिनिवत्त rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
संरचना blk_zone_report *blkdev_get_zonereport(पूर्णाङ्क fd, uint64_t sector, uint32_t nzones)
{
	संरचना blk_zone_report *rep;
	size_t rep_size;
	पूर्णाङ्क ret;

	rep_size = परिमाण(संरचना blk_zone_report) + परिमाण(संरचना blk_zone) * nzones;
	rep = calloc(1, rep_size);
	सचे (!rep)
		पटिनिवत्त रिक्त;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	सचे (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		पटिनिवत्त रिक्त;
	}

	पटिनिवत्त rep;
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

	सचे (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	सचे ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	सचे (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	सचे (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	सचे (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	सचे (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	पटिनिवत्त EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
