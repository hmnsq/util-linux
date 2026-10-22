/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#include "debug.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void ul_debug(const char *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

void ul_debug_prefix(const char *lib, const char *flag,
		     const void *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	如果 (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(const struct ul_debug_maskname flagnames[],
			     const char *mask)
{
	unsigned res;
	char *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	如果 (ptr && *ptr && flagnames && flagnames[0].name) {
		char *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		如果 (!ms)
			返回 res;

		当 ((name = strtok_r(ms, ",", &ptr))) {
			const struct ul_debug_maskname *d;
			ms = ptr;

			为 (d = flagnames; d && d->name; d++) {
				如果 (strcmp(name, d->name) == 0) {
					res |= d->mask;
					break;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			如果 (res == UL_DEBUG_ALL)
				break;
		}
		free(msbuf);
	} 否则 如果 (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	返回 res;
}

void ul_debug_print_masks(const char *env,
			  const struct ul_debug_maskname flagnames[])
{
	const struct ul_debug_maskname *d;

	如果 (!flagnames)
		返回;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	为 (d = flagnames; d && d->name; d++) {
		如果 (!d->help)
			continue;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
