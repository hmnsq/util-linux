/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#অন্তর্ভাব "debug.h"

#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>


শূন্য ul_debug(ধ্রুব বর্ণ *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

শূন্য ul_debug_prefix(ধ্রুব বর্ণ *lib, ধ্রুব বর্ণ *flag,
		     ধ্রুব শূন্য *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	যদি (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ধ্রুব সংরচনা ul_debug_maskname flagnames[],
			     ধ্রুব বর্ণ *mask)
{
	unsigned res;
	বর্ণ *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	যদি (ptr && *ptr && flagnames && flagnames[0].name) {
		বর্ণ *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		যদি (!ms)
			প্রত্যাগম res;

		যাবত্ ((name = strtok_r(ms, ",", &ptr))) {
			ধ্রুব সংরচনা ul_debug_maskname *d;
			ms = ptr;

			প্রতি (d = flagnames; d && d->name; d++) {
				যদি (strcmp(name, d->name) == 0) {
					res |= d->mask;
					ভঙ্গ;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			যদি (res == UL_DEBUG_ALL)
				ভঙ্গ;
		}
		মুক্ত(msbuf);
	} অন্যথা যদি (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	প্রত্যাগম res;
}

শূন্য ul_debug_print_masks(ধ্রুব বর্ণ *env,
			  ধ্রুব সংরচনা ul_debug_maskname flagnames[])
{
	ধ্রুব সংরচনা ul_debug_maskname *d;

	যদি (!flagnames)
		প্রত্যাগম;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	প্রতি (d = flagnames; d && d->name; d++) {
		যদি (!d->help)
			অনুবর্ত;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
