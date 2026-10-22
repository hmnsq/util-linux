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
ຈຳນວນເຕັມ match_fstype(const var_na *type, const var_na *pattern)
{
	ຈຳນວນເຕັມ no = 0;		/* negated types list */
	ຈຳນວນເຕັມ len;
	const var_na *p;

	ຖ້າ (!pattern && !type)
		ສົ່ງຄືນ 1;
	ຖ້າ (!pattern)
		ສົ່ງຄືນ 0;

	ຖ້າ (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	ໃນຂະນະທີ່(1) {
		ຖ້າ (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ສົ່ງຄືນ 0;
		ຖ້າ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ສົ່ງຄືນ !no;
		p = strchr(p,',');
		ຖ້າ (!p)
			bha_nga;
		p++;
	}
	ສົ່ງຄືນ no;
}
