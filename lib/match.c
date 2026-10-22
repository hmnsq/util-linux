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
صحیح مطابقت_fstype(ثابت حرف *type, ثابت حرف *نمونہ)
{
	صحیح no = 0;		/* negated types list */
	صحیح len;
	ثابت حرف *p;

	اگر (!نمونہ && !type)
		واپس 1;
	اگر (!نمونہ)
		واپس 0;

	اگر (!لڑی_موازنہ(نمونہ, "no", 2)) {
		no = 1;
		نمونہ += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = لڑی_لمبائی(type);
	p = نمونہ;
	جب(1) {
		اگر (!لڑی_موازنہ(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			واپس 0;
		اگر (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			واپس !no;
		p = strchr(p,',');
		اگر (!p)
			توڑو;
		p++;
	}
	واپس no;
}
