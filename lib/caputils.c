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

#అన్తర్భావ <sys/prctl.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "caputils.h"
#అన్తర్భావ "pathnames.h"
#అన్తర్భావ "procfs.h"
#అన్తర్భావ "nls.h"

స్థిర పూర్ణాంకం test_cap(గుర్తులేని పూర్ణాంకం cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	తిరిగి prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

స్థిర పూర్ణాంకం cap_చివరి_by_bsearch(పూర్ణాంకం *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	పూర్ణాంకం cap = గరిష్ఠ_పూర్ణాంకం;
	గుర్తులేని పూర్ణాంకం cap0 = 0, cap1 = గరిష్ఠ_పూర్ణాంకం;

	అయితే ((పూర్ణాంకం)cap0 < cap) {
		ఒకవేళ (test_cap(cap))
			cap0 = cap;
		లేకపోతే
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	తిరిగి 0;
}

స్థిర పూర్ణాంకం cap_చివరి_by_procfs(పూర్ణాంకం *ret)
{
	FILE *f = fopen(మార్గం_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	పూర్ణాంకం rc = -EINVAL;

	*ret = 0;

	ఒకవేళ (f && fd_is_procfs(fileno(f))) {
		పూర్ణాంకం cap;

		/* we check if the cap after this one really isn't valid */
		ఒకవేళ (fscanf(f, "%d", &cap) == 1 &&
		    cap < గరిష్ఠ_పూర్ణాంకం && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ఒకవేళ (f)
		fclose(f);
	తిరిగి rc;
}

పూర్ణాంకం cap_చివరి_cap(శూన్య)
{
	స్థిర పూర్ణాంకం cap = -1;

	ఒకవేళ (cap != -1)
		తిరిగి cap;
	ఒకవేళ (cap_చివరి_by_procfs(&cap) < 0)
		cap_చివరి_by_bsearch(&cap);

	తిరిగి cap;
}

శూన్య cap_permitted_గమ్యం_ambient(శూన్య)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	నిర్మాణం __user_cap_header_struct శీర్షిక = {
		.సంస్కరణ = LINUX_CAPABILITY_సంస్కరణ_3,
		.pid = 0,
	};
	నిర్మాణం __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ఒకవేళ (capget(&శీర్షిక, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ఒకవేళ (capset(&శీర్షిక, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	కొరకు (cap = 0; cap < (పరిమాణం(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ఒకవేళ (cap > (uint64_t) cap_చివరి_cap())
			కొనసాగు;

		ఒకవేళ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#నిర్వచితమైతే TEST_కార్యక్రమం_CAPUTILS
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	పూర్ణాంకం rc = 0, cap;

	ఒకవేళ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				కార్యక్రమం_invocation_short_name);
		తిరిగి EXIT_FAILURE;
	}

	ఒకవేళ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_చివరి_by_procfs(&cap);
		ఒకవేళ (rc == 0)
			రాయి("last cap: %d\n", cap);

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_చివరి_by_bsearch(&cap);
		ఒకవేళ (rc == 0)
			రాయి("last cap: %d\n", cap);

	} లేకపోతే ఒకవేళ (strcmp(argv[1], "--last") == 0)
		రాయి("last cap: %d\n", cap_చివరి_cap());

	తిరిగి rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#అయితే_ముగింపు
