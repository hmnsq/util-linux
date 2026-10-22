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

#অন্তর্ভাব <sys/prctl.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "caputils.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "procfs.h"
#অন্তর্ভাব "nls.h"

স্থির পূর্ণসংখ্যা test_cap(unsigned পূর্ণসংখ্যা cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ফেরত prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

স্থির পূর্ণসংখ্যা cap_last_by_bsearch(পূর্ণসংখ্যা *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	পূর্ণসংখ্যা cap = INT_MAX;
	unsigned পূর্ণসংখ্যা cap0 = 0, cap1 = INT_MAX;

	যখন ((পূর্ণসংখ্যা)cap0 < cap) {
		যদি (test_cap(cap))
			cap0 = cap;
		অন্যথা
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা cap_last_by_procfs(পূর্ণসংখ্যা *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	পূর্ণসংখ্যা rc = -EINVAL;

	*ret = 0;

	যদি (f && fd_is_procfs(fileno(f))) {
		পূর্ণসংখ্যা cap;

		/* we check if the cap after this one really isn't valid */
		যদি (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	যদি (f)
		fclose(f);
	ফেরত rc;
}

পূর্ণসংখ্যা cap_last_cap(শূন্য)
{
	স্থির পূর্ণসংখ্যা cap = -1;

	যদি (cap != -1)
		ফেরত cap;
	যদি (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	ফেরত cap;
}

শূন্য cap_permitted_to_ambient(শূন্য)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	কাঠামো __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	কাঠামো __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	যদি (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	যদি (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	জন্য (cap = 0; cap < (আকার(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		যদি (cap > (uint64_t) cap_last_cap())
			চালু;

		যদি ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	পূর্ণসংখ্যা rc = 0, cap;

	যদি (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		ফেরত EXIT_FAILURE;
	}

	যদি (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		যদি (rc == 0)
			printf("last cap: %d\n", cap);

	} অন্যথা যদি (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		যদি (rc == 0)
			printf("last cap: %d\n", cap);

	} অন্যথা যদি (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	ফেরত rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
