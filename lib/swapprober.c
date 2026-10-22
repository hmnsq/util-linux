/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "nls.h"

#അന്തര്ഭാവ "swapheader.h"
#അന്തര്ഭാവ "swapprober.h"

blkid_probe get_swap_prober(സ്ഥിരം അക്ഷരം *devname)
{
	blkid_probe pr;
	പൂർണ്ണസംഖ്യ rc;
	സ്ഥിരം അക്ഷരം *version = ശൂന്യ;
	അക്ഷരം *swap_filter[] = { "swap", ശൂന്യ };

	pr = blkid_new_probe_from_filename(devname);
	എങ്കിൽ (!pr) {
		warn(_("%s: unable to probe device"), devname);
		തിരികെ ശൂന്യ;
	}

	blkid_probe_enable_superblocks(pr, TRUE);
	blkid_probe_set_superblocks_flags(pr,
			BLKID_SUBLKS_LABEL | BLKID_SUBLKS_UUID |
			BLKID_SUBLKS_VERSION);

	blkid_probe_filter_superblocks_type(pr, BLKID_FLTR_ONLYIN, swap_filter);

	rc = blkid_do_safeprobe(pr);
	എങ്കിൽ (rc == -1)
		warn(_("%s: unable to probe device"), devname);
	അല്ലെങ്കിൽ എങ്കിൽ (rc == -2)
		warnx(_("%s: ambiguous probing result; use wipefs(8)"), devname);
	അല്ലെങ്കിൽ എങ്കിൽ (rc == 1)
		warnx(_("%s: not a valid swap partition"), devname);

	എങ്കിൽ (rc == 0) {
		/* Only the SWAPSPACE2 is supported. */
		എങ്കിൽ (blkid_probe_lookup_മൂല്യം(pr, "VERSION", &version, ശൂന്യ) == 0
		    && version
		    && strcmp(version, stringify_മൂല്യം(SWAP_VERSION)) != 0)
			warnx(_("%s: unsupported swap version '%s'"),
						devname, version);
		അല്ലെങ്കിൽ
			തിരികെ pr;
	}

	blkid_free_probe(pr);
	തിരികെ ശൂന്യ;
}
