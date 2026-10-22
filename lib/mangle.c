/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ctype.h>

#અન્તર્ભાવ "mangle.h"
#અન્તર્ભાવ "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

સ્થિર અંતઃરેખ પૂર્ણાંક is_unwanted_char(unsigned અક્ષર c)
{
	પાછા c <= 0x20 || c == 0x7f || c == '\\';
}


અક્ષર *mangle(અચલ અક્ષર *s)
{
	અક્ષર *ss, *sp;

	જો (!s)
		પાછા ખાલી;

	ss = sp = malloc(4 * strlen(s) + 1);
	જો (!sp)
		પાછા ખાલી;
	જ્યારે(1) {
		જો (!*s) {
			*sp = '\0';
			તોડો;
		}
		જો (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} નહીંતર
			*sp++ = *s;
		s++;
	}
	પાછા ss;
}


શૂન્ય unmangle_to_buffer(અચલ અક્ષર *s, અક્ષર *buf, size_t len)
{
	size_t sz = 0;

	જો (!s)
		પાછા;

	જ્યારે(*s && sz < len - 1) {
		જો (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} નહીંતર {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(અચલ અક્ષર *s, અક્ષર *buf, size_t len)
{
	size_t sz = 0;
	અચલ અક્ષર *buf0 = buf;

	જો (!s)
		પાછા 0;

	જ્યારે(*s && sz < len - 1) {
		જો (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} નહીંતર {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	પાછા buf - buf0 + 1;
}

size_t unescape_to_buffer(અચલ અક્ષર *s, અચલ અક્ષર *wanted, અક્ષર *buf, size_t len)
{
	size_t sz = 0;
	અચલ અક્ષર *buf0 = buf;

	જ્યારે (*s && sz < len - 1) {
		જો (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} નહીંતર {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	પાછા buf - buf0 + 1;
}

સ્થિર અંતઃરેખ અચલ અક્ષર *skip_nonspaces(અચલ અક્ષર *s)
{
	જ્યારે (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	પાછા s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
અક્ષર *unmangle(અચલ અક્ષર *s, અચલ અક્ષર **end)
{
	અક્ષર *buf;
	અચલ અક્ષર *e;
	size_t sz;

	જો (!s)
		પાછા ખાલી;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	જો (end)
		*end = e;
	જો (e == s)
		પાછા ખાલી;	/* empty string */

	buf = malloc(sz);
	જો (!buf)
		પાછા ખાલી;

	unmangle_to_buffer(s, buf, sz);
	પાછા buf;
}

#ifdef TEST_PROGRAM_MANGLE
#અન્તર્ભાવ <errno.h>
પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અક્ષર *p = ખાલી;
	જો (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		પાછા EXIT_FAILURE;
	}

	જો (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		મુક્ત(p);
	}

	નહીંતર જો (!strcmp(argv[1], "--unmangle")) {
		અક્ષર *x = unmangle(argv[2], ખાલી);

		જો (x) {
			printf("unmangled: '%s'\n", x);
			મુક્ત(x);
		}

		x = strdup(argv[2]);
		જો (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			મુક્ત(x);
		}
	}

	નહીંતર જો (!strcmp(argv[1], "--unescape")) {
		અક્ષર *x = strdup(argv[2]);
		જો (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			મુક્ત(x);
		}
	}

	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
