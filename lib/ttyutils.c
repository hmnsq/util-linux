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
#  如果 defined (__s390__) || defined (__s390x__)
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
#否则
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static 整数 get_env_int(const char *name)
{
	const char *cp = getenv(name);

	如果 (cp) {
		char *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		如果 (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			返回 x;
	}

	返回 -1;
}

整数 get_terminal_dimension(整数 *cols, 整数 *lines)
{
	整数 c = 0, l = 0;

#如果 defined(TIOCGWINSZ)
	struct winsize	w_win;
	如果 (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	struct ttysize	t_win;
	如果 (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	如果 (cols) {
		如果 (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	如果 (lines) {
		如果 (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	返回 0;
}

整数 get_terminal_width(整数 default_width)
{
	整数 width = 0;

	get_terminal_dimension(&width, NULL);

	返回 width > 0 ? width : default_width;
}

整数 get_terminal_stdfd(void)
{
	如果 (isatty(STDIN_FILENO))
		返回 STDIN_FILENO;
	如果 (isatty(STDOUT_FILENO))
		返回 STDOUT_FILENO;
	如果 (isatty(STDERR_FILENO))
		返回 STDERR_FILENO;

	返回 -EINVAL;
}

整数 get_terminal_name(const char **path,
		      const char **name,
		      const char **number)
{
	const char *tty;
	const char *p;
	整数 fd;


	如果 (name)
		*name = NULL;
	如果 (path)
		*path = NULL;
	如果 (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	如果 (fd < 0)
		返回 fd;	/* error */

	tty = ttyname(fd);
	如果 (!tty)
		返回 -1;

	如果 (path)
		*path = tty;
	如果 (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	如果 (name)
		*name = tty;
	如果 (number) {
		为 (p = tty; p && *p; p++) {
			如果 (isdigit(*p)) {
				*number = p;
				break;
			}
		}
	}
	返回 0;
}

整数 get_terminal_type(const char **type)
{
	*type = getenv("TERM");
	如果 (*type)
		返回 -EINVAL;
	返回 0;
}

char *get_terminal_default_type(const char *ttyname, 整数 is_serial)
{
	如果 (ttyname) {
#ifdef HAVE_GETTTYNAM
		struct ttyent *ent = getttynam(ttyname);

		如果 (ent && ent->ty_type)
			返回 strdup(ent->ty_type);
#endif

#如果 defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		如果 (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			返回 strdup(DEFAULT_TTYS0);
		否则 如果 (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			返回 strdup(DEFAULT_TTY32);
		否则 如果 (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			返回 strdup(DEFAULT_TTYS1);
#endif
	}

	返回 strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
整数 主要(void)
{
	const char *path, *name, *num;
	整数 c, l;

	如果 (get_terminal_name(&path, &name, &num) == 0) {
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


	返回 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
