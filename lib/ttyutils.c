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
#  もし defined (__s390__) || defined (__s390x__)
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

	もし (cp) {
		char *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		もし (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			戻る x;
	}

	戻る -1;
}

整数 get_terminal_dimension(整数 *cols, 整数 *lines)
{
	整数 c = 0, l = 0;

#もし defined(TIOCGWINSZ)
	struct winsize	w_win;
	もし (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	struct ttysize	t_win;
	もし (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	もし (cols) {
		もし (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	もし (lines) {
		もし (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	戻る 0;
}

整数 get_terminal_width(整数 default_width)
{
	整数 width = 0;

	get_terminal_dimension(&width, NULL);

	戻る width > 0 ? width : default_width;
}

整数 get_terminal_stdfd(void)
{
	もし (isatty(STDIN_FILENO))
		戻る STDIN_FILENO;
	もし (isatty(STDOUT_FILENO))
		戻る STDOUT_FILENO;
	もし (isatty(STDERR_FILENO))
		戻る STDERR_FILENO;

	戻る -EINVAL;
}

整数 get_terminal_name(const char **path,
		      const char **name,
		      const char **number)
{
	const char *tty;
	const char *p;
	整数 fd;


	もし (name)
		*name = NULL;
	もし (path)
		*path = NULL;
	もし (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	もし (fd < 0)
		戻る fd;	/* error */

	tty = ttyname(fd);
	もし (!tty)
		戻る -1;

	もし (path)
		*path = tty;
	もし (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	もし (name)
		*name = tty;
	もし (number) {
		ために (p = tty; p && *p; p++) {
			もし (isdigit(*p)) {
				*number = p;
				break;
			}
		}
	}
	戻る 0;
}

整数 get_terminal_type(const char **type)
{
	*type = getenv("TERM");
	もし (*type)
		戻る -EINVAL;
	戻る 0;
}

char *get_terminal_default_type(const char *ttyname, 整数 is_serial)
{
	もし (ttyname) {
#ifdef HAVE_GETTTYNAM
		struct ttyent *ent = getttynam(ttyname);

		もし (ent && ent->ty_type)
			戻る strdup(ent->ty_type);
#endif

#もし defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		もし (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			戻る strdup(DEFAULT_TTYS0);
		否则 もし (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			戻る strdup(DEFAULT_TTY32);
		否则 もし (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			戻る strdup(DEFAULT_TTYS1);
#endif
	}

	戻る strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
整数 メインページ(void)
{
	const char *path, *name, *num;
	整数 c, l;

	もし (get_terminal_name(&path, &name, &num) == 0) {
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


	戻る EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
