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

#شامل <sys/prctl.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <limits.h>
#شامل <errno.h>

#شامل "c.h"
#شامل "caputils.h"
#شامل "pathnames.h"
#شامل "procfs.h"
#شامل "nls.h"

ساکن صحیح test_cap(غیرعلامتی صحیح cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	واپس prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

ساکن صحیح cap_آخری_by_bsearch(صحیح *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	صحیح cap = زیادہ_سے_زیادہ_عدد;
	غیرعلامتی صحیح cap0 = 0, cap1 = زیادہ_سے_زیادہ_عدد;

	جب ((صحیح)cap0 < cap) {
		اگر (test_cap(cap))
			cap0 = cap;
		ورنہ
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	واپس 0;
}

ساکن صحیح cap_آخری_by_procfs(صحیح *ret)
{
	FILE *f = fopen(راستہ_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	صحیح rc = -EINVAL;

	*ret = 0;

	اگر (f && fd_is_procfs(fileno(f))) {
		صحیح cap;

		/* we check if the cap after this one really isn't valid */
		اگر (fscanf(f, "%d", &cap) == 1 &&
		    cap < زیادہ_سے_زیادہ_عدد && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	اگر (f)
		fclose(f);
	واپس rc;
}

صحیح cap_آخری_cap(خالی)
{
	ساکن صحیح cap = -1;

	اگر (cap != -1)
		واپس cap;
	اگر (cap_آخری_by_procfs(&cap) < 0)
		cap_آخری_by_bsearch(&cap);

	واپس cap;
}

خالی cap_permitted_مقصد_ambient(خالی)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	ساخت __user_cap_header_struct سرخی = {
		.نسخہ = LINUX_CAPABILITY_نسخہ_3,
		.pid = 0,
	};
	ساخت __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	اگر (capget(&سرخی, payload) < 0)
		err(EXIT_FAILURE, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	اگر (capset(&سرخی, payload) < 0)
		err(EXIT_FAILURE, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	برائے (cap = 0; cap < (حجم(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		اگر (cap > (uint64_t) cap_آخری_cap())
			جاری;

		اگر ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(EXIT_FAILURE, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#اگر_تعریف_شدہ TEST_پروگرام_CAPUTILS
صحیح مین(صحیح argc, حرف *argv[])
{
	صحیح rc = 0, cap;

	اگر (argc < 2) {
		fprintf(stderr, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				پروگرام_invocation_short_name);
		واپس EXIT_FAILURE;
	}

	اگر (strcmp(argv[1], "--last-by-procfs") == 0) {
		rc = cap_آخری_by_procfs(&cap);
		اگر (rc == 0)
			لکھو("last cap: %d\n", cap);

	} ورنہ اگر (strcmp(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_آخری_by_bsearch(&cap);
		اگر (rc == 0)
			لکھو("last cap: %d\n", cap);

	} ورنہ اگر (strcmp(argv[1], "--last") == 0)
		لکھو("last cap: %d\n", cap_آخری_cap());

	واپس rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
#اگر_ختم
