/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "nls.h"

#অন্তর্ভাৱ "swapheader.h"
#অন্তর্ভাৱ "swapprober.h"

blkid_probe get_swap_prober(ধ্রুৱ ৱর্ণ *devname)
{
	blkid_probe pr;
	পূর্ণাঙ্ক rc;
	ধ্রুৱ ৱর্ণ *version = রিক্ত;
	ৱর্ণ *swap_filter[] = { "swap", রিক্ত };

	pr = blkid_new_probe_from_filename(devname);
	যদি (!pr) {
		warn(_("%s: unable to probe device"), devname);
		ঘূৰাই রিক্ত;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	যদি (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	অন্যথা যদি (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	অন্যথা যদি (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	যদি (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		যদি (blkid_probe_lookup_মান(pr, "VERSION", &version, রিক্ত) == 0
		    && version
		    && strcmp(version, stringify_মান(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		অন্যথা
			ঘূৰাই pr;
	}

	blkid_free_probe(pr);
	ঘূৰাই রিক্ত;
}
