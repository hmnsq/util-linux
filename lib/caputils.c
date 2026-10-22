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

#अन्तर्भाव <sys/prctl.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "caputils.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "procfs.h"
#अन्तर्भाव "nls.h"

स्थिर पूर्णाङ्क test_cap(अचिन्हित पूर्णाङ्क cap)
{
	/* prctl returns 0 or 1 for valid caps, -1 otherwise */
	प्रत्यागम prctl(PR_CAPBSET_READ, cap, 0, 0, 0) >= 0;
}

स्थिर पूर्णाङ्क cap_last_by_bsearch(पूर्णाङ्क *ret)
{
	/* starting with cap=INT_MAX means we always know
	 * that cap1 is invalid after the first iteration */
	पूर्णाङ्क cap = पूर्णांक_उच्च;
	अचिन्हित पूर्णाङ्क cap0 = 0, cap1 = पूर्णांक_उच्च;

	यावत् ((पूर्णाङ्क)cap0 < cap) {
		यदि (test_cap(cap))
			cap0 = cap;
		अन्यथा
			cap1 = cap;

		cap = (cap0 + cap1) / 2U;
	}

	*ret = cap;
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क cap_last_by_procfs(पूर्णाङ्क *ret)
{
	खाता *f = ख_खोलो(_PATH_PROC_CAPLASTCAP, "r" UL_CLOEXECSTR);
	पूर्णाङ्क rc = -EINVAL;

	*ret = 0;

	यदि (f && fd_is_procfs(fileno(f))) {
		पूर्णाङ्क cap;

		/* we check if the cap after this one really isn't valid */
		यदि (ख_पूछो(f, "%d", &cap) == 1 &&
		    cap < पूर्णांक_उच्च && !test_cap(cap + 1)) {

			*ret = cap;
			rc = 0;
		}
	}

	यदि (f)
		ख_बंद(f);
	प्रत्यागम rc;
}

पूर्णाङ्क cap_last_cap(शून्य)
{
	स्थिर पूर्णाङ्क cap = -1;

	यदि (cap != -1)
		प्रत्यागम cap;
	यदि (cap_last_by_procfs(&cap) < 0)
		cap_last_by_bsearch(&cap);

	प्रत्यागम cap;
}

शून्य cap_permitted_to_ambient(शून्य)
{
	/* We use capabilities system calls to propagate the permitted
	 * capabilities into the ambient set because we may have
	 * already forked so be in async-signal-safe context. */
	संरचना __user_cap_header_struct header = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0,
	};
	संरचना __user_cap_data_struct payload[_LINUX_CAPABILITY_U32S_3] = {{ 0 }};
	uint64_t effective, cap;

	यदि (capget(&header, payload) < 0)
		err(निकास_त्रुटि, _("capget failed"));

	/* In order the make capabilities ambient, we first need to ensure
	 * that they are all inheritable. */
	payload[0].inheritable = payload[0].permitted;
	payload[1].inheritable = payload[1].permitted;

	यदि (capset(&header, payload) < 0)
		err(निकास_त्रुटि, _("capset failed"));

	effective = ((uint64_t)payload[1].effective << 32) |  (uint64_t)payload[0].effective;

	प्रति (cap = 0; cap < (परिमाण(effective) * 8); cap++) {
		/* This is the same check as cap_valid(), but using
		 * the runtime value for the last valid cap. */
		यदि (cap > (uint64_t) cap_last_cap())
			अनुवर्त;

		यदि ((effective & (1ULL << cap))
		    && prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_RAISE, cap, 0, 0) < 0)
			err(निकास_त्रुटि, _("prctl(PR_CAP_AMBIENT) failed"));
	}
}

#ifdef TEST_PROGRAM_CAPUTILS
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क rc = 0, cap;

	यदि (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s --last-by-procfs\n"
				"       %1$s --last-by-bsearch\n"
				"       %1$s --last\n",
				program_invocation_short_name);
		प्रत्यागम निकास_त्रुटि;
	}

	यदि (म_भेद(argv[1], "--last-by-procfs") == 0) {
		rc = cap_last_by_procfs(&cap);
		यदि (rc == 0)
			म_लिखो("last cap: %d\n", cap);

	} अन्यथा यदि (म_भेद(argv[1], "--last-by-bsearch") == 0) {
		rc = cap_last_by_bsearch(&cap);
		यदि (rc == 0)
			म_लिखो("last cap: %d\n", cap);

	} अन्यथा यदि (म_भेद(argv[1], "--last") == 0)
		म_लिखो("last cap: %d\n", cap_last_cap());

	प्रत्यागम rc == 0 ? निकास_सफल : निकास_त्रुटि;
}
#endif
