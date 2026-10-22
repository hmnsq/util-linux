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
정수 match_fstype(const char *type, const char *pattern)
{
	정수 no = 0;		/* negated types list */
	정수 len;
	const char *p;

	만약 (!pattern && !type)
		반환 1;
	만약 (!pattern)
		반환 0;

	만약 (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	동안(1) {
		만약 (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			반환 0;
		만약 (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			반환 !no;
		p = strchr(p,',');
		만약 (!p)
			break;
		p++;
	}
	반환 no;
}
