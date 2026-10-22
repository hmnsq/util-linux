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

static inline целое is_unwanted_char(unsigned char c)
{
	вернуть c <= 0x20 || c == 0x7f || c == '\\';
}


char *mangle(const char *s)
{
	char *ss, *sp;

	если (!s)
		вернуть NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	если (!sp)
		вернуть NULL;
	пока(1) {
		если (!*s) {
			*sp = '\0';
			break;
		}
		если (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} иначе
			*sp++ = *s;
		s++;
	}
	вернуть ss;
}


void unmangle_to_buffer(const char *s, char *buf, size_t len)
{
	size_t sz = 0;

	если (!s)
		вернуть;

	пока(*s && sz < len - 1) {
		если (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} иначе {
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

	если (!s)
		вернуть 0;

	пока(*s && sz < len - 1) {
		если (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} иначе {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	вернуть buf - buf0 + 1;
}

size_t unescape_to_buffer(const char *s, const char *wanted, char *buf, size_t len)
{
	size_t sz = 0;
	const char *buf0 = buf;

	пока (*s && sz < len - 1) {
		если (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} иначе {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	вернуть buf - buf0 + 1;
}

static inline const char *skip_nonspaces(const char *s)
{
	пока (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	вернуть s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
char *unmangle(const char *s, const char **end)
{
	char *buf;
	const char *e;
	size_t sz;

	если (!s)
		вернуть NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	если (end)
		*end = e;
	если (e == s)
		вернуть NULL;	/* empty string */

	buf = malloc(sz);
	если (!buf)
		вернуть NULL;

	unmangle_to_buffer(s, buf, sz);
	вернуть buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
целое основной(целое argc, char *argv[])
{
	char *p = NULL;
	если (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		вернуть EXIT_FAILURE;
	}

	если (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	иначе если (!strcmp(argv[1], "--unmangle")) {
		char *x = unmangle(argv[2], NULL);

		если (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		если (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	иначе если (!strcmp(argv[1], "--unescape")) {
		char *x = strdup(argv[2]);
		если (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	вернуть EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
