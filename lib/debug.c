/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#अन्तर्भाव "debug.h"

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>


शून्य ul_debug(अचर वर्ण *mesg, ...)
{
	बहु_सूची ap;
	बहु_शुरू(ap, mesg);
	भख_लिखो(मानक_त्रुटि, mesg, ap);
	बहु_पूर्ण(ap);
	ख_अक्षर_दो('\n', मानक_त्रुटि);
}

शून्य ul_debug_prefix(अचर वर्ण *lib, अचर वर्ण *flag,
		     अचर शून्य *handle, अचिन्हित mask)
{
	ख_लिखो(मानक_त्रुटि, "%d: %s: %8s: ", getpid(), lib, flag);
	यदि (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		ख_लिखो(मानक_त्रुटि, "[%p]: ", handle);
}

अचिन्हित ul_debug_parse_mask(अचर संरचना ul_debug_maskname flagnames[],
			     अचर वर्ण *mask)
{
	अचिन्हित res;
	वर्ण *ptr;

	/* let's check for a numeric mask first */
	res = म_से_अदीर्घ(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	यदि (ptr && *ptr && flagnames && flagnames[0].name) {
		वर्ण *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		यदि (!ms)
			फर्काऊ res;

		जबसम्म ((name = strtok_r(ms, ",", &ptr))) {
			अचर संरचना ul_debug_maskname *d;
			ms = ptr;

			लागि (d = flagnames; d && d->name; d++) {
				यदि (म_भेद(name, d->name) == 0) {
					res |= d->mask;
					रोक;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			यदि (res == UL_DEBUG_ALL)
				रोक;
		}
		मुक्त(msbuf);
	} अन्यथा यदि (ptr && म_भेद(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	फर्काऊ res;
}

शून्य ul_debug_print_masks(अचर वर्ण *env,
			  अचर संरचना ul_debug_maskname flagnames[])
{
	अचर संरचना ul_debug_maskname *d;

	यदि (!flagnames)
		फर्काऊ;

	ख_लिखो(मानक_त्रुटि, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	लागि (d = flagnames; d && d->name; d++) {
		यदि (!d->help)
			जारी;
		ख_लिखो(मानक_त्रुटि, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
