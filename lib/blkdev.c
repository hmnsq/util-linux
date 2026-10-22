/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdint.h>

#ifdef HAVE_LINUX_FD_H
#അന്തര്ഭാവ <linux/fd.h>
#endif

#ifdef HAVE_LINUX_BLKZONED_H
#അന്തര്ഭാവ <linux/blkzoned.h>
#endif

#ifdef HAVE_SYS_DISKLABEL_H
#അന്തര്ഭാവ <sys/disklabel.h>
#endif

#ifdef HAVE_SYS_DISK_H
# include <sys/disk.h>
#endif

#ifndef EBADFD
# define EBADFD 77		/* File descriptor in bad state */
#endif

#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "blkdev.h"
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "linux_version.h"
#അന്തര്ഭാവ "fileutils.h"
#അന്തര്ഭാവ "nls.h"

സ്ഥിതം long blkdev_valid_offset(പൂർണ്ണസംഖ്യ fd, off_t offset)
{
	അക്ഷരം ch;

	എങ്കിൽ (lseek (fd, offset, 0) < 0)
		തിരികെ 0;
	എങ്കിൽ (ul_read_all (fd, &ch, 1) < 1)
		തിരികെ 0;
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ is_blkdev(പൂർണ്ണസംഖ്യ fd)
{
	ഘടന stat st;
	തിരികെ (fstat(fd, &st) == 0 && S_ISBLK(st.st_mode));
}

off_t blkdev_find_size (പൂർണ്ണസംഖ്യ fd)
{
	off_t high, താഴെയുള്ള = 0;

	വേണ്ടി (high = 1024; blkdev_valid_offset(fd, high); ) {
		എങ്കിൽ (high == SINT_MAX(off_t)) {
			errno = EFBIG;
			തിരികെ -1;
		}

		താഴെയുള്ള = high;

		എങ്കിൽ (high >= SINT_MAX(off_t)/2)
			high = SINT_MAX(off_t);
		അല്ലെങ്കിൽ
			high *= 2;
	}

	ആകുമ്പോൾ (താഴെയുള്ള < high - 1) {
		off_t mid = (താഴെയുള്ള + high) / 2;

		എങ്കിൽ (blkdev_valid_offset (fd, mid))
			താഴെയുള്ള = mid;
		അല്ലെങ്കിൽ
			high = mid;
	}
	blkdev_valid_offset (fd, 0);
	തിരികെ (താഴെയുള്ള + 1);
}

/* get size in bytes */
പൂർണ്ണസംഖ്യ blkdev_get_size(പൂർണ്ണസംഖ്യ fd, uint64_t *bytes)
{
#ifdef DKIOCGETBLOCKCOUNT
	/* Apple Darwin */
	എങ്കിൽ (ioctl(fd, DKIOCGETBLOCKCOUNT, bytes) >= 0) {
		*bytes <<= 9;
		തിരികെ 0;
	}
#endif

#ifdef BLKGETSIZE64
	എങ്കിൽ (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		തിരികെ 0;
#endif

#ifdef BLKGETSIZE
	{
		unsigned long size;

		എങ്കിൽ (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((uint64_t) size) << 9;
			തിരികെ 0;
		}
	}

#endif /* BLKGETSIZE */

#ifdef DIOCGMEDIASIZE
	/* FreeBSD */
	എങ്കിൽ (ioctl(fd, DIOCGMEDIASIZE, bytes) >= 0)
		തിരികെ 0;
#endif

#ifdef FDGETPRM
	{
		ഘടന floppy_struct this_floppy;

		എങ്കിൽ (ioctl(fd, FDGETPRM, &this_floppy) >= 0) {
			*bytes = ((uint64_t) this_floppy.size) << 9;
			തിരികെ 0;
		}
	}
#endif /* FDGETPRM */

#എങ്കിൽ defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO)
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
		പൂർണ്ണസംഖ്യ part = -1;
		ഘടന disklabel lab;
		ഘടന partition *pp;
		ഘടന stat st;

		എങ്കിൽ ((fstat(fd, &st) >= 0) &&
		    (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
			part = st.st_rdev & 7;

		എങ്കിൽ (part >= 0 && (ioctl(fd, DIOCGDINFO, (അക്ഷരം *)&lab) >= 0)) {
			pp = &lab.d_partitions[part];
			എങ്കിൽ (pp->p_size) {
				 *bytes = ((uint64_t) pp->p_size) << 9;
				 തിരികെ 0;
			}
		}
	}
#endif /* defined(HAVE_SYS_DISKLABEL_H) && defined(DIOCGDINFO) */

	{
		ഘടന stat st;

		എങ്കിൽ (fstat(fd, &st) != 0)
			തിരികെ -1;
		എങ്കിൽ (S_ISREG(st.st_mode)) {
			*bytes = (uint64_t) st.st_size;
			തിരികെ 0;
		}
		എങ്കിൽ (!S_ISBLK(st.st_mode)) {
			errno = ENOTBLK;
			തിരികെ -1;
		}
	}

	{
		off_t size = blkdev_find_size(fd);

		എങ്കിൽ (size < 0)
			തിരികെ -1;

		*bytes = (uint64_t) size;
	}

	തിരികെ 0;
}

/* get 512-byte sector count */
പൂർണ്ണസംഖ്യ blkdev_get_sectors(പൂർണ്ണസംഖ്യ fd, uint64_t *sectors)
{
	uint64_t bytes;

	എങ്കിൽ (blkdev_get_size(fd, &bytes) == 0) {
		*sectors = (bytes >> 9);
		തിരികെ 0;
	}

	തിരികെ -1;
}

/*
 * Get logical sector size.
 *
 * This is the smallest unit the storage device can
 * address. It is typically 512 bytes.
 */
#ifdef BLKSSZGET
പൂർണ്ണസംഖ്യ blkdev_get_sector_size(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ *sector_size)
{
	എങ്കിൽ (ioctl(fd, BLKSSZGET, sector_size) >= 0)
		തിരികെ 0;
	തിരികെ -1;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ blkdev_get_sector_size(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)), പൂർണ്ണസംഖ്യ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ blkdev_get_physector_size(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ *sector_size)
{
	എങ്കിൽ (ioctl(fd, BLKPBSZGET, sector_size) >= 0)
		തിരികെ 0;
	തിരികെ -1;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ blkdev_get_physector_size(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)), പൂർണ്ണസംഖ്യ *sector_size)
{
	*sector_size = DEFAULT_SECTOR_SIZE;
	തിരികെ 0;
}
#endif

/*
 * Return the alignment status of a device
 */
#ifdef BLKALIGNOFF
പൂർണ്ണസംഖ്യ blkdev_is_misaligned(പൂർണ്ണസംഖ്യ fd)
{
	പൂർണ്ണസംഖ്യ aligned;

	എങ്കിൽ (ioctl(fd, BLKALIGNOFF, &aligned) < 0)
		തിരികെ 0; /* probably kernel < 2.6.32 */
	/*
	 * Note that kernel returns -1 as alignment offset if no compatible
	 * sizes and alignments exist for stacked devices
	 */
	തിരികെ aligned != 0 ? 1 : 0;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ blkdev_is_misaligned(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)))
{
	തിരികെ 0;
}
#endif

പൂർണ്ണസംഖ്യ open_blkdev_or_file(സ്ഥിരം ഘടന stat *st, സ്ഥിരം അക്ഷരം *name, സ്ഥിരം പൂർണ്ണസംഖ്യ oflag)
{
	പൂർണ്ണസംഖ്യ fd;

	എങ്കിൽ (S_ISBLK(st->st_mode)) {
		fd = open(name, oflag | O_EXCL | O_CLOEXEC);
	} അല്ലെങ്കിൽ
		fd = open(name, oflag);
	എങ്കിൽ (-1 < fd && !is_same_inode(fd, st)) {
		close(fd);
		errno = EBADFD;
		തിരികെ -1;
	}
	എങ്കിൽ (-1 < fd && S_ISBLK(st->st_mode) && blkdev_is_misaligned(fd))
		warnx(_("warning: %s is misaligned"), name);
	തിരികെ fd;
}

#ifdef CDROM_GET_CAPABILITY
പൂർണ്ണസംഖ്യ blkdev_is_cdrom(പൂർണ്ണസംഖ്യ fd)
{
	പൂർണ്ണസംഖ്യ ret = ioctl(fd, CDROM_GET_CAPABILITY, ശൂന്യ);
	തിരികെ ret < 0 ? 0 : ret;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ blkdev_is_cdrom(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)))
{
	തിരികെ 0;
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
പൂർണ്ണസംഖ്യ blkdev_get_geometry(പൂർണ്ണസംഖ്യ fd, unsigned പൂർണ്ണസംഖ്യ *h, unsigned പൂർണ്ണസംഖ്യ *s)
{
	ഘടന hd_geometry geometry;

	എങ്കിൽ (ioctl(fd, HDIO_GETGEO, &geometry) == 0) {
		*h = geometry.heads;
		*s = geometry.sectors;
		തിരികെ 0;
	}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ blkdev_get_geometry(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)),
		unsigned പൂർണ്ണസംഖ്യ *h, unsigned പൂർണ്ണസംഖ്യ *s)
{
	*h = 0;
	*s = 0;
#endif
	തിരികെ -1;
}

/*
 * Convert scsi type to human readable string.
 */
സ്ഥിരം അക്ഷരം *blkdev_scsi_type_to_name(പൂർണ്ണസംഖ്യ type)
{
	തിരഞ്ഞെടുപ്പ് (type) {
	അവസ്ഥ SCSI_TYPE_DISK:
		തിരികെ "disk";
	അവസ്ഥ SCSI_TYPE_TAPE:
		തിരികെ "tape";
	അവസ്ഥ SCSI_TYPE_PRINTER:
		തിരികെ "printer";
	അവസ്ഥ SCSI_TYPE_PROCESSOR:
		തിരികെ "processor";
	അവസ്ഥ SCSI_TYPE_WORM:
		തിരികെ "worm";
	അവസ്ഥ SCSI_TYPE_ROM:
		തിരികെ "rom";
	അവസ്ഥ SCSI_TYPE_SCANNER:
		തിരികെ "scanner";
	അവസ്ഥ SCSI_TYPE_MOD:
		തിരികെ "mo-disk";
	അവസ്ഥ SCSI_TYPE_MEDIUM_CHANGER:
		തിരികെ "changer";
	അവസ്ഥ SCSI_TYPE_COMM:
		തിരികെ "comm";
	അവസ്ഥ SCSI_TYPE_RAID:
		തിരികെ "raid";
	അവസ്ഥ SCSI_TYPE_ENCLOSURE:
		തിരികെ "enclosure";
	അവസ്ഥ SCSI_TYPE_RBC:
		തിരികെ "rbc";
	അവസ്ഥ SCSI_TYPE_OSD:
		തിരികെ "osd";
	അവസ്ഥ SCSI_TYPE_NO_LUN:
		തിരികെ "no-lun";
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}
	തിരികെ ശൂന്യ;
}

/* return 0 on success */
പൂർണ്ണസംഖ്യ blkdev_lock(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *devname, സ്ഥിരം അക്ഷരം *lockmode)
{
	പൂർണ്ണസംഖ്യ oper, rc, സന്ദേശം = 0;

	എങ്കിൽ (!lockmode)
		lockmode = getenv("LOCK_BLOCK_DEVICE");
	എങ്കിൽ (!lockmode)
		തിരികെ 0;

	എങ്കിൽ (c_strcasecmp(lockmode, "yes") == 0 ||
	    strcmp(lockmode, "1") == 0)
		oper = LOCK_EX;

	അല്ലെങ്കിൽ എങ്കിൽ (c_strcasecmp(lockmode, "nonblock") == 0)
		oper = LOCK_EX | LOCK_NB;

	അല്ലെങ്കിൽ എങ്കിൽ (c_strcasecmp(lockmode, "no") == 0 ||
		 strcmp(lockmode, "0") == 0)
		തിരികെ 0;
	അല്ലെങ്കിൽ {
		warnx(_("unsupported lock mode: %s"), lockmode);
		തിരികെ -EINVAL;
	}

	എങ്കിൽ (!(oper & LOCK_NB)) {
		/* Try non-block first to provide message */
		rc = flock(fd, oper | LOCK_NB);
		എങ്കിൽ (rc == 0)
			തിരികെ 0;
		എങ്കിൽ (rc != 0 && errno == EWOULDBLOCK) {
			fprintf(stderr, _("%s: %s: device already locked, waiting to get lock ... "),
					program_invocation_short_name, devname);
			സന്ദേശം = 1;
		}
	}
	rc = flock(fd, oper);
	എങ്കിൽ (rc != 0) {
		തിരഞ്ഞെടുപ്പ് (errno) {
		അവസ്ഥ EWOULDBLOCK: /* LOCK_NB */
			warnx(_("%s: device already locked"), devname);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			warn(_("%s: failed to get lock"), devname);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (സന്ദേശം)
		fprintf(stderr, _("OK\n"));
	തിരികെ rc;
}

#ifdef HAVE_LINUX_BLKZONED_H
ഘടന blk_zone_report *blkdev_get_zonereport(പൂർണ്ണസംഖ്യ fd, uint64_t sector, uint32_t nzones)
{
	ഘടന blk_zone_report *rep;
	size_t rep_size;
	പൂർണ്ണസംഖ്യ ret;

	rep_size = വലുപ്പം(ഘടന blk_zone_report) + വലുപ്പം(ഘടന blk_zone) * nzones;
	rep = calloc(1, rep_size);
	എങ്കിൽ (!rep)
		തിരികെ ശൂന്യ;

	rep->sector = sector;
	rep->nr_zones = nzones;

	ret = ioctl(fd, BLKREPORTZONE, rep);
	എങ്കിൽ (ret || rep->nr_zones != nzones) {
		സ്വതന്ത്രം(rep);
		തിരികെ ശൂന്യ;
	}

	തിരികെ rep;
}
#endif


#ifdef TEST_PROGRAM_BLKDEV
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <fcntl.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	uint64_t bytes, sectors;
	പൂർണ്ണസംഖ്യ sector_size, phy_sector_size;
	പൂർണ്ണസംഖ്യ fd;

	എങ്കിൽ (argc != 2) {
		fprintf(stderr, "usage: %s device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	എങ്കിൽ ((fd = open(argv[1], O_RDONLY|O_CLOEXEC)) < 0)
		err(EXIT_FAILURE, "open %s failed", argv[1]);

	എങ്കിൽ (blkdev_get_size(fd, &bytes) < 0)
		err(EXIT_FAILURE, "blkdev_get_size() failed");
	എങ്കിൽ (blkdev_get_sectors(fd, &sectors) < 0)
		err(EXIT_FAILURE, "blkdev_get_sectors() failed");
	എങ്കിൽ (blkdev_get_sector_size(fd, &sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_sector_size() failed");
	എങ്കിൽ (blkdev_get_physector_size(fd, &phy_sector_size) < 0)
		err(EXIT_FAILURE, "blkdev_get_physector_size() failed");

	printf("          bytes: %" PRIu64 "\n", bytes);
	printf("        sectors: %" PRIu64 "\n", sectors);
	printf("    sector size: %d\n", sector_size);
	printf("phy-sector size: %d\n", phy_sector_size);

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_BLKDEV */
