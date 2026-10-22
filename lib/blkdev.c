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

स्थिर दीर्घ blkdev_valid_offset(पूर्णांक fd, off_t offset)
{
	अक्षर ch;

	जर (lseek (fd, offset, 0) < 0)
		परत 0;
	जर (ul_read_all (fd, &ch, 1) < 1)
		परत 0;
	परत 1;
}

पूर्णांक is_blkdev(पूर्णांक fd)
{
	संरचना stat st;
	परत (ख_स्थिति(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (पूर्णांक fd)
{
	off_t high, खालचा = 0;

	साठी (high = 1024; blkdev_valid_offset(fd, high); ) {
		जर (high == SINT_MAX(off_t)) {
			त्रुटि_सं = EFBIG;
			परत -1;
		}

		खालचा = high;

		जर (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अन्यथा
			high *= 2;
	}

	पर्यंत (खालचा < high - 1) {
		off_t mid = (खालचा + high) / 2;

		जर (blkdev_valid_offset (fd, mid))
			खालचा = mid;
		अन्यथा
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	परत (खालचा + 1);
}

/* get size in bytes */
पूर्णांक blkdev_get_size(पूर्णांक fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	जर (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		परत 0;
	}
#endif

#ifdef BLKGETSIZE64
	जर (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		परत 0;
#endif

#ifdef BLKGETSIZE
	{
		अचिन्हित दीर्घ size;

		जर (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			परत 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	जर (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		परत 0;
#endif

#ifdef FDGETPRM
	{
		संरचना floppy_struct this_floppy;

		जर (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			परत 0;
		}
	}
#endif /* FDGETPRM */

#जर defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		पूर्णांक part = -1;
		संरचना disklabel lab;
		संरचना partition *pp;
		संरचना stat st;

		जर ((ख_स्थिति(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		जर (part >= 0 && (ioctl(fd, DIOCGDINFO, (अक्षर *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			जर (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 परत 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		संरचना stat st;

		जर (ख_स्थिति(fd, &st) != 0)
			परत -1;
		जर (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			परत 0;
		}
		जर (!S_ISBLK(st.st_mode)) {
			त्रुटि_सं = ENOTBLK;
			परत -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		जर (size < 0)
			परत -1;

		*bytes = (uint64_t) size;
	}

	परत 0;
}

/* get 512-byte sector count */
पूर्णांक blkdev_get_sectors(पूर्णांक fd, uint64_t *sectors)
{
	uint64_t bytes;

	जर (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		परत 0;
	}

	परत -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
पूर्णांक blkdev_get_sector_size(पूर्णांक fd, पूर्णांक *sector_size)
{
	जर (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		परत 0;
	परत -1;
}
#अन्यथा
पूर्णांक blkdev_get_sector_size(पूर्णांक fd __attribute__((__unused__)), पूर्णांक *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	परत 0;
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
पूर्णांक blkdev_get_physector_size(पूर्णांक fd, पूर्णांक *sector_size)
{
	जर (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		परत 0;
	परत -1;
}
#अन्यथा
पूर्णांक blkdev_get_physector_size(पूर्णांक fd __attribute__((__unused__)), पूर्णांक *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	परत 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
पूर्णांक blkdev_is_misaligned(पूर्णांक fd)
{
	पूर्णांक aligned;

	जर (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		परत 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	परत aligned != 0 ? 1 : 0;
}
#अन्यथा
पूर्णांक blkdev_is_misaligned(पूर्णांक fd __attribute__((__unused__)))
{
	परत 0;
}
#endif

पूर्णांक open_blkdev_or_file(स्थिरांक संरचना stat *st, स्थिरांक अक्षर *name, स्थिरांक पूर्णांक oflag)
{
	पूर्णांक fd;

	जर (S_ISBLK(st->st_mode)) {
		fd = खोलो(name, oflag | O_EXCL | O_CLOEXEC);
	} अन्यथा
		fd = खोलो(name, oflag);
	जर (-1 < fd && !is_same_inode(fd, st)) {
		बंद(fd);
		त्रुटि_सं = EBADFD;
		परत -1;
	}
	जर (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	परत fd;
}

#ifdef CDROM_GET_CAPABILITY
पूर्णांक blkdev_is_cdrom(पूर्णांक fd)
{
	पूर्णांक ret = ioctl(fd, CDROM_GET_CAPABILITY, रिक्त);
	परत ret < 0 ? 0 : ret;
}
#अन्यथा
पूर्णांक blkdev_is_cdrom(पूर्णांक fd __attribute__((__unused__)))
{
	परत 0;
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
पूर्णांक blkdev_get_geometry(पूर्णांक fd, अचिन्हित पूर्णांक *h, अचिन्हित पूर्णांक *s)
{
	संरचना hd_geometry geometry;

	जर (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		परत 0;
	}
#अन्यथा
पूर्णांक blkdev_get_geometry(पूर्णांक fd __attribute__((__unused__)),
		अचिन्हित पूर्णांक *h, अचिन्हित पूर्णांक *s)
{
	*h = 0;
	*s = 0;
#endif
	परत -1;
}

/*
 * Convert scsi type to human readable string.
 */
स्थिरांक अक्षर *blkdev_scsi_type_to_name(पूर्णांक type)
{
	निवड (type) {
	स्थिती SCSI_TYPE_DISK:
		परत "disk";
	स्थिती SCSI_TYPE_TAPE:
		परत "tape";
	स्थिती SCSI_TYPE_PRINTER:
		परत "printer";
	स्थिती SCSI_TYPE_PROCESSOR:
		परत "processor";
	स्थिती SCSI_TYPE_WORM:
		परत "worm";
	स्थिती SCSI_TYPE_ROM:
		परत "rom";
	स्थिती SCSI_TYPE_SCANNER:
		परत "scanner";
	स्थिती SCSI_TYPE_MOD:
		परत "mo-disk";
	स्थिती SCSI_TYPE_MEDIUM_CHANGER:
		परत "changer";
	स्थिती SCSI_TYPE_COMM:
		परत "comm";
	स्थिती SCSI_TYPE_RAID:
		परत "raid";
	स्थिती SCSI_TYPE_ENCLOSURE:
		परत "enclosure";
	स्थिती SCSI_TYPE_RBC:
		परत "rbc";
	स्थिती SCSI_TYPE_OSD:
		परत "osd";
	स्थिती SCSI_TYPE_NO_LUN:
		परत "no-lun";
	पूर्वनिर्धारित:
		थांबा;
	}
	परत रिक्त;
}

/* return 0 on success */
पूर्णांक blkdev_lock(पूर्णांक fd, स्थिरांक अक्षर *devname, स्थिरांक अक्षर *lockmode)
{
	पूर्णांक oper, rc, संदेश = 0;

	जर (!lockmode)
		lockmode = दो_पर्या("LOCK_BLOCK_DEVICE");
	जर (!lockmode)
		परत 0;

	जर (c_strcasecmp(lockmode, "yes") == 0 ||
	    म_भेद(lockmode, "1") == 0)
		oper = LOCK_EX;

	अन्यथा जर (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अन्यथा जर (c_strcasecmp(lockmode, "no") == 0 ||
		 म_भेद(lockmode, "0") == 0)
		परत 0;
	अन्यथा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		परत -EINVAL;
	}

	जर (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		जर (rc == 0)
			परत 0;
		जर (rc != 0 && त्रुटि_सं == EWOULDBLOCK) {
			ख_लिखो(मानक_त्रुटि, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			संदेश = 1;
		}
	}
	rc = flock(fd, oper);
	जर (rc != 0) {
		निवड (त्रुटि_सं) {
		स्थिती EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			थांबा;
		पूर्वनिर्धारित:
			warn(_("%s: failed to get lock"), devname);
		}
	} अन्यथा जर (संदेश)
		ख_लिखो(मानक_त्रुटि, _("OK\n"));
	परत rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
संरचना blk_zone_report *blkdev_get_zonereport(पूर्णांक fd, uint64_t sector, uint32_t nzones)
{
	संरचना blk_zone_report *rep;
	माप_प्रकार rep_size;
	पूर्णांक ret;

	rep_size = आकार(संरचना blk_zone_report) + आकार(संरचना blk_zone) * nzones;
	rep = सुस्मृति(1, rep_size);
	जर (!rep)
		परत रिक्त;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	जर (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		परत रिक्त;
	}

	परत rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <fcntl.h>

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	uint64_t bytes, sectors;
	पूर्णांक sector_size, phy_sector_size;
	पूर्णांक fd;

	जर (argc != 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s device\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	जर ((fd = खोलो(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(निकास_त्रुटि, "open %s failed", argv[1]);

	जर (blkdev_get_size(fd, &bytes) < 0)
		err(निकास_त्रुटि, "blkdev_get_size() failed");
	जर (blkdev_get_sectors(fd, &sectors) < 0)
		err(निकास_त्रुटि, "blkdev_get_sectors() failed");
	जर (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(निकास_त्रुटि, "blkdev_get_sector_size() failed");
	जर (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(निकास_त्रुटि, "blkdev_get_physector_size() failed");

	म_लिखो("          bytes: %" PRIu64 "\n", bytes);
	म_लिखो("        sectors: %" PRIu64 "\n", sectors);
	म_लिखो("    sector size: %d\n", sector_size);
	म_लिखो("phy-sector size: %d\n", phy_sector_size);

	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_BLKDEV */
