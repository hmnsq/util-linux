/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdint.h>

#నిర్వచితమైతే HAVE_LINUX_FD_H
#అన్తర్భావ <linux/fd.h>
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_LINUX_BLKZONED_H
#అన్తర్భావ <linux/blkzoned.h>
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_SYS_DISKLABEL_H
#అన్తర్భావ <sys/disklabel.h>
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_SYS_DISK_H
# include <sys/disk.h>
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#అయితే_ముగింపు

#అన్తర్భావ "all-io.h"
#అన్తర్భావ "blkdev.h"
#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "linux_version.h"
#అన్తర్భావ "fileutils.h"
#అన్తర్భావ "nls.h"

స్థిర దీర్ఘ blkdev_valid_స్థానభ్రంశం(పూర్ణాంకం fd, off_t స్థానభ్రంశం)
{
	అక్షరం ch;

	ఒకవేళ (lseek (fd, స్థానభ్రంశం, 0) < 0)
		తిరిగి 0;
	ఒకవేళ (ul_read_all (fd, &ch, 1) < 1)
		తిరిగి 0;
	తిరిగి 1;
}

పూర్ణాంకం is_blkdev(పూర్ణాంకం fd)
{
	నిర్మాణం stat st;
	తిరిగి (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_కనుగొను_కొలత (పూర్ణాంకం fd)
{
	off_t high, దిగువ = 0;

	కొరకు (high = 1024; blkdev_valid_స్థానభ్రంశం(fd, high); ) {
		ఒకవేళ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			తిరిగి -1;
		}

		దిగువ = high;

		ఒకవేళ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		లేకపోతే
			high *= 2;
	}

	అయితే (దిగువ < high - 1) {
		off_t mid = (దిగువ + high) / 2;

		ఒకవేళ (blkdev_valid_స్థానభ్రంశం (fd, mid))
			దిగువ = mid;
		లేకపోతే
			high = mid;
	}
	blkdev_valid_స్థానభ్రంశం (fd, 0);
	తిరిగి (దిగువ + 1);
}

/* get size in bytes */
పూర్ణాంకం blkdev_get_కొలత(పూర్ణాంకం fd, uint64_t *బైట్లు)
{
#నిర్వచితమైతే DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	ఒకవేళ (ioctl(fd, DKIOCGETBLOCKCOUNT, బైట్లు) >= 0) {
		*బైట్లు <<= 9;
		తిరిగి 0;
	}
#అయితే_ముగింపు

#నిర్వచితమైతే BLKGETSIZE64
	ఒకవేళ (ioctl(fd, BLKGETSIZE64, బైట్లు) >= 0)
		తిరిగి 0;
#అయితే_ముగింపు

#నిర్వచితమైతే BLKGETSIZE
	{
		గుర్తులేని దీర్ఘ కొలత;

		ఒకవేళ (ioctl(fd, BLKGETSIZE, &కొలత) >= 0) {
			*బైట్లు = ((uint64_t) కొలత) << 9;
			తిరిగి 0;
		}
	}

#అయితే_ముగింపు /* BLKGETSIZE */

#నిర్వచితమైతే DIOCGMEDIASIZE
	/* FreeBSD */
	ఒకవేళ (ioctl(fd, DIOCGMEDIASIZE, బైట్లు) >= 0)
		తిరిగి 0;
#అయితే_ముగింపు

#నిర్వచితమైతే FDGETPRM
	{
		నిర్మాణం floppy_struct this_floppy;

		ఒకవేళ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*బైట్లు = ((uint64_t) this_floppy.కొలత) << 9;
			తిరిగి 0;
		}
	}
#అయితే_ముగింపు /* FDGETPRM */

#ఒకవేళ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		పూర్ణాంకం part = -1;
		నిర్మాణం disklabel lab;
		నిర్మాణం విభజన *pp;
		నిర్మాణం stat st;

		ఒకవేళ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		ఒకవేళ (part >= 0 && (ioctl(fd, DIOCGDINFO, (అక్షరం *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			ఒకవేళ (pp->p_కొలత) {
				 *బైట్లు = ((uint64_t) pp->p_కొలత) << 9;
				 తిరిగి 0;
			}
		}
	}
#అయితే_ముగింపు /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		నిర్మాణం stat st;

		ఒకవేళ (fstat(fd, &st) != 0)
			తిరిగి -1;
		ఒకవేళ (S_ISREG(st.st_mode)) {
			*బైట్లు = (uint64_t) st.st_size;
			తిరిగి 0;
		}
		ఒకవేళ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			తిరిగి -1;
		}
	}

	{
		off_t కొలత = blkdev_కనుగొను_కొలత(fd);

		ఒకవేళ (కొలత < 0)
			తిరిగి -1;

		*బైట్లు = (uint64_t) కొలత;
	}

	తిరిగి 0;
}

/* get 512-byte sector count */
పూర్ణాంకం blkdev_get_sectors(పూర్ణాంకం fd, uint64_t *sectors)
{
	uint64_t బైట్లు;

	ఒకవేళ (blkdev_get_కొలత(fd, &బైట్లు) == 0) {
		*sectors = (బైట్లు >> 9);
		తిరిగి 0;
	}

	తిరిగి -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#నిర్వచితమైతే BLKSSZGET
పూర్ణాంకం blkdev_get_sector_కొలత(పూర్ణాంకం fd, పూర్ణాంకం *sector_కొలత)
{
	ఒకవేళ (ioctl(fd, BLKSSZGET, sector_కొలత) >= 0)
		తిరిగి 0;
	తిరిగి -1;
}
#లేకపోతే
పూర్ణాంకం blkdev_get_sector_కొలత(పూర్ణాంకం fd __attribute__((__unused__)), పూర్ణాంకం *sector_కొలత)
{
	*sector_కొలత = DEFAULT_SECTOR_కొలత;
	తిరిగి 0;
}
#అయితే_ముగింపు

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
#నిర్వచితమైతే BLKPBSZGET
పూర్ణాంకం blkdev_get_physector_కొలత(పూర్ణాంకం fd, పూర్ణాంకం *sector_కొలత)
{
	ఒకవేళ (ioctl(fd, BLKPBSZGET, sector_కొలత) >= 0)
		తిరిగి 0;
	తిరిగి -1;
}
#లేకపోతే
పూర్ణాంకం blkdev_get_physector_కొలత(పూర్ణాంకం fd __attribute__((__unused__)), పూర్ణాంకం *sector_కొలత)
{
	*sector_కొలత = DEFAULT_SECTOR_కొలత;
	తిరిగి 0;
}
#అయితే_ముగింపు

/*
 * Return the alignment status of a device
 */
#నిర్వచితమైతే BLKALIGNOFF
పూర్ణాంకం blkdev_is_misaligned(పూర్ణాంకం fd)
{
	పూర్ణాంకం aligned;

	ఒకవేళ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		తిరిగి 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	తిరిగి aligned != 0 ? 1 : 0;
}
#లేకపోతే
పూర్ణాంకం blkdev_is_misaligned(పూర్ణాంకం fd __attribute__((__unused__)))
{
	తిరిగి 0;
}
#అయితే_ముగింపు

పూర్ణాంకం open_blkdev_or_file(స్థిరరాశి నిర్మాణం stat *st, స్థిరరాశి అక్షరం *name, స్థిరరాశి పూర్ణాంకం oflag)
{
	పూర్ణాంకం fd;

	ఒకవేళ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} లేకపోతే
		fd = open(name, oflag);
	ఒకవేళ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		తిరిగి -1;
	}
	ఒకవేళ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	తిరిగి fd;
}

#నిర్వచితమైతే CDROM_GET_CAPABILITY
పూర్ణాంకం blkdev_is_cdrom(పూర్ణాంకం fd)
{
	పూర్ణాంకం ret = ioctl(fd, CDROM_GET_CAPABILITY, ఖాళీ);
	తిరిగి ret < 0 ? 0 : ret;
}
#లేకపోతే
పూర్ణాంకం blkdev_is_cdrom(పూర్ణాంకం fd __attribute__((__unused__)))
{
	తిరిగి 0;
}
#అయితే_ముగింపు

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#నిర్వచితమైతే HDIO_GETGEO
పూర్ణాంకం blkdev_get_geometry(పూర్ణాంకం fd, గుర్తులేని పూర్ణాంకం *h, గుర్తులేని పూర్ణాంకం *s)
{
	నిర్మాణం hd_geometry geometry;

	ఒకవేళ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		తిరిగి 0;
	}
#లేకపోతే
పూర్ణాంకం blkdev_get_geometry(పూర్ణాంకం fd __attribute__((__unused__)),
		గుర్తులేని పూర్ణాంకం *h, గుర్తులేని పూర్ణాంకం *s)
{
	*h = 0;
	*s = 0;
#అయితే_ముగింపు
	తిరిగి -1;
}

/*
 * Convert scsi type to human readable string.
 */
స్థిరరాశి అక్షరం *blkdev_scsi_type_గమ్యం_name(పూర్ణాంకం type)
{
	ఎంపిక (type) {
	స్థితి SCSI_TYPE_DISK:
		తిరిగి "disk";
	స్థితి SCSI_TYPE_పట్టీ:
		తిరిగి "tape";
	స్థితి SCSI_TYPE_PRINTER:
		తిరిగి "printer";
	స్థితి SCSI_TYPE_PROCESSOR:
		తిరిగి "processor";
	స్థితి SCSI_TYPE_WORM:
		తిరిగి "worm";
	స్థితి SCSI_TYPE_ROM:
		తిరిగి "rom";
	స్థితి SCSI_TYPE_SCANNER:
		తిరిగి "scanner";
	స్థితి SCSI_TYPE_MOD:
		తిరిగి "mo-disk";
	స్థితి SCSI_TYPE_MEDIUM_CHANGER:
		తిరిగి "changer";
	స్థితి SCSI_TYPE_COMM:
		తిరిగి "comm";
	స్థితి SCSI_TYPE_RAID:
		తిరిగి "raid";
	స్థితి SCSI_TYPE_ENCLOSURE:
		తిరిగి "enclosure";
	స్థితి SCSI_TYPE_RBC:
		తిరిగి "rbc";
	స్థితి SCSI_TYPE_OSD:
		తిరిగి "osd";
	స్థితి SCSI_TYPE_NO_LUN:
		తిరిగి "no-lun";
	అప్రమేయ:
		ఆపు;
	}
	తిరిగి ఖాళీ;
}

/* return 0 on success */
పూర్ణాంకం blkdev_తాళం(పూర్ణాంకం fd, స్థిరరాశి అక్షరం *devname, స్థిరరాశి అక్షరం *lockmode)
{
	పూర్ణాంకం oper, rc, సందేశం = 0;

	ఒకవేళ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	ఒకవేళ (!lockmode)
		తిరిగి 0;

	ఒకవేళ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	లేకపోతే ఒకవేళ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	లేకపోతే ఒకవేళ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		తిరిగి 0;
	లేకపోతే {
		warnx(_("unsupported lock mode: %s"), lockmode);
		తిరిగి -EINVAL;
	}

	ఒకవేళ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		ఒకవేళ (rc == 0)
			తిరిగి 0;
		ఒకవేళ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					కార్యక్రమం_invocation_short_name, devname);
			సందేశం = 1;
		}
	}
	rc = flock(fd, oper);
	ఒకవేళ (rc != 0) {
		ఎంపిక (errno) {
		స్థితి EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			ఆపు;
		అప్రమేయ:
			warn(_("%s: failed to get lock"), devname);
		}
	} లేకపోతే ఒకవేళ (సందేశం)
		fprintf(stderr, _("OK\n"));
	తిరిగి rc;
}

#నిర్వచితమైతే HAVE_LINUX_BLKZONED_H
నిర్మాణం blk_zone_report *blkdev_get_zonereport(పూర్ణాంకం fd, uint64_t sector, uint32_t nzones)
{
	నిర్మాణం blk_zone_report *rep;
	size_t rep_కొలత;
	పూర్ణాంకం ret;

	rep_కొలత = పరిమాణం(నిర్మాణం blk_zone_report) + పరిమాణం(నిర్మాణం blk_zone) * nzones;
	rep = సున్నా_జ్ఞాపకం_ఇవ్వు(1, rep_కొలత);
	ఒకవేళ (!rep)
		తిరిగి ఖాళీ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	ఒకవేళ (ret || rep->nr_zones != nzones) {
		విడుదల(rep);
		తిరిగి ఖాళీ;
	}

	తిరిగి rep;
}
#అయితే_ముగింపు


#నిర్వచితమైతే TEST_కార్యక్రమం_BLKDEV
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <fcntl.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	uint64_t బైట్లు, sectors;
	పూర్ణాంకం sector_కొలత, phy_sector_కొలత;
	పూర్ణాంకం fd;

	ఒకవేళ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		నిష్క్రమించు(EXIT_FAILURE);
	}

	ఒకవేళ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	ఒకవేళ (blkdev_get_కొలత(fd, &బైట్లు) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	ఒకవేళ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	ఒకవేళ (blkdev_get_sector_కొలత(fd, &sector_కొలత) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	ఒకవేళ (blkdev_get_physector_కొలత(fd, &phy_sector_కొలత) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	రాయి("          bytes: %" PRIu64 "\n", బైట్లు);
	రాయి("        sectors: %" PRIu64 "\n", sectors);
	రాయి("    sector size: %d\n", sector_కొలత);
	రాయి("phy-sector size: %d\n", phy_sector_కొలత);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_BLKDEV */
