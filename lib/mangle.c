/*
 * Functions for \oct encoding used in mtab/fstab/swaps/etc.
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2010]
 */
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <ctype.h>

#समावेश "mangle.h"
#समावेश "c.h"

#घोषणा isoctal(a)		(((a) & ~7) == '0')

#घोषणा from_hex(c)		(है_अंक(c) ? c - '0' : छोटे(c) - 'a' + 10)

अटल अंतरभूत पूर्णांक is_unwanted_char(अचिन्हित अक्षर c)
{
	वापस c <= 0x20 || c == 0x7f || c == '\\';
}


अक्षर *mangle(स्थिर अक्षर *s)
{
	अक्षर *ss, *sp;

	अगर (!s)
		वापस शून्य;

	ss = sp = दो_स्मृति(4 * म_माप(s) + 1);
	अगर (!sp)
		वापस शून्य;
	जबतक(1) {
		अगर (!*s) {
			*sp = '\0';
			अवरोध;
		}
		अगर (is_unwanted_char(*s)) {
			*sp++ = '\\';
			*sp++ = '0' + ((*s & 0300) >> 6);
			*sp++ = '0' + ((*s & 070) >> 3);
			*sp++ = '0' + (*s & 07);
		} अन्यथा
			*sp++ = *s;
		s++;
	}
	वापस ss;
}


व्योम unmangle_to_बफर(स्थिर अक्षर *s, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;

	अगर (!s)
		वापस;

	जबतक(*s && sz < len - 1) {
		अगर (*s == '\\' && sz + 3 < len - 1 && isoctal(s[1]) &&
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

माप_प्रकार unhexmangle_to_बफर(स्थिर अक्षर *s, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	स्थिर अक्षर *buf0 = buf;

	अगर (!s)
		वापस 0;

	जबतक(*s && sz < len - 1) {
		अगर (*s == '\\' && sz + 3 < len - 1 && s[1] == 'x' &&
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
	वापस buf - buf0 + 1;
}

माप_प्रकार unescape_to_बफर(स्थिर अक्षर *s, स्थिर अक्षर *wanted, अक्षर *buf, माप_प्रकार len)
{
	माप_प्रकार sz = 0;
	स्थिर अक्षर *buf0 = buf;

	जबतक (*s && sz < len - 1) {
		अगर (*s == '\\' && sz + 1 < len - 1 && म_अक्षर(wanted, s[1])) {
			*buf++ = s[1];
			s += 2;
			sz += 2;
		} अन्यथा {
			*buf++ = *s++;
			sz++;
		}
	}
	*buf = '\0';
	वापस buf - buf0 + 1;
}

अटल अंतरभूत स्थिर अक्षर *skip_nonspaces(स्थिर अक्षर *s)
{
	जबतक (s && *s && !(*s == ' ' || *s == '\t'))
		s++;
	वापस s;
}

/*
 * Returns mallocated buffer or NULL in case of error.
 */
अक्षर *unmangle(स्थिर अक्षर *s, स्थिर अक्षर **end)
{
	अक्षर *buf;
	स्थिर अक्षर *e;
	माप_प्रकार sz;

	अगर (!s)
		वापस शून्य;

	e = skip_nonspaces(s);
	sz = e - s + 1;

	अगर (end)
		*end = e;
	अगर (e == s)
		वापस शून्य;	/* empty string */

	buf = दो_स्मृति(sz);
	अगर (!buf)
		वापस शून्य;

	unmangle_to_बफर(s, buf, sz);
	वापस buf;
}

#अगर_घोषित TEST_PROGRAM_MANGLE
#समावेश <errno.h>
पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	अक्षर *p = शून्य;
	अगर (argc < 3) {
		ख_लिखो(मानक_त्रुटि, "usage: %s --mangle|unmangle|unescape <string>\n",
						program_invocation_short_name);
		वापस निकास_त्रुटि;
	}

	अगर (!म_भेद(argv[1], "--mangle")) {
		p = mangle(argv[2]);
		म_लिखो("mangled: '%s'\n", p);
		मुक्त(p);
	}

	अन्यथा अगर (!म_भेद(argv[1], "--unmangle")) {
		अक्षर *x = unmangle(argv[2], शून्य);

		अगर (x) {
			म_लिखो("unmangled: '%s'\n", x);
			मुक्त(x);
		}

		x = strdup(argv[2]);
		अगर (x) {
			unmangle_to_बफर(x, x, म_माप(x) + 1);

			म_लिखो("self-unmangled: '%s'\n", x);
			मुक्त(x);
		}
	}

	अन्यथा अगर (!म_भेद(argv[1], "--unescape")) {
		अक्षर *x = strdup(argv[2]);
		अगर (x) {
			unescape_to_बफर(x, ",\"", x, म_माप(x) + 1);

			म_लिखो("self-unescaped: '%s'\n", x);
			मुक्त(x);
		}
	}

	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_MANGLE */
