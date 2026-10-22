/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */

#అన్తర్భావ "debug.h"

#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>


శూన్య ul_debug(స్థిరరాశి అక్షరం *mesg, ...)
{
	va_list ap;
	va_start(ap, mesg);
	vfprintf(stderr, mesg, ap);
	va_end(ap);
	fputc('\n', stderr);
}

శూన్య ul_debug_ఉపసర్గ(స్థిరరాశి అక్షరం *lib, స్థిరరాశి అక్షరం *flag,
		     స్థిరరాశి శూన్య *handle, గుర్తులేని ముసుగు)
{
	fprintf(stderr, "%d: %s: %8s: ", getpid(), lib, flag);
	ఒకవేళ (handle && !(ముసుగు & __UL_DEBUG_FL_NOADDR))
		fprintf(stderr, "[%p]: ", handle);
}

గుర్తులేని ul_debug_parse_ముసుగు(స్థిరరాశి నిర్మాణం ul_debug_maskname flagnames[],
			     స్థిరరాశి అక్షరం *ముసుగు)
{
	గుర్తులేని res;
	అక్షరం *ptr;

	/* let's check for a numeric mask first */
	res = strtoul(ముసుగు, &ptr, 0);

	/* perhaps it's a comma-separated string? */
	ఒకవేళ (ptr && *ptr && flagnames && flagnames[0].name) {
		అక్షరం *msbuf, *ms, *name;
		res = 0;

		ms = msbuf = strdup(ముసుగు);
		ఒకవేళ (!ms)
			తిరిగి res;

		అయితే ((name = strtok_r(ms, ",", &ptr))) {
			స్థిరరాశి నిర్మాణం ul_debug_maskname *d;
			ms = ptr;

			కొరకు (d = flagnames; d && d->name; d++) {
				ఒకవేళ (strcmp(name, d->name) == 0) {
					res |= d->ముసుగు;
					ఆపు;
				}
			}
			/* nothing else we can do by OR-ing the mask */
			ఒకవేళ (res == UL_DEBUG_ALL)
				ఆపు;
		}
		విడుదల(msbuf);
	} లేకపోతే ఒకవేళ (ptr && strcmp(ptr, "all") == 0)
		res = UL_DEBUG_ALL;

	తిరిగి res;
}

శూన్య ul_debug_print_masks(స్థిరరాశి అక్షరం *env,
			  స్థిరరాశి నిర్మాణం ul_debug_maskname flagnames[])
{
	స్థిరరాశి నిర్మాణం ul_debug_maskname *d;

	ఒకవేళ (!flagnames)
		తిరిగి;

	fprintf(stderr, "Available \"%s=<name>[,...]|<mask>\" debug masks:\n",
			env);
	కొరకు (d = flagnames; d && d->name; d++) {
		ఒకవేళ (!d->help)
			కొనసాగు;
		fprintf(stderr, "   %-8s [0x%06x] : %s\n",
				d->name, d->ముసుగు, d->help);
	}
}
