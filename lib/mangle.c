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

স্থির অন্তঃরেখ পূর্ণসংখ্যা is_unwanted_char(unsigned অক্ষর c)
{
	ফেরত c <= 0x20 || c == 0x7f || c == '\\';
}


অক্ষর *mangle(ধ্রুবক অক্ষর *s)
{
	অক্ষর *ss, *sp;

	যদি (!s)
		ফেরত ফাঁকা;

	ss = sp = malloc(4 * strlen(s) + 1);
	যদি (!sp)
		ফেরত ফাঁকা;
	যখন(1) {
		যদি (!*s) {
			*sp = '\0';
			বিরতি;
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
	ফেরত ss;
}


শূন্য unmangle_to_বাফার(ধ্রুবক অক্ষর *s, অক্ষর *buf, size_t len)
{
	size_t sz = 0;

	যদি (!s)
		ফেরত;

	যখন(*s && sz < len - 1) {
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

size_t unhexmangle_to_বাফার(ধ্রুবক অক্ষর *s, অক্ষর *buf, size_t len)
{
	size_t sz = 0;
	ধ্রুবক অক্ষর *buf0 = buf;

	যদি (!s)
		ফেরত 0;

	যখন(*s && sz < len - 1) {
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
	ফেরত buf - buf0 + 1;
}

size_t unescape_to_বাফার(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *wanted, অক্ষর *buf, size_t len)
{
	size_t sz = 0;
	ধ্রুবক অক্ষর *buf0 = buf;

	যখন (*s && sz < len - 1) {
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
	ফেরত buf - buf0 + 1;
}

স্থির অন্তঃরেখ ধ্রুবক অক্ষর *skip_nonspaces(ধ্রুবক অক্ষর *s)
{
	যখন (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ফেরত s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
অক্ষর *unmangle(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর **end)
{
	অক্ষর *buf;
	ধ্রুবক অক্ষর *e;
	size_t sz;

	যদি (!s)
		ফেরত ফাঁকা;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	যদি (end)
		*end = e;
	যদি (e == s)
		ফেরত ফাঁকা;	/* empty string */

	buf = malloc(sz);
	যদি (!buf)
		ফেরত ফাঁকা;

	unmangle_to_বাফার(s, buf, sz);
	ফেরত buf;
}

#ifdef TEST_PROGRAM_MANGLE
#অন্তর্ভাব <errno.h>
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	অক্ষর *p = ফাঁকা;
	যদি (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ফেরত EXIT_FAILURE;
	}

	যদি (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		মুক্ত(p);
	}

	অন্যথা যদি (!strcmp(argv[1], "--unmangle")) {
		অক্ষর *x = unmangle(argv[2], ফাঁকা);

		যদি (x) {
			printf("unmangled: '%s'\n", x);
			মুক্ত(x);
		}

		x = strdup(argv[2]);
		যদি (x) {
			unmangle_to_বাফার(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			মুক্ত(x);
		}
	}

	অন্যথা যদি (!strcmp(argv[1], "--unescape")) {
		অক্ষর *x = strdup(argv[2]);
		যদি (x) {
			unescape_to_বাফার(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			মুক্ত(x);
		}
	}

	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
