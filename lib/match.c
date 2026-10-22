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
ಪೂರ್ಣಾಂಕ ಹೊಂದಿಕೆ_fstype(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ)
{
	ಪೂರ್ಣಾಂಕ no = 0;		/* negated types list */
	ಪೂರ್ಣಾಂಕ len;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;

	ಆದರೆ (!ನಮೂನೆ && !type)
		ಹಿಂತಿರುಗಿ 1;
	ಆದರೆ (!ನಮೂನೆ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ನಮೂನೆ, "no", 2)) {
		no = 1;
		ನಮೂನೆ += 2;
	}

	/* Does type occur in types, separated by commas? */
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(type);
	p = ನಮೂನೆ;
	ಆಗ(1) {
		ಆದರೆ (!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(p, "no", 2) && !c_strncasecmp(p+2, type, len) &&
		    (p[len+2] == 0 || p[len+2] == ','))
			ಹಿಂತಿರುಗಿ 0;
		ಆದರೆ (c_strncasecmp(p, type, len) == 0 && (p[len] == 0 || p[len] == ','))
			ಹಿಂತಿರುಗಿ !no;
		p = strchr(p,',');
		ಆದರೆ (!p)
			ನಿಲ್ಲಿಸು;
		p++;
	}
	ಹಿಂತಿರುಗಿ no;
}
