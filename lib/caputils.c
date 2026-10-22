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

#અન્તર્ભાવ <sys/prctl.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "caputils.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "procfs.h"
#અન્તર્ભાવ "nls.h"

સ્થિર પૂર્ણાંક test_cap(unsigned પૂર્ણાંક cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	પાછા prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

સ્થિર પૂર્ણાંક cap_last_by_bsearch(પૂર્ણાંક *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	પૂર્ણાંક cap = INT_MAX;
	unsigned પૂર્ણાંક cap0 = 0, cap1 = INT_MAX;

	જ્યારે ((પૂર્ણાંક)cap0 < cap) {
		જો (test_cap(cap))
			cap0 = cap;
		નહીંતર
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	પાછા 0;
}

સ્થિર પૂર્ણાંક cap_last_by_procfs(પૂર્ણાંક *ret)
{
	FILE *f = fopen(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	પૂર્ણાંક rc = -EINVAL;

	*ret = 0;

	જો (f && fd_is_procfs(fileno(f))) {
		પૂર્ણાંક cap;

		/* we check if the cap after this one really isn't valid */
		જો (fscanf(f, "%d", &cap) == 1 &&
		    cap < INT_MAX && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	જો (f)
		fclose(f);
	પાછા rc;
}

પૂર્ણાંક cap_last_cap(શૂન્ય)
{
	સ્થિર પૂર્ણાંક cap = -1;

	જો (cap != -1)
		પાછા cap;
	જો (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	પાછા cap;
}

શૂન્ય cap_permitted_to_ambient(શૂન્ય)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	સંરચના __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	સંરચના __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	જો (capget(&header, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	જો (capset(&header, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	માટે (cap = 0; cap < (કદ(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		જો (cap > (uint64_t) cap_last_cap())
			ચાલુ;

		જો ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	પૂર્ણાંક rc = 0, cap;

	જો (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		પાછા EXIT_FAILURE;
	}

	જો (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		જો (rc == 0)
			printf("last cap: %d\n", cap);

	} નહીંતર જો (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		જો (rc == 0)
			printf("last cap: %d\n", cap);

	} નહીંતર જો (strcmp(argv[1], "--last") == 0)
		printf("last cap: %d\n", cap_last_cap());

	પાછા rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif
