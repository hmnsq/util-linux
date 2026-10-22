/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <ctype.h>

#അന്തര്ഭാവ "mangle.h"
#അന്തര്ഭാവ "c.h"

#define isoctal(a)		(((a) & ~7) == '0')

#define from_hex(c)		(isdigit(c) ? c - '0' : tolower(c) - 'a' + 10)

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ is_unwanted_char(unsigned അക്ഷരം c)
{
	തിരികെ c <= 0x20 || c == 0x7f || c == '\\';
}


അക്ഷരം *mangle(സ്ഥിരം അക്ഷരം *s)
{
	അക്ഷരം *ss, *sp;

	എങ്കിൽ (!s)
		തിരികെ ശൂന്യ;

	ss = sp = malloc(4 * strlen(s) + 1);
	എങ്കിൽ (!sp)
		തിരികെ ശൂന്യ;
	ആകുമ്പോൾ(1) {
		എങ്കിൽ (!*s) {
			*sp = '\0';
			നിർത്തുക;
		}
		എങ്കിൽ (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} അല്ലെങ്കിൽ
			*sp++ = *s;
		s++;
	}
	തിരികെ ss;
}


ശൂന്യം unmangle_to_buffer(സ്ഥിരം അക്ഷരം *s, അക്ഷരം *buf, size_t len)
{
	size_t sz = 0;

	എങ്കിൽ (!s)
		തിരികെ;

	ആകുമ്പോൾ(*s && sz < len - 1) {
		എങ്കിൽ (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} അല്ലെങ്കിൽ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

size_t unhexmangle_to_buffer(സ്ഥിരം അക്ഷരം *s, അക്ഷരം *buf, size_t len)
{
	size_t sz = 0;
	സ്ഥിരം അക്ഷരം *buf0 = buf;

	എങ്കിൽ (!s)
		തിരികെ 0;

	ആകുമ്പോൾ(*s && sz < len - 1) {
		എങ്കിൽ (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    isxdigit(s[2]) && isxdigit(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} അല്ലെങ്കിൽ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	തിരികെ buf - buf0 + 1;
}

size_t unescape_to_buffer(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *wanted, അക്ഷരം *buf, size_t len)
{
	size_t sz = 0;
	സ്ഥിരം അക്ഷരം *buf0 = buf;

	ആകുമ്പോൾ (*s && sz < len - 1) {
		എങ്കിൽ (*s == '\\' && sz + 1 < len - 1 && strchr(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} അല്ലെങ്കിൽ {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	തിരികെ buf - buf0 + 1;
}

സ്ഥിതം വരിയിൽ സ്ഥിരം അക്ഷരം *skip_nonspaces(സ്ഥിരം അക്ഷരം *s)
{
	ആകുമ്പോൾ (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	തിരികെ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
അക്ഷരം *unmangle(സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം **end)
{
	അക്ഷരം *buf;
	സ്ഥിരം അക്ഷരം *e;
	size_t sz;

	എങ്കിൽ (!s)
		തിരികെ ശൂന്യ;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	എങ്കിൽ (end)
		*end = e;
	എങ്കിൽ (e == s)
		തിരികെ ശൂന്യ;	/* empty string */

	buf = malloc(sz);
	എങ്കിൽ (!buf)
		തിരികെ ശൂന്യ;

	unmangle_to_buffer(s, buf, sz);
	തിരികെ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#അന്തര്ഭാവ <errno.h>
പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	അക്ഷരം *p = ശൂന്യ;
	എങ്കിൽ (argc < 3) {
		fprintf(stderr, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		തിരികെ EXIT_FAILURE;
	}

	എങ്കിൽ (!strcmp(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		printf("mangled: '%s'\n", p);
		സ്വതന്ത്രം(p);
	}

	അല്ലെങ്കിൽ എങ്കിൽ (!strcmp(argv[1], "--unmangle")) {
		അക്ഷരം *x = unmangle(argv[2], ശൂന്യ);

		എങ്കിൽ (x) {
			printf("unmangled: '%s'\n", x);
			സ്വതന്ത്രം(x);
		}

		x = strdup(argv[2]);
		എങ്കിൽ (x) {
			unmangle_to_buffer(x, x, strlen(x) + 1);

			printf("self-unmangled: '%s'\n", x);
			സ്വതന്ത്രം(x);
		}
	}

	അല്ലെങ്കിൽ എങ്കിൽ (!strcmp(argv[1], "--unescape")) {
		അക്ഷരം *x = strdup(argv[2]);
		എങ്കിൽ (x) {
			unescape_to_buffer(x, ",\"", x, strlen(x) + 1);

			printf("self-unescaped: '%s'\n", x);
			സ്വതന്ത്രം(x);
		}
	}

	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_MANGLE */
