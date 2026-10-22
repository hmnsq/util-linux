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

blkid_probe get_ಅದಲುಬದಲು_prober(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *devname)
{
	blkid_probe pr;
	ಪೂರ್ಣಾಂಕ rc;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆವೃತ್ತಿ = ಖಾಲಿ;
	ಅಕ್ಷರ *ಅದಲುಬದಲು_filter[] = { "swap", ಖಾಲಿ };

	pr = blkid_new_probe_from_filename(devname);
	ಆದರೆ (!pr) {
		warn(_("%s: unable to probe device"), devname);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_ಗಣ_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_ಆವೃತ್ತಿ);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, ಅದಲುಬದಲು_filter);

	rc = blkid_do_safeprobe(pr);
	ಆದರೆ (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ಆದರೆ (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ಆದರೆ (blkid_probe_lookup_ಮೌಲ್ಯ(pr, "VERSION", &ಆವೃತ್ತಿ, ಖಾಲಿ) == 0
		    && ಆವೃತ್ತಿ
		    && strcmp(ಆವೃತ್ತಿ, stringify_ಮೌಲ್ಯ(ಅದಲುಬದಲು_ಆವೃತ್ತಿ)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, ಆವೃತ್ತಿ);
		ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ pr;
	}

	blkid_free_probe(pr);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
