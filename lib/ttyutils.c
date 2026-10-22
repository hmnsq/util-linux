/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <ctype.h>
#ضمن <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#ضمن "c.h"
#ضمن "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  𐡄𐡍 defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وإلاإذا defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#وإلا
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

ساكن صحيح get_env_int(ثابت حرف *name)
{
	ثابت حرف *cp = getenv(name);

	𐡄𐡍 (cp) {
		حرف *end = عدم;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		𐡄𐡍 (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			أرجع x;
	}

	أرجع -1;
}

صحيح get_terminal_dimension(صحيح *cols, صحيح *lines)
{
	صحيح c = 0, l = 0;

#𐡄𐡍 defined(TIOCGWINSZ)
	بنية winsize	w_win;
	𐡄𐡍 (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وإلاإذا defined(TIOCGSIZE)
	بنية ttysize	t_win;
	𐡄𐡍 (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	𐡄𐡍 (cols) {
		𐡄𐡍 (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	𐡄𐡍 (lines) {
		𐡄𐡍 (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	أرجع 0;
}

صحيح get_terminal_width(صحيح default_width)
{
	صحيح width = 0;

	get_terminal_dimension(&width, عدم);

	أرجع width > 0 ? width : default_width;
}

صحيح get_terminal_stdfd(فراغ)
{
	𐡄𐡍 (isatty(STDIN_FILENO))
		أرجع STDIN_FILENO;
	𐡄𐡍 (isatty(STDOUT_FILENO))
		أرجع STDOUT_FILENO;
	𐡄𐡍 (isatty(STDERR_FILENO))
		أرجع STDERR_FILENO;

	أرجع -EINVAL;
}

صحيح get_terminal_name(ثابت حرف **path,
		      ثابت حرف **name,
		      ثابت حرف **number)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحيح fd;


	𐡄𐡍 (name)
		*name = عدم;
	𐡄𐡍 (path)
		*path = عدم;
	𐡄𐡍 (number)
		*number = عدم;

	fd = get_terminal_stdfd();
	𐡄𐡍 (fd < 0)
		أرجع fd;	/* error */

	tty = ttyname(fd);
	𐡄𐡍 (!tty)
		أرجع -1;

	𐡄𐡍 (path)
		*path = tty;
	𐡄𐡍 (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	𐡄𐡍 (name)
		*name = tty;
	𐡄𐡍 (number) {
		لكل (p = tty; p && *p; p++) {
			𐡄𐡍 (isdigit(*p)) {
				*number = p;
				اكسر;
			}
		}
	}
	أرجع 0;
}

صحيح get_terminal_type(ثابت حرف **type)
{
	*type = getenv("TERM");
	𐡄𐡍 (*type)
		أرجع -EINVAL;
	أرجع 0;
}

حرف *get_terminal_default_type(ثابت حرف *ttyname, صحيح is_serial)
{
	𐡄𐡍 (ttyname) {
#ifdef HAVE_GETTTYNAM
		بنية ttyent *ent = getttynam(ttyname);

		𐡄𐡍 (ent && ent->ty_type)
			أرجع strdup(ent->ty_type);
#endif

#𐡄𐡍 defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		𐡄𐡍 (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			أرجع strdup(DEFAULT_TTYS0);
		وإلا 𐡄𐡍 (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			أرجع strdup(DEFAULT_TTY32);
		وإلا 𐡄𐡍 (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			أرجع strdup(DEFAULT_TTYS1);
#endif
	}

	أرجع strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
صحيح main(فراغ)
{
	ثابت حرف *path, *name, *num;
	صحيح c, l;

	𐡄𐡍 (get_terminal_name(&path, &name, &num) == 0) {
		حرف *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", num);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		حرر(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	أرجع EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
