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

#ವ್ಯಾಖ್ಯಾನಿಸು isoctal(a)		(((a) & ~7) == '0')

#ವ್ಯಾಖ್ಯಾನಿಸು from_hex(c)		(ಅಂಕಿಯೇ(c) ? c - '0' : tolower(c) - 'a' + 10)

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ is_unwanted_char(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c)
{
	ಹಿಂತಿರುಗಿ c <= 0x20 || c == 0x7f || c == '\\';
}


ಅಕ್ಷರ *mangle(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಅಕ್ಷರ *ss, *sp;

	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ss = sp = ಸ್ಮರಣೆ_ಕೊಡು(4 * ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) + 1);
	ಆದರೆ (!sp)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆಗ(1) {
		ಆದರೆ (!*s) {
			*sp = '\0';
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ಇಲ್ಲದಿದ್ದರೆ
			*sp++ = *s;
		s++;
	}
	ಹಿಂತಿರುಗಿ ss;
}


ಶೂನ್ಯ unmangle_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಅಕ್ಷರ *buf, size_t len)
{
	size_t sz = 0;

	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ;

	ಆಗ(*s && sz < len - 1) {
		ಆದರೆ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಅಕ್ಷರ *buf, size_t len)
{
	size_t sz = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf0 = buf;

	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ 0;

	ಆಗ(*s && sz < len - 1) {
		ಆದರೆ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ಹಿಂತಿರುಗಿ buf - buf0 + 1;
}

size_t unescape_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *wanted, ಅಕ್ಷರ *buf, size_t len)
{
	size_t sz = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf0 = buf;

	ಆಗ (*s && sz < len - 1) {
		ಆದರೆ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ಇಲ್ಲದಿದ್ದರೆ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ಹಿಂತಿರುಗಿ buf - buf0 + 1;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *skip_nonspaces(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆಗ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ಹಿಂತಿರುಗಿ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
ಅಕ್ಷರ *unmangle(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **ಕೊನೆ)
{
	ಅಕ್ಷರ *buf;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e;
	size_t sz;

	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ಆದರೆ (ಕೊನೆ)
		*ಕೊನೆ = e;
	ಆದರೆ (e == s)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;	/* empty string */

	buf = ಸ್ಮರಣೆ_ಕೊಡು(sz);
	ಆದರೆ (!buf)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	unmangle_ಗಮ್ಯ_ಬಫರ್(s, buf, sz);
	ಹಿಂತಿರುಗಿ buf;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_MANGLE
#ಅನ್ತರ್ಭಾವ <errno.h>
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
	ಅಕ್ಷರ *p = ಖಾಲಿ;
	ಆದರೆ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						ಕಾರ್ಯಕ್ರಮ_invocation_short_name);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಆದರೆ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		ಬರೆ("mangled: '%s'\n", p);
		ಬಿಡುಗಡೆ(p);
	}

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!strcmp(argv[1], "--unmangle")) {
		ಅಕ್ಷರ *x = unmangle(argv[2], ಖಾಲಿ);

		ಆದರೆ (x) {
			ಬರೆ("unmangled: '%s'\n", x);
			ಬಿಡುಗಡೆ(x);
		}

		x = strdup(argv[2]);
		ಆದರೆ (x) {
			unmangle_ಗಮ್ಯ_ಬಫರ್(x, x, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(x) + 1);

			ಬರೆ("self-unmangled: '%s'\n", x);
			ಬಿಡುಗಡೆ(x);
		}
	}

	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!strcmp(argv[1], "--unescape")) {
		ಅಕ್ಷರ *x = strdup(argv[2]);
		ಆದರೆ (x) {
			unescape_ಗಮ್ಯ_ಬಫರ್(x, ",\"", x, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(x) + 1);

			ಬರೆ("self-unescaped: '%s'\n", x);
			ಬಿಡುಗಡೆ(x);
		}
	}

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_MANGLE */
