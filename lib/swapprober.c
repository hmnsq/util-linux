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

blkid_probe get_swap_prober(अचर वर्ण *devname)
{
	blkid_probe pr;
	पूर्णाङ्क rc;
	अचर वर्ण *version = रिक्त;
	वर्ण *swap_filter[] = { "swap", रिक्त };

	pr = blkid_new_probe_from_filename(devname);
	यदि (!pr) {
		warn(_("%s: unable to probe device"), devname);
		फर्काऊ रिक्त;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	यदि (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	अन्यथा यदि (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	अन्यथा यदि (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	यदि (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		यदि (blkid_probe_lookup_मान(pr, "VERSION", &version, रिक्त) == 0
		    && version
		    && म_भेद(version, stringify_मान(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		अन्यथा
			फर्काऊ pr;
	}

	blkid_free_probe(pr);
	फर्काऊ रिक्त;
}
