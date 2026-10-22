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

#അന്തര്ഭാവ <sys/prctl.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "caputils.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "procfs.h"
#അന്തര്ഭാവ "nls.h"

സ്ഥിതം പൂർണ്ണസംഖ്യ test_cap(unsigned പൂർണ്ണസംഖ്യ cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	തിരികെ prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cap_last_by_bsearch(പൂർണ്ണസംഖ്യ *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	പൂർണ്ണസംഖ്യ cap = INT_MAX;
	unsigned പൂർണ്ണസംഖ്യ cap0 = 0, cap1 = INT_MAX;

	ആകുമ്പോൾ ((പൂർണ്ണസംഖ്യ)cap0 < cap) {
		എങ്കിൽ (test_cap(cap))
			cap0 = cap;
		അല്ലെങ്കിൽ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cap_last_by_procfs(പൂർണ്ണസംഖ്യ *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	പൂർണ്ണസംഖ്യ rc = -EINVAL;

	*ret = 0;

	എങ്കിൽ (f && fd_is_procfs(fileno(f))) {
		പൂർണ്ണസംഖ്യ cap;

		/* we check if the cap after this one really isn't valid */
		എങ്കിൽ (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	എങ്കിൽ (f)
		fclose(f);
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ cap_last_cap(ശൂന്യം)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ cap = -1;

	എങ്കിൽ (cap != -1)
		തിരികെ cap;
	എങ്കിൽ (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	തിരികെ cap;
}

ശൂന്യം cap_permitted_to_ambient(ശൂന്യം)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ഘടന __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	ഘടന __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	എങ്കിൽ (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	എങ്കിൽ (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	വേണ്ടി (cap = 0; cap < (വലുപ്പം(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		എങ്കിൽ (cap > (uint64_t) cap_last_cap())
			തുടരുക;

		എങ്കിൽ ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ rc = 0, cap;

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		തിരികെ EXIT_FAILURE;
	}

	എങ്കിൽ (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		എങ്കിൽ (rc == 0)
			printf("last cap: %d\n", cap);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		എങ്കിൽ (rc == 0)
			printf("last cap: %d\n", cap);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	തിരികെ rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
