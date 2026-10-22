/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "nls.h"

#ಅನ್ತರ್ಭಾವ "swapheader.h"
#ಅನ್ತರ್ಭಾವ "swapprober.h"

blkid_probe get_swap_prober(ಧ್ರುವ ವರ್ಣ *devname)
{
	blkid_probe pr;
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಧ್ರುವ ವರ್ಣ *version = ರಿಕ್ತ;
	ವರ್ಣ *swap_filter[] = { "swap", ರಿಕ್ತ };

	pr = blkid_new_probe_from_filename(devname);
	ಯದಿ (!pr) {
		warn(_("%s: unable to probe device"), devname);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	ಯದಿ (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	ಅನ್ಯಥಾ ಯದಿ (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	ಅನ್ಯಥಾ ಯದಿ (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ಯದಿ (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ಯದಿ (blkid_probe_lookup_ಮೌಲ್ಯ(pr, "VERSION", &version, ರಿಕ್ತ) == 0
		    && version
		    && strcmp(version, stringify_ಮೌಲ್ಯ(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ pr;
	}

	blkid_free_probe(pr);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}
