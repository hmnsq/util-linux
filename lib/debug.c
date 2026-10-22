/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#شامل "debug.h"

#شامل <stdarg.h>
#شامل <stdlib.h>
#شامل <stdio.h>
#شامل <string.h>


تهی ul_debug(ثابت نویسه *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

تهی ul_debug_prefix(ثابت نویسه *lib, ثابت نویسه *flag,
		     ثابت تهی *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	اگر (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ثابت ساختار ul_debug_maskname flagnames[],
			     ثابت نویسه *mask)
{
	unsigned res;
	نویسه *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	اگر (ptr && *ptr && flagnames && flagnames[0].name) {
		نویسه *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		اگر (!ms)
			بازگشت res;

		وقتی ((name = strtok_r(ms, ",", &ptr))) {
			ثابت ساختار ul_debug_maskname *d;
			ms = ptr;

			برای (d = flagnames; d && d->name; d++) {
				اگر (strcmp(name, d->name) == 0) {
					res |= d->mask;
					بشکن;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			اگر (res == UL_DEBUG_ALL)
				بشکن;
		}
		آزاد(msbuf);
	} وگرنه اگر (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	بازگشت res;
}

تهی ul_debug_print_masks(ثابت نویسه *env,
			  ثابت ساختار ul_debug_maskname flagnames[])
{
	ثابت ساختار ul_debug_maskname *d;

	اگر (!flagnames)
		بازگشت;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	برای (d = flagnames; d && d->name; d++) {
		اگر (!d->help)
			ادامه;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
