/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#अन्तर्भाव "debug.h"

#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>


शून्य ul_debug(ध्रुव वर्ण *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

शून्य ul_debug_उपसर्गः(ध्रुव वर्ण *lib, ध्रुव वर्ण *flag,
		     ध्रुव शून्य *handle, अचिह्नित आवरणम्)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	यदि (handle && !(आवरणम् & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

अचिह्नित ul_debug_parse_आवरणम्(ध्रुव संरचना ul_debug_maskname flagnames[],
			     ध्रुव वर्ण *आवरणम्)
{
	अचिह्नित res;
	वर्ण *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(आवरणम्, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	यदि (ptr && *ptr && flagnames && flagnames[0].name) {
		वर्ण *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(आवरणम्);
		यदि (!ms)
			प्रत्यागम res;

		यावत् ((name = strtok_r(ms, ",", &ptr))) {
			ध्रुव संरचना ul_debug_maskname *d;
			ms = ptr;

			प्रति (d = flagnames; d && d->name; d++) {
				यदि (वाक्यं_तुलय(name, d->name) == 0) {
					res |= d->आवरणम्;
					भङ्ग;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			यदि (res == UL_DEBUG_ALL)
				भङ्ग;
		}
		मुक्त(msbuf);
	} अन्यथा यदि (ptr && वाक्यं_तुलय(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	प्रत्यागम res;
}

शून्य ul_debug_print_masks(ध्रुव वर्ण *env,
			  ध्रुव संरचना ul_debug_maskname flagnames[])
{
	ध्रुव संरचना ul_debug_maskname *d;

	यदि (!flagnames)
		प्रत्यागम;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	प्रति (d = flagnames; d && d->name; d++) {
		यदि (!d->help)
			अनुवर्त;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->आवरणम्, d->help);
	}
}
