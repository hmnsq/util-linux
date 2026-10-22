/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#ضمن <string.h>

#ضمن "match.h"
#ضمن "cctype.h"

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
صحيح match_fstype(ثابت حرف *type, ثابت حرف *pattern)
{
	صحيح no = 0;		/* negated types list */
	صحيح len;
	ثابت حرف *p;

	אם (!pattern && !type)
		החזר 1;
	אם (!pattern)
		החזר 0;

	אם (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	כלעוד(1) {
		אם (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			החזר 0;
		אם (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			החזר !no;
		p = strchr(p,',');
		אם (!p)
			اكسر;
		p++;
	}
	החזר no;
}
