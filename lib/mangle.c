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

static inline 정수 is_unwanted_char(unsigned char c)
{
	반환 c <= 0x20 || c == 0x7f || c == '\\';
}


char *mangle(const char *s)
{
	char *ss, *sp;

	만약 (!s)
		반환 NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	만약 (!sp)
		반환 NULL;
	동안(1) {
		만약 (!*s) {
			*sp = '\0';
			break;
		}
		만약 (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} 아니면
			*sp++ = *s;
		s++;
	}
	반환 ss;
}


void unmangle_to_buffer(const char *s, char *buf, size_t len)
{
	size_t sz = 0;

	만약 (!s)
		반환;

	동안(*s && sz < len - 1) {
		만약 (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} 아니면 {
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

	만약 (!s)
		반환 0;

	동안(*s && sz < len - 1) {
		만약 (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} 아니면 {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	반환 buf - buf0 + 1;
}

size_t unescape_to_buffer(const char *s, const char *wanted, char *buf, size_t len)
{
	size_t sz = 0;
	const char *buf0 = buf;

	동안 (*s && sz < len - 1) {
		만약 (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} 아니면 {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	반환 buf - buf0 + 1;
}

static inline const char *skip_nonspaces(const char *s)
{
	동안 (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	반환 s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
char *unmangle(const char *s, const char **end)
{
	char *buf;
	const char *e;
	size_t sz;

	만약 (!s)
		반환 NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	만약 (end)
		*end = e;
	만약 (e == s)
		반환 NULL;	/* empty string */

	buf = malloc(sz);
	만약 (!buf)
		반환 NULL;

	unmangle_to_buffer(s, buf, sz);
	반환 buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
정수 main(정수 argc, char *argv[])
{
	char *p = NULL;
	만약 (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		반환 EXIT_FAILURE;
	}

	만약 (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	아니면 만약 (!strcmp(argv[1], "--unmangle")) {
		char *x = unmangle(argv[2], NULL);

		만약 (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		만약 (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	아니면 만약 (!strcmp(argv[1], "--unescape")) {
		char *x = strdup(argv[2]);
		만약 (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	반환 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
