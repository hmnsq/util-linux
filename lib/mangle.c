/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

#ಅನ್ತರ್ಭಾವ "mangle.h"
#ಅನ್ತರ್ಭಾವ "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ is_unwanted_char(unsigned ವರ್ಣ c)
{
	ಪ್ರತ್ಯಾಗಮ c <= 0x20 || c == 0x7f || c == '\\';
}


ವರ್ಣ *mangle(ಧ್ರುವ ವರ್ಣ *s)
{
	ವರ್ಣ *ss, *sp;

	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ss = sp = malloc(4 * strlen(s) + 1);
	ಯದಿ (!sp)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯಾವತ್(1) {
		ಯದಿ (!*s) {
			*sp = '\0';
			ಭಙ್ಗ;
		}
		ಯದಿ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ಅನ್ಯಥಾ
			*sp++ = *s;
		s++;
	}
	ಪ್ರತ್ಯಾಗಮ ss;
}


ಶೂನ್ಯ unmangle_to_buffer(ಧ್ರುವ ವರ್ಣ *s, ವರ್ಣ *buf, size_t len)
{
	size_t sz = 0;

	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ;

	ಯಾವತ್(*s && sz < len - 1) {
		ಯದಿ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ಅನ್ಯಥಾ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ಧ್ರುವ ವರ್ಣ *s, ವರ್ಣ *buf, size_t len)
{
	size_t sz = 0;
	ಧ್ರುವ ವರ್ಣ *buf0 = buf;

	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್(*s && sz < len - 1) {
		ಯದಿ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ಅನ್ಯಥಾ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ಪ್ರತ್ಯಾಗಮ buf - buf0 + 1;
}

size_t unescape_to_buffer(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *wanted, ವರ್ಣ *buf, size_t len)
{
	size_t sz = 0;
	ಧ್ರುವ ವರ್ಣ *buf0 = buf;

	ಯಾವತ್ (*s && sz < len - 1) {
		ಯದಿ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ಅನ್ಯಥಾ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ಪ್ರತ್ಯಾಗಮ buf - buf0 + 1;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಧ್ರುವ ವರ್ಣ *skip_nonspaces(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯಾವತ್ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ಪ್ರತ್ಯಾಗಮ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
ವರ್ಣ *unmangle(ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ **end)
{
	ವರ್ಣ *buf;
	ಧ್ರುವ ವರ್ಣ *e;
	size_t sz;

	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ಯದಿ (end)
		*end = e;
	ಯದಿ (e == s)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;	/* empty string */

	buf = malloc(sz);
	ಯದಿ (!buf)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	unmangle_to_buffer(s, buf, sz);
	ಪ್ರತ್ಯಾಗಮ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#ಅನ್ತರ್ಭಾವ <errno.h>
ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ವರ್ಣ *p = ರಿಕ್ತ;
	ಯದಿ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	ಯದಿ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		ಮುಕ್ತ(p);
	}

	ಅನ್ಯಥಾ ಯದಿ (!strcmp(argv[1], "--unmangle")) {
		ವರ್ಣ *x = unmangle(argv[2], ರಿಕ್ತ);

		ಯದಿ (x) {
			printf("unmangled: '%s'\n", x);
			ಮುಕ್ತ(x);
		}

		x = strdup(argv[2]);
		ಯದಿ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			ಮುಕ್ತ(x);
		}
	}

	ಅನ್ಯಥಾ ಯದಿ (!strcmp(argv[1], "--unescape")) {
		ವರ್ಣ *x = strdup(argv[2]);
		ಯದಿ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			ಮುಕ್ತ(x);
		}
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
