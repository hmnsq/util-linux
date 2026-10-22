/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "mangle.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isoctal(a)		(((a) & ~7) == '0')

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ from_hex(c)		(ਅੰਕ_ਹੈ(c) ? c - '0' : tolower(c) - 'a' + 10)

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ is_unwanted_char(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c)
{
	ਵਾਪਸ c <= 0x20 || c == 0x7f || c == '\\';
}


ਅੱਖਰ *mangle(ਅਚਲ ਅੱਖਰ *s)
{
	ਅੱਖਰ *ss, *sp;

	ਜੇ (!s)
		ਵਾਪਸ ਨਿਰਾ;

	ss = sp = ਸਮ੍ਰਿਤੀ_ਦਿਓ(4 * ਲੜੀ_ਲੰਬਾਈ(s) + 1);
	ਜੇ (!sp)
		ਵਾਪਸ ਨਿਰਾ;
	ਜਦੋਂ(1) {
		ਜੇ (!*s) {
			*sp = '\0';
			ਰੋਕੋ;
		}
		ਜੇ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ਨਹੀਂਤਾਂ
			*sp++ = *s;
		s++;
	}
	ਵਾਪਸ ss;
}


ਖਾਲੀ unmangle_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *s, ਅੱਖਰ *buf, size_t len)
{
	size_t sz = 0;

	ਜੇ (!s)
		ਵਾਪਸ;

	ਜਦੋਂ(*s && sz < len - 1) {
		ਜੇ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ਨਹੀਂਤਾਂ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *s, ਅੱਖਰ *buf, size_t len)
{
	size_t sz = 0;
	ਅਚਲ ਅੱਖਰ *buf0 = buf;

	ਜੇ (!s)
		ਵਾਪਸ 0;

	ਜਦੋਂ(*s && sz < len - 1) {
		ਜੇ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ਨਹੀਂਤਾਂ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ਵਾਪਸ buf - buf0 + 1;
}

size_t unescape_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *wanted, ਅੱਖਰ *buf, size_t len)
{
	size_t sz = 0;
	ਅਚਲ ਅੱਖਰ *buf0 = buf;

	ਜਦੋਂ (*s && sz < len - 1) {
		ਜੇ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ਨਹੀਂਤਾਂ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	ਵਾਪਸ buf - buf0 + 1;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਅਚਲ ਅੱਖਰ *skip_nonspaces(ਅਚਲ ਅੱਖਰ *s)
{
	ਜਦੋਂ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	ਵਾਪਸ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
ਅੱਖਰ *unmangle(ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ **ਸਿਰਾ)
{
	ਅੱਖਰ *buf;
	ਅਚਲ ਅੱਖਰ *e;
	size_t sz;

	ਜੇ (!s)
		ਵਾਪਸ ਨਿਰਾ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	ਜੇ (ਸਿਰਾ)
		*ਸਿਰਾ = e;
	ਜੇ (e == s)
		ਵਾਪਸ ਨਿਰਾ;	/* empty string */

	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(sz);
	ਜੇ (!buf)
		ਵਾਪਸ ਨਿਰਾ;

	unmangle_ਮੰਜ਼ਿਲ_ਬਫਰ(s, buf, sz);
	ਵਾਪਸ buf;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_MANGLE
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਅੱਖਰ *p = ਨਿਰਾ;
	ਜੇ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਜੇ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		ਲਿਖੋ("mangled: '%s'\n", p);
		ਮੁਕਤ(p);
	}

	ਨਹੀਂਤਾਂ ਜੇ (!strcmp(argv[1], "--unmangle")) {
		ਅੱਖਰ *x = unmangle(argv[2], ਨਿਰਾ);

		ਜੇ (x) {
			ਲਿਖੋ("unmangled: '%s'\n", x);
			ਮੁਕਤ(x);
		}

		x = strdup(argv[2]);
		ਜੇ (x) {
			unmangle_ਮੰਜ਼ਿਲ_ਬਫਰ(x, x, ਲੜੀ_ਲੰਬਾਈ(x) + 1);

			ਲਿਖੋ("self-unmangled: '%s'\n", x);
			ਮੁਕਤ(x);
		}
	}

	ਨਹੀਂਤਾਂ ਜੇ (!strcmp(argv[1], "--unescape")) {
		ਅੱਖਰ *x = strdup(argv[2]);
		ਜੇ (x) {
			unescape_ਮੰਜ਼ਿਲ_ਬਫਰ(x, ",\"", x, ਲੜੀ_ਲੰਬਾਈ(x) + 1);

			ਲਿਖੋ("self-unescaped: '%s'\n", x);
			ਮੁਕਤ(x);
		}
	}

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_MANGLE */
