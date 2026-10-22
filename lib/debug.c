/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#ଅନ୍ତର୍ଭାଵ "debug.h"

#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>


ଶୂନ୍ୟ ul_debug(ଅଚଳ ଅକ୍ଷର *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

ଶୂନ୍ୟ ul_debug_prefix(ଅଚଳ ଅକ୍ଷର *lib, ଅଚଳ ଅକ୍ଷର *flag,
		     ଅଚଳ ଶୂନ୍ୟ *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ଯଦି (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ଅଚଳ ସଂରଚନା ul_debug_maskname flagnames[],
			     ଅଚଳ ଅକ୍ଷର *mask)
{
	unsigned res;
	ଅକ୍ଷର *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ଯଦି (ptr && *ptr && flagnames && flagnames[0].name) {
		ଅକ୍ଷର *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		ଯଦି (!ms)
			ଫେରନ୍ତୁ res;

		ଯେତେବେଳେ ((name = strtok_r(ms, ",", &ptr))) {
			ଅଚଳ ସଂରଚନା ul_debug_maskname *d;
			ms = ptr;

			ପାଇଁ (d = flagnames; d && d->name; d++) {
				ଯଦି (strcmp(name, d->name) == 0) {
					res |= d->mask;
					ଭାଙ୍ଗ;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ଯଦି (res == UL_DEBUG_ALL)
				ଭାଙ୍ଗ;
		}
		ମୁକ୍ତ(msbuf);
	} ଅନ୍ୟଥା ଯଦି (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ଫେରନ୍ତୁ res;
}

ଶୂନ୍ୟ ul_debug_print_masks(ଅଚଳ ଅକ୍ଷର *env,
			  ଅଚଳ ସଂରଚନା ul_debug_maskname flagnames[])
{
	ଅଚଳ ସଂରଚନା ul_debug_maskname *d;

	ଯଦି (!flagnames)
		ଫେରନ୍ତୁ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	ପାଇଁ (d = flagnames; d && d->name; d++) {
		ଯଦି (!d->help)
			ଜାରି;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
