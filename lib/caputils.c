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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/prctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "caputils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pathnames.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "procfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"

ਸਥਿਰ ਪੂਰਨ test_cap(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	ਵਾਪਸ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ਸਥਿਰ ਪੂਰਨ cap_ਆਖਰੀ_by_bsearch(ਪੂਰਨ *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	ਪੂਰਨ cap = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cap0 = 0, cap1 = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;

	ਜਦੋਂ ((ਪੂਰਨ)cap0 < cap) {
		ਜੇ (test_cap(cap))
			cap0 = cap;
		ਨਹੀਂਤਾਂ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ cap_ਆਖਰੀ_by_procfs(ਪੂਰਨ *ret)
{
	FILE *f = fopen(ਰਸਤਾ_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	ਪੂਰਨ rc = -EINVAL;

	*ret = 0;

	ਜੇ (f && fd_is_procfs(fileno(f))) {
		ਪੂਰਨ cap;

		/* we check if the cap after this one really isn't valid */
		ਜੇ (fscanf(f, "%d", &cap) == 1 &&
		    cap < ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	ਜੇ (f)
		fclose(f);
	ਵਾਪਸ rc;
}

ਪੂਰਨ cap_ਆਖਰੀ_cap(ਖਾਲੀ)
{
	ਸਥਿਰ ਪੂਰਨ cap = -1;

	ਜੇ (cap != -1)
		ਵਾਪਸ cap;
	ਜੇ (cap_ਆਖਰੀ_by_procfs(&cap) < 0)
		cap_ਆਖਰੀ_by_bsearch(&cap);

	ਵਾਪਸ cap;
}

ਖਾਲੀ cap_permitted_ਮੰਜ਼ਿਲ_ambient(ਖਾਲੀ)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ਬਣਤਰ __user_cap_header_struct ਸਿਰਲੇਖ = {
		.ਸੰਸਕਰਣ = LINUX_CAPABILITY_ਸੰਸਕਰਣ_3,
		.pid = 0,
	};
	ਬਣਤਰ __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	ਜੇ (capget(&ਸਿਰਲੇਖ, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	ਜੇ (capset(&ਸਿਰਲੇਖ, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	ਲਈ (cap = 0; cap < (ਆਕਾਰ(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		ਜੇ (cap > (uint64_t) cap_ਆਖਰੀ_cap())
			ਜਾਰੀ;

		ਜੇ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_CAPUTILS
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ rc = 0, cap;

	ਜੇ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਜੇ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_ਆਖਰੀ_by_procfs(&cap);
		ਜੇ (rc == 0)
			ਲਿਖੋ("last cap: %d\n", cap);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_ਆਖਰੀ_by_bsearch(&cap);
		ਜੇ (rc == 0)
			ਲਿਖੋ("last cap: %d\n", cap);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[1], "--last") == 0)
		ਲਿਖੋ("last cap: %d\n", cap_ਆਖਰੀ_cap());

	ਵਾਪਸ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ
