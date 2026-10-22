/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#അന്തര്ഭാവ "debug.h"

#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>


ശൂന്യം ul_debug(സ്ഥിരം അക്ഷരം *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

ശൂന്യം ul_debug_prefix(സ്ഥിരം അക്ഷരം *lib, സ്ഥിരം അക്ഷരം *flag,
		     സ്ഥിരം ശൂന്യം *handle, unsigned mask)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	എങ്കിൽ (handle && !(mask & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

unsigned ul_debug_parse_mask(സ്ഥിരം ഘടന ul_debug_maskname flagnames[],
			     സ്ഥിരം അക്ഷരം *mask)
{
	unsigned res;
	അക്ഷരം *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(mask, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	എങ്കിൽ (ptr && *ptr && flagnames && flagnames[0].name) {
		അക്ഷരം *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(mask);
		എങ്കിൽ (!ms)
			തിരികെ res;

		ആകുമ്പോൾ ((name = strtok_r(ms, ",", &ptr))) {
			സ്ഥിരം ഘടന ul_debug_maskname *d;
			ms = ptr;

			വേണ്ടി (d = flagnames; d && d->name; d++) {
				എങ്കിൽ (strcmp(name, d->name) == 0) {
					res |= d->mask;
					നിർത്തുക;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			എങ്കിൽ (res == UL_DEBUG_ALL)
				നിർത്തുക;
		}
		സ്വതന്ത്രം(msbuf);
	} അല്ലെങ്കിൽ എങ്കിൽ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	തിരികെ res;
}

ശൂന്യം ul_debug_print_masks(സ്ഥിരം അക്ഷരം *env,
			  സ്ഥിരം ഘടന ul_debug_maskname flagnames[])
{
	സ്ഥിരം ഘടന ul_debug_maskname *d;

	എങ്കിൽ (!flagnames)
		തിരികെ;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	വേണ്ടി (d = flagnames; d && d->name; d++) {
		എങ്കിൽ (!d->help)
			തുടരുക;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->mask, d->help);
	}
}
