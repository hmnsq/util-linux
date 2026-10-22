/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include <ctype.h>
#include <unistd.h>

#ifdef HAVE_GETTTYNAM
# include <ttyent.h>
#endif

#include "c.h"
#include "ttyutils.h"

#ifdef __linux__
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "linux"
#  endif
#  གལ་ཏེ defined (__s390__) || defined (__s390x__)
#    define DEFAULT_TTYS0  "dumb"
#    define DEFAULT_TTY32  "ibm327x"
#    define DEFAULT_TTYS1  "vt220"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#elif defined(__GNU__)
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "hurd"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt102"
#  endif
#དེ་མིན
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static ཧྲིལ get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	གལ་ཏེ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		གལ་ཏེ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ཕྱིར x;
	}

	ཕྱིར -1;
}

ཧྲིལ get_terminal_dimension(ཧྲིལ *cols, ཧྲིལ *lines)
{
	ཧྲིལ c = 0, l = 0;

#གལ་ཏེ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	གལ་ཏེ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	གལ་ཏེ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	གལ་ཏེ (cols) {
		གལ་ཏེ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	གལ་ཏེ (lines) {
		གལ་ཏེ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ཕྱིར 0;
}

ཧྲིལ get_terminal_width(ཧྲིལ default_width)
{
	ཧྲིལ width = 0;

	get_terminal_dimension(&width, NULL);

	ཕྱིར width > 0 ? width : default_width;
}

ཧྲིལ get_terminal_stdfd(void)
{
	གལ་ཏེ (isatty(STDIN_FILENO))
		ཕྱིར STDIN_FILENO;
	གལ་ཏེ (isatty(STDOUT_FILENO))
		ཕྱིར STDOUT_FILENO;
	གལ་ཏེ (isatty(STDERR_FILENO))
		ཕྱིར STDERR_FILENO;

	ཕྱིར -EINVAL;
}

ཧྲིལ get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	ཧྲིལ fd;


	གལ་ཏེ (name)
		*name = NULL;
	གལ་ཏེ (path)
		*path = NULL;
	གལ་ཏེ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	གལ་ཏེ (fd < 0)
		ཕྱིར fd;	/* error */

	tty = ttyname(fd);
	གལ་ཏེ (!tty)
		ཕྱིར -1;

	གལ་ཏེ (path)
		*path = tty;
	གལ་ཏེ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	གལ་ཏེ (name)
		*name = tty;
	གལ་ཏེ (number) {
		ཆེད་དུ (p = tty; p && *p; p++) {
			གལ་ཏེ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ཕྱིར 0;
}

ཧྲིལ get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	གལ་ཏེ (*type)
		ཕྱིར -EINVAL;
	ཕྱིར 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, ཧྲིལ is_serial)
{
	གལ་ཏེ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		གལ་ཏེ (ent && ent->ty_type)
			ཕྱིར strdup(ent->ty_type);
#endif

#གལ་ཏེ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		གལ་ཏེ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ཕྱིར strdup(DEFAULT_TTYS0);
		དེ་མིན གལ་ཏེ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ཕྱིར strdup(DEFAULT_TTY32);
		དེ་མིན གལ་ཏེ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ཕྱིར strdup(DEFAULT_TTYS1);
#endif
	}

	ཕྱིར strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ཧྲིལ གཙོ(void)
{
	const var_na *path, *name, *num;
	ཧྲིལ c, l;

	གལ་ཏེ (get_terminal_name(&path, &name, &num) == 0) {
		var_na *term;

		fprintf(stderr, "tty path:      %s\n", path);
		fprintf(stderr, "tty name:      %s\n", name);
		fprintf(stderr, "tty number:    %s\n", num);

		fprintf(stderr, "tty term:      %s\n", getenv("TERM"));

		term = get_terminal_default_type(name, 0);
		fprintf(stderr, "tty dflt term: %s\n", term);
		free(term);
	}
	get_terminal_dimension(&c, &l);
	fprintf(stderr,         "tty cols:      %d\n", c);
	fprintf(stderr,         "tty lines:     %d\n", l);


	ཕྱིར EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
