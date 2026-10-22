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
ціле match_fstype(const char *type, const char *pattern)
{
	ціле no = 0;		/* negated types list */
	ціле len;
	const char *p;

	якщо (!pattern && !type)
		повернути 1;
	якщо (!pattern)
		повернути 0;

	якщо (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	поки(1) {
		якщо (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			повернути 0;
		якщо (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			повернути !no;
		p = strchr(p,',');
		якщо (!p)
			break;
		p++;
	}
	повернути no;
}
