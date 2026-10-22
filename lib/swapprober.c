/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ "c.h"
#అన్తర్భావ "nls.h"

#అన్తర్భావ "swapheader.h"
#అన్తర్భావ "swapprober.h"

blkid_probe get_మార్పిడి_prober(స్థిరరాశి అక్షరం *devname)
{
	blkid_probe pr;
	పూర్ణాంకం rc;
	స్థిరరాశి అక్షరం *సంస్కరణ = ఖాళీ;
	అక్షరం *మార్పిడి_filter[] = { "swap", ఖాళీ };

	pr = blkid_new_probe_from_filename(devname);
	ఒకవేళ (!pr) {
		warn(_("%s: unable to probe device"), devname);
		తిరిగి ఖాళీ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_సమితి_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_సంస్కరణ);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, మార్పిడి_filter);

	rc = blkid_do_safeprobe(pr);
	ఒకవేళ (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	లేకపోతే ఒకవేళ (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	లేకపోతే ఒకవేళ (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	ఒకవేళ (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		ఒకవేళ (blkid_probe_lookup_విలువ(pr, "VERSION", &సంస్కరణ, ఖాళీ) == 0
		    && సంస్కరణ
		    && strcmp(సంస్కరణ, stringify_విలువ(మార్పిడి_సంస్కరణ)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, సంస్కరణ);
		లేకపోతే
			తిరిగి pr;
	}

	blkid_free_probe(pr);
	తిరిగి ఖాళీ;
}
