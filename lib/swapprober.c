/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"

#ଅନ୍ତର୍ଭାଵ "swapheader.h"
#ଅନ୍ତର୍ଭାଵ "swapprober.h"

blkid_probe get_swap_prober(ଅଚଳ ଅକ୍ଷର *devname)
{
	blkid_probe pr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ଅଚଳ ଅକ୍ଷର *version = ଖାଲି;
	ଅକ୍ଷର *swap_filter[] = { "swap", ଖାଲି };

	pr = blkid_new_probe_from_filename(devname);
	ଯଦି (!pr) {
		warn(_("%s: unable to probe device"), devname);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	ଯଦି (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	ଅନ୍ୟଥା ଯଦି (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	ଅନ୍ୟଥା ଯଦି (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ଯଦି (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ଯଦି (blkid_probe_lookup_ମୂଲ୍ୟ(pr, "VERSION", &version, ଖାଲି) == 0
		    && version
		    && strcmp(version, stringify_ମୂଲ୍ୟ(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		ଅନ୍ୟଥା
			ଫେରନ୍ତୁ pr;
	}

	blkid_free_probe(pr);
	ଫେରନ୍ତୁ ଖାଲି;
}
