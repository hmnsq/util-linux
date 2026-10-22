/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#அந்தர்பாவ "debug.h"

#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>


வெறுமை ul_debug(மாறிலி எழுத்து *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

வெறுமை ul_debug_prefix(மாறிலி எழுத்து *lib, மாறிலி எழுத்து *flag,
		     மாறிலி வெறுமை *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	எனில் (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(மாறிலி கட்டமைப்பு ul_debug_maskname flagnames[],
			     மாறிலி எழுத்து *mask)
{
	unsigned res;
	எழுத்து *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	எனில் (ptr && *ptr && flagnames && flagnames[0].name) {
		எழுத்து *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		எனில் (!ms)
			திரும்பு res;

		போது ((name = strtok_r(ms, ",", &ptr))) {
			மாறிலி கட்டமைப்பு ul_debug_maskname *d;
			ms = ptr;

			க்கு (d = flagnames; d && d->name; d++) {
				எனில் (strcmp(name, d->name) == 0) {
					res |= d->mask;
					நிறுத்து;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			எனில் (res == UL_DEBUG_ALL)
				நிறுத்து;
		}
		விடுவி(msbuf);
	} இல்லையெனில் எனில் (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	திரும்பு res;
}

வெறுமை ul_debug_print_masks(மாறிலி எழுத்து *env,
			  மாறிலி கட்டமைப்பு ul_debug_maskname flagnames[])
{
	மாறிலி கட்டமைப்பு ul_debug_maskname *d;

	எனில் (!flagnames)
		திரும்பு;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	க்கு (d = flagnames; d && d->name; d++) {
		எனில் (!d->help)
			தொடர்;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
