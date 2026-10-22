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

#ಅನ್ತರ್ಭಾವ <sys/prctl.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "caputils.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "procfs.h"
#ಅನ್ತರ್ಭಾವ "nls.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_cap(unsigned ಪೂರ್ಣಾಙ್ಕ cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ಪ್ರತ್ಯಾಗಮ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cap_last_by_bsearch(ಪೂರ್ಣಾಙ್ಕ *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	ಪೂರ್ಣಾಙ್ಕ cap = INT_MAX;
	unsigned ಪೂರ್ಣಾಙ್ಕ cap0 = 0, cap1 = INT_MAX;

	ಯಾವತ್ ((ಪೂರ್ಣಾಙ್ಕ)cap0 < cap) {
		ಯದಿ (test_cap(cap))
			cap0 = cap;
		ಅನ್ಯಥಾ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cap_last_by_procfs(ಪೂರ್ಣಾಙ್ಕ *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	ಪೂರ್ಣಾಙ್ಕ rc = -EINVAL;

	*ret = 0;

	ಯದಿ (f && fd_is_procfs(fileno(f))) {
		ಪೂರ್ಣಾಙ್ಕ cap;

		/* we check if the cap after this one really isn't valid */
		ಯದಿ (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ಯದಿ (f)
		fclose(f);
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ cap_last_cap(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cap = -1;

	ಯದಿ (cap != -1)
		ಪ್ರತ್ಯಾಗಮ cap;
	ಯದಿ (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	ಪ್ರತ್ಯಾಗಮ cap;
}

ಶೂನ್ಯ cap_permitted_to_ambient(ಶೂನ್ಯ)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ಸಂರಚನಾ __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	ಸಂರಚನಾ __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ಯದಿ (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ಯದಿ (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	ಪ್ರತಿ (cap = 0; cap < (ಪರಿಮಾಣ(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ಯದಿ (cap > (uint64_t) cap_last_cap())
			ಅನುವರ್ತ;

		ಯದಿ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0, cap;

	ಯದಿ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	ಯದಿ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		ಯದಿ (rc == 0)
			printf("last cap: %d\n", cap);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		ಯದಿ (rc == 0)
			printf("last cap: %d\n", cap);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	ಪ್ರತ್ಯಾಗಮ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
