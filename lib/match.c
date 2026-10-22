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
পূর্ণসংখ্যা match_fstype(ধ্রুবক অক্ষর *type, ধ্রুবক অক্ষর *pattern)
{
	পূর্ণসংখ্যা no = 0;		/* negated types list */
	পূর্ণসংখ্যা len;
	ধ্রুবক অক্ষর *p;

	যদি (!pattern && !type)
		ফেরত 1;
	যদি (!pattern)
		ফেরত 0;

	যদি (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	যখন(1) {
		যদি (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ফেরত 0;
		যদি (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ফেরত !no;
		p = strchr(p,',');
		যদি (!p)
			বিরতি;
		p++;
	}
	ফেরত no;
}
