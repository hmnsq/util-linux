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
#  གལ་སྲིད defined (__s390__) || defined (__s390x__)
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

	གལ་སྲིད (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		གལ་སྲིད (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ལོག x;
	}

	ལོག -1;
}

ཧྲིལ get_terminal_dimension(ཧྲིལ *cols, ཧྲིལ *lines)
{
	ཧྲིལ c = 0, l = 0;

#གལ་སྲིད defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	གལ་སྲིད (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	གལ་སྲིད (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	གལ་སྲིད (cols) {
		གལ་སྲིད (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	གལ་སྲིད (lines) {
		གལ་སྲིད (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ལོག 0;
}

ཧྲིལ get_terminal_width(ཧྲིལ default_width)
{
	ཧྲིལ width = 0;

	get_terminal_dimension(&width, NULL);

	ལོག width > 0 ? width : default_width;
}

ཧྲིལ get_terminal_stdfd(void)
{
	གལ་སྲིད (isatty(STDIN_FILENO))
		ལོག STDIN_FILENO;
	གལ་སྲིད (isatty(STDOUT_FILENO))
		ལོག STDOUT_FILENO;
	གལ་སྲིད (isatty(STDERR_FILENO))
		ལོག STDERR_FILENO;

	ལོག -EINVAL;
}

ཧྲིལ get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	ཧྲིལ fd;


	གལ་སྲིད (name)
		*name = NULL;
	གལ་སྲིད (path)
		*path = NULL;
	གལ་སྲིད (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	གལ་སྲིད (fd < 0)
		ལོག fd;	/* error */

	tty = ttyname(fd);
	གལ་སྲིད (!tty)
		ལོག -1;

	གལ་སྲིད (path)
		*path = tty;
	གལ་སྲིད (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	གལ་སྲིད (name)
		*name = tty;
	གལ་སྲིད (number) {
		དོན་ལུ (p = tty; p && *p; p++) {
			གལ་སྲིད (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ལོག 0;
}

ཧྲིལ get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	གལ་སྲིད (*type)
		ལོག -EINVAL;
	ལོག 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, ཧྲིལ is_serial)
{
	གལ་སྲིད (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		གལ་སྲིད (ent && ent->ty_type)
			ལོག strdup(ent->ty_type);
#endif

#གལ་སྲིད defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		གལ་སྲིད (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ལོག strdup(DEFAULT_TTYS0);
		དེ་མིན གལ་སྲིད (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ལོག strdup(DEFAULT_TTY32);
		དེ་མིན གལ་སྲིད (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ལོག strdup(DEFAULT_TTYS1);
#endif
	}

	ལོག strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
ཧྲིལ main(void)
{
	const var_na *path, *name, *num;
	ཧྲིལ c, l;

	གལ་སྲིད (get_terminal_name(&path, &name, &num) == 0) {
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


	ལོག EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
