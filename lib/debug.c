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
	ถ้า (handle && !(mask & __UL_DEBUG_FL_NOADDR))
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
	ถ้า (ptr && *ptr && flagnames && flagnames[0].name) {
		var_na *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		ถ้า (!ms)
			คืน res;

		ขณะ ((name = strtok_r(ms, ",", &ptr))) {
			const sa_nracanaa ul_debug_maskname *d;
			ms = ptr;

			สำหรับ (d = flagnames; d && d->name; d++) {
				ถ้า (strcmp(name, d->name) == 0) {
					res |= d->mask;
					bha_nga;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ถ้า (res == UL_DEBUG_ALL)
				bha_nga;
		}
		free(msbuf);
	} มิฉะนั้น ถ้า (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	คืน res;
}

void ul_debug_print_masks(const var_na *env,
			  const sa_nracanaa ul_debug_maskname flagnames[])
{
	const sa_nracanaa ul_debug_maskname *d;

	ถ้า (!flagnames)
		คืน;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	สำหรับ (d = flagnames; d && d->name; d++) {
		ถ้า (!d->help)
			continue;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
