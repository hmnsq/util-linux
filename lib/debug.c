/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#ಅನ್ತರ್ಭಾವ "debug.h"

#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>


ಶೂನ್ಯ ul_debug(ಧ್ರುವ ವರ್ಣ *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

ಶೂನ್ಯ ul_debug_prefix(ಧ್ರುವ ವರ್ಣ *lib, ಧ್ರುವ ವರ್ಣ *flag,
		     ಧ್ರುವ ಶೂನ್ಯ *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ಯದಿ (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(ಧ್ರುವ ಸಂರಚನಾ ul_debug_maskname flagnames[],
			     ಧ್ರುವ ವರ್ಣ *mask)
{
	unsigned res;
	ವರ್ಣ *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ಯದಿ (ptr && *ptr && flagnames && flagnames[0].name) {
		ವರ್ಣ *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		ಯದಿ (!ms)
			ಪ್ರತ್ಯಾಗಮ res;

		ಯಾವತ್ ((name = strtok_r(ms, ",", &ptr))) {
			ಧ್ರುವ ಸಂರಚನಾ ul_debug_maskname *d;
			ms = ptr;

			ಪ್ರತಿ (d = flagnames; d && d->name; d++) {
				ಯದಿ (strcmp(name, d->name) == 0) {
					res |= d->mask;
					ಭಙ್ಗ;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ಯದಿ (res == UL_DEBUG_ALL)
				ಭಙ್ಗ;
		}
		ಮುಕ್ತ(msbuf);
	} ಅನ್ಯಥಾ ಯದಿ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ಪ್ರತ್ಯಾಗಮ res;
}

ಶೂನ್ಯ ul_debug_print_masks(ಧ್ರುವ ವರ್ಣ *env,
			  ಧ್ರುವ ಸಂರಚನಾ ul_debug_maskname flagnames[])
{
	ಧ್ರುವ ಸಂರಚನಾ ul_debug_maskname *d;

	ಯದಿ (!flagnames)
		ಪ್ರತ್ಯಾಗಮ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	ಪ್ರತಿ (d = flagnames; d && d->name; d++) {
		ಯದಿ (!d->help)
			ಅನುವರ್ತ;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
