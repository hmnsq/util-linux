/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#समावेश "debug.h"

#समावेश <stdarg.h>
#समावेश <stdlib.h>
#समावेश <stdio.h>
#समावेश <string.h>


व्योम ul_debug(स्थिर अक्षर *mesg, ...)
{
	बहु_सूची ap;
	बहु_शुरू(ap, mesg);
	भख_लिखो(मानक_त्रुटि, mesg, ap);
	बहु_पूर्ण(ap);
	ख_अक्षर_दो('\n', मानक_त्रुटि);
}

व्योम ul_debug_prefix(स्थिर अक्षर *lib, स्थिर अक्षर *flag,
		     स्थिर व्योम *handle, अचिन्हित mask)
{
	ख_लिखो(मानक_त्रुटि, "%d: %s: %8s: ", getpid(), lib, flag);
	अगर (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		ख_लिखो(मानक_त्रुटि, "[%p]: ", handle);
}

अचिन्हित ul_debug_parse_mask(स्थिर काष्ठा ul_debug_maskname flagnames[],
			     स्थिर अक्षर *mask)
{
	अचिन्हित res;
	अक्षर *ptr;

	/* let's check for a numeric mask first */
	res = म_से_अदीर्घ(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	अगर (ptr && *ptr && flagnames && flagnames[0].name) {
		अक्षर *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		अगर (!ms)
			वापस res;

		जबतक ((name = strtok_r(ms, ",", &ptr))) {
			स्थिर काष्ठा ul_debug_maskname *d;
			ms = ptr;

			क्रम (d = flagnames; d && d->name; d++) {
				अगर (म_भेद(name, d->name) == 0) {
					res |= d->mask;
					अवरोध;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			अगर (res == UL_DEBUG_ALL)
				अवरोध;
		}
		मुक्त(msbuf);
	} अन्यथा अगर (ptr && म_भेद(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	वापस res;
}

व्योम ul_debug_print_masks(स्थिर अक्षर *env,
			  स्थिर काष्ठा ul_debug_maskname flagnames[])
{
	स्थिर काष्ठा ul_debug_maskname *d;

	अगर (!flagnames)
		वापस;

	ख_लिखो(मानक_त्रुटि, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	क्रम (d = flagnames; d && d->name; d++) {
		अगर (!d->help)
			जारी;
		ख_लिखो(मानक_त्रुटि, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
