/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "match.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"

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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା match_fstype(ଅଚଳ ଅକ୍ଷର *type, ଅଚଳ ଅକ୍ଷର *pattern)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା no = 0;		/* negated types list */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ଅଚଳ ଅକ୍ଷର *p;

	ଯଦି (!pattern && !type)
		ଫେରନ୍ତୁ 1;
	ଯଦି (!pattern)
		ଫେରନ୍ତୁ 0;

	ଯଦି (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	ଯେତେବେଳେ(1) {
		ଯଦି (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ଫେରନ୍ତୁ 0;
		ଯଦି (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ଫେରନ୍ତୁ !no;
		p = strchr(p,',');
		ଯଦି (!p)
			ଭାଙ୍ଗ;
		p++;
	}
	ଫେରନ୍ତୁ no;
}
