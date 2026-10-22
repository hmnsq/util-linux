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
#  जइ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#नोचेद्यदि defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#अण्णहा
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

स्थिर पूर्णाङ्क get_env_int(ध्रुव वर्ण *name)
{
	ध्रुव वर्ण *cp = getenv(name);

	जइ (cp) {
		वर्ण *end = रिक्त;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		जइ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			प्रत्यागम x;
	}

	प्रत्यागम -1;
}

पूर्णाङ्क get_terminal_dimension(पूर्णाङ्क *cols, पूर्णाङ्क *पङ्क्तयः)
{
	पूर्णाङ्क c = 0, l = 0;

#जइ defined(TIOCGWINSZ)
	संरचना winsize	w_win;
	जइ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#नोचेद्यदि defined(TIOCGSIZE)
	संरचना ttysize	t_win;
	जइ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_पङ्क्तयः;
	}
#endif
	जइ (cols) {
		जइ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	जइ (पङ्क्तयः) {
		जइ (l <= 0)
			l = get_env_int("LINES");
		*पङ्क्तयः = l;
	}
	प्रत्यागम 0;
}

पूर्णाङ्क get_terminal_width(पूर्णाङ्क default_width)
{
	पूर्णाङ्क width = 0;

	get_terminal_dimension(&width, रिक्त);

	प्रत्यागम width > 0 ? width : default_width;
}

पूर्णाङ्क get_terminal_stdfd(शून्य)
{
	जइ (isatty(STDIN_FILENO))
		प्रत्यागम STDIN_FILENO;
	जइ (isatty(STDOUT_FILENO))
		प्रत्यागम STDOUT_FILENO;
	जइ (isatty(STDERR_FILENO))
		प्रत्यागम STDERR_FILENO;

	प्रत्यागम -EINVAL;
}

पूर्णाङ्क get_terminal_name(ध्रुव वर्ण **path,
		      ध्रुव वर्ण **name,
		      ध्रुव वर्ण **सङ्ख्या)
{
	ध्रुव वर्ण *tty;
	ध्रुव वर्ण *p;
	पूर्णाङ्क fd;


	जइ (name)
		*name = रिक्त;
	जइ (path)
		*path = रिक्त;
	जइ (सङ्ख्या)
		*सङ्ख्या = रिक्त;

	fd = get_terminal_stdfd();
	जइ (fd < 0)
		प्रत्यागम fd;	/* error */

	tty = ttyname(fd);
	जइ (!tty)
		प्रत्यागम -1;

	जइ (path)
		*path = tty;
	जइ (name || सङ्ख्या)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	जइ (name)
		*name = tty;
	जइ (सङ्ख्या) {
		प्रति (p = tty; p && *p; p++) {
			जइ (isdigit(*p)) {
				*सङ्ख्या = p;
				भङ्ग;
			}
		}
	}
	प्रत्यागम 0;
}

पूर्णाङ्क get_terminal_type(ध्रुव वर्ण **type)
{
	*type = getenv("TERM");
	जइ (*type)
		प्रत्यागम -EINVAL;
	प्रत्यागम 0;
}

वर्ण *get_terminal_default_type(ध्रुव वर्ण *ttyname, पूर्णाङ्क is_serial)
{
	जइ (ttyname) {
#ifdef HAVE_GETTTYNAM
		संरचना ttyent *ent = getttynam(ttyname);

		जइ (ent && ent->ty_type)
			प्रत्यागम strdup(ent->ty_type);
#endif

#जइ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		जइ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			प्रत्यागम strdup(DEFAULT_TTYS0);
		अण्णहा जइ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			प्रत्यागम strdup(DEFAULT_TTY32);
		अण्णहा जइ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			प्रत्यागम strdup(DEFAULT_TTYS1);
#endif
	}

	प्रत्यागम strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
पूर्णाङ्क main(शून्य)
{
	ध्रुव वर्ण *path, *name, *अङ्कः;
	पूर्णाङ्क c, l;

	जइ (get_terminal_name(&path, &name, &अङ्कः) == 0) {
		वर्ण *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", अङ्कः);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		मुक्त(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	प्रत्यागम EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
