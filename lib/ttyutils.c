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
#  यदि defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#अथवायदि defined(__GNU__)
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

स्थिर पूर्णाङ्क get_env_int(अचर वर्ण *name)
{
	अचर वर्ण *cp = दो_पर्या(name);

	यदि (cp) {
		वर्ण *end = रिक्त;
		दीर्घ x;

		त्रुटि_सं = 0;
		x = म_से_दीर्घ(cp, &end, 10);

		यदि (त्रुटि_सं == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= पूर्णांक_उच्च)
			फर्काऊ x;
	}

	फर्काऊ -1;
}

पूर्णाङ्क get_terminal_dimension(पूर्णाङ्क *cols, पूर्णाङ्क *पङ्क्तिहरू)
{
	पूर्णाङ्क c = 0, l = 0;

#यदि defined(TIOCGWINSZ)
	संरचना winsize	w_win;
	यदि (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#अथवायदि defined(TIOCGSIZE)
	संरचना ttysize	t_win;
	यदि (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_पङ्क्तिहरू;
	}
#endif
	यदि (cols) {
		यदि (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	यदि (पङ्क्तिहरू) {
		यदि (l <= 0)
			l = get_env_int("LINES");
		*पङ्क्तिहरू = l;
	}
	फर्काऊ 0;
}

पूर्णाङ्क get_terminal_width(पूर्णाङ्क default_width)
{
	पूर्णाङ्क width = 0;

	get_terminal_dimension(&width, रिक्त);

	फर्काऊ width > 0 ? width : default_width;
}

पूर्णाङ्क get_terminal_stdfd(शून्य)
{
	यदि (isatty(STDIN_FILENO))
		फर्काऊ STDIN_FILENO;
	यदि (isatty(STDOUT_FILENO))
		फर्काऊ STDOUT_FILENO;
	यदि (isatty(STDERR_FILENO))
		फर्काऊ STDERR_FILENO;

	फर्काऊ -EINVAL;
}

पूर्णाङ्क get_terminal_name(अचर वर्ण **path,
		      अचर वर्ण **name,
		      अचर वर्ण **सङ्ख्या)
{
	अचर वर्ण *tty;
	अचर वर्ण *p;
	पूर्णाङ्क fd;


	यदि (name)
		*name = रिक्त;
	यदि (path)
		*path = रिक्त;
	यदि (सङ्ख्या)
		*सङ्ख्या = रिक्त;

	fd = get_terminal_stdfd();
	यदि (fd < 0)
		फर्काऊ fd;	/* error */

	tty = ttyname(fd);
	यदि (!tty)
		फर्काऊ -1;

	यदि (path)
		*path = tty;
	यदि (name || सङ्ख्या)
		tty = म_भेदन(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	यदि (name)
		*name = tty;
	यदि (सङ्ख्या) {
		लागि (p = tty; p && *p; p++) {
			यदि (है_अंक(*p)) {
				*सङ्ख्या = p;
				रोक;
			}
		}
	}
	फर्काऊ 0;
}

पूर्णाङ्क get_terminal_type(अचर वर्ण **type)
{
	*type = दो_पर्या("TERM");
	यदि (*type)
		फर्काऊ -EINVAL;
	फर्काऊ 0;
}

वर्ण *get_terminal_default_type(अचर वर्ण *ttyname, पूर्णाङ्क is_serial)
{
	यदि (ttyname) {
#ifdef HAVE_GETTTYNAM
		संरचना ttyent *ent = getttynam(ttyname);

		यदि (ent && ent->ty_type)
			फर्काऊ strdup(ent->ty_type);
#endif

#यदि defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		यदि (म_भेद(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			फर्काऊ strdup(DEFAULT_TTYS0);
		अन्यथा यदि (म_भेदन(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			फर्काऊ strdup(DEFAULT_TTY32);
		अन्यथा यदि (म_भेद(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			फर्काऊ strdup(DEFAULT_TTYS1);
#endif
	}

	फर्काऊ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
पूर्णाङ्क main(शून्य)
{
	अचर वर्ण *path, *name, *अङ्क;
	पूर्णाङ्क c, l;

	यदि (get_terminal_name(&path, &name, &अङ्क) == 0) {
		वर्ण *term;

		ख_लिखो(मानक_त्रुटि, "tty path:      %s\n", path);
		ख_लिखो(मानक_त्रुटि, "tty name:      %s\n", name);
		ख_लिखो(मानक_त्रुटि, "tty number:    %s\n", अङ्क);

		ख_लिखो(मानक_त्रुटि, "tty term:      %s\n", दो_पर्या("TERM"));

		term = get_terminal_default_type(name, 0);
		ख_लिखो(मानक_त्रुटि, "tty dflt term: %s\n", term);
		मुक्त(term);
	}
	get_terminal_dimension(&c, &l);
	ख_लिखो(मानक_त्रुटि,         "tty cols:      %d\n", c);
	ख_लिखो(मानक_त्रुटि,         "tty lines:     %d\n", l);


	फर्काऊ निकास_सफल;
}
#endif /* TEST_PROGRAM_TTYUTILS */
