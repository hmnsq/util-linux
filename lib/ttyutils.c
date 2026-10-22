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
#  ако defined (__s390__) || defined (__s390x__)
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
#инако
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static цео get_env_int(const char *name)
{
	const char *cp = getenv(name);

	ако (cp) {
		char *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ако (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			врати x;
	}

	врати -1;
}

цео get_terminal_dimension(цео *cols, цео *lines)
{
	цео c = 0, l = 0;

#ако defined(TIOCGWINSZ)
	struct winsize	w_win;
	ако (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	struct ttysize	t_win;
	ако (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ако (cols) {
		ако (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ако (lines) {
		ако (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	врати 0;
}

цео get_terminal_width(цео default_width)
{
	цео width = 0;

	get_terminal_dimension(&width, NULL);

	врати width > 0 ? width : default_width;
}

цео get_terminal_stdfd(void)
{
	ако (isatty(STDIN_FILENO))
		врати STDIN_FILENO;
	ако (isatty(STDOUT_FILENO))
		врати STDOUT_FILENO;
	ако (isatty(STDERR_FILENO))
		врати STDERR_FILENO;

	врати -EINVAL;
}

цео get_terminal_name(const char **path,
		      const char **name,
		      const char **number)
{
	const char *tty;
	const char *p;
	цео fd;


	ако (name)
		*name = NULL;
	ако (path)
		*path = NULL;
	ако (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ако (fd < 0)
		врати fd;	/* error */

	tty = ttyname(fd);
	ако (!tty)
		врати -1;

	ако (path)
		*path = tty;
	ако (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ако (name)
		*name = tty;
	ако (number) {
		за (p = tty; p && *p; p++) {
			ако (isdigit(*p)) {
				*number = p;
				break;
			}
		}
	}
	врати 0;
}

цео get_terminal_type(const char **type)
{
	*type = getenv("TERM");
	ако (*type)
		врати -EINVAL;
	врати 0;
}

char *get_terminal_default_type(const char *ttyname, цео is_serial)
{
	ако (ttyname) {
#ifdef HAVE_GETTTYNAM
		struct ttyent *ent = getttynam(ttyname);

		ако (ent && ent->ty_type)
			врати strdup(ent->ty_type);
#endif

#ако defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ако (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			врати strdup(DEFAULT_TTYS0);
		инако ако (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			врати strdup(DEFAULT_TTY32);
		инако ако (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			врати strdup(DEFAULT_TTYS1);
#endif
	}

	врати strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
цео main(void)
{
	const char *path, *name, *num;
	цео c, l;

	ако (get_terminal_name(&path, &name, &num) == 0) {
		char *term;

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


	врати EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
