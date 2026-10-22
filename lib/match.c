/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#include <string.h>

#include "match.h"
#include "cctype.h"

/*
 * match_fstype:
 * @type: filesystem type
 * @pattern: filesystem name or comma delimited list of names
 *
 * The @pattern list of filesystem can be prefixed with a global
 * "no" prefix to invert matching of the whole list. The "no" could
 * also be used for individual items in the @pattern list. So,
 * "nofoo,bar" has the same meaning as "nofoo,nobar".
 */
ကိန်းပြည့် match_fstype(const var_na *type, const var_na *pattern)
{
	ကိန်းပြည့် no = 0;		/* negated types list */
	ကိန်းပြည့် len;
	const var_na *p;

	အကယ်၍ (!pattern && !type)
		ပြန်ပေး 1;
	အကယ်၍ (!pattern)
		ပြန်ပေး 0;

	အကယ်၍ (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	နေစဉ်(1) {
		အကယ်၍ (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ပြန်ပေး 0;
		အကယ်၍ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ပြန်ပေး !no;
		p = strchr(p,',');
		အကယ်၍ (!p)
			bha_nga;
		p++;
	}
	ပြန်ပေး no;
}
