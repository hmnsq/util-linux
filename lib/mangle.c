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

स्थिर अन्तःस्थ पूर्णाङ्क is_unwanted_char(अचिन्हित वर्ण c)
{
	फर्काऊ c <= 0x20 || c == 0x7f || c == '\\';
}


वर्ण *mangle(अचर वर्ण *s)
{
	वर्ण *ss, *sp;

	यदि (!s)
		फर्काऊ रिक्त;

	ss = sp = दो_स्मृति(4 * म_माप(s) + 1);
	यदि (!sp)
		फर्काऊ रिक्त;
	जबसम्म(1) {
		यदि (!*s) {
			*sp = '\0';
			रोक;
		}
		यदि (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} अन्यथा
			*sp++ = *s;
		s++;
	}
	फर्काऊ ss;
}


शून्य unmangle_to_buffer(अचर वर्ण *s, वर्ण *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;

	यदि (!s)
		फर्काऊ;

	जबसम्म(*s && sz < len - 1) {
		यदि (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
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

माप_प्रकार unhexmangle_to_buffer(अचर वर्ण *s, वर्ण *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	अचर वर्ण *buf0 = buf;

	यदि (!s)
		फर्काऊ 0;

	जबसम्म(*s && sz < len - 1) {
		यदि (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
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
	फर्काऊ buf - buf0 + 1;
}

माप_प्रकार unescape_to_buffer(अचर वर्ण *s, अचर वर्ण *wanted, वर्ण *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	अचर वर्ण *buf0 = buf;

	जबसम्म (*s && sz < len - 1) {
		यदि (*s == '\\' && sz + 1 < len - 1 && म_अक्षर(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} अन्यथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	फर्काऊ buf - buf0 + 1;
}

स्थिर अन्तःस्थ अचर वर्ण *skip_nonspaces(अचर वर्ण *s)
{
	जबसम्म (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	फर्काऊ s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
वर्ण *unmangle(अचर वर्ण *s, अचर वर्ण **end)
{
	वर्ण *buf;
	अचर वर्ण *e;
	माप_प्रकार sz;

	यदि (!s)
		फर्काऊ रिक्त;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	यदि (end)
		*end = e;
	यदि (e == s)
		फर्काऊ रिक्त;	/* empty string */

	buf = दो_स्मृति(sz);
	यदि (!buf)
		फर्काऊ रिक्त;

	unmangle_to_buffer(s, buf, sz);
	फर्काऊ buf;
}

#ifdef TEST_PROGRAM_MANGLE
#अन्तर्भाव <errno.h>
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	वर्ण *p = रिक्त;
	यदि (argc < 3) {
		ख_लिखो(मानक_त्रुटि, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		फर्काऊ निकास_त्रुटि;
	}

	यदि (!म_भेद(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		म_लिखो("mangled: '%s'\n", p);
		मुक्त(p);
	}

	अन्यथा यदि (!म_भेद(argv[1], "--unmangle")) {
		वर्ण *x = unmangle(argv[2], रिक्त);

		यदि (x) {
			म_लिखो("unmangled: '%s'\n", x);
			मुक्त(x);
		}

		x = strdup(argv[2]);
		यदि (x) {
			unmangle_to_buffer(x, x, म_माप(x) + 1);

			म_लिखो("self-unmangled: '%s'\n", x);
			मुक्त(x);
		}
	}

	अन्यथा यदि (!म_भेद(argv[1], "--unescape")) {
		वर्ण *x = strdup(argv[2]);
		यदि (x) {
			unescape_to_buffer(x, ",\"", x, म_माप(x) + 1);

			म_लिखो("self-unescaped: '%s'\n", x);
			मुक्त(x);
		}
	}

	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_MANGLE */
