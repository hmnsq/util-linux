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

#ଅନ୍ତର୍ଭାଵ <sys/prctl.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "caputils.h"
#ଅନ୍ତର୍ଭାଵ "pathnames.h"
#ଅନ୍ତର୍ଭାଵ "procfs.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା test_cap(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ଫେରନ୍ତୁ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap_last_by_bsearch(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap = INT_MAX;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap0 = 0, cap1 = INT_MAX;

	ଯେତେବେଳେ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cap0 < cap) {
		ଯଦି (test_cap(cap))
			cap0 = cap;
		ଅନ୍ୟଥା
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap_last_by_procfs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = -EINVAL;

	*ret = 0;

	ଯଦି (f && fd_is_procfs(fileno(f))) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap;

		/* we check if the cap after this one really isn't valid */
		ଯଦି (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ଯଦି (f)
		fclose(f);
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap_last_cap(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cap = -1;

	ଯଦି (cap != -1)
		ଫେରନ୍ତୁ cap;
	ଯଦି (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	ଫେରନ୍ତୁ cap;
}

ଶୂନ୍ୟ cap_permitted_to_ambient(ଶୂନ୍ୟ)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ସଂରଚନା __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	ସଂରଚନା __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ଯଦି (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ଯଦି (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	ପାଇଁ (cap = 0; cap < (ଆକାର(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ଯଦି (cap > (uint64_t) cap_last_cap())
			ଜାରି;

		ଯଦି ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0, cap;

	ଯଦି (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଯଦି (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		ଯଦି (rc == 0)
			printf("last cap: %d\n", cap);

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		ଯଦି (rc == 0)
			printf("last cap: %d\n", cap);

	} ଅନ୍ୟଥା ଯଦି (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	ଫେରନ୍ତୁ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
