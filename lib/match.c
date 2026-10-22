/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#అన్తర్భావ <string.h>

#అన్తర్భావ "match.h"
#అన్తర్భావ "cctype.h"

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
పూర్ణాంకం సరిపోలిక_fstype(స్థిరరాశి అక్షరం *type, స్థిరరాశి అక్షరం *నమూనా)
{
	పూర్ణాంకం no = 0;		/* negated types list */
	పూర్ణాంకం len;
	స్థిరరాశి అక్షరం *p;

	ఒకవేళ (!నమూనా && !type)
		తిరిగి 1;
	ఒకవేళ (!నమూనా)
		తిరిగి 0;

	ఒకవేళ (!అక్షరశ్రేణి_పోలిక(నమూనా, "no", 2)) {
		no = 1;
		నమూనా += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = అక్షరశ్రేణి_పొడవు(type);
	p = నమూనా;
	అయితే(1) {
		ఒకవేళ (!అక్షరశ్రేణి_పోలిక(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			తిరిగి 0;
		ఒకవేళ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			తిరిగి !no;
		p = strchr(p,',');
		ఒకవేళ (!p)
			ఆపు;
		p++;
	}
	తిరిగి no;
}
