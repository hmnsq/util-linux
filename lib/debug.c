/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#અન્તર્ભાવ "debug.h"

#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>


શૂન્ય ul_debug(અચલ અક્ષર *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

શૂન્ય ul_debug_prefix(અચલ અક્ષર *lib, અચલ અક્ષર *flag,
		     અચલ શૂન્ય *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	જો (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(અચલ સંરચના ul_debug_maskname flagnames[],
			     અચલ અક્ષર *mask)
{
	unsigned res;
	અક્ષર *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	જો (ptr && *ptr && flagnames && flagnames[0].name) {
		અક્ષર *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		જો (!ms)
			પાછા res;

		જ્યારે ((name = strtok_r(ms, ",", &ptr))) {
			અચલ સંરચના ul_debug_maskname *d;
			ms = ptr;

			માટે (d = flagnames; d && d->name; d++) {
				જો (strcmp(name, d->name) == 0) {
					res |= d->mask;
					તોડો;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			જો (res == UL_DEBUG_ALL)
				તોડો;
		}
		મુક્ત(msbuf);
	} નહીંતર જો (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	પાછા res;
}

શૂન્ય ul_debug_print_masks(અચલ અક્ષર *env,
			  અચલ સંરચના ul_debug_maskname flagnames[])
{
	અચલ સંરચના ul_debug_maskname *d;

	જો (!flagnames)
		પાછા;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	માટે (d = flagnames; d && d->name; d++) {
		જો (!d->help)
			ચાલુ;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
