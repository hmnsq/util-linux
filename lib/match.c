/*
 * Copyright (C) 2011 Karel Zak <kzak@redhat.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "match.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"

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
ਪੂਰਨ ਮੇਲ_fstype(ਅਚਲ ਅੱਖਰ *type, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ)
{
	ਪੂਰਨ no = 0;		/* negated types list */
	ਪੂਰਨ len;
	ਅਚਲ ਅੱਖਰ *p;

	ਜੇ (!ਨਮੂਨਾ && !type)
		ਵਾਪਸ 1;
	ਜੇ (!ਨਮੂਨਾ)
		ਵਾਪਸ 0;

	ਜੇ (!ਲੜੀ_ਤੁਲਨਾ(ਨਮੂਨਾ, "no", 2)) {
		no = 1;
		ਨਮੂਨਾ += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = ਲੜੀ_ਲੰਬਾਈ(type);
	p = ਨਮੂਨਾ;
	ਜਦੋਂ(1) {
		ਜੇ (!ਲੜੀ_ਤੁਲਨਾ(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ਵਾਪਸ 0;
		ਜੇ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ਵਾਪਸ !no;
		p = strchr(p,',');
		ਜੇ (!p)
			ਰੋਕੋ;
		p++;
	}
	ਵਾਪਸ no;
}
