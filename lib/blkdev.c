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

#यदि_परिभाषितम् HAVE_LINUX_FD_H
#अन्तर्भाव <linux/fd.h>
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_LINUX_BLKZONED_H
#अन्तर्भाव <linux/blkzoned.h>
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_SYS_DISKLABEL_H
#अन्तर्भाव <sys/disklabel.h>
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_SYS_DISK_H
# include <sys/disk.h>
#यदि_अन्तः

#यदि_अपरिभाषितम् EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#यदि_अन्तः

#अन्तर्भाव "all-io.h"
#अन्तर्भाव "blkdev.h"
#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "linux_version.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "nls.h"

स्थिर दीर्घ blkdev_valid_विस्थापनम्(पूर्णाङ्क fd, off_t विस्थापनम्)
{
	वर्ण ch;

	यदि (lseek (fd, विस्थापनम्, 0) < 0)
		प्रत्यागम 0;
	यदि (ul_read_all (fd, &ch, 1) < 1)
		प्रत्यागम 0;
	प्रत्यागम 1;
}

पूर्णाङ्क is_blkdev(पूर्णाङ्क fd)
{
	संरचना stat st;
	प्रत्यागम (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_आकार (पूर्णाङ्क fd)
{
	off_t high, नीचः = 0;

	प्रति (high = 1024; blkdev_valid_विस्थापनम्(fd, high); ) {
		यदि (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			प्रत्यागम -1;
		}

		नीचः = high;

		यदि (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अन्यथा
			high *= 2;
	}

	यावत् (नीचः < high - 1) {
		off_t mid = (नीचः + high) / 2;

		यदि (blkdev_valid_विस्थापनम् (fd, mid))
			नीचः = mid;
		अन्यथा
			high = mid;
	}
	blkdev_valid_विस्थापनम् (fd, 0);
	प्रत्यागम (नीचः + 1);
}

/* get size in bytes */
पूर्णाङ्क blkdev_get_आकार(पूर्णाङ्क fd, uint64_t *अष्टकानि)
{
#यदि_परिभाषितम् DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	यदि (ioctl(fd, DKIOCGETBLOCKCOUNT, अष्टकानि) >= 0) {
		*अष्टकानि <<= 9;
		प्रत्यागम 0;
	}
#यदि_अन्तः

#यदि_परिभाषितम् BLKGETSIZE64
	यदि (ioctl(fd, BLKGETSIZE64, अष्टकानि) >= 0)
		प्रत्यागम 0;
#यदि_अन्तः

#यदि_परिभाषितम् BLKGETSIZE
	{
		अचिह्नित दीर्घ आकार;

		यदि (ioctl(fd, BLKGETSIZE, &आकार) >= 0) {
			*अष्टकानि = ((uint64_t) आकार) << 9;
			प्रत्यागम 0;
		}
	}

#यदि_अन्तः /* BLKGETSIZE */

#यदि_परिभाषितम् DIOCGMEDIASIZE
	/* FreeBSD */
	यदि (ioctl(fd, DIOCGMEDIASIZE, अष्टकानि) >= 0)
		प्रत्यागम 0;
#यदि_अन्तः

#यदि_परिभाषितम् FDGETPRM
	{
		संरचना floppy_struct this_floppy;

		यदि (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*अष्टकानि = ((uint64_t) this_floppy.आकार) << 9;
			प्रत्यागम 0;
		}
	}
#यदि_अन्तः /* FDGETPRM */

#यदि defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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

		यदि ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		यदि (part >= 0 && (ioctl(fd, DIOCGDINFO, (वर्ण *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			यदि (pp->p_आकार) {
				 *अष्टकानि = ((uint64_t) pp->p_आकार) << 9;
				 प्रत्यागम 0;
			}
		}
	}
#यदि_अन्तः /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		संरचना stat st;

		यदि (fstat(fd, &st) != 0)
			प्रत्यागम -1;
		यदि (S_ISREG(st.st_mode)) {
			*अष्टकानि = (uint64_t) st.st_size;
			प्रत्यागम 0;
		}
		यदि (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			प्रत्यागम -1;
		}
	}

	{
		off_t आकार = blkdev_find_आकार(fd);

		यदि (आकार < 0)
			प्रत्यागम -1;

		*अष्टकानि = (uint64_t) आकार;
	}

	प्रत्यागम 0;
}

/* get 512-byte sector count */
पूर्णाङ्क blkdev_get_sectors(पूर्णाङ्क fd, uint64_t *sectors)
{
	uint64_t अष्टकानि;

	यदि (blkdev_get_आकार(fd, &अष्टकानि) == 0) {
		*sectors = (अष्टकानि >> 9);
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
#यदि_परिभाषितम् BLKSSZGET
पूर्णाङ्क blkdev_get_sector_आकार(पूर्णाङ्क fd, पूर्णाङ्क *sector_आकार)
{
	यदि (ioctl(fd, BLKSSZGET, sector_आकार) >= 0)
		प्रत्यागम 0;
	प्रत्यागम -1;
}
#अन्यथा
पूर्णाङ्क blkdev_get_sector_आकार(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_आकार)
{
	*sector_आकार = DEFAULT_SECTOR_आकार;
	प्रत्यागम 0;
}
#यदि_अन्तः

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
#यदि_परिभाषितम् BLKPBSZGET
पूर्णाङ्क blkdev_get_physector_आकार(पूर्णाङ्क fd, पूर्णाङ्क *sector_आकार)
{
	यदि (ioctl(fd, BLKPBSZGET, sector_आकार) >= 0)
		प्रत्यागम 0;
	प्रत्यागम -1;
}
#अन्यथा
पूर्णाङ्क blkdev_get_physector_आकार(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_आकार)
{
	*sector_आकार = DEFAULT_SECTOR_आकार;
	प्रत्यागम 0;
}
#यदि_अन्तः

/*
 * Return the alignment status of a device
 */
#यदि_परिभाषितम् BLKALIGNOFF
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd)
{
	पूर्णाङ्क aligned;

	यदि (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		प्रत्यागम 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	प्रत्यागम aligned != 0 ? 1 : 0;
}
#अन्यथा
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
}
#यदि_अन्तः

पूर्णाङ्क open_blkdev_or_file(ध्रुव संरचना stat *st, ध्रुव वर्ण *name, ध्रुव पूर्णाङ्क oflag)
{
	पूर्णाङ्क fd;

	यदि (S_ISBLK(st->st_mode)) {
		fd = उद्घाटय(name, oflag | O_EXCL | O_CLOEXEC);
	} अन्यथा
		fd = उद्घाटय(name, oflag);
	यदि (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		प्रत्यागम -1;
	}
	यदि (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	प्रत्यागम fd;
}

#यदि_परिभाषितम् CDROM_GET_CAPABILITY
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd)
{
	पूर्णाङ्क ret = ioctl(fd, CDROM_GET_CAPABILITY, रिक्त);
	प्रत्यागम ret < 0 ? 0 : ret;
}
#अन्यथा
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
}
#यदि_अन्तः

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#यदि_परिभाषितम् HDIO_GETGEO
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd, अचिह्नित पूर्णाङ्क *h, अचिह्नित पूर्णाङ्क *s)
{
	संरचना hd_geometry geometry;

	यदि (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		प्रत्यागम 0;
	}
#अन्यथा
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd __attribute__((__unused__)),
		अचिह्नित पूर्णाङ्क *h, अचिह्नित पूर्णाङ्क *s)
{
	*h = 0;
	*s = 0;
#यदि_अन्तः
	प्रत्यागम -1;
}

/*
 * Convert scsi type to human readable string.
 */
ध्रुव वर्ण *blkdev_scsi_type_गन्तव्यम्_name(पूर्णाङ्क type)
{
	विकल्प (type) {
	पक्ष SCSI_TYPE_DISK:
		प्रत्यागम "disk";
	पक्ष SCSI_TYPE_पट्टिका:
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
पूर्णाङ्क blkdev_तालकम्(पूर्णाङ्क fd, ध्रुव वर्ण *devname, ध्रुव वर्ण *lockmode)
{
	पूर्णाङ्क oper, rc, सन्देशः = 0;

	यदि (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	यदि (!lockmode)
		प्रत्यागम 0;

	यदि (c_strcasecmp(lockmode, "yes") == 0 ||
	    वाक्यं_तुलय(lockmode, "1") == 0)
		oper = LOCK_EX;

	अन्यथा यदि (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अन्यथा यदि (c_strcasecmp(lockmode, "no") == 0 ||
		 वाक्यं_तुलय(lockmode, "0") == 0)
		प्रत्यागम 0;
	अन्यथा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		प्रत्यागम -EINVAL;
	}

	यदि (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		यदि (rc == 0)
			प्रत्यागम 0;
		यदि (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					क्रमादेशः_invocation_short_name, devname);
			सन्देशः = 1;
		}
	}
	rc = flock(fd, oper);
	यदि (rc != 0) {
		विकल्प (errno) {
		पक्ष EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			भङ्ग;
		स्वभाव:
			warn(_("%s: failed to get lock"), devname);
		}
	} अन्यथा यदि (सन्देशः)
		fprintf(stderr, _("OK\n"));
	प्रत्यागम rc;
}

#यदि_परिभाषितम् HAVE_LINUX_BLKZONED_H
संरचना blk_zone_report *blkdev_get_zonereport(पूर्णाङ्क fd, uint64_t sector, uint32_t nzones)
{
	संरचना blk_zone_report *rep;
	size_t rep_आकार;
	पूर्णाङ्क ret;

	rep_आकार = परिमाण(संरचना blk_zone_report) + परिमाण(संरचना blk_zone) * nzones;
	rep = शून्यस्मृतिं_देहि(1, rep_आकार);
	यदि (!rep)
		प्रत्यागम रिक्त;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	यदि (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		प्रत्यागम रिक्त;
	}

	प्रत्यागम rep;
}
#यदि_अन्तः


#यदि_परिभाषितम् TEST_क्रमादेशः_BLKDEV
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <fcntl.h>

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	uint64_t अष्टकानि, sectors;
	पूर्णाङ्क sector_आकार, phy_sector_आकार;
	पूर्णाङ्क fd;

	यदि (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		निर्गच्छ(EXIT_FAILURE);
	}

	यदि ((fd = उद्घाटय(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	यदि (blkdev_get_आकार(fd, &अष्टकानि) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	यदि (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	यदि (blkdev_get_sector_आकार(fd, &sector_आकार) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	यदि (blkdev_get_physector_आकार(fd, &phy_sector_आकार) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	प्रारूप_लिख("          bytes: %" PRIu64 "\n", अष्टकानि);
	प्रारूप_लिख("        sectors: %" PRIu64 "\n", sectors);
	प्रारूप_लिख("    sector size: %d\n", sector_आकार);
	प्रारूप_लिख("phy-sector size: %d\n", phy_sector_आकार);

	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_BLKDEV */
