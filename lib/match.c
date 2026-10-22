/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#அந்தர்பாவ <string.h>

#அந்தர்பாவ "match.h"
#அந்தர்பாவ "cctype.h"

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
முழு match_fstype(மாறிலி எழுத்து *type, மாறிலி எழுத்து *pattern)
{
	முழு no = 0;		/* negated types list */
	முழு len;
	மாறிலி எழுத்து *p;

	எனில் (!pattern && !type)
		திரும்பு 1;
	எனில் (!pattern)
		திரும்பு 0;

	எனில் (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	போது(1) {
		எனில் (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			திரும்பு 0;
		எனில் (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			திரும்பு !no;
		p = strchr(p,',');
		எனில் (!p)
			நிறுத்து;
		p++;
	}
	திரும்பு no;
}
