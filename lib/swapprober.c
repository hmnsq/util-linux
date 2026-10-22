/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "swapheader.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "swapprober.h"

blkid_probe get_ਅਦਲਾ_ਬਦਲੀ_prober(ਅਚਲ ਅੱਖਰ *devname)
{
	blkid_probe pr;
	ਪੂਰਨ rc;
	ਅਚਲ ਅੱਖਰ *ਸੰਸਕਰਣ = ਨਿਰਾ;
	ਅੱਖਰ *ਅਦਲਾ_ਬਦਲੀ_filter[] = { "swap", ਨਿਰਾ };

	pr = blkid_new_probe_from_filename(devname);
	ਜੇ (!pr) {
		warn(_("%s: unable to probe device"), devname);
		ਵਾਪਸ ਨਿਰਾ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_ਸੈੱਟ_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_ਸੰਸਕਰਣ);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, ਅਦਲਾ_ਬਦਲੀ_filter);

	rc = blkid_do_safeprobe(pr);
	ਜੇ (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	ਨਹੀਂਤਾਂ ਜੇ (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	ਨਹੀਂਤਾਂ ਜੇ (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ਜੇ (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ਜੇ (blkid_probe_lookup_ਮੁੱਲ(pr, "VERSION", &ਸੰਸਕਰਣ, ਨਿਰਾ) == 0
		    && ਸੰਸਕਰਣ
		    && strcmp(ਸੰਸਕਰਣ, stringify_ਮੁੱਲ(ਅਦਲਾ_ਬਦਲੀ_ਸੰਸਕਰਣ)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, ਸੰਸਕਰਣ);
		ਨਹੀਂਤਾਂ
			ਵਾਪਸ pr;
	}

	blkid_free_probe(pr);
	ਵਾਪਸ ਨਿਰਾ;
}
