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
puur_naa_nka match_fstype(const var_na *type, const var_na *pattern)
{
	puur_naa_nka no = 0;		/* negated types list */
	puur_naa_nka len;
	const var_na *p;

	ᰀᰧ (!pattern && !type)
		ᰀᰧᰌ 1;
	ᰀᰧ (!pattern)
		ᰀᰧᰌ 0;

	ᰀᰧ (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	ᰀᰬ(1) {
		ᰀᰧ (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ᰀᰧᰌ 0;
		ᰀᰧ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ᰀᰧᰌ !no;
		p = strchr(p,',');
		ᰀᰧ (!p)
			bha_nga;
		p++;
	}
	ᰀᰧᰌ no;
}
