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

#ضمن <sys/prctl.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <limits.h>
#ضمن <errno.h>

#ضمن "c.h"
#ضمن "caputils.h"
#ضمن "pathnames.h"
#ضمن "procfs.h"
#ضمن "nls.h"

ساكن صحيح test_cap(بیعلامت صحيح cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	أرجع prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ساكن صحيح cap_آخرین_by_bsearch(صحيح *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	صحيح cap = بیشترین_صحیح;
	بیعلامت صحيح cap0 = 0, cap1 = بیشترین_صحیح;

	طالما ((صحيح)cap0 < cap) {
		إذا (test_cap(cap))
			cap0 = cap;
		وإلا
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	أرجع 0;
}

ساكن صحيح cap_آخرین_by_procfs(صحيح *ret)
{
	FILE *f = fopen(مسیر_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	صحيح rc = -EINVAL;

	*ret = 0;

	إذا (f && fd_is_procfs(fileno(f))) {
		صحيح cap;

		/* we check if the cap after this one really isn't valid */
		إذا (fscanf(f, "%d", &cap) == 1 &&
		    cap < بیشترین_صحیح && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	إذا (f)
		fclose(f);
	أرجع rc;
}

صحيح cap_آخرین_cap(فراغ)
{
	ساكن صحيح cap = -1;

	إذا (cap != -1)
		أرجع cap;
	إذا (cap_آخرین_by_procfs(&cap) < 0)
		cap_آخرین_by_bsearch(&cap);

	أرجع cap;
}

فراغ cap_permitted_مقصد_ambient(فراغ)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	بنية __user_cap_header_struct سرآیند = {
		.نسخه = LINUX_CAPABILITY_نسخه_3,
		.pid = 0,
	};
	بنية __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	إذا (capget(&سرآیند, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	إذا (capset(&سرآیند, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	لكل (cap = 0; cap < (حجم(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		إذا (cap > (uint64_t) cap_آخرین_cap())
			استمر;

		إذا ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#اگر_تعریف_شده TEST_برنامه_CAPUTILS
صحيح اصلی(صحيح argc, حرف *argv[])
{
	صحيح rc = 0, cap;

	إذا (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				برنامه_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	إذا (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_آخرین_by_procfs(&cap);
		إذا (rc == 0)
			چاپ_کن("last cap: %d\n", cap);

	} وإلا إذا (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_آخرین_by_bsearch(&cap);
		إذا (rc == 0)
			چاپ_کن("last cap: %d\n", cap);

	} وإلا إذا (strcmp(argv[1], "--last") == 0)
		چاپ_کن("last cap: %d\n", cap_آخرین_cap());

	أرجع rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#پایان_اگر
