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
#  ᯀᯮᯝ defined (__s390__) || defined (__s390x__)
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
#ᯀᯮᯝᯂᯪ
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

	ᯀᯮᯝ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ᯀᯮᯝ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ᯀᯮᯞ x;
	}

	ᯀᯮᯞ -1;
}

puur_naa_nka get_terminal_dimension(puur_naa_nka *cols, puur_naa_nka *lines)
{
	puur_naa_nka c = 0, l = 0;

#ᯀᯮᯝ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ᯀᯮᯝ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ᯀᯮᯝ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ᯀᯮᯝ (cols) {
		ᯀᯮᯝ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ᯀᯮᯝ (lines) {
		ᯀᯮᯝ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ᯀᯮᯞ 0;
}

puur_naa_nka get_terminal_width(puur_naa_nka default_width)
{
	puur_naa_nka width = 0;

	get_terminal_dimension(&width, NULL);

	ᯀᯮᯞ width > 0 ? width : default_width;
}

puur_naa_nka get_terminal_stdfd(void)
{
	ᯀᯮᯝ (isatty(STDIN_FILENO))
		ᯀᯮᯞ STDIN_FILENO;
	ᯀᯮᯝ (isatty(STDOUT_FILENO))
		ᯀᯮᯞ STDOUT_FILENO;
	ᯀᯮᯝ (isatty(STDERR_FILENO))
		ᯀᯮᯞ STDERR_FILENO;

	ᯀᯮᯞ -EINVAL;
}

puur_naa_nka get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	puur_naa_nka fd;


	ᯀᯮᯝ (name)
		*name = NULL;
	ᯀᯮᯝ (path)
		*path = NULL;
	ᯀᯮᯝ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ᯀᯮᯝ (fd < 0)
		ᯀᯮᯞ fd;	/* error */

	tty = ttyname(fd);
	ᯀᯮᯝ (!tty)
		ᯀᯮᯞ -1;

	ᯀᯮᯝ (path)
		*path = tty;
	ᯀᯮᯝ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ᯀᯮᯝ (name)
		*name = tty;
	ᯀᯮᯝ (number) {
		for (p = tty; p && *p; p++) {
			ᯀᯮᯝ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ᯀᯮᯞ 0;
}

puur_naa_nka get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ᯀᯮᯝ (*type)
		ᯀᯮᯞ -EINVAL;
	ᯀᯮᯞ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, puur_naa_nka is_serial)
{
	ᯀᯮᯝ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ᯀᯮᯝ (ent && ent->ty_type)
			ᯀᯮᯞ strdup(ent->ty_type);
#endif

#ᯀᯮᯝ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ᯀᯮᯝ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ᯀᯮᯞ strdup(DEFAULT_TTYS0);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ᯀᯮᯞ strdup(DEFAULT_TTY32);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ᯀᯮᯞ strdup(DEFAULT_TTYS1);
#endif
	}

	ᯀᯮᯞ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
puur_naa_nka main(void)
{
	const var_na *path, *name, *num;
	puur_naa_nka c, l;

	ᯀᯮᯝ (get_terminal_name(&path, &name, &num) == 0) {
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


	ᯀᯮᯞ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
