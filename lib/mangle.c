/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>

#ଅନ୍ତର୍ଭାଵ "mangle.h"
#ଅନ୍ତର୍ଭାଵ "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_unwanted_char(unsigned ଅକ୍ଷର c)
{
	ଫେରନ୍ତୁ c <= 0x20 || c == 0x7f || c == '\\';
}


ଅକ୍ଷର *mangle(ଅଚଳ ଅକ୍ଷର *s)
{
	ଅକ୍ଷର *ss, *sp;

	ଯଦି (!s)
		ଫେରନ୍ତୁ ଖାଲି;

	ss = sp = malloc(4 * strlen(s) + 1);
	ଯଦି (!sp)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯେତେବେଳେ(1) {
		ଯଦି (!*s) {
			*sp = '\0';
			ଭାଙ୍ଗ;
		}
		ଯଦି (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ଅନ୍ୟଥା
			*sp++ = *s;
		s++;
	}
	ଫେରନ୍ତୁ ss;
}


ଶୂନ୍ୟ unmangle_to_buffer(ଅଚଳ ଅକ୍ଷର *s, ଅକ୍ଷର *buf, size_t len)
{
	size_t sz = 0;

	ଯଦି (!s)
		ଫେରନ୍ତୁ;

	ଯେତେବେଳେ(*s && sz < len - 1) {
		ଯଦି (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ଅନ୍ୟଥା {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ଅଚଳ ଅକ୍ଷର *s, ଅକ୍ଷର *buf, size_t len)
{
	size_t sz = 0;
	ଅଚଳ ଅକ୍ଷର *buf0 = buf;

	ଯଦି (!s)
		ଫେରନ୍ତୁ 0;

	ଯେତେବେଳେ(*s && sz < len - 1) {
		ଯଦି (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ଅନ୍ୟଥା {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ଫେରନ୍ତୁ buf - buf0 + 1;
}

size_t unescape_to_buffer(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *wanted, ଅକ୍ଷର *buf, size_t len)
{
	size_t sz = 0;
	ଅଚଳ ଅକ୍ଷର *buf0 = buf;

	ଯେତେବେଳେ (*s && sz < len - 1) {
		ଯଦି (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ଅନ୍ୟଥା {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ଫେରନ୍ତୁ buf - buf0 + 1;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଅଚଳ ଅକ୍ଷର *skip_nonspaces(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯେତେବେଳେ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ଫେରନ୍ତୁ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
ଅକ୍ଷର *unmangle(ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର **end)
{
	ଅକ୍ଷର *buf;
	ଅଚଳ ଅକ୍ଷର *e;
	size_t sz;

	ଯଦି (!s)
		ଫେରନ୍ତୁ ଖାଲି;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ଯଦି (end)
		*end = e;
	ଯଦି (e == s)
		ଫେରନ୍ତୁ ଖାଲି;	/* empty string */

	buf = malloc(sz);
	ଯଦି (!buf)
		ଫେରନ୍ତୁ ଖାଲି;

	unmangle_to_buffer(s, buf, sz);
	ଫେରନ୍ତୁ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#ଅନ୍ତର୍ଭାଵ <errno.h>
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ଅକ୍ଷର *p = ଖାଲି;
	ଯଦି (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଯଦି (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		ମୁକ୍ତ(p);
	}

	ଅନ୍ୟଥା ଯଦି (!strcmp(argv[1], "--unmangle")) {
		ଅକ୍ଷର *x = unmangle(argv[2], ଖାଲି);

		ଯଦି (x) {
			printf("unmangled: '%s'\n", x);
			ମୁକ୍ତ(x);
		}

		x = strdup(argv[2]);
		ଯଦି (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			ମୁକ୍ତ(x);
		}
	}

	ଅନ୍ୟଥା ଯଦି (!strcmp(argv[1], "--unescape")) {
		ଅକ୍ଷର *x = strdup(argv[2]);
		ଯଦି (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			ମୁକ୍ତ(x);
		}
	}

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
