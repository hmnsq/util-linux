/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  जर defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#किंवाजर defined(__GNU__)
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
#endif

स्थिर पूर्णांक get_env_int(स्थिरांक अक्षर *name)
{
	स्थिरांक अक्षर *cp = दो_पर्या(name);

	जर (cp) {
		अक्षर *end = रिक्त;
		दीर्घ x;

		त्रुटि_सं = 0;
		x = म_से_दीर्घ(cp, &end, 10);

		जर (त्रुटि_सं == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= पूर्णांक_उच्च)
			परत x;
	}

	परत -1;
}

पूर्णांक get_terminal_dimension(पूर्णांक *cols, पूर्णांक *ओळी)
{
	पूर्णांक c = 0, l = 0;

#जर defined(TIOCGWINSZ)
	संरचना winsize	w_win;
	जर (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#किंवाजर defined(TIOCGSIZE)
	संरचना ttysize	t_win;
	जर (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_ओळी;
	}
#endif
	जर (cols) {
		जर (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	जर (ओळी) {
		जर (l <= 0)
			l = get_env_int("LINES");
		*ओळी = l;
	}
	परत 0;
}

पूर्णांक get_terminal_width(पूर्णांक default_width)
{
	पूर्णांक width = 0;

	get_terminal_dimension(&width, रिक्त);

	परत width > 0 ? width : default_width;
}

पूर्णांक get_terminal_stdfd(शून्य)
{
	जर (isatty(STDIN_FILENO))
		परत STDIN_FILENO;
	जर (isatty(STDOUT_FILENO))
		परत STDOUT_FILENO;
	जर (isatty(STDERR_FILENO))
		परत STDERR_FILENO;

	परत -EINVAL;
}

पूर्णांक get_terminal_name(स्थिरांक अक्षर **path,
		      स्थिरांक अक्षर **name,
		      स्थिरांक अक्षर **संख्या)
{
	स्थिरांक अक्षर *tty;
	स्थिरांक अक्षर *p;
	पूर्णांक fd;


	जर (name)
		*name = रिक्त;
	जर (path)
		*path = रिक्त;
	जर (संख्या)
		*संख्या = रिक्त;

	fd = get_terminal_stdfd();
	जर (fd < 0)
		परत fd;	/* error */

	tty = ttyname(fd);
	जर (!tty)
		परत -1;

	जर (path)
		*path = tty;
	जर (name || संख्या)
		tty = म_भेदन(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	जर (name)
		*name = tty;
	जर (संख्या) {
		साठी (p = tty; p && *p; p++) {
			जर (है_अंक(*p)) {
				*संख्या = p;
				थांबा;
			}
		}
	}
	परत 0;
}

पूर्णांक get_terminal_type(स्थिरांक अक्षर **type)
{
	*type = दो_पर्या("TERM");
	जर (*type)
		परत -EINVAL;
	परत 0;
}

अक्षर *get_terminal_default_type(स्थिरांक अक्षर *ttyname, पूर्णांक is_serial)
{
	जर (ttyname) {
#ifdef HAVE_GETTTYNAM
		संरचना ttyent *ent = getttynam(ttyname);

		जर (ent && ent->ty_type)
			परत strdup(ent->ty_type);
#endif

#जर defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		जर (म_भेद(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			परत strdup(DEFAULT_TTYS0);
		अन्यथा जर (म_भेदन(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			परत strdup(DEFAULT_TTY32);
		अन्यथा जर (म_भेद(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			परत strdup(DEFAULT_TTYS1);
#endif
	}

	परत strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
पूर्णांक main(शून्य)
{
	स्थिरांक अक्षर *path, *name, *अंक;
	पूर्णांक c, l;

	जर (get_terminal_name(&path, &name, &अंक) == 0) {
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


	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_TTYUTILS */
