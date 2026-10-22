/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "nls.h"

#અન્તર્ભાવ "swapheader.h"
#અન્તર્ભાવ "swapprober.h"

blkid_probe get_swap_prober(અચલ અક્ષર *devname)
{
	blkid_probe pr;
	પૂર્ણાંક rc;
	અચલ અક્ષર *version = ખાલી;
	અક્ષર *swap_filter[] = { "swap", ખાલી };

	pr = blkid_new_probe_from_filename(devname);
	જો (!pr) {
		warn(_("%s: unable to probe device"), devname);
		પાછા ખાલી;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	જો (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	નહીંતર જો (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	નહીંતર જો (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	જો (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		જો (blkid_probe_lookup_કિંમત(pr, "VERSION", &version, ખાલી) == 0
		    && version
		    && strcmp(version, stringify_કિંમત(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		નહીંતર
			પાછા pr;
	}

	blkid_free_probe(pr);
	પાછા ખાલી;
}
