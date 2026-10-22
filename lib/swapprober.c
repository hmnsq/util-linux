/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश "c.h"
#समावेश "nls.h"

#समावेश "swapheader.h"
#समावेश "swapprober.h"

blkid_probe get_swap_prober(स्थिर अक्षर *devname)
{
	blkid_probe pr;
	पूर्णांक rc;
	स्थिर अक्षर *version = शून्य;
	अक्षर *swap_filter[] = { "swap", शून्य };

	pr = blkid_new_probe_from_filename(devname);
	अगर (!pr) {
		warn(_("%s: unable to probe device"), devname);
		वापस शून्य;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	अगर (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	अन्यथा अगर (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	अन्यथा अगर (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	अगर (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		अगर (blkid_probe_lookup_मूल्य(pr, "VERSION", &version, शून्य) == 0
		    && version
		    && म_भेद(version, stringify_मूल्य(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		अन्यथा
			वापस pr;
	}

	blkid_free_probe(pr);
	वापस शून्य;
}
