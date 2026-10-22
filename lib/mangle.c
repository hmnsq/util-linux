/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <ctype.h>

#अन्तर्भाव "mangle.h"
#अन्तर्भाव "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

स्थिर अन्तःस्थ पूर्णाङ्क is_unwanted_char(unsigned वर्ण c)
{
	पटिनिवत्त c <= 0x20 || c == 0x7f || c == '\\';
}


वर्ण *mangle(ध्रुव वर्ण *s)
{
	वर्ण *ss, *sp;

	सचे (!s)
		पटिनिवत्त रिक्त;

	ss = sp = malloc(4 * strlen(s) + 1);
	सचे (!sp)
		पटिनिवत्त रिक्त;
	याव(1) {
		सचे (!*s) {
			*sp = '\0';
			भङ्ग;
		}
		सचे (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} अञ्ञथा
			*sp++ = *s;
		s++;
	}
	पटिनिवत्त ss;
}


शून्य unmangle_to_buffer(ध्रुव वर्ण *s, वर्ण *buf, size_t len)
{
	size_t sz = 0;

	सचे (!s)
		पटिनिवत्त;

	याव(*s && sz < len - 1) {
		सचे (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} अञ्ञथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(ध्रुव वर्ण *s, वर्ण *buf, size_t len)
{
	size_t sz = 0;
	ध्रुव वर्ण *buf0 = buf;

	सचे (!s)
		पटिनिवत्त 0;

	याव(*s && sz < len - 1) {
		सचे (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} अञ्ञथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	पटिनिवत्त buf - buf0 + 1;
}

size_t unescape_to_buffer(ध्रुव वर्ण *s, ध्रुव वर्ण *wanted, वर्ण *buf, size_t len)
{
	size_t sz = 0;
	ध्रुव वर्ण *buf0 = buf;

	याव (*s && sz < len - 1) {
		सचे (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} अञ्ञथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	पटिनिवत्त buf - buf0 + 1;
}

स्थिर अन्तःस्थ ध्रुव वर्ण *skip_nonspaces(ध्रुव वर्ण *s)
{
	याव (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	पटिनिवत्त s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
वर्ण *unmangle(ध्रुव वर्ण *s, ध्रुव वर्ण **end)
{
	वर्ण *buf;
	ध्रुव वर्ण *e;
	size_t sz;

	सचे (!s)
		पटिनिवत्त रिक्त;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	सचे (end)
		*end = e;
	सचे (e == s)
		पटिनिवत्त रिक्त;	/* empty string */

	buf = malloc(sz);
	सचे (!buf)
		पटिनिवत्त रिक्त;

	unmangle_to_buffer(s, buf, sz);
	पटिनिवत्त buf;
}

#ifdef TEST_PROGRAM_MANGLE
#अन्तर्भाव <errno.h>
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *p = रिक्त;
	सचे (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		पटिनिवत्त EXIT_FAILURE;
	}

	सचे (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		मुक्त(p);
	}

	अञ्ञथा सचे (!strcmp(argv[1], "--unmangle")) {
		वर्ण *x = unmangle(argv[2], रिक्त);

		सचे (x) {
			printf("unmangled: '%s'\n", x);
			मुक्त(x);
		}

		x = strdup(argv[2]);
		सचे (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			मुक्त(x);
		}
	}

	अञ्ञथा सचे (!strcmp(argv[1], "--unescape")) {
		वर्ण *x = strdup(argv[2]);
		सचे (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			मुक्त(x);
		}
	}

	पटिनिवत्त EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
