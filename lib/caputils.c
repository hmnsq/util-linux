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

#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "c.h"
#include "caputils.h"
#include "pathnames.h"
#include "procfs.h"
#include "nls.h"

static puur_naa_nka test_cap(unsigned puur_naa_nka cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ꦮꦭꦶ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

static puur_naa_nka cap_last_by_bsearch(puur_naa_nka *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	puur_naa_nka cap = INT_MAX;
	unsigned puur_naa_nka cap0 = 0, cap1 = INT_MAX;

	ꦱꦼꦢꦼꦭ ((puur_naa_nka)cap0 < cap) {
		ꦪꦼꦤ꧀ (test_cap(cap))
			cap0 = cap;
		ꦭꦶꦪ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ꦮꦭꦶ 0;
}

static puur_naa_nka cap_last_by_procfs(puur_naa_nka *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	puur_naa_nka rc = -EINVAL;

	*ret = 0;

	ꦪꦼꦤ꧀ (f && fd_is_procfs(fileno(f))) {
		puur_naa_nka cap;

		/* we check if the cap after this one really isn't valid */
		ꦪꦼꦤ꧀ (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ꦪꦼꦤ꧀ (f)
		fclose(f);
	ꦮꦭꦶ rc;
}

puur_naa_nka cap_last_cap(void)
{
	static puur_naa_nka cap = -1;

	ꦪꦼꦤ꧀ (cap != -1)
		ꦮꦭꦶ cap;
	ꦪꦼꦤ꧀ (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	ꦮꦭꦶ cap;
}

void cap_permitted_to_ambient(void)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	sa_nracanaa __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	sa_nracanaa __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ꦪꦼꦤ꧀ (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ꦪꦼꦤ꧀ (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	ꦏꦁꦒꦺꦴ (cap = 0; cap < (parimaa_na(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ꦪꦼꦤ꧀ (cap > (uint64_t) cap_last_cap())
			continue;

		ꦪꦼꦤ꧀ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka rc = 0, cap;

	ꦪꦼꦤ꧀ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		ꦮꦭꦶ EXIT_FAILURE;
	}

	ꦪꦼꦤ꧀ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		ꦪꦼꦤ꧀ (rc == 0)
			printf("last cap: %d\n", cap);

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		ꦪꦼꦤ꧀ (rc == 0)
			printf("last cap: %d\n", cap);

	} ꦭꦶꦪ ꦪꦼꦤ꧀ (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	ꦮꦭꦶ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
