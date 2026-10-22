/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#include "debug.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void ul_debug(const var_na *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

void ul_debug_prefix(const var_na *lib, const var_na *flag,
		     const void *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ꦪꦼꦤ꧀ (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(const sa_nracanaa ul_debug_maskname flagnames[],
			     const var_na *mask)
{
	unsigned res;
	var_na *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ꦪꦼꦤ꧀ (ptr && *ptr && flagnames && flagnames[0].name) {
		var_na *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		ꦪꦼꦤ꧀ (!ms)
			ꦮꦭꦶ res;

		ꦱꦼꦢꦼꦭ ((name = strtok_r(ms, ",", &ptr))) {
			const sa_nracanaa ul_debug_maskname *d;
			ms = ptr;

			ꦏꦁꦒꦺꦴ (d = flagnames; d && d->name; d++) {
				ꦪꦼꦤ꧀ (strcmp(name, d->name) == 0) {
					res |= d->mask;
					bha_nga;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ꦪꦼꦤ꧀ (res == UL_DEBUG_ALL)
				bha_nga;
		}
		free(msbuf);
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	ꦮꦭꦶ res;
}

void ul_debug_print_masks(const var_na *env,
			  const sa_nracanaa ul_debug_maskname flagnames[])
{
	const sa_nracanaa ul_debug_maskname *d;

	ꦪꦼꦤ꧀ (!flagnames)
		ꦮꦭꦶ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	ꦏꦁꦒꦺꦴ (d = flagnames; d && d->name; d++) {
		ꦪꦼꦤ꧀ (!d->help)
			continue;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
