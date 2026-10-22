/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن "c.h"
#ضمن "nls.h"

#ضمن "swapheader.h"
#ضمن "swapprober.h"

blkid_probe get_swap_prober(ثابت حرف *devname)
{
	blkid_probe pr;
	صحيح rc;
	ثابت حرف *version = عدم;
	حرف *swap_filter[] = { "swap", عدم };

	pr = blkid_new_probe_from_filename(devname);
	إذا (!pr) {
		warn(_("%s: unable to probe device"), devname);
		أرجع عدم;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	إذا (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	وإلا إذا (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	وإلا إذا (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	إذا (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		إذا (blkid_probe_lookup_قيمة(pr, "VERSION", &version, عدم) == 0
		    && version
		    && strcmp(version, stringify_قيمة(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		وإلا
			أرجع pr;
	}

	blkid_free_probe(pr);
	أرجع عدم;
}
