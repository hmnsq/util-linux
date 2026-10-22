/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <ctype.h>

#అన్తర్భావ "mangle.h"
#అన్తర్భావ "c.h"

#నిర్వచించు isoctal(a)		(((a) & ~7) == '0')

#నిర్వచించు from_hex(c)		(అంకెనా(c) ? c - '0' : tolower(c) - 'a' + 10)

స్థిర అంతఃవరుస పూర్ణాంకం is_unwanted_char(గుర్తులేని అక్షరం c)
{
	తిరిగి c <= 0x20 || c == 0x7f || c == '\\';
}


అక్షరం *mangle(స్థిరరాశి అక్షరం *s)
{
	అక్షరం *ss, *sp;

	ఒకవేళ (!s)
		తిరిగి ఖాళీ;

	ss = sp = జ్ఞాపకం_ఇవ్వు(4 * అక్షరశ్రేణి_పొడవు(s) + 1);
	ఒకవేళ (!sp)
		తిరిగి ఖాళీ;
	అయితే(1) {
		ఒకవేళ (!*s) {
			*sp = '\0';
			ఆపు;
		}
		ఒకవేళ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} లేకపోతే
			*sp++ = *s;
		s++;
	}
	తిరిగి ss;
}


శూన్య unmangle_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *s, అక్షరం *buf, size_t len)
{
	size_t sz = 0;

	ఒకవేళ (!s)
		తిరిగి;

	అయితే(*s && sz < len - 1) {
		ఒకవేళ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} లేకపోతే {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *s, అక్షరం *buf, size_t len)
{
	size_t sz = 0;
	స్థిరరాశి అక్షరం *buf0 = buf;

	ఒకవేళ (!s)
		తిరిగి 0;

	అయితే(*s && sz < len - 1) {
		ఒకవేళ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} లేకపోతే {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	తిరిగి buf - buf0 + 1;
}

size_t unescape_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *wanted, అక్షరం *buf, size_t len)
{
	size_t sz = 0;
	స్థిరరాశి అక్షరం *buf0 = buf;

	అయితే (*s && sz < len - 1) {
		ఒకవేళ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} లేకపోతే {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	తిరిగి buf - buf0 + 1;
}

స్థిర అంతఃవరుస స్థిరరాశి అక్షరం *skip_nonspaces(స్థిరరాశి అక్షరం *s)
{
	అయితే (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	తిరిగి s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
అక్షరం *unmangle(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం **చివర)
{
	అక్షరం *buf;
	స్థిరరాశి అక్షరం *e;
	size_t sz;

	ఒకవేళ (!s)
		తిరిగి ఖాళీ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ఒకవేళ (చివర)
		*చివర = e;
	ఒకవేళ (e == s)
		తిరిగి ఖాళీ;	/* empty string */

	buf = జ్ఞాపకం_ఇవ్వు(sz);
	ఒకవేళ (!buf)
		తిరిగి ఖాళీ;

	unmangle_గమ్యం_బఫరు(s, buf, sz);
	తిరిగి buf;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_MANGLE
#అన్తర్భావ <errno.h>
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	అక్షరం *p = ఖాళీ;
	ఒకవేళ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						కార్యక్రమం_invocation_short_name);
		తిరిగి EXIT_FAILURE;
	}

	ఒకవేళ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		రాయి("mangled: '%s'\n", p);
		విడుదల(p);
	}

	లేకపోతే ఒకవేళ (!strcmp(argv[1], "--unmangle")) {
		అక్షరం *x = unmangle(argv[2], ఖాళీ);

		ఒకవేళ (x) {
			రాయి("unmangled: '%s'\n", x);
			విడుదల(x);
		}

		x = strdup(argv[2]);
		ఒకవేళ (x) {
			unmangle_గమ్యం_బఫరు(x, x, అక్షరశ్రేణి_పొడవు(x) + 1);

			రాయి("self-unmangled: '%s'\n", x);
			విడుదల(x);
		}
	}

	లేకపోతే ఒకవేళ (!strcmp(argv[1], "--unescape")) {
		అక్షరం *x = strdup(argv[2]);
		ఒకవేళ (x) {
			unescape_గమ్యం_బఫరు(x, ",\"", x, అక్షరశ్రేణి_పొడవు(x) + 1);

			రాయి("self-unescaped: '%s'\n", x);
			విడుదల(x);
		}
	}

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_MANGLE */
