/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mangle.h"
#include "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

static anta_hstha puur_naa_nka is_unwanted_char(unsigned var_na c)
{
	ꦮꦭꦶ c <= 0x20 || c == 0x7f || c == '\\';
}


var_na *mangle(const var_na *s)
{
	var_na *ss, *sp;

	ꦪꦼꦤ꧀ (!s)
		ꦮꦭꦶ NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	ꦪꦼꦤ꧀ (!sp)
		ꦮꦭꦶ NULL;
	ꦱꦼꦢꦼꦭ(1) {
		ꦪꦼꦤ꧀ (!*s) {
			*sp = '\0';
			bha_nga;
		}
		ꦪꦼꦤ꧀ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ꦭꦶꦪ
			*sp++ = *s;
		s++;
	}
	ꦮꦭꦶ ss;
}


void unmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;

	ꦪꦼꦤ꧀ (!s)
		ꦮꦭꦶ;

	ꦱꦼꦢꦼꦭ(*s && sz < len - 1) {
		ꦪꦼꦤ꧀ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ꦭꦶꦪ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ꦪꦼꦤ꧀ (!s)
		ꦮꦭꦶ 0;

	ꦱꦼꦢꦼꦭ(*s && sz < len - 1) {
		ꦪꦼꦤ꧀ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ꦭꦶꦪ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ꦮꦭꦶ buf - buf0 + 1;
}

size_t unescape_to_buffer(const var_na *s, const var_na *wanted, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ꦱꦼꦢꦼꦭ (*s && sz < len - 1) {
		ꦪꦼꦤ꧀ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ꦭꦶꦪ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ꦮꦭꦶ buf - buf0 + 1;
}

static anta_hstha const var_na *skip_nonspaces(const var_na *s)
{
	ꦱꦼꦢꦼꦭ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ꦮꦭꦶ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
var_na *unmangle(const var_na *s, const var_na **end)
{
	var_na *buf;
	const var_na *e;
	size_t sz;

	ꦪꦼꦤ꧀ (!s)
		ꦮꦭꦶ NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ꦪꦼꦤ꧀ (end)
		*end = e;
	ꦪꦼꦤ꧀ (e == s)
		ꦮꦭꦶ NULL;	/* empty string */

	buf = malloc(sz);
	ꦪꦼꦤ꧀ (!buf)
		ꦮꦭꦶ NULL;

	unmangle_to_buffer(s, buf, sz);
	ꦮꦭꦶ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	var_na *p = NULL;
	ꦪꦼꦤ꧀ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ꦮꦭꦶ EXIT_FAILURE;
	}

	ꦪꦼꦤ꧀ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	ꦭꦶꦪ ꦪꦼꦤ꧀ (!strcmp(argv[1], "--unmangle")) {
		var_na *x = unmangle(argv[2], NULL);

		ꦪꦼꦤ꧀ (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		ꦪꦼꦤ꧀ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	ꦭꦶꦪ ꦪꦼꦤ꧀ (!strcmp(argv[1], "--unescape")) {
		var_na *x = strdup(argv[2]);
		ꦪꦼꦤ꧀ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	ꦮꦭꦶ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
