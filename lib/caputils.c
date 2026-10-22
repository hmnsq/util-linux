/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 */

#அந்தர்பாவ <sys/prctl.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "caputils.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "procfs.h"
#அந்தர்பாவ "nls.h"

நிலையான முழு test_cap(unsigned முழு cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	திரும்பு prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

நிலையான முழு cap_last_by_bsearch(முழு *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	முழு cap = INT_MAX;
	unsigned முழு cap0 = 0, cap1 = INT_MAX;

	போது ((முழு)cap0 < cap) {
		எனில் (test_cap(cap))
			cap0 = cap;
		இல்லையெனில்
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	திரும்பு 0;
}

நிலையான முழு cap_last_by_procfs(முழு *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	முழு rc = -EINVAL;

	*ret = 0;

	எனில் (f && fd_is_procfs(fileno(f))) {
		முழு cap;

		/* we check if the cap after this one really isn't valid */
		எனில் (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	எனில் (f)
		fclose(f);
	திரும்பு rc;
}

முழு cap_last_cap(வெறுமை)
{
	நிலையான முழு cap = -1;

	எனில் (cap != -1)
		திரும்பு cap;
	எனில் (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	திரும்பு cap;
}

வெறுமை cap_permitted_to_ambient(வெறுமை)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	கட்டமைப்பு __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	கட்டமைப்பு __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	எனில் (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	எனில் (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	க்கு (cap = 0; cap < (அளவு(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		எனில் (cap > (uint64_t) cap_last_cap())
			தொடர்;

		எனில் ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	முழு rc = 0, cap;

	எனில் (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		திரும்பு EXIT_FAILURE;
	}

	எனில் (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		எனில் (rc == 0)
			printf("last cap: %d\n", cap);

	} இல்லையெனில் எனில் (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		எனில் (rc == 0)
			printf("last cap: %d\n", cap);

	} இல்லையெனில் எனில் (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	திரும்பு rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
