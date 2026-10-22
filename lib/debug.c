/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#অন্তর্ভাব "debug.h"

#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>


শূন্য ul_debug(ধ্রুবক অক্ষর *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

শূন্য ul_debug_prefix(ধ্রুবক অক্ষর *lib, ধ্রুবক অক্ষর *flag,
		     ধ্রুবক শূন্য *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	যদি (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ধ্রুবক কাঠামো ul_debug_maskname flagnames[],
			     ধ্রুবক অক্ষর *mask)
{
	unsigned res;
	অক্ষর *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	যদি (ptr && *ptr && flagnames && flagnames[0].name) {
		অক্ষর *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		যদি (!ms)
			ফেরত res;

		যখন ((name = strtok_r(ms, ",", &ptr))) {
			ধ্রুবক কাঠামো ul_debug_maskname *d;
			ms = ptr;

			জন্য (d = flagnames; d && d->name; d++) {
				যদি (strcmp(name, d->name) == 0) {
					res |= d->mask;
					বিরতি;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			যদি (res == UL_DEBUG_ALL)
				বিরতি;
		}
		মুক্ত(msbuf);
	} অন্যথা যদি (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ফেরত res;
}

শূন্য ul_debug_print_masks(ধ্রুবক অক্ষর *env,
			  ধ্রুবক কাঠামো ul_debug_maskname flagnames[])
{
	ধ্রুবক কাঠামো ul_debug_maskname *d;

	যদি (!flagnames)
		ফেরত;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	জন্য (d = flagnames; d && d->name; d++) {
		যদি (!d->help)
			চালু;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
