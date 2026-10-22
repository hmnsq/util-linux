/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <ctype.h>

#অন্তর্ভাব "mangle.h"
#অন্তর্ভাব "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

স্থির অন্তঃস্থ পূর্ণাঙ্ক is_unwanted_char(unsigned বর্ণ c)
{
	প্রত্যাগম c <= 0x20 || c == 0x7f || c == '\\';
}


বর্ণ *mangle(ধ্রুব বর্ণ *s)
{
	বর্ণ *ss, *sp;

	যদি (!s)
		প্রত্যাগম রিক্ত;

	ss = sp = malloc(4 * strlen(s) + 1);
	যদি (!sp)
		প্রত্যাগম রিক্ত;
	যাবত্(1) {
		যদি (!*s) {
			*sp = '\0';
			ভঙ্গ;
		}
		যদি (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} অন্যথা
			*sp++ = *s;
		s++;
	}
	প্রত্যাগম ss;
}


শূন্য unmangle_to_buffer(ধ্রুব বর্ণ *s, বর্ণ *buf, size_t len)
{
	size_t sz = 0;

	যদি (!s)
		প্রত্যাগম;

	যাবত্(*s && sz < len - 1) {
		যদি (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} অন্যথা {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ধ্রুব বর্ণ *s, বর্ণ *buf, size_t len)
{
	size_t sz = 0;
	ধ্রুব বর্ণ *buf0 = buf;

	যদি (!s)
		প্রত্যাগম 0;

	যাবত্(*s && sz < len - 1) {
		যদি (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} অন্যথা {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	প্রত্যাগম buf - buf0 + 1;
}

size_t unescape_to_buffer(ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *wanted, বর্ণ *buf, size_t len)
{
	size_t sz = 0;
	ধ্রুব বর্ণ *buf0 = buf;

	যাবত্ (*s && sz < len - 1) {
		যদি (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} অন্যথা {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	প্রত্যাগম buf - buf0 + 1;
}

স্থির অন্তঃস্থ ধ্রুব বর্ণ *skip_nonspaces(ধ্রুব বর্ণ *s)
{
	যাবত্ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	প্রত্যাগম s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
বর্ণ *unmangle(ধ্রুব বর্ণ *s, ধ্রুব বর্ণ **end)
{
	বর্ণ *buf;
	ধ্রুব বর্ণ *e;
	size_t sz;

	যদি (!s)
		প্রত্যাগম রিক্ত;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	যদি (end)
		*end = e;
	যদি (e == s)
		প্রত্যাগম রিক্ত;	/* empty string */

	buf = malloc(sz);
	যদি (!buf)
		প্রত্যাগম রিক্ত;

	unmangle_to_buffer(s, buf, sz);
	প্রত্যাগম buf;
}

#ifdef TEST_PROGRAM_MANGLE
#অন্তর্ভাব <errno.h>
পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	বর্ণ *p = রিক্ত;
	যদি (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		প্রত্যাগম EXIT_FAILURE;
	}

	যদি (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		মুক্ত(p);
	}

	অন্যথা যদি (!strcmp(argv[1], "--unmangle")) {
		বর্ণ *x = unmangle(argv[2], রিক্ত);

		যদি (x) {
			printf("unmangled: '%s'\n", x);
			মুক্ত(x);
		}

		x = strdup(argv[2]);
		যদি (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			মুক্ত(x);
		}
	}

	অন্যথা যদি (!strcmp(argv[1], "--unescape")) {
		বর্ণ *x = strdup(argv[2]);
		যদি (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			মুক্ত(x);
		}
	}

	প্রত্যাগম EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
