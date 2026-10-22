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
#  ᮚᮦᮔ᮪ defined (__s390__) || defined (__s390x__)
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
#ᮜᮤᮚ
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

	ᮚᮦᮔ᮪ (cp) {
		var_na *end = NULL;
		long x;

		errno = 0;
		x = strtol(cp, &end, 10);

		ᮚᮦᮔ᮪ (errno == 0 && end && *end == '\0' && end > cp &&
		    x > 0 && x <= INT_MAX)
			ᮝᮜᮤ x;
	}

	ᮝᮜᮤ -1;
}

puur_naa_nka get_terminal_dimension(puur_naa_nka *cols, puur_naa_nka *lines)
{
	puur_naa_nka c = 0, l = 0;

#ᮚᮦᮔ᮪ defined(TIOCGWINSZ)
	sa_nracanaa winsize	w_win;
	ᮚᮦᮔ᮪ (ioctl (STDOUT_FILENO, TIOCGWINSZ, &w_win) == 0) {
		c = w_win.ws_col;
		l = w_win.ws_row;
	}
#elif defined(TIOCGSIZE)
	sa_nracanaa ttysize	t_win;
	ᮚᮦᮔ᮪ (ioctl (STDOUT_FILENO, TIOCGSIZE, &t_win) == 0) {
		c = t_win.ts_cols;
		l = t_win.ts_lines;
	}
#endif
	ᮚᮦᮔ᮪ (cols) {
		ᮚᮦᮔ᮪ (c <= 0)
			c = get_env_int("COLUMNS");
		*cols = c;
	}
	ᮚᮦᮔ᮪ (lines) {
		ᮚᮦᮔ᮪ (l <= 0)
			l = get_env_int("LINES");
		*lines = l;
	}
	ᮝᮜᮤ 0;
}

puur_naa_nka get_terminal_width(puur_naa_nka default_width)
{
	puur_naa_nka width = 0;

	get_terminal_dimension(&width, NULL);

	ᮝᮜᮤ width > 0 ? width : default_width;
}

puur_naa_nka get_terminal_stdfd(void)
{
	ᮚᮦᮔ᮪ (isatty(STDIN_FILENO))
		ᮝᮜᮤ STDIN_FILENO;
	ᮚᮦᮔ᮪ (isatty(STDOUT_FILENO))
		ᮝᮜᮤ STDOUT_FILENO;
	ᮚᮦᮔ᮪ (isatty(STDERR_FILENO))
		ᮝᮜᮤ STDERR_FILENO;

	ᮝᮜᮤ -EINVAL;
}

puur_naa_nka get_terminal_name(const var_na **path,
		      const var_na **name,
		      const var_na **number)
{
	const var_na *tty;
	const var_na *p;
	puur_naa_nka fd;


	ᮚᮦᮔ᮪ (name)
		*name = NULL;
	ᮚᮦᮔ᮪ (path)
		*path = NULL;
	ᮚᮦᮔ᮪ (number)
		*number = NULL;

	fd = get_terminal_stdfd();
	ᮚᮦᮔ᮪ (fd < 0)
		ᮝᮜᮤ fd;	/* error */

	tty = ttyname(fd);
	ᮚᮦᮔ᮪ (!tty)
		ᮝᮜᮤ -1;

	ᮚᮦᮔ᮪ (path)
		*path = tty;
	ᮚᮦᮔ᮪ (name || number)
		tty = strncmp(tty, "/dev/", 5) == 0 ? tty + 5 : tty;
	ᮚᮦᮔ᮪ (name)
		*name = tty;
	ᮚᮦᮔ᮪ (number) {
		for (p = tty; p && *p; p++) {
			ᮚᮦᮔ᮪ (isdigit(*p)) {
				*number = p;
				bha_nga;
			}
		}
	}
	ᮝᮜᮤ 0;
}

puur_naa_nka get_terminal_type(const var_na **type)
{
	*type = getenv("TERM");
	ᮚᮦᮔ᮪ (*type)
		ᮝᮜᮤ -EINVAL;
	ᮝᮜᮤ 0;
}

var_na *get_terminal_default_type(const var_na *ttyname, puur_naa_nka is_serial)
{
	ᮚᮦᮔ᮪ (ttyname) {
#ifdef HAVE_GETTTYNAM
		sa_nracanaa ttyent *ent = getttynam(ttyname);

		ᮚᮦᮔ᮪ (ent && ent->ty_type)
			ᮝᮜᮤ strdup(ent->ty_type);
#endif

#ᮚᮦᮔ᮪ defined (__s390__) || defined (__s390x__)
		/*
		 * Special terminal on first serial line on a S/390(x) which
		 * is due legacy reasons a block terminal of type 3270 or
		 * higher.  Whereas the second serial line on a S/390(x) is
		 * a real character terminal which is compatible with VT220.
		 */
		ᮚᮦᮔ᮪ (strcmp(ttyname, "ttyS0") == 0)		/* linux/drivers/s390/char/con3215.c */
			ᮝᮜᮤ strdup(DEFAULT_TTYS0);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (strncmp(ttyname, "3270/tty", 8) == 0)	/* linux/drivers/s390/char/con3270.c */
			ᮝᮜᮤ strdup(DEFAULT_TTY32);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(ttyname, "ttyS1") == 0)		/* linux/drivers/s390/char/sclp_vt220.c */
			ᮝᮜᮤ strdup(DEFAULT_TTYS1);
#endif
	}

	ᮝᮜᮤ strdup(is_serial ? DEFAULT_STERM : DEFAULT_VCTERM);
}

#ifdef TEST_PROGRAM_TTYUTILS
# include <stdlib.h>
puur_naa_nka main(void)
{
	const var_na *path, *name, *num;
	puur_naa_nka c, l;

	ᮚᮦᮔ᮪ (get_terminal_name(&path, &name, &num) == 0) {
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


	ᮝᮜᮤ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_TTYUTILS */
