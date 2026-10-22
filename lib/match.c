/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "match.h"
#અન્તર્ભાવ "cctype.h"

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
પૂર્ણાંક match_fstype(અચલ અક્ષર *type, અચલ અક્ષર *pattern)
{
	પૂર્ણાંક no = 0;		/* negated types list */
	પૂર્ણાંક len;
	અચલ અક્ષર *p;

	જો (!pattern && !type)
		પાછા 1;
	જો (!pattern)
		પાછા 0;

	જો (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	જ્યારે(1) {
		જો (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			પાછા 0;
		જો (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			પાછા !no;
		p = strchr(p,',');
		જો (!p)
			તોડો;
		p++;
	}
	પાછા no;
}
