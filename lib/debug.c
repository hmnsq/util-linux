/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#ضمن "debug.h"

#ضمن <stdarg.h>
#ضمن <stdlib.h>
#ضمن <stdio.h>
#ضمن <string.h>


فراغ ul_debug(ثابت حرف *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

فراغ ul_debug_prefix(ثابت حرف *lib, ثابت حرف *flag,
		     ثابت فراغ *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ܐܢ (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ثابت بنية ul_debug_maskname flagnames[],
			     ثابت حرف *mask)
{
	unsigned res;
	حرف *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ܐܢ (ptr && *ptr && flagnames && flagnames[0].name) {
		حرف *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		ܐܢ (!ms)
			ܦܢܐ res;

		ܟܡܐ ((name = strtok_r(ms, ",", &ptr))) {
			ثابت بنية ul_debug_maskname *d;
			ms = ptr;

			لكل (d = flagnames; d && d->name; d++) {
				ܐܢ (strcmp(name, d->name) == 0) {
					res |= d->mask;
					اكسر;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ܐܢ (res == UL_DEBUG_ALL)
				اكسر;
		}
		حرر(msbuf);
	} ܐܚܪܢܐ ܐܢ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ܦܢܐ res;
}

فراغ ul_debug_print_masks(ثابت حرف *env,
			  ثابت بنية ul_debug_maskname flagnames[])
{
	ثابت بنية ul_debug_maskname *d;

	ܐܢ (!flagnames)
		ܦܢܐ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	لكل (d = flagnames; d && d->name; d++) {
		ܐܢ (!d->help)
			استمر;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
