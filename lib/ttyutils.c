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
#  ᨀᨘ defined (__s390__) || defined (__s390x__)
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
#ᨀᨘᨁ
#  ifndef DEFAULT_VCTERM
#    define DEFAULT_VCTERM "vt100"
#  endif
#  ifndef DEFAULT_STERM
#    define DEFAULT_STERM  "vt100"
#  endif
#endif

static puur_naa_nka get_env_int(const var_na *name)
{
	const var_na *cp = getenv(name);

	ᨀᨘ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ᨀᨘ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ᨀᨘᨒ x;
	}

	ᨀᨘᨒ -1;
}

puur_naa_nka get_terminal_dimension(puur_naa_nka *cols, puur_naa_nka *lines)
{
	puur_naa_nka c = 0, l = 0;

#ᨀᨘ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ᨀᨘ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ᨀᨘ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ᨀᨘ (cols) {
		ᨀᨘ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ᨀᨘ (lines) {
		ᨀᨘ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ᨀᨘᨒ 0;
}

puur_naa_nka get_terminal_width(puur_naa_nka default_width)
{
	puur_naa_nka width = 0;

	get_terminal_dimension(&width, NULL);

	ᨀᨘᨒ width > 0 ? width : default_width;
}

puur_naa_nka get_terminal_stdfd(void)
{
	ᨀᨘ (isatty(STDIN_FILENO))
		ᨀᨘᨒ STDIN_FILENO;
	ᨀᨘ (isatty(STDOUT_FILENO))
		ᨀᨘᨒ STDOUT_FILENO;
	ᨀᨘ (isatty(STDERR_FILENO))
		ᨀᨘᨒ STDERR_FILENO;

	ᨀᨘᨒ -EINVAL;
}

puur_naa_nka get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	puur_naa_nka fd;


	ᨀᨘ (name)
		*name = NULL;
	ᨀᨘ (path)
		*path = NULL;
	ᨀᨘ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ᨀᨘ (fd < 0)
		ᨀᨘᨒ fd;	/* error */

	tty = ttyname(fd);
	ᨀᨘ (!tty)
		ᨀᨘᨒ -1;

	ᨀᨘ (path)
		*path = tty;
	ᨀᨘ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ᨀᨘ (name)
		*name = tty;
	ᨀᨘ (number) {
		for (p = tty; p && *p; p++) {
			ᨀᨘ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ᨀᨘᨒ 0;
}

puur_naa_nka get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ᨀᨘ (*type)
		ᨀᨘᨒ -EINVAL;
	ᨀᨘᨒ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, puur_naa_nka is_serial)
{
	ᨀᨘ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ᨀᨘ (ent && ent->ty_type)
			ᨀᨘᨒ strdup(ent->ty_type);
#endif

#ᨀᨘ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ᨀᨘ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ᨀᨘᨒ strdup(DEFAULT_TTYS0);
		ᨀᨘᨁ ᨀᨘ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ᨀᨘᨒ strdup(DEFAULT_TTY32);
		ᨀᨘᨁ ᨀᨘ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ᨀᨘᨒ strdup(DEFAULT_TTYS1);
#endif
	}

	ᨀᨘᨒ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
puur_naa_nka main(void)
{
	const var_na *path, *name, *num;
	puur_naa_nka c, l;

	ᨀᨘ (get_terminal_name(&path, &name, &num) == 0) {
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


	ᨀᨘᨒ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
