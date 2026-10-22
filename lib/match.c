/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#समावेश <string.h>

#समावेश "match.h"
#समावेश "cctype.h"

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
पूर्णांक match_fstype(स्थिर अक्षर *type, स्थिर अक्षर *pattern)
{
	पूर्णांक no = 0;		/* negated types list */
	पूर्णांक len;
	स्थिर अक्षर *p;

	अगर (!pattern && !type)
		वापस 1;
	अगर (!pattern)
		वापस 0;

	अगर (!म_भेदन(pattern, "no", 2)) {
		no = 1;
		pattern += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = म_माप(type);
	p = pattern;
	जबतक(1) {
		अगर (!म_भेदन(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			वापस 0;
		अगर (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			वापस !no;
		p = म_अक्षर(p,',');
		अगर (!p)
			अवरोध;
		p++;
	}
	वापस no;
}
