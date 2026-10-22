/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश <ctype.h>
#समावेश <unistd.h>

#अगर_घोषित HAVE_GETTTYNAM
# include <ttyent.h>
#पूर्ण_अगर

#समावेश "c.h"
#समावेश "ttyutils.h"

#अगर_घोषित __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  अगर defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#या_अगर defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#अन्यथा
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#पूर्ण_अगर

अटल पूर्णांक get_env_int(स्थिर अक्षर *name)
{
	स्थिर अक्षर *cp = दो_पर्या(name);

	अगर (cp) {
		अक्षर *end = शून्य;
		दीर्घ x;

		त्रुटि_सं = 0;
		x = म_से_दीर्घ(cp, &end, 10);

		अगर (त्रुटि_सं == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= पूर्णांक_उच्च)
			वापस x;
	}

	वापस -1;
}

पूर्णांक get_terminal_dimension(पूर्णांक *cols, पूर्णांक *पंक्तियाँ)
{
	पूर्णांक c = 0, l = 0;

#अगर defined(TIOCGWINSZ)
	काष्ठा winsize	w_win;
	अगर (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#या_अगर defined(TIOCGSIZE)
	काष्ठा ttysize	t_win;
	अगर (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_पंक्तियाँ;
	}
#पूर्ण_अगर
	अगर (cols) {
		अगर (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	अगर (पंक्तियाँ) {
		अगर (l <= 0)
			l = get_env_int("LINES");
		*पंक्तियाँ = l;
	}
	वापस 0;
}

पूर्णांक get_terminal_width(पूर्णांक default_width)
{
	पूर्णांक width = 0;

	get_terminal_dimension(&width, शून्य);

	वापस width > 0 ? width : default_width;
}

पूर्णांक get_terminal_stdfd(व्योम)
{
	अगर (isatty(STDIN_FILENO))
		वापस STDIN_FILENO;
	अगर (isatty(STDOUT_FILENO))
		वापस STDOUT_FILENO;
	अगर (isatty(STDERR_FILENO))
		वापस STDERR_FILENO;

	वापस -EINVAL;
}

पूर्णांक get_terminal_name(स्थिर अक्षर **path,
		      स्थिर अक्षर **name,
		      स्थिर अक्षर **संख्या)
{
	स्थिर अक्षर *tty;
	स्थिर अक्षर *p;
	पूर्णांक fd;


	अगर (name)
		*name = शून्य;
	अगर (path)
		*path = शून्य;
	अगर (संख्या)
		*संख्या = शून्य;

	fd = get_terminal_stdfd();
	अगर (fd < 0)
		वापस fd;	/* error */

	tty = ttyname(fd);
	अगर (!tty)
		वापस -1;

	अगर (path)
		*path = tty;
	अगर (name || संख्या)
		tty = म_भेदन(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	अगर (name)
		*name = tty;
	अगर (संख्या) {
		क्रम (p = tty; p && *p; p++) {
			अगर (है_अंक(*p)) {
				*संख्या = p;
				अवरोध;
			}
		}
	}
	वापस 0;
}

पूर्णांक get_terminal_type(स्थिर अक्षर **type)
{
	*type = दो_पर्या("TERM");
	अगर (*type)
		वापस -EINVAL;
	वापस 0;
}

अक्षर *get_terminal_default_type(स्थिर अक्षर *ttyname, पूर्णांक is_serial)
{
	अगर (ttyname) {
#अगर_घोषित HAVE_GETTTYNAM
		काष्ठा ttyent *ent = getttynam(ttyname);

		अगर (ent && ent->ty_type)
			वापस strdup(ent->ty_type);
#पूर्ण_अगर

#अगर defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		अगर (म_भेद(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			वापस strdup(DEFAULT_TTYS0);
		अन्यथा अगर (म_भेदन(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			वापस strdup(DEFAULT_TTY32);
		अन्यथा अगर (म_भेद(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			वापस strdup(DEFAULT_TTYS1);
#पूर्ण_अगर
	}

	वापस strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#अगर_घोषित TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
पूर्णांक मुख्य(व्योम)
{
	स्थिर अक्षर *path, *name, *अंक;
	पूर्णांक c, l;

	अगर (get_terminal_name(&path, &name, &अंक) == 0) {
		अक्षर *term;

		ख_लिखो(मानक_त्रुटि, "tty path:      %s\n", path);
		ख_लिखो(मानक_त्रुटि, "tty name:      %s\n", name);
		ख_लिखो(मानक_त्रुटि, "tty number:    %s\n", अंक);

		ख_लिखो(मानक_त्रुटि, "tty term:      %s\n", दो_पर्या("TERM"));

		term = get_terminal_default_type(name, 0);
		ख_लिखो(मानक_त्रुटि, "tty dflt term: %s\n", term);
		मुक्त(term);
	}
	get_terminal_dimension(&c, &l);
	ख_लिखो(मानक_त्रुटि,         "tty cols:      %d\n", c);
	ख_लिखो(मानक_त्रुटि,         "tty lines:     %d\n", l);


	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_TTYUTILS */
