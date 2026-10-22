/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "match.h"
#അന്തര്ഭാവ "cctype.h"

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
പൂർണ്ണസംഖ്യ match_fstype(സ്ഥിരം അക്ഷരം *type, സ്ഥിരം അക്ഷരം *pattern)
{
	പൂർണ്ണസംഖ്യ no = 0;		/* negated types list */
	പൂർണ്ണസംഖ്യ len;
	സ്ഥിരം അക്ഷരം *p;

	എങ്കിൽ (!pattern && !type)
		തിരികെ 1;
	എങ്കിൽ (!pattern)
		തിരികെ 0;

	എങ്കിൽ (!strncmp(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = strlen(type);
	p = pattern;
	ആകുമ്പോൾ(1) {
		എങ്കിൽ (!strncmp(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			തിരികെ 0;
		എങ്കിൽ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			തിരികെ !no;
		p = strchr(p,',');
		എങ്കിൽ (!p)
			നിർത്തുക;
		p++;
	}
	തിരികെ no;
}
