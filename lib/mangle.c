/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <ctype.h>

#شامل "mangle.h"
#شامل "c.h"

#تعریف_کرو isoctal(a)		(((a) & ~7) == '0')

#تعریف_کرو from_hex(c)		(ہندسہ_ہے(c) ? c - '0' : tolower(c) - 'a' + 10)

ساکن اندرونی صحیح is_unwanted_char(غیرعلامتی حرف c)
{
	واپس c <= 0x20 || c == 0x7f || c == '\\';
}


حرف *mangle(ثابت حرف *s)
{
	حرف *ss, *sp;

	اگر (!s)
		واپس تہی;

	ss = sp = حافظہ_دو(4 * لڑی_لمبائی(s) + 1);
	اگر (!sp)
		واپس تہی;
	جب(1) {
		اگر (!*s) {
			*sp = '\0';
			توڑو;
		}
		اگر (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} ورنہ
			*sp++ = *s;
		s++;
	}
	واپس ss;
}


خالی unmangle_مقصد_بفر(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;

	اگر (!s)
		واپس;

	جب(*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} ورنہ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_مقصد_بفر(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	اگر (!s)
		واپس 0;

	جب(*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} ورنہ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	واپس buf - buf0 + 1;
}

size_t unescape_مقصد_بفر(ثابت حرف *s, ثابت حرف *wanted, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	جب (*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} ورنہ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	واپس buf - buf0 + 1;
}

ساکن اندرونی ثابت حرف *skip_nonspaces(ثابت حرف *s)
{
	جب (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	واپس s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
حرف *unmangle(ثابت حرف *s, ثابت حرف **انتہا)
{
	حرف *buf;
	ثابت حرف *e;
	size_t sz;

	اگر (!s)
		واپس تہی;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	اگر (انتہا)
		*انتہا = e;
	اگر (e == s)
		واپس تہی;	/* empty string */

	buf = حافظہ_دو(sz);
	اگر (!buf)
		واپس تہی;

	unmangle_مقصد_بفر(s, buf, sz);
	واپس buf;
}

#اگر_تعریف_شدہ TEST_پروگرام_MANGLE
#شامل <errno.h>
صحیح مین(صحیح argc, حرف *argv[])
{
	حرف *p = تہی;
	اگر (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						پروگرام_invocation_short_name);
		واپس EXIT_FAILURE;
	}

	اگر (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		لکھو("mangled: '%s'\n", p);
		آزاد(p);
	}

	ورنہ اگر (!strcmp(argv[1], "--unmangle")) {
		حرف *x = unmangle(argv[2], تہی);

		اگر (x) {
			لکھو("unmangled: '%s'\n", x);
			آزاد(x);
		}

		x = strdup(argv[2]);
		اگر (x) {
			unmangle_مقصد_بفر(x, x, لڑی_لمبائی(x) + 1);

			لکھو("self-unmangled: '%s'\n", x);
			آزاد(x);
		}
	}

	ورنہ اگر (!strcmp(argv[1], "--unescape")) {
		حرف *x = strdup(argv[2]);
		اگر (x) {
			unescape_مقصد_بفر(x, ",\"", x, لڑی_لمبائی(x) + 1);

			لکھو("self-unescaped: '%s'\n", x);
			آزاد(x);
		}
	}

	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_MANGLE */
