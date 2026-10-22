/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>


ਖਾਲੀ ul_debug(ਅਚਲ ਅੱਖਰ *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

ਖਾਲੀ ul_debug_ਅਗੇਤਰ(ਅਚਲ ਅੱਖਰ *lib, ਅਚਲ ਅੱਖਰ *flag,
		     ਅਚਲ ਖਾਲੀ *handle, ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ਜੇ (handle && !(ਨਕਾਬ & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ul_debug_parse_ਨਕਾਬ(ਅਚਲ ਬਣਤਰ ul_debug_maskname flagnames[],
			     ਅਚਲ ਅੱਖਰ *ਨਕਾਬ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ res;
	ਅੱਖਰ *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(ਨਕਾਬ, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ਜੇ (ptr && *ptr && flagnames && flagnames[0].name) {
		ਅੱਖਰ *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(ਨਕਾਬ);
		ਜੇ (!ms)
			ਵਾਪਸ res;

		ਜਦੋਂ ((name = strtok_r(ms, ",", &ptr))) {
			ਅਚਲ ਬਣਤਰ ul_debug_maskname *d;
			ms = ptr;

			ਲਈ (d = flagnames; d && d->name; d++) {
				ਜੇ (strcmp(name, d->name) == 0) {
					res |= d->ਨਕਾਬ;
					ਰੋਕੋ;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ਜੇ (res == UL_DEBUG_ALL)
				ਰੋਕੋ;
		}
		ਮੁਕਤ(msbuf);
	} ਨਹੀਂਤਾਂ ਜੇ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ਵਾਪਸ res;
}

ਖਾਲੀ ul_debug_print_masks(ਅਚਲ ਅੱਖਰ *env,
			  ਅਚਲ ਬਣਤਰ ul_debug_maskname flagnames[])
{
	ਅਚਲ ਬਣਤਰ ul_debug_maskname *d;

	ਜੇ (!flagnames)
		ਵਾਪਸ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	ਲਈ (d = flagnames; d && d->name; d++) {
		ਜੇ (!d->help)
			ਜਾਰੀ;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->ਨਕਾਬ, d->help);
	}
}
