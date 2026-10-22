/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#شامل "debug.h"

#شامل <stdarg.h>
#شامل <stdlib.h>
#شامل <stdio.h>
#شامل <string.h>


خالی ul_debug(ثابت حرف *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

خالی ul_debug_سابقہ(ثابت حرف *lib, ثابت حرف *flag,
		     ثابت خالی *handle, غیرعلامتی نقاب)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	اگر (handle && !(نقاب & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

غیرعلامتی ul_debug_parse_نقاب(ثابت ساخت ul_debug_maskname flagnames[],
			     ثابت حرف *نقاب)
{
	غیرعلامتی res;
	حرف *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(نقاب, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	اگر (ptr && *ptr && flagnames && flagnames[0].name) {
		حرف *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(نقاب);
		اگر (!ms)
			واپس res;

		جب ((name = strtok_r(ms, ",", &ptr))) {
			ثابت ساخت ul_debug_maskname *d;
			ms = ptr;

			برائے (d = flagnames; d && d->name; d++) {
				اگر (strcmp(name, d->name) == 0) {
					res |= d->نقاب;
					توڑو;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			اگر (res == UL_DEBUG_ALL)
				توڑو;
		}
		آزاد(msbuf);
	} ورنہ اگر (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	واپس res;
}

خالی ul_debug_print_masks(ثابت حرف *env,
			  ثابت ساخت ul_debug_maskname flagnames[])
{
	ثابت ساخت ul_debug_maskname *d;

	اگر (!flagnames)
		واپس;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	برائے (d = flagnames; d && d->name; d++) {
		اگر (!d->help)
			جاری;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->نقاب, d->help);
	}
}
