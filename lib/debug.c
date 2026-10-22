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

शून्य ul_debug_prefix(ध्रुव वर्ण *lib, ध्रुव वर्ण *flag,
		     ध्रुव शून्य *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	सचे (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ध्रुव संरचना ul_debug_maskname flagnames[],
			     ध्रुव वर्ण *mask)
{
	unsigned res;
	वर्ण *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	सचे (ptr && *ptr && flagnames && flagnames[0].name) {
		वर्ण *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		सचे (!ms)
			पटिनिवत्त res;

		याव ((name = strtok_r(ms, ",", &ptr))) {
			ध्रुव संरचना ul_debug_maskname *d;
			ms = ptr;

			अत्थाय (d = flagnames; d && d->name; d++) {
				सचे (strcmp(name, d->name) == 0) {
					res |= d->mask;
					भङ्ग;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			सचे (res == UL_DEBUG_ALL)
				भङ्ग;
		}
		मुक्त(msbuf);
	} अञ्ञथा सचे (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	पटिनिवत्त res;
}

शून्य ul_debug_print_masks(ध्रुव वर्ण *env,
			  ध्रुव संरचना ul_debug_maskname flagnames[])
{
	ध्रुव संरचना ul_debug_maskname *d;

	सचे (!flagnames)
		पटिनिवत्त;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	अत्थाय (d = flagnames; d && d->name; d++) {
		सचे (!d->help)
			अनुवर्त;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
