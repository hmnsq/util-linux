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
整数 match_fstype(const char *type, const char *pattern)
{
	整数 no = 0;		/* negated types list */
	整数 len;
	const char *p;

	如果 (!pattern && !type)
		返回 1;
	如果 (!pattern)
		返回 0;

	如果 (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	当(1) {
		如果 (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			返回 0;
		如果 (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			返回 !no;
		p = strchr(p,',');
		如果 (!p)
			break;
		p++;
	}
	返回 no;
}
