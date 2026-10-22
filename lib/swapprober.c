/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include "c.h"
#include "nls.h"

#include "swapheader.h"
#include "swapprober.h"

blkid_probe get_swap_prober(const char *devname)
{
	blkid_probe pr;
	цяло rc;
	const char *version = NULL;
	char *swap_filter[] = { "swap", NULL };

	pr = blkid_new_probe_from_filename(devname);
	ако (!pr) {
		warn(_("%s: unable to probe device"), devname);
		върни NULL;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	ако (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	иначе ако (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	иначе ако (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ако (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ако (blkid_probe_lookup_value(pr, "VERSION", &version, NULL) == 0
		    && version
		    && strcmp(version, stringify_value(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		иначе
			върни pr;
	}

	blkid_free_probe(pr);
	върни NULL;
}
