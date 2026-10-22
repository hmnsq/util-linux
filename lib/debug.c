/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#অন্তর্ভাৱ "debug.h"

#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>


শূন্য ul_debug(ধ্রুৱ ৱর্ণ *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

শূন্য ul_debug_prefix(ধ্রুৱ ৱর্ণ *lib, ধ্রুৱ ৱর্ণ *flag,
		     ধ্রুৱ শূন্য *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	যদি (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ধ্রুৱ সংরচনা ul_debug_maskname flagnames[],
			     ধ্রুৱ ৱর্ণ *mask)
{
	unsigned res;
	ৱর্ণ *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	যদি (ptr && *ptr && flagnames && flagnames[0].name) {
		ৱর্ণ *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		যদি (!ms)
			ঘূৰাই res;

		যেতিয়ালৈ ((name = strtok_r(ms, ",", &ptr))) {
			ধ্রুৱ সংরচনা ul_debug_maskname *d;
			ms = ptr;

			বাবে (d = flagnames; d && d->name; d++) {
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

	ঘূৰাই res;
}

শূন্য ul_debug_print_masks(ধ্রুৱ ৱর্ণ *env,
			  ধ্রুৱ সংরচনা ul_debug_maskname flagnames[])
{
	ধ্রুৱ সংরচনা ul_debug_maskname *d;

	যদি (!flagnames)
		ঘূৰাই;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	বাবে (d = flagnames; d && d->name; d++) {
		যদি (!d->help)
			অনুৱর্ত;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
