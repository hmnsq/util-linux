/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <sys/file.h>
#समावेश <sys/ioctl.h>
#समावेश <errno.h>
#समावेश <unistd.h>
#समावेश <stdint.h>

#अगर_घोषित HAVE_LINUX_FD_H
#समावेश <linux/fd.h>
#पूर्ण_अगर

#अगर_घोषित HAVE_LINUX_BLKZONED_H
#समावेश <linux/blkzoned.h>
#पूर्ण_अगर

#अगर_घोषित HAVE_SYS_DISKLABEL_H
#समावेश <sys/disklabel.h>
#पूर्ण_अगर

#अगर_घोषित HAVE_SYS_DISK_H
# include <sys/disk.h>
#पूर्ण_अगर

#अगर_अघोषित EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#पूर्ण_अगर

#समावेश "all-io.h"
#समावेश "blkdev.h"
#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "linux_version.h"
#समावेश "fileutils.h"
#समावेश "nls.h"

अटल दीर्घ blkdev_valid_offset(पूर्णांक fd, off_t offset)
{
	अक्षर ch;

	अगर (lseek (fd, offset, 0) < 0)
		वापस 0;
	अगर (ul_read_all (fd, &ch, 1) < 1)
		वापस 0;
	वापस 1;
}

पूर्णांक is_blkdev(पूर्णांक fd)
{
	काष्ठा stat st;
	वापस (ख_स्थिति(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_आकार (पूर्णांक fd)
{
	off_t high, निचला = 0;

	क्रम (high = 1024; blkdev_valid_offset(fd, high); ) {
		अगर (high == SINT_MAX(off_t)) {
			त्रुटि_सं = EFBIG;
			वापस -1;
		}

		निचला = high;

		अगर (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		अन्यथा
			high *= 2;
	}

	जबतक (निचला < high - 1) {
		off_t mid = (निचला + high) / 2;

		अगर (blkdev_valid_offset (fd, mid))
			निचला = mid;
		अन्यथा
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	वापस (निचला + 1);
}

/* get size in bytes */
पूर्णांक blkdev_get_आकार(पूर्णांक fd, uint64_t *bytes)
{
#अगर_घोषित DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	अगर (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		वापस 0;
	}
#पूर्ण_अगर

#अगर_घोषित BLKGETSIZE64
	अगर (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		वापस 0;
#पूर्ण_अगर

#अगर_घोषित BLKGETSIZE
	{
		अचिन्हित दीर्घ आकार;

		अगर (ioctl(fd, BLKGETSIZE, &आकार) >= 0) {
			*bytes = ((uint64_t) आकार) << 9;
			वापस 0;
		}
	}

#पूर्ण_अगर /* BLKGETSIZE */

#अगर_घोषित DIOCGMEDIASIZE
	/* FreeBSD */
	अगर (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		वापस 0;
#पूर्ण_अगर

#अगर_घोषित FDGETPRM
	{
		काष्ठा floppy_struct this_floppy;

		अगर (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.आकार) << 9;
			वापस 0;
		}
	}
#पूर्ण_अगर /* FDGETPRM */

#अगर defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		काष्ठा disklabel lab;
		काष्ठा partition *pp;
		काष्ठा stat st;

		अगर ((ख_स्थिति(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		अगर (part >= 0 && (ioctl(fd, DIOCGDINFO, (अक्षर *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			अगर (pp->p_आकार) {
				 *bytes = ((uint64_t) pp->p_आकार) << 9;
				 वापस 0;
			}
		}
	}
#पूर्ण_अगर /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		काष्ठा stat st;

		अगर (ख_स्थिति(fd, &st) != 0)
			वापस -1;
		अगर (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			वापस 0;
		}
		अगर (!S_ISBLK(st.st_mode)) {
			त्रुटि_सं = ENOTBLK;
			वापस -1;
		}
	}

	{
		off_t आकार = blkdev_find_आकार(fd);

		अगर (आकार < 0)
			वापस -1;

		*bytes = (uint64_t) आकार;
	}

	वापस 0;
}

/* get 512-byte sector count */
पूर्णांक blkdev_get_sectors(पूर्णांक fd, uint64_t *sectors)
{
	uint64_t bytes;

	अगर (blkdev_get_आकार(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		वापस 0;
	}

	वापस -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#अगर_घोषित BLKSSZGET
पूर्णांक blkdev_get_sector_आकार(पूर्णांक fd, पूर्णांक *sector_आकार)
{
	अगर (ioctl(fd, BLKSSZGET, sector_आकार) >= 0)
		वापस 0;
	वापस -1;
}
#अन्यथा
पूर्णांक blkdev_get_sector_आकार(पूर्णांक fd __attribute__((__unused__)), पूर्णांक *sector_आकार)
{
	*sector_आकार = DEFAULT_SECTOR_आकार;
	वापस 0;
}
#पूर्ण_अगर

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
#अगर_घोषित BLKPBSZGET
पूर्णांक blkdev_get_physector_आकार(पूर्णांक fd, पूर्णांक *sector_आकार)
{
	अगर (ioctl(fd, BLKPBSZGET, sector_आकार) >= 0)
		वापस 0;
	वापस -1;
}
#अन्यथा
पूर्णांक blkdev_get_physector_आकार(पूर्णांक fd __attribute__((__unused__)), पूर्णांक *sector_आकार)
{
	*sector_आकार = DEFAULT_SECTOR_आकार;
	वापस 0;
}
#पूर्ण_अगर

/*
 * Return the alignment status of a device
 */
#अगर_घोषित BLKALIGNOFF
पूर्णांक blkdev_is_misaligned(पूर्णांक fd)
{
	पूर्णांक aligned;

	अगर (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		वापस 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	वापस aligned != 0 ? 1 : 0;
}
#अन्यथा
पूर्णांक blkdev_is_misaligned(पूर्णांक fd __attribute__((__unused__)))
{
	वापस 0;
}
#पूर्ण_अगर

पूर्णांक open_blkdev_or_file(स्थिर काष्ठा stat *st, स्थिर अक्षर *name, स्थिर पूर्णांक oflag)
{
	पूर्णांक fd;

	अगर (S_ISBLK(st->st_mode)) {
		fd = खोलो(name, oflag | O_EXCL | O_CLOEXEC);
	} अन्यथा
		fd = खोलो(name, oflag);
	अगर (-1 < fd && !is_same_inode(fd, st)) {
		बंद(fd);
		त्रुटि_सं = EBADFD;
		वापस -1;
	}
	अगर (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	वापस fd;
}

#अगर_घोषित CDROM_GET_CAPABILITY
पूर्णांक blkdev_is_cdrom(पूर्णांक fd)
{
	पूर्णांक ret = ioctl(fd, CDROM_GET_CAPABILITY, शून्य);
	वापस ret < 0 ? 0 : ret;
}
#अन्यथा
पूर्णांक blkdev_is_cdrom(पूर्णांक fd __attribute__((__unused__)))
{
	वापस 0;
}
#पूर्ण_अगर

/*
 * Get kernel's interpretation of the device's geometry.
 *
 * Returns the heads and sectors - but not cylinders
 * as it's truncated for disks with more than 65535 tracks.
 *
 * Note that this is deprecated in favor of LBA addressing.
 */
#अगर_घोषित HDIO_GETGEO
पूर्णांक blkdev_get_geometry(पूर्णांक fd, अचिन्हित पूर्णांक *h, अचिन्हित पूर्णांक *s)
{
	काष्ठा hd_geometry geometry;

	अगर (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		वापस 0;
	}
#अन्यथा
पूर्णांक blkdev_get_geometry(पूर्णांक fd __attribute__((__unused__)),
		अचिन्हित पूर्णांक *h, अचिन्हित पूर्णांक *s)
{
	*h = 0;
	*s = 0;
#पूर्ण_अगर
	वापस -1;
}

/*
 * Convert scsi type to human readable string.
 */
स्थिर अक्षर *blkdev_scsi_type_to_name(पूर्णांक type)
{
	चयन (type) {
	हाल SCSI_TYPE_DISK:
		वापस "disk";
	हाल SCSI_TYPE_TAPE:
		वापस "tape";
	हाल SCSI_TYPE_PRINTER:
		वापस "printer";
	हाल SCSI_TYPE_PROCESSOR:
		वापस "processor";
	हाल SCSI_TYPE_WORM:
		वापस "worm";
	हाल SCSI_TYPE_ROM:
		वापस "rom";
	हाल SCSI_TYPE_SCANNER:
		वापस "scanner";
	हाल SCSI_TYPE_MOD:
		वापस "mo-disk";
	हाल SCSI_TYPE_MEDIUM_CHANGER:
		वापस "changer";
	हाल SCSI_TYPE_COMM:
		वापस "comm";
	हाल SCSI_TYPE_RAID:
		वापस "raid";
	हाल SCSI_TYPE_ENCLOSURE:
		वापस "enclosure";
	हाल SCSI_TYPE_RBC:
		वापस "rbc";
	हाल SCSI_TYPE_OSD:
		वापस "osd";
	हाल SCSI_TYPE_NO_LUN:
		वापस "no-lun";
	शेष:
		अवरोध;
	}
	वापस शून्य;
}

/* return 0 on success */
पूर्णांक blkdev_lock(पूर्णांक fd, स्थिर अक्षर *devname, स्थिर अक्षर *lockmode)
{
	पूर्णांक oper, rc, संदेश = 0;

	अगर (!lockmode)
		lockmode = दो_पर्या("LOCK_BLOCK_DEVICE");
	अगर (!lockmode)
		वापस 0;

	अगर (c_strcasecmp(lockmode, "yes") == 0 ||
	    म_भेद(lockmode, "1") == 0)
		oper = LOCK_EX;

	अन्यथा अगर (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	अन्यथा अगर (c_strcasecmp(lockmode, "no") == 0 ||
		 म_भेद(lockmode, "0") == 0)
		वापस 0;
	अन्यथा {
		warnx(_("unsupported lock mode: %s"), lockmode);
		वापस -EINVAL;
	}

	अगर (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		अगर (rc == 0)
			वापस 0;
		अगर (rc != 0 && त्रुटि_सं == EWOULDBLOCK) {
			ख_लिखो(मानक_त्रुटि, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			संदेश = 1;
		}
	}
	rc = flock(fd, oper);
	अगर (rc != 0) {
		चयन (त्रुटि_सं) {
		हाल EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			अवरोध;
		शेष:
			warn(_("%s: failed to get lock"), devname);
		}
	} अन्यथा अगर (संदेश)
		ख_लिखो(मानक_त्रुटि, _("OK\n"));
	वापस rc;
}

#अगर_घोषित HAVE_LINUX_BLKZONED_H
काष्ठा blk_zone_report *blkdev_get_zonereport(पूर्णांक fd, uint64_t sector, uint32_t nzones)
{
	काष्ठा blk_zone_report *rep;
	माप_प्रकार rep_आकार;
	पूर्णांक ret;

	rep_आकार = माप(काष्ठा blk_zone_report) + माप(काष्ठा blk_zone) * nzones;
	rep = सुस्मृति(1, rep_आकार);
	अगर (!rep)
		वापस शून्य;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	अगर (ret || rep->nr_zones != nzones) {
		मुक्त(rep);
		वापस शून्य;
	}

	वापस rep;
}
#पूर्ण_अगर


#अगर_घोषित TEST_PROGRAM_BLKDEV
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <fcntl.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	uint64_t bytes, sectors;
	पूर्णांक sector_आकार, phy_sector_आकार;
	पूर्णांक fd;

	अगर (argc != 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %s device\n", argv[0]);
		निकास(निकास_त्रुटि);
	}

	अगर ((fd = खोलो(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(निकास_त्रुटि, "open %s failed", argv[1]);

	अगर (blkdev_get_आकार(fd, &bytes) < 0)
		err(निकास_त्रुटि, "blkdev_get_size() failed");
	अगर (blkdev_get_sectors(fd, &sectors) < 0)
		err(निकास_त्रुटि, "blkdev_get_sectors() failed");
	अगर (blkdev_get_sector_आकार(fd, &sector_आकार) < 0)
		err(निकास_त्रुटि, "blkdev_get_sector_size() failed");
	अगर (blkdev_get_physector_आकार(fd, &phy_sector_आकार) < 0)
		err(निकास_त्रुटि, "blkdev_get_physector_size() failed");

	म_लिखो("          bytes: %" PRIu64 "\n", bytes);
	म_लिखो("        sectors: %" PRIu64 "\n", sectors);
	म_लिखो("    sector size: %d\n", sector_आकार);
	म_लिखो("phy-sector size: %d\n", phy_sector_आकार);

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_BLKDEV */
