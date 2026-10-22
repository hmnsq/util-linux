/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#অন্তর্ভাব <string.h>

#অন্তর্ভাব "match.h"
#অন্তর্ভাব "cctype.h"

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
পূর্ণাঙ্ক match_fstype(ধ্রুব বর্ণ *type, ধ্রুব বর্ণ *pattern)
{
	পূর্ণাঙ্ক no = 0;		/* negated types list */
	পূর্ণাঙ্ক len;
	ধ্রুব বর্ণ *p;

	যদি (!pattern && !type)
		প্রত্যাগম 1;
	যদি (!pattern)
		প্রত্যাগম 0;

	যদি (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	যাবত্(1) {
		যদি (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			প্রত্যাগম 0;
		যদি (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			প্রত্যাগম !no;
		p = strchr(p,',');
		যদি (!p)
			ভঙ্গ;
		p++;
	}
	প্রত্যাগম no;
}
