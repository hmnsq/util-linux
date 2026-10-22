/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <ctype.h>

#ضمن "mangle.h"
#ضمن "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

ساكن مضمن صحيح is_unwanted_char(unsigned حرف c)
{
	ܦܢܐ c <= 0x20 || c == 0x7f || c == '\\';
}


حرف *mangle(ثابت حرف *s)
{
	حرف *ss, *sp;

	ܐܢ (!s)
		ܦܢܐ عدم;

	ss = sp = malloc(4 * strlen(s) + 1);
	ܐܢ (!sp)
		ܦܢܐ عدم;
	ܟܡܐ(1) {
		ܐܢ (!*s) {
			*sp = '\0';
			اكسر;
		}
		ܐܢ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ܐܚܪܢܐ
			*sp++ = *s;
		s++;
	}
	ܦܢܐ ss;
}


فراغ unmangle_to_buffer(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;

	ܐܢ (!s)
		ܦܢܐ;

	ܟܡܐ(*s && sz < len - 1) {
		ܐܢ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ܐܚܪܢܐ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	ܐܢ (!s)
		ܦܢܐ 0;

	ܟܡܐ(*s && sz < len - 1) {
		ܐܢ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ܐܚܪܢܐ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ܦܢܐ buf - buf0 + 1;
}

size_t unescape_to_buffer(ثابت حرف *s, ثابت حرف *wanted, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	ܟܡܐ (*s && sz < len - 1) {
		ܐܢ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ܐܚܪܢܐ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ܦܢܐ buf - buf0 + 1;
}

ساكن مضمن ثابت حرف *skip_nonspaces(ثابت حرف *s)
{
	ܟܡܐ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ܦܢܐ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
حرف *unmangle(ثابت حرف *s, ثابت حرف **end)
{
	حرف *buf;
	ثابت حرف *e;
	size_t sz;

	ܐܢ (!s)
		ܦܢܐ عدم;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ܐܢ (end)
		*end = e;
	ܐܢ (e == s)
		ܦܢܐ عدم;	/* empty string */

	buf = malloc(sz);
	ܐܢ (!buf)
		ܦܢܐ عدم;

	unmangle_to_buffer(s, buf, sz);
	ܦܢܐ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#ضمن <errno.h>
صحيح main(صحيح argc, حرف *argv[])
{
	حرف *p = عدم;
	ܐܢ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ܦܢܐ EXIT_FAILURE;
	}

	ܐܢ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		حرر(p);
	}

	ܐܚܪܢܐ ܐܢ (!strcmp(argv[1], "--unmangle")) {
		حرف *x = unmangle(argv[2], عدم);

		ܐܢ (x) {
			printf("unmangled: '%s'\n", x);
			حرر(x);
		}

		x = strdup(argv[2]);
		ܐܢ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			حرر(x);
		}
	}

	ܐܚܪܢܐ ܐܢ (!strcmp(argv[1], "--unescape")) {
		حرف *x = strdup(argv[2]);
		ܐܢ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			حرر(x);
		}
	}

	ܦܢܐ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
