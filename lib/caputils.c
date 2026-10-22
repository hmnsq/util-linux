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

static số_nguyên test_cap(unsigned số_nguyên cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	trả_về prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

static số_nguyên cap_last_by_bsearch(số_nguyên *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	số_nguyên cap = INT_MAX;
	unsigned số_nguyên cap0 = 0, cap1 = INT_MAX;

	trong_khi ((số_nguyên)cap0 < cap) {
		nếu (test_cap(cap))
			cap0 = cap;
		không_thì
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	trả_về 0;
}

static số_nguyên cap_last_by_procfs(số_nguyên *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	số_nguyên rc = -EINVAL;

	*ret = 0;

	nếu (f && fd_is_procfs(fileno(f))) {
		số_nguyên cap;

		/* we check if the cap after this one really isn't valid */
		nếu (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	nếu (f)
		fclose(f);
	trả_về rc;
}

số_nguyên cap_last_cap(void)
{
	static số_nguyên cap = -1;

	nếu (cap != -1)
		trả_về cap;
	nếu (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	trả_về cap;
}

void cap_permitted_to_ambient(void)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	struct __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	struct __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	nếu (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	nếu (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	cho (cap = 0; cap < (sizeof(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		nếu (cap > (uint64_t) cap_last_cap())
			continue;

		nếu ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
số_nguyên main(số_nguyên argc, char *argv[])
{
	số_nguyên rc = 0, cap;

	nếu (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		trả_về EXIT_FAILURE;
	}

	nếu (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		nếu (rc == 0)
			printf("last cap: %d\n", cap);

	} không_thì nếu (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		nếu (rc == 0)
			printf("last cap: %d\n", cap);

	} không_thì nếu (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	trả_về rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
