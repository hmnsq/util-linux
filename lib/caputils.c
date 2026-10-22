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

static ကိန်းပြည့် test_cap(unsigned ကိန်းပြည့် cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ပြန်ပေး prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

static ကိန်းပြည့် cap_last_by_bsearch(ကိန်းပြည့် *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	ကိန်းပြည့် cap = INT_MAX;
	unsigned ကိန်းပြည့် cap0 = 0, cap1 = INT_MAX;

	နေစဉ် ((ကိန်းပြည့်)cap0 < cap) {
		အကယ်၍ (test_cap(cap))
			cap0 = cap;
		မဟုတ်ပါက
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ပြန်ပေး 0;
}

static ကိန်းပြည့် cap_last_by_procfs(ကိန်းပြည့် *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	ကိန်းပြည့် rc = -EINVAL;

	*ret = 0;

	အကယ်၍ (f && fd_is_procfs(fileno(f))) {
		ကိန်းပြည့် cap;

		/* we check if the cap after this one really isn't valid */
		အကယ်၍ (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	အကယ်၍ (f)
		fclose(f);
	ပြန်ပေး rc;
}

ကိန်းပြည့် cap_last_cap(void)
{
	static ကိန်းပြည့် cap = -1;

	အကယ်၍ (cap != -1)
		ပြန်ပေး cap;
	အကယ်၍ (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	ပြန်ပေး cap;
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

	အကယ်၍ (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	အကယ်၍ (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	အတွက် (cap = 0; cap < (parimaa_na(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		အကယ်၍ (cap > (uint64_t) cap_last_cap())
			continue;

		အကယ်၍ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	ကိန်းပြည့် rc = 0, cap;

	အကယ်၍ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		ပြန်ပေး EXIT_FAILURE;
	}

	အကယ်၍ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		အကယ်၍ (rc == 0)
			printf("last cap: %d\n", cap);

	} မဟုတ်ပါက အကယ်၍ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		အကယ်၍ (rc == 0)
			printf("last cap: %d\n", cap);

	} မဟုတ်ပါက အကယ်၍ (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	ပြန်ပေး rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
