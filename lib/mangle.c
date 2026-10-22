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

static inline số_nguyên is_unwanted_char(unsigned char c)
{
	trả_về c <= 0x20 || c == 0x7f || c == '\\';
}


char *mangle(const char *s)
{
	char *ss, *sp;

	nếu (!s)
		trả_về NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	nếu (!sp)
		trả_về NULL;
	trong_khi(1) {
		nếu (!*s) {
			*sp = '\0';
			break;
		}
		nếu (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} không_thì
			*sp++ = *s;
		s++;
	}
	trả_về ss;
}


void unmangle_to_buffer(const char *s, char *buf, size_t len)
{
	size_t sz = 0;

	nếu (!s)
		trả_về;

	trong_khi(*s && sz < len - 1) {
		nếu (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} không_thì {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(const char *s, char *buf, size_t len)
{
	size_t sz = 0;
	const char *buf0 = buf;

	nếu (!s)
		trả_về 0;

	trong_khi(*s && sz < len - 1) {
		nếu (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} không_thì {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	trả_về buf - buf0 + 1;
}

size_t unescape_to_buffer(const char *s, const char *wanted, char *buf, size_t len)
{
	size_t sz = 0;
	const char *buf0 = buf;

	trong_khi (*s && sz < len - 1) {
		nếu (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} không_thì {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	trả_về buf - buf0 + 1;
}

static inline const char *skip_nonspaces(const char *s)
{
	trong_khi (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	trả_về s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
char *unmangle(const char *s, const char **end)
{
	char *buf;
	const char *e;
	size_t sz;

	nếu (!s)
		trả_về NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	nếu (end)
		*end = e;
	nếu (e == s)
		trả_về NULL;	/* empty string */

	buf = malloc(sz);
	nếu (!buf)
		trả_về NULL;

	unmangle_to_buffer(s, buf, sz);
	trả_về buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
số_nguyên main(số_nguyên argc, char *argv[])
{
	char *p = NULL;
	nếu (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		trả_về EXIT_FAILURE;
	}

	nếu (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	không_thì nếu (!strcmp(argv[1], "--unmangle")) {
		char *x = unmangle(argv[2], NULL);

		nếu (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		nếu (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	không_thì nếu (!strcmp(argv[1], "--unescape")) {
		char *x = strdup(argv[2]);
		nếu (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
