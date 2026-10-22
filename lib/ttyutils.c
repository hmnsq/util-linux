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
#  ܐܢ defined (__s390__) || defined (__s390x__)
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
#ܐܚܪܢܐ
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

	ܐܢ (cp) {
		حرف *end = عدم;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ܐܢ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ܦܢܐ x;
	}

	ܦܢܐ -1;
}

صحيح get_terminal_dimension(صحيح *cols, صحيح *lines)
{
	صحيح c = 0, l = 0;

#ܐܢ defined(TIOCGWINSZ)
	بنية winsize	w_win;
	ܐܢ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#وإلاإذا defined(TIOCGSIZE)
	بنية ttysize	t_win;
	ܐܢ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ܐܢ (cols) {
		ܐܢ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ܐܢ (lines) {
		ܐܢ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ܦܢܐ 0;
}

صحيح get_terminal_width(صحيح default_width)
{
	صحيح width = 0;

	get_terminal_dimension(&width, عدم);

	ܦܢܐ width > 0 ? width : default_width;
}

صحيح get_terminal_stdfd(فراغ)
{
	ܐܢ (isatty(STDIN_FILENO))
		ܦܢܐ STDIN_FILENO;
	ܐܢ (isatty(STDOUT_FILENO))
		ܦܢܐ STDOUT_FILENO;
	ܐܢ (isatty(STDERR_FILENO))
		ܦܢܐ STDERR_FILENO;

	ܦܢܐ -EINVAL;
}

صحيح get_terminal_name(ثابت حرف **path,
		      ثابت حرف **name,
		      ثابت حرف **number)
{
	ثابت حرف *tty;
	ثابت حرف *p;
	صحيح fd;


	ܐܢ (name)
		*name = عدم;
	ܐܢ (path)
		*path = عدم;
	ܐܢ (number)
		*number = عدم;

	fd = get_terminal_stdfd();
	ܐܢ (fd < 0)
		ܦܢܐ fd;	/* error */

	tty = ttyname(fd);
	ܐܢ (!tty)
		ܦܢܐ -1;

	ܐܢ (path)
		*path = tty;
	ܐܢ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ܐܢ (name)
		*name = tty;
	ܐܢ (number) {
		لكل (p = tty; p && *p; p++) {
			ܐܢ (isdigit(*p)) {
				*number = p;
				اكسر;
			}
		}
	}
	ܦܢܐ 0;
}

صحيح get_terminal_type(ثابت حرف **type)
{
	*type = getenv("TERM");
	ܐܢ (*type)
		ܦܢܐ -EINVAL;
	ܦܢܐ 0;
}

حرف *get_terminal_default_type(ثابت حرف *ttyname, صحيح is_serial)
{
	ܐܢ (ttyname) {
#ifdef HAVE_GETTTYNAM
		بنية ttyent *ent = getttynam(ttyname);

		ܐܢ (ent && ent->ty_type)
			ܦܢܐ strdup(ent->ty_type);
#endif

#ܐܢ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ܐܢ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ܦܢܐ strdup(DEFAULT_TTYS0);
		ܐܚܪܢܐ ܐܢ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ܦܢܐ strdup(DEFAULT_TTY32);
		ܐܚܪܢܐ ܐܢ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ܦܢܐ strdup(DEFAULT_TTYS1);
#endif
	}

	ܦܢܐ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
صحيح main(فراغ)
{
	ثابت حرف *path, *name, *num;
	صحيح c, l;

	ܐܢ (get_terminal_name(&path, &name, &num) == 0) {
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


	ܦܢܐ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
