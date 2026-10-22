/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#अन्तर्भाव "debug.h"

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>


शून्य ul_debug(स्थिरांक अक्षर *mesg, ...)
{
	बहु_सूची ap;
	बहु_शुरू(ap, mesg);
	भख_लिखो(मानक_त्रुटि, mesg, ap);
	बहु_पूर्ण(ap);
	ख_अक्षर_दो('\n', मानक_त्रुटि);
}

शून्य ul_debug_prefix(स्थिरांक अक्षर *lib, स्थिरांक अक्षर *flag,
		     स्थिरांक शून्य *handle, अचिन्हित mask)
{
	ख_लिखो(मानक_त्रुटि, "%d: %s: %8s: ", getpid(), lib, flag);
	जर (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		ख_लिखो(मानक_त्रुटि, "[%p]: ", handle);
}

अचिन्हित ul_debug_parse_mask(स्थिरांक संरचना ul_debug_maskname flagnames[],
			     स्थिरांक अक्षर *mask)
{
	अचिन्हित res;
	अक्षर *ptr;

	/* let's check for a numeric mask first */
	res = म_से_अदीर्घ(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	जर (ptr && *ptr && flagnames && flagnames[0].name) {
		अक्षर *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		जर (!ms)
			परत res;

		पर्यंत ((name = strtok_r(ms, ",", &ptr))) {
			स्थिरांक संरचना ul_debug_maskname *d;
			ms = ptr;

			साठी (d = flagnames; d && d->name; d++) {
				जर (म_भेद(name, d->name) == 0) {
					res |= d->mask;
					थांबा;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			जर (res == UL_DEBUG_ALL)
				थांबा;
		}
		मुक्त(msbuf);
	} अन्यथा जर (ptr && म_भेद(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	परत res;
}

शून्य ul_debug_print_masks(स्थिरांक अक्षर *env,
			  स्थिरांक संरचना ul_debug_maskname flagnames[])
{
	स्थिरांक संरचना ul_debug_maskname *d;

	जर (!flagnames)
		परत;

	ख_लिखो(मानक_त्रुटि, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	साठी (d = flagnames; d && d->name; d++) {
		जर (!d->help)
			सुरूठेवा;
		ख_लिखो(मानक_त्रुटि, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
