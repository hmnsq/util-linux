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
#  إذا defined (__s390__) || defined (__s390x__)
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

	إذا (cp) {
		حرف *end = عدم;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		إذا (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			أرجع x;
	}

	أرجع -1;
}

صحيح get_terminal_dimension(صحيح *cols, صحيح *هێڵەکان)
{
	صحيح c = 0, l = 0;

#إذا defined(TIOCGWINSZ)
	بنية winsize	w_win;
	إذا (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وإلاإذا defined(TIOCGSIZE)
	بنية ttysize	t_win;
	إذا (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_هێڵەکان;
	}
#endif
	إذا (cols) {
		إذا (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	إذا (هێڵەکان) {
		إذا (l <= 0)
			l = get_env_int("LINES");
		*هێڵەکان = l;
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
	إذا (isatty(STDIN_FILENO))
		أرجع STDIN_FILENO;
	إذا (isatty(STDOUT_FILENO))
		أرجع STDOUT_FILENO;
	إذا (isatty(STDERR_FILENO))
		أرجع STDERR_FILENO;

	أرجع -EINVAL;
}

صحيح get_terminal_name(ثابت حرف **path,
		      ثابت حرف **name,
		      ثابت حرف **ژمار)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحيح fd;


	إذا (name)
		*name = عدم;
	إذا (path)
		*path = عدم;
	إذا (ژمار)
		*ژمار = عدم;

	fd = get_terminal_stdfd();
	إذا (fd < 0)
		أرجع fd;	/* error */

	tty = ttyname(fd);
	إذا (!tty)
		أرجع -1;

	إذا (path)
		*path = tty;
	إذا (name || ژمار)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	إذا (name)
		*name = tty;
	إذا (ژمار) {
		لكل (p = tty; p && *p; p++) {
			إذا (isdigit(*p)) {
				*ژمار = p;
				اكسر;
			}
		}
	}
	أرجع 0;
}

صحيح get_terminal_type(ثابت حرف **type)
{
	*type = getenv("TERM");
	إذا (*type)
		أرجع -EINVAL;
	أرجع 0;
}

حرف *get_terminal_default_type(ثابت حرف *ttyname, صحيح is_serial)
{
	إذا (ttyname) {
#ifdef HAVE_GETTTYNAM
		بنية ttyent *ent = getttynam(ttyname);

		إذا (ent && ent->ty_type)
			أرجع strdup(ent->ty_type);
#endif

#إذا defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		إذا (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			أرجع strdup(DEFAULT_TTYS0);
		وإلا إذا (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			أرجع strdup(DEFAULT_TTY32);
		وإلا إذا (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			أرجع strdup(DEFAULT_TTYS1);
#endif
	}

	أرجع strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
صحيح main(فراغ)
{
	ثابت حرف *path, *name, *ژمارە;
	صحيح c, l;

	إذا (get_terminal_name(&path, &name, &ژمارە) == 0) {
		حرف *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", ژمارە);

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
