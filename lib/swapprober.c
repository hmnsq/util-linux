/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव "c.h"
#अन्तर्भाव "nls.h"

#अन्तर्भाव "swapheader.h"
#अन्तर्भाव "swapprober.h"

blkid_probe get_swap_prober(ध्रुव वर्ण *devname)
{
	blkid_probe pr;
	पूर्णाङ्क rc;
	ध्रुव वर्ण *version = रिक्त;
	वर्ण *swap_filter[] = { "swap", रिक्त };

	pr = blkid_new_probe_from_filename(devname);
	सचे (!pr) {
		warn(_("%s: unable to probe device"), devname);
		पटिनिवत्त रिक्त;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	सचे (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	अञ्ञथा सचे (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	अञ्ञथा सचे (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	सचे (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		सचे (blkid_probe_lookup_मानम्(pr, "VERSION", &version, रिक्त) == 0
		    && version
		    && strcmp(version, stringify_मानम्(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		अञ्ञथा
			पटिनिवत्त pr;
	}

	blkid_free_probe(pr);
	पटिनिवत्त रिक्त;
}
