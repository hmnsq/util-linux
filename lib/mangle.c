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

static inline 整数 is_unwanted_char(unsigned char c)
{
	戻る c <= 0x20 || c == 0x7f || c == '\\';
}


char *mangle(const char *s)
{
	char *ss, *sp;

	もし (!s)
		戻る NULL;

	ss = sp = malloc(4 * strlen(s) + 1);
	もし (!sp)
		戻る NULL;
	ながら(1) {
		もし (!*s) {
			*sp = '\0';
			break;
		}
		もし (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} 否则
			*sp++ = *s;
		s++;
	}
	戻る ss;
}


void unmangle_to_buffer(const char *s, char *buf, size_t len)
{
	size_t sz = 0;

	もし (!s)
		戻る;

	ながら(*s && sz < len - 1) {
		もし (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} 否则 {
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

	もし (!s)
		戻る 0;

	ながら(*s && sz < len - 1) {
		もし (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} 否则 {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	戻る buf - buf0 + 1;
}

size_t unescape_to_buffer(const char *s, const char *wanted, char *buf, size_t len)
{
	size_t sz = 0;
	const char *buf0 = buf;

	ながら (*s && sz < len - 1) {
		もし (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} 否则 {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	戻る buf - buf0 + 1;
}

static inline const char *skip_nonspaces(const char *s)
{
	ながら (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	戻る s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
char *unmangle(const char *s, const char **end)
{
	char *buf;
	const char *e;
	size_t sz;

	もし (!s)
		戻る NULL;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	もし (end)
		*end = e;
	もし (e == s)
		戻る NULL;	/* empty string */

	buf = malloc(sz);
	もし (!buf)
		戻る NULL;

	unmangle_to_buffer(s, buf, sz);
	戻る buf;
}

#ifdef TEST_PROGRAM_MANGLE
#include <errno.h>
整数 メインページ(整数 argc, char *argv[])
{
	char *p = NULL;
	もし (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		戻る EXIT_FAILURE;
	}

	もし (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		free(p);
	}

	否则 もし (!strcmp(argv[1], "--unmangle")) {
		char *x = unmangle(argv[2], NULL);

		もし (x) {
			printf("unmangled: '%s'\n", x);
			free(x);
		}

		x = strdup(argv[2]);
		もし (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			free(x);
		}
	}

	否则 もし (!strcmp(argv[1], "--unescape")) {
		char *x = strdup(argv[2]);
		もし (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			free(x);
		}
	}

	戻る EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
