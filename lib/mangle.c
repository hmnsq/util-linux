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

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

ایستا درونخطی صحیح is_unwanted_char(unsigned نویسه c)
{
	بازگشت c <= 0x20 || c == 0x7f || c == '\\';
}


نویسه *mangle(ثابت نویسه *s)
{
	نویسه *ss, *sp;

	اگر (!s)
		بازگشت پوچ;

	ss = sp = malloc(4 * strlen(s) + 1);
	اگر (!sp)
		بازگشت پوچ;
	وقتی(1) {
		اگر (!*s) {
			*sp = '\0';
			بشکن;
		}
		اگر (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} وگرنه
			*sp++ = *s;
		s++;
	}
	بازگشت ss;
}


تهی unmangle_to_buffer(ثابت نویسه *s, نویسه *buf, size_t len)
{
	size_t sz = 0;

	اگر (!s)
		بازگشت;

	وقتی(*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} وگرنه {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ثابت نویسه *s, نویسه *buf, size_t len)
{
	size_t sz = 0;
	ثابت نویسه *buf0 = buf;

	اگر (!s)
		بازگشت 0;

	وقتی(*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} وگرنه {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	بازگشت buf - buf0 + 1;
}

size_t unescape_to_buffer(ثابت نویسه *s, ثابت نویسه *wanted, نویسه *buf, size_t len)
{
	size_t sz = 0;
	ثابت نویسه *buf0 = buf;

	وقتی (*s && sz < len - 1) {
		اگر (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} وگرنه {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	بازگشت buf - buf0 + 1;
}

ایستا درونخطی ثابت نویسه *skip_nonspaces(ثابت نویسه *s)
{
	وقتی (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	بازگشت s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
نویسه *unmangle(ثابت نویسه *s, ثابت نویسه **end)
{
	نویسه *buf;
	ثابت نویسه *e;
	size_t sz;

	اگر (!s)
		بازگشت پوچ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	اگر (end)
		*end = e;
	اگر (e == s)
		بازگشت پوچ;	/* empty string */

	buf = malloc(sz);
	اگر (!buf)
		بازگشت پوچ;

	unmangle_to_buffer(s, buf, sz);
	بازگشت buf;
}

#ifdef TEST_PROGRAM_MANGLE
#شامل <errno.h>
صحیح اصلی(صحیح argc, نویسه *argv[])
{
	نویسه *p = پوچ;
	اگر (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		بازگشت EXIT_FAILURE;
	}

	اگر (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		آزاد(p);
	}

	وگرنه اگر (!strcmp(argv[1], "--unmangle")) {
		نویسه *x = unmangle(argv[2], پوچ);

		اگر (x) {
			printf("unmangled: '%s'\n", x);
			آزاد(x);
		}

		x = strdup(argv[2]);
		اگر (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			آزاد(x);
		}
	}

	وگرنه اگر (!strcmp(argv[1], "--unescape")) {
		نویسه *x = strdup(argv[2]);
		اگر (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			آزاد(x);
		}
	}

	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
