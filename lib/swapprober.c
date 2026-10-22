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
	整数 rc;
	const char *version = NULL;
	char *swap_filter[] = { "swap", NULL };

	pr = blkid_new_probe_from_filename(devname);
	如果 (!pr) {
		warn(_("%s: unable to probe device"), devname);
		返回 NULL;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	如果 (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	否则 如果 (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	否则 如果 (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	如果 (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		如果 (blkid_probe_lookup_value(pr, "VERSION", &version, NULL) == 0
		    && version
		    && strcmp(version, stringify_value(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		否则
			返回 pr;
	}

	blkid_free_probe(pr);
	返回 NULL;
}
