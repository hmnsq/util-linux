/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#شامل <string.h>

#شامل "match.h"
#شامل "cctype.h"

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
صحیح match_fstype(ثابت نویسه *type, ثابت نویسه *pattern)
{
	صحیح no = 0;		/* negated types list */
	صحیح len;
	ثابت نویسه *p;

	اگر (!pattern && !type)
		بازگشت 1;
	اگر (!pattern)
		بازگشت 0;

	اگر (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	وقتی(1) {
		اگر (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			بازگشت 0;
		اگر (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			بازگشت !no;
		p = strchr(p,',');
		اگر (!p)
			بشکن;
		p++;
	}
	بازگشت no;
}
