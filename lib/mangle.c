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

#تعریف_کن isoctal(a)		(((a) & ~7) == '0')

#تعریف_کن from_hex(c)		(آیا_رقم(c) ? c - '0' : tolower(c) - 'a' + 10)

ساكن مضمن صحيح is_unwanted_char(بیعلامت حرف c)
{
	أرجع c <= 0x20 || c == 0x7f || c == '\\';
}


حرف *mangle(ثابت حرف *s)
{
	حرف *ss, *sp;

	إذا (!s)
		أرجع عدم;

	ss = sp = حافظه_بده(4 * طول_رشته(s) + 1);
	إذا (!sp)
		أرجع عدم;
	طالما(1) {
		إذا (!*s) {
			*sp = '\0';
			اكسر;
		}
		إذا (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} وإلا
			*sp++ = *s;
		s++;
	}
	أرجع ss;
}


فراغ unmangle_مقصد_میانگیر(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;

	إذا (!s)
		أرجع;

	طالما(*s && sz < len - 1) {
		إذا (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} وإلا {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_مقصد_میانگیر(ثابت حرف *s, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	إذا (!s)
		أرجع 0;

	طالما(*s && sz < len - 1) {
		إذا (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} وإلا {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	أرجع buf - buf0 + 1;
}

size_t unescape_مقصد_میانگیر(ثابت حرف *s, ثابت حرف *wanted, حرف *buf, size_t len)
{
	size_t sz = 0;
	ثابت حرف *buf0 = buf;

	طالما (*s && sz < len - 1) {
		إذا (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} وإلا {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	أرجع buf - buf0 + 1;
}

ساكن مضمن ثابت حرف *skip_nonspaces(ثابت حرف *s)
{
	طالما (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	أرجع s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
حرف *unmangle(ثابت حرف *s, ثابت حرف **انتها)
{
	حرف *buf;
	ثابت حرف *e;
	size_t sz;

	إذا (!s)
		أرجع عدم;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	إذا (انتها)
		*انتها = e;
	إذا (e == s)
		أرجع عدم;	/* empty string */

	buf = حافظه_بده(sz);
	إذا (!buf)
		أرجع عدم;

	unmangle_مقصد_میانگیر(s, buf, sz);
	أرجع buf;
}

#اگر_تعریف_شده TEST_برنامه_MANGLE
#ضمن <errno.h>
صحيح اصلی(صحيح argc, حرف *argv[])
{
	حرف *p = عدم;
	إذا (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						برنامه_invocation_short_name);
		أرجع EXIT_FAILURE;
	}

	إذا (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		چاپ_کن("mangled: '%s'\n", p);
		حرر(p);
	}

	وإلا إذا (!strcmp(argv[1], "--unmangle")) {
		حرف *x = unmangle(argv[2], عدم);

		إذا (x) {
			چاپ_کن("unmangled: '%s'\n", x);
			حرر(x);
		}

		x = strdup(argv[2]);
		إذا (x) {
			unmangle_مقصد_میانگیر(x, x, طول_رشته(x) + 1);

			چاپ_کن("self-unmangled: '%s'\n", x);
			حرر(x);
		}
	}

	وإلا إذا (!strcmp(argv[1], "--unescape")) {
		حرف *x = strdup(argv[2]);
		إذا (x) {
			unescape_مقصد_میانگیر(x, ",\"", x, طول_رشته(x) + 1);

			چاپ_کن("self-unescaped: '%s'\n", x);
			حرر(x);
		}
	}

	أرجع EXIT_SUCCESS;
}
#پایان_اگر /* TEST_PROGRAM_MANGLE */
