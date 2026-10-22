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
	ᰀᰧᰌ c <= 0x20 || c == 0x7f || c == '\\';
}


var_na *mangle(const var_na *s)
{
	var_na *ss, *sp;

	ᰀᰧ (!s)
		ᰀᰧᰌ NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	ᰀᰧ (!sp)
		ᰀᰧᰌ NULL;
	ᰀᰬ(1) {
		ᰀᰧ (!*s) {
			*sp = '\0';
			bha_nga;
		}
		ᰀᰧ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ᰀᰧᰀ
			*sp++ = *s;
		s++;
	}
	ᰀᰧᰌ ss;
}


void unmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;

	ᰀᰧ (!s)
		ᰀᰧᰌ;

	ᰀᰬ(*s && sz < len - 1) {
		ᰀᰧ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ᰀᰧᰀ {
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

	ᰀᰧ (!s)
		ᰀᰧᰌ 0;

	ᰀᰬ(*s && sz < len - 1) {
		ᰀᰧ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ᰀᰧᰀ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ᰀᰧᰌ buf - buf0 + 1;
}

size_t unescape_to_buffer(const var_na *s, const var_na *wanted, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ᰀᰬ (*s && sz < len - 1) {
		ᰀᰧ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ᰀᰧᰀ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ᰀᰧᰌ buf - buf0 + 1;
}

static anta_hstha const var_na *skip_nonspaces(const var_na *s)
{
	ᰀᰬ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ᰀᰧᰌ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
var_na *unmangle(const var_na *s, const var_na **end)
{
	var_na *buf;
	const var_na *e;
	size_t sz;

	ᰀᰧ (!s)
		ᰀᰧᰌ NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ᰀᰧ (end)
		*end = e;
	ᰀᰧ (e == s)
		ᰀᰧᰌ NULL;	/* empty string */

	buf = malloc(sz);
	ᰀᰧ (!buf)
		ᰀᰧᰌ NULL;

	unmangle_to_buffer(s, buf, sz);
	ᰀᰧᰌ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	var_na *p = NULL;
	ᰀᰧ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ᰀᰧᰌ EXIT_FAILURE;
	}

	ᰀᰧ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	ᰀᰧᰀ ᰀᰧ (!strcmp(argv[1], "--unmangle")) {
		var_na *x = unmangle(argv[2], NULL);

		ᰀᰧ (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		ᰀᰧ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	ᰀᰧᰀ ᰀᰧ (!strcmp(argv[1], "--unescape")) {
		var_na *x = strdup(argv[2]);
		ᰀᰧ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	ᰀᰧᰌ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
