/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#अन्तर्भाव <string.h>

#अन्तर्भाव "match.h"
#अन्तर्भाव "cctype.h"

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
पूर्णाङ्क मेलनम्_fstype(ध्रुव वर्ण *type, ध्रुव वर्ण *आदर्शः)
{
	पूर्णाङ्क no = 0;		/* negated types list */
	पूर्णाङ्क len;
	ध्रुव वर्ण *p;

	यदि (!आदर्शः && !type)
		प्रत्यागम 1;
	यदि (!आदर्शः)
		प्रत्यागम 0;

	यदि (!वाक्ये_तुलय(आदर्शः, "no", 2)) {
		no = 1;
		आदर्शः += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = वाक्यदैर्घ्यम्(type);
	p = आदर्शः;
	यावत्(1) {
		यदि (!वाक्ये_तुलय(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			प्रत्यागम 0;
		यदि (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			प्रत्यागम !no;
		p = strchr(p,',');
		यदि (!p)
			भङ्ग;
		p++;
	}
	प्रत्यागम no;
}
