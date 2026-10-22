/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل "c.h"
#شامل "nls.h"

#شامل "swapheader.h"
#شامل "swapprober.h"

blkid_probe get_swap_prober(ثابت نویسه *devname)
{
	blkid_probe pr;
	صحیح rc;
	ثابت نویسه *version = پوچ;
	نویسه *swap_filter[] = { "swap", پوچ };

	pr = blkid_new_probe_from_filename(devname);
	اگر (!pr) {
		warn(_("%s: unable to probe device"), devname);
		بازگشت پوچ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	اگر (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	وگرنه اگر (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	وگرنه اگر (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	اگر (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		اگر (blkid_probe_lookup_مقدار(pr, "VERSION", &version, پوچ) == 0
		    && version
		    && strcmp(version, stringify_مقدار(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		وگرنه
			بازگشت pr;
	}

	blkid_free_probe(pr);
	بازگشت پوچ;
}
