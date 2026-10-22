/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#ಅನ್ತರ್ಭಾವ "debug.h"

#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>


ಶೂನ್ಯ ul_debug(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

ಶೂನ್ಯ ul_debug_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lib, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *flag,
		     ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *handle, ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ಆದರೆ (handle && !(ಮುಖವಾಡ & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

ಚಿಹ್ನೆರಹಿತ ul_debug_parse_ಮುಖವಾಡ(ಸ್ಥಿರಾಂಕ ರಚನೆ ul_debug_maskname flagnames[],
			     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಖವಾಡ)
{
	ಚಿಹ್ನೆರಹಿತ res;
	ಅಕ್ಷರ *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(ಮುಖವಾಡ, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ಆದರೆ (ptr && *ptr && flagnames && flagnames[0].name) {
		ಅಕ್ಷರ *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(ಮುಖವಾಡ);
		ಆದರೆ (!ms)
			ಹಿಂತಿರುಗಿ res;

		ಆಗ ((name = strtok_r(ms, ",", &ptr))) {
			ಸ್ಥಿರಾಂಕ ರಚನೆ ul_debug_maskname *d;
			ms = ptr;

			ಗಾಗಿ (d = flagnames; d && d->name; d++) {
				ಆದರೆ (strcmp(name, d->name) == 0) {
					res |= d->ಮುಖವಾಡ;
					ನಿಲ್ಲಿಸು;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ಆದರೆ (res == UL_DEBUG_ALL)
				ನಿಲ್ಲಿಸು;
		}
		ಬಿಡುಗಡೆ(msbuf);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ಹಿಂತಿರುಗಿ res;
}

ಶೂನ್ಯ ul_debug_print_masks(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *env,
			  ಸ್ಥಿರಾಂಕ ರಚನೆ ul_debug_maskname flagnames[])
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ ul_debug_maskname *d;

	ಆದರೆ (!flagnames)
		ಹಿಂತಿರುಗಿ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	ಗಾಗಿ (d = flagnames; d && d->name; d++) {
		ಆದರೆ (!d->help)
			ಮುಂದುವರಿ;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->ಮುಖವಾಡ, d->help);
	}
}
