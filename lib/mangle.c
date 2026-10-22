/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <ctype.h>

#அந்தர்பாவ "mangle.h"
#அந்தர்பாவ "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

நிலையான உள்வரி முழு is_unwanted_char(unsigned எழுத்து c)
{
	திரும்பு c <= 0x20 || c == 0x7f || c == '\\';
}


எழுத்து *mangle(மாறிலி எழுத்து *s)
{
	எழுத்து *ss, *sp;

	எனில் (!s)
		திரும்பு வெற்று;

	ss = sp = malloc(4 * strlen(s) + 1);
	எனில் (!sp)
		திரும்பு வெற்று;
	போது(1) {
		எனில் (!*s) {
			*sp = '\0';
			நிறுத்து;
		}
		எனில் (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} இல்லையெனில்
			*sp++ = *s;
		s++;
	}
	திரும்பு ss;
}


வெறுமை unmangle_to_buffer(மாறிலி எழுத்து *s, எழுத்து *buf, size_t len)
{
	size_t sz = 0;

	எனில் (!s)
		திரும்பு;

	போது(*s && sz < len - 1) {
		எனில் (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} இல்லையெனில் {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(மாறிலி எழுத்து *s, எழுத்து *buf, size_t len)
{
	size_t sz = 0;
	மாறிலி எழுத்து *buf0 = buf;

	எனில் (!s)
		திரும்பு 0;

	போது(*s && sz < len - 1) {
		எனில் (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} இல்லையெனில் {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	திரும்பு buf - buf0 + 1;
}

size_t unescape_to_buffer(மாறிலி எழுத்து *s, மாறிலி எழுத்து *wanted, எழுத்து *buf, size_t len)
{
	size_t sz = 0;
	மாறிலி எழுத்து *buf0 = buf;

	போது (*s && sz < len - 1) {
		எனில் (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} இல்லையெனில் {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	திரும்பு buf - buf0 + 1;
}

நிலையான உள்வரி மாறிலி எழுத்து *skip_nonspaces(மாறிலி எழுத்து *s)
{
	போது (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	திரும்பு s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
எழுத்து *unmangle(மாறிலி எழுத்து *s, மாறிலி எழுத்து **end)
{
	எழுத்து *buf;
	மாறிலி எழுத்து *e;
	size_t sz;

	எனில் (!s)
		திரும்பு வெற்று;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	எனில் (end)
		*end = e;
	எனில் (e == s)
		திரும்பு வெற்று;	/* empty string */

	buf = malloc(sz);
	எனில் (!buf)
		திரும்பு வெற்று;

	unmangle_to_buffer(s, buf, sz);
	திரும்பு buf;
}

#ifdef TEST_PROGRAM_MANGLE
#அந்தர்பாவ <errno.h>
முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	எழுத்து *p = வெற்று;
	எனில் (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		திரும்பு EXIT_FAILURE;
	}

	எனில் (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		விடுவி(p);
	}

	இல்லையெனில் எனில் (!strcmp(argv[1], "--unmangle")) {
		எழுத்து *x = unmangle(argv[2], வெற்று);

		எனில் (x) {
			printf("unmangled: '%s'\n", x);
			விடுவி(x);
		}

		x = strdup(argv[2]);
		எனில் (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			விடுவி(x);
		}
	}

	இல்லையெனில் எனில் (!strcmp(argv[1], "--unescape")) {
		எழுத்து *x = strdup(argv[2]);
		எனில் (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			விடுவி(x);
		}
	}

	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
