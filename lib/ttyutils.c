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
#  nếu defined (__s390__) || defined (__s390x__)
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
#không_thì
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static số_nguyên get_env_int(const char *name)
{
	const char *cp = getenv(name);

	nếu (cp) {
		char *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		nếu (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			trả_về x;
	}

	trả_về -1;
}

số_nguyên get_terminal_dimension(số_nguyên *cols, số_nguyên *lines)
{
	số_nguyên c = 0, l = 0;

#nếu defined(TIOCGWINSZ)
	struct winsize	w_win;
	nếu (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	struct ttysize	t_win;
	nếu (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	nếu (cols) {
		nếu (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	nếu (lines) {
		nếu (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	trả_về 0;
}

số_nguyên get_terminal_width(số_nguyên default_width)
{
	số_nguyên width = 0;

	get_terminal_dimension(&width, NULL);

	trả_về width > 0 ? width : default_width;
}

số_nguyên get_terminal_stdfd(void)
{
	nếu (isatty(STDIN_FILENO))
		trả_về STDIN_FILENO;
	nếu (isatty(STDOUT_FILENO))
		trả_về STDOUT_FILENO;
	nếu (isatty(STDERR_FILENO))
		trả_về STDERR_FILENO;

	trả_về -EINVAL;
}

số_nguyên get_terminal_name(const char **path,
		      const char **name,
		      const char **number)
{
	const char *tty;
	const char *p;
	số_nguyên fd;


	nếu (name)
		*name = NULL;
	nếu (path)
		*path = NULL;
	nếu (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	nếu (fd < 0)
		trả_về fd;	/* error */

	tty = ttyname(fd);
	nếu (!tty)
		trả_về -1;

	nếu (path)
		*path = tty;
	nếu (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	nếu (name)
		*name = tty;
	nếu (number) {
		cho (p = tty; p && *p; p++) {
			nếu (isdigit(*p)) {
				*number = p;
				break;
			}
		}
	}
	trả_về 0;
}

số_nguyên get_terminal_type(const char **type)
{
	*type = getenv("TERM");
	nếu (*type)
		trả_về -EINVAL;
	trả_về 0;
}

char *get_terminal_default_type(const char *ttyname, số_nguyên is_serial)
{
	nếu (ttyname) {
#ifdef HAVE_GETTTYNAM
		struct ttyent *ent = getttynam(ttyname);

		nếu (ent && ent->ty_type)
			trả_về strdup(ent->ty_type);
#endif

#nếu defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		nếu (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			trả_về strdup(DEFAULT_TTYS0);
		không_thì nếu (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			trả_về strdup(DEFAULT_TTY32);
		không_thì nếu (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			trả_về strdup(DEFAULT_TTYS1);
#endif
	}

	trả_về strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
số_nguyên main(void)
{
	const char *path, *name, *num;
	số_nguyên c, l;

	nếu (get_terminal_name(&path, &name, &num) == 0) {
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


	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
