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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ test_cap(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ಹಿಂತಿರುಗಿ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cap_ಕೊನೆಯ_by_bsearch(ಪೂರ್ಣಾಂಕ *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	ಪೂರ್ಣಾಂಕ cap = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cap0 = 0, cap1 = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;

	ಆಗ ((ಪೂರ್ಣಾಂಕ)cap0 < cap) {
		ಆದರೆ (test_cap(cap))
			cap0 = cap;
		ಇಲ್ಲದಿದ್ದರೆ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cap_ಕೊನೆಯ_by_procfs(ಪೂರ್ಣಾಂಕ *ret)
{
	FILE *f = fopen(ಮಾರ್ಗ_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	ಪೂರ್ಣಾಂಕ rc = -EINVAL;

	*ret = 0;

	ಆದರೆ (f && fd_is_procfs(fileno(f))) {
		ಪೂರ್ಣಾಂಕ cap;

		/* we check if the cap after this one really isn't valid */
		ಆದರೆ (fscanf(f, "%d", &cap) == 1 &&
		    cap < ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ಆದರೆ (f)
		fclose(f);
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ cap_ಕೊನೆಯ_cap(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cap = -1;

	ಆದರೆ (cap != -1)
		ಹಿಂತಿರುಗಿ cap;
	ಆದರೆ (cap_ಕೊನೆಯ_by_procfs(&cap) < 0)
		cap_ಕೊನೆಯ_by_bsearch(&cap);

	ಹಿಂತಿರುಗಿ cap;
}

ಶೂನ್ಯ cap_permitted_ಗಮ್ಯ_ambient(ಶೂನ್ಯ)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ರಚನೆ __user_cap_header_struct ಶೀರ್ಷಿಕೆ = {
		.ಆವೃತ್ತಿ = LINUX_CAPABILITY_ಆವೃತ್ತಿ_3,
		.pid = 0,
	};
	ರಚನೆ __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ಆದರೆ (capget(&ಶೀರ್ಷಿಕೆ, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ಆದರೆ (capset(&ಶೀರ್ಷಿಕೆ, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	ಗಾಗಿ (cap = 0; cap < (ಗಾತ್ರ(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ಆದರೆ (cap > (uint64_t) cap_ಕೊನೆಯ_cap())
			ಮುಂದುವರಿ;

		ಆದರೆ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_CAPUTILS
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ rc = 0, cap;

	ಆದರೆ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಆದರೆ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_ಕೊನೆಯ_by_procfs(&cap);
		ಆದರೆ (rc == 0)
			ಬರೆ("last cap: %d\n", cap);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_ಕೊನೆಯ_by_bsearch(&cap);
		ಆದರೆ (rc == 0)
			ಬರೆ("last cap: %d\n", cap);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[1], "--last") == 0)
		ಬರೆ("last cap: %d\n", cap_ಕೊನೆಯ_cap());

	ಹಿಂತಿರುಗಿ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ
