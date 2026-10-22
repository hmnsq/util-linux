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

#define from_hex(c)		(है_अंक(c) ? c - '0' : छोटे(c) - 'a' + 10)

स्थिर ओळीत पूर्णांक is_unwanted_char(अचिन्हित अक्षर c)
{
	परत c <= 0x20 || c == 0x7f || c == '\\';
}


अक्षर *mangle(स्थिरांक अक्षर *s)
{
	अक्षर *ss, *sp;

	जर (!s)
		परत रिक्त;

	ss = sp = दो_स्मृति(4 * म_माप(s) + 1);
	जर (!sp)
		परत रिक्त;
	पर्यंत(1) {
		जर (!*s) {
			*sp = '\0';
			थांबा;
		}
		जर (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} अन्यथा
			*sp++ = *s;
		s++;
	}
	परत ss;
}


शून्य unmangle_to_buffer(स्थिरांक अक्षर *s, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;

	जर (!s)
		परत;

	पर्यंत(*s && sz < len - 1) {
		जर (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
		    isoctal(s[2]) && isoctal(s[3])) {

			*buf++ = 64*(s[1] & 7) + 8*(s[2] & 7) + (s[3] & 7);
			s += 4;
			sz += 4;
		} अन्यथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
}

माप_प्रकार unhexmangle_to_buffer(स्थिरांक अक्षर *s, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	स्थिरांक अक्षर *buf0 = buf;

	जर (!s)
		परत 0;

	पर्यंत(*s && sz < len - 1) {
		जर (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
		    है_षष्ठादशक(s[2]) && है_षष्ठादशक(s[3])) {

			*buf++ = from_hex(s[2]) << 4 | from_hex(s[3]);
			s += 4;
			sz += 4;
		} अन्यथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	परत buf - buf0 + 1;
}

माप_प्रकार unescape_to_buffer(स्थिरांक अक्षर *s, स्थिरांक अक्षर *wanted, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	स्थिरांक अक्षर *buf0 = buf;

	पर्यंत (*s && sz < len - 1) {
		जर (*s == '\\' && sz + 1 < len - 1 && म_अक्षर(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} अन्यथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	परत buf - buf0 + 1;
}

स्थिर ओळीत स्थिरांक अक्षर *skip_nonspaces(स्थिरांक अक्षर *s)
{
	पर्यंत (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	परत s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
अक्षर *unmangle(स्थिरांक अक्षर *s, स्थिरांक अक्षर **end)
{
	अक्षर *buf;
	स्थिरांक अक्षर *e;
	माप_प्रकार sz;

	जर (!s)
		परत रिक्त;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	जर (end)
		*end = e;
	जर (e == s)
		परत रिक्त;	/* empty string */

	buf = दो_स्मृति(sz);
	जर (!buf)
		परत रिक्त;

	unmangle_to_buffer(s, buf, sz);
	परत buf;
}

#ifdef TEST_PROGRAM_MANGLE
#अन्तर्भाव <errno.h>
पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *p = रिक्त;
	जर (argc < 3) {
		ख_लिखो(मानक_त्रुटि, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		परत निकास_त्रुटि;
	}

	जर (!म_भेद(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		म_लिखो("mangled: '%s'\n", p);
		मुक्त(p);
	}

	अन्यथा जर (!म_भेद(argv[1], "--unmangle")) {
		अक्षर *x = unmangle(argv[2], रिक्त);

		जर (x) {
			म_लिखो("unmangled: '%s'\n", x);
			मुक्त(x);
		}

		x = strdup(argv[2]);
		जर (x) {
			unmangle_to_buffer(x, x, म_माप(x) + 1);

			म_लिखो("self-unmangled: '%s'\n", x);
			मुक्त(x);
		}
	}

	अन्यथा जर (!म_भेद(argv[1], "--unescape")) {
		अक्षर *x = strdup(argv[2]);
		जर (x) {
			unescape_to_buffer(x, ",\"", x, म_माप(x) + 1);

			म_लिखो("self-unescaped: '%s'\n", x);
			मुक्त(x);
		}
	}

	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_MANGLE */
