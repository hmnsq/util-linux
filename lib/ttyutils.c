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
#  если defined (__s390__) || defined (__s390x__)
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
#иначе
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static целое get_env_int(const char *name)
{
	const char *cp = getenv(name);

	если (cp) {
		char *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		если (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			вернуть x;
	}

	вернуть -1;
}

целое get_terminal_dimension(целое *cols, целое *lines)
{
	целое c = 0, l = 0;

#если defined(TIOCGWINSZ)
	struct winsize	w_win;
	если (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	struct ttysize	t_win;
	если (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	если (cols) {
		если (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	если (lines) {
		если (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	вернуть 0;
}

целое get_terminal_width(целое default_width)
{
	целое width = 0;

	get_terminal_dimension(&width, NULL);

	вернуть width > 0 ? width : default_width;
}

целое get_terminal_stdfd(void)
{
	если (isatty(STDIN_FILENO))
		вернуть STDIN_FILENO;
	если (isatty(STDOUT_FILENO))
		вернуть STDOUT_FILENO;
	если (isatty(STDERR_FILENO))
		вернуть STDERR_FILENO;

	вернуть -EINVAL;
}

целое get_terminal_name(const char **path,
		      const char **name,
		      const char **number)
{
	const char *tty;
	const char *p;
	целое fd;


	если (name)
		*name = NULL;
	если (path)
		*path = NULL;
	если (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	если (fd < 0)
		вернуть fd;	/* error */

	tty = ttyname(fd);
	если (!tty)
		вернуть -1;

	если (path)
		*path = tty;
	если (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	если (name)
		*name = tty;
	если (number) {
		для (p = tty; p && *p; p++) {
			если (isdigit(*p)) {
				*number = p;
				break;
			}
		}
	}
	вернуть 0;
}

целое get_terminal_type(const char **type)
{
	*type = getenv("TERM");
	если (*type)
		вернуть -EINVAL;
	вернуть 0;
}

char *get_terminal_default_type(const char *ttyname, целое is_serial)
{
	если (ttyname) {
#ifdef HAVE_GETTTYNAM
		struct ttyent *ent = getttynam(ttyname);

		если (ent && ent->ty_type)
			вернуть strdup(ent->ty_type);
#endif

#если defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		если (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			вернуть strdup(DEFAULT_TTYS0);
		иначе если (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			вернуть strdup(DEFAULT_TTY32);
		иначе если (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			вернуть strdup(DEFAULT_TTYS1);
#endif
	}

	вернуть strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
целое основной(void)
{
	const char *path, *name, *num;
	целое c, l;

	если (get_terminal_name(&path, &name, &num) == 0) {
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


	вернуть EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
