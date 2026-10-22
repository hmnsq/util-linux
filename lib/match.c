/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "match.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"

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
ಪೂರ್ಣಾಙ್ಕ match_fstype(ಧ್ರುವ ವರ್ಣ *type, ಧ್ರುವ ವರ್ಣ *pattern)
{
	ಪೂರ್ಣಾಙ್ಕ no = 0;		/* negated types list */
	ಪೂರ್ಣಾಙ್ಕ len;
	ಧ್ರುವ ವರ್ಣ *p;

	ಯದಿ (!pattern && !type)
		ಪ್ರತ್ಯಾಗಮ 1;
	ಯದಿ (!pattern)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	ಯಾವತ್(1) {
		ಯದಿ (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ಪ್ರತ್ಯಾಗಮ 0;
		ಯದಿ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ಪ್ರತ್ಯಾಗಮ !no;
		p = strchr(p,',');
		ಯದಿ (!p)
			ಭಙ್ಗ;
		p++;
	}
	ಪ್ರತ್ಯಾಗಮ no;
}
