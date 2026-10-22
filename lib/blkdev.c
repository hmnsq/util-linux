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

स्थिर दीर्घ blkdev_valid_offset(पूर्णाङ्क fd, off_t offset)
{
	वर्ण ch;

	यदि (lseek (fd, offset, 0) < 0)
		फर्काऊ 0;
	यदि (ul_read_all (fd, &ch, 1) < 1)
		फर्काऊ 0;
	फर्काऊ 1;
}

पूर्णाङ्क is_blkdev(पूर्णाङ्क fd)
{
	संरचना stat st;
	फर्काऊ (ख_स्थिति(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (पूर्णाङ्क fd)
{
	off_t high, तल्लो = 0;

	लागि (high = 1024; blkdev_valid_offset(fd, high); ) {
		यदि (high == SINT_MAX(off_t)) {
			त्रुटि_सं = EFBIG;
			फर्काऊ -1;
		}

		तल्लो = high;

		यदि (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अन्यथा
			high *= 2;
	}

	जबसम्म (तल्लो < high - 1) {
		off_t mid = (तल्लो + high) / 2;

		यदि (blkdev_valid_offset (fd, mid))
			तल्लो = mid;
		अन्यथा
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	फर्काऊ (तल्लो + 1);
}

/* get size in bytes */
पूर्णाङ्क blkdev_get_size(पूर्णाङ्क fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	यदि (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		फर्काऊ 0;
	}
#endif

#ifdef BLKGETSIZE64
	यदि (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		फर्काऊ 0;
#endif

#ifdef BLKGETSIZE
	{
		अचिन्हित दीर्घ size;

		यदि (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			फर्काऊ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	यदि (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		फर्काऊ 0;
#endif

#ifdef FDGETPRM
	{
		संरचना floppy_struct this_floppy;

		यदि (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			फर्काऊ 0;
		}
	}
#endif /* FDGETPRM */

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

		यदि ((ख_स्थिति(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		यदि (part >= 0 && (ioctl(fd, DIOCGDINFO, (वर्ण *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			यदि (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 फर्काऊ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		संरचना stat st;

		यदि (ख_स्थिति(fd, &st) != 0)
			फर्काऊ -1;
		यदि (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			फर्काऊ 0;
		}
		यदि (!S_ISBLK(st.st_mode)) {
			त्रुटि_सं = ENOTBLK;
			फर्काऊ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		यदि (size < 0)
			फर्काऊ -1;

		*bytes = (uint64_t) size;
	}

	फर्काऊ 0;
}

/* get 512-byte sector count */
पूर्णाङ्क blkdev_get_sectors(पूर्णाङ्क fd, uint64_t *sectors)
{
	uint64_t bytes;

	यदि (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		फर्काऊ 0;
	}

	फर्काऊ -1;
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
	यदि (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		फर्काऊ 0;
	फर्काऊ -1;
}
#अन्यथा
पूर्णाङ्क blkdev_get_sector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	फर्काऊ 0;
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
	यदि (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		फर्काऊ 0;
	फर्काऊ -1;
}
#अन्यथा
पूर्णाङ्क blkdev_get_physector_size(पूर्णाङ्क fd __attribute__((__unused__)), पूर्णाङ्क *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	फर्काऊ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd)
{
	पूर्णाङ्क aligned;

	यदि (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		फर्काऊ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	फर्काऊ aligned != 0 ? 1 : 0;
}
#अन्यथा
पूर्णाङ्क blkdev_is_misaligned(पूर्णाङ्क fd __attribute__((__unused__)))
{
	फर्काऊ 0;
}
#endif

पूर्णाङ्क open_blkdev_or_file(अचर संरचना stat *st, अचर वर्ण *name, अचर पूर्णाङ्क oflag)
{
	पूर्णाङ्क fd;

	यदि (S_ISBLK(st->st_mode)) {
		fd = खोलो(name, oflag | O_EXCL | O_CLOEXEC);
	} अन्यथा
		fd = खोलो(name, oflag);
	यदि (-1 < fd && !is_same_inode(fd, st)) {
		बंद(fd);
		त्रुटि_सं = EBADFD;
		फर्काऊ -1;
	}
	यदि (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	फर्काऊ fd;
}

#ifdef CDROM_GET_CAPABILITY
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd)
{
	पूर्णाङ्क ret = ioctl(fd, CDROM_GET_CAPABILITY, रिक्त);
	फर्काऊ ret < 0 ? 0 : ret;
}
#अन्यथा
पूर्णाङ्क blkdev_is_cdrom(पूर्णाङ्क fd __attribute__((__unused__)))
{
	फर्काऊ 0;
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
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd, अचिन्हित पूर्णाङ्क *h, अचिन्हित पूर्णाङ्क *s)
{
	संरचना hd_geometry geometry;

	यदि (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		फर्काऊ 0;
	}
#अन्यथा
पूर्णाङ्क blkdev_get_geometry(पूर्णाङ्क fd __attribute__((__unused__)),
		अचिन्हित पूर्णाङ्क *h, अचिन्हित पूर्णाङ्क *s)
{
	*h = 0;
	*s = 0;
#endif
	फर्काऊ -1;
}

/*
 * Convert scsi type to human readable string.
 */
अचर वर्ण *blkdev_scsi_type_to_name(पूर्णाङ्क type)
{
	विकल्प (type) {
	पक्ष SCSI_TYPE_DISK:
		फर्काऊ "disk";
	पक्ष SCSI_TYPE_TAPE:
		फर्काऊ "tape";
	पक्ष SCSI_TYPE_PRINTER:
		फर्काऊ "printer";
	पक्ष SCSI_TYPE_PROCESSOR:
		फर्काऊ "processor";
	पक्ष SCSI_TYPE_WORM:
		फर्काऊ "worm";
	पक्ष SCSI_TYPE_ROM:
		फर्काऊ "rom";
	पक्ष SCSI_TYPE_SCANNER:
		फर्काऊ "scanner";
	पक्ष SCSI_TYPE_MOD:
		फर्काऊ "mo-disk";
	पक्ष SCSI_TYPE_MEDIUM_CHANGER:
		फर्काऊ "changer";
	पक्ष SCSI_TYPE_COMM:
		फर्काऊ "comm";
	पक्ष SCSI_TYPE_RAID:
		फर्काऊ "raid";
	पक्ष SCSI_TYPE_ENCLOSURE:
		फर्काऊ "enclosure";
	पक्ष SCSI_TYPE_RBC:
		फर्काऊ "rbc";
	पक्ष SCSI_TYPE_OSD:
		फर्काऊ "osd";
	पक्ष SCSI_TYPE_NO_LUN:
		फर्काऊ "no-lun";
	स्वभाव:
		रोक;
	}
	फर्काऊ रिक्त;
}

/* return 0 on success */
पूर्णाङ्क blkdev_lock(पूर्णाङ्क fd, अचर वर्ण *devname, अचर वर्ण *lockmode)
{
	पूर्णाङ्क oper, rc, सन्देश = 0;

	यदि (!lockmode)
		lockmode = दो_पर्या("LOCK_BLOCK_DEVICE");
	यदि (!lockmode)
		फर्काऊ 0;

	यदि (c_strcasecmp(lockmode, "yes") == 0 ||
	    म_भेद(lockmode, "1") == 0)
		oper = LOCK_EX;

	अन्यथा यदि (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अन्यथा यदि (c_strcasecmp(lockmode, "no") == 0 ||
		 म_भेद(lockmode, "0") == 0)
		फर्काऊ 0;
	अन्यथा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		फर्काऊ -EINVAL;
	}

	यदि (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		यदि (rc == 0)
			फर्काऊ 0;
		यदि (rc != 0 && त्रुटि_सं == EWOULDBLOCK) {
			ख_लिखो(मानक_त्रुटि, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			सन्देश = 1;
		}
	}
	rc = flock(fd, oper);
	यदि (rc != 0) {
		विकल्प (त्रुटि_सं) {
		पक्ष EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			रोक;
		स्वभाव:
			warn(_("%s: failed to get lock"), devname);
		}
	} अन्यथा यदि (सन्देश)
		ख_लिखो(मानक_त्रुटि, _("OK\n"));
	फर्काऊ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
संरचना blk_zone_report *blkdev_get_zonereport(पूर्णाङ्क fd, uint64_t sector, uint32_t nzones)
{
	संरचना blk_zone_report *rep;
	माप_प्रकार rep_size;
	पूर्णाङ्क ret;

	rep_size = परिमाण(संरचना blk_zone_report) + परिमाण(संरचना blk_zone) * nzones;
	rep = सुस्मृति(1, rep_size);
	यदि (!rep)
		फर्काऊ रिक्त;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	यदि (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		फर्काऊ रिक्त;
	}

	फर्काऊ rep;
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

	यदि (argc != 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s device\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	यदि ((fd = खोलो(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(निकास_त्रुटि, "open %s failed", argv[1]);

	यदि (blkdev_get_size(fd, &bytes) < 0)
		err(निकास_त्रुटि, "blkdev_get_size() failed");
	यदि (blkdev_get_sectors(fd, &sectors) < 0)
		err(निकास_त्रुटि, "blkdev_get_sectors() failed");
	यदि (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(निकास_त्रुटि, "blkdev_get_sector_size() failed");
	यदि (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(निकास_त्रुटि, "blkdev_get_physector_size() failed");

	म_लिखो("          bytes: %" PRIu64 "\n", bytes);
	म_लिखो("        sectors: %" PRIu64 "\n", sectors);
	म_लिखो("    sector size: %d\n", sector_size);
	म_लिखो("phy-sector size: %d\n", phy_sector_size);

	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_BLKDEV */
