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

static anta_hstha ចំនួនគត់ is_unwanted_char(unsigned var_na c)
{
	ត្រឡប់ c <= 0x20 || c == 0x7f || c == '\\';
}


var_na *mangle(const var_na *s)
{
	var_na *ss, *sp;

	ប្រសិនបើ (!s)
		ត្រឡប់ NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	ប្រសិនបើ (!sp)
		ត្រឡប់ NULL;
	ខណៈពេល(1) {
		ប្រសិនបើ (!*s) {
			*sp = '\0';
			bha_nga;
		}
		ប្រសិនបើ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} បើមិនដូច្នេះ
			*sp++ = *s;
		s++;
	}
	ត្រឡប់ ss;
}


void unmangle_to_buffer(const var_na *s, var_na *buf, size_t len)
{
	size_t sz = 0;

	ប្រសិនបើ (!s)
		ត្រឡប់;

	ខណៈពេល(*s && sz < len - 1) {
		ប្រសិនបើ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} បើមិនដូច្នេះ {
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

	ប្រសិនបើ (!s)
		ត្រឡប់ 0;

	ខណៈពេល(*s && sz < len - 1) {
		ប្រសិនបើ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} បើមិនដូច្នេះ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ត្រឡប់ buf - buf0 + 1;
}

size_t unescape_to_buffer(const var_na *s, const var_na *wanted, var_na *buf, size_t len)
{
	size_t sz = 0;
	const var_na *buf0 = buf;

	ខណៈពេល (*s && sz < len - 1) {
		ប្រសិនបើ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} បើមិនដូច្នេះ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ត្រឡប់ buf - buf0 + 1;
}

static anta_hstha const var_na *skip_nonspaces(const var_na *s)
{
	ខណៈពេល (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ត្រឡប់ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
var_na *unmangle(const var_na *s, const var_na **end)
{
	var_na *buf;
	const var_na *e;
	size_t sz;

	ប្រសិនបើ (!s)
		ត្រឡប់ NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ប្រសិនបើ (end)
		*end = e;
	ប្រសិនបើ (e == s)
		ត្រឡប់ NULL;	/* empty string */

	buf = malloc(sz);
	ប្រសិនបើ (!buf)
		ត្រឡប់ NULL;

	unmangle_to_buffer(s, buf, sz);
	ត្រឡប់ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	var_na *p = NULL;
	ប្រសិនបើ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ត្រឡប់ EXIT_FAILURE;
	}

	ប្រសិនបើ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	បើមិនដូច្នេះ ប្រសិនបើ (!strcmp(argv[1], "--unmangle")) {
		var_na *x = unmangle(argv[2], NULL);

		ប្រសិនបើ (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		ប្រសិនបើ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	បើមិនដូច្នេះ ប្រសិនបើ (!strcmp(argv[1], "--unescape")) {
		var_na *x = strdup(argv[2]);
		ប្រសិនបើ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	ត្រឡប់ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
