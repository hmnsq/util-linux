/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "nls.h"

#அந்தர்பாவ "swapheader.h"
#அந்தர்பாவ "swapprober.h"

blkid_probe get_swap_prober(மாறிலி எழுத்து *devname)
{
	blkid_probe pr;
	முழு rc;
	மாறிலி எழுத்து *version = வெற்று;
	எழுத்து *swap_filter[] = { "swap", வெற்று };

	pr = blkid_new_probe_from_filename(devname);
	எனில் (!pr) {
		warn(_("%s: unable to probe device"), devname);
		திரும்பு வெற்று;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	எனில் (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	இல்லையெனில் எனில் (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	இல்லையெனில் எனில் (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	எனில் (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		எனில் (blkid_probe_lookup_மதிப்பு(pr, "VERSION", &version, வெற்று) == 0
		    && version
		    && strcmp(version, stringify_மதிப்பு(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		இல்லையெனில்
			திரும்பு pr;
	}

	blkid_free_probe(pr);
	திரும்பு வெற்று;
}
