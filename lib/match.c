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
صحيح سمون_fstype(ثابت حرف *type, ثابت حرف *نمونه)
{
	صحيح no = 0;		/* negated types list */
	صحيح len;
	ثابت حرف *p;

	إذا (!نمونه && !type)
		أرجع 1;
	إذا (!نمونه)
		أرجع 0;

	إذا (!مزي_پرتله(نمونه, "no", 2)) {
		no = 1;
		نمونه += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = مزي_اوږدوالی(type);
	p = نمونه;
	طالما(1) {
		إذا (!مزي_پرتله(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			أرجع 0;
		إذا (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			أرجع !no;
		p = strchr(p,',');
		إذا (!p)
			اكسر;
		p++;
	}
	أرجع no;
}
