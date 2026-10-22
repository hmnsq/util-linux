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
số_nguyên match_fstype(const char *type, const char *pattern)
{
	số_nguyên no = 0;		/* negated types list */
	số_nguyên len;
	const char *p;

	nếu (!pattern && !type)
		trả_về 1;
	nếu (!pattern)
		trả_về 0;

	nếu (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	trong_khi(1) {
		nếu (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			trả_về 0;
		nếu (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			trả_về !no;
		p = strchr(p,',');
		nếu (!p)
			break;
		p++;
	}
	trả_về no;
}
